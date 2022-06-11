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
    if (const constant_svalue *cst_sval = sval->dyn_cast_constant_svalue ())
      cst_sval->dump(false);

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

  bool can_purge_p (state_t s) const final override 
  {
    return false;
  }

  bool
  reset_when_passed_to_unknown_fn_p (state_t s ATTRIBUTE_UNUSED,
                                     bool is_mutable) const final override
  {
    return is_mutable;
  }

  state_t m_stop;
  state_t m_zero;
};

class zero_diagnostic : public pending_diagnostic
{
public:
  zero_diagnostic(const zero_state_machine &sm, tree arg) : m_sm(sm), m_arg(arg) {}

  int get_controlling_option () const final override 
  {
    return 0;
  }

  bool emit (rich_location *rich_loc) final override 
  {
    diagnostic_metadata m;
    m.add_cwe(369);
    return warning_meta(rich_loc, m, get_controlling_option (), "div by zero");
  }

    label_text describe_state_change (const evdesc::state_change &change)
    final override
  {
    if (change.m_new_state == m_sm.m_zero)
      {
	return label_text::borrow ("opened here");
      }
    return label_text ();
  }

  label_text describe_final_event (const evdesc::final_event &ev) final override
  {
	  return ev.formatted_print ("leaks here");
  }

  const char *get_kind () const final override 
  {
    return "zero_diag";
  }

  bool subclass_equal_p (const pending_diagnostic &other) const final override 
  {
    return same_tree_p (m_arg, ((const zero_diagnostic &)other).m_arg);
  }

private:
  const zero_state_machine &m_sm;
  tree m_arg;
};

zero_state_machine::zero_state_machine (logger *logger)
    : state_machine ("zero", logger)
{
  m_stop = add_state ("stop");
  m_zero = add_state ("zero");
}

bool
zero_state_machine::on_stmt (sm_context *sm_ctxt, const supernode *node,
                             const gimple *stmt) const
{
  // debug_gimple_stmt ((gimple *) stmt);
  if (const gassign *assign_stmt = dyn_cast<const gassign *> (stmt))
    {
      tree lhs = gimple_assign_lhs (assign_stmt);

      tree_code tc = gimple_assign_rhs_code (assign_stmt);
      switch (tc)
        {
        case INTEGER_CST:
          {
            tree rhs = gimple_assign_rhs1 (assign_stmt);
            if (zerop (rhs)) 
              {
                 sm_ctxt->on_transition (node, stmt, lhs, m_start, m_zero);
              }
            else 
              {
                sm_ctxt->on_transition (node, stmt, lhs, m_start, m_stop); 
              }
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
                tree diag = sm_ctxt->get_diagnostic_tree (divisor);
                // sm_ctxt->warn (node, stmt, diag, new zero_diagnostic (*this, diag));
                sm_ctxt->on_transition (node, stmt, divisor, m_zero, m_stop);
              }
            break;
          }
        case TRUNC_MOD_EXPR:
        case CEIL_MOD_EXPR:
        case FLOOR_MOD_EXPR:
        case ROUND_MOD_EXPR:
          {
            tree divisor = gimple_assign_rhs2 (assign_stmt);
            if (sm_ctxt->get_state (stmt, divisor) == m_zero)
              {
                tree diag = sm_ctxt->get_diagnostic_tree (divisor);
                // sm_ctxt->warn (node, stmt, diag, new zero_diagnostic (*this, diag));
                sm_ctxt->on_transition (node, stmt, divisor, m_zero, m_stop);
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
  // bool isZero = integer_zerop (cst);
  // bool isNonZero = integer_nonzerop (cst);
  // if ((op == NE_EXPR && isZero) || (op == EQ_EXPR && isNonZero))
  //   {
  //     log ("got 'ARG != 0' or 'ARG == c' match");
  //     sm_ctxt->on_transition (node, stmt, lhs, m_start, m_stop);
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
} // anonymous namespace

state_machine *
make_zero_state_machine (logger *logger)
{
  return new zero_state_machine (logger);
}

} // namespace ana

#endif /* #if ENABLE_ANALYZER */
