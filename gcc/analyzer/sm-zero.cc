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
#include "gimple-pretty-print.h"

#if ENABLE_ANALYZER

namespace ana {
namespace {

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
    // if (tree cst = sval->maybe_get_constant ())
    //   {
    //     if (zerop (cst))
    //       return m_zero;
    //   }
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

  state_t m_unknown;
  state_t m_maybe_zero;
  state_t m_zero;
  state_t m_not_zero;
};

class zero_diagnostic : public pending_diagnostic
{
 public:
  zero_diagnostic (const zero_state_machine &sm, tree arg)
  : m_sm (sm), m_arg(arg)
  {}

  int get_controlling_option () const final override {
    return 0;
  }

  bool subclass_equal_p (const pending_diagnostic &base_other) const override
  {
    const zero_diagnostic &other = (const zero_diagnostic &)base_other;
m_set_to_zero;
    return same_tree_p (m_arg, other.m_arg);
  }

  /* Vfunc for emitting the diagnostic.  The rich_location will have been
     populated with a diagnostic_path.
     Return true if a diagnostic is actually emitted.  */
  bool emit (rich_location *rich_loc) final override {
    diagnostic_metadata m;
    m.add_cwe(369);
    return warning_meta (rich_loc, m, get_controlling_option (),
			   "division by zero");
  }

  const char *get_kind () const final override
  {
    return "zero";
  }

  label_text describe_state_change (const evdesc::state_change &ev) final override
  {
    if (ev.m_old_state == m_sm.get_start_state () 
        && ev.m_new_state == m_sm.m_zero)
      {
        m_set_to_zero = ev.m_event_id;
        return ev.formatted_print ("%qE is set to 0 here.",
					 ev.m_expr);
      }
    return label_text ();
  }

  label_text describe_final_event (const evdesc::final_event &ev) final override
  {
    if (m_set_to_zero.known_p ())
      return ev.formatted_print("%qE is the divisior here and was set to zero at %@", ev.m_expr, &m_set_to_zero);
    return label_text ();
  }

private:
  const zero_state_machine &m_sm;
  diagnostic_event_id_t m_set_to_zero;
  tree m_arg;
};

zero_state_machine::zero_state_machine (logger *logger)
    : state_machine ("zero", logger)
{
  m_unknown = add_state ("unknown");
  m_maybe_zero = add_state ("maybezero");  
  m_zero = add_state ("m_zero");
  m_not_zero = add_state ("notzero");
}

bool
zero_state_machine::on_stmt (sm_context *sm_ctxt, const supernode *node,
                             const gimple *stmt) const
{
  if (const gassign *assign_stmt = dyn_cast<const gassign *> (stmt))
    {
      tree lhs = gimple_assign_lhs (assign_stmt);

      tree_code tc = gimple_assign_rhs_code (assign_stmt);
      switch (tc)
        {
        case INTEGER_CST:
          {
            tree rhs = gimple_assign_rhs1 (assign_stmt);
            if (zerop (rhs)) {
              sm_ctxt->on_transition (node, stmt, lhs, m_start, m_zero);
            }
            else
              sm_ctxt->on_transition (node, stmt, lhs, m_start, m_unknown); 
            break;
          }
        case TRUNC_DIV_EXPR:
        case CEIL_DIV_EXPR:
        case FLOOR_DIV_EXPR:
        case ROUND_DIV_EXPR:
          {
            tree divisor = gimple_assign_rhs2 (assign_stmt);
            if (sm_ctxt->get_state (stmt, divisor) == m_zero)
              {
                tree arg = sm_ctxt->get_diagnostic_tree (divisor);
                sm_ctxt->warn(node, stmt, arg, new zero_diagnostic(*this, arg));
                sm_ctxt->on_transition (node, stmt, lhs, m_zero, m_unknown);
              }
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
  // state_t current = sm_ctxt->get_state (stmt, lhs);
  // if (current == m_unknown)
  //   return;

  // tree cst = rhs->maybe_get_constant ();
  // if (!cst)
  //   {
  //     // If rhs is not known, just go back to start
  //     sm_ctxt->on_transition (node, stmt, lhs, current, m_unknown);
  //     return;
  //   }

  // bool isZero = integer_zerop (cst);
  // bool isNonZero = integer_nonzerop (cst);
  // if ((op == NE_EXPR && isZero) || (op == EQ_EXPR && isNonZero))
  //   {
  //     log ("got 'ARG != 0' or 'ARG == c' match");
  //     sm_ctxt->on_transition (node, stmt, lhs, m_start, m_not_zero);
  //   }
  // else if (op == EQ_EXPR && isZero)
  //   {
  //     log ("got 'ARG == 0' match");
  //     sm_ctxt->on_transition (node, stmt, lhs, m_start, m_zero);
  //   }
}

void
zero_state_machine::on_phi (sm_context *sm_ctxt ATTRIBUTE_UNUSED,
                            const supernode *node ATTRIBUTE_UNUSED,
                            const gphi *phi ATTRIBUTE_UNUSED, 
                            tree rhs ATTRIBUTE_UNUSED) const
{
}

bool
zero_state_machine::can_purge_p (state_t s) const
{
  return false;
}
} // anonymous namespace

state_machine *
make_zero_state_machine (logger *logger)
{
  return new zero_state_machine (logger);
}

} // namespace ana

#endif /* #if ENABLE_ANALYZER */
