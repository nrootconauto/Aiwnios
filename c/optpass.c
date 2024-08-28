#include "aiwn_hash.h"
#include "aiwn_lexparser.h"
#include "aiwn_mem.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

static int64_t IsConst(CRPN *rpn) {
  switch (rpn->type) {
  case IC_CHR:
  case IC_F64:
  case IC_I64:
    return 1;
  }
  return 0;
}

static int64_t PtrWidthOfRPN(CRPN *rpn) {
  int64_t r = 1;
  if (rpn->ic_dim) {
    if (rpn->ic_dim->next)
      r *= rpn->ic_dim->next->total_cnt;
    r *= rpn->ic_class->sz;
  } else
    r *= rpn->ic_class[-1].sz;

  return r;
}

static void FixFunArgs(CCmpCtrl *cctrl, CRPN *rpn) {
  int64_t cnt, cnt2, vargc;
  CRPN *rpn2 = rpn->base.next, *ic, *last;
  CHashFun *ic_fun;
  CMemberLst *arg, *args_lst_rev;
  for (cnt = 0; cnt != rpn->length; cnt++)
    rpn2 = ICFwd(rpn2);
  AssignRawTypeToNode(cctrl, rpn2);
  ic_fun = rpn2->ic_fun;
  rpn2 = rpn->base.next;
  arg = ic_fun->base.members_lst;
  cnt2 = ic_fun->argc;
  if (ic_fun->base.flags & CLSF_VARGS)
    cnt2 -= 2;
  for (cnt = 0; cnt != rpn->length; cnt++) {
    if (cnt >= cnt2)
      break;
    rpn2 = ICArgN(rpn, rpn->length - cnt - 1); // REVERSE polish notation
    if (rpn2->type == IC_NOP) {
      if (arg->member_class->raw_type == RT_F64) {
        rpn2->type = IC_F64;
        rpn2->flt = ((double *)&arg->dft_val)[0];
        AssignRawTypeToNode(cctrl, rpn2);
      } else {
        rpn2->type = IC_I64;
        rpn2->integer = arg->dft_val;
        AssignRawTypeToNode(cctrl, rpn2);
      }
    } else {
      // If we are passing an F64 to an I64 argument,be sure to convert the F64
      // to a I64
      if ((arg->member_class->raw_type == RT_F64) ^ // Check for difference
          (AssignRawTypeToNode(cctrl, rpn2) == RT_F64)) {
        if (arg->member_class->raw_type == RT_F64) {
          ic = A_CALLOC(sizeof(CRPN), cctrl->hc);
          ic->type = IC_TO_F64;
          QueIns(ic, rpn2->base.last);
          AssignRawTypeToNode(cctrl, ic);
        } else {
          ic = A_CALLOC(sizeof(CRPN), cctrl->hc);
          ic->type = IC_TO_I64;
          AssignRawTypeToNode(cctrl, ic);
          QueIns(ic, rpn2->base.last);
        }
      }
    }
    arg = arg->next;
  }
  for (; cnt < cnt2; cnt++) {
    ic = A_CALLOC(sizeof(CRPN), cctrl->hc);
    rpn->length++;
    if (arg->member_class->raw_type == RT_F64) {
      ic->type = IC_F64;
      ic->flt = ((double *)&arg->dft_val)[0];
      AssignRawTypeToNode(cctrl, ic);
    } else {
      ic->type = IC_I64;
      ic->integer = arg->dft_val;
      AssignRawTypeToNode(cctrl, ic);
    }
    QueIns(ic, rpn);
    arg = arg->next;
  }
  if (ic_fun->base.flags & CLSF_VARGS) {
    rpn2 = ICArgN(rpn, rpn->length - cnt2 - 1 + 1);
    last = rpn;
    vargc = rpn->length - cnt2;
    // Pass argv
    ic = A_CALLOC(sizeof(CRPN), cctrl->hc);
    ic->type = __IC_VARGS;
    ic->length = vargc;
    ic->raw_type = RT_I64i;
    QueIns(ic, last);
    // Pass argc
    ic = A_CALLOC(sizeof(CRPN), cctrl->hc);
    ic->type = IC_I64;
    ic->integer = vargc;
    AssignRawTypeToNode(cctrl, ic);
    QueIns(ic, ICFwd(last->base.next)->base.last);
    rpn->length = cnt2 + 2;
  }
}

// This one takes into acount the default arguments of function calls(either)
// ommited or put with IC_NOP
//
// Lone function's without arguments will be turned into calls
//
void OptPassFixFunArgs(CCmpCtrl *cctrl) {
  CRPN *rpn, *rpn2, *last;
  for (rpn = cctrl->code_ctrl->ir_code->next; rpn != cctrl->code_ctrl->ir_code;
       rpn = rpn->base.next) {
    switch (rpn->type) {
      break;
    case IC_CALL:
      FixFunArgs(cctrl, rpn);
    }
  }
}

// This one makes takes into account the array dimensions of accessed arrays
// and will multiply them by the array dim
//
// THIS WILL ALSO REMOVE "derefencing function pointers"
//
void OptPassExpandPtrs(CCmpCtrl *cctrl) {
  CRPN *rpn, *new, *lit, *new2, *new3, *a, *b, **list;
  CArrayDim *dim;
  int64_t cnt = 0, cnt2, total_off, raw_type;
  for (rpn = cctrl->code_ctrl->ir_code->next; rpn != cctrl->code_ctrl->ir_code;
       rpn = rpn->base.next) {
    switch (rpn->type) {
    case IC_SUB:
    case IC_ADD:
    case IC_ARRAY_ACC:
    case IC_ARROW:
    case IC_DOT:
    case IC_STATIC:
    case IC_LOCAL:
    case IC_GLOBAL:
    case IC_SUB_EQ:
    case IC_ADD_EQ:
    case IC_DEREF:
      cnt++;
    }
  }
  list = A_CALLOC(sizeof(CRPN *) * cnt, cctrl->hc);
  cnt = 0;
  for (rpn = cctrl->code_ctrl->ir_code->next; rpn != cctrl->code_ctrl->ir_code;
       rpn = rpn->base.next) {
    switch (rpn->type) {
    case IC_SUB:
    case IC_ADD:
    case IC_ARRAY_ACC:
    case IC_ARROW:
    case IC_DOT:
    case IC_LOCAL:
    case IC_STATIC:
    case IC_GLOBAL:
    case IC_SUB_EQ:
    case IC_ADD_EQ:
    case IC_DEREF:
      list[cnt++] = rpn;
    }
  }
  for (cnt2 = 0; cnt2 != cnt; cnt2++) {
    rpn = list[cnt2];
    switch (rpn->type) {
      break;
    case IC_DEREF:
      b = rpn->base.next;
      if (b->raw_type == RT_FUNC)
        ICFree(rpn);
      break;
    case IC_SUB_EQ:
    case IC_SUB:
      b = rpn->base.next;
      a = ICFwd(b);
      if (a->ic_class->ptr_star_cnt || a->ic_dim) {
        if (b->ic_class->ptr_star_cnt || b->ic_dim) {
          new = A_CALLOC(sizeof(CRPN), cctrl->hc);
          new->type = IC_DIV;
          QueIns(new, rpn->base.last);
          lit = A_CALLOC(sizeof(CRPN), cctrl->hc);
          lit->type = IC_I64;
          lit->integer = PtrWidthOfRPN(a);
          QueIns(lit, new);
          AssignRawTypeToNode(cctrl, new);
        } else {
          new = A_CALLOC(sizeof(CRPN), cctrl->hc);
          new->type = IC_MUL;
          QueIns(new, b->base.last);
          lit = A_CALLOC(sizeof(CRPN), cctrl->hc);
          lit->type = IC_I64;
          lit->integer = PtrWidthOfRPN(a);
          QueIns(lit, new);
          AssignRawTypeToNode(cctrl, new);
        }
      }
      break;
    case IC_ADD_EQ:
    case IC_ADD:
      b = rpn->base.next;
      a = ICFwd(b);
      if (a->ic_class->ptr_star_cnt || a->ic_dim) {
        new = A_CALLOC(sizeof(CRPN), cctrl->hc);
        new->type = IC_MUL;
        QueIns(new, b->base.last);
        lit = A_CALLOC(sizeof(CRPN), cctrl->hc);
        lit->type = IC_I64;
        lit->integer = PtrWidthOfRPN(a);
        QueIns(lit, new);
        AssignRawTypeToNode(cctrl, new);
      } else if ((b->ic_class->ptr_star_cnt || b->ic_dim) &&
                 rpn->type != IC_ADD_EQ) {
        new = A_CALLOC(sizeof(CRPN), cctrl->hc);
        new->type = IC_MUL;
        QueIns(new, a->base.last);
        lit = A_CALLOC(sizeof(CRPN), cctrl->hc);
        lit->type = IC_I64;
        lit->integer = PtrWidthOfRPN(b);
        QueIns(lit, new);
        AssignRawTypeToNode(cctrl, new);
      }
      break;
    case IC_ARRAY_ACC:
      b = rpn->base.next;
      a = ICFwd(b);
      dim = rpn->ic_dim;
      if (!dim)
        goto aderef;
      if (dim->next) {
        new = A_CALLOC(sizeof(CRPN), cctrl->hc);
        new->type = IC_MUL;
        new->raw_type = RT_I64i;
        new->ic_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
        QueIns(new, rpn);
        lit = A_CALLOC(sizeof(CRPN), cctrl->hc);
        lit->type = IC_I64;
        lit->raw_type = new->raw_type;
        lit->ic_class = new->ic_class;
        lit->integer = PtrWidthOfRPN(a);
        QueIns(lit, new);
        AssignRawTypeToNode(cctrl, new);
        dim = dim->next;
        rpn->type = IC_ADD;
      } else {
      aderef:
        new2 = A_CALLOC(sizeof(CRPN), cctrl->hc);
        new2->type = IC_ADD;
        QueIns(new2, rpn);
        new = A_CALLOC(sizeof(CRPN), cctrl->hc);
        new->type = IC_MUL;
        QueIns(new, new2);
        lit = A_CALLOC(sizeof(CRPN), cctrl->hc);
        lit->type = IC_I64;
        lit->integer = PtrWidthOfRPN(a);
        QueIns(lit, new);
        rpn->type = IC_DEREF;
        AssignRawTypeToNode(cctrl, new2);
      }
      break;
    case IC_ARROW:
    addrof_arr:
      // If we already have a addrof,no need to do it twice
      if (rpn->base.last != cctrl->code_ctrl->ir_code) {
        b = rpn->base.last;
        if (b->type == IC_ADDR_OF) {
          break;
        }
      }
      if (rpn->local_mem->dim.next) {
        new = A_CALLOC(sizeof(CRPN), cctrl->hc);
        new->type = IC_ADDR_OF;
        QueIns(new, rpn->base.last);
        AssignRawTypeToNode(cctrl, new);
      }
      break;
    case IC_DOT:
      goto addrof_arr;
      break;
    case IC_STATIC:
      goto addrof_arr;
    case IC_LOCAL:
      goto addrof_arr;
      break;
    case IC_GLOBAL:
      if (rpn->global_var->base.type & HTT_GLBL_VAR)
        if (rpn->global_var->dim.next) {
          //
          // Do not get address of thing if we are already getting the address
          // of it
          //
          b = rpn->base.last;
          if (b->type != IC_ADDR_OF) {
            new = A_CALLOC(sizeof(CRPN), cctrl->hc);
            new->type = IC_ADDR_OF;
            QueIns(new, rpn->base.last);
            AssignRawTypeToNode(cctrl, new);
          }
        }
      break;
    default:
      dim = NULL;
    }
  }
  for (rpn = cctrl->code_ctrl->ir_code->next; rpn != cctrl->code_ctrl->ir_code;
       rpn = rpn->base.next) {
    switch (rpn->type) {
      break;
    case IC_DOT:
      raw_type = rpn->raw_type;
      a = rpn->base.next;
      total_off = rpn->local_mem->off;
      for (; a->type == IC_DOT; a = b) {
        b = a->base.next;
        raw_type = a->raw_type;
        total_off += a->local_mem->off;
        ICFree(a);
      }
      new = A_CALLOC(sizeof(CRPN), cctrl->hc);
      new->type = IC_ADDR_OF;
      new->raw_type = RT_PTR;
      QueIns(new, a->base.last);
      new2 = A_CALLOC(sizeof(CRPN), cctrl->hc);
      new2->type = IC_ADD;
      new2->raw_type = RT_PTR;
      QueIns(new2, rpn);
      new3 = A_CALLOC(sizeof(CRPN), cctrl->hc);
      new3->type = IC_I64;
      new3->integer = total_off;
      new3->raw_type = RT_I64i;
      QueIns(new3, new2);
      rpn->type = IC_DEREF;
      rpn->raw_type = raw_type;
      break;
    case IC_ARROW:
      new = A_CALLOC(sizeof(CRPN), cctrl->hc);
      new->type = IC_DEREF;
      new->raw_type = rpn->raw_type;
      new->ic_fun = rpn->ic_fun;
      new->ic_dim = rpn->ic_dim;
      new2 = A_CALLOC(sizeof(CRPN), cctrl->hc);
      new2->type = IC_I64;
      new2->integer = rpn->local_mem->off;
      new2->raw_type = RT_I64i;
      QueIns(new2, rpn);
      QueIns(new, rpn->base.last);
      rpn->type = IC_ADD;
      rpn->raw_type = RT_PTR;
    }
  }
  // Remove addresses of dereferences
  for (rpn = cctrl->code_ctrl->ir_code->next;
       rpn != cctrl->code_ctrl->ir_code;) {
    a = rpn;
    b = rpn->base.next;
    if (a->type == IC_ADDR_OF) {
      if (b->type == IC_DEREF) {
        rpn = b->base.next;
        ICFree(a);
        ICFree(b);
        continue;
      }
    }
    rpn = a->base.next;
  }
  A_FREE(list);
}

#define COMM_ACCUMULATE_FUN(name, __type, oper, is_i64, only_rightside)        \
  static __type name(CCmpCtrl *cctrl, CRPN *rpn, int64_t operator, __type cur, \
                     int64_t first) {                                          \
    if (rpn->type != operator)                                                 \
      return cur;                                                              \
    CRPN *b = rpn->base.next, *a;                                              \
    a = ICFwd(b);                                                              \
    if (IsConst(a) && !first && !only_rightside) {                             \
      if (a->type != IC_F64)                                                   \
        cur oper a->integer;                                                   \
      else                                                                     \
        cur oper a->flt;                                                       \
      ICFree(rpn);                                                             \
      ICFree(a);                                                               \
      return name(cctrl, b, operator, cur, 0);                                 \
    }                                                                          \
    if (IsConst(b) && !first) {                                                \
      if (b->type != IC_F64)                                                   \
        cur oper b->integer;                                                   \
      else                                                                     \
        cur oper b->flt;                                                       \
      ICFree(rpn);                                                             \
      ICFree(b);                                                               \
      return name(cctrl, a, operator, cur, 0);                                 \
    }                                                                          \
    if (first) {                                                               \
      if (IsConst(a) && !only_rightside) {                                     \
        if (a->type != IC_F64)                                                 \
          cur oper a->integer;                                                 \
        else                                                                   \
          cur oper a->flt;                                                     \
        cur = name(cctrl, b, operator, cur, 0);                                \
        if (!is_i64) {                                                         \
          a->type = IC_F64;                                                    \
          a->flt = cur;                                                        \
        } else {                                                               \
          a->type = IC_I64;                                                    \
          a->integer = cur;                                                    \
        }                                                                      \
      } else if (IsConst(b)) {                                                 \
        if (b->type != IC_F64)                                                 \
          cur oper b->integer;                                                 \
        else                                                                   \
          cur oper b->flt;                                                     \
        cur = name(cctrl, a, operator, cur, 0);                                \
        if (!is_i64) {                                                         \
          b->type = IC_F64;                                                    \
          b->flt = cur;                                                        \
        } else {                                                               \
          b->type = IC_I64;                                                    \
          b->integer = cur;                                                    \
        }                                                                      \
      }                                                                        \
      return 0;                                                                \
    }                                                                          \
    return cur;                                                                \
  }

COMM_ACCUMULATE_FUN(SumComConstsF64, double, +=, 0, 0);
COMM_ACCUMULATE_FUN(SumComConstsI64, int64_t, +=, 1, 0);
COMM_ACCUMULATE_FUN(MulComConstsF64, double, *=, 0, 0);
COMM_ACCUMULATE_FUN(MulComConstsI64, int64_t, *=, 1, 0);
COMM_ACCUMULATE_FUN(SumComConstsF642, double, +=, 0, 1);
COMM_ACCUMULATE_FUN(SumComConstsI642, int64_t, +=, 1, 1);
COMM_ACCUMULATE_FUN(MulComConstsF642, double, *=, 0, 1);
COMM_ACCUMULATE_FUN(MulComConstsI642, int64_t, *=, 1, 1);

void OptPassMergeCommunitives(CCmpCtrl *cctrl) {
  CRPN *rpn;
  for (rpn = cctrl->code_ctrl->ir_code->next; rpn != cctrl->code_ctrl->ir_code;
       rpn = rpn->base.next) {
    switch (rpn->type) {
      break;
    case IC_SUB:
      if (rpn->raw_type == RT_F64)
        SumComConstsF642(cctrl, rpn, rpn->type, 0, 1);
      else
        SumComConstsI642(cctrl, rpn, rpn->type, 0, 1);
      break;
    case IC_ADD:
      if (rpn->raw_type == RT_F64)
        SumComConstsF64(cctrl, rpn, rpn->type, 0, 1);
      else
        SumComConstsI64(cctrl, rpn, rpn->type, 0, 1);
      break;
    case IC_MUL:
      if (rpn->raw_type == RT_F64)
        MulComConstsF64(cctrl, rpn, rpn->type, 1, 1);
      else
        MulComConstsI64(cctrl, rpn, rpn->type, 1, 1);
      break;
    case IC_DIV:
      if (rpn->raw_type == RT_F64)
        MulComConstsF642(cctrl, rpn, rpn->type, 1, 1);
      else
        MulComConstsI642(cctrl, rpn, rpn->type, 1, 1);
    }
  }
}

void OptPassConstFold(CCmpCtrl *cctrl) {
#define SET_AB                                                                 \
  if (next->type == IC_F64)                                                    \
    b = next->flt;                                                             \
  else                                                                         \
    b = next->integer;                                                         \
  next = ICFwd(next);                                                          \
  if (next->type == IC_F64)                                                    \
    a = next->flt;                                                             \
  else                                                                         \
    a = next->integer;
#define SET_A2B2                                                               \
  if (next->type == IC_F64)                                                    \
    b2 = next->flt;                                                            \
  else                                                                         \
    b2 = next->integer;                                                        \
  next = ICFwd(next);                                                          \
  if (next->type == IC_F64)                                                    \
    a2 = next->flt;                                                            \
  else                                                                         \
    a2 = next->integer;
  CRPN *rpn, *next, next2;
  double a, b;
  int64_t i, a2, b2, changed;
loop:
  changed = 0;
  for (rpn = cctrl->code_ctrl->ir_code->next;
       cctrl->code_ctrl->ir_code != rpn;) {
    switch (rpn->type) {
      break;
    case IC_TO_BOOL:
      next = rpn->base.next;
      if (IsConst(next)) {
        if (next->type == IC_F64)
          a2 = next->flt;
        else
          a2 = next->integer;
        rpn->integer = !!a2;
        rpn->raw_type = RT_I64i;
        rpn->type = IC_I64;
        ICFree(next);
        changed = 1;
      } else
        goto next1;
      break;
    case IC_GOTO:
    skip:
      rpn = ICFwd(rpn);
      break;
    case IC_LABEL:
      goto next1;
      break;
    next1:
      rpn = rpn->base.next;
      break;
    case IC_STATIC:
    case IC_LOCAL:
      goto next1;
      break;
    case IC_GLOBAL:
      goto next1;
      break;
    case IC_NOP:
      goto next1;
      break;
    case IC_PAREN:
      abort();
      break;
    case IC_TYPECAST:
      if (IsConst(next = rpn->base.next)) {
        if (rpn->raw_type == RT_F64) {
          switch (rpn->type) {
          default:
            rpn->flt = *(double *)&next->integer;
            break;
          case IC_F64:
            rpn->flt = next->flt;
          }
          rpn->type = IC_F64;
        } else {
          switch (rpn->type) {
          case IC_F64:
            rpn->integer = *(int64_t *)&next->flt;
            break;
          default:
            rpn->integer = next->integer;
          }
          rpn->type = IC_I64;
        }
        changed = 1;
        ICFree(next);
        break;
      } else
        goto next1;
    case IC_NEG:
      if (IsConst(next = rpn->base.next)) {
        rpn->raw_type = next->raw_type;
        rpn->ic_class = next->ic_class;
        if (next->type == IC_F64) {
          rpn->type = IC_F64;
          rpn->flt = -next->flt;
          ICFree(next);
          changed = 1;
        } else {
          rpn->type = next->type;
          rpn->integer = -next->integer;
          ICFree(next);
          changed = 1;
        }
      }
      rpn = rpn->base.next;
      break;
    case IC_POS:
      if (IsConst(next = rpn->base.next)) {
        rpn->raw_type = next->raw_type;
        rpn->ic_class = next->ic_class;
        if (next->type == IC_F64) {
          rpn->type = IC_F64;
          rpn->flt = +next->flt;
          ICFree(next);
          changed = 1;
        } else {
          rpn->type = next->type;
          rpn->integer = +next->integer;
          ICFree(next);
          changed = 1;
        }
        rpn = rpn->base.next;
      } else
        goto next1;
      break;
    case IC_NAME:
      goto next1;
      break;
    case IC_STR:
      goto next1;
      break;
    case IC_CHR:
      goto next1;
      break;
    case IC_POW:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        SET_AB;
        rpn->type = IC_F64;
        rpn->flt = pow(a, b);
      } else
        goto next1;
    binop_next:
      ICFree(ICFwd(rpn->base.next));
      ICFree(rpn->base.next);
      changed = 1;
      rpn = rpn->base.next;
      break;
    case IC_ADD:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = a + b;
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 + b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_EQ:
      goto next1;
      break;
    case IC_SUB:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = a - b;
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 - b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_DIV:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          if (b)
            rpn->flt = a / b;
          else
            rpn->integer = 0; //???
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          if (b2)
            rpn->integer = a2 / b2;
          else
            rpn->integer = 0; //???
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_MUL:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = a * b;
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 * b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_DEREF:
      goto next1;
      break;
    case IC_AND:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = ((int64_t *)&a)[0] & ((int64_t *)&b)[0];
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 & b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_ADDR_OF:
      goto next1;
      break;
    case IC_XOR:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = ((int64_t *)&a)[0] ^ ((int64_t *)&b)[0];
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 ^ b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_MOD:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          if (!b)
            goto next1;
          rpn->type = IC_F64;
          rpn->flt = fmod(a, b);
        } else {
          SET_A2B2;
          if (!b2)
            goto next1;
          rpn->type = IC_I64;
          rpn->integer = a2 % b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_OR:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = ((int64_t *)&a)[0] | ((int64_t *)&b)[0];
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 | b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_LT:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = a < b;
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 < b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_GT:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = a > b;
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 > b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_LNOT:
      if (IsConst(next = rpn->base.next)) {
        rpn->type = IC_I64;
        if (next->type == IC_F64)
          rpn->integer = !next->flt;
        else
          rpn->integer = !next->integer;
        ICFree(next);
        changed = 1;
      }
      goto next1;
      break;
    case IC_BNOT:
      if (IsConst(next = rpn->base.next)) {
        rpn->type = IC_I64;
        if (next->type == IC_F64)
          rpn->integer = ~next->integer; // Type punning
        else
          rpn->integer = ~next->integer;
        ICFree(next);
        changed = 1;
      }
      goto next1;
      break;
    case IC_POST_INC:
      goto next1;
      break;
    case IC_POST_DEC:
      goto next1;
      break;
    case IC_PRE_INC:
      goto next1;
      break;
    case IC_PRE_DEC:
      goto next1;
      break;
    case IC_AND_AND:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (next->type == IC_F64)
          b2 = !!next->flt;
        else
          b2 = next->integer;
        next = ICFwd(next);
        if (next->type == IC_F64)
          a2 = !!next->flt;
        else
          a2 = next->integer;
        rpn->type = IC_I64;
        rpn->integer = a2 && b2;
        goto binop_next;
      } else
        goto next1;
      break;
    case IC_OR_OR:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (next->type == IC_F64)
          b2 = !!next->flt;
        else
          b2 = next->integer;
        next = ICFwd(next);
        if (next->type == IC_F64)
          a2 = !!next->flt;
        else
          a2 = next->integer;
        rpn->type = IC_I64;
        rpn->integer = a2 || b2;
        goto binop_next;
      } else
        goto next1;
      break;
    case IC_XOR_XOR:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (next->type == IC_F64)
          b2 = !!next->flt;
        else
          b2 = next->integer;
        next = ICFwd(next);
        if (next->type == IC_F64)
          a2 = !!next->flt;
        else
          a2 = next->integer;
        rpn->type = IC_I64;
        rpn->integer = !!a2 ^ !!b2;
        goto binop_next;
      } else
        goto next1;
      break;
    case IC_EQ_EQ:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = a == b;
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 == b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_NE:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = a != b;
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 != b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_LE:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = a <= b;
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 <= b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_GE:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->flt = a >= b;
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 >= b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_LSH:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->integer = ((int64_t *)&a)[0]
                         << ((int64_t *)&b)[0]; // Type punning
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 << b2;
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_RSH:
      if (IsConst(next = rpn->base.next) && IsConst(ICFwd(rpn->base.next))) {
        if (rpn->raw_type == RT_F64) {
          SET_AB;
          rpn->type = IC_F64;
          rpn->integer =
              ((int64_t *)&a)[0] >> ((int64_t *)&b)[0]; // Type punning
        } else {
          SET_A2B2;
          rpn->type = IC_I64;
          rpn->integer = a2 >> b2; // TODO arithmetic or logical
        }
      } else
        goto next1;
      goto binop_next;
      break;
    case IC_ADD_EQ:
      goto next1;
      break;
    case IC_SUB_EQ:
      goto next1;
      break;
    case IC_MUL_EQ:
      goto next1;
      break;
    case IC_DIV_EQ:
      goto next1;
      break;
    case IC_LSH_EQ:
      goto next1;
      break;
    case IC_RSH_EQ:
      goto next1;
      break;
    case IC_AND_EQ:
      goto next1;
      break;
    case IC_OR_EQ:
      goto next1;
      break;
    case IC_XOR_EQ:
      goto next1;
      break;
    case IC_ARROW:
      goto next1;
      break;
    case IC_MOD_EQ:
      goto next1;
      break;
    case IC_I64:
      goto next1;
      break;
    case IC_F64:
      goto next1;
      break;
    case IC_ARRAY_ACC:
      goto next1;
      break;
    default:
      goto next1;
    }
  }
  if (changed)
    goto loop;
}

typedef struct {
  int64_t score;
  CMemberLst *m;
} COptMemberVar;
static int64_t OptMemberVarSort(COptMemberVar *a, COptMemberVar *b) {
  return a->score - b->score;
}

static int64_t OptMemberVarSortSz(COptMemberVar *a, COptMemberVar *b) {
  int64_t asz = a->m->member_class->sz * a->m->dim.total_cnt;
  int64_t bsz = b->m->member_class->sz * b->m->dim.total_cnt;
  return asz - bsz;
}

void OptPassRegAlloc(CCmpCtrl *cctrl) {
  /*
   * Heres the deal. AIWNIOS will not do fancy register allocation
   * shenanigins. We will get a score for REG_MAYBE|REG_ALLOC(bigger
   * score means more chance of getting put in a register).
   * If we dereference a variable with IC_ADDR,it will be turned into
   * REG_NONE(can't get pointer of register)
   */
  COptMemberVar *mv;
  CMemberLst *tmpm;
  CRPN *rpn, *next;
  int64_t i, cnt, ireg, freg, off, align, sz;
  if (!cctrl->cur_fun)
    return;
  mv = A_CALLOC(sizeof(COptMemberVar) * cctrl->cur_fun->base.member_cnt, NULL);
  i = 0;
  for (tmpm = cctrl->cur_fun->base.members_lst; tmpm; tmpm = tmpm->next) {
    mv[i].m = tmpm;
    if (tmpm->flags & MLF_STATIC)
      mv[i].m->reg = REG_NONE;
    mv[i++].score = 1;
  }
  for (rpn = cctrl->code_ctrl->ir_code->next; rpn != cctrl->code_ctrl->ir_code;
       rpn = rpn->base.next) {
    switch (rpn->type) {
      break;
    case IC_GOTO:
      continue;
      break;
    case IC_STATIC:
    case IC_LABEL:
      continue;
      break;
      break;
    case IC_LOCAL:
      for (i = 0; i != cctrl->cur_fun->base.member_cnt; i++) {
        if (mv[i].m == rpn->local_mem) {
          mv[i].score++;
          break;
        }
      }
      break;
    case IC_GLOBAL:
      continue;
      break;
    case IC_NOP:
      continue;
      break;
    case IC_PAREN:
      abort();
      break;
    case IC_NEG:
      continue;
      break;
    case IC_POS:
      continue;
      break;
    case IC_NAME:
      abort();
      break;
    case IC_STR:
      continue;
      break;
    case IC_CHR:
      continue;
      break;
    case IC_POW:
      continue;
      break;
    case IC_ADD:
      continue;
      break;
    case IC_EQ:
      continue;
      break;
    case IC_SUB:
      continue;
      break;
    case IC_DIV:
      continue;
      break;
    case IC_MUL:
      continue;
      break;
    case IC_DEREF:
      continue;
      break;
    case IC_AND:
      continue;
      break;
    case IC_ADDR_OF:
      next = rpn->base.next;
      if (next->type == IC_LOCAL) {
        for (i = 0; i != cctrl->cur_fun->base.member_cnt; i++) {
          if (mv[i].m == next->local_mem) {
            mv[i].score = 0;
            mv[i].m->reg = REG_NONE;
            break;
          }
        }
      }
      continue;
      break;
    case IC_XOR:
      continue;
      break;
    case IC_MOD:
      continue;
      break;
    case IC_OR:
      continue;
      break;
    case IC_LT:
      continue;
      break;
    case IC_GT:
      continue;
      break;
    case IC_LNOT:
      continue;
      break;
    case IC_BNOT:
      continue;
      break;
    case IC_POST_INC:
      continue;
      break;
    case IC_POST_DEC:
      continue;
      break;
    case IC_PRE_INC:
      continue;
      break;
    case IC_PRE_DEC:
      continue;
      break;
    case IC_AND_AND:
      continue;
      break;
    case IC_OR_OR:
      continue;
      break;
    case IC_XOR_XOR:
      continue;
      break;
    case IC_EQ_EQ:
      continue;
      break;
    case IC_NE:
      continue;
      break;
    case IC_LE:
      continue;
      break;
    case IC_GE:
      continue;
      break;
    case IC_LSH:
      continue;
      break;
    case IC_RSH:
      continue;
      break;
    case IC_ADD_EQ:
      continue;
      break;
    case IC_SUB_EQ:
      continue;
      break;
    case IC_MUL_EQ:
      continue;
      break;
    case IC_DIV_EQ:
      continue;
      break;
    case IC_LSH_EQ:
      continue;
      break;
    case IC_RSH_EQ:
      continue;
      break;
    case IC_AND_EQ:
      continue;
      break;
    case IC_OR_EQ:
      continue;
      break;
    case IC_XOR_EQ:
      continue;
      break;
    case IC_ARROW:
      continue;
      break;
    case IC_MOD_EQ:
      continue;
      break;
    case IC_I64:
      continue;
      break;
    case IC_F64:
      continue;
      break;
    case IC_ARRAY_ACC:
      continue;
      break;
    }
  }
  qsort(mv, cnt = cctrl->cur_fun->base.member_cnt, sizeof(COptMemberVar),
        (void *)&OptMemberVarSort);
  ireg = AIWNIOS_IREG_START;
  freg = AIWNIOS_FREG_START;
  for (i = cnt - 1; i >= 0; i--) {
    if (mv[i].m->reg == REG_MAYBE || mv[i].m->reg == REG_ALLOC) {
      if (mv[i].m->member_class->raw_type == RT_F64) {
        if (freg - AIWNIOS_FREG_START < AIWNIOS_FREG_CNT) {
#if defined(__riscv__) || defined(__riscv)
          switch (freg++ - AIWNIOS_FREG_START) {
            break;
          case 0:
            mv[i].m->reg = 9;
            break;
          case 1:
            mv[i].m->reg = 18;
            break;
          case 2:
            mv[i].m->reg = 19;
            break;
          case 3:
            mv[i].m->reg = 20;
            break;
          case 4:
            mv[i].m->reg = 21;
            break;
          case 5:
            mv[i].m->reg = 22;
            break;
          case 6:
            mv[i].m->reg = 23;
            break;
          case 7:
            mv[i].m->reg = 24;
            break;
          case 8:
            mv[i].m->reg = 25;
            break;
          case 9:
            mv[i].m->reg = 26;
            break;
          case 10:
            mv[i].m->reg = 27;
            break;
          case 11:
            mv[i].m->reg = 8;
            break;
          default:
            abort();
          }
#else
          mv[i].m->reg = freg++;
#endif
        } else
          mv[i].m->reg = REG_NONE;
      } else if (RT_I8i <= mv[i].m->member_class->raw_type &&
                 mv[i].m->member_class->raw_type <= RT_PTR &&
                 !mv[i].m->dim.next) {
        if (ireg - AIWNIOS_IREG_START < AIWNIOS_IREG_CNT) {
#if defined(__riscv__) || defined(__riscv)
          switch (ireg++ - AIWNIOS_IREG_START) {
            break;
          case 0:
            mv[i].m->reg = 9;
            break;
          case 1:
            mv[i].m->reg = 18;
            break;
          case 2:
            mv[i].m->reg = 19;
            break;
          case 3:
            mv[i].m->reg = 20;
            break;
          case 4:
            mv[i].m->reg = 21;
            break;
          case 5:
            mv[i].m->reg = 22;
            break;
          case 6:
            mv[i].m->reg = 23;
            break;
          case 7:
            mv[i].m->reg = 24;
            break;
          case 8:
            mv[i].m->reg = 25;
            break;
          case 9:
            mv[i].m->reg = 26;
            break;
          case 10:
            mv[i].m->reg = 27;
            break;
          default:
            abort();
          }
#endif
#if defined(__x86_64__)
          switch (ireg++ - AIWNIOS_IREG_START) {
            break;
          case 0:
            mv[i].m->reg = RDI;
            break;
          case 1:
            mv[i].m->reg = RSI;
            break;
          case 2:
            mv[i].m->reg = R10;
            break;
          case 3:
            mv[i].m->reg = R11;
            break;
          case 4:
            mv[i].m->reg = R12;
            break;
          case 5:
            // TODO something with R13
            mv[i].m->reg = R14;
            break;
          case 6:
            mv[i].m->reg = R15;
            break;
          default:
            abort();
          }
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
          mv[i].m->reg = ireg++;
#endif
        } else
          mv[i].m->reg = REG_NONE;
      } else
        mv[i].m->reg = REG_NONE;
    }
  }
  // Time to assign the rest of the function members to the base pointer
  qsort(mv, cnt, sizeof(COptMemberVar), (void *)&OptMemberVarSortSz);
#if defined(__riscv__) || defined(__riscv)
  // In RiscV
  //  s0    -> TOP
  //  s0-8  -> return_address
  //  s0-16 -> old s0
  off = 16;
#else
  off = 0;
#endif
  for (i = 0; i != cnt; i++) {
    if (mv[i].m->reg == REG_NONE && !(mv[i].m->flags & MLF_STATIC)) {
      switch (mv[i].m->member_class->raw_type) {
        break;
      case RT_U0:
        align = 1;
        break;
      case RT_U8i:
        align = 1;
        break;
      case RT_I8i:
        align = 1;
        break;
      case RT_I16i:
        align = 2;
        break;
      case RT_U16i:
        align = 2;
        break;
      case RT_I32i:
        align = 4;
        break;
      case RT_U32i:
        align = 4;
        break;
      default:
        align = 8;
      }
      off += (align - off % align) % align;
      sz = mv[i].m->member_class->sz;
      sz *= mv[i].m->dim.total_cnt;
      mv[i].m->off = off;
#if defined(__x86_64__) || defined(__riscv) || defined(__riscv__)
      // In X86_64 the base pointer above of the  stack's bottom,
      // I will move the items down by thier size so they are at the bottom
      //
      //  RBP<===base ptr+0
      //  I64 x(local_mem->off+=8) //Move x to be 8 bytes below RBP
      //  RSP<=== -8 //Move x down here
      mv[i].m->off += mv[i].m->member_class->sz * mv[i].m->dim.total_cnt;
#endif
      off += sz;
    }
  }
  if (cctrl->cur_fun)
    cctrl->cur_fun->base.sz = off; // Stack size
  // Replace references of function members to IC_LOCAL/IREG/FREG
  for (rpn = cctrl->code_ctrl->ir_code->next; rpn != cctrl->code_ctrl->ir_code;
       rpn = rpn->base.next) {
    if (rpn->type == IC_LOCAL) {
      if (rpn->local_mem->flags & MLF_STATIC) {
        rpn->type = IC_STATIC;
        rpn->integer = rpn->local_mem->static_bytes;
      } else if (rpn->local_mem->reg != REG_NONE) {
        i = rpn->local_mem->reg;
        if (rpn->local_mem->member_class->raw_type == RT_F64) {
          rpn->type = IC_FREG;
        } else
          rpn->type = IC_IREG;
        rpn->integer = i;
      } else {
        i = rpn->local_mem->off;
        rpn->type = IC_BASE_PTR;
        rpn->integer = i;
      }
    }
  }
  A_FREE(mv);
}
static int64_t AlwaysPasses(CRPN *rpn) {
  if (!IsConst(rpn))
    return 0;
  switch (rpn->type) {
    break;
  case IC_I64:
    return !!rpn->integer;
    break;
  case IC_F64:
    return !!rpn->flt;
    break;
  case IC_CHR:
    return !!rpn->integer;
    break;
  case IC_STR:
    return 1;
  }
  return 0;
}
void OptPassRemoveUselessArith(CCmpCtrl *cctrl) {
  CRPN *r, *next, *next2;
  int64_t is_value;
#define RPN_IS_VALUE(RPN, v)                                                   \
  is_value = 0;                                                                \
  switch (RPN->type) {                                                         \
    break;                                                                     \
  case IC_I64:                                                                 \
    is_value = RPN->integer == v;                                              \
    break;                                                                     \
  case IC_F64:                                                                 \
    is_value = RPN->flt == v;                                                  \
  }
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = next2) {
    next2 = r->base.next;
    switch (r->type) {
      break;
    case IC_ADD:
      next = ICArgN(r, 0);
      RPN_IS_VALUE(next, 0);
      if (is_value) {
        next2 = ICArgN(r, 1);
        ICFree(next);
        ICFree(r);
        continue;
      }
      next = ICArgN(r, 1);
      RPN_IS_VALUE(next, 0);
      if (is_value) {
        next2 = ICArgN(r, 0);
        ICFree(next);
        ICFree(r);
        continue;
      }
      break;
    case IC_SUB:
      next = ICArgN(r, 0);
      RPN_IS_VALUE(next, 0);
      if (is_value) {
        next2 = ICArgN(r, 1);
        ICFree(next);
        ICFree(r);
        continue;
      }
      break;
    case IC_MUL:
      next = ICArgN(r, 0);
      RPN_IS_VALUE(next, 1);
      if (is_value) {
        next2 = ICArgN(r, 1);
        ICFree(next);
        ICFree(r);
        continue;
      }
      next = ICArgN(r, 1);
      RPN_IS_VALUE(next, 1);
      if (is_value) {
        next2 = ICArgN(r, 0);
        ICFree(next);
        ICFree(r);
        continue;
      }
      break;
    case IC_DIV:
      next = ICArgN(r, 0);
      RPN_IS_VALUE(next, 1);
      if (is_value) {
        next2 = ICArgN(r, 1);
        ICFree(next);
        ICFree(r);
        continue;
      }
    }
  }
}
static void OptPassRemoveUselessTypecasts(CCmpCtrl *cctrl) {
  CRPN *r, *next;
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = next) {
    next = r->base.next;
    if (r->type == IC_TYPECAST) {
      if ((r->raw_type == RT_F64) == (next->raw_type == RT_F64)) {
        next->raw_type = r->raw_type;
        next->ic_class = r->ic_class;
        ICFree(r);
      }
    }
  }
}
static void OptPassMergeAddressOffsets(CCmpCtrl *cctrl) {
  CRPN *r, *arg, *next, *base, *off;
  int64_t run;
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = next) {
    next = r->base.next;
    if (r->type == IC_ADD) {
      for (run = 0; run != 2; run++) {
        arg = ICArgN(r, run);
        off = ICArgN(r, 1 - run);
        if (arg->type != IC_ADDR_OF)
          continue;
        if (off->type != IC_I64)
          continue;
        base = arg->base.next;
        switch (base->type) {
        case IC_SHORT_ADDR:
        case __IC_STATIC_REF:
          base->integer += off->integer;
          goto fin;
          break;
        case IC_BASE_PTR:
// Stack grows down
#if defined(__x86_64__) || defined(__riscv) || defined(__riscv__)
          base->integer -= off->integer;
#else
          base->integer += off->integer;
#endif
        fin:
          arg->raw_type = r->raw_type;
          ICFree(r);
          ICFree(off);
          next = base;
          goto nxt;
        }
      }
    }
  nxt:;
  }
  // When we do "cd3.z" we have *(base_ptr+16). Turn *(base+16) into a frame ptr
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = next) {
    next = r->base.next;
    if (r->type == IC_DEREF && next->type == IC_ADDR_OF) {
      base = next->base.next;
      if (base->type == IC_BASE_PTR) {
        // Set the raw type of the IC_DEREF to the base_ptr
        base->raw_type = r->raw_type;
        ICFree(r);
        ICFree(next);
        next = base;
      }
    }
  }
}

char *Compile(CCmpCtrl *cctrl, int64_t *res_sz, char **dbg_info,
              CHeapCtrl *heap) {
  CRPN *r;
  int64_t old_flags = cctrl->flags;
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = r->base.next) {
    AssignRawTypeToNode(cctrl, r);
  }
  OptPassFixFunArgs(cctrl);
  OptPassExpandPtrs(cctrl);
  OptPassConstFold(cctrl);
  OptPassMergeCommunitives(cctrl);
  // OptPassDeadCodeElim(cctrl);
  OptPassRegAlloc(cctrl);
  OptPassRemoveUselessArith(cctrl);
  OptPassRemoveUselessTypecasts(cctrl);
  OptPassMergeAddressOffsets(cctrl);
  cctrl->flags = old_flags;
  return OptPassFinal(cctrl, res_sz, dbg_info, heap);
}
