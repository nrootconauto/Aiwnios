// RISCV assembly generator
#include "aiwn_hash.h"
#include "aiwn_lexparser.h"
#include "aiwn_mem.h"
#include "aiwn_riscv.h"
#include <assert.h>
#include <stddef.h>
extern void DoNothing();
// backendd_user_data5 is the "fail" codemisc for IC_LT/GT etc
// backend_user_data6 is the "pass" codemisc for IC_LT/GT etc
// backend_user_data7 is the number of Fregs
// backend_user_data9 is set if we were called from __SexyAssignOp
//
// a

#define RISCV_IPOOP1          5
#define RISCV_IPOOP2          6
#define RISCV_FPOOP1          28
#define RISCV_FPOOP2          1
#define RISCV_IRET            10
#define RISCV_FRET            10
#define RISCV_FASSIGN_TMP     12
#define RISCV_IASSIGN_TMP     12
#define RISCV_ASSIGN_ADDR_TMP 13
#define RISCV_PTR_TMP         31
#define RISCV_IIMM_TMP        15

static int64_t MIR(CCmpCtrl *cc, int64_t r) {
  CRPN *rpn = cc->cur_rpn;
  if (!rpn)
    return r;
  rpn->changes_iregs |= 1 << r;
  return r;
}
static int64_t MFR(CCmpCtrl *cc, int64_t r) {
  CRPN *rpn = cc->cur_rpn;
  if (!rpn)
    return r;
  rpn->changes_fregs |= 1 << r;
  return r;
}

static int64_t CanKeepInTmp(CRPN *me, CRPN *have, CRPN *other,
                            int64_t is_left_side) {
  int64_t mask;
  if (me->flags & ICF_SPILLED)
    return 0;
  if (have->res.mode == MD_I64 || have->res.mode == MD_F64)
    return 0; // No need to stuff in tmp
  if (is_left_side) {
    if (other)
      mask = other->changes_iregs2 | me->changes_iregs;
    else
      mask = me->changes_iregs;
  } else {
    mask = me->changes_iregs;
  }

  if (have->tmp_res.mode == MD_REG && have->tmp_res.raw_type != RT_F64)
    if (mask & (1ull << have->tmp_res.reg))
      return 0;

  if (is_left_side) {
    if (other)
      mask = other->changes_fregs2 | me->changes_fregs;
    else
      mask = me->changes_fregs;
  } else {
    mask = me->changes_fregs;
  }
  if (have->tmp_res.mode == MD_REG && have->tmp_res.raw_type == RT_F64)
    if (mask & (1ull << have->tmp_res.reg))
      return 0;

  return 1;
}
//
// Sets keep_in_tmp
//
static int64_t SpillsTmpRegs(CRPN *rpn);
static void SetKeepTmps(CRPN *rpn) {
  int64_t idx;
  CRPN *left, *right;
  switch (rpn->type) {
  case IC_FS:
  case IC_GS:
    return;
  case IC_RET:
  case IC_GOTO_IF:
    goto unop;
  case __IC_CALL:
  case IC_CALL:
    for (idx = 0; idx != rpn->length; idx++) {
      right = ICArgN(rpn, idx);
      if (right->tmp_res.mode) {
        right->res.keep_in_tmp = 0;
      }
    }
    break;
  case __IC_VARGS:
    for (idx = 0; idx != rpn->length; idx++) {
      right = ICArgN(rpn, idx);
      if (right->tmp_res.mode) {
        right->res = right->tmp_res;
        right->res.keep_in_tmp = 1;
      }
    }
    return;
  case IC_TO_F64:
  case IC_TO_I64:
  case IC_NEG:
  case IC_TO_BOOL:
  unop:
    right = ICArgN(rpn, 0);
    right->res = right->tmp_res;
    if (right->tmp_res.mode)
      right->res.keep_in_tmp = 1;
    break;
  case IC_POS:
    goto unop;
    break;
  case IC_POW:
  binop:
    left = ICArgN(rpn, 1);
    right = ICArgN(rpn, 0);
    // In AArch64,I do left last,safe to  do stuff with it as ICArgN(rpn,0) wont
    // mutate regs
    if (CanKeepInTmp(rpn, right, left, 0) && !SpillsTmpRegs(left) &&
        right->tmp_res.mode) {
      if (right->tmp_res.mode == MD_REG) {
        right->res = right->tmp_res;
        right->res.keep_in_tmp = 1;
        break;
      }
    }
    break;
  case IC_ADD:
    goto binop;
    break;
  case IC_EQ:
    goto abinop;
    break;
  case IC_SUB:
    goto binop;
    break;
  case IC_DIV:
    goto binop;
    break;
  case IC_MUL:
    goto binop;
    break;
  case IC_DEREF:
    goto unop;
    break;
  case IC_AND:
    goto binop;
    break;
  case IC_ADDR_OF:
    goto unop;
    break;
  case IC_XOR:
    goto binop;
    break;
  case IC_MOD:
    // composuite operation
    break;
  case IC_OR:
    goto binop;
    break;
  case IC_LT:
    goto binop;
    break;
  case IC_GT:
    goto binop;
    break;
  case IC_LNOT:
    goto unop;
    break;
  case IC_BNOT:
    goto unop;
    break;
  case IC_POST_INC:
    // Need a temporary to store result so no
    break;
  case IC_POST_DEC:
    // Need a temporary to store result so no
    break;
  case IC_PRE_INC:
    break;
  case IC_PRE_DEC:
    break;
  case IC_AND_AND:
    goto binop;
  case IC_OR_OR:
    goto binop;
    break;
  case IC_XOR_XOR:
    goto binop;
    break;
  case IC_EQ_EQ:
    goto binop;
    break;
  case IC_NE:
    goto binop;
    break;
  case IC_LE:
    goto binop;
    break;
  case IC_GE:
    goto binop;
    break;
  case IC_LSH:
    goto binop;
    break;
  case IC_RSH:
    goto binop;
    break;
  case IC_ADD_EQ:
    goto abinop;
    break;
  case IC_SUB_EQ:
    goto abinop;
    break;
  case IC_MUL_EQ:
    goto abinop;
    break;
  case IC_DIV_EQ:
    goto abinop;
    break;
  case IC_LSH_EQ:
    goto abinop;
    break;
  case IC_RSH_EQ:
    goto abinop;
    break;
  case IC_AND_EQ:
    goto abinop;
    break;
  case IC_OR_EQ:
    goto abinop;
    break;
  case IC_XOR_EQ:
    goto abinop;
    break;
  abinop:
    left = ICArgN(rpn, 1);
    right = ICArgN(rpn, 0);
    if (left->type == IC_IREG || left->type == IC_FREG ||
        left->type == IC_BASE_PTR) {
      if (right->tmp_res.mode == MD_REG && CanKeepInTmp(rpn, right, left, 0)) {
        right->res = right->tmp_res;
        right->res.keep_in_tmp = 1;
      }
    }
    break;
  case IC_BT:
  case IC_BTS:
  case IC_BTR:
  case IC_BTC:
  case IC_LBTS:
  case IC_LBTR:
  case IC_LBTC:
  case IC_STORE:
    goto binop;
  case IC_TYPECAST:
    goto unop;
  }
  return;
t:
  return;
}

static int64_t IsConst(CRPN *rpn);
static int64_t ConstVal(CRPN *rpn);
static int64_t __SexyAssignOp(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                              int64_t code_off);
static int64_t IsSavedFReg(int64_t r) {
  switch (r) {
  case 8 ... 9:
  case 18 ... 27:
    return 1;
  }
  return 0;
}
static int64_t IsCompoundCompare(CRPN *r) {
  CRPN *next = r->base.next;
  switch (r->type) {
  case IC_LT:
  case IC_GT:
  case IC_LE:
  case IC_GE:
    next = ICFwd(next);
    switch (next->type) {
    case IC_LT:
    case IC_GT:
    case IC_LE:
    case IC_GE:
      return 1;
    }
    return 0;
    /*case IC_NE:
    case IC_EQ_EQ:
      next = ICFwd(next);
      switch (next->type) {
      case IC_EQ_EQ:
      case IC_NE:
        return 1;
      }
      return 0;
      */
  }
  return 0;
}
static int64_t Is12Bit(int64_t i) {
  return i >= -(1 << 11) && i <= (1 << 11) - 1;
}
static int64_t IsSavedIReg(int64_t r);
static int64_t ModeIsDerefToROff(CRPN *m) {
  CRPN *a, *b;
  if (m->type == IC_DEREF) {
    m = m->base.next;
    if (m->type == IC_ADD || m->type == IC_SUB) {
      a = ICArgN(m, 0);
      b = ICArgN(m, 1);
      // Dont do 123-reg
      if (m->type != IC_SUB && IsConst(b) && Is12Bit(ConstVal(b)))
        return 1;
      if (IsConst(a) && Is12Bit(ConstVal(a)))
        return 1;
    }
  }
  return 0;
}

static int64_t RawTypeIs64(int64_t r) {
  switch (r) {
  case RT_U64i:
  case RT_I64i:
  case RT_F64:
  case RT_PTR:
    return 1;
  }
  return 0;
}
//
// Assembly section
//

extern int64_t ICMov(CCmpCtrl *cctrl, CICArg *dst, CICArg *src, char *bin,
                     int64_t code_off);
extern int64_t __OptPassFinal(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                              int64_t code_off);

#define AIWNIOS_ADD_CODE(func)                                                 \
  {                                                                            \
    int32_t have = func;                                                       \
    if (bin)                                                                   \
      *(int32_t *)((char *)bin + code_off) = func;                             \
    code_off += 4;                                                             \
  }

//
// Codegen section
//

static int64_t IsTerminalInst(CRPN *r) {
  switch (r->type) {
    break;
  case IC_RET:
  case IC_GOTO:
    return 1;
  }
  return 0;
}

static int64_t IsConst(CRPN *rpn) {
  switch (rpn->type) {
  case IC_CHR:
  case IC_F64:
  case IC_I64:
    return 1;
  }
  return 0;
}

static int64_t ConstVal(CRPN *rpn) {
  switch (rpn->type) {
  case IC_CHR:
  case IC_I64:
    return rpn->integer;
  case IC_F64:
    return rpn->flt;
  }
  return 0;
}

//
// If we call a function the tmp regs go to poo poo land
//
static int64_t SpillsTmpRegs(CRPN *rpn) {
  int64_t idx;
  switch (rpn->type) {
    break;
  case IC_FS:
  case IC_GS:
    return 0;
  case __IC_CALL:
  case IC_CALL:
    return 1;
    break;
  case __IC_VARGS:
    for (idx = 0; idx != rpn->length; idx++)
      if (SpillsTmpRegs(ICArgN(rpn, idx)))
        return 1;
    return 0;
  case IC_TO_F64:
  case IC_TO_I64:
  case IC_NEG:
  case IC_TO_BOOL:
  unop:
    return SpillsTmpRegs(rpn->base.next);
    break;
  case IC_POS:
    goto unop;
    break;
  case IC_POW:
    return 1; // calls pow
  binop:
    if (SpillsTmpRegs(rpn->base.next))
      return 1;
    if (SpillsTmpRegs(ICFwd(rpn->base.next)))
      return 1;
    break;
  case IC_ADD:
    goto binop;
    break;
  case IC_EQ:
    goto binop;
    break;
  case IC_SUB:
    goto binop;
    break;
  case IC_DIV:
    goto binop;
    break;
  case IC_MUL:
    goto binop;
    break;
  case IC_DEREF:
    goto unop;
    break;
  case IC_AND:
    goto binop;
    break;
  case IC_ADDR_OF:
    goto unop;
    break;
  case IC_XOR:
    goto binop;
    break;
  case IC_MOD:
    goto binop;
    break;
  case IC_OR:
    goto binop;
    break;
  case IC_LT:
    goto binop;
    break;
  case IC_GT:
    goto binop;
    break;
  case IC_LNOT:
    goto unop;
    break;
  case IC_BNOT:
    goto unop;
    break;
  case IC_POST_INC:
    goto binop;
    break;
  case IC_POST_DEC:
    goto binop;
    break;
  case IC_PRE_INC:
    goto binop;
    break;
  case IC_PRE_DEC:
    goto binop;
    break;
  case IC_AND_AND:
    goto binop;
  case IC_OR_OR:
    goto binop;
    break;
  case IC_XOR_XOR:
    goto binop;
    break;
  case IC_EQ_EQ:
    goto binop;
    break;
  case IC_NE:
    goto binop;
    break;
  case IC_LE:
    goto binop;
    break;
  case IC_GE:
    goto binop;
    break;
  case IC_LSH:
    goto binop;
    break;
  case IC_RSH:
    goto binop;
    break;
  case IC_ADD_EQ:
    goto binop;
    break;
  case IC_SUB_EQ:
    goto binop;
    break;
  case IC_MUL_EQ:
    goto binop;
    break;
  case IC_DIV_EQ:
    goto binop;
    break;
  case IC_LSH_EQ:
    goto binop;
    break;
  case IC_RSH_EQ:
    goto binop;
    break;
  case IC_AND_EQ:
    goto binop;
    break;
  case IC_OR_EQ:
    goto binop;
    break;
  case IC_XOR_EQ:
    goto binop;
    break;
  case IC_MOD_EQ:
    goto binop;
    break;
  case IC_BT:
  case IC_BTS:
  case IC_BTR:
  case IC_BTC:
  case IC_LBTS:
  case IC_LBTR:
  case IC_LBTC:
  case IC_STORE:
    goto binop;
  case IC_TYPECAST:
    goto unop;
  }
  return 0;
t:
  return 1;
}

//
// Will overwrite the arg's value,but it's ok as we are about to use it's
// value(and discard it)
//
static int64_t PutICArgIntoReg(CCmpCtrl *cctrl, CICArg *arg, int64_t raw_type,
                               int64_t fallback, char *bin, int64_t code_off) {
  CICArg tmp = {0};
  if (arg->mode == MD_REG) {
    if ((raw_type == RT_F64) != (arg->raw_type == RT_F64)) {
      // They differ so continue as usaul
    } else { // They are the same
      return code_off;
    }
  }
  tmp.raw_type = raw_type;
  tmp.reg = fallback;
  tmp.mode = MD_REG;
  code_off = ICMov(cctrl, &tmp, arg, bin, code_off);
  memcpy(arg, &tmp, sizeof(CICArg));
  return code_off;
}

#define RISCV_REG_SP 2
#define RISCV_REG_FP 8
static int64_t PushToStack(CCmpCtrl *cctrl, CICArg *arg, char *bin,
                           int64_t code_off) {
  CICArg tmp = *arg;
  AIWNIOS_ADD_CODE(RISCV_ADDI(RISCV_REG_SP, RISCV_REG_SP, -8));
  code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 5, bin, code_off);
  if (tmp.raw_type != RT_F64) {
    AIWNIOS_ADD_CODE(RISCV_SD(tmp.reg, RISCV_REG_SP, 0));
  } else {
    AIWNIOS_ADD_CODE(RISCV_FSD(tmp.reg, RISCV_REG_SP, 0));
  }
  return code_off;
}
static int64_t PopFromStack(CCmpCtrl *cctrl, CICArg *arg, char *bin,
                            int64_t code_off) {
  CICArg stk = {0};
  stk.mode = MD_INDIR_REG;
  stk.reg = RISCV_REG_SP;
  if (arg->raw_type == RT_F64)
    stk.raw_type = RT_F64;
  else
    stk.raw_type = RT_I64i;
  code_off = ICMov(cctrl, arg, &stk, bin, code_off);
  AIWNIOS_ADD_CODE(RISCV_ADDI(RISCV_REG_SP, RISCV_REG_SP, 8));
  return code_off;
}
static int64_t Is32Bit(int64_t num) {
  if (num > -0x7fFFffFFll)
    if (num < 0x7fFFffFFll)
      return 1;
  return 0;
}
static int64_t __ICMoveI64(CCmpCtrl *cctrl, int64_t reg, uint64_t imm,
                           char *bin, int64_t code_off) {
  CCodeMisc *misc;
  int64_t low12;
  if (!imm) {
    AIWNIOS_ADD_CODE(RISCV_ADDI(reg, 0, imm));
    return code_off;
  }
  if (Is12Bit(imm)) {
    AIWNIOS_ADD_CODE(RISCV_ADDI(reg, 0, imm));
  } else if (Is32Bit(imm)) {
    low12 = imm - (imm & ~((1 << 12) - 1));
    if (Is12Bit(low12)) { /*Chekc for bit 12 being set*/
      AIWNIOS_ADD_CODE(RISCV_LUI(reg, imm >> 12));
      AIWNIOS_ADD_CODE(RISCV_ADDI(reg, reg, low12));
    } else {
      AIWNIOS_ADD_CODE(RISCV_LUI(reg, 1 + (imm >> 12)));
      AIWNIOS_ADD_CODE(RISCV_ADDI(reg, reg, low12));
    }
  } else if (!(cctrl->flags & CCF_AOT_COMPILE) && bin &&
             Is32Bit((int64_t)imm - (int64_t)(bin + code_off))) {
    imm -= (int64_t)(bin + code_off);
    low12 = imm - (imm & ~((1 << 12) - 1));
    if (Is12Bit(low12)) {
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, reg), (imm >> 12)));
      AIWNIOS_ADD_CODE(RISCV_ADDI(reg, reg, low12));
    } else {
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, reg), 1 + (imm >> 12)));
      AIWNIOS_ADD_CODE(RISCV_ADDI(reg, reg, low12));
    }
  } else {
    for (misc = cctrl->code_ctrl->code_misc->next;
         misc != cctrl->code_ctrl->code_misc; misc = misc->base.next) {
      if (misc->type == CMT_INT_CONST)
        if (misc->integer == *(int64_t *)&imm) {
          goto found;
        }
    }
    misc = A_CALLOC(sizeof(CCodeMisc), cctrl->hc);
    misc->type = CMT_INT_CONST;
    misc->integer = imm;
    QueIns(misc, cctrl->code_ctrl->code_misc->last);
  found:
    AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, reg), 0));
    AIWNIOS_ADD_CODE(RISCV_LD(reg, reg, 0));
    if (bin)
      CodeMiscAddRef(misc, bin + code_off - 8);
  }
  return code_off;
}
static int64_t __ICMoveF64(CCmpCtrl *cctrl, int64_t reg, double imm, char *bin,
                           int64_t code_off) {
  CCodeMisc *misc;
  for (misc = cctrl->code_ctrl->code_misc->next;
       misc != cctrl->code_ctrl->code_misc; misc = misc->base.next) {
    if (misc->type == CMT_FLOAT_CONST)
      if (misc->integer == *(int64_t *)&imm) {
        goto found;
      }
  }
  misc = A_CALLOC(sizeof(CCodeMisc), cctrl->hc);
  misc->type = CMT_FLOAT_CONST;
  misc->integer = *(int64_t *)&imm;
  QueIns(misc, cctrl->code_ctrl->code_misc->last);
found:
  AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, 5), 0));
  AIWNIOS_ADD_CODE(RISCV_FLD(MFR(cctrl, reg), 5, 0));
  if (bin)
    CodeMiscAddRef(misc, bin + code_off - 8);
  return code_off;
}

static void PushSpilledTmp(CCmpCtrl *cctrl, CRPN *rpn) {
  int64_t raw_type = rpn->raw_type;
  CICArg *res = &rpn->res;
  if (rpn->flags & ICF_PRECOMPUTED)
    return;
  // These have no need to be put into a temporay
  switch (rpn->type) {
    break;
  case IC_I64:
    res->mode = MD_I64;
    res->integer = rpn->integer;
    res->raw_type = RT_I64i;
    return;
    break;
  case IC_F64:
    res->mode = MD_F64;
    res->flt = rpn->flt;
    res->raw_type = RT_F64;
    return;
    break;
  case IC_IREG:
  case IC_FREG:
    res->mode = MD_REG;
    res->raw_type = raw_type;
    res->reg = rpn->integer;
    return;
    break;
  case IC_BASE_PTR:
    res->mode = MD_FRAME;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
  case IC_STATIC:
    res->mode = MD_PTR;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
  case __IC_STATIC_REF:
    res->mode = MD_STATIC;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
  }
  res->keep_in_tmp = 0; // Dont use tmp registers
  rpn->flags |= ICF_SPILLED;
  res->is_tmp = 1;
  if (raw_type != RT_F64) {
    res->mode = MD_FRAME;
    res->raw_type = raw_type;
    if (cctrl->backend_user_data0 < (res->off = cctrl->backend_user_data1 +=
                                     8)) { // Will set ref->off before the top
      cctrl->backend_user_data0 = res->off;
    }
  } else {
    res->mode = MD_FRAME;
    res->raw_type = raw_type;
    if (cctrl->backend_user_data0 < (res->off = cctrl->backend_user_data1 +=
                                     8)) { // Will set ref->off before the top
      cctrl->backend_user_data0 = res->off;
    }
  }
  // See two above notes
  res->off -= 8;
  // In functions,wiggle room comes after the function locals.
  if (cctrl->cur_fun && res->mode == MD_FRAME)
    res->off += cctrl->cur_fun->base.sz + 16; //+16 for LR/FP
  else if (res->mode == MD_FRAME) {
    if (cctrl->backend_user_data4) {
      res->off += cctrl->backend_user_data4; // wiggle room start(if set)
    } else
      // Anonymus potatoes like to assume LR/FP pair too.
      res->off += 24;
  }
}
int64_t ITmpRegToReg(int64_t r) {
  switch (r) {
    break;
  case 0:
    return 28;
  case 1:
    return 29;
  case 2:
    return 30;
  default:
    abort();
  }
}
int64_t FTmpRegToReg(int64_t r) {
  switch (r) {
    break;
  case 0 ... 2:
    return r + 29;
  default:
    abort();
  }
}

//
// Takes an argument called inher. We can use the parent's destination
// as a temporary. I'll give an example
//
// DO: -~!a;
// - dst=0
//  ~ dst=0
//   ! dst=0
//
// DON'T: -~!a
// - dst=0
//  ~ dst=1
//   ! dst=2

static void PushTmp(CCmpCtrl *cctrl, CRPN *rpn, CICArg *inher_from) {
  int64_t raw_type = rpn->raw_type;
  CICArg *res = &rpn->res;
  if (rpn->flags & ICF_PRECOMPUTED)
    return;
  // These have no need to be put into a temporay
  switch (rpn->type) {
    break;
  case __IC_STATIC_REF:
    res->mode = MD_STATIC;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
  case IC_STATIC:
    res->mode = MD_PTR;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
    // These dudes will compile down to a MD_I64/MD_F64
  case IC_I64:
    res->mode = MD_I64;
    res->integer = rpn->integer;
    res->raw_type = RT_I64i;
    return;
    break;
  case IC_F64:
    res->mode = MD_F64;
    res->flt = rpn->flt;
    res->raw_type = RT_F64;
    return;
  case IC_IREG:
  case IC_FREG:
    res->mode = MD_REG;
    res->raw_type = raw_type;
    res->reg = rpn->integer;
    return;
    break;
  case IC_BASE_PTR:
    res->mode = MD_FRAME;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
  }
  if (inher_from) {
    if (rpn->raw_type == RT_F64 && inher_from->raw_type == RT_F64 &&
        inher_from->mode != MD_NULL) {
      rpn->res = *inher_from;
      rpn->flags |= ICF_TMP_NO_UNDO;
      return;
    } else if (rpn->raw_type != RT_F64 && inher_from->raw_type != RT_F64 &&
               inher_from->mode != MD_NULL) {
      rpn->res = *inher_from;
      rpn->flags |= ICF_TMP_NO_UNDO;
      return;
    }
  }
use_defacto:
  if (raw_type != RT_F64) {
    if (cctrl->backend_user_data2 < AIWNIOS_TMP_IREG_CNT) {
      res->mode = MD_REG;
      res->raw_type = raw_type;
      res->off = 0;
      res->reg = ITmpRegToReg(cctrl->backend_user_data2++);
    } else {
      PushSpilledTmp(cctrl, rpn);
    }
  } else {
    if (cctrl->backend_user_data3 < AIWNIOS_TMP_FREG_CNT) {
      res->mode = MD_REG;
      res->raw_type = raw_type;
      res->off = 0;
      res->reg = FTmpRegToReg(cctrl->backend_user_data3++);
    } else {
      PushSpilledTmp(cctrl, rpn);
    }
  }
}

static void PopTmp(CCmpCtrl *cctrl, CRPN *rpn) {
  int64_t raw_type = rpn->raw_type;
  // We didnt modift the res with PushTmp,so dont modify the results here
  if (rpn->flags & ICF_PRECOMPUTED)
    return;
  if (rpn->flags & ICF_TMP_NO_UNDO)
    return;
  // These have no need to be put into a temporay
  switch (rpn->type) {
    break;
  case IC_I64:
  case IC_F64:
  case IC_IREG:
  case IC_FREG:
  case IC_BASE_PTR:
  case IC_STATIC:
  case __IC_STATIC_REF:
    return;
  }
  if (rpn->flags & ICF_SPILLED) {
    cctrl->backend_user_data1 -= 8;
  } else {
    if (raw_type != RT_F64) {
    indir:
      if (rpn->flags & (ICF_INDIR_REG)) {
        cctrl->backend_user_data2 -= rpn->res.pop_n_tmp_regs;
        assert(cctrl->backend_user_data2 >= 0);
      } else {
        --cctrl->backend_user_data2;
        assert(cctrl->backend_user_data2 >= 0);
      }
    } else {
      if (rpn->flags & (ICF_SIB | ICF_INDIR_REG))
        goto indir;
      --cctrl->backend_user_data3;
      assert(cctrl->backend_user_data3 >= 0);
    }
  }
  assert(cctrl->backend_user_data1 >= 0);
}
// Returns the non-constant side of a +Const
static CRPN *__AddOffset(CRPN *r, int64_t *const_val) {
  if (r->type != IC_ADD && r->type != IC_SUB)
    return NULL;
  int64_t mul = (r->type == IC_ADD) ? 1 : -1;
  CRPN *n0 = ICArgN(r, 0), *n1 = ICArgN(r, 1);
  if (IsConst(n0) && Is12Bit(ConstVal(n0))) {
    if (const_val)
      *const_val = mul * ConstVal(n0);
    return n1;
  }
  // Dont do 256-idx(ONLY DO idx-256)
  if (IsConst(n1) && Is12Bit(ConstVal(n1)) && r->type != IC_SUB) {
    if (const_val)
      *const_val = mul * ConstVal(n1);
    return n0;
  }
  return NULL;
}
static int64_t IsCompare(int64_t c) {
  switch (c) {
  case IC_GT:
  case IC_LT:
  case IC_GE:
  case IC_LE:
    return 1;
  }
  return 0;
}

// Returns 1 if we hit the bottom
static int64_t PushTmpDepthFirst(CCmpCtrl *cctrl, CRPN *r, int64_t spilled) {
  switch (r->type) {
  case __IC_STATIC_REF:
  case IC_STATIC:
  case IC_GLOBAL:
  case IC_I64:
  case IC_F64:
  case IC_IREG:
  case IC_FREG:
  case IC_BASE_PTR:
  case IC_STR:
  case IC_CHR:
  case IC_FS:
  case IC_GS:
    if (!spilled)
      PushTmp(cctrl, r, NULL);
    else
      PushSpilledTmp(cctrl, r);
    return 0;
  }
  int64_t a, argc, old_icnt = cctrl->backend_user_data2,
                   old_fcnt = cctrl->backend_user_data3, i, i2, tmp,
                   old_scnt = cctrl->backend_user_data1;
  CRPN *arg, *arg2, *d, *b, *idx, *orig, **array, *new;
  switch (r->type) {
    break;
  case IC_MAX_I64:
  case IC_MAX_U64:
  case IC_MIN_I64:
  case IC_MIN_U64:
    goto binop;
  case IC_SHORT_ADDR:
    goto fin;
  case IC_CALL:
  case __IC_CALL:;
    for (i = 0; i < r->length + 1; i++) {
      PushTmpDepthFirst(
          cctrl, ICArgN(r, i),
          i != r->length /* Exlcude function */); // See __ICFCallTOS,We spill
                                                  // non-"constant" arguments so
                                                  // we can load them into
                                                  // argument reigsters
    }
    for (i = r->length; i >= 0; i--) {
      PopTmp(cctrl, ICArgN(r, i));
    }
    goto fin;
    break;
  case IC_TO_I64:
  case IC_TO_BOOL:
  unop:
    PushTmpDepthFirst(cctrl, r->base.next, 0);
    PopTmp(cctrl, r->base.next);
    goto fin;
    break;
  case IC_TO_F64:
    goto unop;
    break;
  case IC_PAREN:
    goto unop;
    break;
  case IC_NEG:
    goto unop;
    break;
  case IC_POS:
    goto unop;
    break;
  case IC_LT:
  case IC_GT:
  case IC_GE:
  case IC_LE:
  cmp_binop:
    arg = ICArgN(r, 1);
    arg2 = ICArgN(r, 0);
    if (!IsCompoundCompare(r)) {
      PushTmpDepthFirst(cctrl, arg, SpillsTmpRegs(arg2));
      PushTmpDepthFirst(cctrl, arg2, 0);
      PopTmp(cctrl, arg2);
      PopTmp(cctrl, arg);
      goto fin;
    }
    // There are 2 compares here,but 3 args
    // a<b<c
    argc = 1;
    d = r;
    while (IsCompare(d->type)) {
      d = ICFwd(d->base.next);
      argc++;
    }
    array = A_MALLOC(sizeof(CRPN *) * argc, NULL);
    argc = 0;
    d = r;
    while (IsCompare(d->type)) {
      array[argc++] = d->base.next;
      PushTmpDepthFirst(cctrl, d->base.next, 1);
      d = ICFwd(d->base.next);
    }
    array[argc++] = d;
    PushTmpDepthFirst(cctrl, array[argc - 1], 1);
    while (argc--)
      PopTmp(cctrl, array[argc]);
    A_FREE(array);
    goto fin;
    break;
  case IC_BT:
  case IC_BTC:
  case IC_BTR:
  case IC_BTS:
  case IC_LBTC:
  case IC_LBTR:
  case IC_LBTS:
  case IC_POW:
  binop:
    arg = ICArgN(r, 1);
    arg2 = ICArgN(r, 0);
    PushTmpDepthFirst(cctrl, arg, SpillsTmpRegs(arg2));
    PushTmpDepthFirst(cctrl, arg2, 0);
    PopTmp(cctrl, arg2);
    PopTmp(cctrl, arg);
    goto fin;
    break;
  case IC_ADD:
    goto binop;
    break;
  case IC_EQ:
    PushTmpDepthFirst(cctrl, ICArgN(r, 0), SpillsTmpRegs(ICArgN(r, 1)));
    PushTmpDepthFirst(cctrl, ICArgN(r, 1), 0);
    PopTmp(cctrl, ICArgN(r, 1));
    PopTmp(cctrl, ICArgN(r, 0));
    goto fin;
    break;
  case IC_SUB:
    goto binop;
    break;
  case IC_DIV:
    goto binop;
    break;
  case IC_MUL:
    goto binop;
    break;
  case IC_DEREF:
    orig = r;
    tmp = 0;
    arg = r->base.next;
    // You dont derefernece a func pointer
    if (ModeIsDerefToROff(r) && r->raw_type != RT_FUNC) {
      arg = r->base.next;
      arg2 = ICArgN(arg, 0);
      if (IsConst(arg2)) {
        arg2 = ICArgN(arg, 1);
        PushTmpDepthFirst(cctrl, arg2, 0);
        PopTmp(cctrl, arg2);
        goto fin;
      } else {
        // It's either r+const or const+r
        arg2 = ICArgN(arg, 0);
        PushTmpDepthFirst(cctrl, arg2, 0);
        PopTmp(cctrl, arg2);
        goto fin;
      }
    }
  deref_norm:
    PushTmpDepthFirst(cctrl, r->base.next, 0);
    PopTmp(cctrl, r->base.next);
    goto fin;
    break;
  case IC_AND:
    goto binop;
    break;
  case IC_ADDR_OF:
    goto unop;
    break;
  case IC_XOR:
    goto binop;
    break;
  case IC_MOD:
    goto binop;
    break;
  case IC_OR:
    goto binop;
    break;
  case IC_LNOT:
    goto unop;
    break;
  case IC_BNOT:
    goto unop;
    break;
  case IC_POST_INC:
  post_op:
    //
    //  Doing R9=(*R9)++ is a bad idea as we need to keep R9's pointer for
    //  storing the result
    //
    //  MOV R9,[R9]
    //  INC R9
    //  MOV [R9], R9
    //
    //  I will Keep 2 differnet tmps ,no sharing result reigster with the source
    //  reigster
    arg = ICArgN(r, 0);
    if (!spilled)
      PushTmp(cctrl, r, NULL);
    else
      PushSpilledTmp(cctrl, r);
    PushTmpDepthFirst(cctrl, arg, 0);
    PopTmp(cctrl, arg);
    return 1;
    break;
  case IC_POST_DEC:
    goto post_op;
    break;
  case IC_PRE_INC:
    goto post_op;
    break;
  case IC_PRE_DEC:
    goto post_op;
    break;
  case IC_AND_AND:
  logical_binop:
    arg = ICArgN(r, 1);
    arg2 = ICArgN(r, 0);
    PushTmpDepthFirst(cctrl, arg, SpillsTmpRegs(arg2));
    PushTmpDepthFirst(cctrl, arg2, 0);
    PopTmp(cctrl, arg2);
    PopTmp(cctrl, arg);
    goto fin;
    break;
  case IC_OR_OR:
    goto logical_binop;
    break;
  case IC_XOR_XOR:
    goto logical_binop;
    break;
  case IC_EQ_EQ:
    goto binop;
    break;
  case IC_NE:
    goto binop;
    break;
  case IC_LSH:
    goto binop;
    break;
  case IC_RSH:
    goto binop;
  assign_xop:
    arg = ICArgN(r, 1);
    arg2 = ICArgN(r, 0);
    // Equals (when used with IC_DEREF) are weird,they will preserve the
    // IC_DEREF->next
    //  and the DEREF
    //  *(arg->next)+=arg2
    //  IS TRAETED THE SAME AS
    //  tmp=arg->next;
    //  *tmp=*tmp+arg2;
    //
    //  arg is same as *(arg->next)
    // WE WILL NEED TO KEEP arg->next loaded in a register and such
    if (arg->type == IC_DEREF) {
      if (SpillsTmpRegs(arg2))
        PushSpilledTmp(cctrl, arg);
      else
        PushTmp(cctrl, arg, NULL);
      PushTmpDepthFirst(cctrl, arg->base.next, SpillsTmpRegs(arg2));
    } else
      PushTmpDepthFirst(cctrl, arg, SpillsTmpRegs(arg2));
    PushTmpDepthFirst(cctrl, arg2, 0);
    PopTmp(cctrl, arg2);
    if (arg->type == IC_DEREF)
      PopTmp(cctrl, arg->base.next);
    PopTmp(cctrl, arg);
    goto fin;
    break;
  case IC_ADD_EQ:
    goto assign_xop;
    break;
  case IC_SUB_EQ:
    goto assign_xop;
    break;
  case IC_MUL_EQ:
    goto assign_xop;
    break;
  case IC_DIV_EQ:
    goto assign_xop;
    break;
  case IC_LSH_EQ:
    goto assign_xop;
    break;
  case IC_RSH_EQ:
    goto assign_xop;
    break;
  case IC_AND_EQ:
    goto assign_xop;
    break;
  case IC_OR_EQ:
    goto assign_xop;
    break;
  case IC_XOR_EQ:
    goto assign_xop;
    break;
  case IC_MOD_EQ:
    goto assign_xop;
    break;
  case IC_COMMA:
    PushTmpDepthFirst(cctrl, ICArgN(r, 1), 0);
    PopTmp(cctrl, ICArgN(r, 1));
    ICArgN(r, 1)->res.mode = MD_NULL;
    PushTmpDepthFirst(cctrl, ICArgN(r, 0), 0);
    PopTmp(cctrl, ICArgN(r, 0));
    goto fin;
    break;
  case IC_TYPECAST:
    goto unop;
    break;
  case __IC_VARGS:
    if (!spilled)
      PushTmp(cctrl, r, NULL);
    else
      PushSpilledTmp(cctrl, r);
    for (a = 0; a != r->length; a++) {
      PushTmpDepthFirst(cctrl, ICArgN(r, a),
                        1); // TODO check if next vargs spill
    }
    while (a--)
      PopTmp(cctrl, ICArgN(r, a));
    break;
  case IC_RELOC:
    goto fin;
    break;
  default:
    return 0;
  fin:
    if (!spilled)
      PushTmp(cctrl, r, NULL);
    else
      PushSpilledTmp(cctrl, r);
  }
  return 1;
}

static int64_t DstRegAffectsMode(CICArg *d, CICArg *arg) {
  if (d->mode != MD_REG)
    return 0;
  int64_t r = d->reg;
  switch (arg->mode) {
    break;
  case MD_REG:
    return r == arg->reg;
    break;
  case MD_INDIR_REG:
    return r == arg->reg;
    break;
  }
  return 0;
}
// Arguments are called backwards
static int64_t ArgRPNMutatesArgumentRegister(CRPN *rpn, int64_t reg,
                                             int64_t is_farg) {
  int64_t i = rpn->length - (reg - 10) - 1;
  CRPN *arg;
  while (++i < rpn->length) {
    if (SpillsTmpRegs(arg = ICArgN(rpn, i)))
      return 1;
    // In Aiwnios RISC-V ABI,I pass all arguments in int registers
    if (is_farg) {
      if ((arg->changes_fregs | arg->changes_fregs2) & (1ull << reg)) {
        return 1;
      }
    } else if ((arg->changes_iregs | arg->changes_iregs2) & (1ull << reg)) {
      return 1;
    }
  }
  return 0;
}
static int64_t __ICFCallTOS(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                            int64_t code_off) {
  int64_t i, has_vargs = 0;
  CICArg tmp = {0}, tmp2 = {0};
  CRPN *rpn2, *arg;
  int64_t to_pop = rpn->length * 8, to_pop2 = to_pop, ptr = to_pop,
          vargs_pop = 0;
  void *fptr;
// Here's Nroots deal. Things that wont change"IC_IREG,IC_FREG,IC_IMM_X64" will
// be passed later. Other arguments will be passed to a temporary area on the
// stack 21.,this makes passing "spilled" arguments easier
#define WONT_CHANGE(t)                                                         \
  ((t) == IC_IREG || (t) == IC_FREG || (t) == IC_I64 || (t) == IC_F64 ||       \
   (t) == IC_BASE_PTR)
#define BEFORE_CALL                                                            \
  {                                                                            \
    CRPN *rpn2;                                                                \
    int64_t i;                                                                 \
    for (i = (rpn->length > 8 ? 8 : rpn->length) - 1; i >= 0; i--) {           \
      rpn2 = ICArgN(rpn, rpn->length - i - 1); /* REVBERSE polish notation */  \
      tmp.mode = MD_REG;                                                       \
      tmp.raw_type = rpn2->res.raw_type;                                       \
      tmp.reg = 10 + i;                                                        \
      arg = ICArgN(rpn2, 0);                                                   \
      if (rpn2->type == IC_ADDR_OF && arg->type == IC_BASE_PTR) {              \
        if (Is12Bit(-arg->integer)) {                                          \
          AIWNIOS_ADD_CODE(RISCV_ADDI(10 + i, RISCV_REG_FP, -arg->integer));   \
        } else {                                                               \
          code_off = __ICMoveI64(cctrl, 10 + i, -arg->integer, bin, code_off); \
          AIWNIOS_ADD_CODE(RISCV_ADD(10 + i, RISCV_REG_FP, 10 + i));           \
        }                                                                      \
      } else if (WONT_CHANGE(rpn2->type)) {                                    \
        switch (rpn2->type) {                                                  \
        case IC_IREG:                                                          \
        case IC_FREG:                                                          \
          tmp2.mode = MD_REG;                                                  \
          tmp2.raw_type = rpn2->res.raw_type;                                  \
          tmp2.reg = rpn2->integer;                                            \
          break;                                                               \
        case IC_I64:                                                           \
          tmp2.mode = MD_I64;                                                  \
          tmp2.raw_type = RT_I64i;                                             \
          tmp2.integer = rpn2->integer;                                        \
          break;                                                               \
        case IC_BASE_PTR:                                                      \
          tmp2.mode = MD_FRAME;                                                \
          tmp2.off = rpn2->integer;                                            \
          tmp2.raw_type = tmp.raw_type;                                        \
          break;                                                               \
        case IC_F64:                                                           \
          tmp2.mode = MD_F64;                                                  \
          tmp2.raw_type = RT_F64;                                              \
          tmp2.flt = rpn2->flt;                                                \
          break;                                                               \
        }                                                                      \
        code_off = ICMov(cctrl, &tmp, &tmp2, bin, code_off);                   \
      } else {                                                                 \
        code_off = ICMov(cctrl, &tmp, &rpn2->res, bin,                         \
                         code_off); /* In PushSpilledTmp I spilled them*/      \
      }                                                                        \
      if (tmp.raw_type == RT_F64) {                                            \
        AIWNIOS_ADD_CODE(RISCV_FMV_X_D(10 + i, 10 + i));                       \
      }                                                                        \
      \		 
                                                                                                                                            \
    }                                                                          \
  }
  int64_t mutated = 0;
  for (i = 0; i < rpn->length; i++) {
    rpn2 = ICArgN(rpn, i);
    arg = ICArgN(rpn2, 0);
    if (rpn2->type == __IC_VARGS) {
      to_pop -= 8; // We dont count argv
      ptr -= 8;
      has_vargs = 1;
      vargs_pop = rpn2->length * 8;
      code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
    } else if (!WONT_CHANGE(rpn2->type)) {
      // See BEFORE_CALL
      if (rpn2->type == IC_ADDR_OF && arg->type == IC_BASE_PTR &&
          rpn->length - i - 1 < 8) {
        rpn2->res.mode = MD_REG;
        rpn2->res.raw_type = RT_I64i;
        rpn2->res.reg = 10 + rpn->length - i - 1;
        continue;
      }
      if (!ArgRPNMutatesArgumentRegister(rpn, rpn->length - i - 1 + 10,
                                         rpn2->res.raw_type == RT_F64) &&
          rpn->length - i - 1 < 8) {
        // Favor storing in argument registers if they arent changed
        rpn2->res.mode = MD_REG;
        rpn2->res.raw_type = rpn2->res.raw_type;
        rpn2->res.reg = 10 + rpn->length - i - 1;
        code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
      } else
        code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
    }
  }
  if (to_pop >= 8 * 8)
    to_pop -= 8 * 8;
  else
    to_pop = 0;
  if (to_pop)
    AIWNIOS_ADD_CODE(RISCV_ADDI(RISCV_REG_SP, RISCV_REG_SP, -to_pop));
  ptr = to_pop;
  for (i = 0; i < rpn->length; i++) {
    rpn2 = ICArgN(rpn, i);
    if (rpn2->type == __IC_VARGS) {
    } else if (rpn->length - 1 - i >= 8) {
      ptr -= 8; // Arguments are reversed
      tmp.mode = MD_INDIR_REG;
      tmp.reg = RISCV_REG_SP;
      tmp.off = ptr;
      // PROMOTE to 64bit
      tmp.raw_type = rpn2->res.raw_type == RT_F64 ? RT_F64 : RT_I64i;
      code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
      code_off = ICMov(cctrl, &tmp, &rpn2->res, bin, code_off);
    }
  }
  BEFORE_CALL;
  rpn2 = ICArgN(rpn, rpn->length);
  if (rpn2->type == IC_SHORT_ADDR) {
    AIWNIOS_ADD_CODE(RISCV_AUIPC(5, 1 << 12));
    AIWNIOS_ADD_CODE(RISCV_JALR(1, 5, 0));
    if (bin)
      CodeMiscAddRef(rpn2->code_misc, bin + code_off - 8);
    goto after_call;
  }
  if (rpn2->type == IC_GLOBAL) {
    if (rpn2->global_var->base.type & HTT_FUN) {
      fptr = ((CHashFun *)rpn2->global_var)->fun_ptr;
      if (!fptr || fptr == &DoNothing)
        goto defacto;
    use_fptr:;
      int64_t old_code_off = code_off;
      int64_t idx = (int64_t)fptr - (int64_t)(bin + code_off);
      int64_t low12 = idx - (idx & ~((1 << 12) - 1));
      code_off = old_code_off;
      if (!Is32Bit(idx))
        goto defacto;
      if (Is12Bit(low12)) { /*Chekc for bit 12 being set*/
        AIWNIOS_ADD_CODE(RISCV_AUIPC(5, idx >> 12));
        AIWNIOS_ADD_CODE(RISCV_JALR(1, 5, low12));
      } else {
        AIWNIOS_ADD_CODE(RISCV_AUIPC(5, (idx >> 12) + 1));
        AIWNIOS_ADD_CODE(RISCV_JALR(1, 5, low12));
      }
    }
  } else {
  defacto:
    rpn2->res.raw_type = RT_PTR; // Not set for some reason
    if (rpn2->type == IC_RELOC) {
      AIWNIOS_ADD_CODE(RISCV_AUIPC(5, 0));
      AIWNIOS_ADD_CODE(RISCV_LD(5, 5, 0));
      if (bin)
        CodeMiscAddRef(rpn2->code_misc, bin + code_off - 8);
      AIWNIOS_ADD_CODE(RISCV_JALR(1, 5, 0));
    } else if (rpn2->type == IC_I64 && bin) {
      fptr = rpn2->integer;
      // Avoid infitite loop as above we go to defacto if not 32bit
      if (Is32Bit((char *)fptr - (char *)(bin + code_off)))
        goto use_fptr;
      goto dft;
    } else {
    dft:
      code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &rpn2->res, RT_PTR, 5, bin, code_off);
      if (rpn2->res.reg >= 10 && rpn2->res.reg - 10 <= 7) {
        tmp.reg = 5;
        tmp.mode = MD_REG;
        tmp.raw_type = RT_I64i;
        code_off = ICMov(cctrl, &tmp, &rpn2->res, bin, code_off);
        rpn2->res = tmp;
      }
      AIWNIOS_ADD_CODE(RISCV_JALR(1, rpn2->res.reg, 0));
    }
  }
after_call:
  if (to_pop + vargs_pop)
    AIWNIOS_ADD_CODE(
        RISCV_ADDI(RISCV_REG_SP, RISCV_REG_SP, to_pop + vargs_pop));
  if (rpn->raw_type != RT_U0 && rpn->res.mode != MD_NULL) {
    tmp.reg = RISCV_IRET;
    tmp.mode = MD_REG;
    tmp.raw_type =
        rpn->raw_type == RT_F64 ? RT_F64 : RT_I64i; // Promote to 64bits
    rpn->tmp_res = tmp;
    if (rpn->res.keep_in_tmp)
      rpn->res = tmp;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
  }
  return code_off;
}
int64_t ModeEqual(CICArg *dst, CICArg *src) {
  if (dst->raw_type == src->raw_type) {
    if (dst->mode == src->mode) {
      switch (dst->mode) {
        break;
      case MD_STATIC:
        if (dst->off == src->off)
          return 1;
        break;
      case MD_PTR:
        if (dst->off == src->off)
          return 1;
        break;
      case MD_REG:
        if (dst->reg == src->reg)
          return 1;
        break;
      case MD_FRAME:
        if (dst->off == src->off)
          return 1;
        break;
      case MD_INDIR_REG:
        if (dst->off == src->off && dst->reg == src->reg)
          return 1;
      }
    }
  } else if ((dst->raw_type == RT_F64) == (src->raw_type == RT_F64))
    return dst->mode == MD_REG && src->mode == MD_REG &&
           dst->reg == src->reg; // All Registers are promoted to 64bit
  return 0;
}
int64_t ICMov(CCmpCtrl *cctrl, CICArg *dst, CICArg *src, char *bin,
              int64_t code_off) {
  int64_t use_reg, use_reg2, restore_from_tmp = 0, indir_off = 0,
                             indir_off2 = 0, opc;
  CICArg tmp = {0}, tmp2 = {0}, *new;
  assert(src->mode > 0 || src->mode == -1);
  assert(dst->mode > 0 || dst->mode == -1);
  if (src->mode == 0)
    abort();
  if (dst->mode == MD_NULL)
    goto ret;
  if (ModeEqual(dst, src))
    goto ret;
  switch (dst->mode) {
    break;
  case MD_CODE_MISC_PTR:
    if (src->mode == MD_REG &&
        ((src->raw_type == RT_F64) == (dst->raw_type == RT_F64))) {
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, RISCV_PTR_TMP), 0));
      switch (dst->raw_type) {
        break;
      case RT_U8i:
      case RT_I8i:
        AIWNIOS_ADD_CODE(RISCV_SB(src->reg, RISCV_PTR_TMP, 0));
        break;
      case RT_U16i:
      case RT_I16i:
        AIWNIOS_ADD_CODE(RISCV_SH(src->reg, RISCV_PTR_TMP, 0));
        break;
      case RT_U32i:
      case RT_I32i:
        AIWNIOS_ADD_CODE(RISCV_SW(src->reg, RISCV_PTR_TMP, 0));
        break;
      case RT_PTR:
      case RT_U64i:
      case RT_I64i:
      case RT_FUNC:
        AIWNIOS_ADD_CODE(RISCV_SD(src->reg, RISCV_PTR_TMP, 0));
        break;
      case RT_F64:
        AIWNIOS_ADD_CODE(RISCV_FSD(src->reg, RISCV_PTR_TMP, 0));
        break;
      default:
        abort();
      }
      if (bin)
        CodeMiscAddRef(dst->code_misc, bin + code_off - 8);
      goto ret;
    }
    goto dft;
    break;
  case MD_STATIC:
    if ((src->raw_type == RT_F64) == (dst->raw_type == RT_F64) &&
        src->mode == MD_REG) {
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, RISCV_PTR_TMP), 0));
      AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, RISCV_PTR_TMP), RISCV_PTR_TMP,
                                  0)); // Standard relocation opcods
      if (bin)
        CodeMiscAddRef(cctrl->statics_label, bin + code_off - 8)->offset =
            dst->off;
      switch (dst->raw_type) {
        break;
      case RT_U8i:
      case RT_I8i:
        AIWNIOS_ADD_CODE(RISCV_SB(src->reg, RISCV_PTR_TMP, 0));
        break;
      case RT_U16i:
      case RT_I16i:
        AIWNIOS_ADD_CODE(RISCV_SH(src->reg, RISCV_PTR_TMP, 0));
        break;
      case RT_U32i:
      case RT_I32i:
        AIWNIOS_ADD_CODE(RISCV_SW(src->reg, RISCV_PTR_TMP, 0));
        break;
      case RT_PTR:
      case RT_U64i:
      case RT_I64i:
      case RT_FUNC:
        AIWNIOS_ADD_CODE(RISCV_SD(src->reg, RISCV_PTR_TMP, 0));
        break;
      case RT_F64:
        AIWNIOS_ADD_CODE(RISCV_FSD(src->reg, RISCV_PTR_TMP, 0));
        break;
      default:
        abort();
      }
      goto ret;
    }
    goto dft;
    break;
  case MD_INDIR_REG:
    use_reg2 = dst->reg;
    indir_off = dst->off;
  indir_r2:
    if ((src->raw_type == RT_F64) == (dst->raw_type == RT_F64) &&
        src->mode == MD_REG) {
      use_reg = src->reg;
    } else {
    dft:
      tmp.raw_type = src->raw_type;
      if (dst->raw_type != RT_F64)
        use_reg = tmp.reg = RISCV_IPOOP1;
      else
        use_reg = tmp.reg = RISCV_FRET;
      if (dst->mode == MD_REG)
        use_reg = tmp.reg = dst->reg;
      tmp.mode = MD_REG;
      if (dst->raw_type == RT_F64 && src->raw_type != dst->raw_type) {
        tmp.raw_type = RT_F64;
        if (src->mode == MD_REG) {
          AIWNIOS_ADD_CODE(RISCV_FCVT_D_L(use_reg, src->reg));
        } else {
          tmp2 = tmp;
          tmp2.mode = MD_REG;
          tmp2.reg = 5;
          tmp2.raw_type = src->raw_type;
          code_off = ICMov(cctrl, &tmp2, src, bin, code_off);
          code_off = ICMov(cctrl, &tmp, &tmp2, bin, code_off);
        }
      } else if (src->raw_type == RT_F64 && src->raw_type != dst->raw_type) {
        tmp.raw_type = RT_I64i;
        if (src->mode == MD_REG) {
          AIWNIOS_ADD_CODE(RISCV_FCVT_L_D(MFR(cctrl, use_reg), src->reg));

        } else {
          tmp2 = tmp;
          tmp2.mode = MD_REG;
          tmp2.reg = RISCV_FRET;
          tmp2.raw_type = src->raw_type;
          code_off = ICMov(cctrl, &tmp2, src, bin, code_off);
          code_off = ICMov(cctrl, &tmp, &tmp2, bin, code_off);
        }
      } else
        code_off = ICMov(cctrl, &tmp, src, bin, code_off);
      code_off = ICMov(cctrl, dst, &tmp, bin, code_off);
      goto ret;
    }
    goto store_r2;
    break;
  case MD_PTR:
    if (src->mode != MD_REG ||
        ((src->raw_type == RT_F64) != (dst->raw_type == RT_F64)))
      goto dft;
    use_reg = src->reg;
    use_reg2 = RISCV_IPOOP1;
    code_off = __ICMoveI64(cctrl, use_reg2, dst->off, bin, code_off);
    indir_off = 0;
    goto store_r2;
  store_r2:
    if (!Is12Bit(indir_off)) {
      code_off = __ICMoveI64(cctrl, RISCV_PTR_TMP, indir_off, bin, code_off);
      AIWNIOS_ADD_CODE(
          RISCV_ADD(MIR(cctrl, RISCV_PTR_TMP), RISCV_PTR_TMP, use_reg2));
      use_reg2 = RISCV_PTR_TMP;
      indir_off = 0;
    }
    switch (dst->raw_type) {
    case RT_U0:
      break;
    case RT_F64:
      AIWNIOS_ADD_CODE(RISCV_FSD(use_reg, use_reg2, indir_off));
      break;
    case RT_U8i:
    case RT_I8i:
      AIWNIOS_ADD_CODE(RISCV_SB(use_reg, use_reg2, indir_off));
      break;
    case RT_U16i:
    case RT_I16i:
      AIWNIOS_ADD_CODE(RISCV_SH(use_reg, use_reg2, indir_off));
      break;
    case RT_U32i:
    case RT_I32i:
      AIWNIOS_ADD_CODE(RISCV_SW(use_reg, use_reg2, indir_off));
      break;
    case RT_U64i:
    case RT_I64i:
    case RT_FUNC: // func ptr
    case RT_PTR:
      AIWNIOS_ADD_CODE(RISCV_SD(use_reg, use_reg2, indir_off));
      break;
    default:
      abort();
    }
    break;
  case MD_FRAME:
    use_reg2 = RISCV_REG_FP;
    indir_off = -dst->off;
    goto indir_r2;
    break;
  case MD_REG:
    use_reg = dst->reg;
    if (src->mode == MD_FRAME) {
      if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
        use_reg2 = RISCV_REG_FP;
        indir_off = -src->off;
        goto load_r2;
      } else if ((src->raw_type == RT_F64) ^ (RT_F64 == dst->raw_type)) {
        goto dft;
      }
      use_reg2 = RISCV_REG_FP;
      indir_off = -src->off;
      goto load_r2;
    } else if (src->mode == MD_CODE_MISC_PTR) {
      if ((src->raw_type == RT_F64) != (dst->raw_type == RT_F64))
        goto dft;
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, RISCV_PTR_TMP), 0));
      AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, RISCV_PTR_TMP), RISCV_PTR_TMP, 0));
      if (bin)
        CodeMiscAddRef(src->code_misc, bin + code_off - 8);
      indir_off = 0;
      use_reg2 = RISCV_PTR_TMP;
      goto load_r2;
    } else if (src->mode == MD_REG) {
      if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
        AIWNIOS_ADD_CODE(RISCV_SGNJ(MIR(cctrl, use_reg), src->reg, src->reg))
      } else if (src->raw_type != RT_F64 && dst->raw_type != RT_F64) {
        AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, use_reg), src->reg, 0))
      } else if (src->raw_type == RT_F64 && dst->raw_type != RT_F64) {
        goto dft;
      } else if (src->raw_type != RT_F64 && dst->raw_type == RT_F64) {
        goto dft;
      }
    } else if (src->mode == MD_PTR) {
      if ((src->raw_type == RT_F64) == (RT_F64 == dst->raw_type)) {
        use_reg2 = RISCV_PTR_TMP;
        code_off = __ICMoveI64(cctrl, use_reg2, src->off, bin, code_off);
        goto load_r2;
      } else {
        goto dft;
      }
    load_r2:
      if (!Is12Bit(indir_off)) {
        code_off = __ICMoveI64(cctrl, RISCV_PTR_TMP, indir_off, bin, code_off);
        AIWNIOS_ADD_CODE(
            RISCV_ADD(MIR(cctrl, RISCV_PTR_TMP), RISCV_PTR_TMP, use_reg2));
        use_reg2 = RISCV_PTR_TMP;
        indir_off = 0;
      }
      switch (src->raw_type) {
      case RT_U0:
        break;
      case RT_U8i:
        AIWNIOS_ADD_CODE(RISCV_LBU(MIR(cctrl, dst->reg), use_reg2, indir_off));
        break;
      case RT_I8i:
        AIWNIOS_ADD_CODE(RISCV_LB(MIR(cctrl, dst->reg), use_reg2, indir_off));
        break;
      case RT_U16i:
        AIWNIOS_ADD_CODE(RISCV_LHU(MIR(cctrl, dst->reg), use_reg2, indir_off));
        break;
      case RT_I16i:
        AIWNIOS_ADD_CODE(RISCV_LH(MIR(cctrl, dst->reg), use_reg2, indir_off));
        break;
      case RT_U32i:
        AIWNIOS_ADD_CODE(RISCV_LWU(MIR(cctrl, dst->reg), use_reg2, indir_off));
        break;
      case RT_I32i:
        AIWNIOS_ADD_CODE(RISCV_LW(MIR(cctrl, dst->reg), use_reg2, indir_off));
        break;
      case RT_U64i:
      case RT_PTR:
      case RT_FUNC:
      case RT_I64i:
        AIWNIOS_ADD_CODE(RISCV_LD(MIR(cctrl, dst->reg), use_reg2, indir_off));
        break;
      case RT_F64:
        AIWNIOS_ADD_CODE(RISCV_FLD(MFR(cctrl, dst->reg), use_reg2, indir_off));
        break;
      default:
        abort();
      }
    } else if (src->mode == MD_INDIR_REG) {
      use_reg2 = src->reg;
      indir_off = src->off;
      if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
        use_reg2 = src->reg;
      } else if ((src->raw_type == RT_F64) ^ (RT_F64 == dst->raw_type)) {
        goto dft;
      }
      goto load_r2;
    } else if (src->mode == MD_I64 && dst->raw_type != RT_F64) {
      code_off = __ICMoveI64(cctrl, dst->reg, src->integer, bin, code_off);
    } else if (src->mode == MD_F64 && dst->raw_type == RT_F64) {
      code_off = __ICMoveF64(cctrl, dst->reg, src->flt, bin, code_off);
    } else if (src->mode == MD_I64 && dst->raw_type == RT_F64) {
      code_off = __ICMoveF64(cctrl, dst->reg, src->integer, bin, code_off);
    } else if (src->mode == MD_F64 && dst->raw_type != RT_F64) {
      // Nroot here,be sure to do int64_t as we take uint64_t on riscV,which
      // bugs out with negative values
      code_off = __ICMoveI64(cctrl, dst->reg, (int64_t)src->flt, bin, code_off);
    } else if (src->mode == MD_STATIC) {
      if (dst->mode == MD_REG &&
          (dst->raw_type == RT_F64) == (src->raw_type == RT_F64)) {
        AIWNIOS_ADD_CODE(RISCV_AUIPC(RISCV_PTR_TMP, 0));
        switch (dst->raw_type) {
          break;
        case RT_U8i:
        case RT_I8i:
          AIWNIOS_ADD_CODE(RISCV_LB(MIR(cctrl, dst->reg), RISCV_PTR_TMP, 0));
          break;
        case RT_U16i:
          AIWNIOS_ADD_CODE(RISCV_LHU(MIR(cctrl, dst->reg), RISCV_PTR_TMP, 0));
          break;
        case RT_I16i:
          AIWNIOS_ADD_CODE(RISCV_LH(MIR(cctrl, dst->reg), RISCV_PTR_TMP, 0));
          break;
        case RT_U32i:
          AIWNIOS_ADD_CODE(RISCV_LWU(MIR(cctrl, dst->reg), RISCV_PTR_TMP, 0));
          break;
        case RT_I32i:
          AIWNIOS_ADD_CODE(RISCV_LW(MIR(cctrl, dst->reg), RISCV_PTR_TMP, 0));
          break;
        case RT_PTR:
        case RT_U64i:
        case RT_I64i:
        case RT_FUNC:
          AIWNIOS_ADD_CODE(RISCV_LD(MIR(cctrl, dst->reg), RISCV_PTR_TMP, 0));
          break;
        case RT_F64:
          AIWNIOS_ADD_CODE(RISCV_FLD(MFR(cctrl, dst->reg), RISCV_PTR_TMP, 0));
          break;
        default:
          abort();
        }
        if (bin)
          CodeMiscAddRef(cctrl->statics_label, bin + code_off - 8)->offset =
              src->off;
      } else
        goto dft;
      goto ret;
    } else
      goto dft;
    break;
  case MD_I64:
  case MD_F64:
    break;
  default:
    abort();
  }
ret:
  return code_off;
}

static int64_t DerefToICArg(CCmpCtrl *cctrl, CICArg *res, CRPN *rpn,
                            int64_t base_reg_fallback, char *bin,
                            int64_t code_off) {
  if (rpn->type == IC_DEREF) {
    if (ModeIsDerefToROff(rpn))
      goto ent;
    *res = rpn->res;
    rpn = rpn->base.next;
    code_off = __OptPassFinal(cctrl, rpn, bin, code_off);
    code_off = PutICArgIntoReg(cctrl, &rpn->res, RT_PTR, base_reg_fallback, bin,
                               code_off);
    res->mode = MD_INDIR_REG;
    res->reg = rpn->res.reg;
    res->off = 0;
    return code_off;
  }
  if (!ModeIsDerefToROff(rpn)) {
    code_off = __OptPassFinal(cctrl, rpn, bin, code_off);
    if (rpn->res.mode == MD_FRAME) {
      res->raw_type = rpn->raw_type;
      res->mode = MD_INDIR_REG;
      res->reg = RISCV_REG_FP;
      res->off = -rpn->res.off;
      return code_off;
    }
    *res = rpn->res;
    return code_off;
  }
ent:
  int64_t r = rpn->raw_type, rsz, off, mul, lea = 0, reg;
  CRPN *next = rpn->base.next, *next2, *next3, *next4, *tmp;
  next2 = ICArgN(next, 0);
  next3 = ICArgN(next, 1);
  if (IsConst(next2)) {
    code_off = __OptPassFinal(cctrl, next3, bin, code_off);
    code_off = PutICArgIntoReg(cctrl, &next3->res, RT_PTR, base_reg_fallback,
                               bin, code_off);
    off = ConstVal(next2);
    reg = next3->res.reg;
  } else {
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);
    code_off = PutICArgIntoReg(cctrl, &next2->res, RT_PTR, base_reg_fallback,
                               bin, code_off);
    off = ConstVal(next3);
    reg = next2->res.reg;
  }
  res->mode = MD_INDIR_REG;
  res->reg = reg;
  res->off = off;
  res->raw_type = r;
  return code_off;
}

#define TYPECAST_ASSIGN_BEGIN(DST, SRC)                                        \
  {                                                                            \
    int64_t pop = 0, pop2 = 0;                                                 \
    CICArg _orig = {0};                                                        \
    CRPN *_tc = DST;                                                           \
    CRPN *SRC2 = SRC;                                                          \
    DST = DST->base.next;                                                      \
    if (DST->type == IC_DEREF) {                                               \
      code_off = DerefToICArg(cctrl, &_orig, DST, 1, bin, code_off);           \
      DST->res = _orig;                                                        \
      DST->raw_type = _tc->raw_type;                                           \
    } else if ((DST->raw_type == RT_F64) ^ (SRC2->raw_type == RT_F64)) {       \
      pop2 = pop = 1;                                                          \
      _orig = DST->res;                                                        \
      code_off =                                                               \
          PutICArgIntoReg(cctrl, &DST->res, DST->raw_type, 1, bin, code_off);  \
      code_off = PushToStack(cctrl, &DST->res, bin, code_off);                 \
      DST->res.mode = MD_INDIR_REG;                                            \
      DST->res.off = 0;                                                        \
      DST->res.reg = RISCV_REG_SP;                                             \
      DST->res.raw_type = _tc->raw_type;                                       \
      DST->raw_type = _tc->raw_type;                                           \
    } else {                                                                   \
      pop2 = 1;                                                                \
      DST->res.raw_type = _tc->raw_type;                                       \
      DST->raw_type = _tc->raw_type;                                           \
    }
#define TYPECAST_ASSIGN_END(DST)                                               \
  if (pop)                                                                     \
    code_off = PopFromStack(cctrl, &_orig, bin, code_off);                     \
  }

static int64_t __SexyPreOp(CCmpCtrl *cctrl, CRPN *rpn,
                           int64_t (*i_imm)(char *, int64_t, int64_t),
                           int64_t (*ireg)(char *, int64_t, int64_t),
                           int64_t (*freg)(char *, int64_t, int64_t), char *bin,
                           int64_t code_off) {
  int64_t code, sz;
  CRPN *next = rpn->base.next, *tc;
  CICArg derefed = {0}, tmp = {0}, tmp2 = {0};
  if (next->type == IC_TYPECAST) {
    TYPECAST_ASSIGN_BEGIN(next, next);
    switch (next->raw_type) {
    case RT_I8i:
    case RT_U8i:
      sz = 1;
      break;
    case RT_I16i:
    case RT_U16i:
      sz = 2;
      break;
    case RT_I32i:
    case RT_U32i:
      sz = 4;
      break;
    default:
      sz = 8;
      break;
    }
    if (next->raw_type == RT_F64) {
      code_off =
          DerefToICArg(cctrl, &derefed, next, RISCV_IPOOP2, bin, code_off);
      tmp = derefed; //==RT_F64
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, RISCV_FPOOP1, bin,
                                 code_off);
      code_off = __ICMoveF64(cctrl, RISCV_FRET, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(freg(MFR(cctrl, tmp.reg), tmp.reg, RISCV_FRET));
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else {
      CICArg orig = {0};
      code_off =
          DerefToICArg(cctrl, &next->res, next, RISCV_IPOOP1, bin, code_off);
      orig = next->res;
      tmp = next->res;
      code_off =
          PutICArgIntoReg(cctrl, &tmp, RT_I64i, RISCV_IPOOP1, bin, code_off);
      if (Is12Bit(rpn->integer)) {
        AIWNIOS_ADD_CODE(i_imm(MIR(cctrl, tmp.reg), tmp.reg, rpn->integer));
      } else {
        code_off = __ICMoveI64(cctrl, 5, rpn->integer, bin, code_off);
        AIWNIOS_ADD_CODE(ireg(MIR(cctrl, tmp.reg), tmp.reg, 5));
      }
      code_off = ICMov(cctrl, &orig, &tmp, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    TYPECAST_ASSIGN_END(next);
  } else {
    if (next->raw_type == RT_F64) {
      code_off =
          DerefToICArg(cctrl, &derefed, next, RISCV_FPOOP2, bin, code_off);
      tmp = derefed; //==RT_F64
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, RISCV_FPOOP1, bin,
                                 code_off);
      code_off = __ICMoveF64(cctrl, RISCV_FRET, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(freg(MFR(cctrl, tmp.reg), tmp.reg, RISCV_FRET));
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else {
      CICArg orig = {0};
      code_off =
          DerefToICArg(cctrl, &next->res, next, RISCV_IPOOP2, bin, code_off);
      orig = next->res;
      tmp = next->res;
      code_off =
          PutICArgIntoReg(cctrl, &tmp, RT_I64i, RISCV_IPOOP1, bin, code_off);
      if (Is12Bit(rpn->integer)) {
        AIWNIOS_ADD_CODE(i_imm(MIR(cctrl, tmp.reg), tmp.reg, rpn->integer));
      } else {
        code_off = __ICMoveI64(cctrl, 5, rpn->integer, bin, code_off);
        AIWNIOS_ADD_CODE(ireg(MIR(cctrl, tmp.reg), tmp.reg, 5));
      }
      code_off = ICMov(cctrl, &orig, &tmp, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
  }
  return code_off;
}
static int64_t __SexyAssignOp(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                              int64_t code_off) {
  CRPN *next = ICArgN(rpn, 1), *next2 = ICArgN(rpn, 0), *next_next;
  CICArg old = rpn->res, dummy = {0}, old2 = {0};
enter:;
  cctrl->backend_user_data9 = 1;
  int64_t old_type = rpn->type, od = cctrl->backend_user_data2,
          next_old = next->type, decr = 0, old_next2 = next2->type, pop = 0,
          use_tc = 0, use_raw_type = rpn->raw_type, use_f64 = 0,
          old_raw_type = rpn->raw_type, old_flags = next->flags,
          old_flags2 = next2->flags, set_flags = 0, old_flags3;
  if (next->type == IC_TYPECAST) {
    use_raw_type = next->raw_type;
    next = ICArgN(next, 0);
    use_tc = 1;
    goto enter;
  }
  switch (rpn->type) {
    break;
  case IC_ADD: // Will do assign on NORMAL tooo
  case IC_ADD_EQ:
    // TODO Account for atomic/immediates
// PushTmp will set next->res
#define XXX_EQ_OP(op)                                                          \
  rpn->type = op;                                                              \
  use_f64 = next2->raw_type == RT_F64 || next->raw_type == RT_F64;             \
  switch (next->type) {                                                        \
  case IC_IREG:                                                                \
  case IC_FREG:                                                                \
  case IC_BASE_PTR:                                                            \
    rpn->res = next->res;                                                      \
    dummy = next->res;                                                         \
    if (use_f64) {                                                             \
      if (rpn->res.raw_type != RT_F64) {                                       \
        rpn->res.raw_type = RT_F64;                                            \
        rpn->res.reg = RISCV_FRET;                                             \
        rpn->res.mode = MD_REG;                                                \
      }                                                                        \
      rpn->raw_type = RT_F64;                                                  \
    }                                                                          \
    code_off = __OptPassFinal(cctrl, rpn, bin, code_off);                      \
    rpn->raw_type = old_raw_type;                                              \
    code_off = ICMov(cctrl, &dummy, &rpn->res, bin, code_off);                 \
    code_off = ICMov(cctrl, &old, &rpn->res, bin, code_off);                   \
    rpn->res = old;                                                            \
    break;                                                                     \
  case IC_DEREF:                                                               \
    code_off = __OptPassFinal(cctrl, next->base.next, bin, code_off);          \
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);                    \
    dummy = ((CRPN *)next->base.next)->res;                                    \
    code_off = PutICArgIntoReg(                                                \
        cctrl, &next2->res, use_f64 ? RT_F64 : next2->res.raw_type,            \
        use_f64 ? RISCV_FPOOP2 : RISCV_IPOOP2, bin, code_off);                 \
    code_off = PutICArgIntoReg(cctrl, &dummy, RT_U64i, RISCV_ASSIGN_ADDR_TMP,  \
                               bin, code_off);                                 \
    next->res.reg = dummy.reg;                                                 \
    next->res.mode = MD_INDIR_REG;                                             \
    next->res.off = 0;                                                         \
    old2 = next->res;                                                          \
    code_off = PutICArgIntoReg(                                                \
        cctrl, &next->res, use_f64 ? RT_F64 : next->res.raw_type,              \
        use_f64 ? RISCV_FPOOP1 : RISCV_IPOOP1, bin, code_off);                 \
    next->flags |= ICF_PRECOMPUTED;                                            \
    next2->flags |= ICF_PRECOMPUTED;                                           \
    rpn->res.mode = MD_REG;                                                    \
    rpn->res.reg = use_f64 ? RISCV_FASSIGN_TMP : RISCV_IASSIGN_TMP;            \
    if (use_f64)                                                               \
      rpn->raw_type = RT_F64;                                                  \
    rpn->res.raw_type = rpn->raw_type;                                         \
    code_off = __OptPassFinal(cctrl, rpn, bin, code_off);                      \
    set_flags = rpn->res.set_flags;                                            \
    rpn->raw_type = old_raw_type;                                              \
    next->flags &= ~ICF_PRECOMPUTED;                                           \
    next2->flags &= ~ICF_PRECOMPUTED;                                          \
    if (!use_tc)                                                               \
      old2.raw_type = next->raw_type;                                          \
    else                                                                       \
      old2.raw_type = use_raw_type;                                            \
    old2.off = 0;                                                              \
    code_off = ICMov(cctrl, &old2, &rpn->res, bin, code_off);                  \
    code_off = ICMov(cctrl, &old, &rpn->res, bin, code_off);                   \
    rpn->res = old;                                                            \
    next->flags = old_flags, next2->flags = old_flags2;                        \
    break;                                                                     \
  default:                                                                     \
    rpn->res = next->res;                                                      \
    if (use_f64) {                                                             \
      if (next->res.raw_type != RT_F64) {                                      \
        rpn->res.raw_type = RT_F64;                                            \
        rpn->res.reg = RISCV_FPOOP1;                                           \
        rpn->res.mode = MD_REG;                                                \
      }                                                                        \
      rpn->raw_type = RT_F64;                                                  \
    }                                                                          \
    code_off = __OptPassFinal(cctrl, rpn, bin, code_off);                      \
    set_flags = rpn->res.set_flags;                                            \
    rpn->raw_type = old_raw_type;                                              \
    code_off = ICMov(cctrl, &next->res, &rpn->res, bin, code_off);             \
    code_off = ICMov(cctrl, &old, &rpn->res, bin, code_off);                   \
  }                                                                            \
  rpn->type = old_type, rpn->res = old;                                        \
  next->type = next_old;                                                       \
  next2->type = old_next2;
    XXX_EQ_OP(IC_ADD);
    break;
  case IC_SUB:
  case IC_SUB_EQ:
    // Account for atomic/immediates TODO TODO TODO
    XXX_EQ_OP(IC_SUB);
    break;
  case IC_MUL:
  case IC_MUL_EQ:
    XXX_EQ_OP(IC_MUL);
    break;
  case IC_DIV:
  case IC_DIV_EQ:
    XXX_EQ_OP(IC_DIV);
    break;
  case IC_LSH:
  case IC_LSH_EQ:
    XXX_EQ_OP(IC_LSH);
    break;
  case IC_RSH:
  case IC_RSH_EQ:
    XXX_EQ_OP(IC_RSH);
    break;
  case IC_AND:
  case IC_AND_EQ:
    // TODO Account for atomic/immediates
    XXX_EQ_OP(IC_AND);
    break;
  case IC_OR:
  case IC_OR_EQ:
    // Account for atomic/immediates TODO
    XXX_EQ_OP(IC_OR);
    break;
  case IC_XOR:
  case IC_XOR_EQ:
    XXX_EQ_OP(IC_XOR);
    break;
  case IC_MOD:
  case IC_MOD_EQ:
    XXX_EQ_OP(IC_MOD);
  }
fin:
  if (set_flags)
    rpn->res.set_flags = 1;
  cctrl->backend_user_data9 = 0;
  return code_off;
}

static int64_t IsSavedIReg(int64_t r) {
  switch (r) {
  case 9:
  case 18 ... 27:
    return 1;
  }
  return 0;
}

// This is used for FuncProlog/Epilog. It is used for finding the non-violatle
// registers that must be saved and pushed to the stack
//
// to_push is a char[16] ,1 is push,0 is not push
//
// Returns number of pushed regs
//
static int64_t __FindPushedIRegs(CCmpCtrl *cctrl, char *to_push) {
  CMemberLst *lst;
  CRPN *r;
  int64_t cnt = 0;
  memset(to_push, 0, 32);
  if (!cctrl->cur_fun) {
    // Perhaps we are being called from HolyC and we aren't using a "cur_fun"
    for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
         r = r->base.next) {
      if (r->type == IC_IREG && r->raw_type != RT_F64) {
        if (!to_push[r->integer])
          if (IsSavedIReg(r->integer)) {
            to_push[r->integer] = 1;
            cnt++;
          }
      }
    }
    return cnt;
  }
  for (lst = cctrl->cur_fun->base.members_lst; lst; lst = lst->next) {
    if (lst->reg != REG_NONE && lst->member_class->raw_type != RT_F64) {
      assert(lst->reg >= 0);
      to_push[lst->reg] = 1;
      cnt++;
    }
  }
  return cnt;
}

static int64_t __FindPushedFRegs(CCmpCtrl *cctrl, char *to_push) {
  CMemberLst *lst;
  int64_t cnt = 0;
  memset(to_push, 0, 32);
  CRPN *r;
  // Already computed
  if (!cctrl->cur_fun) {
    // Perhaps we are being called from HolyC and we aren't using a "cur_fun"
    for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
         r = r->base.next) {
      if (r->type == IC_FREG && r->raw_type == RT_F64) {
        if (!to_push[r->integer])
          if (IsSavedFReg(r->integer)) {
            to_push[r->integer] = 1;
            Misc_Bts(&cctrl->backend_user_data8, r->integer);
            cnt++;
          }
      }
    }
    return cctrl->backend_user_data7 = cnt;
  }
  for (lst = cctrl->cur_fun->base.members_lst; lst; lst = lst->next) {
    if (lst->reg != REG_NONE && lst->member_class->raw_type == RT_F64) {
      assert(lst->reg >= 0);
      to_push[lst->reg] = 1;
      Misc_Bts(&cctrl->backend_user_data8, lst->reg);
      cnt++;
    }
  }
  return cctrl->backend_user_data7 = cnt;
}

// Add 1 in first bit for oppositive
#define RISCV_COND_A  0x10
#define RISCV_COND_AE 0x20
#define RISCV_COND_B  0x30
#define RISCV_COND_BE 0x40
#define RISCV_COND_E  0x60
#define RISCV_COND_Z  X86_COND_E
#define RISCV_COND_G  0x70
#define RISCV_COND_GE 0x80
#define RISCV_COND_L  0x90
#define RISCV_COND_LE 0xa0
#define RISCV_COND_S  0xb0
#define RISCV_COND_NS 0xb1
static int64_t RISCV_Jcc(int64_t cond, int64_t a, int64_t b, int64_t off) {
enter:
  switch (cond) {
  case RISCV_COND_A:
    return RISCV_BLTU(b, a, off);
  case RISCV_COND_AE:
    return RISCV_BGEU(a, b, off);
  case RISCV_COND_B:
    return RISCV_BLTU(a, b, off);
  case RISCV_COND_BE:
    return RISCV_BGEU(b, a, off);
  case RISCV_COND_E:
    return RISCV_BEQ(a, b, off);
  case RISCV_COND_G:
    return RISCV_BLT(b, a, off);
  case RISCV_COND_GE:
    return RISCV_BGE(a, b, off);
  case RISCV_COND_L:
    return RISCV_BLT(a, b, off);
  case RISCV_COND_LE:
    return RISCV_BGE(b, a, off);
  case RISCV_COND_A ^ 1:
    cond = RISCV_COND_BE;
    goto enter;
  case RISCV_COND_AE ^ 1:
    cond = RISCV_COND_B;
    goto enter;
  case RISCV_COND_B ^ 1:
    cond = RISCV_COND_AE;
    goto enter;
  case RISCV_COND_BE ^ 1:
    cond = RISCV_COND_A;
    goto enter;
  case RISCV_COND_E ^ 1:
    return RISCV_BNE(a, b, off);
  case RISCV_COND_G ^ 1:
    cond = RISCV_COND_LE;
    goto enter;
  case RISCV_COND_GE ^ 1:
    cond = RISCV_COND_L;
    goto enter;
  case RISCV_COND_L ^ 1:
    cond = RISCV_COND_GE;
    goto enter;
  case RISCV_COND_LE ^ 1:
    cond = RISCV_COND_G;
    goto enter;
  }
}
static int64_t FuncProlog(CCmpCtrl *cctrl, char *bin, int64_t code_off) {
  char push_ireg[32];
  char push_freg[32];
  char ilist[32];
  char flist[32];
  int64_t i, i2, i3, r, r2, ireg_arg_cnt, freg_arg_cnt, stk_arg_cnt, fsz, code,
      off;
  int64_t has_vargs = 0, arg_cnt = 0;
  CMemberLst *lst;
  CICArg fun_arg = {0}, write_to = {0}, stack = {0}, tmp = {0};
  CRPN *rpn, *arg;
  if (cctrl->cur_fun) {
    fsz = cctrl->cur_fun->base.sz + 16; //+16 for RBP/return address
    arg_cnt = cctrl->cur_fun->argc;
    if (cctrl->cur_fun->base.flags & CLSF_VARGS)
      arg_cnt--; // argv
  } else {
    fsz = 16;
    for (rpn = cctrl->code_ctrl->ir_code->next;
         rpn != cctrl->code_ctrl->ir_code; rpn = rpn->base.next) {
      switch (rpn->type) {
      case __IC_SET_FRAME_SIZE:
        fsz = rpn->integer;
        break;
      case IC_GET_VARGS_PTR:
        has_vargs = 1;
        break;
      case __IC_ARG:
        arg_cnt++;
        break;
      }
    }
  }
  // ALIGN TO 8
  if (fsz % 8)
    fsz += 8 - fsz % 8;
  int64_t to_push = (i2 = __FindPushedIRegs(cctrl, push_ireg)) * 8 +
                    (i3 = __FindPushedFRegs(cctrl, push_freg)) * 8 +
                    cctrl->backend_user_data0 + fsz + 16,
          old_regs_start;
  if (i2 % 2)
    to_push += 8; // We push a dummy register in a pair if not aligned to 2
  if (i3 % 2)
    to_push += 8; // Ditto

  if (to_push % 16)
    to_push += 8;
  cctrl->backend_user_data4 = fsz + 8;
  old_regs_start = fsz + cctrl->backend_user_data0 + 8;

  i2 = 0;
  for (i = 0; i != 32; i++)
    if (push_ireg[i])
      ilist[i2++] = i;
  if (i2 % 2)
    ilist[i2++] = 0;
  i3 = 0;
  for (i = 0; i != 32; i++)
    if (push_freg[i])
      flist[i3++] = i;
  if (i3 % 2)
    flist[i3++] = 1; // 0 is return register

  AIWNIOS_ADD_CODE(RISCV_ADDI(RISCV_REG_SP, RISCV_REG_SP, -16));
  AIWNIOS_ADD_CODE(RISCV_SD(1, RISCV_REG_SP, 8)); // 1 is return register
  AIWNIOS_ADD_CODE(RISCV_SD(RISCV_REG_FP, RISCV_REG_SP, 0));
  AIWNIOS_ADD_CODE(RISCV_ADDI(RISCV_REG_FP, RISCV_REG_SP, 16));

  //
  // Eariler in OptPass.c and AIWNIOS_CodeGen.HC,I made room for ra/s0 in frame
  //
  if (Is12Bit(-to_push)) {
    AIWNIOS_ADD_CODE(RISCV_ADDI(RISCV_REG_SP, RISCV_REG_SP, -to_push));
  } else {
    code_off = __ICMoveI64(cctrl, 5 /* t0 */, -to_push, bin, code_off);
    AIWNIOS_ADD_CODE(RISCV_ADD(RISCV_REG_SP, RISCV_REG_SP, 5 /* t0 */));
  }

  off = -old_regs_start;
  for (i = 0; i != i2; i++) {
    stack.mode = MD_INDIR_REG;
    stack.reg = RISCV_REG_FP;
    stack.raw_type = RT_I64i;
    stack.off = off;
    tmp.mode = MD_REG;
    tmp.reg = ilist[i];
    tmp.raw_type = RT_I64i;
    if (ilist[i])
      code_off = ICMov(cctrl, &stack, &tmp, bin, code_off);
    off -= 8;
  }
  for (i = 0; i != i3; i++) {
    stack.mode = MD_INDIR_REG;
    stack.reg = RISCV_REG_FP;
    stack.raw_type = RT_F64;
    stack.off = off;
    tmp.mode = MD_REG;
    tmp.reg = flist[i];
    tmp.raw_type = RT_F64;
    if (flist[i])
      code_off = ICMov(cctrl, &stack, &tmp, bin, code_off);
    off -= 8;
  }
  stk_arg_cnt = ireg_arg_cnt = freg_arg_cnt = 0;
  if (cctrl->cur_fun) {
    lst = cctrl->cur_fun->base.members_lst;
    for (i = 0; i != cctrl->cur_fun->argc; i++) {
      if (i <= 7) {
        fun_arg.mode = MD_REG;
        fun_arg.raw_type = lst->member_class->raw_type;
        ;
        fun_arg.reg = 10 + i;
      } else {
        fun_arg.mode = MD_INDIR_REG;
        fun_arg.raw_type = fun_arg.reg = RISCV_REG_FP;
        fun_arg.off = stk_arg_cnt++ * 8;
      }
      // This *shoudlnt* mutate any of the argument registers
      if (lst->reg >= 0 && lst->reg != REG_NONE) {
        write_to.mode = MD_REG;
        write_to.reg = lst->reg;
        write_to.off = 0;
        write_to.raw_type = lst->member_class->raw_type;
      } else {
        write_to.mode = MD_FRAME;
        write_to.off = lst->off;
        write_to.raw_type = lst->member_class->raw_type;
      }
      if ((cctrl->cur_fun->base.flags & CLSF_VARGS) &&
          !strcmp("argv", lst->str)) {
        AIWNIOS_ADD_CODE(RISCV_ADDI(5, RISCV_REG_FP, stk_arg_cnt * 8));
        fun_arg.reg = 5;
        fun_arg.mode = MD_REG;
        fun_arg.raw_type = RT_I64i;
      }
      code_off = ICMov(cctrl, &write_to, &fun_arg, bin, code_off);
      lst = lst->next;
    }
  } else {
    // Things from the HolyC side use __IC_ARG
    // We go backwards as this is REVERSE polish notation
    for (i = 0, rpn = cctrl->code_ctrl->ir_code->last;
         rpn != cctrl->code_ctrl->ir_code; rpn = rpn->base.last) {
      if (rpn->type == __IC_ARG) {
        arg = rpn->base.next;
        PushTmp(cctrl, arg, NULL);
        PopTmp(cctrl, arg);
        i = rpn->integer;
        if (i <= 7) {
          fun_arg.mode = MD_REG;
          fun_arg.raw_type = arg->res.raw_type;
          fun_arg.reg = 10 + i;
          if (fun_arg.raw_type == RT_F64) {
            AIWNIOS_ADD_CODE(RISCV_FMV_D_X(fun_arg.reg, fun_arg.reg));
          }
        } else {
          fun_arg.mode = MD_INDIR_REG;
          fun_arg.raw_type = arg->res.raw_type;
          fun_arg.reg = RISCV_REG_FP;
          fun_arg.off = stk_arg_cnt++ * 8;
        }
        code_off = ICMov(cctrl, &arg->res, &fun_arg, bin, code_off);
      } else if (rpn->type == IC_GET_VARGS_PTR) {
        arg = ICArgN(rpn, 0);
        PushTmp(cctrl, arg, NULL);
        PopTmp(cctrl, arg);
        if (arg->res.mode == MD_REG) {
          AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, arg->res.reg), RISCV_REG_FP,
                                      stk_arg_cnt * 8));
        } else {
          AIWNIOS_ADD_CODE(
              RISCV_ADDI(MIR(cctrl, 5), RISCV_REG_FP, stk_arg_cnt * 8));
          tmp.reg = 5;
          tmp.mode = MD_REG;
          tmp.raw_type = RT_I64i;
          code_off = ICMov(cctrl, &arg->res, &tmp, bin, code_off);
        }
      }
    }
  }
  return code_off;
}

//
// DO NOT CHANGE WITHOUT LOOKING CLOSEY AT FuncProlog
//
static int64_t FuncEpilog(CCmpCtrl *cctrl, char *bin, int64_t code_off) {
  char push_ireg[32], ilist[32];
  char push_freg[32], flist[32];
  int64_t i, r, r2, fsz, i2, i3, off, is_vargs = 0, arg_cnt = 0;
  int64_t fsave_area;
  CICArg spill_loc = {0}, write_to = {0}, stack = {0}, tmp = {0};
  CRPN *rpn;
  /* <== OLD SP
   * saved regs
   * wiggle room
   * locals
   * OLD FP,LR<===FP=SP
   */
  if (cctrl->cur_fun) {
    fsz = cctrl->cur_fun->base.sz + 16; //+16 for LR/FP
    arg_cnt = cctrl->cur_fun->argc;
    is_vargs = !!(cctrl->cur_fun->base.flags & CLSF_VARGS);
  } else {
    fsz = 16;
    for (rpn = cctrl->code_ctrl->ir_code->next;
         rpn != cctrl->code_ctrl->ir_code; rpn = rpn->base.next) {
      if (rpn->type == __IC_SET_FRAME_SIZE) {
        fsz = rpn->integer;
        break;
      } else if (rpn->type == IC_GET_VARGS_PTR) {
        is_vargs = 1;
      } else if (rpn->type == __IC_ARG) {
        arg_cnt++;
      }
    }
  }
  // ALIGN TO 8
  if (fsz % 8)
    fsz += 8 - fsz % 8;
  int64_t to_push = (i2 = __FindPushedIRegs(cctrl, push_ireg)) * 8 +
                    (i3 = __FindPushedFRegs(cctrl, push_freg)) * 8 + fsz +
                    cctrl->backend_user_data0,
          old_regs_start; // old_FP,old_LR
  if (i2 % 2)
    to_push += 8; // We push a dummy register in a pair if not aligned to 2
  if (i3 % 2)
    to_push += 8; // Ditto
  if (to_push % 16)
    to_push += 8;
  old_regs_start = fsz + cctrl->backend_user_data0 + 8;
  i2 = 0;
  for (i = 0; i != 32; i++)
    if (push_ireg[i])
      ilist[i2++] = i;
  if (i2 % 2)
    ilist[i2++] = 0; // 0 is hard zero
  i3 = 0;
  for (i = 0; i != 32; i++)
    if (push_freg[i])
      flist[i3++] = i;
  if (i3 % 2)
    flist[i3++] = 1; // Dont use 0 as it is a return register
  //<==== OLD SP
  // first saved reg pair<==-16
  // next saved reg pair<===-32
  //
  off = -old_regs_start;
  for (i = 0; i != i2; i++) {
    stack.mode = MD_INDIR_REG;
    stack.reg = RISCV_REG_FP;
    stack.raw_type = RT_I64i;
    stack.off = off;
    tmp.mode = MD_REG;
    tmp.reg = ilist[i];
    tmp.raw_type = RT_I64i;
    if (ilist[i] != 0)
      code_off = ICMov(cctrl, &tmp, &stack, bin, code_off);
    off -= 8;
  }
  fsave_area = off;
  for (i = 0; i != i3; i++) {
    stack.mode = MD_INDIR_REG;
    stack.reg = RISCV_REG_FP;
    stack.raw_type = RT_F64;
    stack.off = off;
    tmp.mode = MD_REG;
    tmp.reg = flist[i];
    tmp.raw_type = RT_F64;
    if (flist[i] != 0)
      code_off = ICMov(cctrl, &tmp, &stack, bin, code_off);
    off -= 8;
  }

  AIWNIOS_ADD_CODE(RISCV_ADDI(RISCV_REG_SP, RISCV_REG_FP, 0));
  AIWNIOS_ADD_CODE(RISCV_LD(1, RISCV_REG_FP, -8));
  AIWNIOS_ADD_CODE(RISCV_LD(RISCV_REG_FP, RISCV_REG_FP, -16));
  AIWNIOS_ADD_CODE(RISCV_JALR(MIR(cctrl, 0), 1, 0));
  return code_off;
}
static int64_t __SexyPostOp(CCmpCtrl *cctrl, CRPN *rpn,
                            int64_t (*i_imm)(char *, int64_t, int64_t),
                            int64_t (*ireg)(char *, int64_t, int64_t),
                            int64_t (*freg)(char *, int64_t, int64_t),
                            char *bin, int64_t code_off) {
  //
  // See hack in PushTmpDepthFirst motherfucker
  //
  CRPN *next = rpn->base.next, *tc;
  int64_t code;
  int64_t sz;
  CICArg derefed = {0}, tmp = {0}, tmp2 = {0};
  if (next->type == IC_TYPECAST) {
    TYPECAST_ASSIGN_BEGIN(next, next);
    if (next->raw_type == RT_F64) {
      code_off =
          DerefToICArg(cctrl, &derefed, next, RISCV_IPOOP2, bin, code_off);
      tmp = derefed; //==RT_F64
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 1, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      code_off = __ICMoveF64(cctrl, RISCV_FRET, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(freg(MFR(cctrl, tmp.reg), tmp.reg, RISCV_FRET));
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
    } else if (Is12Bit(rpn->integer)) {
      code_off =
          DerefToICArg(cctrl, &derefed, next, RISCV_IPOOP2, bin, code_off);
      tmp = derefed;
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, RISCV_IPOOP1, bin,
                                 code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      AIWNIOS_ADD_CODE(i_imm(MIR(cctrl, tmp.reg), tmp.reg, rpn->integer));
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
    } else {
      code_off =
          DerefToICArg(cctrl, &derefed, next, RISCV_IPOOP2, bin, code_off);
      tmp = derefed;
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, RISCV_IPOOP1, bin,
                                 code_off);
      code_off = __ICMoveI64(cctrl, 5, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(ireg(MIR(cctrl, tmp.reg), tmp.reg, 5));
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    TYPECAST_ASSIGN_END(next);
  } else {
    if (next->raw_type == RT_F64) {
      code_off =
          DerefToICArg(cctrl, &derefed, next, RISCV_IPOOP2, bin, code_off);
      tmp = derefed; //==RT_F64
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 1, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      code_off = __ICMoveF64(cctrl, RISCV_FRET, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(freg(MFR(cctrl, tmp.reg), tmp.reg, RISCV_FRET));
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
    } else if (Is12Bit(rpn->integer)) {
      code_off =
          DerefToICArg(cctrl, &derefed, next, RISCV_IPOOP2, bin, code_off);
      tmp = derefed;
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, RISCV_IPOOP1, bin,
                                 code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      AIWNIOS_ADD_CODE(i_imm(MIR(cctrl, tmp.reg), tmp.reg, rpn->integer));
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
    } else {
      code_off =
          DerefToICArg(cctrl, &derefed, next, RISCV_IPOOP1, bin, code_off);
      tmp = derefed;
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, RISCV_IPOOP1, bin,
                                 code_off);
      code_off = __ICMoveI64(cctrl, 5, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(ireg(MIR(cctrl, tmp.reg), tmp.reg, 5));
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
  }
  return code_off;
}

static int64_t DoNothing3(int64_t, int64_t, int64_t) {
}
// Psudeo op
static int64_t RISCV_FNEG(int64_t d, int64_t a) {
  return RISCV_SGNJN(d, a, a);
}
static int64_t RISCV_NEG(int64_t d, int64_t a) {
  return RISCV_SUB(d, 0, a); // 0 is hardwired zero
}
static int64_t RISCV_NOT(int64_t d, int64_t a) {
  return RISCV_XORI(d, a, -1);
}
static int64_t RISCV_SUBI(int64_t d, int64_t a, int64_t imm) {
  return RISCV_ADDI(d, a, -imm);
}
// This will account for long jumps
static int64_t RISCV_JccToLabel(CCodeMisc *misc, int64_t cond, int64_t a,
                                int64_t b, char *bin, int64_t code_off) {
  // First pass assumes worst case jumps
  if (bin && Is12Bit(misc->code_off - code_off)) {
    AIWNIOS_ADD_CODE(RISCV_Jcc(cond, a, b, 0));
    if (bin)
      CodeMiscAddRef(misc, bin + code_off - 4)->is_4_bytes = 1;
    return code_off;
  }
  AIWNIOS_ADD_CODE(RISCV_Jcc(cond ^ 1, a, b, 8)); //+4
  AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));              //+4
  if (bin)
    CodeMiscAddRef(misc, bin + code_off - 4)->is_jal = 1;
  return code_off;
}
static int64_t RISCV_FJcc(CCmpCtrl *cctrl, CCodeMisc *to, int64_t cond,
                          int64_t a, int64_t b, char *bin, int64_t code_off) {
  switch (cond) {
  case RISCV_COND_E:
    AIWNIOS_ADD_CODE(RISCV_FEQ_D(MIR(cctrl, RISCV_IRET), a, b))
    code_off =
        RISCV_JccToLabel(to, RISCV_COND_E ^ 1, RISCV_IRET, 0, bin, code_off);
    break;
  case RISCV_COND_E ^ 1:
    AIWNIOS_ADD_CODE(RISCV_FEQ_D(MIR(cctrl, RISCV_IRET), a, b))
    code_off = RISCV_JccToLabel(to, RISCV_COND_E, RISCV_IRET, 0, bin, code_off);
    break;
  case RISCV_COND_A ^ 1:
  case RISCV_COND_G ^ 1:
  case RISCV_COND_LE:
  case RISCV_COND_BE:
    AIWNIOS_ADD_CODE(RISCV_FLE_D(MIR(cctrl, RISCV_IRET), a, b))
    code_off =
        RISCV_JccToLabel(to, RISCV_COND_E ^ 1, RISCV_IRET, 0, bin, code_off);
    break;
  case RISCV_COND_L:
  case RISCV_COND_B:
  case RISCV_COND_AE ^ 1:
  case RISCV_COND_GE ^ 1:
    AIWNIOS_ADD_CODE(RISCV_FLT_D(MIR(cctrl, RISCV_IRET), a, b))
    code_off =
        RISCV_JccToLabel(to, RISCV_COND_E ^ 1, RISCV_IRET, 0, bin, code_off);
    break;
  case RISCV_COND_LE ^ 1:
  case RISCV_COND_BE ^ 1:
  case RISCV_COND_A:
  case RISCV_COND_G:
    AIWNIOS_ADD_CODE(RISCV_FLT_D(MIR(cctrl, RISCV_IRET), b, a))
    code_off =
        RISCV_JccToLabel(to, RISCV_COND_E ^ 1, RISCV_IRET, 0, bin, code_off);
    break;
  case RISCV_COND_L ^ 1:
  case RISCV_COND_B ^ 1:
  case RISCV_COND_AE:
  case RISCV_COND_GE:
    AIWNIOS_ADD_CODE(RISCV_FLE_D(MIR(cctrl, RISCV_IRET), b, a))
    code_off =
        RISCV_JccToLabel(to, RISCV_COND_E ^ 1, RISCV_IRET, 0, bin, code_off);
    break;
  }
  return code_off;
}
static int64_t Make4ByteMask(int64_t idx) {
  int64_t b12 = idx >> 12;
  int64_t b11 = idx >> 11;
  int64_t b4_1 = (idx >> 1) & ((1 << 4) - 1);
  int64_t b10_5 = (idx >> 5) & ((1 << 6) - 1);
  return (b12 << 31) | (b10_5 << 25) | (b4_1 << 8) | (b4_1 << 8) | (b11 << 7);
}
//
// ALWAYS ASSUME WORST CASE if we dont have a ALLOC'ed peice of RAM
//
int64_t __OptPassFinal(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                       int64_t code_off) {
  CCodeMisc *misc, *misc2;
  CRPN *old_rpn = cctrl->cur_rpn;
  cctrl->cur_rpn = rpn;
  CRPN *next, *next2, **range, **range_args, *next3, *a, *b;
  CICArg tmp = {0}, orig_dst = {0}, tmp2 = {0}, derefed = {0};
  int64_t i = 0, cnt, i2, use_reg, a_reg, b_reg, into_reg, use_flt_cmp, reverse,
          is_typecast, use_lock_prefix = 0;
  int64_t *range_cmp_types, use_flags = rpn->res.set_flags;
  CCodeMisc *old_fail_misc = (CCodeMisc *)cctrl->backend_user_data5,
            *old_pass_misc = (CCodeMisc *)cctrl->backend_user_data6;
  cctrl->backend_user_data5 = 0;
  cctrl->backend_user_data6 = 0;
  rpn->res.set_flags = 0;
  if (rpn->flags & ICF_PRECOMPUTED)
    goto ret;
  char *enter_addr2, *enter_addr, *exit_addr, **fail1_addr, **fail2_addr,
      ***range_fail_addrs;
  struct CRangeFailTuple {
    int64_t a, b, is_flt;
  } *range_fail_regs;
  if (cctrl->code_ctrl->dbg_info && cctrl->code_ctrl->final_pass &&
      rpn->ic_line) { // Final run
    if (MSize(cctrl->code_ctrl->dbg_info) / 8 >
        rpn->ic_line - cctrl->code_ctrl->min_ln) {
      i = (int64_t)(cctrl->code_ctrl
                        ->dbg_info[rpn->ic_line - cctrl->code_ctrl->min_ln]);
      if (!i)
        cctrl->code_ctrl->dbg_info[rpn->ic_line - cctrl->code_ctrl->min_ln] =
            bin + code_off;
      else if ((int64_t)bin + code_off < i)
        cctrl->code_ctrl->dbg_info[rpn->ic_line - cctrl->code_ctrl->min_ln] =
            bin + code_off;
    }
  }
  static const void *poop_ants[IC_CNT] = {
      [IC_FS] = &&ic_fs,
      [IC_GS] = &&ic_gs,
      [IC_LOCK] = &&ic_lock,
      [IC_GOTO] = &&ic_goto,
      [IC_GOTO_IF] = &&ic_goto_if,
      [IC_TO_BOOL] = &&ic_to_bool,
      [IC_TO_I64] = &&ic_to_i64,
      [IC_TO_F64] = &&ic_to_f64,
      [IC_LABEL] = &&ic_label,
      [IC_STATIC] = &&ic_static,
      [IC_GLOBAL] = &&ic_global,
      [IC_NOP] = &&ic_nop,
      [IC_NEG] = &&ic_neg,
      [IC_POS] = &&ic_pos,
      [IC_STR] = &&ic_str,
      [IC_CHR] = &&ic_chr,
      [IC_POW] = &&ic_pow,
      [IC_ADD] = &&ic_add,
      [IC_EQ] = &&ic_eq,
      [IC_SUB] = &&ic_sub,
      [IC_DIV] = &&ic_div,
      [IC_MUL] = &&ic_mul,
      [IC_DEREF] = &&ic_deref,
      [IC_AND] = &&ic_and,
      [IC_ADDR_OF] = &&ic_addr_of,
      [IC_XOR] = &&ic_xor,
      [IC_MOD] = &&ic_mod,
      [IC_OR] = &&ic_or,
      [IC_LT] = &&ic_lt,
      [IC_GT] = &&ic_gt,
      [IC_LNOT] = &&ic_lnot,
      [IC_BNOT] = &&ic_bnot,
      [IC_POST_INC] = &&ic_post_inc,
      [IC_POST_DEC] = &&ic_post_dec,
      [IC_PRE_INC] = &&ic_pre_inc,
      [IC_PRE_DEC] = &&ic_pre_dec,
      [IC_AND_AND] = &&ic_and_and,
      [IC_OR_OR] = &&ic_or_or,
      [IC_XOR_XOR] = &&ic_xor_xor,
      [IC_EQ_EQ] = &&ic_eq_eq,
      [IC_NE] = &&ic_ne,
      [IC_LE] = &&ic_le,
      [IC_GE] = &&ic_ge,
      [IC_LSH] = &&ic_lsh,
      [IC_RSH] = &&ic_rsh,
      [IC_ADD_EQ] = &&ic_add_eq,
      [IC_SUB_EQ] = &&ic_sub_eq,
      [IC_MUL_EQ] = &&ic_mul_eq,
      [IC_DIV_EQ] = &&ic_div_eq,
      [IC_LSH_EQ] = &&ic_lsh_eq,
      [IC_RSH_EQ] = &&ic_rsh_eq,
      [IC_AND_EQ] = &&ic_and_eq,
      [IC_OR_EQ] = &&ic_or_eq,
      [IC_XOR_EQ] = &&ic_xor_eq,
      [IC_MOD_EQ] = &&ic_mod_eq,
      [IC_I64] = &&ic_i64,
      [IC_F64] = &&ic_f64,
      [IC_ARRAY_ACC] = &&ic_array_acc,
      [IC_RET] = &&ic_ret,
      [IC_CALL] = &&ic_call,
      [IC_COMMA] = &&ic_comma,
      [IC_UNBOUNDED_SWITCH] = &&ic_unbounded_switch,
      [IC_BOUNDED_SWITCH] = &&ic_bounded_switch,
      [IC_SUB_RET] = &&ic_sub_ret,
      [IC_SUB_PROLOG] = &&ic_sub_prolog,
      [IC_SUB_CALL] = &&ic_sub_call,
      [IC_TYPECAST] = &&ic_typecast,
      [IC_BASE_PTR] = &&ic_base_ptr,
      [IC_IREG] = &&ic_ireg,
      [IC_FREG] = &&ic_freg,
      [__IC_VARGS] = &&ic_vargs,
      [IC_RELOC] = &&ic_reloc,
      [__IC_CALL] = &&ic_call,
      [__IC_STATIC_REF] = &&ic_static_ref,
      [__IC_SET_STATIC_DATA] = &&ic_set_static_data,
      [IC_SHORT_ADDR] = &&ic_short_addr,
      [IC_BT] = &&ic_bt,
      [IC_BTC] = &&ic_btc,
      [IC_BTS] = &&ic_bts,
      [IC_BTR] = &&ic_btr,
      [IC_LBTC] = &&ic_lbtc,
      [IC_LBTS] = &&ic_lbts,
      [IC_LBTR] = &&ic_lbtr,
      [IC_MAX_I64] = &&ic_max_i64,
      [IC_MIN_I64] = &&ic_min_i64,
      [IC_MAX_U64] = &&ic_max_u64,
      [IC_MIN_U64] = &&ic_min_u64,
      [IC_SIGN_I64] = &&ic_sign_i64,
      [IC_SQR_I64] = &&ic_sqr_i64,
      [IC_SQR_U64] = &&ic_sqr_u64,
      [IC_SQR] = &&ic_sqr,
      [IC_ABS] = &&ic_abs,
      [IC_SQRT] = &&ic_sqrt,
      [IC_SIN] = &&ic_sin,
      [IC_COS] = &&ic_cos,
      [IC_TAN] = &&ic_tan,
      [IC_ATAN] = &&ic_atan,
      [IC_RAW_BYTES] = &&ic_raw_bytes,
  };
  if (!poop_ants[rpn->type])
    goto ret;
  goto *poop_ants[rpn->type];
  do {
  ic_max_u64:
    // Integers only
#define IC_MXX_X64(OP)                                                         \
  {                                                                            \
    a = ICArgN(rpn, 1);                                                        \
    b = ICArgN(rpn, 0);                                                        \
    code_off = __OptPassFinal(cctrl, a, bin, code_off);                        \
    code_off = __OptPassFinal(cctrl, b, bin, code_off);                        \
    tmp.mode = MD_REG;                                                         \
    tmp.raw_type = rpn->res.raw_type;                                          \
    into_reg = tmp.reg = RISCV_IRET; /* avoid conflicting with a/b's reg*/     \
    code_off = PutICArgIntoReg(cctrl, &b->res, tmp.raw_type, RISCV_IPOOP1,     \
                               bin, code_off);                                 \
    code_off = ICMov(cctrl, &tmp, &a->res, bin, code_off);                     \
    /*+4 for Jcc,+4 for ADDI a,b,0*/                                           \
    AIWNIOS_ADD_CODE(RISCV_Jcc(OP, into_reg, b->res.reg, 4 + 4));              \
    AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, into_reg), b->res.reg, 0));         \
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                   \
  }
    IC_MXX_X64(RISCV_COND_A);
    break;
  ic_max_i64:
    IC_MXX_X64(RISCV_COND_G);
    break;
  ic_min_u64:
    IC_MXX_X64(RISCV_COND_B);
    break;
  ic_min_i64:
    IC_MXX_X64(RISCV_COND_L);
    break;
  ic_gs:
    next = rpn->base.next;
    if (next->type != IC_RELOC && next->type != IC_I64) {
      printf("Expected a relocation at IC_GS,aborting(Contact nrootcoauto for "
             "more info)\n");
      abort();
    }
    goto segment;
    break;
  ic_fs:
    next = rpn->base.next;
    if (next->type != IC_RELOC && next->type != IC_I64) {
      printf("Expected a relocation at IC_FS,aborting(Contact nrootcoauto for "
             "more info)\n");
      abort();
    }
  segment:
    into_reg = RISCV_IPOOP1;
    if (rpn->res.mode == MD_REG) {
      into_reg = rpn->res.reg;
    }
    if (next->type == IC_RELOC) {
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, into_reg), 0));
      AIWNIOS_ADD_CODE(RISCV_LD(MIR(cctrl, into_reg), into_reg, 0));
      if (bin)
        CodeMiscAddRef(next->code_misc, bin + code_off - 8);
      AIWNIOS_ADD_CODE(RISCV_ADD(MIR(cctrl, into_reg), 4, into_reg));
      AIWNIOS_ADD_CODE(RISCV_LD(MIR(cctrl, into_reg), into_reg, 0));
    } else if (next->type == IC_I64) {
      if (-1 != RISCV_LD(into_reg, 4, next->integer)) {
        AIWNIOS_ADD_CODE(RISCV_LD(MIR(cctrl, into_reg), 4, next->integer));
      } else {
        code_off = __ICMoveI64(cctrl, into_reg, next->integer, bin, code_off);
        AIWNIOS_ADD_CODE(RISCV_ADD(MIR(cctrl, into_reg), 4, into_reg));
        AIWNIOS_ADD_CODE(RISCV_LD(MIR(cctrl, into_reg), into_reg, 0));
      }
    }
    if (rpn->res.mode != MD_REG) {
      tmp.mode = MD_REG;
      tmp.raw_type = RT_I64i;
      tmp.reg = into_reg;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_lock:
    cctrl->is_lock_expr = 1;
    code_off = __OptPassFinal(cctrl, rpn->base.next, bin, code_off);
    cctrl->is_lock_expr = 0;
    break;
  ic_short_addr:
    if (rpn->res.mode == MD_REG)
      into_reg = rpn->res.reg;
    else
      into_reg = RISCV_IRET;
    AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, into_reg), 2 << 12));
    AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, into_reg), into_reg, 0));
    if (bin)
      CodeMiscAddRef(rpn->code_misc, bin + code_off - 8);
    if (rpn->res.mode != MD_REG) {
      tmp.mode = MD_REG;
      tmp.raw_type = RT_I64i;
      tmp.reg = into_reg;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_reloc:
    if (rpn->res.mode == MD_REG)
      into_reg = rpn->res.reg;
    else
      into_reg = RISCV_IRET;
    misc = rpn->code_misc;
    if (misc->aot_before_hint && 0) {
      i = -code_off + misc->aot_before_hint;
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, into_reg), i >> 12));
      AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, into_reg), into_reg, i));
    } else {
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, into_reg), 0));
      AIWNIOS_ADD_CODE(RISCV_LD(MIR(cctrl, into_reg), into_reg, 0));
      if (bin)
        CodeMiscAddRef(rpn->code_misc, bin + code_off - 8);
    }
    if (rpn->res.mode != MD_REG) {
      tmp.mode = MD_REG;
      tmp.raw_type = RT_I64i;
      tmp.reg = RISCV_IRET;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_goto_if:
    reverse = 0;
    next = rpn->base.next;
  gti_enter:
    if (!IsCompoundCompare(next))
      switch (next->type) {
        break;
      case IC_LNOT:
        reverse = !reverse;
        next = next->base.next;
        goto gti_enter;
        break;
      case IC_EQ_EQ:
#define CMP_AND_JMP(COND)                                                      \
  {                                                                            \
    next3 = next->base.next;                                                   \
    next2 = ICFwd(next3);                                                      \
    PushTmpDepthFirst(cctrl, next3, SpillsTmpRegs(next2));                     \
    PushTmpDepthFirst(cctrl, next2, 0);                                        \
    PopTmp(cctrl, next2);                                                      \
    PopTmp(cctrl, next3);                                                      \
    if (next3->raw_type != RT_F64 && next2->raw_type != RT_F64) {              \
      code_off = __OptPassFinal(cctrl, next3, bin, code_off);                  \
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);                  \
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i, RISCV_IPOOP2,    \
                                 bin, code_off);                               \
      code_off = PutICArgIntoReg(cctrl, &next3->res, RT_I64i, RISCV_IPOOP1,    \
                                 bin, code_off);                               \
      code_off =                                                               \
          RISCV_JccToLabel(rpn->code_misc, (COND) ^ reverse, next2->res.reg,   \
                           next3->res.reg, bin, code_off);                     \
    } else if (next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {       \
      code_off = __OptPassFinal(cctrl, next3, bin, code_off);                  \
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);                  \
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_F64, RISCV_FPOOP2,     \
                                 bin, code_off);                               \
      code_off = PutICArgIntoReg(cctrl, &next3->res, RT_F64, RISCV_FPOOP1,     \
                                 bin, code_off);                               \
      code_off = RISCV_FJcc(cctrl, rpn->code_misc, (COND) ^ reverse,           \
                            next2->res.reg, next3->res.reg, bin, code_off);    \
    }                                                                          \
  }
        CMP_AND_JMP(RISCV_COND_E);
        goto ret;
        break;
      case IC_NE:
        CMP_AND_JMP(RISCV_COND_E | 1);
        goto ret;
        break;
      case IC_LT:
        next3 = next->base.next;
        next2 = ICFwd(next3);
        if (next3->raw_type == RT_U64i || next2->raw_type == RT_U64i ||
            next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {
          CMP_AND_JMP(RISCV_COND_B);
        } else {
          CMP_AND_JMP(RISCV_COND_L);
        }
        goto ret;
        break;
      case IC_GT:
        next3 = next->base.next;
        next2 = ICFwd(next3);
        if (next3->raw_type == RT_U64i || next2->raw_type == RT_U64i ||
            next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {
          CMP_AND_JMP(RISCV_COND_A);
        } else {
          CMP_AND_JMP(RISCV_COND_G);
        }
        goto ret;
        break;
      case IC_LE:
        next3 = next->base.next;
        next2 = ICFwd(next3);
        if (next3->raw_type == RT_U64i || next2->raw_type == RT_U64i ||
            next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {
          CMP_AND_JMP(RISCV_COND_BE);
        } else {
          CMP_AND_JMP(RISCV_COND_LE);
        }
        goto ret;
        break;
      case IC_GE:
        next3 = next->base.next;
        next2 = ICFwd(next3);
        if (next3->raw_type == RT_U64i || next2->raw_type == RT_U64i ||
            next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {
          CMP_AND_JMP(RISCV_COND_AE);
        } else {
          CMP_AND_JMP(RISCV_COND_GE);
        }
        goto ret;
      case IC_BT:
      case IC_BTC:
      case IC_BTS:
      case IC_BTR:
      case IC_LBTC:
      case IC_LBTS:
      case IC_LBTR:
        abort(); // For now
      case IC_OR_OR:
      case IC_AND_AND:
        PushTmpDepthFirst(cctrl, next, 0);
        PopTmp(cctrl, next);
        if (!rpn->code_misc2)
          rpn->code_misc2 = CodeMiscNew(cctrl, CMT_LABEL);
        if (!reverse) {
          cctrl->backend_user_data5 = (int64_t)rpn->code_misc2;
          cctrl->backend_user_data6 = (int64_t)rpn->code_misc;
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          rpn->code_misc2->addr = bin + code_off;
          rpn->code_misc->code_off = code_off;
          goto ret;
        } else {
          cctrl->backend_user_data5 = (int64_t)rpn->code_misc;
          cctrl->backend_user_data6 = (int64_t)rpn->code_misc2;
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          rpn->code_misc2->addr = bin + code_off;
          rpn->code_misc2->code_off = code_off;
          goto ret;
        }
      }
    PushTmpDepthFirst(cctrl, next, 0);
    PopTmp(cctrl, next);
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    if (next->raw_type == RT_F64) {
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_F64, RISCV_IPOOP1, bin,
                                 code_off);
      code_off = __ICMoveF64(cctrl, RISCV_FPOOP2, 0, bin, code_off);
      AIWNIOS_ADD_CODE(
          RISCV_FEQ_D(MIR(cctrl, RISCV_IRET), next->res.reg, RISCV_FPOOP2));
      // freg==0.,so do opposite
      if (!reverse) {
        code_off = RISCV_JccToLabel(rpn->code_misc, RISCV_COND_E, RISCV_IRET, 0,
                                    bin, code_off);
      } else {
        code_off = RISCV_JccToLabel(rpn->code_misc, RISCV_COND_E ^ 1,
                                    RISCV_IRET, 0, bin, code_off);
      }
    } else {
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_I64i, RISCV_IRET, bin,
                                 code_off);
      if (!reverse) {
        code_off = RISCV_JccToLabel(rpn->code_misc, RISCV_COND_E ^ 1,
                                    next->res.reg, 0, bin, code_off);
      } else {
        code_off = RISCV_JccToLabel(rpn->code_misc, RISCV_COND_E, next->res.reg,
                                    0, bin, code_off);
      }
    }
    break;
  ic_to_bool:
    into_reg = RISCV_IRET;
    tmp.mode = MD_REG;
    tmp.raw_type = RT_I64i;
    if (rpn->res.mode == MD_REG)
      into_reg = rpn->res.reg;
    tmp.reg = into_reg;
    next = rpn->base.next;
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    code_off =
        PutICArgIntoReg(cctrl, &next->res, RT_I64i, RISCV_IRET, bin, code_off);
    AIWNIOS_ADD_CODE(RISCV_SLTU(MIR(cctrl, into_reg), 0, next->res.reg));
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
    // These both do the same thing,only thier type detirmines what happens
  ic_to_i64:
  ic_to_f64:
    next = rpn->base.next;
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
    break;
  ic_typecast:
    next = rpn->base.next;
    if (next->raw_type == RT_F64 && rpn->raw_type != RT_F64) {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next->res, next->raw_type, 1, bin, code_off);
      tmp.mode = MD_REG;
      tmp.reg = RISCV_IRET;
      tmp.raw_type = rpn->raw_type;
      if (rpn->res.mode == MD_REG)
        tmp.reg = rpn->res.reg;
      AIWNIOS_ADD_CODE(RISCV_FMV_X_D(MIR(cctrl, tmp.reg), next->res.reg));
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else if (next->raw_type != RT_F64 && rpn->raw_type == RT_F64) {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next->res, next->raw_type, 1, bin, code_off);
      tmp.mode = MD_REG;
      tmp.reg = RISCV_FRET;
      tmp.raw_type = rpn->raw_type;
      if (rpn->res.mode == MD_REG)
        tmp.reg = rpn->res.reg;
      AIWNIOS_ADD_CODE(RISCV_FMV_D_X(MIR(cctrl, tmp.reg), next->res.reg));
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else if (next->type == IC_DEREF) {
      code_off = DerefToICArg(cctrl, &tmp, next, RISCV_IPOOP1, bin, code_off);
      tmp.raw_type = rpn->raw_type;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
    }
    break;
  ic_goto:
    if (cctrl->code_ctrl->final_pass) {
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
      CodeMiscAddRef(rpn->code_misc, bin + code_off - 4)->is_jal = 1;
    } else
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
    break;
  ic_label:
    rpn->code_misc->addr = bin + code_off;
    rpn->code_misc->code_off = code_off;
    break;
  ic_global:
    //
    // Here's the deal. Global arrays should be acceessed via
    // IC_ADDR_OF. So loading int register here
    //
    if (rpn->global_var->base.type & HTT_FUN) {
      next = rpn;
      goto get_glbl_ptr;
    } else {
      assert(!rpn->global_var->dim.next);
      tmp.mode = MD_PTR;
      tmp.off = (int64_t)rpn->global_var->data_addr;
      tmp.raw_type = rpn->global_var->var_class->raw_type;
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp)
        rpn->res = tmp;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_nop:
    // Bungis
    break;
    abort();
    break;
  ic_neg:
#define BACKEND_UNOP(flt_op, int_op)                                           \
  next = rpn->base.next;                                                       \
  code_off = __OptPassFinal(cctrl, next, bin, code_off);                       \
  into_reg = rpn->res.raw_type == RT_F64 ? RISCV_FRET : RISCV_IRET;            \
  if (rpn->res.mode == MD_REG)                                                 \
    into_reg = rpn->res.reg;                                                   \
  if (rpn->res.mode == MD_NULL)                                                \
    break;                                                                     \
  tmp.mode = MD_REG;                                                           \
  tmp.raw_type = rpn->res.raw_type;                                            \
  tmp.reg = into_reg;                                                          \
  code_off = PutICArgIntoReg(                                                  \
      cctrl, &next->res, next->res.raw_type,                                   \
      next->res.raw_type == RT_F64 ? RISCV_FRET : RISCV_IRET, bin, code_off);  \
  if (rpn->raw_type == RT_F64) {                                               \
    AIWNIOS_ADD_CODE(flt_op(MFR(cctrl, into_reg), next->res.reg));             \
  } else                                                                       \
    AIWNIOS_ADD_CODE(int_op(MFR(cctrl, into_reg), next->res.reg));             \
  rpn->tmp_res = tmp;                                                          \
  if (rpn->res.keep_in_tmp)                                                    \
    rpn->res = tmp;                                                            \
  code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    BACKEND_UNOP(RISCV_FNEG, RISCV_NEG);
    break;
  ic_pos:
    next = ICArgN(rpn, 0);
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    rpn->tmp_res = next->res;
    if (rpn->res.keep_in_tmp)
      rpn->res = next->res;
    code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
    break;
    abort();
    break;
  ic_str:
    if (rpn->res.mode == MD_REG) {
      into_reg = rpn->res.reg;
    } else
      into_reg = RISCV_IRET;
    if (cctrl->flags & CCF_STRINGS_ON_HEAP) {
      //
      // README: We will "NULL"ify the rpn->code_misc->str later so we dont
      // free it
      //
      code_off = __ICMoveI64(cctrl, into_reg, (int64_t)rpn->code_misc->str, bin,
                             code_off);
    } else {
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, into_reg), 0));
      AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, into_reg), into_reg, 0));
      CodeMiscAddRef(rpn->code_misc, bin + code_off - 8);
    }
    if (rpn->res.mode != MD_REG) {
      tmp.raw_type = rpn->raw_type;
      tmp.reg = into_reg;
      tmp.mode = MD_REG;
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp)
        rpn->res = tmp;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_chr:
    if (rpn->res.mode == MD_REG) {
      into_reg = rpn->res.reg;
    } else
      into_reg = RISCV_IPOOP1; // Store into

    code_off = __ICMoveI64(cctrl, into_reg, rpn->integer, bin, code_off);

    if (rpn->res.mode != MD_REG) {
      tmp.raw_type = rpn->raw_type;
      tmp.reg = into_reg;
      tmp.mode = MD_REG;
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp)
        rpn->res = tmp;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_pow:
    // TODO
    break;
    break;
  ic_unbounded_switch:
    //
    // Load poop into tmp,then we continue the sexyness as normal
    //
    // Also make sure RISCV_IPOOP2 has lo bound(See IC_BOUNDED_SWITCH)
    //
    next2 = ICArgN(rpn, 0);
    fail1_addr = NULL;
    fail2_addr = NULL;
    PushTmpDepthFirst(cctrl, next2, 0);
    PopTmp(cctrl, next2);
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);
    tmp.raw_type = RT_I64i;
    tmp.mode = MD_REG;
    tmp.reg = RISCV_IPOOP1;
    code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
    code_off = __ICMoveI64(cctrl, RISCV_IPOOP2, rpn->code_misc->lo, bin,
                           code_off); // X1
    goto jmp_tab_sexy;
    break;
  ic_bounded_switch:
    next2 = ICArgN(rpn, 0);
    PushTmpDepthFirst(cctrl, next2, 0);
    PopTmp(cctrl, next2);
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);
    tmp.raw_type = RT_I64i;
    tmp.mode = MD_REG;
    tmp.reg = RISCV_IPOOP1;
    code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
    code_off =
        __ICMoveI64(cctrl, RISCV_IPOOP2, rpn->code_misc->lo, bin, code_off);
    code_off =
        __ICMoveI64(cctrl, RISCV_IRET, rpn->code_misc->hi, bin, code_off);
    fail1_addr = bin + code_off;
    AIWNIOS_ADD_CODE(0); // fill in later
    fail2_addr = bin + code_off;
    AIWNIOS_ADD_CODE(0); // filled in later
  jmp_tab_sexy:
    AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, RISCV_IRET), 0));
    AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, RISCV_IRET), RISCV_IRET, 0));
    CodeMiscAddRef(rpn->code_misc, bin + code_off - 8);
    // IPOOP2 has lower bound
    AIWNIOS_ADD_CODE(
        RISCV_SUB(MIR(cctrl, RISCV_IPOOP1), RISCV_IPOOP1, RISCV_IPOOP2));
    AIWNIOS_ADD_CODE(
        RISCV_SLLI(MIR(cctrl, RISCV_IPOOP1), RISCV_IPOOP1, 3)); //*8
    AIWNIOS_ADD_CODE(
        RISCV_ADD(MIR(cctrl, RISCV_IPOOP1), RISCV_IPOOP1, RISCV_IRET));
    AIWNIOS_ADD_CODE(RISCV_LD(MIR(cctrl, RISCV_IPOOP1), RISCV_IPOOP1, 0));
    AIWNIOS_ADD_CODE(RISCV_JALR(0, RISCV_IPOOP1, 0));
    // In case our switch statement is massive,we will use a big jump here
    if (bin && fail1_addr)
      *(int32_t *)fail1_addr =
          RISCV_Jcc(RISCV_COND_G, tmp.reg, RISCV_IRET,
                    (char *)(bin + code_off) - (char *)fail1_addr);
    if (bin && fail2_addr)
      *(int32_t *)fail2_addr =
          RISCV_Jcc(RISCV_COND_L, tmp.reg, RISCV_IPOOP2,
                    (char *)(bin + code_off) - (char *)fail2_addr);
    if (rpn->type == IC_BOUNDED_SWITCH) {
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
      if (bin)
        CodeMiscAddRef(rpn->code_misc->dft_lab, bin + code_off - 4)->is_jal =
            1; // 4 bytes wide;
    }
    break;
  ic_sub_ret:
    tmp.mode = MD_REG;
    tmp.reg = 1;
    tmp.raw_type = RT_I64i;
    code_off = PopFromStack(cctrl, &tmp, bin, code_off);
    AIWNIOS_ADD_CODE(RISCV_JALR(0, 1, 0));
    break;
  ic_sub_prolog:
    break;
  ic_sub_call:
    AIWNIOS_ADD_CODE(RISCV_ADDI(RISCV_REG_SP, RISCV_REG_SP, -8));
    // I want the return adress,I dont actually use ic_sub_prolog
    AIWNIOS_ADD_CODE(
        RISCV_AUIPC(MIR(cctrl, RISCV_IRET), 0)); // ADRESS STARTS HERE+4(1)
    AIWNIOS_ADD_CODE(
        RISCV_ADDI(MIR(cctrl, RISCV_IPOOP1), RISCV_IRET, 4 * 6)); //+4(2)
    AIWNIOS_ADD_CODE(
        RISCV_SD(MIR(cctrl, RISCV_IPOOP1), RISCV_REG_SP, 0));            //+4(2)
    AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, RISCV_IRET), 0));            //+4(3)
    AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, RISCV_IRET), RISCV_IRET, 0)); //+4(4)
    CodeMiscAddRef(rpn->code_misc, bin + code_off - 8);
    AIWNIOS_ADD_CODE(RISCV_JALR(1, RISCV_IRET, 0)); //+4(6)
    break;
  ic_add:
//
//  /\   /\      ____
//  ||___||     /    \
//  | o_o | <==(README)
//  | \_/ |     \____/
//  ||   ||
//  \/   \/
// We compute the righthand argument first
// Because the lefthand side can be assigned into(and we will need
// the temporary register containg the pointer presetn and not messed up)
//
// RIGHT ==> may have func call that invalidates tmp regs
// LEFT ==> may have a MD_INDIR_REG(where reg is a tmp reg)
// *LEFT=RIHGT ==> Write into the left's address
//
#define BACKEND_BINOP(f_op, i_op, i_imm_op)                                    \
  do {                                                                         \
    next = ICArgN(rpn, 1);                                                     \
    next2 = ICArgN(rpn, 0);                                                    \
    orig_dst = next->res;                                                      \
    code_off = __OptPassFinal(cctrl, next, bin, code_off);                     \
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);                    \
    if (rpn->res.mode == MD_NULL)                                              \
      break;                                                                   \
    if (rpn->res.mode == MD_REG) {                                             \
      into_reg = rpn->res.reg;                                                 \
    } else                                                                     \
      into_reg = rpn->res.raw_type == RT_F64 ? RISCV_FRET : RISCV_IRET;        \
    if (rpn->raw_type == RT_F64) {                                             \
      code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type,             \
                                 RISCV_FPOOP2, bin, code_off);                 \
      code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type,            \
                                 RISCV_FPOOP1, bin, code_off);                 \
    } else if (i_imm_op != DoNothing3 && IsConst(next2) &&                     \
               Is12Bit(ConstVal(next2))) {                                     \
      code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type,             \
                                 RISCV_FPOOP2, bin, code_off);                 \
      AIWNIOS_ADD_CODE(                                                        \
          i_imm_op(MIR(cctrl, into_reg), next->res.reg, ConstVal(next2)));     \
      tmp.mode = MD_REG;                                                       \
      tmp.raw_type = rpn->raw_type;                                            \
      tmp.reg = into_reg;                                                      \
      if (rpn->res.mode != MD_NULL)                                            \
        code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);               \
      break;                                                                   \
    } else {                                                                   \
      code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type,             \
                                 RISCV_IPOOP2, bin, code_off);                 \
      code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type,            \
                                 RISCV_IPOOP1, bin, code_off);                 \
    }                                                                          \
    if (rpn->raw_type == RT_F64) {                                             \
      AIWNIOS_ADD_CODE(                                                        \
          f_op(MFR(cctrl, into_reg), next->res.reg, next2->res.reg));          \
    } else {                                                                   \
      AIWNIOS_ADD_CODE(                                                        \
          i_op(MIR(cctrl, into_reg), next->res.reg, next2->res.reg));          \
    }                                                                          \
    tmp.mode = MD_REG;                                                         \
    tmp.raw_type = rpn->raw_type;                                              \
    tmp.reg = into_reg;                                                        \
    rpn->tmp_res = tmp;                                                        \
    if (rpn->res.keep_in_tmp)                                                  \
      rpn->res = tmp;                                                          \
    if (rpn->res.mode != MD_NULL) {                                            \
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                 \
    }                                                                          \
  } while (0);
    BACKEND_BINOP(RISCV_FADD_D, RISCV_ADD, RISCV_ADDI);
    break;
  ic_comma:
    next = ICArgN(rpn, 1);
    next2 = ICArgN(rpn, 0);
    orig_dst = next->res;
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    code_off = ICMov(cctrl, &rpn->res, &next2->res, bin, code_off);
    break;
  ic_eq:
    next = ICArgN(rpn, 1);
    next2 = ICArgN(rpn, 0);
    orig_dst = next->res;
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);
    if (next->type == IC_TYPECAST) {
      TYPECAST_ASSIGN_BEGIN(next, next2);
      if (rpn->res.mode != MD_NULL) {
        // Avoid a double-derefernce
        code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type,
                                   rpn->raw_type == RT_F64 ? RISCV_FPOOP1
                                                           : RISCV_IPOOP1,
                                   bin, code_off);
      }
      code_off = ICMov(cctrl, &next->res, &next2->res, bin, code_off);
      TYPECAST_ASSIGN_END(next);
    } else if (next->type == IC_DEREF) {
      tmp = next->res;
      code_off = DerefToICArg(cctrl, &tmp, next, RISCV_IPOOP2, bin, code_off);
      if (rpn->res.mode != MD_NULL) {
        // Avoid a double-derefernce
        code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type,
                                   rpn->raw_type == RT_F64 ? RISCV_FPOOP1
                                                           : RISCV_IPOOP1,
                                   bin, code_off);
      }
      code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      break;
    } else if (next->type == IC_GLOBAL) {
      next->res.mode = MD_PTR;
      next->res.raw_type = next->raw_type;
      if (next->global_var->base.type & HTT_GLBL_VAR) {
        next->res.off = (int64_t)next->global_var->data_addr;
      } else if (next->global_var->base.type & HTT_FUN) {
        next->res.off = (int64_t)((CHashFun *)next->global_var)->fun_ptr;
      }
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      if (rpn->res.mode != MD_NULL) {
        // Avoid a double-derefernce
        code_off = PutICArgIntoReg(
            cctrl, &next2->res, rpn->raw_type,
            rpn->raw_type == RT_F64 ? RISCV_FRET : RISCV_IPOOP1, bin, code_off);
      }
      code_off = ICMov(cctrl, &next->res, &next2->res, bin, code_off);
    } else {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      if (rpn->res.mode != MD_NULL) {
        // Avoid a double-derefernce
        code_off = PutICArgIntoReg(
            cctrl, &next2->res, rpn->raw_type,
            rpn->raw_type == RT_F64 ? RISCV_FRET : RISCV_IPOOP1, bin, code_off);
      }
      code_off = ICMov(cctrl, &next->res, &next2->res, bin, code_off);
    }
    code_off = ICMov(cctrl, &rpn->res, &next2->res, bin, code_off);
    break;
  ic_sub:
    BACKEND_BINOP(RISCV_FSUB_D, RISCV_SUB, RISCV_SUBI);
    break;
  ic_div:
    next = rpn->base.next;
    next2 = ICArgN(rpn, 1);
    if (next->type != IC_F64 && next2->raw_type != RT_F64) {
      if (IsConst(next))
        if (__builtin_popcountll(next->integer) == 1) {
          switch (next2->raw_type) {
          case RT_I8i:
          case RT_I16i:
          case RT_I32i:
          case RT_I64i:
            goto div_normal;
            break;
          default:
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            if (rpn->res.mode == MD_REG)
              into_reg = rpn->res.reg;
            else
              into_reg = RISCV_IRET;
            tmp.mode = MD_REG;
            tmp.raw_type = RT_I64i;
            tmp.reg = into_reg;
            code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
            AIWNIOS_ADD_CODE(RISCV_SRLI(MIR(cctrl, into_reg), into_reg,
                                        __builtin_ffsll(next->integer) - 1));
            break;
          }
          code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
          break;
        }
    }
  div_normal:
    if (rpn->raw_type == RT_U64i) {
      BACKEND_BINOP(RISCV_FDIV_D, RISCV_DIVU, DoNothing3);
    } else {
      BACKEND_BINOP(RISCV_FDIV_D, RISCV_DIV, DoNothing3);
    }
    break;
  ic_mul:
    next = rpn->base.next;
    next2 = ICArgN(rpn, 1);
    if (next->type == IC_I64 && next2->raw_type != RT_F64) {
      if (__builtin_popcountll(next->integer) == 1) {
        code_off = __OptPassFinal(cctrl, next2, bin, code_off);
        code_off = PutICArgIntoReg(cctrl, &next2->res, next2->raw_type,
                                   RISCV_IPOOP1, bin, code_off);
        if (rpn->res.mode == MD_REG) {
          code_off = ICMov(cctrl, &rpn->res, &next2->res, bin, code_off);
          AIWNIOS_ADD_CODE(RISCV_SLLI(MIR(cctrl, rpn->res.reg), rpn->res.reg,
                                      __builtin_ffsll(next->integer) - 1));
        } else {
          tmp.raw_type = RT_I64i;
          tmp.reg = RISCV_IRET;
          if (rpn->res.mode == MD_REG)
            tmp.reg = rpn->res.reg;
          tmp.mode = MD_REG;
          code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
          AIWNIOS_ADD_CODE(RISCV_SLLI(MIR(cctrl, tmp.reg), tmp.reg,
                                      __builtin_ffsll(next->integer) - 1));
          rpn->tmp_res = tmp;
          if (rpn->res.keep_in_tmp)
            rpn->res = tmp;
          code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
        }
        break;
      }
    }
    if (rpn->raw_type == RT_U64i) {
      BACKEND_BINOP(RISCV_FMUL_D, RISCV_MUL,
                    DoNothing3); // TODO unsigned multiply
    } else {
      BACKEND_BINOP(RISCV_FMUL_D, RISCV_MUL, DoNothing3);
    }
    break;
  ic_deref:
    next = rpn->base.next;
    i = 0;
    //
    // Here's the Donald Trump deal,Derefernced function pointers
    // don't derefence(you dont copy a function into memory).
    //
    // I64 (*fptr)()=&Foo;
    // (*fptr)(); //Doesn't "dereference"
    //
    if (rpn->raw_type == RT_FUNC) {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
      goto ret;
    }
    code_off = DerefToICArg(cctrl, &tmp, rpn, RISCV_IPOOP2, bin, code_off);
    rpn->tmp_res = tmp;
    if (rpn->res.keep_in_tmp)
      rpn->res = tmp;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_and:
#define FBITWISE(i_op)                                                         \
  {                                                                            \
    next2 = rpn->base.next;                                                    \
    next = ICArgN(rpn, 1);                                                     \
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);                    \
    code_off = __OptPassFinal(cctrl, next, bin, code_off);                     \
    code_off = PutICArgIntoReg(cctrl, &next2->res, RT_F64, RISCV_FPOOP1, bin,  \
                               code_off);                                      \
    code_off = PutICArgIntoReg(cctrl, &next->res, RT_F64, RISCV_FPOOP2, bin,   \
                               code_off);                                      \
    AIWNIOS_ADD_CODE(RISCV_FMV_D_X(MIR(cctrl, RISCV_IPOOP1), RISCV_FPOOP1));   \
    AIWNIOS_ADD_CODE(RISCV_FMV_D_X(MIR(cctrl, RISCV_IPOOP2), RISCV_FPOOP2));   \
    AIWNIOS_ADD_CODE(                                                          \
        i_op(MIR(cctrl, RISCV_IPOOP1), RISCV_IPOOP1, RISCV_IPOOP2));           \
    tmp.raw_type = RT_F64;                                                     \
    tmp.reg = RISCV_IPOOP1;                                                    \
    tmp.mode = MD_REG;                                                         \
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                   \
  }
    if (rpn->raw_type == RT_F64) {
      FBITWISE(RISCV_AND);
    } else
      BACKEND_BINOP(RISCV_AND, RISCV_AND, RISCV_ANDI);
    break;
  ic_addr_of:
    next = rpn->base.next;
    switch (next->type) {
      break;
    case __IC_STATIC_REF:
      if (rpn->res.mode == MD_REG)
        into_reg = rpn->res.reg;
      else
        into_reg = RISCV_IRET;
      AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, into_reg), 0));
      AIWNIOS_ADD_CODE(RISCV_ADDI(MIR(cctrl, into_reg), into_reg, 0));
      if (bin)
        CodeMiscAddRef(cctrl->statics_label, bin + code_off - 8)->offset =
            next->integer;
      goto restore_reg;
      break;
    case IC_DEREF:
      next = rpn->base.next;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_PTR, RISCV_IPOOP2, bin,
                                 code_off);
      code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
      break;
    case IC_BASE_PTR:
      if (rpn->res.mode == MD_REG)
        into_reg = rpn->res.reg;
      else
        into_reg = RISCV_IRET;
      if (Is12Bit(-next->integer)) {
        AIWNIOS_ADD_CODE(
            RISCV_ADDI(MIR(cctrl, into_reg), RISCV_REG_FP, -next->integer));
      } else {
        code_off = __ICMoveI64(cctrl, into_reg, -next->integer, bin, code_off);
        AIWNIOS_ADD_CODE(
            RISCV_ADD(MIR(cctrl, into_reg), RISCV_REG_FP, into_reg));
      }
      goto restore_reg;
      break;
    case IC_STATIC:
      if (rpn->res.mode == MD_REG)
        into_reg = rpn->res.reg;
      else
        into_reg = RISCV_IRET;
      code_off = __ICMoveI64(cctrl, into_reg, rpn->integer, bin, code_off);
      goto restore_reg;
    case IC_GLOBAL:
    get_glbl_ptr:
      if (rpn->res.mode == MD_REG)
        into_reg = rpn->res.reg;
      else
        into_reg = RISCV_IRET;
      if (next->global_var->base.type & HTT_GLBL_VAR) {
        enter_addr = next->global_var->data_addr;
      } else if (next->global_var->base.type & HTT_FUN) {
        enter_addr = ((CHashFun *)next->global_var)->fun_ptr;
      } else
        abort();
      // Undefined?
      if (!enter_addr || enter_addr == &DoNothing) {
        misc = AddRelocMisc(cctrl, next->global_var->base.str);
        AIWNIOS_ADD_CODE(RISCV_AUIPC(MIR(cctrl, into_reg), 2 << 12));
        AIWNIOS_ADD_CODE(RISCV_LD(MIR(cctrl, into_reg), into_reg, 0));
        CodeMiscAddRef(misc, bin + code_off - 8);
        goto restore_reg;
      }
      code_off =
          __ICMoveI64(cctrl, into_reg, (int64_t)enter_addr, bin, code_off);
    restore_reg:
      tmp.mode = MD_REG;
      tmp.raw_type = rpn->raw_type;
      tmp.reg = into_reg;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      break;
    default:
      abort();
    }
    break;
  ic_xor:
    if (rpn->raw_type == RT_F64) {
      FBITWISE(RISCV_XOR);
      break;
    }
    BACKEND_BINOP(RISCV_XOR, RISCV_XOR, RISCV_XORI);
    break;
  ic_mod:
    next = ICArgN(rpn, 1);
    next2 = ICArgN(rpn, 0);
    if (rpn->raw_type != RT_F64) {
      if (IsConst(next2) && rpn->raw_type == RT_U64i) {
        if (__builtin_popcountll(ConstVal(next2)) == 1) {
          if (__builtin_ffsll(ConstVal(next2)) <= 11) {
            orig_dst = rpn->res;
            code_off = __OptPassFinal(cctrl, next, bin, code_off);
            code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
            code_off = PutICArgIntoReg(cctrl, &rpn->res, RT_I64i, RISCV_IPOOP1,
                                       bin, code_off);
            AIWNIOS_ADD_CODE(RISCV_ANDI(
                MIR(cctrl, rpn->res.reg), rpn->res.reg,
                (1ll << (__builtin_ffsll(ConstVal(next2)) - 1)) - 1));
            code_off = ICMov(cctrl, &orig_dst, &rpn->res, bin, code_off);
            break;
          }
        }
      }
      if (rpn->raw_type == RT_U64i) {
        BACKEND_BINOP(RISCV_REMU, RISCV_REMU, DoNothing3);
      } else {
        BACKEND_BINOP(RISCV_REM, RISCV_REM, DoNothing3);
      }
    } else {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_F64, RISCV_FPOOP2, bin,
                                 code_off);
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_F64, RISCV_FPOOP1, bin,
                                 code_off);
      tmp2.reg = RISCV_FRET;
      tmp2.mode = MD_REG;
      tmp2.raw_type = RT_F64;
      AIWNIOS_ADD_CODE(
          RISCV_FDIV_D(MFR(cctrl, tmp2.reg), next->res.reg, next2->res.reg));
      tmp.reg = RISCV_IRET;
      tmp.mode = MD_REG;
      tmp.raw_type = RT_I64i;
      code_off = ICMov(cctrl, &tmp, &tmp2, bin, code_off);
      code_off = ICMov(cctrl, &tmp2, &tmp, bin, code_off);
      AIWNIOS_ADD_CODE(
          RISCV_FMUL_D(MFR(cctrl, RISCV_FPOOP2), tmp2.reg, next2->res.reg));
      AIWNIOS_ADD_CODE(
          RISCV_FSUB_D(MFR(cctrl, tmp2.reg), next->res.reg, RISCV_FPOOP2));
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
    }
    break;
  ic_or:
    if (rpn->raw_type == RT_F64) {
      FBITWISE(RISCV_OR);
    }
    BACKEND_BINOP(RISCV_OR, RISCV_OR, RISCV_ORI);
    break;
  ic_lt:
  ic_gt:
  ic_le:
  ic_ge:
    if (!old_fail_misc && !rpn->code_misc && bin)
      rpn->code_misc = CodeMiscNew(cctrl, CMT_LABEL);
    //
    // Ask nroot how this works if you want to know
    //
    // X/V3 is for lefthand side
    // X/V4 is for righthand side
    range = NULL;
    range_args = NULL;
    range_fail_addrs = NULL;
    range_cmp_types = NULL;
    range_fail_regs = NULL;
  get_range_items:
    cnt = 0;
    for (next = rpn; 1; next = ICFwd(next->base.next) // TODO explain
    ) {
      switch (next->type) {
      case IC_LT:
      case IC_GT:
      case IC_LE:
      case IC_GE:
        goto cmp_pass;
      }
      if (range_args)
        range_args[cnt] = next; // See below
      break;
    cmp_pass:
      //
      // <
      //    right rpn.next
      //    left  ICFwd(rpn.next)
      //
      if (range_args)
        range_args[cnt] = next->base.next;
      if (range)
        range[cnt] = next;
      cnt++;
    }
    if (!range) {
      // This are reversed
      range_args = A_MALLOC(sizeof(CRPN *) * (cnt + 1), cctrl->hc);
      range = A_MALLOC(sizeof(CRPN *) * cnt, cctrl->hc);
      range_fail_addrs = A_MALLOC(sizeof(CRPN *) * cnt, cctrl->hc);
      range_cmp_types = A_MALLOC(sizeof(int64_t) * cnt, cctrl->hc);
      range_fail_regs =
          A_MALLOC(sizeof(struct CRangeFailTuple) * cnt, cctrl->hc);
      goto get_range_items;
    }
    code_off = __OptPassFinal(cctrl, next = range_args[cnt], bin, code_off);
    for (i = cnt - 1; i >= 0; i--) {
      next2 = range_args[i];
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      memcpy(&tmp, &next->res, sizeof(CICArg));
      memcpy(&tmp2, &next2->res, sizeof(CICArg));
      use_flt_cmp = next->raw_type == RT_F64 || next2->raw_type == RT_F64;
      i2 = RT_I64i;
      i2 = next->res.raw_type > i2 ? next->res.raw_type : i2;
      i2 = next2->res.raw_type > i2 ? next2->res.raw_type : i2;
      code_off = PutICArgIntoReg(cctrl, &tmp, i2,
                                 use_flt_cmp ? RISCV_FPOOP2 : RISCV_IPOOP2, bin,
                                 code_off);
      code_off = PutICArgIntoReg(cctrl, &tmp2, i2,
                                 use_flt_cmp ? RISCV_FPOOP1 : RISCV_IPOOP1, bin,
                                 code_off);
      //
      // We use the opposite compare because if fail we jump to the fail
      // zone
      //
      if (use_flt_cmp) {
      unsigned_cmp:
        switch (range[i]->type) {
          break;
        case IC_LT:
          range_cmp_types[i] = RISCV_COND_B | 1;
          break;
        case IC_GT:
          range_cmp_types[i] = RISCV_COND_A | 1;
          break;
        case IC_LE:
          range_cmp_types[i] = RISCV_COND_BE | 1;
          break;
        case IC_GE:
          range_cmp_types[i] = RISCV_COND_AE | 1;
        }
      } else if (next->raw_type == RT_U64i || next2->raw_type == RT_U64i) {
        goto unsigned_cmp;
      } else {
        switch (range[i]->type) {
          break;
        case IC_LT:
          range_cmp_types[i] = RISCV_COND_L | 1;
          break;
        case IC_GT:
          range_cmp_types[i] = RISCV_COND_G | 1;
          break;
        case IC_LE:
          range_cmp_types[i] = RISCV_COND_LE | 1;
          break;
        case IC_GE:
          range_cmp_types[i] = RISCV_COND_GE | 1;
        }
      }
      range_fail_addrs[i] = bin + code_off;
      // BGE/Etc
      if (use_flt_cmp) {
        range_fail_regs[i].a = tmp.reg;
        range_fail_regs[i].b = tmp2.reg;
        range_fail_regs[i].is_flt = 1;
      } else {
        range_fail_regs[i].a = tmp.reg;
        range_fail_regs[i].b = tmp2.reg;
        range_fail_regs[i].is_flt = 0;
      }
      if (range_fail_regs[i].is_flt) {
        // We used RISCV_Fcc_D to set RISCV_IRET,just check if not 0
        code_off =
            RISCV_FJcc(cctrl, old_fail_misc ? old_fail_misc : rpn->code_misc,
                       range_cmp_types[i], tmp.reg, tmp2.reg, bin, code_off);

      } else {
        code_off = RISCV_JccToLabel(
            old_fail_misc ? old_fail_misc : rpn->code_misc, range_cmp_types[i],
            range_fail_regs[i].a, range_fail_regs[i].b, bin, code_off);
      }
      next = next2;
    }
    if (!(old_fail_misc && old_pass_misc)) {
      tmp.mode = MD_I64;
      tmp.integer = 1;
      tmp.raw_type = RT_I64i;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      exit_addr = bin + code_off;
      AIWNIOS_ADD_CODE(0);
      if (rpn->code_misc) {
        rpn->code_misc->addr = bin + code_off;
        rpn->code_misc->code_off = code_off;
      }
      tmp.mode = MD_I64;
      tmp.integer = 0;
      tmp.raw_type = RT_I64i;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      if (bin) {
        *(int32_t *)(exit_addr) =
            RISCV_JAL(0, (char *)(bin + code_off) - (char *)exit_addr);
      }
    } else {
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
      CodeMiscAddRef(old_pass_misc, bin + code_off - 4)->is_jal = 1;
    }
    A_FREE(range);
    A_FREE(range_args);
    A_FREE(range_fail_addrs);
    A_FREE(range_fail_regs);
    A_FREE(range_cmp_types);
    break;
  ic_lnot:
    if (rpn->res.mode == MD_REG)
      into_reg = rpn->res.reg;
    else
      into_reg = RISCV_IRET;
    next = rpn->base.next;
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    code_off = PutICArgIntoReg(
        cctrl, &next->res, next->res.raw_type,
        next->res.raw_type == RT_F64 ? RISCV_FRET : RISCV_IRET, bin, code_off);
    if (old_pass_misc && old_fail_misc && next->res.raw_type != RT_F64) {
      code_off = RISCV_JccToLabel(old_pass_misc, RISCV_COND_E, next->res.reg, 0,
                                  bin, code_off);
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
      if (bin)
        CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->is_jal = 1;
      goto ret;
    } else if (old_pass_misc && old_fail_misc && next->res.raw_type == RT_F64) {
      code_off = __ICMoveF64(cctrl, RISCV_FPOOP1, 0, bin, code_off);
      code_off = RISCV_FJcc(cctrl, old_pass_misc, RISCV_COND_E, next->res.reg,
                            RISCV_FPOOP1, bin, code_off);
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
      if (bin)
        CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->is_jal = 1;
      goto ret;
    }
    if (next->res.raw_type != RT_F64) {
      AIWNIOS_ADD_CODE(RISCV_SLTIU(into_reg, next->res.reg, 1));
    } else {
      code_off = __ICMoveF64(cctrl, RISCV_FPOOP1, 0, bin, code_off);
      // f==0.
      AIWNIOS_ADD_CODE(RISCV_FEQ_D(into_reg, next->res.reg, RISCV_FPOOP1));
    }
    tmp.mode = MD_REG;
    tmp.reg = into_reg;
    tmp.raw_type = RT_I64i;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_bnot:
    BACKEND_UNOP(RISCV_NOT, RISCV_NOT);
    break;
  ic_post_inc:
    code_off = __SexyPostOp(cctrl, rpn, RISCV_ADDI, RISCV_ADD, RISCV_FADD_D,
                            bin, code_off);
    break;
  ic_post_dec:
    code_off = __SexyPostOp(cctrl, rpn, RISCV_SUBI, RISCV_SUB, RISCV_FSUB_D,
                            bin, code_off);
    break;
  ic_pre_inc:
    code_off = __SexyPreOp(cctrl, rpn, RISCV_ADDI, RISCV_ADD, RISCV_FADD_D, bin,
                           code_off);
    break;
  ic_pre_dec:
    code_off = __SexyPreOp(cctrl, rpn, RISCV_SUBI, RISCV_SUB, RISCV_FSUB_D, bin,
                           code_off);
    break;
  ic_and_and:
#define F64_TO_BOOL_IF_NEEDED(_mode, fallback)                                 \
  {                                                                            \
    if ((_mode)->raw_type == RT_F64) {                                         \
      CICArg tmp;                                                              \
      tmp.mode = MD_REG;                                                       \
      tmp.reg = fallback;                                                      \
      tmp.raw_type = RT_I64i;                                                  \
      code_off = __ICMoveF64(cctrl, RISCV_FPOOP2, 0, bin, code_off);           \
      code_off = PutICArgIntoReg(cctrl, (_mode), RT_F64, RISCV_FPOOP1, bin,    \
                                 code_off);                                    \
      AIWNIOS_ADD_CODE(                                                        \
          RISCV_FEQ_D(MIR(cctrl, fallback), (_mode)->reg, RISCV_FPOOP2));      \
      AIWNIOS_ADD_CODE(RISCV_SLTIU(MIR(cctrl, fallback), fallback, 1));        \
      *(_mode) = tmp;                                                          \
    }                                                                          \
  }
    if (old_pass_misc && old_fail_misc) {
      a = ICArgN(rpn, 1);
      b = ICArgN(rpn, 0);
      code_off = __OptPassFinal(cctrl, a, bin, code_off);
      code_off = PutICArgIntoReg(
          cctrl, &a->res, a->res.raw_type == RT_F64 ? RT_F64 : RT_I64i,
          a->res.raw_type == RT_F64 ? RISCV_FRET : RISCV_IRET, bin, code_off);
      F64_TO_BOOL_IF_NEEDED(&a->res, RISCV_IPOOP1);
      code_off = RISCV_JccToLabel(old_fail_misc, RISCV_COND_E, a->res.reg, 0,
                                  bin, code_off);
      code_off = __OptPassFinal(cctrl, b, bin, code_off);
      code_off = PutICArgIntoReg(
          cctrl, &b->res, b->res.raw_type == RT_F64 ? RT_F64 : RT_I64i,
          b->res.raw_type == RT_F64 ? RISCV_FRET : RISCV_IRET, bin, code_off);
      F64_TO_BOOL_IF_NEEDED(&b->res, RISCV_IPOOP2);
      code_off = RISCV_JccToLabel(old_fail_misc, RISCV_COND_E, b->res.reg, 0,
                                  bin, code_off);
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
      CodeMiscAddRef(old_pass_misc, bin + code_off - 4)->is_jal = 1;
      goto ret;
    }
    if (!rpn->code_misc)
      rpn->code_misc = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc2)
      rpn->code_misc2 = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc3)
      rpn->code_misc3 = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc4)
      rpn->code_misc4 = CodeMiscNew(cctrl, CMT_LABEL);
    a = ICArgN(rpn, 1);
    b = ICArgN(rpn, 0);
    cctrl->backend_user_data5 = (int64_t)rpn->code_misc;
    cctrl->backend_user_data6 = (int64_t)rpn->code_misc2;
    code_off = __OptPassFinal(cctrl, a, bin, code_off);
    cctrl->backend_user_data6 = (int64_t)rpn->code_misc3;
    rpn->code_misc2->addr = bin + code_off;
    rpn->code_misc2->code_off = code_off;
    code_off = __OptPassFinal(cctrl, b, bin, code_off);
    rpn->code_misc->addr = bin + code_off;
    rpn->code_misc->code_off = code_off;
    tmp.mode = MD_I64;
    tmp.raw_type = RT_I64i;
    tmp.integer = 0;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
    CodeMiscAddRef(rpn->code_misc4, bin + code_off - 4)->is_jal = 1;
    rpn->code_misc3->addr = bin + code_off;
    rpn->code_misc3->code_off = code_off;
    tmp.integer = 1;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    rpn->code_misc4->addr = bin + code_off;
    rpn->code_misc4->code_off = code_off;
    break;
  ic_or_or:
    if (old_pass_misc || old_fail_misc) {
      a = ICArgN(rpn, 1);
      b = ICArgN(rpn, 0);
      code_off = __OptPassFinal(cctrl, a, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &a->res, a->res.raw_type, RISCV_IRET,
                                 bin, code_off);
      F64_TO_BOOL_IF_NEEDED(&a->res, RISCV_IPOOP1);
      code_off = RISCV_JccToLabel(old_pass_misc, RISCV_COND_E ^ 1, a->res.reg,
                                  0, bin, code_off);
      code_off = __OptPassFinal(cctrl, b, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &b->res, b->res.raw_type, RISCV_IRET,
                                 bin, code_off);
      F64_TO_BOOL_IF_NEEDED(&b->res, RISCV_IPOOP1);
      code_off = RISCV_JccToLabel(old_pass_misc, RISCV_COND_E ^ 1, b->res.reg,
                                  0, bin, code_off);
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
      CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->is_jal = 1;
      goto ret;
    }
    // A
    // rpn->code_misc3:
    // B
    // rpn->code_misc4:
    // res=0;
    // JMP rpn->code_misc2
    // rpn->code_misc:
    // res=1
    // rpn->code_misc2:
    if (!rpn->code_misc)
      rpn->code_misc = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc2)
      rpn->code_misc2 = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc3)
      rpn->code_misc3 = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc4)
      rpn->code_misc4 = CodeMiscNew(cctrl, CMT_LABEL);
    a = ICArgN(rpn, 1);
    b = ICArgN(rpn, 0);
    cctrl->backend_user_data5 = (int64_t)rpn->code_misc3;
    cctrl->backend_user_data6 = (int64_t)rpn->code_misc;
    code_off = __OptPassFinal(cctrl, a, bin, code_off);
    rpn->code_misc3->addr = bin + code_off;
    rpn->code_misc3->code_off = code_off;
    cctrl->backend_user_data5 = (int64_t)rpn->code_misc4;
    code_off = __OptPassFinal(cctrl, b, bin, code_off);
    rpn->code_misc4->addr = bin + code_off;
    rpn->code_misc4->code_off = code_off;
    tmp.mode = MD_I64;
    tmp.integer = 0;
    tmp.raw_type = RT_I64i;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
    CodeMiscAddRef(rpn->code_misc2, bin + code_off - 4)->is_jal = 1;
    rpn->code_misc->addr = bin + code_off;
    rpn->code_misc->code_off = code_off;
    tmp.integer = 1;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    rpn->code_misc2->addr = bin + code_off;
    rpn->code_misc2->code_off = code_off;
    break;
  ic_xor_xor:
#define BACKEND_LOGICAL_BINOP(op)                                              \
  next = ICArgN(rpn, 1);                                                       \
  next2 = ICArgN(rpn, 0);                                                      \
  code_off = __OptPassFinal(cctrl, next2, bin, code_off);                      \
  code_off = __OptPassFinal(cctrl, next, bin, code_off);                       \
  if (rpn->res.mode == MD_REG) {                                               \
    into_reg = rpn->res.reg;                                                   \
  } else                                                                       \
    into_reg = RISCV_IRET;                                                     \
  code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, RISCV_IPOOP2,   \
                             bin, code_off);                                   \
  code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type, RISCV_IPOOP1,  \
                             bin, code_off);                                   \
  F64_TO_BOOL_IF_NEEDED(&next->res, RISCV_IPOOP1);                             \
  F64_TO_BOOL_IF_NEEDED(&next2->res, RISCV_IPOOP2);                            \
  AIWNIOS_ADD_CODE(RISCV_SLTU(MIR(cctrl, RISCV_IPOOP2), 0, next->res.reg));    \
  AIWNIOS_ADD_CODE(RISCV_SLTU(MIR(cctrl, RISCV_IPOOP1), 0, next2->res.reg));   \
  AIWNIOS_ADD_CODE(op(into_reg, RISCV_IPOOP1, RISCV_IPOOP2));                  \
  if (rpn->res.mode != MD_REG) {                                               \
    tmp.raw_type = rpn->raw_type;                                              \
    tmp.reg = RISCV_IRET;                                                      \
    tmp.mode = MD_REG;                                                         \
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                   \
  }
    BACKEND_LOGICAL_BINOP(RISCV_XOR);
    break;
  ic_eq_eq:
#define BACKEND_CMP(cond)                                                      \
  next = ICArgN(rpn, 1);                                                       \
  next2 = ICArgN(rpn, 0);                                                      \
  code_off = __OptPassFinal(cctrl, next, bin, code_off);                       \
  code_off = __OptPassFinal(cctrl, next2, bin, code_off);                      \
  i = RT_I64i;                                                                 \
  if (next2->res.raw_type == RT_F64 || next->res.raw_type == RT_F64)           \
    i = RT_F64;                                                                \
  code_off = PutICArgIntoReg(cctrl, &next2->res, i,                            \
                             (i == RT_F64) ? RISCV_FPOOP1 : RISCV_IPOOP1, bin, \
                             code_off);                                        \
  code_off = PutICArgIntoReg(cctrl, &next->res, i,                             \
                             (i == RT_F64) ? RISCV_FPOOP2 : RISCV_IPOOP2, bin, \
                             code_off);                                        \
  if (old_pass_misc && old_fail_misc) {                                        \
    if (i != RT_F64) {                                                         \
      code_off = RISCV_JccToLabel(old_pass_misc, cond, next->res.reg,          \
                                  next2->res.reg, bin, code_off);              \
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));                                       \
      CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->is_jal = 1;           \
    } else {                                                                   \
      AIWNIOS_ADD_CODE(                                                        \
          RISCV_FEQ_D(RISCV_IRET, next->res.reg, next2->res.reg));             \
      code_off = RISCV_JccToLabel(old_pass_misc, (cond) ^ 1, RISCV_IRET, 0,    \
                                  bin, code_off);                              \
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));                                       \
      CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->is_jal = 1;           \
    }                                                                          \
  } else {                                                                     \
    if (i != RT_F64) {                                                         \
      AIWNIOS_ADD_CODE(RISCV_SUB(RISCV_IRET, next->res.reg, next2->res.reg));  \
      if ((cond) == RISCV_COND_E) {                                            \
        AIWNIOS_ADD_CODE(RISCV_SLTIU(MIR(cctrl, RISCV_IRET), RISCV_IRET, 1));  \
      } else                                                                   \
        AIWNIOS_ADD_CODE(RISCV_SLTU(MIR(cctrl, RISCV_IRET), 0, RISCV_IRET));   \
    } else {                                                                   \
      AIWNIOS_ADD_CODE(                                                        \
          RISCV_FEQ_D(MIR(cctrl, RISCV_IRET), next->res.reg, next2->res.reg)); \
      if ((cond) == (RISCV_COND_E | 1)) {                                      \
        AIWNIOS_ADD_CODE(RISCV_SLTIU(MIR(cctrl, RISCV_IRET), RISCV_IRET, 1));  \
      }                                                                        \
    }                                                                          \
    tmp.mode = MD_REG, tmp.reg = RISCV_IRET, tmp.raw_type = RT_I64i;           \
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                   \
  }
    BACKEND_CMP(RISCV_COND_E);
    break;
  ic_ne:
    BACKEND_CMP(RISCV_COND_E ^ 1);
    break;
  ic_lsh:
    if (rpn->raw_type != RT_F64) {
      next2 = ICArgN(rpn, 0);
      next = ICArgN(rpn, 1);
      into_reg = RISCV_IPOOP1;
      if (rpn->res.mode == MD_REG) {
        into_reg = rpn->res.reg;
      }
      if (IsConst(next2) && Is12Bit(ConstVal(next2))) {
        code_off = __OptPassFinal(cctrl, next, bin, code_off);
        code_off = PutICArgIntoReg(cctrl, &next->res, RT_U64i, RISCV_IPOOP2,
                                   bin, code_off);
        AIWNIOS_ADD_CODE(
            RISCV_SLLI(MIR(cctrl, into_reg), next->res.reg, ConstVal(next2)));
        goto lsfin;
      }
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_U64i, RISCV_IPOOP1, bin,
                                 code_off);
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_U64i, RISCV_IPOOP2, bin,
                                 code_off);
      AIWNIOS_ADD_CODE(
          RISCV_SLL(MIR(cctrl, into_reg), next->res.reg, next2->res.reg));
    lsfin:
      tmp2.mode = MD_REG;
      tmp2.raw_type = RT_I64i;
      tmp2.reg = into_reg;
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
    } else {
      // TODO
    }
    break;
  ic_rsh:;
    int64_t _signed = 0;
    switch (rpn->raw_type) {
    case RT_I8i:
    case RT_I16i:
    case RT_I32i:
    case RT_I64i:
      _signed = 1;
    }
    into_reg = RISCV_IPOOP1;
    if (rpn->res.mode == MD_REG) {
      into_reg = rpn->res.reg;
    }
    if (rpn->raw_type != RT_F64) {
      next2 = ICArgN(rpn, 0);
      next = ICArgN(rpn, 1);
      if (IsConst(next2) && Is12Bit(ConstVal(next2))) {
        code_off = __OptPassFinal(cctrl, next, bin, code_off);
        code_off = PutICArgIntoReg(cctrl, &next->res, RT_U64i, RISCV_IPOOP2,
                                   bin, code_off);
        if (_signed)
          AIWNIOS_ADD_CODE(
              RISCV_SRAI(MIR(cctrl, into_reg), next->res.reg, ConstVal(next2)))
        else
          AIWNIOS_ADD_CODE(
              RISCV_SRLI(MIR(cctrl, into_reg), next->res.reg, ConstVal(next2)))
        goto rsfin;
      }
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_U64i, RISCV_IPOOP1, bin,
                                 code_off);
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_U64i, RISCV_IPOOP2, bin,
                                 code_off);
      if (_signed)
        AIWNIOS_ADD_CODE(
            RISCV_SRA(MIR(cctrl, into_reg), next->res.reg, next2->res.reg))
      else
        AIWNIOS_ADD_CODE(
            RISCV_SRL(MIR(cctrl, into_reg), next->res.reg, next2->res.reg))
    rsfin:
      tmp2.mode = MD_REG;
      tmp2.raw_type = RT_I64i;
      tmp2.reg = into_reg;
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
    } else {
      // TODO
    }
    break;
  ic_call:
    //
    // ()_()_()_()
    // /          \     // When we call a function,all temp registers
    // | O______O |     // are invalidated,so we can reset the temp register
    // | \______/ |===> // counts (for the new expressions).
    // \__________/     //
    //   \./   \./      // We will restore the temp register counts after the
    //   call
    //
    i = cctrl->backend_user_data2;
    i2 = cctrl->backend_user_data3;
    cctrl->backend_user_data2 = 0;
    cctrl->backend_user_data3 = 0;
    code_off = __ICFCallTOS(cctrl, rpn, bin, code_off);
    cctrl->backend_user_data2 = i;
    cctrl->backend_user_data3 = i2;
    break;
  ic_vargs:
    //  ^^^^^^^^
    // (        \      ___________________
    // ( (*) (*)|     /                   \
    //  \   ^   |    |__IFCall            |
    //   \  <>  | <==| Will unwind for you| //TODO validate for X86_64
    //    \    /      \__________________/
    //      \_/
    AIWNIOS_ADD_CODE(RISCV_ADDI(RISCV_REG_SP, RISCV_REG_SP, -8 * rpn->length));
    for (i = 0; i != rpn->length; i++) {
      next = ICArgN(rpn, rpn->length - i - 1);
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      tmp.reg = RISCV_REG_SP;
      tmp.off = 8 * i;
      tmp.mode = MD_INDIR_REG;
      tmp.raw_type = next->raw_type;
      if (tmp.raw_type < RT_I64i)
        tmp.raw_type = RT_I64i;
      code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);
    }
    break;
  ic_add_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_sub_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_mul_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_div_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_lsh_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_rsh_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_and_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_or_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_xor_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_mod_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_i64:
    tmp.mode = MD_I64;
    tmp.raw_type = RT_I64i;
    tmp.integer = rpn->integer;
    if (rpn->res.mode != MD_I64)
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_f64:
    tmp.mode = MD_F64;
    tmp.raw_type = RT_F64;
    tmp.flt = rpn->flt;
    if (rpn->res.mode != MD_F64)
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_array_acc:
    abort();
    break;
  ic_ret:
    next = ICArgN(rpn, 0);
    PushTmpDepthFirst(cctrl, next, 0);
    PopTmp(cctrl, next);
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    if (cctrl->cur_fun)
      tmp.raw_type = cctrl->cur_fun->return_class->raw_type;
    else if (rpn->ic_class) {
      // No return type so just return what we have
      tmp.raw_type = rpn->ic_class->raw_type;
    } else
      // No return type so just return what we have
      tmp.raw_type = next->raw_type;
    if (tmp.raw_type == RT_F64)
      tmp.reg = RISCV_FRET;
    else
      tmp.reg = RISCV_IRET;
    tmp.mode = MD_REG;
    code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);
    // TempleOS will always store F64 result in integer register
    // Let's merge the two togheter
    if (tmp.raw_type == RT_F64) {
      AIWNIOS_ADD_CODE(RISCV_FMV_X_D(MIR(cctrl, RISCV_IRET), RISCV_FRET));
    } else {
      // Vise versa
      AIWNIOS_ADD_CODE(RISCV_FMV_D_X(MIR(cctrl, RISCV_IRET), RISCV_FRET));
    }
    // TODO  jump to return area,not generate epilog for each poo poo
    AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
    CodeMiscAddRef(cctrl->epilog_label, bin + code_off - 4)->is_jal = 1;
    break;
  ic_base_ptr:
    tmp.raw_type = rpn->raw_type;
    tmp.off = rpn->integer;
    tmp.reg = RISCV_REG_FP;
    tmp.mode = MD_FRAME;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    rpn->res = tmp;
    break;
  ic_static:
    tmp.raw_type = rpn->raw_type;
    tmp.off = rpn->integer;
    tmp.mode = MD_PTR;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    rpn->res = tmp;
    break;
  ic_ireg:
    tmp.raw_type = RT_I64i;
    tmp.reg = rpn->integer;
    tmp.mode = MD_REG;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    rpn->res = tmp;
    break;
  ic_freg:
    tmp.raw_type = RT_F64;
    tmp.reg = rpn->integer;
    tmp.mode = MD_REG;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    rpn->res = tmp;
    break;
  ic_bt:
  ic_btc:
  ic_bts:
  ic_btr:
  ic_lbtc:
  ic_lbts:
  ic_lbtr:
  ic_sign_i64:
  ic_sqr_i64:
  ic_sqr_u64:
  ic_sqr:
  ic_abs:
  ic_sqrt:
  ic_cos:
  ic_sin:
  ic_tan:
  ic_atan:
    /*break;case IC_MAX_I64:
    break;case IC_MIN_I64:
    break;case IC_MAX_U64:
    break;case IC_MIN_U64:
    break;case IC_SIGN_I64:
    break;case IC_SQR_I64:
    break;case IC_SQR_U64:
    break;case IC_SQR:
    break;case IC_ABS:
    break;case IC_SQRT:
    break;case IC_SIN:
    break;case IC_COS:
    break;case IC_TAN:
    break;case IC_ATAN:*/
    break;
  ic_static_ref:
    tmp.raw_type = rpn->raw_type;
    tmp.off = rpn->integer;
    tmp.mode = MD_STATIC;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_set_static_data:
    // TODO later
    break;
  ic_raw_bytes:
    if (cctrl->code_ctrl->final_pass) {
      memcpy(bin + code_off, rpn->raw_bytes, rpn->length);
    }
    code_off += rpn->length;
  } while (0);
ret:
  if (old_fail_misc || old_pass_misc) {
    misc = old_fail_misc;
    misc2 = old_pass_misc;
    switch (rpn->type) {
    case IC_OR_OR:
    case IC_AND_AND:
    case IC_LT:
    case IC_GT:
    case IC_GE:
    case IC_LE:
    case IC_EQ_EQ:
    case IC_NE:
    case IC_LNOT:
    case IC_BT:
    case IC_BTS:
    case IC_BTR:
    case IC_BTC:
    case IC_LBTS:
    case IC_LBTR:
    case IC_LBTC:
      break;
    default:
      if (rpn->res.raw_type == RT_F64) {
        code_off = PutICArgIntoReg(cctrl, &rpn->res, RT_F64, RISCV_FPOOP1, bin,
                                   code_off);
        code_off = __ICMoveF64(cctrl, RISCV_FPOOP2, 0, bin, code_off);
        AIWNIOS_ADD_CODE(
            RISCV_FEQ_D(MIR(cctrl, RISCV_IRET), RISCV_FPOOP2, rpn->res.reg));
        // Pass address(iret==TRUE if is 0)
        code_off = RISCV_JccToLabel(old_pass_misc, RISCV_COND_E, RISCV_IRET, 0,
                                    bin, code_off);
      } else {
        tmp = rpn->res;
        code_off =
            PutICArgIntoReg(cctrl, &tmp, RT_I64i, RISCV_IRET, bin, code_off);
        // Pass address
        code_off = RISCV_JccToLabel(old_pass_misc, RISCV_COND_E ^ 1, tmp.reg, 0,
                                    bin, code_off);
      }
      // Fail address
      AIWNIOS_ADD_CODE(RISCV_JAL(0, 0));
      CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->is_jal = 1;
    }
  }
  if (rpn->flags & ICF_STUFF_IN_REG) {
    tmp = rpn->res;
    if (rpn->res.raw_type != RT_F64)
      rpn->res.raw_type = RT_I64i;
    else
      rpn->res.raw_type = RT_F64;
    rpn->res.mode = MD_REG;
    rpn->res.reg = rpn->stuff_in_reg;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
  }
  if (old_rpn) {
    old_rpn->changes_iregs2 |= cctrl->cur_rpn->changes_iregs;
    old_rpn->changes_fregs2 |= cctrl->cur_rpn->changes_fregs;
  }
  cctrl->cur_rpn = old_rpn;
  cctrl->backend_user_data5 = (int64_t)old_fail_misc;
  cctrl->backend_user_data6 = (int64_t)old_pass_misc;
  return code_off;
}
// This dude calls __OptPassFinal 3 times.
// 1. Get size of WORST CASE compiled body,and generate any extra needed
// CCodeMisc's
//    This pass  will also asign CRPN->res with tmp registers/FRAME offsets
// 2. Fill in function body,accounting for not worst case jumps
// 3. Fill in the poo poo's
//
char *OptPassFinal(CCmpCtrl *cctrl, int64_t *res_sz, char **dbg_info,
                   CHeapCtrl *heap) {
  int64_t code_off, run, idx, cnt = 0, cnt2, final_size, is_terminal;
  int64_t min_ln = 0, max_ln = 0, statics_sz = 0;
  char *bin = NULL;
  char *ptr;
  CCodeMisc *misc;
  CCodeMiscRef *cm_ref, *cm_ref_tmp;
  CHashImport *import;
  CRPN *r;
  CacheRPNArgs(cctrl);
  cctrl->fregs_restore_label = CodeMiscNew(cctrl, CMT_LABEL);
  cctrl->fregs_save_label = CodeMiscNew(cctrl, CMT_LABEL);
  cctrl->epilog_label = CodeMiscNew(cctrl, CMT_LABEL);
  cctrl->statics_label = CodeMiscNew(cctrl, CMT_LABEL);
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = r->base.next) {
    if (r->ic_line) {
      if (!min_ln)
        min_ln = r->ic_line;
      min_ln = (min_ln < r->ic_line) ? min_ln : r->ic_line;
      if (!max_ln)
        max_ln = r->ic_line;
      max_ln = (max_ln > r->ic_line) ? max_ln : r->ic_line;
    }
    if (r->type == IC_BASE_PTR) {
    } else if (r->type == __IC_STATICS_SIZE)
      statics_sz = r->integer;
  }
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = ICFwd(r)) {
    cnt++;
  }
  if (dbg_info) {
    // Dont allocate on cctrl->hc heap ctrl as we want to share our datqa
    cctrl->code_ctrl->dbg_info = dbg_info;
    cctrl->code_ctrl->min_ln = min_ln;
  }
  CRPN *forwards[cnt2 = cnt];
  cnt = 0;
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = ICFwd(r)) {
    forwards[cnt2 - ++cnt] = r;
  }
  // We DONT reset the wiggle room
  cctrl->backend_user_data0 = 16;
  // 0 get maximum code size,
  // 1 compute bin
  for (run = 0; run < 2; run++) {
    cctrl->code_ctrl->final_pass = run;
    cctrl->backend_user_data1 = 0;
    cctrl->backend_user_data2 = 0;
    cctrl->backend_user_data3 = 0;
    if (run == 0) {
      code_off = 0;
      bin = NULL;
    } else if (run == 1) {
      // Second run,be sure to set SetKeepTmps(we see the changed reigsters in
      // first pass)
      for (r = cctrl->code_ctrl->ir_code->last; r != cctrl->code_ctrl->ir_code;
           r = r->base.last) {
        // Do first(REVERSE polish notation) things first
        SetKeepTmps(r);
      }
      // Dont allocate on cctrl->hc heap ctrl as we want to share our data
      bin = A_MALLOC(1024 + code_off, heap ? heap : Fs->code_heap);
      code_off = 0;
    }
    code_off = FuncProlog(cctrl, bin, code_off);
    for (cnt = 0; cnt < cnt2; cnt++) {
    enter:
      r = forwards[cnt];
      if (PushTmpDepthFirst(cctrl, r, 0))
        PopTmp(cctrl, r);
      // These modes expect you run ->__sib_base_rpn/__sib_index_rpn
      if (r->res.mode != __MD_X86_64_LEA_SIB &&
          r->res.mode != __MD_X86_64_SIB) {
        r->res.mode = MD_NULL;
      }
      cctrl->backend_user_data1 = 0;
      cctrl->backend_user_data2 = 0;
      cctrl->backend_user_data3 = 0;
      code_off = __OptPassFinal(cctrl, r, bin, code_off);
      // assert(cctrl->backend_user_data1 == 0);
      // assert(cctrl->backend_user_data2 == 0);
      // assert(cctrl->backend_user_data3 == 0);
      if (IsTerminalInst(r)) {
        cnt++;
        for (; cnt < cnt2; cnt++) {
          if (forwards[cnt]->type == IC_LABEL)
            goto enter;
        }
      }
    }
    cctrl->epilog_label->addr = bin + code_off;
    cctrl->epilog_label->code_off = code_off;
    code_off = FuncEpilog(cctrl, bin, code_off);
    for (misc = cctrl->code_ctrl->code_misc->next;
         misc != cctrl->code_ctrl->code_misc; misc = misc->base.next) {
      int64_t old_code_off = code_off;
      switch (misc->type) {
        break;
      case CMT_INT_CONST:
        if (code_off % 8)
          code_off += 8 - code_off % 8;
        misc->addr = bin + code_off;
        if (bin)
          *(int64_t *)(bin + code_off) = misc->integer;
        code_off += 8;
      fill_in_refs:
#define FILL_IN_REFS                                                           \
  for (cm_ref = misc->refs; cm_ref; cm_ref = cm_ref_tmp) {                     \
    cm_ref_tmp = cm_ref->next;                                                 \
    if (run) {                                                                 \
      if (cm_ref->is_abs64) {                                                  \
        if (misc->patch_addr)                                                  \
          *misc->patch_addr = cm_ref->add_to;                                  \
      } else if (cm_ref->is_4_bytes) {                                         \
        idx = (char *)misc->addr - (char *)cm_ref->add_to + cm_ref->offset;    \
        if (idx & 3)                                                           \
          abort();                                                             \
        int64_t b12 = (idx >> 12) & 1;                                         \
        int64_t b11 = (idx >> 11) & 1;                                         \
        int64_t b4_1 = (idx >> 1) & ((1 << 4) - 1);                            \
        int64_t b10_5 = (idx >> 5) & ((1 << 6) - 1);                           \
        *(int32_t *)(cm_ref->add_to) |= (b12 << 31) | (b10_5 << 25) |          \
                                        (b4_1 << 8) | (b4_1 << 8) |            \
                                        (b11 << 7);                            \
      } else if (cm_ref->is_jal) {                                             \
        idx = (char *)misc->addr - (char *)cm_ref->add_to + cm_ref->offset;    \
        int64_t b19_12 = (idx >> 12) & ((1 << (19 - 12 + 1)) - 1);             \
        int64_t b11 = (idx >> 11) & 1;                                         \
        int64_t b20 = (idx >> 20) & 1;                                         \
        int64_t b10_1 = (idx >> 1) & ((1 << 10) - 1);                          \
        *(int32_t *)(cm_ref->add_to) |=                                        \
            (b20 << 31) | (b10_1 << 21) | (b11 << 20) | (b19_12 << 12);        \
      } else {                                                                 \
        int64_t low12;                                                         \
        idx = (char *)misc->addr - (char *)cm_ref->add_to + cm_ref->offset;    \
        *(int32_t *)(cm_ref->add_to) &= 0xfff;                                 \
        low12 = idx - (idx & ~((1 << 12) - 1));                                \
        if (Is12Bit(low12)) { /*Chekc for bit 12 being set*/                   \
          *(int32_t *)(cm_ref->add_to) |= (idx >> 12) << 12;                   \
          *(int32_t *)((char *)cm_ref->add_to + 4) |= low12 << 20;             \
        } else {                                                               \
          *(int32_t *)(cm_ref->add_to) |= ((idx >> 12) + 1) << 12;             \
          *(int32_t *)((char *)cm_ref->add_to + 4) |= low12 << 20;             \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    A_FREE(cm_ref);                                                            \
  }                                                                            \
  misc->refs = NULL;
        FILL_IN_REFS;
        break;
      case CMT_FLOAT_CONST:
        if (code_off % 8)
          code_off += 8 - code_off % 8;
        misc->addr = bin + code_off;
        if (bin)
          *(double *)(bin + code_off) = misc->flt;
        code_off += 8;
        goto fill_in_refs;
        break;
      case CMT_JMP_TAB:
        if (code_off % 8)
          code_off += 8 - code_off % 8;
        misc->addr = bin + code_off;
        if (misc->patch_addr)
          *misc->patch_addr = misc->addr;
        if (bin) {
          for (idx = 0; idx <= misc->hi - misc->lo; idx++)
            *(void **)(bin + code_off + idx * 8) =
                (char *)misc->jmp_tab[idx]->addr;
        }
        code_off += (misc->hi - misc->lo + 1) * 8;
        goto fill_in_refs;
        break;
      case CMT_LABEL:
        if (misc->patch_addr)
          *misc->patch_addr = misc->addr;
        // We assign the statics offset later
        if (misc == cctrl->statics_label)
          break;
        goto fill_in_refs;
      case CMT_SHORT_ADDR:
        if (run && misc->patch_addr) {
          *misc->patch_addr = misc->addr;
        }
        break;
      case CMT_RELOC_U64:
        if (code_off % 8)
          code_off += 8 - code_off % 8;
        misc->addr = bin + code_off;
        if (bin)
          *(void **)(bin + code_off) = &DoNothing;
        code_off += 8;
        if (run) {
          *(import = A_CALLOC(sizeof(CHashImport), NULL)) = (CHashImport){
              .base =
                  {
                      .type = HTT_IMPORT_SYS_SYM,
                      .str = A_STRDUP(misc->str, NULL),
                  },
              .address = misc->addr,
          };
          HashAdd(import, Fs->hash_table);
        }
        if (run && misc->patch_addr) {
          *misc->patch_addr = misc->addr;
        }
        goto fill_in_refs;
        break;
      case CMT_STRING:
        if (!(cctrl->flags & CCF_STRINGS_ON_HEAP)) {
          if (code_off % 8)
            code_off += 8 - code_off % 8;
          misc->addr = bin + code_off;
          if (bin)
            memcpy(bin + code_off, misc->str, misc->str_len);
          code_off += misc->str_len;
        } else {
          if (run) {
            // See IC_STR: We "steal" the ->str because it's already on the
            // heap.
            //  IC_STR steals the ->str point so we dont want to Free it
            misc->str = NULL;
          }
        }
        goto fill_in_refs;
      }
    }
    if (code_off % 8) // Align to 8
      code_off += 8 - code_off % 8;
    cctrl->code_ctrl->statics_offset = code_off;
    cctrl->statics_label->addr = bin + cctrl->code_ctrl->statics_offset;
    // Fill in the static references
    if (bin)
      for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
           r = r->base.next) {
        if (r->type == __IC_SET_STATIC_DATA) {
          memcpy(bin + cctrl->code_ctrl->statics_offset + r->code_misc->integer,
                 r->code_misc->str, r->code_misc->str_len);
        }
      }
    if (statics_sz)
      code_off += statics_sz + 8;
    misc = cctrl->statics_label;
    if (bin)
      FILL_IN_REFS;
  }
  final_size = code_off;
  if (dbg_info) {
    cnt = MSize(dbg_info) / 8;
    dbg_info[0] = bin;
  }
  if (res_sz)
    *res_sz = final_size;
  __builtin___clear_cache(bin, bin + MSize(bin));
  return bin;
}
