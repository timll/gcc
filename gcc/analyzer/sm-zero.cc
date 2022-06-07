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
              inform(UNKNOWN_LOCATION, "Divide by zero %qE", divisor);
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
  // bool could_be_zero = false;
  // bool could_be_non_zero = false;
  // for (int i = 0; i < phi->nargs; i++) 
  //   {
  //     tree arg = gimple_phi_arg(phi, i)->def;
  //     state_t state = sm_ctxt->get_state(phi, arg);
  //     if (state == m_zero)
  //       could_be_zero = true;
  //     else if (state == m_not_zero)
  //       could_be_non_zero = true;

  //     if (could_be_zero && could_be_non_zero)
  //       break;
  //   }
  
  // if (could_be_zero && could_be_non_zero)
  //   sm_ctxt->on_transition(node, phi, lhs, m_start, m_maybe_zero);
  // else if (could_be_zero)
  //   sm_ctxt->on_transition(node, phi, lhs, m_start, m_zero);
  // else if (could_be_non_zero)
  //   sm_ctxt->on_transition(node, phi, lhs, m_start, m_not_zero);
  // state_t state = sm_ctxt->get_state(phi, rhs);
  // if (state == m_not_zero)
    // sm_ctxt->on_transition(node, phi, lhs, m_start, state);
}

bool
zero_state_machine::can_purge_p (state_t s) const
{
  return true;
}
} // anonymous namespace

state_machine *
make_zero_state_machine (logger *logger)
{
  return new zero_state_machine (logger);
}

} // namespace ana

#endif /* #if ENABLE_ANALYZER */
