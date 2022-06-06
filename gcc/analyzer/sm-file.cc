/* A state machine for detecting misuses of <stdio.h>'s FILE * API.
   Copyright (C) 2019-2022 Free Software Foundation, Inc.
   Contributed by David Malcolm <dmalcolm@redhat.com>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "function.h"
#include "basic-block.h"
#include "gimple.h"
#include "options.h"
#include "diagnostic-path.h"
#include "diagnostic-metadata.h"
#include "function.h"
#include "json.h"
#include "analyzer/analyzer.h"
#include "diagnostic-event-id.h"
#include "analyzer/analyzer-logging.h"
#include "analyzer/sm.h"
#include "analyzer/pending-diagnostic.h"
#include "analyzer/function-set.h"
#include "analyzer/analyzer-selftests.h"
#include "tristate.h"
#include "selftest.h"
#include "analyzer/call-string.h"
#include "analyzer/program-point.h"
#include "analyzer/store.h"
#include "analyzer/region-model.h"
#include "analyzer/program-state.h"
#include "print-tree.h"

#if ENABLE_ANALYZER

namespace ana {

namespace {

/* A state machine for detecting misuses of <stdio.h>'s FILE * API.  */

class fileptr_state_machine : public state_machine
{
public:
  fileptr_state_machine (logger *logger);

  bool inherited_state_p () const final override { return false; }

  state_machine::state_t
  get_default_state (const svalue *sval) const final override
  {
    if (tree cst = sval->maybe_get_constant ())
      {
	if (zerop (cst))
	  return m_null;
      }
    return m_start;
  }

  bool on_stmt (sm_context *sm_ctxt,
		const supernode *node,
		const gimple *stmt) const final override;

  void on_condition (sm_context *sm_ctxt,
		     const supernode *node,
		     const gimple *stmt,
		     const svalue *lhs,
		     enum tree_code op,
		     const svalue *rhs) const final override;

  bool can_purge_p (state_t s) const final override;
  pending_diagnostic *on_leak (tree var) const final override;

  /* State for a FILE * returned from fopen that hasn't been checked for
     NULL.
     It could be an open stream, or could be NULL.  */
  state_t m_unchecked;

  /* State for a FILE * that's known to be NULL.  */
  state_t m_null;

  /* State for a FILE * that's known to be a non-NULL open stream.  */
  state_t m_nonnull;

  /* State for a FILE * that's had fclose called on it.  */
  state_t m_closed;

  /* Stop state, for a FILE * we don't want to track any more.  */
  state_t m_stop;
};

/* Base class for diagnostics relative to fileptr_state_machine.  */

class file_diagnostic : public pending_diagnostic
{
public:
  file_diagnostic (const fileptr_state_machine &sm, tree arg)
  : m_sm (sm), m_arg (arg)
  {}

  bool subclass_equal_p (const pending_diagnostic &base_other) const override
  {
    return same_tree_p (m_arg, ((const file_diagnostic &)base_other).m_arg);
  }

  label_text describe_state_change (const evdesc::state_change &change)
    override
  {
    if (change.m_old_state == m_sm.get_start_state ()
	&& change.m_new_state == m_sm.m_unchecked)
      // TODO: verify that it's the fopen stmt, not a copy
      return label_text::borrow ("opened here");
    if (change.m_old_state == m_sm.m_unchecked
	&& change.m_new_state == m_sm.m_nonnull)
      {
	if (change.m_expr)
	  return change.formatted_print ("assuming %qE is non-NULL",
					 change.m_expr);
	else
	  return change.formatted_print ("assuming FILE * is non-NULL");
      }
    if (change.m_new_state == m_sm.m_null)
      {
	if (change.m_expr)
	  return change.formatted_print ("assuming %qE is NULL",
					 change.m_expr);
	else
	  return change.formatted_print ("assuming FILE * is NULL");
      }
    return label_text ();
  }

protected:
  const fileptr_state_machine &m_sm;
  tree m_arg;
};

class double_fclose : public file_diagnostic
{
public:
  double_fclose (const fileptr_state_machine &sm, tree arg)
    : file_diagnostic (sm, arg)
  {}

  const char *get_kind () const final override { return "double_fclose"; }

  int get_controlling_option () const final override
  {
    return OPT_Wanalyzer_double_fclose;
  }

  bool emit (rich_location *rich_loc) final override
  {
    return warning_at (rich_loc, get_controlling_option (),
		       "double %<fclose%> of FILE %qE",
		       m_arg);
  }

  label_text describe_state_change (const evdesc::state_change &change)
    override
  {
    if (change.m_new_state == m_sm.m_closed)
      {
	m_first_fclose_event = change.m_event_id;
	return change.formatted_print ("first %qs here", "fclose");
      }
    return file_diagnostic::describe_state_change (change);
  }

  label_text describe_final_event (const evdesc::final_event &ev) final override
  {
    if (m_first_fclose_event.known_p ())
      return ev.formatted_print ("second %qs here; first %qs was at %@",
				 "fclose", "fclose",
				 &m_first_fclose_event);
    return ev.formatted_print ("second %qs here", "fclose");
  }

private:
  diagnostic_event_id_t m_first_fclose_event;
};

class file_leak : public file_diagnostic
{
public:
  file_leak (const fileptr_state_machine &sm, tree arg)
    : file_diagnostic (sm, arg)
  {}

  const char *get_kind () const final override { return "file_leak"; }

  int get_controlling_option () const final override
  {
    return OPT_Wanalyzer_file_leak;
  }

  bool emit (rich_location *rich_loc) final override
  {
    diagnostic_metadata m;
    /* CWE-775: "Missing Release of File Descriptor or Handle after
       Effective Lifetime". */
    m.add_cwe (775);
    if (m_arg)
      return warning_meta (rich_loc, m, get_controlling_option (),
			   "leak of FILE %qE",
			   m_arg);
    else
      return warning_meta (rich_loc, m, get_controlling_option (),
			   "leak of FILE");
  }

  label_text describe_state_change (const evdesc::state_change &change)
    final override
  {
    if (change.m_new_state == m_sm.m_unchecked)
      {
	m_fopen_event = change.m_event_id;
	return label_text::borrow ("opened here");
      }
    return file_diagnostic::describe_state_change (change);
  }

  label_text describe_final_event (const evdesc::final_event &ev) final override
  {
    if (m_fopen_event.known_p ())
      {
	if (ev.m_expr)
	  return ev.formatted_print ("%qE leaks here; was opened at %@",
				     ev.m_expr, &m_fopen_event);
	else
	  return ev.formatted_print ("leaks here; was opened at %@",
				     &m_fopen_event);
      }
    else
      {
	if (ev.m_expr)
	  return ev.formatted_print ("%qE leaks here", ev.m_expr);
	else
	  return ev.formatted_print ("leaks here");
      }
  }

private:
  diagnostic_event_id_t m_fopen_event;
};

/* fileptr_state_machine's ctor.  */

fileptr_state_machine::fileptr_state_machine (logger *logger)
: state_machine ("file", logger)
{
  m_unchecked = add_state ("unchecked");
  m_null = add_state ("null");
  m_nonnull = add_state ("nonnull");
  m_closed = add_state ("closed");
  m_stop = add_state ("stop");
}

/* Get a set of functions that are known to take a FILE * that must be open,
   and are known to not close it.  */

static function_set
get_file_using_fns ()
{
  // TODO: populate this list more fully
  static const char * const funcnames[] = {
    /* This array must be kept sorted.  */
    "__fbufsize",
    "__flbf",
    "__fpending",
    "__fpurge",
    "__freadable",
    "__freading",
    "__fsetlocking",
    "__fwritable",
    "__fwriting",
    "clearerr",
    "clearerr_unlocked",
    "feof",
    "feof_unlocked",
    "ferror",
    "ferror_unlocked",
    "fflush", // safe to call with NULL
    "fflush_unlocked",  // safe to call with NULL
    "fgetc",
    "fgetc_unlocked",
    "fgetpos",
    "fgets",
    "fgets_unlocked",
    "fgetwc_unlocked",
    "fgetws_unlocked",
    "fileno",
    "fileno_unlocked",
    "fprintf",
    "fputc",
    "fputc_unlocked",
    "fputs",
    "fputs_unlocked",
    "fputwc_unlocked",
    "fputws_unlocked",
    "fread_unlocked",
    "fseek",
    "fsetpos",
    "ftell",
    "fwrite_unlocked",
    "getc",
    "getc_unlocked",
    "getwc_unlocked",
    "putc",
    "putc_unlocked",
    "rewind",
    "setbuf",
    "setbuffer",
    "setlinebuf",
    "setvbuf",
    "ungetc",
    "vfprintf"
  };
  const size_t count = ARRAY_SIZE (funcnames);
  function_set fs (funcnames, count);
  return fs;
}

/* Return true if FNDECL is known to require an open FILE *, and is known
   to not close it.  */

static bool
is_file_using_fn_p (tree fndecl)
{
  function_set fs = get_file_using_fns ();
  if (fs.contains_decl_p (fndecl))
    return true;

  /* Also support variants of these names prefixed with "_IO_".  */
  const char *name = IDENTIFIER_POINTER (DECL_NAME (fndecl));
  if (startswith (name, "_IO_") && fs.contains_name_p (name + 4))
    return true;

  return false;
}

/* Implementation of state_machine::on_stmt vfunc for fileptr_state_machine.  */

bool
fileptr_state_machine::on_stmt (sm_context *sm_ctxt,
				const supernode *node,
				const gimple *stmt) const
{
  if (const gcall *call = dyn_cast <const gcall *> (stmt))
    if (tree callee_fndecl = sm_ctxt->get_fndecl_for_call (call))
      {
	if (is_named_call_p (callee_fndecl, "fopen", call, 2))
	  {
	    tree lhs = gimple_call_lhs (call);
	    if (lhs)
	      sm_ctxt->on_transition (node, stmt, lhs, m_start, m_unchecked);
	    else
	      {
		/* TODO: report leak.  */
	      }
	    return true;
	  }

	if (is_named_call_p (callee_fndecl, "fclose", call, 1))
	  {
	    tree arg = gimple_call_arg (call, 0);

	    sm_ctxt->on_transition (node, stmt, arg, m_start, m_closed);

	    // TODO: is it safe to call fclose (NULL) ?
	    sm_ctxt->on_transition (node, stmt, arg, m_unchecked, m_closed);
	    sm_ctxt->on_transition (node, stmt, arg, m_null, m_closed);

	    sm_ctxt->on_transition (node, stmt , arg, m_nonnull, m_closed);

	    if (sm_ctxt->get_state (stmt, arg) == m_closed)
	      {
		tree diag_arg = sm_ctxt->get_diagnostic_tree (arg);
		sm_ctxt->warn (node, stmt, arg,
			       new double_fclose (*this, diag_arg));
		sm_ctxt->set_next_state (stmt, arg, m_stop);
	      }
	    return true;
	  }

  // XXX: mkstemp & mktemp
  if (is_named_call_p (callee_fndecl, "mkstemp", call, 1))
    {
      tree arg = gimple_call_arg (call, 0);
      const program_state *state = sm_ctxt->get_old_program_state ();
      const svalue *r_value = state->m_region_model->get_rvalue (arg, NULL);
      if (const region_svalue *reg = dyn_cast <const region_svalue *> (r_value)) 
        {
          if (const string_region *str_reg = dyn_cast <const string_region *> (reg->get_pointee())) 
            {
              tree str_cst = str_reg->get_string_cst ();
              size_t len = TREE_STRING_LENGTH (str_cst);
              const char *str = TREE_STRING_POINTER (str_cst);
              const char *last = str + (len - 7);
              if (len < 7 || strncmp(last, "XXXXXX", 6) != 0) 
                {
                  rich_location *rich_loc = new rich_location (line_table, gimple_location (stmt));
                  diagnostic_metadata m;
                  warning_meta (rich_loc, m, 0, "Invalid argument. The last six characters of template have to be XXXXXX.");
                }
            } else if (const decl_region *decl_reg = dyn_cast <const decl_region *> (reg->get_pointee()))
            {
              // const svalue *str = decl_reg->maybe_get_constant_value (state->m_region_model->get_manager ());
              decl_reg->dump(false);
            }
        }
    }
  // TODO: warns already in the linker???
  if (is_named_call_p (callee_fndecl, "mktemp", call, 1))
    {
      diagnostic_metadata m;
      m.add_cwe(273);
      rich_location *rich_loc = new rich_location (line_table, gimple_location (stmt));
      // TODO: opt param
      warning_meta (rich_loc, m, 0, "Usage of mktemp is insecure. Use mkstemp instead.");
    }

	if (is_file_using_fn_p (callee_fndecl))
	  {
	    // TODO: operations on unchecked file
	    return true;
	  }
	// etc
      }

  return false;
}

/* Implementation of state_machine::on_condition vfunc for
   fileptr_state_machine.
   Potentially transition state 'unchecked' to 'nonnull' or to 'null'.  */

void
fileptr_state_machine::on_condition (sm_context *sm_ctxt,
				     const supernode *node,
				     const gimple *stmt,
				     const svalue *lhs,
				     enum tree_code op,
				     const svalue *rhs) const
{
  if (!rhs->all_zeroes_p ())
    return;

  // TODO: has to be a FILE *, specifically
  if (!any_pointer_p (lhs))
    return;
  // TODO: has to be a FILE *, specifically
  if (!any_pointer_p (rhs))
    return;

  if (op == NE_EXPR)
    {
      log ("got 'ARG != 0' match");
      sm_ctxt->on_transition (node, stmt,
			      lhs, m_unchecked, m_nonnull);
    }
  else if (op == EQ_EXPR)
    {
      log ("got 'ARG == 0' match");
      sm_ctxt->on_transition (node, stmt,
			      lhs, m_unchecked, m_null);
    }
}

/* Implementation of state_machine::can_purge_p vfunc for fileptr_state_machine.
   Don't allow purging of pointers in state 'unchecked' or 'nonnull'
   (to avoid false leak reports).  */

bool
fileptr_state_machine::can_purge_p (state_t s) const
{
  return s != m_unchecked && s != m_nonnull;
}

/* Implementation of state_machine::on_leak vfunc for
   fileptr_state_machine, for complaining about leaks of FILE * in
   state 'unchecked' and 'nonnull'.  */

pending_diagnostic *
fileptr_state_machine::on_leak (tree var) const
{
  return new file_leak (*this, var);
}

// XXX: zero state machine
class zero_state_machine : public state_machine
{
public:
  zero_state_machine(logger *logger);

  bool
  inherited_state_p () const final override
  {
    return false;
  }

  state_machine::state_t
  get_default_state (const svalue *sval) const final override
  {
    if (tree cst = sval->maybe_get_constant ())
      {
        if (zerop (cst))
          return m_zero;
      }
    return m_start;
  }

  bool on_stmt (sm_context *sm_ctxt, const supernode *node,
                const gimple *stmt) const final override;
  void
  on_phi (sm_context *sm_ctxt ATTRIBUTE_UNUSED,
          const supernode *node ATTRIBUTE_UNUSED,
          const gphi *phi ATTRIBUTE_UNUSED, tree rhs ATTRIBUTE_UNUSED) const final override;

  void on_condition (sm_context *sm_ctxt ATTRIBUTE_UNUSED,
                     const supernode *node ATTRIBUTE_UNUSED,
                     const gimple *stmt ATTRIBUTE_UNUSED,
                     const svalue *lhs ATTRIBUTE_UNUSED,
                     enum tree_code op ATTRIBUTE_UNUSED,
                     const svalue *rhs ATTRIBUTE_UNUSED) const final override;

  bool can_purge_p (state_t s) const final override;

  bool
  reset_when_passed_to_unknown_fn_p (state_t s ATTRIBUTE_UNUSED,
                                     bool is_mutable) const final override
  {
    return is_mutable;
  }

protected:
  state_t m_unknown;
  state_t m_maybe_zero;
  state_t m_zero;
  state_t m_not_zero;
};

zero_state_machine::zero_state_machine (logger *logger)
    : state_machine ("zero", logger)
{
  m_unknown = add_state ("unknown");
  m_maybe_zero = add_state ("maybezero");  
  m_zero = add_state ("zero");
  m_not_zero = add_state ("notzero");
}

bool
zero_state_machine::on_stmt (sm_context *sm_ctxt, const supernode *node,
                             const gimple *stmt) const
{
  if (const gassign *assign_stmt = dyn_cast<const gassign *> (stmt))
    {
      tree lhs = gimple_assign_lhs (assign_stmt);

      state_t current = sm_ctxt->get_state (stmt, lhs);
      if (current == m_unknown)
        return false;

      tree_code tc = gimple_assign_rhs_code (assign_stmt);
      switch (tc)
        {
        case INTEGER_CST:
        {
          tree rhs = gimple_assign_rhs1 (assign_stmt);
          if (integer_zerop (rhs))
            sm_ctxt->on_transition (node, stmt, lhs, m_start, m_zero);
          else
            sm_ctxt->on_transition (node, stmt, lhs, m_start, m_not_zero);
          break;
        }
        case TRUNC_DIV_EXPR:
        case CEIL_DIV_EXPR:
        case FLOOR_DIV_EXPR:
        case ROUND_DIV_EXPR:
        {
            tree divisor = gimple_assign_rhs2 (assign_stmt);
            if (sm_ctxt->get_state (stmt, divisor) == m_zero)
              inform(UNKNOWN_LOCATION, "Divide by zero");
            break;
        }
        default:
          break;
        }
    }
  
  return false;
}

void
zero_state_machine::on_condition (sm_context *sm_ctxt, const supernode *node,
                                  const gimple *stmt, const svalue *lhs,
                                  enum tree_code op, const svalue *rhs) const
{
  state_t current = sm_ctxt->get_state (stmt, lhs);
  if (current == m_unknown)
    return;

  tree cst = rhs->maybe_get_constant ();
  if (!cst)
    {
      // If rhs is not known, just go back to start
      sm_ctxt->on_transition (node, stmt, lhs, current, m_start);
      return;
    }

  bool isZero = integer_zerop (cst);
  bool isNonZero = integer_nonzerop (cst);
  if ((op == NE_EXPR && isZero) || (op == EQ_EXPR && isNonZero))
    {
      log ("got 'ARG != 0' or 'ARG == c' match");
      sm_ctxt->on_transition (node, stmt, lhs, m_start, m_not_zero);
    }
  else if (op == EQ_EXPR && isZero)
    {
      log ("got 'ARG == 0' match");
      sm_ctxt->on_transition (node, stmt, lhs, m_start, m_zero);
    }
}

void
zero_state_machine::on_phi (sm_context *sm_ctxt ATTRIBUTE_UNUSED,
                            const supernode *node ATTRIBUTE_UNUSED,
                            const gphi *phi ATTRIBUTE_UNUSED, 
                            tree rhs ATTRIBUTE_UNUSED) const
{
  tree lhs = gimple_phi_result(phi);
  bool could_be_zero = false;
  bool could_be_non_zero = false;
  for (int i = 0; i < phi->nargs; i++) 
    {
      tree arg = gimple_phi_arg(phi, i)->def;
      state_t state = sm_ctxt->get_state(phi, arg);
      if (state == m_zero)
        could_be_zero = true;
      else if (state == m_not_zero)
        could_be_non_zero = true;

      if (could_be_zero && could_be_non_zero)
        break;
    }
  
  if (could_be_zero && could_be_non_zero)
    sm_ctxt->on_transition(node, phi, lhs, m_start, m_maybe_zero);
  else if (could_be_zero)
    sm_ctxt->on_transition(node, phi, lhs, m_start, m_zero);
  else if (could_be_non_zero)
    sm_ctxt->on_transition(node, phi, lhs, m_start, m_not_zero);
}

bool
zero_state_machine::can_purge_p (state_t s) const
{
  return false;
}

} // anonymous namespace

/* Internal interface to this file. */

state_machine *
make_fileptr_state_machine (logger *logger)
{
  return new fileptr_state_machine (logger);
}

state_machine *
make_zero_state_machine (logger *logger)
{
  return new zero_state_machine (logger);
}

#if CHECKING_P

namespace selftest {

/* Run all of the selftests within this file.  */

void
analyzer_sm_file_cc_tests ()
{
  function_set fs = get_file_using_fns ();
  fs.assert_sorted ();
  fs.assert_sane ();
}

} // namespace selftest

#endif /* CHECKING_P */

} // namespace ana

#endif /* #if ENABLE_ANALYZER */
