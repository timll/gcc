// /* An state machine, for tracking the value of integers.

//    Copyright (C) 2019-2022 Free Software Foundation, Inc.
//    Contributed by Tim Lange <mail@tim-lange.me>.

// This file is part of GCC.

// GCC is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.

// GCC is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GCC; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.  */

// #if ENABLE_ANALYZER

// namespace ana
// {

// namespace
// {

// // XXX: zero state machine
// class zero_state_machine : public state_machine
// {
// public:
//   zero_state_machine(logger *logger);

//   bool
//   inherited_state_p () const final override
//   {
//     return false;
//   }

//   state_machine::state_t
//   get_default_state (const svalue *sval) const final override
//   {
//     if (tree cst = sval->maybe_get_constant ())
//       {
//         if (zerop (cst))
//           return m_zero;
//       }
//     return m_start;
//   }

//   bool on_stmt (sm_context *sm_ctxt, const supernode *node,
//                 const gimple *stmt) const final override;

//   virtual void
//   on_phi (sm_context *sm_ctxt ATTRIBUTE_UNUSED,
//           const supernode *node ATTRIBUTE_UNUSED,
//           const gphi *phi ATTRIBUTE_UNUSED, tree rhs ATTRIBUTE_UNUSED) const
//   {
//   }

//   void on_condition (sm_context *sm_ctxt ATTRIBUTE_UNUSED,
//                      const supernode *node ATTRIBUTE_UNUSED,
//                      const gimple *stmt ATTRIBUTE_UNUSED,
//                      const svalue *lhs ATTRIBUTE_UNUSED,
//                      enum tree_code op ATTRIBUTE_UNUSED,
//                      const svalue *rhs ATTRIBUTE_UNUSED) const final override;

//   bool can_purge_p (state_t s) const final override;

//   bool
//   reset_when_passed_to_unknown_fn_p (state_t s ATTRIBUTE_UNUSED,
//                                      bool is_mutable) const final override
//   {
//     return is_mutable;
//   }

// protected:
//   state_t m_unknown;
//   state_t m_maybe_zero;
//   state_t m_zero;
//   state_t m_not_zero;
// };

// zero_state_machine::zero_state_machine (logger *logger)
//     : state_machine ("zero", logger)
// {
//   m_unknown = add_state ("unknown");
//   m_maybe_zero = add_state ("maybezero");  
//   m_zero = add_state ("zero");
//   m_not_zero = add_state ("notzero");
// }

// bool
// zero_state_machine::on_stmt (sm_context *sm_ctxt, const supernode *node,
//                              const gimple *stmt) const
// {
//   if (const gassign *assign_stmt = dyn_cast<const gassign *> (stmt))
//     {
//       tree lhs = gimple_assign_lhs (assign_stmt);

//       tree rhs = gimple_assign_rhs1 (assign_stmt);
//       state_t current = sm_ctxt->get_state (node, lhs);

//       if (zerop (rhs))
//         sm_ctxt->on_transition (node, stmt, lhs, current, m_zero);
//       else
//         sm_ctxt->on_transition (node, stmt, lhs, current, m_not_zero);
//     }
// }

// void
// zero_state_machine::on_condition (sm_context *sm_ctxt, const supernode *node,
//                                   const gimple *stmt, const svalue *lhs,
//                                   enum tree_code op, const svalue *rhs) const
// {
//   state_t current = sm_ctxt->get_state (node, lhs);

//   tree cst = rhs->maybe_get_constant ();
//   if (!cst)
//     {
//       sm_ctxt->on_transition (node, stmt, lhs, current, m_unknown);
//       return;
//     }

//   bool isZero = zerop (cst);
//   if ((op == NE_EXPR && isZero) || (op == EQ_EXPR && !isZero))
//     {
//       log ("got 'ARG != 0' or 'ARG == c' match");
//       sm_ctxt->on_transition (node, stmt, lhs, current, m_not_zero);
//     }
//   else if (op == EQ_EXPR && isZero)
//     {
//       log ("got 'ARG == 0' match");
//       sm_ctxt->on_transition (node, stmt, lhs, current, m_zero);
//     }
// }

// bool
// zero_state_machine::can_purge_p (state_t s) const
// {
//   return false;
// }

// } // anonymous namespace

// state_machine *
// make_zero_state_machine (logger *logger)
// {
//   return new zero_state_machine (logger);
// }

// } // namespace ana

// #endif /* #if ENABLE_ANALYZER */
