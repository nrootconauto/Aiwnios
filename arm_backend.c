#pragma once
#include "aiwn.h"
static int64_t SpillsTmpRegs(CRPN* rpn);
static int64_t ICMov(CCmpCtrl* cctrl, CICArg* dst, CICArg* src, char* bin,
	int64_t code_off);
//
// ASSci art
// POOPING README
//     _^_
//    {   }        ______
//   {     }      /      \
//  {(o) (o)} <==( README )
// { \_____/ }    \______/
// \_________/
//
// In parser.c I defined some backend_user_dataX varaibles
// **backend_user_data0** is the maximum extra stack wiggle room^ that is
// used for random poop the program may need(if we run out of tmp regs)
//
// **backend_user_data1** is the current wiggle room^ size
// **backend_user_data2** is the current tmp int reg(overflow goes to stack)
// **backend_user_data3** is the current tmp float reg(overflow goes to stack)
// **backend_user_data4** is wiggle room start
// Stack layout
//  old X29
//  old X30
//  Pushed saved registers
//  Wiggle room
//  locals
//

//
// [^]Wiggle room means room on the stack that is reserved
//

//
// December 5,2022 Sachi tried to call me like 3 times in the morning
// December 5,2022 I need to make a base class set the raw_type
// December 9,2022 I thought about Tucker Carlson while walking to get tea
// Decemeber 16,2022 Im stuck at home because of a snow storm
// Decemeber 23,2022,i got 3 monster energy drinks
//
static int64_t IsTerminalInst(CRPN *r) {
  switch(r->type) {
    break;
    case IC_RET:
    case IC_GOTO:
    return 1;
  }
  return 0;
}


static int64_t IsConst(CRPN* rpn)
{
	switch (rpn->type) {
	case IC_CHR:
	case IC_F64:
	case IC_I64:
		return 1;
	}
	return 0;
}

static int64_t ConstVal(CRPN* rpn)
{
	switch (rpn->type) {
	case IC_CHR:
	case IC_I64:
	  return rpn->integer;
	case IC_F64:
  	return rpn->flt;
	}
	return 0;
}

enum {
	// Meet the arguments
	ARM_REG_X0 = 0, // I am also a return value
	ARM_REG_X1,
	ARM_REG_X2,
	ARM_REG_X3,
	ARM_REG_X4,
	ARM_REG_X5,
	ARM_REG_X6,
	ARM_REG_X7,
	// Meet the silly willies(scratch)
	ARM_REG_X8,
	ARM_REG_X9,
	ARM_REG_X10,
	ARM_REG_X11,
	ARM_REG_X12,
	ARM_REG_X13,
	ARM_REG_X14,
	ARM_REG_X15,
	ARM_REG_X16,
	ARM_REG_X17,
	// Meet the platform register(does something?)
	ARM_REG_X18,
	// Meet the bodacious bunch(saved accross calls)
	ARM_REG_X19,
	ARM_REG_X20,
	ARM_REG_X21,
	ARM_REG_X22,
	ARM_REG_X23,
	ARM_REG_X24,
	ARM_REG_X25,
	ARM_REG_X26,
	ARM_REG_X27,
	ARM_REG_X28,
	// Meet the frame pointer
	ARM_REG_X29,
#define ARM_REG_FP ARM_REG_X29
	// Meet the return address
	ARM_REG_X30,
#define ARM_REG_LR ARM_REG_X30
	// Meet the (secret) stack pointer
	ARM_REG_SP,
};
enum {
	// Meet the floating point arguments
	ARM_REG_V0 = 0, // Also return register
	ARM_REG_V1,
	ARM_REG_V2,
	ARM_REG_V3,
	ARM_REG_V4,
	ARM_REG_V5,
	ARM_REG_V6,
	ARM_REG_V7,
	// Meet the saved registers
	ARM_REG_V8,
	ARM_REG_V9,
	ARM_REG_V10,
	ARM_REG_V11,
	ARM_REG_V12,
	ARM_REG_V13,
	ARM_REG_V14,
	ARM_REG_V15,
	// Meet the poo poo registers(not saved)
	ARM_REG_V16,
	ARM_REG_V17,
	ARM_REG_V18,
	ARM_REG_V19,
	ARM_REG_V20,
	ARM_REG_V21,
	ARM_REG_V22,
	ARM_REG_V23,
	ARM_REG_V24,
	ARM_REG_V25,
	ARM_REG_V26,
	ARM_REG_V27,
	ARM_REG_V28,
	ARM_REG_V29,
	ARM_REG_V30,
	ARM_REG_V31,
};

static int64_t PutICArgIntoReg(CCmpCtrl* cctrl, CICArg* arg, int64_t raw_type,
	int64_t fallback, char* bin, int64_t code_off);
#define AIWNIOS_ADD_CODE(val)                   \
	{                                           \
		if (bin) {                              \
			int64_t v2 = (val);                 \
			if (v2 == ARM_ERR_INV_REG)          \
				throw(*(uint32_t*)"ASM");       \
			if (v2 == ARM_ERR_INV_OFF)          \
				throw(*(uint32_t*)"ASM");       \
			((int32_t*)bin)[code_off / 4] = v2; \
		}                                       \
		code_off += 4;                          \
	}

static int64_t PushToStack(CCmpCtrl* cctrl, CICArg* arg, char* bin,
	int64_t code_off)
{
	CICArg tmp = *arg;
	PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 0, bin, code_off);
	if (tmp.raw_type != RT_F64) {
		AIWNIOS_ADD_CODE(ARM_strPreImmX(tmp.reg, ARM_REG_SP, -16));
	} else {
		AIWNIOS_ADD_CODE(ARM_strPreImmF64(tmp.reg, ARM_REG_SP, -16));
	}
	return code_off;
}
static int64_t PopFromStack(CCmpCtrl* cctrl, CICArg* arg, char* bin,
	int64_t code_off)
{
	CICArg tmp = *arg;
	if (arg->mode == MD_REG) {
		// All is good
	} else {
		tmp.mode = MD_REG;
		tmp.reg = 0;
	}
	if (tmp.raw_type != RT_F64) {
		AIWNIOS_ADD_CODE(ARM_ldrPostImmX(tmp.reg, ARM_REG_SP, 16));
	} else {
		AIWNIOS_ADD_CODE(ARM_ldrPostImmF64(tmp.reg, ARM_REG_SP, 16));
	}
	if (arg->mode != MD_REG)
		code_off = ICMov(cctrl, arg, &tmp, bin, code_off);
	return code_off;
}
//
// Will overwrite the arg's value,but it's ok as we are about to use it's
// value(and discard it)
//
static int64_t PutICArgIntoReg(CCmpCtrl* cctrl, CICArg* arg, int64_t raw_type,
	int64_t fallback, char* bin, int64_t code_off)
{
	CICArg tmp={0};
	if (arg->mode == MD_REG) {
		if ((raw_type == RT_F64) ^ (arg->raw_type == RT_F64)) {
			// They differ so continue as usaul
		} else // They are the same
			return code_off;
	}
	tmp.raw_type = raw_type;
	tmp.reg = fallback;
	tmp.mode = MD_REG;
	code_off = ICMov(cctrl, &tmp, arg, bin, code_off);
	memcpy(arg, &tmp, sizeof(CICArg));
	return code_off;
}

static int64_t __OptPassFinal(CCmpCtrl* cctrl, CRPN* rpn, char* bin,
	int64_t code_off);
static int64_t __ICMoveI64(CCmpCtrl* cctrl, int64_t reg, uint64_t imm, char* bin,
	int64_t code_off)
{
  int64_t code;
  if(bin&&!(cctrl->flags&CCF_AOT_COMPILE)) { //TODO make sure not AOT
    if(ARM_ERR_INV_OFF!=(code=ARM_adrX(reg,imm-(int64_t)(bin+code_off)))) {
      AIWNIOS_ADD_CODE(code);
      return code_off;
    }
  }
	// Here's the deal,you can store a 16bit literal into a register,but you
	// can shift it to get bigger range,and you can make it negative
	if ((imm & 0xffffll) == imm) {
		AIWNIOS_ADD_CODE(ARM_movzImmX(reg, imm, 0));
		return code_off;
	}
	if ((imm & 0xffff0000ll) == imm) {
		AIWNIOS_ADD_CODE(ARM_movzImmX(reg, imm >> 16, 16));
		return code_off;
	}
	if ((imm & 0xffff00000000ll) == imm) {
		AIWNIOS_ADD_CODE(ARM_movzImmX(reg, imm >> 32, 32));
		return code_off;
	}
	if ((imm & 0xffff000000000000ll) == imm) {
		AIWNIOS_ADD_CODE(ARM_movzImmX(reg, imm >> 48, 48));
		return code_off;
	}
	/*
	imm = ~imm;
	if ((imm & 0xffff) == imm) {
		AIWNIOS_ADD_CODE(ARM_movnImmX(reg, imm, 0));
		return code_off;
	}
	if ((imm & 0xffff0000) == imm) {
		AIWNIOS_ADD_CODE(ARM_movnImmX(reg, imm >> 16, 16));
		return code_off;
	}
	if ((imm & 0xffff00000000ll) == imm) {
		AIWNIOS_ADD_CODE(ARM_movnImmX(reg, imm >> 32, 32));
		return code_off;
	}
	if ((imm & 0xffff000000000000ll) == imm) {
		AIWNIOS_ADD_CODE(ARM_movnImmX(reg, imm >> 48, 48));
		return code_off;
	}
	imm = ~imm;
  */
	CCodeMisc* misc = cctrl->code_ctrl->code_misc->next;
	char* dptr;
	for (misc; misc != cctrl->code_ctrl->code_misc; misc = misc->base.next) {
		if (misc->type == CMT_INT_CONST && misc->integer == imm) {
			goto found;
		}
	}
	misc = A_CALLOC(sizeof(CCodeMisc), cctrl->hc);
	misc->type = CMT_INT_CONST;
	misc->integer = imm;
	QueIns(misc, cctrl->code_ctrl->code_misc->last);
found:
	if (bin && misc->addr && cctrl->code_ctrl->final_pass >= 2) {
		dptr = (char*)misc->addr - (bin + code_off);
		AIWNIOS_ADD_CODE(ARM_ldrLabelX(reg, dptr));
	} else
		AIWNIOS_ADD_CODE(0); //See above AIWNIOS_ADD_CODE
	return code_off;
}

static int64_t __ICMoveF64(CCmpCtrl* cctrl, int64_t reg, double imm, char* bin,
	int64_t code_off)
{
	//(For now),I ain't messing around with binary literals in ARM
	CCodeMisc* misc = cctrl->code_ctrl->code_misc->next;
	char* dptr;
	for (misc; misc != cctrl->code_ctrl->code_misc; misc = misc->base.next) {
		if (misc->type == CMT_FLOAT_CONST)
			if (misc->integer == *(int64_t*)&imm) {
				goto found;
			}
	}
	misc = A_CALLOC(sizeof(CCodeMisc), cctrl->hc);
	misc->type = CMT_FLOAT_CONST;
	misc->integer = *(int64_t*)&imm;
	QueIns(misc, cctrl->code_ctrl->code_misc->last);
found:
	if (bin && misc->addr && cctrl->code_ctrl->final_pass >= 2) {
		dptr = (char*)misc->addr - (bin + code_off);
		AIWNIOS_ADD_CODE(ARM_ldrLabelF64(reg, dptr));
	} else
		AIWNIOS_ADD_CODE(0); //See above AIWNIOS_ADD_CODE
	return code_off;
}

#define ARM_LABEL_DIST (((1 << 19) - 1) << 2)
static int64_t PtrDist(int64_t a, int64_t b)
{
	if (a > b)
		return a - b;
	return b - a;
}

static void PushSpilledTmp(CCmpCtrl* cctrl, CRPN* rpn)
{
	int64_t raw_type = rpn->raw_type;
	CICArg* res = &rpn->res;
	// These have no need to be put into a temporay
	switch (rpn->type) {
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
	rpn->flags |= ICF_SPILLED;
	if (raw_type != RT_F64) {
		res->mode = MD_FRAME;
		res->raw_type = raw_type;
		if (cctrl->backend_user_data0 < (res->off = cctrl->backend_user_data1 += 8)) { // Will set ref->off before the top
			cctrl->backend_user_data0 = res->off;
		}
	} else {
		res->mode = MD_FRAME;
		res->raw_type = raw_type;
		if (cctrl->backend_user_data0 < (res->off = cctrl->backend_user_data1 += 8)) { // Will set ref->off before the top
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
			res->off += cctrl->backend_user_data4; //wiggle room start(if set)
		} else
			//Anonymus potatoes like to assume LR/FP pair too.
			res->off += 16;
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

static void PushTmp(CCmpCtrl* cctrl, CRPN* rpn, CICArg* inher_from)
{
	int64_t raw_type = rpn->raw_type;
	CICArg* res = &rpn->res;
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
	case IC_GLOBAL:
		res->mode = MD_PTR;
		res->raw_type = raw_type;
		res->off = rpn->global_var->data_addr;
		return;
		// These dudes will compile down to a MD_I64/MD_F64
	case IC_I64:
    res->mode=MD_I64;
    res->integer=rpn->integer;
    res->raw_type=RT_I64i;
    return;
	break; case IC_F64:
    res->mode=MD_F64;
    res->flt=rpn->flt;
    res->raw_type=RT_F64;
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
		if (rpn->raw_type == RT_F64 && inher_from->raw_type == RT_F64 && inher_from->mode != MD_NULL) {
			rpn->res = *inher_from;
			rpn->flags |= ICF_TMP_NO_UNDO;
			return;
		} else if (rpn->raw_type != RT_F64 && inher_from->raw_type != RT_F64 && inher_from->mode != MD_NULL) {
			rpn->res = *inher_from;
			rpn->flags |= ICF_TMP_NO_UNDO;
			return;
		}
	}
	if (raw_type != RT_F64) {
		if (cctrl->backend_user_data2 < AIWNIOS_TMP_IREG_CNT) {
			res->mode = MD_REG;
			res->raw_type = raw_type;
			res->off = 0;
			res->reg = AIWNIOS_TMP_IREG_START + cctrl->backend_user_data2++;
		} else {
			PushSpilledTmp(cctrl, rpn);
		}
	} else {
		if (cctrl->backend_user_data3 < AIWNIOS_TMP_FREG_CNT) {
			res->mode = MD_REG;
			res->raw_type = raw_type;
			res->off = 0;
			res->reg = AIWNIOS_TMP_FREG_START + cctrl->backend_user_data3++;
		} else {
			PushSpilledTmp(cctrl, rpn);
		}
	}
}

static void PopTmp(CCmpCtrl* cctrl, CRPN* rpn)
{
	int64_t raw_type = rpn->raw_type;
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
	case IC_GLOBAL:
	case IC_STATIC:
	case __IC_STATIC_REF:
		return;
	}
	if (rpn->flags & ICF_SPILLED) {
		cctrl->backend_user_data1 -= 8;
	} else {
		if (raw_type != RT_F64) {
			assert(--cctrl->backend_user_data2 >= 0);
		} else {
			assert(--cctrl->backend_user_data3 >= 0);
		}
	}
	assert(cctrl->backend_user_data1 >= 0);
}
//
// Here's the deal,we can replace an *(reg+123) with MD_INDIR_REG with
// off==123
//
static int64_t DerefToICArg(CCmpCtrl* cctrl, CICArg* res, CRPN* rpn,
	int64_t base_reg_fallback, char* bin,
	int64_t code_off)
{
	if (rpn->type != IC_DEREF) {
		PushTmp(cctrl, rpn, NULL);
		code_off = __OptPassFinal(cctrl, rpn, bin, code_off);
		*res = rpn->res;
		PopTmp(cctrl, rpn);
		return code_off;
	}
	int64_t r = rpn->raw_type, rsz, off, mul;
	rpn = rpn->base.next;
	CRPN *next = rpn->base.next, *next2, *next3, *next4, *tmp;
	switch (r) {
		break;
	case RT_I8i:
		rsz = 1;
		break;
	case RT_U8i:
		rsz = 1;
		break;
	case RT_I16i:
		rsz = 2;
		break;
	case RT_U16i:
		rsz = 2;
		break;
	case RT_I32i:
		rsz = 4;
		break;
	case RT_U32i:
		rsz = 4;
		break;
	default:
		rsz = 8;
	}
	if (rpn->type == IC_ADD) {
		next2 = ICFwd(rpn->base.next);
		if (next->type == IC_I64) {
			PushTmp(cctrl, next2, NULL);
			code_off = __OptPassFinal(cctrl, next2, bin, code_off);
			PopTmp(cctrl, next2);
			code_off = PutICArgIntoReg(cctrl, &next2->res, RT_PTR, base_reg_fallback,
				bin, code_off);
			res->mode = MD_INDIR_REG;
			res->off = next->integer;
			res->raw_type = r;
			res->reg = next2->res.reg;
			return code_off;
		} else if (next2->type == IC_I64) {
			PushTmp(cctrl, next, NULL);
			code_off = __OptPassFinal(cctrl, next, bin, code_off);
			PopTmp(cctrl, next);
			code_off = PutICArgIntoReg(cctrl, &next->res, RT_PTR, base_reg_fallback,
				bin, code_off);
			res->mode = MD_INDIR_REG;
			res->off = next2->integer;
			res->raw_type = r;
			res->reg = next->res.reg;
			return code_off;
		} else if (next->type == IC_MUL) {
		index_chk:
			off = 0;
			next3 = ICArgN(next, 0);
			next4 = ICArgN(next, 1);
			if (next3->type == IC_IREG)
				off++;
			if (next4->type == IC_IREG)
				off++;
			if (cctrl->backend_user_data2 + 2 - off <= AIWNIOS_TMP_IREG_CNT) {
				if (next3->type == IC_I64) {
					mul = next3->integer;
				idx:
					if (rsz == mul && next4->raw_type != RT_F64) {
						// IC_ADD==rpn
						//   IC_MUL==next
						//      IC_I64==next3
						//      mul==next4
						//   off==next2
						//

						//Store
						off = cctrl->backend_user_data2;
            if(SpillsTmpRegs(next4))
              PushSpilledTmp(cctrl,next2);
            else
              PushTmp(cctrl, next2, NULL);
						PushTmp(cctrl, next4, NULL);
						code_off = __OptPassFinal(cctrl, next2, bin, code_off);
						code_off = __OptPassFinal(cctrl, next4, bin, code_off);
						if (next4->res.mode != MD_REG) {
							code_off = PutICArgIntoReg(cctrl, &next4->res, RT_I64i, cctrl->backend_user_data2++, bin, code_off);
						}
						if (next2->res.mode != MD_REG) {
							code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i, cctrl->backend_user_data2++, bin, code_off);
						}
						PopTmp(cctrl, next2);
						PopTmp(cctrl, next4);
						res->mode = __MD_ARM_SHIFT;
						res->reg = next2->res.reg;
						res->reg2 = next4->res.reg;
						res->raw_type = r;
						//Restore
						cctrl->backend_user_data2 = off;
						return code_off;
					}
				} else if (next4->type == IC_I64) {
					tmp = next3;
					next3 = next4;
					next4 = tmp;
					mul = next3->integer;
					goto idx;
				} else {
					//See above note
					// IC_ADD==rpn==next
					//   mul==next4
					//   off==next2
					//
					next2 = next3;
					mul = 1;
					goto idx;
				}
			} else if (next2->type == IC_MUL) {
				tmp = next;
				next = next2;
				next2 = tmp;
				goto index_chk;
			}
		} else {
			mul = 1;
			next = rpn;
			goto index_chk;
		}
	}
	PushTmp(cctrl, rpn, NULL);
	code_off = __OptPassFinal(cctrl, rpn, bin, code_off);
	code_off = PutICArgIntoReg(cctrl, &rpn->res, RT_PTR, base_reg_fallback, bin,
		code_off);
	PopTmp(cctrl, rpn);
	res->mode = MD_INDIR_REG;
	res->off = 0;
	res->raw_type = r;
	res->reg = rpn->res.reg;
	return code_off;
}
//See __IC_CALL
static int64_t __ICFCall(CCmpCtrl* cctrl, CRPN* rpn, char* bin,
	int64_t code_off)
{
	CRPN *rpn2 = rpn->base.next, *rpn3;
	CICArg *arg_dsts = NULL, tmp={0};
	CCodeMisc* reloc;
	char* fptr;

	//
	// argv_stki is the stack location of the first varg
	//
	int64_t i2, i = rpn->length, to, ii = 0, fi = 0, stki = 0, vargs_sz = 0;
	while (--i >= 0)
		rpn2 = ICFwd(rpn2);
	AssignRawTypeToNode(cctrl, rpn2);
	to = rpn->length;
	arg_dsts = A_MALLOC(to * sizeof(CICArg),
		cctrl->hc);
	rpn2 = ICArgN(rpn, rpn->length);
	PushTmp(cctrl, rpn2, NULL);
	rpn2 = rpn->base.next;
	for (i = 0; i != to;) {
		rpn2 = ICArgN(rpn, rpn->length - i - 1);
		if (rpn2->type == __IC_VARGS) { //Will pop off the vargs for you
			vargs_sz = 8 * rpn2->length;
			if (rpn2->length & 1) //Align to 16
				vargs_sz += 8;
		}
		arg_dsts[i].raw_type = rpn2->raw_type;
		if (rpn2->raw_type == RT_F64) {
			if (fi < 8 && i < to) {
				arg_dsts[i].mode = MD_REG;
				arg_dsts[i].reg = fi++;
			} else {
				arg_dsts[i].mode = MD_INDIR_REG;
				arg_dsts[i].reg = ARM_REG_SP;
				arg_dsts[i].off = stki++ * 8;
			}
		} else {
			if (ii < 8 && i < to) {
				arg_dsts[i].raw_type = RT_I64i;
				arg_dsts[i].mode = MD_REG;
				arg_dsts[i].reg = ii++;
			} else {
				arg_dsts[i].raw_type = RT_I64i;
				arg_dsts[i].mode = MD_INDIR_REG;
				arg_dsts[i].reg = ARM_REG_SP;
				arg_dsts[i].off = stki++ * 8;
			}
		}
		// Check for function calls in future arguments
		rpn3 = rpn2;
		for (i2 = i + 1; i2 < rpn->length; i2++) {
			rpn3 = ICArgN(rpn, rpn->length - i2 - 1);
			if (SpillsTmpRegs(rpn3)) {
				PushSpilledTmp(cctrl, rpn2);
				goto pass;
			}
		}
		PushTmp(cctrl, rpn2, NULL);
	pass:
		code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
		rpn2 = ICFwd(rpn2);
		i++;
	}
	// Keep stack aligned to 16 bytes(8 bytes per item)
	if (stki % 2)
		stki++;
	if (stki)
		AIWNIOS_ADD_CODE(ARM_subImmX(ARM_REG_SP, ARM_REG_SP, stki * 8));
	i = rpn->length - 1;
aloop:
	while (i >= 0) {
		i2 = i;
		// IC_CALL
		// ARGn <===rpn->length -1
		// FUN
		rpn2 = ICArgN(rpn, rpn->length - i - 1);
		PopTmp(cctrl, rpn2);
		code_off = ICMov(cctrl, &arg_dsts[i2], &rpn2->res, bin, code_off);
		i--;
	next:;
	}
	rpn2 = ICArgN(rpn, rpn->length);
	if (rpn2->type == IC_SHORT_ADDR) {
		rpn2->code_misc->addr = code_off + bin;
		AIWNIOS_ADD_CODE(ARM_bl(0));
		goto after_call;
	}
	if (rpn2->type == IC_GLOBAL) {
		if (rpn2->global_var->base.type & HTT_FUN) {
			fptr = ((CHashFun*)rpn2->global_var)->fun_ptr;
			if (cctrl->code_ctrl->final_pass >= 2) {
				if (ARM_ERR_INV_OFF != ARM_bl((char*)fptr - (bin + code_off)))
					AIWNIOS_ADD_CODE(ARM_bl((char*)fptr - (bin + code_off)))
				else
					goto defacto;
			} else
				goto defacto;
		} else
			goto defacto;
	} else {
	defacto:
		if (rpn2->res.mode == MD_PTR) {
			rpn2->res.raw_type = RT_PTR;
			rpn2->res.mode = MD_REG;
			rpn2->res.reg = 8;
		}
		code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
		code_off = PutICArgIntoReg(cctrl, &rpn2->res, RT_PTR, 8, bin, code_off);
		AIWNIOS_ADD_CODE(ARM_blr(rpn2->res.reg));
	}
after_call:
	if (stki || vargs_sz)
		AIWNIOS_ADD_CODE(ARM_addImmX(ARM_REG_SP, ARM_REG_SP, stki * 8 + vargs_sz));
	if (rpn->raw_type != RT_U0) {
		tmp.reg = 0;
		tmp.mode = MD_REG;
		tmp.raw_type = rpn->raw_type;
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
	}
	A_FREE(arg_dsts);
	rpn2 = ICArgN(rpn, rpn->length);
	PopTmp(cctrl, rpn2);
	return code_off;
}

static int64_t ICMov(CCmpCtrl* cctrl, CICArg* dst, CICArg* src, char* bin,
	int64_t code_off)
{
	int64_t use_reg, use_reg2, restore_from_tmp = 0, indir_off = 0,
							   indir_off2 = 0, opc;
	assert(src->mode);
	CICArg tmp={0};
	if (dst->mode == MD_NULL)
		return code_off;
	if ((dst->raw_type == RT_F64) == (src->raw_type == RT_F64))
		if (dst->mode == src->mode) {
			switch (dst->mode) {
				break;
			case __MD_ARM_SHIFT:
				if (dst->reg == src->reg)
					if (dst->reg2 == src->reg2)
						return code_off;
				break;
			case MD_STATIC:
				if (dst->off == src->off)
					return code_off;
				break;
			case MD_PTR:
				if (dst->off == src->off)
					return code_off;
				break;
			case MD_REG:
				if (dst->reg == src->reg)
					return code_off;
				break;
			case MD_FRAME:
				if (dst->off == src->off)
					return code_off;
				break;
			case MD_INDIR_REG:
				if (dst->off == src->off && dst->reg == src->reg)
					return code_off;
			}
		}
	switch (dst->mode) {
	case __MD_ARM_SHIFT:
		if (src->mode == MD_REG && ((src->raw_type == RT_F64) == (dst->raw_type == RT_F64))) {
			switch (dst->raw_type) {
      case RT_U0:
				break;
			case RT_U8i:
			case RT_I8i:
				AIWNIOS_ADD_CODE(ARM_strbRegRegShift(src->reg, dst->reg, dst->reg2));
				break;
			case RT_U16i:
			case RT_I16i:
				AIWNIOS_ADD_CODE(ARM_strhRegRegShift(src->reg, dst->reg, dst->reg2));
				break;
			case RT_U32i:
			case RT_I32i:
				AIWNIOS_ADD_CODE(ARM_strRegRegShift(src->reg, dst->reg, dst->reg2));
				break;
			case RT_U64i:
			case RT_I64i:
			case RT_PTR:
			case RT_FUNC:
				AIWNIOS_ADD_CODE(ARM_strRegRegShiftX(src->reg, dst->reg, dst->reg2));
				break;
			case RT_F64:
				AIWNIOS_ADD_CODE(ARM_strRegRegShiftF64(src->reg, dst->reg, dst->reg2));
			}
			return code_off;
		}
		goto dft;
		break;
	case MD_STATIC:
		use_reg2 = AIWNIOS_TMP_IREG_POOP;
		indir_off = 0;
		if (cctrl->code_ctrl->final_pass >= 2) {
			AIWNIOS_ADD_CODE(ARM_adrX(use_reg2, dst->off - code_off + cctrl->code_ctrl->statics_offset));
		} else
			AIWNIOS_ADD_CODE(0);
		goto indir_r2;
		break;
	case MD_INDIR_REG:
		use_reg2 = dst->reg;
		indir_off = dst->off;
	indir_r2:
		if (src->raw_type == dst->raw_type && src->raw_type == RT_F64 && src->mode == MD_REG) {
			use_reg = src->reg;
		} else if ((src->raw_type == RT_F64) ^ (RT_F64 == dst->raw_type)) { //Do they differ
			goto dft;
		} else if (src->mode == MD_FRAME || src->mode == MD_PTR || src->mode == MD_INDIR_REG) {
			goto dft;
		} else if (src->mode == MD_REG)
			use_reg = src->reg;
		else {
		dft:
			tmp.raw_type = src->raw_type;
			if (dst->raw_type != RT_F64)
				use_reg = tmp.reg = AIWNIOS_TMP_IREG_POOP2;
			else
				use_reg = tmp.reg = 0;
			tmp.mode = MD_REG;
			code_off = ICMov(cctrl, &tmp, src, bin, code_off);
			if (dst->raw_type == RT_F64 && src->raw_type != dst->raw_type) {
				AIWNIOS_ADD_CODE(ARM_scvtf(use_reg, use_reg));
				tmp.raw_type = RT_F64;
			} else if (src->raw_type == RT_F64 && src->raw_type != dst->raw_type) {
				AIWNIOS_ADD_CODE(ARM_fcvtzs(use_reg, use_reg));
				tmp.raw_type = RT_I64i;
			}
			code_off = ICMov(cctrl, dst, &tmp, bin, code_off);
			return code_off;
		}
		goto store_r2;
		break;
	case MD_PTR:
		use_reg2 = AIWNIOS_TMP_IREG_POOP;
		code_off = __ICMoveI64(cctrl, use_reg2, dst->off, bin, code_off);
		indir_off = 0;
		goto indir_r2;
	store_r2:
		if (indir_off < 0) {
			indir_off2 = indir_off;
			indir_off = 0;
			AIWNIOS_ADD_CODE(ARM_subImmX(use_reg2, use_reg2, -indir_off2));
		}
		switch (dst->raw_type) {
    case RT_U0:
			break;
		case RT_F64:
			opc = ARM_strRegImmF64(use_reg, use_reg2, indir_off);
			if (opc != ARM_ERR_INV_OFF) {
				AIWNIOS_ADD_CODE(opc);
			} else {
				//5 is one of our poop registers
				code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
				AIWNIOS_ADD_CODE(ARM_strRegRegF64(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
			}
			break;
		case RT_U8i:
		case RT_I8i:
			opc = ARM_strbRegImm(use_reg, use_reg2, indir_off);
			if (opc != ARM_ERR_INV_OFF) {
				AIWNIOS_ADD_CODE(opc);
			} else {
				code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
				AIWNIOS_ADD_CODE(ARM_strbRegReg(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
			}
			break;
		case RT_U16i:
		case RT_I16i:
			opc = ARM_strhRegImm(use_reg, use_reg2, indir_off);
			if (opc != ARM_ERR_INV_OFF) {
				AIWNIOS_ADD_CODE(opc);
			} else {
				code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
				AIWNIOS_ADD_CODE(ARM_strhRegReg(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
			}
			break;
		case RT_U32i:
		case RT_I32i:
			opc = ARM_strRegImm(use_reg, use_reg2, indir_off);
			if (opc != ARM_ERR_INV_OFF) {
				AIWNIOS_ADD_CODE(opc);
			} else {
				//5 is one of our poop registers
				code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
				AIWNIOS_ADD_CODE(ARM_strRegReg(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
			}
			break;
		case RT_U64i:
		case RT_I64i:
		case RT_FUNC: //func ptr
		case RT_PTR:
			opc = ARM_strRegImmX(use_reg, use_reg2, indir_off);
			if (opc != ARM_ERR_INV_OFF) {
				AIWNIOS_ADD_CODE(opc);
			} else {
				//5 is one of our poop registers
				code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
				AIWNIOS_ADD_CODE(ARM_strRegRegX(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
			}
			break;
		default:
			abort();
		}
    if (indir_off2<0) {
				AIWNIOS_ADD_CODE(ARM_addImmX(use_reg2, use_reg2, -indir_off2));
      } else if(indir_off2>0) {
      	AIWNIOS_ADD_CODE(ARM_subImmX(use_reg2, use_reg2, indir_off2));
      }
		break;
	case MD_FRAME:
		use_reg2 = ARM_REG_FP;
		indir_off = dst->off;
		goto indir_r2;
		break;
	case MD_REG:
		use_reg = dst->reg;
		if (src->mode == MD_FRAME) {
			if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
				use_reg2 = ARM_REG_FP;
				indir_off = src->off;
				goto load_r2;
			} else if ((src->raw_type == RT_F64) ^ (RT_F64 == dst->raw_type)) {
				goto dft;
			}
			use_reg2 = ARM_REG_FP;
			indir_off = src->off;
			goto load_r2;
		} else if (src->mode == __MD_ARM_SHIFT) {
			if ((src->raw_type == RT_F64) != (dst->raw_type == RT_F64))
				goto dft;
			switch (src->raw_type) {
      case RT_U0:
				break;
			case RT_U8i:
				AIWNIOS_ADD_CODE(ARM_ldrbRegRegShift(dst->reg, src->reg, src->reg2));
				AIWNIOS_ADD_CODE(ARM_uxtbX(dst->reg, dst->reg));
				break;
			case RT_I8i:
				AIWNIOS_ADD_CODE(ARM_ldrbRegRegShift(dst->reg, src->reg, src->reg2));
				AIWNIOS_ADD_CODE(ARM_sxtbX(dst->reg, dst->reg));
				break;
			case RT_U16i:
				AIWNIOS_ADD_CODE(ARM_ldrhRegRegShift(dst->reg, src->reg, src->reg2));
				AIWNIOS_ADD_CODE(ARM_uxthX(dst->reg, dst->reg));
				break;
			case RT_I16i:
				AIWNIOS_ADD_CODE(ARM_ldrhRegRegShift(dst->reg, src->reg, src->reg2));
				AIWNIOS_ADD_CODE(ARM_sxthX(dst->reg, dst->reg));
				break;
			case RT_U32i:
				AIWNIOS_ADD_CODE(ARM_ldrRegRegShift(dst->reg, src->reg, src->reg2));
				AIWNIOS_ADD_CODE(ARM_uxtwX(dst->reg, dst->reg));
				break;
			case RT_I32i:
				AIWNIOS_ADD_CODE(ARM_ldrRegRegShift(dst->reg, src->reg, src->reg2));
				AIWNIOS_ADD_CODE(ARM_sxtwX(dst->reg, dst->reg));
				break;
			case RT_U64i:
			case RT_PTR:
			case RT_FUNC:
			case RT_I64i:
				AIWNIOS_ADD_CODE(ARM_ldrRegRegShiftX(dst->reg, src->reg, src->reg2));
				break;
			case RT_F64:
				AIWNIOS_ADD_CODE(ARM_ldrRegRegShiftF64(dst->reg, src->reg, src->reg2));
			}
			return code_off;
		} else if (src->mode == MD_REG) {
			if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
				AIWNIOS_ADD_CODE(ARM_fmovReg(dst->reg, src->reg));
			} else if (src->raw_type != RT_F64 && dst->raw_type != RT_F64) {
				AIWNIOS_ADD_CODE(ARM_movRegX(dst->reg, src->reg));
			} else if (src->raw_type == RT_F64 && dst->raw_type != RT_F64) {
				AIWNIOS_ADD_CODE(ARM_fcvtzs(dst->reg, src->reg));
			} else if (src->raw_type != RT_F64 && dst->raw_type == RT_F64) {
				switch (src->raw_type) {
				case RT_U8i:
				case RT_U16i:
				case RT_U32i:
				case RT_U64i:
					AIWNIOS_ADD_CODE(ARM_ucvtf(dst->reg, src->reg));
					break;
				default:
					AIWNIOS_ADD_CODE(ARM_scvtf(dst->reg, src->reg));
				}
				return code_off;
			}
		} else if (src->mode == MD_PTR) {
			if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
				use_reg2 = 0;
				code_off = __ICMoveI64(cctrl, use_reg2, src->off, bin, code_off);
				goto load_r2;
			} else if ((src->raw_type == RT_F64) ^ (RT_F64 == dst->raw_type)) {
				goto dft;
			}
			//We can use ARM_ldrLabel/ARM_ldrLabelX for 64/32bit shenangins if it is in range
			if (cctrl->code_ctrl->final_pass >= 2)
				switch (src->raw_type) {
					break;
				case RT_I32i:
					opc = ARM_ldrLabel(use_reg, (char*)src->off - (code_off + bin));
					if (opc != ARM_ERR_INV_OFF) {
						AIWNIOS_ADD_CODE(opc);
						AIWNIOS_ADD_CODE(ARM_sxtwX(use_reg, use_reg));
						return code_off;
					}
					break;
				case RT_U32i:
					opc = ARM_ldrLabel(use_reg, (char*)src->off - (code_off + bin));
					if (opc != ARM_ERR_INV_OFF) {
						AIWNIOS_ADD_CODE(opc);
						AIWNIOS_ADD_CODE(ARM_uxtwX(use_reg, use_reg));
						return code_off;
					}
					break;
				case RT_I64i:
				case RT_U64i:
					opc = ARM_ldrLabelX(use_reg, (char*)src->off - (code_off + bin));
					if (opc != ARM_ERR_INV_OFF) {
						AIWNIOS_ADD_CODE(opc);
						return code_off;
					}
					break;
				case RT_F64:
					opc = ARM_ldrLabelF64(use_reg, (char*)src->off - (code_off + bin));
					if (opc != ARM_ERR_INV_OFF) {
						AIWNIOS_ADD_CODE(opc);
						return code_off;
					}
				}

			use_reg2 = AIWNIOS_TMP_IREG_POOP;
			code_off = __ICMoveI64(cctrl, use_reg2, src->off, bin, code_off);
			indir_off = 0;
		load_r2:
			if (indir_off < 0) {
				indir_off2 = indir_off;
				indir_off = 0;
				AIWNIOS_ADD_CODE(ARM_subImmX(use_reg2, use_reg2, -indir_off2));
			}
			switch (src->raw_type) {
      case RT_U0:
				break;
			case RT_F64:
				opc = ARM_ldrRegImmF64(use_reg, use_reg2, indir_off);
				if (opc != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(opc);
				} else {
					//5 is one of our poop registers
					code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
					AIWNIOS_ADD_CODE(ARM_ldrRegRegF64(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
				}
				break;
			case RT_U8i:
				opc = ARM_ldrbRegImm(use_reg, use_reg2, indir_off);
				if (opc != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(opc);
				} else {
					//5 is one of our poop registers
					code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
					AIWNIOS_ADD_CODE(ARM_ldrbRegReg(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
				}
				AIWNIOS_ADD_CODE(ARM_uxtbX(use_reg, use_reg));
				break;
			case RT_I8i:
				opc = ARM_ldrbRegImm(use_reg, use_reg2, indir_off);
				if (opc != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(opc);
				} else {
					//5 is one of our poop registers
					code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
					AIWNIOS_ADD_CODE(ARM_ldrbRegReg(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
				}
				AIWNIOS_ADD_CODE(ARM_sxtbX(use_reg, use_reg));
				break;
			case RT_U16i:
				opc = ARM_ldrhRegImm(use_reg, use_reg2, indir_off);
				if (opc != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(opc);
				} else {
					//5 is one of our poop registers
					code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
					AIWNIOS_ADD_CODE(ARM_ldrhRegReg(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
				}
				AIWNIOS_ADD_CODE(ARM_uxthX(use_reg, use_reg));
				break;
			case RT_I16i:
				opc = ARM_ldrhRegImm(use_reg, use_reg2, indir_off);
				if (opc != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(opc);
				} else {
					//5 is one of our poop registers
					code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
					AIWNIOS_ADD_CODE(ARM_ldrhRegReg(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
				}
				AIWNIOS_ADD_CODE(ARM_sxthX(use_reg, use_reg));
				break;
			case RT_U32i:
				opc = ARM_ldrRegImm(use_reg, use_reg2, indir_off);
				if (opc != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(opc);
				} else {
					//5 is one of our poop registers
					code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
					AIWNIOS_ADD_CODE(ARM_ldrRegReg(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
				}
				AIWNIOS_ADD_CODE(ARM_uxtwX(use_reg, use_reg));
				break;
			case RT_I32i:
				opc = ARM_ldrRegImm(use_reg, use_reg2, indir_off);
				if (opc != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(opc);
				} else {
					//5 is one of our poop registers
					code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
					AIWNIOS_ADD_CODE(ARM_ldrRegReg(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
				}
				AIWNIOS_ADD_CODE(ARM_sxtwX(use_reg, use_reg));
				break;
			case RT_U64i:
			case RT_I64i:
			case RT_FUNC: //func ptr
			case RT_PTR:
				opc = ARM_ldrRegImmX(use_reg, use_reg2, indir_off);
				if (opc != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(opc);
				} else {
					//5 is one of our poop registers
					code_off = __ICMoveI64(cctrl, AIWNIOS_TMP_IREG_POOP, indir_off, bin, code_off);
					AIWNIOS_ADD_CODE(ARM_ldrRegRegX(use_reg, use_reg2, AIWNIOS_TMP_IREG_POOP));
				}

				break;
			default:
				abort();
			}
			if (indir_off2<0) {
				AIWNIOS_ADD_CODE(ARM_addImmX(use_reg2, use_reg2, -indir_off2));
      } else if(indir_off2>0) {
      	AIWNIOS_ADD_CODE(ARM_subImmX(use_reg2, use_reg2, indir_off2));
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
			code_off = __ICMoveI64(cctrl, dst->reg, src->flt, bin, code_off);
		} else if (src->mode == MD_STATIC) {
			use_reg2 = AIWNIOS_TMP_IREG_POOP;
			indir_off = 0;
			if (cctrl->code_ctrl->final_pass >= 2) {
				AIWNIOS_ADD_CODE(ARM_adrX(use_reg2, src->off - code_off + cctrl->code_ctrl->statics_offset));
			} else
				AIWNIOS_ADD_CODE(0);
			if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
			} else if ((src->raw_type == RT_F64) ^ (RT_F64 == dst->raw_type)) {
				goto dft;
			}
			goto load_r2;
		} else
			goto dft;
		break;
	default:
		abort();
	}
	return code_off;
}

//
// If we call a function the tmp regs go to poo poo land
//
static int64_t SpillsTmpRegs(CRPN* rpn)
{
	int64_t idx;
	switch (rpn->type) {
		break;
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
	unop:
		return SpillsTmpRegs(rpn->base.next);
		break;
	case IC_POS:
		goto unop;
		break;
	case IC_POW:
		return 1; //calls pow
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
		break;
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
// Here's the 3rd Donald Trump deal. L-values in HolyC are typecastable
// Here's an example
//
// U8 *a=0xb00b1e5;
// *a(F64*)=pi; //We write an F64 into a U8 pointer
//
// We we need silly sauce to check if we are writing int an typecasted l-value.
// If so,time to rock
//
static int64_t __SexyWriteIntoDst(CCmpCtrl* cctrl, CRPN* deref, CICArg* arg,
	char* bin, int64_t code_off)
{
	CICArg derefed={0}, tmp={0};
	int64_t tc = 0;
	CRPN* orig_rpn = deref;
	code_off = DerefToICArg(cctrl, &derefed, deref, 2, bin, code_off);
	if (!tc) {
	dft:
		code_off = ICMov(cctrl, &derefed, arg, bin, code_off);
	} else {
		if (derefed.mode == MD_REG) {
			if (orig_rpn->raw_type == RT_F64 && arg->raw_type != RT_F64) {
				code_off = PutICArgIntoReg(cctrl, arg, arg->raw_type, 0, bin, code_off);
				AIWNIOS_ADD_CODE(ARM_fmovF64I64(derefed.reg, arg->reg));
			} else if (orig_rpn->raw_type != RT_F64 && arg->raw_type == RT_F64) {
				code_off = PutICArgIntoReg(cctrl, arg, arg->raw_type, 0, bin, code_off);
				AIWNIOS_ADD_CODE(ARM_fmovI64F64(derefed.reg, arg->reg));
			} else {
				goto dft;
			}
		} else {
			tmp.reg = 1;
			tmp.mode = MD_REG;
			tmp.raw_type = orig_rpn->raw_type;
			if (orig_rpn->raw_type == RT_F64 && arg->raw_type != RT_F64) {
				code_off = PutICArgIntoReg(cctrl, arg, arg->raw_type, 0, bin, code_off);
				AIWNIOS_ADD_CODE(ARM_fmovF64I64(1, arg->reg));
			} else if (orig_rpn->raw_type != RT_F64 && arg->raw_type == RT_F64) {
				code_off = PutICArgIntoReg(cctrl, arg, arg->raw_type, 0, bin, code_off);
				AIWNIOS_ADD_CODE(ARM_fmovI64F64(1, arg->reg));
			} else {
				goto dft;
			}
			arg = &tmp;
			goto dft;
		}
	}
	return code_off;
}

static int64_t __SexyPreOp(CCmpCtrl* cctrl, CRPN* rpn,
	int64_t (*i_imm_op)(int64_t, int64_t, int64_t),
  int64_t (*i_reg_op)(int64_t,int64_t,int64_t),
	int32_t (*f_op)(int64_t, int64_t, int64_t),
	char* bin, int64_t code_off)
{
  int64_t code;
	CRPN *next = rpn->base.next, *tc;
	CICArg derefed={0}, tmp={0}, tmp2={0};
	if (next->type == IC_TYPECAST) {
#define TYPECAST_ASSIGN_BEGIN(DST, SRC)                                                    \
	{                                                                                      \
		int64_t pop = 0, pop2 = 0;                                                         \
		CICArg _orig={0};                                                                      \
		CRPN* _tc = DST;                                                                   \
		CRPN* SRC2 = SRC;                                                                  \
		DST = DST->base.next;                                                              \
		if (DST->type == IC_DEREF) {                                                       \
			code_off = DerefToICArg(cctrl, &_orig, DST, 1, bin, code_off);                 \
			DST->res = _orig;                                                              \
			DST->raw_type = _tc->raw_type;                                                 \
		} else if ((DST->raw_type == RT_F64) ^ (SRC2->raw_type == RT_F64)) {               \
			pop2 = pop = 1;                                                                \
			PushTmp(cctrl, DST, NULL);                                                     \
			_orig = DST->res;                                                              \
			code_off = PutICArgIntoReg(cctrl, &DST->res, DST->raw_type, 0, bin, code_off); \
			code_off = PushToStack(cctrl, &DST->res, bin, code_off);                       \
			DST->res.mode = MD_INDIR_REG;                                                  \
			DST->res.off = 0;                                                              \
			DST->res.reg = ARM_REG_SP;                                                     \
			DST->res.raw_type = _tc->raw_type;                                             \
			DST->raw_type = _tc->raw_type;                                                 \
		} else {                                                                           \
			PushTmp(cctrl, DST, NULL);                                                     \
			pop2 = 1;                                                                      \
			DST->res.raw_type = _tc->raw_type;                                             \
			DST->raw_type = _tc->raw_type;                                                 \
		}
#define TYPECAST_ASSIGN_END(DST)                               \
	if (pop)                                                   \
		code_off = PopFromStack(cctrl, &_orig, bin, code_off); \
	if (pop2)                                                  \
		PopTmp(cctrl, DST);                                    \
	}
		TYPECAST_ASSIGN_BEGIN(next, next);
		if (next->raw_type == RT_F64) {
			CICArg orig = next->res;
			code_off = PutICArgIntoReg(cctrl, &next->res, next->raw_type, 2, bin, code_off);
			code_off = __ICMoveF64(cctrl, 1, rpn->integer, bin, code_off);
			AIWNIOS_ADD_CODE(f_op(next->res.reg, next->res.reg, 1));
			code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
			code_off = ICMov(cctrl, &orig, &next->res, bin, code_off);
		} else {
			CICArg orig = next->res;
			code_off = PutICArgIntoReg(cctrl, &next->res, next->raw_type, 1, bin, code_off);
			code=i_imm_op(next->res.reg, next->res.reg, rpn->integer);
      if(code!=ARM_ERR_INV_OFF) {
        AIWNIOS_ADD_CODE(code);
      } else {
        code_off=__ICMoveI64(cctrl,2,rpn->integer,bin,code_off);
        AIWNIOS_ADD_CODE(i_reg_op(next->res.reg, next->res.reg,2))
      }
			code_off = ICMov(cctrl, &orig, &next->res, bin, code_off);
			code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
		}
		TYPECAST_ASSIGN_END(next);
	} else {
		if (next->raw_type == RT_F64) {
			code_off = DerefToICArg(cctrl, &derefed, next, 3, bin, code_off);
			tmp = derefed; //==RT_F64
			code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 2, bin, code_off);
			code_off = __ICMoveF64(cctrl, 1, rpn->integer, bin, code_off);
			AIWNIOS_ADD_CODE(f_op(tmp.reg, tmp.reg, 1));
			code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		} else {
			code_off = DerefToICArg(cctrl, &derefed, next, 3, bin, code_off);
			tmp = derefed;
			code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 2, bin, code_off);
      code=i_imm_op(tmp.reg,tmp.reg,rpn->integer);
			if(code!=ARM_ERR_INV_OFF) {
        AIWNIOS_ADD_CODE(code);
      } else {
        code_off=__ICMoveI64(cctrl,1,rpn->integer,bin,code_off);
        AIWNIOS_ADD_CODE(i_reg_op(tmp.reg, tmp.reg,1))
      }
			code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		}
	}
	return code_off;
}

static int64_t __SexyPostOp(CCmpCtrl* cctrl, CRPN* rpn,
	int64_t (*i_imm_op)(int64_t, int64_t, int64_t),
	int64_t (*i_reg_op)(int64_t, int64_t, int64_t),
  int64_t (*f_op)(int64_t, int64_t, int64_t),
	char* bin, int64_t code_off)
{
	CRPN *next = rpn->base.next, *tc;
  int64_t code;
	CICArg derefed={0}, tmp={0}, tmp2={0};
	if (next->type == IC_TYPECAST) {
		TYPECAST_ASSIGN_BEGIN(next, next);
		if (next->raw_type == RT_F64) {
			CICArg orig = next->res;
			code_off = PutICArgIntoReg(cctrl, &next->res, next->raw_type, 2, bin, code_off);
			code_off = __ICMoveF64(cctrl, 1, rpn->integer, bin, code_off);
			code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
			AIWNIOS_ADD_CODE(f_op(next->res.reg, next->res.reg, 1));
			code_off = ICMov(cctrl, &orig, &next->res, bin, code_off);
		} else {
			CICArg orig = next->res;
			code_off = PutICArgIntoReg(cctrl, &next->res, next->raw_type, 1, bin, code_off);
			code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
      code=i_imm_op(next->res.reg, next->res.reg, rpn->integer);
      if(code!=ARM_ERR_INV_OFF) {
        AIWNIOS_ADD_CODE(code);
      } else {
        code_off=__ICMoveI64(cctrl,2,rpn->integer,bin,code_off);
        AIWNIOS_ADD_CODE(i_reg_op(next->res.reg, next->res.reg,2))
      }
			code_off = ICMov(cctrl, &orig, &next->res, bin, code_off);
		}
		TYPECAST_ASSIGN_END(next);
	} else {
		if (next->raw_type == RT_F64) {
			code_off = DerefToICArg(cctrl, &derefed, next, 3, bin, code_off);
			tmp = derefed; //==RT_F64
			code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 2, bin, code_off);
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
			code_off = __ICMoveF64(cctrl, 1, rpn->integer, bin, code_off);
			AIWNIOS_ADD_CODE(f_op(tmp.reg, tmp.reg, 1));
			code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
		} else {
			code_off = DerefToICArg(cctrl, &derefed, next, 3, bin, code_off);
			tmp = derefed;
			code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 2, bin, code_off);
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      code=i_imm_op(tmp.reg, tmp.reg, rpn->integer);
      if(code!=ARM_ERR_INV_OFF) {
        AIWNIOS_ADD_CODE(code);
      } else { 
        code_off=__ICMoveI64(cctrl,1,rpn->integer,bin,code_off);
        AIWNIOS_ADD_CODE(i_reg_op(tmp.reg, tmp.reg,1))
      }
			code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
		}
	}
	return code_off;
}
static int64_t FBitOp(CCmpCtrl* cctrl, CICArg* res, CICArg* next, CICArg* next2,
	int32_t (*bit_op)(int64_t, int64_t, int64_t), int shift,
	char* bin, int64_t code_off)
{
	CICArg tmp={0};
	if (shift) {
		code_off = PutICArgIntoReg(cctrl, next, RT_F64, 2, bin, code_off);
		code_off = PutICArgIntoReg(cctrl, next2, RT_I64i, 1, bin, code_off);
		AIWNIOS_ADD_CODE(ARM_fmovI64F64(2, next->reg));
		AIWNIOS_ADD_CODE(bit_op(2, 2, 1));
		if (res->mode == MD_REG) {
			AIWNIOS_ADD_CODE(ARM_fmovF64I64(res->reg, 2));
		} else {
			tmp.mode = MD_REG;
			tmp.reg = 2;
			tmp.raw_type = RT_F64;
			code_off = ICMov(cctrl, res, &tmp, bin, code_off);
		}
	} else {
		code_off = PutICArgIntoReg(cctrl, next, RT_F64, 2, bin, code_off);
		code_off = PutICArgIntoReg(cctrl, next2, RT_F64, 1, bin, code_off);
		if (next->raw_type != RT_F64) {
			AIWNIOS_ADD_CODE(ARM_fmovI64F64(2, next->reg));
			next->reg = 2;
			next->raw_type = RT_F64;
		}
		if (next2->raw_type != RT_F64) {
			next2->reg = 1;
			next2->raw_type = RT_F64;
			AIWNIOS_ADD_CODE(ARM_fmovI64F64(1, next2->reg));
		}
		AIWNIOS_ADD_CODE(bit_op(2, next2->reg, next->reg));
		if (res->mode == MD_REG) {
			AIWNIOS_ADD_CODE(ARM_fmovF64I64(res->reg, 2));
		} else {
			tmp.mode = MD_REG;
			tmp.reg = 2;
			tmp.raw_type = RT_F64;
			code_off = ICMov(cctrl, res, &tmp, bin, code_off);
		}
	}
	return code_off;
}
static int64_t __SexyAssignBitOp(CCmpCtrl* cctrl, CRPN* rpn,
	int32_t (*ubit_op)(int64_t, int64_t, int64_t),
	int32_t (*sbit_op)(int64_t, int64_t, int64_t),
	int64_t shift, char* bin, int64_t code_off)
{
	CRPN *next = ICArgN(rpn, 1), *next2, *next3, *b;
	CICArg tmp={0}, tmp2={0}, derefed={0};
	int64_t i = 0, tc = 0;
	b = ICArgN(rpn, 0);
	next2 = rpn->base.next;
	next3 = next2;
	if (SpillsTmpRegs(next2))
		PushSpilledTmp(cctrl, next3);
	else
		PushTmp(cctrl, next3, NULL);
	code_off = __OptPassFinal(cctrl, next3, bin, code_off);
	tmp2 = derefed;
	if (next->type == IC_TYPECAST) {
		tc = 1;
		TYPECAST_ASSIGN_BEGIN(next, b);
		goto enter;
	tc_reenter:;
		TYPECAST_ASSIGN_END(next);
		return code_off;
	}
enter:
	code_off = DerefToICArg(cctrl, &derefed, next2, 2, bin, code_off);
	PopTmp(cctrl, next2);
	code_off = PutICArgIntoReg(cctrl, &next3->res, rpn->raw_type, 1, bin, code_off);
	if (derefed.raw_type == RT_F64)
		code_off = FBitOp(cctrl, &derefed, &tmp2, &next3->res, sbit_op, shift, bin,
			code_off);
	else {
		code_off = PutICArgIntoReg(cctrl, &tmp2, rpn->raw_type, 3, bin, code_off);
		if (next3->res.raw_type == RT_F64) {
			tmp.reg = 0;
			tmp.mode = MD_REG;
			tmp.raw_type = RT_I64i;
			code_off = ICMov(cctrl, &tmp, &next3->res, bin, code_off);
		} else
			tmp = next3->res;
		switch (tmp2.raw_type) {
		case RT_I16i:
		case RT_I32i:
		case RT_I64i:
		case RT_I8i:
		case RT_PTR:
		case RT_FUNC: //func ptr
			AIWNIOS_ADD_CODE(sbit_op(tmp2.reg, tmp2.reg, &tmp));
			break;
		default:
			AIWNIOS_ADD_CODE(ubit_op(tmp2.reg, tmp2.reg, &tmp));
		}
	}
	code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
	if (tc)
		goto tc_reenter;
	return code_off;
}
static int64_t __SexyAssignOp(CCmpCtrl* cctrl, CRPN* rpn,
	int32_t (*i_op)(int64_t, int64_t, int64_t),
	int32_t (*i_imm_op)(int64_t, int64_t, int64_t),
	int32_t (*f_op)(int64_t, int64_t, int64_t),
	char* bin, int64_t code_off)
{
	CRPN *next = ICArgN(rpn, 1), *next2, *next3, *b;
	CICArg tmp={0}, tmp2={0}, derefed={0};
	int64_t i = 0,const_can=0;
	next2 = b = ICArgN(rpn, 0);
	if (SpillsTmpRegs(next))
		PushSpilledTmp(cctrl, next2);
	else
		PushTmp(cctrl, next2, NULL);
  if(i_imm_op)
    if(IsConst(b)&&ARM_ERR_INV_OFF!=i_imm_op(0,0,ConstVal(b)))
      const_can=1;
  if(const_can) {
     if (next->type == IC_TYPECAST) {
      TYPECAST_ASSIGN_BEGIN(next, b);
      PushTmp(cctrl,next2,NULL);
      tmp2 = next->res;
      if(tmp2.raw_type==RT_F64)
        goto defacto; 
      code_off = PutICArgIntoReg(cctrl, &tmp2, next->raw_type, 3, bin, code_off);
      AIWNIOS_ADD_CODE(i_imm_op(tmp2.reg,tmp2.reg,ConstVal(b)));
      code_off = ICMov(cctrl, &next->res, &tmp2, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
      TYPECAST_ASSIGN_END(next);
      PopTmp(cctrl,next2);
      return code_off;
    }  else {
      if(next->raw_type==RT_F64||rpn->raw_type==RT_F64) 
        goto defacto2;
      code_off = DerefToICArg(cctrl, &derefed, next, 2, bin, code_off);
      tmp2=derefed;
      code_off=PutICArgIntoReg(cctrl,&tmp2,RT_I64i,3,bin,code_off);
      AIWNIOS_ADD_CODE(i_imm_op(tmp2.reg,tmp2.reg,ConstVal(b)));
      PopTmp(cctrl, next2);
      code_off = ICMov(cctrl, &derefed, &tmp2, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
      return code_off;
    }
  } else {
    if (next->type == IC_TYPECAST) {
      TYPECAST_ASSIGN_BEGIN(next, b);
defacto:
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next2->res, next->raw_type, 1, bin,
        code_off); // 1
      tmp2 = next->res;
      code_off = PutICArgIntoReg(cctrl, &tmp2, next->raw_type, 3, bin, code_off);
      if (tmp2.raw_type == RT_F64) {
        AIWNIOS_ADD_CODE(f_op(tmp2.reg, tmp2.reg, b->res.reg));
      } else {
        AIWNIOS_ADD_CODE(i_op(tmp2.reg, tmp2.reg, b->res.reg));
      }
      code_off = ICMov(cctrl, &next->res, &tmp2, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
      TYPECAST_ASSIGN_END(next);
      PopTmp(cctrl, next2);
      return code_off;
    } else {
defacto2:
      //
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off = DerefToICArg(cctrl, &derefed, next, 2, bin, code_off);
      PopTmp(cctrl, next2);
      tmp2 = derefed;
      //Check if different
      if((tmp2.raw_type==RT_F64)^(b->res.raw_type==RT_F64)) {
          code_off=PutICArgIntoReg(cctrl,&b->res,RT_F64,1,bin,code_off);
          code_off=PutICArgIntoReg(cctrl,&tmp2,RT_F64,3,bin,code_off);
          AIWNIOS_ADD_CODE(f_op(tmp2.reg, tmp2.reg, b->res.reg));
      } else {
        //                ____________
        // (o)    (o)    /            \
          //  \      /    |             |
        //   \ ___/     |    tmp2 has |
        //  / _____\    |   register 3|
        // /  \___/ \___| as fallback |
        // \ _________________________/
        //
        code_off = PutICArgIntoReg(cctrl, &b->res, next->raw_type, 1, bin, code_off); // 1
        code_off = PutICArgIntoReg(cctrl, &tmp2, next->raw_type, 3, bin, code_off);
        if (tmp2.raw_type == RT_F64) {
          AIWNIOS_ADD_CODE(f_op(tmp2.reg, tmp2.reg, b->res.reg));
        } else {
          AIWNIOS_ADD_CODE(i_op(tmp2.reg, tmp2.reg, b->res.reg));
        }
      }
      code_off = ICMov(cctrl, &derefed, &tmp2, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
      return code_off;
    }
  }
	return code_off;
}
static int64_t __FMod(int64_t dst, int64_t a, int64_t b, char* bin,
	int64_t code_off)
{
	AIWNIOS_ADD_CODE(ARM_fdivReg(0, a, b));
	AIWNIOS_ADD_CODE(ARM_fcvtzs(0, 0))
	AIWNIOS_ADD_CODE(ARM_scvtf(0, 0));
	AIWNIOS_ADD_CODE(ARM_fmulReg(0, 0, b));
	AIWNIOS_ADD_CODE(ARM_fsubReg(dst, a, 0));
	return code_off;
}
// Also handles IC_MOD_EQ
static int64_t __CompileMod(CCmpCtrl* cctrl, CRPN* rpn, char* bin,
	int64_t code_off)
{
	CRPN *next, *next2, *tc;
	CICArg tmp={0}, orig_dst={0}, derefed={0}, tmp2={0};
	int64_t into_reg;
	next = ICArgN(rpn, 1);
	next2 = ICArgN(rpn, 0);
	if (SpillsTmpRegs(next))
		PushSpilledTmp(cctrl, next2);
	else
		PushTmp(cctrl, next2, NULL);
	if (next->type == IC_TYPECAST && rpn->type == IC_MOD_EQ) {
		code_off = __OptPassFinal(cctrl, next2, bin, code_off);
		TYPECAST_ASSIGN_BEGIN(next, next2);
		orig_dst = tmp;
		code_off = PutICArgIntoReg(cctrl, &tmp, next->raw_type, 2, bin, code_off);
		code_off = PutICArgIntoReg(cctrl, &next2->res, next->raw_type, 1, bin, code_off);
		if (next->raw_type == RT_F64) {
			code_off = __FMod(4, tmp.reg, next2->res.reg, bin, code_off);
			tmp.mode = MD_REG;
			tmp.raw_type = RT_F64;
			tmp.reg = 4;
			code_off = ICMov(cctrl, &orig_dst, &tmp, bin, code_off);
			PopTmp(cctrl, next2);
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
			goto tc_fin;
		}
		into_reg = 0;
    if(rpn->raw_type==RT_U64i) {
      AIWNIOS_ADD_CODE(ARM_udivX(into_reg, tmp.reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_umullX(into_reg, into_reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_subRegX(into_reg, tmp.reg, into_reg));
      AIWNIOS_ADD_CODE(ARM_uxtwX(into_reg,into_reg)); //umull takes into account 32bit operands,so I will "accomodate"
    } else {
      AIWNIOS_ADD_CODE(ARM_sdivRegX(into_reg, tmp.reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_mulRegX(into_reg, into_reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_subRegX(into_reg, tmp.reg, into_reg));
    }
    tmp.mode = MD_REG;
		tmp.raw_type = RT_I64i;
		tmp.reg = into_reg;
		tmp.off = 0;
		code_off = ICMov(cctrl, &orig_dst, &tmp, bin, code_off);
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
	tc_fin:
		TYPECAST_ASSIGN_END(next);
		PopTmp(cctrl, next2);
		return code_off;
	}
	if (rpn->type == IC_MOD_EQ) {
		code_off = __OptPassFinal(cctrl, next2, bin, code_off);
		code_off = DerefToICArg(cctrl, &tmp, next, 3, bin, code_off);
		orig_dst = tmp;
		code_off = PutICArgIntoReg(cctrl, &tmp, next->raw_type, 2, bin, code_off);
		code_off = PutICArgIntoReg(cctrl, &next2->res, next->raw_type, 1, bin, code_off);
		if (next->raw_type == RT_F64) {
			code_off = __FMod(4, tmp.reg, next2->res.reg, bin, code_off);
			tmp.mode = MD_REG;
			tmp.raw_type = RT_F64;
			tmp.reg = 4;
			code_off = ICMov(cctrl, &orig_dst, &tmp, bin, code_off);
			PopTmp(cctrl, next2);
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
			return code_off;
		}
		into_reg = 0;
		if(next->raw_type==RT_U64i) {
      AIWNIOS_ADD_CODE(ARM_udivX(into_reg, tmp.reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_umullX(into_reg, into_reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_subRegX(into_reg, tmp.reg, into_reg));
      AIWNIOS_ADD_CODE(ARM_uxtwX(into_reg,into_reg)); //umull takes into account 32bit operands,so I will "accomodate"
    } else {
      AIWNIOS_ADD_CODE(ARM_sdivRegX(into_reg, tmp.reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_mulRegX(into_reg, into_reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_subRegX(into_reg, tmp.reg, into_reg));
    }
		tmp.mode = MD_REG;
		tmp.raw_type = RT_I64i;
		tmp.reg = into_reg;
		tmp.off = 0;
		code_off = ICMov(cctrl, &orig_dst, &tmp, bin, code_off);
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		PopTmp(cctrl, next2);
		return code_off;
	}
	PushTmp(cctrl, next, &rpn->res);
	code_off = __OptPassFinal(cctrl, next2, bin, code_off);
	code_off = __OptPassFinal(cctrl, next, bin, code_off);
	code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type, 1, bin, code_off);
	code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 2, bin, code_off);
	if (rpn->raw_type == RT_F64) {
		code_off = __FMod(4, next->res.reg, next2->res.reg, bin, code_off);
		tmp.mode = MD_REG;
		tmp.raw_type = RT_F64;
		tmp.reg = 4;
	} else {
		into_reg = 0;
		tmp.reg = 0;
		tmp.raw_type = RT_I64i;
		tmp.mode = MD_REG;
		if(rpn->raw_type==RT_U64i) {
      AIWNIOS_ADD_CODE(ARM_udivX(into_reg, next->res.reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_umullX(into_reg, into_reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_subRegX(into_reg, next->res.reg, into_reg));
      AIWNIOS_ADD_CODE(ARM_uxtwX(into_reg,into_reg)); //umull takes into account 32bit operands,so I will "accomodate"
    } else {
      AIWNIOS_ADD_CODE(ARM_sdivRegX(into_reg, next->res.reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_mulRegX(into_reg, into_reg, next2->res.reg));
      AIWNIOS_ADD_CODE(ARM_subRegX(into_reg, next->res.reg, into_reg));
    }
	}
	code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
	PopTmp(cctrl, next);
	PopTmp(cctrl, next2);
	return code_off;
}

// This is used for FuncProlog/Epilog. It is used for finding the non-violatle
// registers that must be saved and pushed to the stack
//
// to_push is a char[32] ,1 is push,0 is not push
//
// Returns number of pushed regs
//
static int64_t __FindPushedIRegs(CCmpCtrl* cctrl, char* to_push)
{
	CMemberLst* lst;
	CRPN* r;
	int64_t cnt = 0;
	memset(to_push, 0, 32);
	if (!cctrl->cur_fun) {
		//Perhaps we are being called from HolyC and we aren't using a "cur_fun"
		for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code; r = r->base.next) {
			if (r->type == IC_IREG && r->raw_type != RT_F64) {
				if (!to_push[r->integer])
					if (AIWNIOS_IREG_START <= r->integer)
						if (AIWNIOS_IREG_CNT > r->integer - AIWNIOS_IREG_START) {
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

static int64_t __FindPushedFRegs(CCmpCtrl* cctrl, char* to_push)
{
	CMemberLst* lst;
	int64_t cnt = 0;
	memset(to_push, 0, 32);
	CRPN* r;
	if (!cctrl->cur_fun) {
		//Perhaps we are being called from HolyC and we aren't using a "cur_fun"
		for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code; r = r->base.next) {
			if (r->type == IC_FREG && r->raw_type == RT_F64) {
				if (!to_push[r->integer])
					if (AIWNIOS_FREG_START <= r->integer)
						if (AIWNIOS_FREG_CNT > r->integer - AIWNIOS_FREG_START) {
							to_push[r->integer] = 1;
							cnt++;
						}
			}
		}
		return cnt;
	}
	for (lst = cctrl->cur_fun->base.members_lst; lst; lst = lst->next) {
		if (lst->reg != REG_NONE && lst->member_class->raw_type == RT_F64) {
			assert(lst->reg >= 0);
			to_push[lst->reg] = 1;
			cnt++;
		}
	}
	return cnt;
}

static int64_t FuncProlog(CCmpCtrl* cctrl, char* bin, int64_t code_off)
{
	char push_ireg[32];
	char push_freg[32];
  char ilist[32];
  char flist[32];
	int64_t i,i2,i3, r, r2, ireg_arg_cnt, freg_arg_cnt, stk_arg_cnt, fsz,code,off;
	CMemberLst* lst;
	CICArg fun_arg={0}, write_to={0};
	CRPN *rpn, *arg;
	/* Old SP
   * saved regs
   * wiggle room
   * locals
   * old_FP,old_LR
   * <===FP=SP
   */
	if (cctrl->cur_fun) {
		fsz = cctrl->cur_fun->base.sz + 16; //+16 for LR/FP
	} else {
		fsz = 16;
		for (rpn = cctrl->code_ctrl->ir_code->next; rpn != cctrl->code_ctrl->ir_code; rpn = rpn->base.next) {
			if (rpn->type == __IC_SET_FRAME_SIZE) {
				fsz = rpn->integer;
				break;
			}
		}
	}
	//ALIGN TO 8
	if (fsz % 8)
		fsz += 8 - fsz % 8;
	int64_t to_push = (i2=__FindPushedIRegs(cctrl, push_ireg)) * 8 + (i3=__FindPushedFRegs(cctrl, push_freg)) * 8 + cctrl->backend_user_data0 + fsz, old_regs_start;
  if(i2%2) to_push+=8; //We push a dummy register in a pair if not aligned to 2
  if(i3%2) to_push+=8; //Ditto
  
  if (to_push % 16)
		to_push += 8;
	cctrl->backend_user_data4 = fsz;
	old_regs_start = fsz + cctrl->backend_user_data0;
  
  i2=0;
	for (i = 0; i != 32; i++)
		if (push_ireg[i])
      ilist[i2++]=i;
  if(i2%2)
    ilist[i2++]=1; //Dont use 0 as it is a reutrn register
  i3=0;
	for (i = 0; i != 32; i++)
		if (push_freg[i])
      flist[i3++]=i;
  if(i3%2)
    flist[i3++]=1;  //Dont use 0 as it is a reutrn register
    
  //<==== OLD SP
  //first saved reg pair<==-16
  //next saved reg pair<===-32
  // 	
  off=16; 
  for(i=0;i!=i2;i+=2) {
      AIWNIOS_ADD_CODE(ARM_stpImmX(ilist[i],ilist[i+1],ARM_REG_SP,-off));
      off+=16;
  }
	for(i=0;i!=i3;i+=2) {
      AIWNIOS_ADD_CODE(ARM_stpImmF64(flist[i],flist[i+1],ARM_REG_SP,-off));
      off+=16;
  }
	if (ARM_ERR_INV_OFF != ARM_subImmX(ARM_REG_SP, ARM_REG_SP, to_push)) {
		AIWNIOS_ADD_CODE(ARM_subImmX(ARM_REG_SP, ARM_REG_SP, to_push));
	} else {
		code_off = __ICMoveI64(cctrl, 8, to_push, bin, code_off);
		AIWNIOS_ADD_CODE(ARM_subRegX(ARM_REG_SP, ARM_REG_SP, 8));
	}
  AIWNIOS_ADD_CODE(ARM_stpImmX(ARM_REG_FP,ARM_REG_LR,ARM_REG_SP,0));
  AIWNIOS_ADD_CODE(ARM_movRegX(ARM_REG_FP,ARM_REG_SP));
	stk_arg_cnt = ireg_arg_cnt = freg_arg_cnt = 0;
	if (cctrl->cur_fun) {
		lst = cctrl->cur_fun->base.members_lst;
		for (i = 0; i != cctrl->cur_fun->argc; i++) {
			if (lst->member_class->raw_type == RT_F64) {
				if (freg_arg_cnt < 8) {
					fun_arg.mode = MD_REG;
					fun_arg.raw_type = RT_F64;
					fun_arg.reg = freg_arg_cnt++;
				} else {
				stk:
					fun_arg.mode = MD_FRAME;
					fun_arg.raw_type = lst->member_class->raw_type;
					fun_arg.off = to_push + stk_arg_cnt++ * 8;
				}
			} else {
				if (ireg_arg_cnt < 8) {
					fun_arg.mode = MD_REG;
					fun_arg.raw_type = RT_I64i;
					fun_arg.reg = ireg_arg_cnt++;
				} else
					goto stk;
			}
			// This *shoudlnt* mutate any of the argument registers
			if (lst->reg >= 0 && lst->reg != REG_NONE) {
				write_to.mode = MD_REG;
				write_to.reg = lst->reg;
				write_to.off = 0;
				write_to.raw_type = lst->member_class->raw_type;
			} else {
				write_to.mode = MD_FRAME;
				write_to.off = lst->off + 16; //We added 16 for FP/LR everywhere else
				write_to.raw_type = lst->member_class->raw_type;
			}
			lst = lst->next;
			code_off = ICMov(cctrl, &write_to, &fun_arg, bin, code_off);
		}
	} else {
		//Things from the HolyC side use __IC_ARG
		//We go backwards as this is REVERSE polish notation
		for (rpn = cctrl->code_ctrl->ir_code->last; rpn != cctrl->code_ctrl->ir_code; rpn = rpn->base.last) {
			if (rpn->type == __IC_ARG) {
				if ((arg = ICArgN(rpn, 0))->raw_type == RT_F64) {
					if (freg_arg_cnt < 8) {
						fun_arg.mode = MD_REG;
						fun_arg.raw_type = RT_F64;
						fun_arg.reg = freg_arg_cnt++;
					} else {
					stk2:
						fun_arg.mode = MD_FRAME;
						fun_arg.raw_type = arg->raw_type;
						if (arg->raw_type < RT_I64i)
							arg->raw_type = RT_I64i;
						fun_arg.off = to_push + stk_arg_cnt++ * 8;
					}
				} else {
					if (ireg_arg_cnt < 8) {
						fun_arg.mode = MD_REG;
						fun_arg.raw_type = RT_I64i;
						fun_arg.reg = ireg_arg_cnt++;
					} else
						goto stk2;
				}

				PushTmp(cctrl, arg, NULL); //This will default to the things reg/frame address
				code_off = ICMov(cctrl, &arg->res, &fun_arg, bin, code_off);
				PopTmp(cctrl, arg);
			}
		}
	}
	return code_off;
}

//
// DO NOT CHANGE WITHOUT LOOKING CLOSEY AT FuncProlog
//
static int64_t FuncEpilog(CCmpCtrl* cctrl, char* bin, int64_t code_off)
{
	char push_ireg[32],ilist[32];
	char push_freg[32],flist[32];
	int64_t i, r, r2, fsz,i2,i3,off;
	CICArg spill_loc={0}, write_to={0};
	CRPN* rpn;
	/* <== OLD SP
   * saved regs
   * wiggle room
   * locals
   * OLD FP,LR<===FP=SP
   */
	if (cctrl->cur_fun)
		fsz = cctrl->cur_fun->base.sz + 16; //+16 for LR/FP
	else {
		fsz = 16;
		for (rpn = cctrl->code_ctrl->ir_code->next; rpn != cctrl->code_ctrl->ir_code; rpn = rpn->base.next) {
			if (rpn->type == __IC_SET_FRAME_SIZE) {
				fsz = rpn->integer;
				break;
			}
		}
	}
	//ALIGN TO 8
	if (fsz % 8)
		fsz += 8 - fsz % 8;
	int64_t to_push = (i2=__FindPushedIRegs(cctrl, push_ireg)) * 8 + (i3=__FindPushedFRegs(cctrl, push_freg)) * 8 + fsz + cctrl->backend_user_data0, old_regs_start; // old_FP,old_LR
	if(i2%2) to_push+=8; //We push a dummy register in a pair if not aligned to 2
  if(i3%2) to_push+=8; //Ditto
  if (to_push % 16)
		to_push += 8;
	old_regs_start = fsz + cctrl->backend_user_data0;
  i2=0;
	for (i = 0; i != 32; i++)
		if (push_ireg[i])
      ilist[i2++]=i;
  if(i2%2)
    ilist[i2++]=1;  //Dont use 0 as it is a reutrn register
  i3=0;
	for (i = 0; i != 32; i++)
		if (push_freg[i])
      flist[i3++]=i;
  if(i3%2)
    flist[i3++]=1;  //Dont use 0 as it is a reutrn register
  
  AIWNIOS_ADD_CODE(ARM_ldpImmX(ARM_REG_FP,ARM_REG_LR,ARM_REG_SP,0));

  if (ARM_ERR_INV_OFF != ARM_addImmX(ARM_REG_SP, ARM_REG_SP, to_push)) {
		AIWNIOS_ADD_CODE(ARM_addImmX(ARM_REG_SP, ARM_REG_SP, to_push));
	} else {
		code_off = __ICMoveI64(cctrl, 8, to_push, bin, code_off);
		AIWNIOS_ADD_CODE(ARM_addRegX(ARM_REG_SP, ARM_REG_SP, 8));
	}

  
  off=16;
  //<==== OLD SP
  //first saved reg pair<==-16
  //next saved reg pair<===-32
  // 	
	for(i=0;i!=i2;i+=2) {
      AIWNIOS_ADD_CODE(ARM_ldpImmX(ilist[i],ilist[i+1],ARM_REG_SP,-off));
      off+=16;
  }
	for(i=0;i!=i3;i+=2) {
      AIWNIOS_ADD_CODE(ARM_ldpImmF64(flist[i],flist[i+1],ARM_REG_SP,-off));
      off+=16;
  }
	AIWNIOS_ADD_CODE(ARM_ret());
	return code_off;
}
static int64_t IsCompoundCompare(CRPN* r)
{
	CRPN* next = r->base.next;
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
	case IC_NE:
	case IC_EQ_EQ:
		next = ICFwd(next);
		switch (next->type) {
		case IC_EQ_EQ:
		case IC_NE:
			return 1;
		}
		return 0;
	}
	return 0;
}
//
// ALWAYS ASSUME WORST CASE if we dont have a ALLOC'ed peice of RAM
//
static int64_t __OptPassFinal(CCmpCtrl* cctrl, CRPN* rpn, char* bin,
	int64_t code_off)
{
	CCodeMisc* misc;
	CRPN *next, *next2, **range, **range_args, *next3, *a, *b;
	CICArg tmp={0}, orig_dst={0}, tmp2={0};
	int64_t i = 0, cnt, i2, use_reg, a_reg, b_reg, into_reg, use_flt_cmp, reverse;
	int64_t* range_cmp_types,use_flags=rpn->res.set_flags;
  rpn->res.set_flags=0;
	char *enter_addr2, *enter_addr, *exit_addr, **fail1_addr, **fail2_addr, ***range_fail_addrs;
	if (cctrl->code_ctrl->dbg_info && cctrl->code_ctrl->final_pass == 3 && rpn->ic_line) { //Final run
    if(MSize(cctrl->code_ctrl->dbg_info)/8>rpn->ic_line - cctrl->code_ctrl->min_ln) {
      i=cctrl->code_ctrl->dbg_info[rpn->ic_line - cctrl->code_ctrl->min_ln];
      if(!i)
        cctrl->code_ctrl->dbg_info[rpn->ic_line - cctrl->code_ctrl->min_ln] = bin + code_off;
      else if((int64_t)bin+code_off<i)
        cctrl->code_ctrl->dbg_info[rpn->ic_line - cctrl->code_ctrl->min_ln] = bin + code_off;
    }
	}
	switch (rpn->type) {
		break;
	case IC_SHORT_ADDR:
		//This is used for function calls only!!!
		abort();
		break;
	case IC_RELOC:
		if (rpn->res.mode == MD_REG)
			into_reg = rpn->res.reg;
		else
			into_reg = 0;
		misc = rpn->code_misc;
    i=-code_off+misc->aot_before_hint;
    //See __HC_SetAOTRelocBeforeRIP(Seriously read it) 
    if((cctrl->flags&CCF_AOT_COMPILE)&&(ARM_ERR_INV_OFF!=ARM_adrX(into_reg,i))&&misc->aot_before_hint<0) {
      AIWNIOS_ADD_CODE(ARM_adrX(into_reg,i));
    } else if (cctrl->code_ctrl->final_pass >= 2) {
      misc->use_cnt++;
			AIWNIOS_ADD_CODE(ARM_ldrLabelX(into_reg, (char*)misc->addr - (bin + code_off)))
		} else {
			AIWNIOS_ADD_CODE(0);
    }
		if (rpn->res.mode != MD_REG) {
			tmp.mode = MD_REG;
			tmp.raw_type = RT_I64i;
			tmp.reg = 0;
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		}
		break;
	case IC_GOTO_IF:
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
#define CMP_AND_JMP(COND)                                                                            \
	{                                                                                                \
		next3 = next->base.next;                                                                     \
		next2 = ICFwd(next3);                                                                        \
		PushTmp(cctrl, next2, NULL);                                                                 \
		if (SpillsTmpRegs(next2))                                                                    \
			PushSpilledTmp(cctrl, next3);                                                            \
		else                                                                                         \
			PushTmp(cctrl, next3, NULL);   \
		code_off = __OptPassFinal(cctrl, next3, bin, code_off);                                      \
		code_off = __OptPassFinal(cctrl, next2, bin, code_off);                                      \
		if (next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {                                \
			code_off = PutICArgIntoReg(cctrl, &next2->res, RT_F64, 2, bin, code_off);                \
			code_off = PutICArgIntoReg(cctrl, &next3->res, RT_F64, 3, bin, code_off);                \
			AIWNIOS_ADD_CODE(ARM_fcmp(next2->res.reg, next3->res.reg));                              \
		} else {                                                                                     \
			code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i, 2, bin, code_off);               \
			code_off = PutICArgIntoReg(cctrl, &next3->res, RT_I64i, 3, bin, code_off);               \
			AIWNIOS_ADD_CODE(ARM_cmpRegX(next2->res.reg, next3->res.reg));                           \
		}                                                                                            \
		PopTmp(cctrl, next3);                                                                        \
		PopTmp(cctrl, next2);                                                                        \
		if (cctrl->code_ctrl->final_pass >= 2) {                                                     \
			if (reverse) {                                                                           \
				AIWNIOS_ADD_CODE(ARM_bcc(COND ^ 1, (char*)rpn->code_misc->addr - (bin + code_off))); \
			} else                                                                                   \
				AIWNIOS_ADD_CODE(ARM_bcc(COND, (char*)rpn->code_misc->addr - (bin + code_off)));     \
		} else                                                                                       \
			AIWNIOS_ADD_CODE(0);                                                                     \
	}
				CMP_AND_JMP(ARM_EQ);
				return code_off;
				break;
			case IC_NE:
				CMP_AND_JMP(ARM_NE);
				return code_off;
				break;
			case IC_LT:
        next3 = next->base.next;                                                                    
        next2 = ICFwd(next3);                                                                        
        if(next3->raw_type == RT_U64i || next2->raw_type == RT_U64i) {
          CMP_AND_JMP(ARM_LO);
        } else {
          CMP_AND_JMP(ARM_LT);
        }
				return code_off;
				break;
			case IC_GT:
				next3 = next->base.next;                                                                    
        next2 = ICFwd(next3);                                                                        
        if(next3->raw_type == RT_U64i || next2->raw_type == RT_U64i) {
          CMP_AND_JMP(ARM_HI);
        } else {
          CMP_AND_JMP(ARM_GT);
        }
				return code_off;
				break;
			case IC_LE:
				next3 = next->base.next;                                                                    
        next2 = ICFwd(next3);                                                                        
        if(next3->raw_type == RT_U64i || next2->raw_type == RT_U64i) {
          CMP_AND_JMP(ARM_LS);
        } else {
          CMP_AND_JMP(ARM_LE);
        }
				return code_off;
				break;
			case IC_GE:
        next3 = next->base.next;                                                                    
        next2 = ICFwd(next3);                                                                        
        if(next3->raw_type == RT_U64i || next2->raw_type == RT_U64i) {
          CMP_AND_JMP(ARM_HS);
        } else {
          CMP_AND_JMP(ARM_GE);
        }
				return code_off;
			}

		PushTmp(cctrl, next, NULL);
    next->res.set_flags=1;
		code_off = __OptPassFinal(cctrl, next, bin, code_off);
    if(next->res.set_flags) {
      if (cctrl->code_ctrl->final_pass >= 2) {
        if(reverse) {
          AIWNIOS_ADD_CODE(ARM_bcc(ARM_EQ,(char*)rpn->code_misc->addr - (bin + code_off)));
        } else {
          AIWNIOS_ADD_CODE(ARM_bcc(ARM_NE,(char*)rpn->code_misc->addr - (bin + code_off)));
        }
      } else {
        AIWNIOS_ADD_CODE(0);
      }
    } else if (next->raw_type == RT_F64) {
			code_off = PutICArgIntoReg(cctrl, &next->res, RT_F64, 1, bin, code_off);
			code_off = __ICMoveF64(cctrl, 0, 0, bin, code_off);
			AIWNIOS_ADD_CODE(ARM_fcmp(next->res.reg, 0));
			if (cctrl->code_ctrl->final_pass >= 2) {
				if (!reverse)
					AIWNIOS_ADD_CODE(ARM_bcc(ARM_NE, (char*)rpn->code_misc->addr - (bin + code_off)))
				else
					AIWNIOS_ADD_CODE(ARM_bcc(ARM_EQ, (char*)rpn->code_misc->addr - (bin + code_off)))
			} else
				AIWNIOS_ADD_CODE(0);
		} else {
			code_off = PutICArgIntoReg(cctrl, &next->res, RT_I64i, 0, bin, code_off);
			if (cctrl->code_ctrl->final_pass >= 2) {
				if (!reverse)
					AIWNIOS_ADD_CODE(ARM_cbnzX(next->res.reg, (char*)rpn->code_misc->addr - (bin + code_off)))
				else
					AIWNIOS_ADD_CODE(ARM_cbzX(next->res.reg, (char*)rpn->code_misc->addr - (bin + code_off)))
			} else
				AIWNIOS_ADD_CODE(0);
		}
		PopTmp(cctrl, next);
		break;
		//These both do the same thing,only thier type detirmines what happens
	case IC_TO_F64:
	case IC_TO_I64:
		PushTmp(cctrl, next = rpn->base.next, &rpn->res);
		code_off = __OptPassFinal(cctrl, next, bin, code_off);
		code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
		PopTmp(cctrl, next);
		break;
	case IC_TYPECAST:
		next = rpn->base.next;
		PushTmp(cctrl, next, &rpn->res);
		if (next->raw_type == RT_F64 && rpn->raw_type != RT_F64) {
			code_off = __OptPassFinal(cctrl, next, bin, code_off);
			code_off = PutICArgIntoReg(cctrl, &next->res, next->raw_type, 0, bin, code_off);
			if (rpn->res.reg == MD_REG) {
				AIWNIOS_ADD_CODE(ARM_fmovI64F64(rpn->res.reg, next->res.reg));
			} else {
				AIWNIOS_ADD_CODE(ARM_fmovI64F64(1, next->res.reg));
				tmp.mode = MD_REG;
				tmp.raw_type = RT_I64i;
				tmp.reg = 1;
				code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
			}
		} else if (next->raw_type != RT_F64 && rpn->raw_type == RT_F64) {
			code_off = __OptPassFinal(cctrl, next, bin, code_off);
			code_off = PutICArgIntoReg(cctrl, &next->res, next->raw_type, 0, bin, code_off);
			if (rpn->res.mode == MD_REG) {
				AIWNIOS_ADD_CODE(ARM_fmovF64I64(rpn->res.reg, next->res.reg));
			} else {
				AIWNIOS_ADD_CODE(ARM_fmovF64I64(1, next->res.reg));
				tmp.mode = MD_REG;
				tmp.raw_type = RT_F64;
				tmp.reg = 1;
				code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
			}
		} else if (next->type == IC_DEREF) {
			code_off = DerefToICArg(cctrl, &tmp, next, 1, bin, code_off);
			tmp.raw_type = rpn->raw_type;
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		} else {
			code_off = __OptPassFinal(cctrl, next, bin, code_off);
			code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
		}
		PopTmp(cctrl, next);
		break;
	case IC_GOTO:
		if (cctrl->code_ctrl->final_pass >= 2) {
			AIWNIOS_ADD_CODE(ARM_b((char*)rpn->code_misc->addr - (code_off + bin)));
		} else
			AIWNIOS_ADD_CODE(ARM_b(0));
		break;
	case IC_LABEL:
		rpn->code_misc->addr = bin + code_off;
		break;
	case IC_GLOBAL:
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
			tmp.off = rpn->global_var->data_addr;
			tmp.raw_type = rpn->global_var->var_class->raw_type;
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		}
		break;
	case IC_NOP:
		// Bungis
		break;
	case IC_PAREN:
		abort();
		break;
	case IC_NEG:
#define BACKEND_UNOP(flt_op, int_op)                                                \
	next = rpn->base.next;                                                          \
	PushTmp(cctrl, next, &rpn->res);                                                \
	code_off = __OptPassFinal(cctrl, next, bin, code_off);                          \
	PopTmp(cctrl, next);                                                            \
	if (rpn->res.mode == MD_REG) {                                                  \
		into_reg = rpn->res.reg;                                                    \
	} else                                                                          \
		into_reg = 0;                                                               \
	code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 0, bin, code_off); \
	if (rpn->raw_type == RT_F64) {                                                  \
		AIWNIOS_ADD_CODE(flt_op(into_reg, next->res.reg));                          \
	} else                                                                          \
		AIWNIOS_ADD_CODE(int_op(into_reg, next->res.reg));                          \
	if (rpn->res.mode != MD_REG) {                                                  \
		tmp.raw_type = rpn->raw_type;                                               \
		tmp.reg = 0;                                                                \
		tmp.mode = MD_REG;                                                          \
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                    \
	}
		BACKEND_UNOP(ARM_fnegReg, ARM_negsRegX);
		break;
	case IC_POS:
		BACKEND_UNOP(ARM_fmovReg, ARM_movRegX);
		break;
	case IC_NAME:
		abort();
		break;
	case IC_STR:
		if (rpn->res.mode == MD_REG) {
			into_reg = rpn->res.reg;
		} else
			into_reg = 0; // Store into reg 0

		if (cctrl->flags & CCF_STRINGS_ON_HEAP) {
			//
			// README: We will "NULL"ify the rpn->code_misc->str later so we dont free it
			//
			code_off = __ICMoveI64(cctrl, into_reg, rpn->code_misc->str, bin, code_off);
		} else {
			if (cctrl->code_ctrl->final_pass >= 2) { //2 is when we finally write the colde
				AIWNIOS_ADD_CODE(
					ARM_adrX(into_reg, (char*)rpn->code_misc->addr - (bin + code_off)));
			} else {
				AIWNIOS_ADD_CODE(0);
			}
		}
		if (rpn->res.mode != MD_REG) {
			tmp.raw_type = rpn->raw_type;
			tmp.reg = 0;
			tmp.mode = MD_REG;
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		}
		break;
	case IC_CHR:
		if (rpn->res.mode == MD_REG) {
			into_reg = rpn->res.reg;
		} else
			into_reg = 0; // Store into reg 0

		code_off = __ICMoveI64(cctrl, into_reg, rpn->integer, bin, code_off);

		if (rpn->res.mode != MD_REG) {
			tmp.raw_type = rpn->raw_type;
			tmp.reg = 0;
			tmp.mode = MD_REG;
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		}
		break;
	case IC_POW:
		// TODO
		break;
		break;
	case IC_UNBOUNDED_SWITCH:
		//
		// Load poop into tmp,then we continue the sexyness as normal
		//
		// Also make sure X1 has lo bound(See IC_BOUNDED_SWITCH)
		//
		next2 = ICArgN(rpn, 0);
		PushTmp(cctrl, next2, NULL);
		PopTmp(cctrl, next2);
		tmp.raw_type = RT_I64i;
		tmp.mode = MD_REG;
		tmp.reg = 0;
		code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
		code_off = __ICMoveI64(cctrl, 1, rpn->code_misc->lo, bin, code_off); // X1
		goto jmp_tab_sexy;
		break;
	case IC_BOUNDED_SWITCH:
		next2 = ICArgN(rpn, 0);
		PushTmp(cctrl, next2, NULL);
		code_off = __OptPassFinal(cctrl, next2, bin, code_off);
		PopTmp(cctrl, next2);
		tmp.raw_type = RT_I64i;
		tmp.mode = MD_REG;
		tmp.reg = 0;
		code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
		code_off = __ICMoveI64(cctrl, 1, rpn->code_misc->lo, bin, code_off);
		code_off = __ICMoveI64(cctrl, 2, rpn->code_misc->hi, bin, code_off);
		AIWNIOS_ADD_CODE(ARM_cmpRegX(tmp.reg, 1));
		fail1_addr = bin + code_off;
		AIWNIOS_ADD_CODE(ARM_bcc(ARM_LT, 0));
		AIWNIOS_ADD_CODE(ARM_cmpRegX(tmp.reg, 2));
		fail2_addr = bin + code_off;
		AIWNIOS_ADD_CODE(ARM_bcc(ARM_GT, 0));
	jmp_tab_sexy:
		//
		//See LAMA snail
		//
		// The jump table offsets are relative to the start of the function
		//   to make it so the code is Position-Independant
		AIWNIOS_ADD_CODE(ARM_subRegX(tmp.reg, tmp.reg, 1)); // 1 has low bound
		if (cctrl->code_ctrl->final_pass >= 2) {
			AIWNIOS_ADD_CODE(
				ARM_adrX(2, (char*)rpn->code_misc->addr - (code_off + bin)));
		} else {
			AIWNIOS_ADD_CODE(0);
		}
		AIWNIOS_ADD_CODE(ARM_ldrRegRegShiftX(tmp.reg, 2, tmp.reg));
		//Load the function base address
		AIWNIOS_ADD_CODE(ARM_adrX(2, -code_off));
		AIWNIOS_ADD_CODE(ARM_addRegX(tmp.reg, 2, tmp.reg));
		AIWNIOS_ADD_CODE(ARM_br(tmp.reg));
		if (rpn->type == IC_BOUNDED_SWITCH && cctrl->code_ctrl->final_pass >= 2) {
			*(int32_t*)fail1_addr = ARM_bcc(
				ARM_LT, (char*)rpn->code_misc->dft_lab->addr - (char*)fail1_addr);
			*(int32_t*)fail2_addr = ARM_bcc(
				ARM_GT, (char*)rpn->code_misc->dft_lab->addr - (char*)fail2_addr);
		}
		break;
	case IC_SUB_RET:
		AIWNIOS_ADD_CODE(ARM_ldrPostImmX(ARM_REG_LR, ARM_REG_SP, 16));
		AIWNIOS_ADD_CODE(ARM_ret());
		break;
	case IC_SUB_PROLOG:
		break;
	case IC_SUB_CALL:
		// adr LR,exit +0
		// str LR,[sp-=16] +4
		// b start_enter +8
		// exit: +12
		AIWNIOS_ADD_CODE(ARM_adrX(ARM_REG_LR, 12));
		AIWNIOS_ADD_CODE(ARM_strPreImmX(ARM_REG_LR, ARM_REG_SP, -16));
		if (cctrl->code_ctrl->final_pass >= 2) {
			AIWNIOS_ADD_CODE(ARM_b((char*)rpn->code_misc->addr - (bin + code_off)));
		} else
			AIWNIOS_ADD_CODE(0);
		break;
	case IC_ADD:
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
#define BACKEND_BINOP(f_op, i_op)                                                    \
	next = ICArgN(rpn, 1);                                                           \
	next2 = ICArgN(rpn, 0);                                                          \
	orig_dst = next->res;                                                            \
	if (!SpillsTmpRegs(next))                                                        \
		PushTmp(cctrl, next2, NULL);                                                 \
	else                                                                             \
		PushSpilledTmp(cctrl, next2);                                                \
	PushTmp(cctrl, next, &rpn->res);                                                 \
	code_off = __OptPassFinal(cctrl, next2, bin, code_off);                          \
	code_off = __OptPassFinal(cctrl, next, bin, code_off);                           \
	PopTmp(cctrl, next2);                                                            \
	PopTmp(cctrl, next);                                                             \
	if (rpn->res.mode == MD_REG) {                                                   \
		into_reg = rpn->res.reg;                                                     \
	} else                                                                           \
		into_reg = 0;                                                                \
	code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 2, bin, code_off);  \
	code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type, 1, bin, code_off); \
	if (rpn->raw_type == RT_F64) {                                                   \
		AIWNIOS_ADD_CODE(f_op(into_reg, next->res.reg, next2->res.reg));             \
	} else {                                                                         \
		AIWNIOS_ADD_CODE(i_op(into_reg, next->res.reg, next2->res.reg));             \
	}                                                                                \
	if (rpn->res.mode != MD_REG&&rpn->res.mode!=MD_NULL) {                                                   \
		tmp.raw_type = rpn->raw_type;                                                \
		tmp.reg = 0;                                                                 \
		tmp.mode = MD_REG;                                                           \
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                     \
	}
#define BACKEND_BINOP_IMM(i_imm_op, i_op)                                           \
	next = ICArgN(rpn, 1);                                                         \
	next2 = ICArgN(rpn, 0);                                                        \
	if (rpn->raw_type != RT_F64 && IsConst(next2) && next2->type != IC_F64&& \
    next2->integer>=0) {      \
		PushTmp(cctrl, next, &rpn->res);                                           \
		code_off = __OptPassFinal(cctrl, next, bin, code_off);                     \
		code_off = PutICArgIntoReg(cctrl, &next->res, RT_I64i, 1, bin, code_off);  \
		if (rpn->res.mode == MD_REG)                                               \
			into_reg = rpn->res.reg;                                               \
		else                                                                       \
			into_reg = 0;                                                          \
		if (i_imm_op(into_reg, next->res.reg, next2->integer) != ARM_ERR_INV_OFF) { \
			AIWNIOS_ADD_CODE(i_imm_op(into_reg, next->res.reg, next2->integer));    \
		} else {                                                                   \
			tmp.mode = MD_REG;                                                     \
			tmp.reg = 2;                                                           \
			tmp.raw_type = RT_I64i;                                                \
			PushTmp(cctrl, next2, NULL);                                           \
			code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);             \
			AIWNIOS_ADD_CODE(i_op(into_reg, next->res.reg, tmp.reg));              \
			PopTmp(cctrl, next2);                                                  \
		}                                                                          \
		if (rpn->res.mode != MD_REG&&rpn->res.mode!=MD_NULL) {                                             \
			tmp.raw_type = rpn->raw_type;                                          \
			tmp.reg = 0;                                                           \
			tmp.mode = MD_REG;                                                     \
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);               \
		}                                                                          \
		PopTmp(cctrl, next);                                                       \
		break;                                                                     \
	}
    if(use_flags&&rpn->res.raw_type!=RT_F64) {
      rpn->res.set_flags=1;
      rpn->res.mode=MD_NULL;
      BACKEND_BINOP_IMM(ARM_addsImmX, ARM_addsRegX);
      BACKEND_BINOP(ARM_faddReg, ARM_addsRegX);
    }else {
      BACKEND_BINOP_IMM(ARM_addImmX, ARM_addRegX);
      BACKEND_BINOP(ARM_faddReg, ARM_addRegX);
    }
		break;
	case IC_COMMA:
		next = ICArgN(rpn, 1);
		next2 = ICArgN(rpn, 0);
		orig_dst = next->res;
		if (!SpillsTmpRegs(next))
			PushTmp(cctrl, next2, NULL);
		else
			PushSpilledTmp(cctrl, next2);
		PushTmp(cctrl, next, &rpn->res);
		code_off = __OptPassFinal(cctrl, next2, bin, code_off);
		code_off = __OptPassFinal(cctrl, next, bin, code_off);
		PopTmp(cctrl, next);
		PopTmp(cctrl, next2);
		// TODO WHICH ONE
		code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
		break;
	case IC_EQ:
		next = ICArgN(rpn, 1);
		next2 = ICArgN(rpn, 0);
		orig_dst = next->res;
		if (!SpillsTmpRegs(next))
			PushTmp(cctrl, next2, NULL);
		else
			PushSpilledTmp(cctrl, next2);
		code_off = __OptPassFinal(cctrl, next2, bin, code_off);
		if (next->type == IC_TYPECAST) {
			TYPECAST_ASSIGN_BEGIN(next, next2);
			code_off = ICMov(cctrl, &next->res, &next2->res, bin, code_off);
			TYPECAST_ASSIGN_END(next);
		} else if (next->type == IC_DEREF) {
			code_off = __SexyWriteIntoDst(cctrl, next, &next2->res, bin, code_off);
		} else {
			PushTmp(cctrl, next, &rpn->res);
			code_off = __OptPassFinal(cctrl, next, bin, code_off);
			PopTmp(cctrl, next);
			code_off = ICMov(cctrl, &next->res, &next2->res, bin, code_off);
		}
		code_off = ICMov(cctrl, &rpn->res, &next2->res, bin, code_off);
		PopTmp(cctrl, next2);
		break;
	case IC_SUB:
		if(use_flags&&rpn->res.raw_type!=RT_F64) {
      rpn->res.set_flags=1;
      rpn->res.mode=MD_NULL;
      BACKEND_BINOP_IMM(ARM_subsImmX, ARM_subsRegX);
      BACKEND_BINOP(ARM_fsubReg, ARM_subsRegX);
    }else {
      BACKEND_BINOP_IMM(ARM_subImmX, ARM_subRegX);
      BACKEND_BINOP(ARM_fsubReg, ARM_subRegX);
    }
		break;
	case IC_DIV:
		next = rpn->base.next;
		next2 = ICArgN(rpn, 1);
		if (next->type == IC_I64 && next2->raw_type != RT_F64) {
			if (__builtin_popcountll(next->integer) == 1) {
				PushTmp(cctrl, next2, &rpn->res);
				code_off = __OptPassFinal(cctrl, next2, bin, code_off);
				code_off = PutICArgIntoReg(cctrl, &next2->res, next2->raw_type, 1, bin, code_off);
				PopTmp(cctrl, next2);
				switch (next2->raw_type) {
				case RT_I8i:
				case RT_I16i:
				case RT_I32i:
				case RT_I64i:
					if (rpn->res.mode == MD_REG) {
						AIWNIOS_ADD_CODE(ARM_asrImmX(rpn->res.reg, next2->res.reg, __builtin_ffsll(next->integer) - 1));
					} else {
						AIWNIOS_ADD_CODE(ARM_asrImmX(1, next2->res.reg, __builtin_ffsll(next->integer) - 1));
						tmp.raw_type = RT_I64i;
						tmp.reg = 1;
						tmp.mode = MD_REG;
						code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
					}
					break;
				default:
					if (rpn->res.mode == MD_REG) {
						AIWNIOS_ADD_CODE(ARM_lsrImmX(rpn->res.reg, next2->res.reg, __builtin_ffsll(next->integer) - 1));
					} else {
						AIWNIOS_ADD_CODE(ARM_lsrImmX(1, next2->res.reg, __builtin_ffsll(next->integer) - 1));
						tmp.raw_type = RT_I64i;
						tmp.reg = 1;
						tmp.mode = MD_REG;
						code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
					}
				}
				break;
			}
		}
    if(rpn->raw_type==RT_U64i) {
      BACKEND_BINOP(ARM_fdivReg, ARM_udivX);      
    } else {
      BACKEND_BINOP(ARM_fdivReg, ARM_sdivRegX);
    }
		break;
	case IC_MUL:
		next = rpn->base.next;
		next2 = ICArgN(rpn, 1);
		if (next->type == IC_I64 && next2->raw_type != RT_F64) {
			if (__builtin_popcountll(next->integer) == 1) {
				PushTmp(cctrl, next2, &rpn->res);
				code_off = __OptPassFinal(cctrl, next2, bin, code_off);
				code_off = PutICArgIntoReg(cctrl, &next2->res, next2->raw_type, 1, bin, code_off);
				PopTmp(cctrl, next2);
				if (rpn->res.mode == MD_REG && rpn->raw_type != RT_F64) {
					AIWNIOS_ADD_CODE(ARM_lslImmX(rpn->res.reg, next2->res.reg, __builtin_ffsll(next->integer) - 1));
				} else {
					AIWNIOS_ADD_CODE(ARM_lslImmX(1, next2->res.reg, __builtin_ffsll(next->integer) - 1));
					tmp.raw_type = RT_I64i;
					tmp.reg = 1;
					tmp.mode = MD_REG;
					code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
				}
				break;
			}
		}
    if(rpn->raw_type==RT_U64i) {
      BACKEND_BINOP(ARM_fmulReg, ARM_umullX);
    } else {
      BACKEND_BINOP(ARM_fmulReg, ARM_mulRegX);
    }
		break;
	case IC_DEREF:
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
			PushTmp(cctrl, next, &rpn->res);
			code_off = __OptPassFinal(cctrl, next, bin, code_off);
			PopTmp(cctrl, next);
			code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
			return code_off;
		}
		code_off = DerefToICArg(cctrl, &tmp, rpn, 2, bin, code_off);
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		break;
	case IC_AND:
#define BACKEND_BIT_BINOP(bit_op, shift)                                     \
	if (rpn->raw_type == RT_F64) {                                           \
		next = ICArgN(rpn, 1);                                               \
		next2 = ICArgN(rpn, 0);                                              \
		PushTmp(cctrl, next, &rpn->res);                                     \
		if (SpillsTmpRegs(next))                                             \
			PushSpilledTmp(cctrl, next2);                                    \
		else                                                                 \
			PushTmp(cctrl, next2, NULL);                                     \
		code_off = __OptPassFinal(cctrl, next2, bin, code_off);              \
		code_off = __OptPassFinal(cctrl, next, bin, code_off);               \
		code_off = FBitOp(cctrl, &rpn->res, &next->res, &next2->res, bit_op, \
			shift, bin, code_off);                                           \
	} else {                                                                 \
		BACKEND_BINOP(bit_op, bit_op);                                       \
	}
    if(use_flags&&rpn->res.raw_type!=RT_F64) {
      rpn->res.set_flags=1;
      rpn->res.mode=MD_NULL;
      BACKEND_BINOP_IMM(ARM_andsImmX,ARM_andsRegX);
      BACKEND_BIT_BINOP(ARM_andsRegX, 0);
		}else {
      BACKEND_BINOP_IMM(ARM_andImmX, ARM_andRegX);
      BACKEND_BIT_BINOP(ARM_andRegX, 0);
    }		
    break;
	case IC_ADDR_OF:
		next = rpn->base.next;
		switch (next->type) {
			break;
		case __IC_STATIC_REF:
			if (rpn->res.mode == MD_REG)
				into_reg = rpn->res.reg;
			else
				into_reg = 0;
			AIWNIOS_ADD_CODE(ARM_adrX(into_reg, (rpn->integer+cctrl->code_ctrl->statics_offset) - code_off));
			goto restore_reg;
			break;
		case IC_DEREF:
			next = rpn->base.next;
			PushTmp(cctrl, next, &rpn->res);
			code_off = __OptPassFinal(cctrl, next, bin, code_off);
			PopTmp(cctrl, next);
			code_off = PutICArgIntoReg(cctrl, &next->res, RT_PTR, 1, bin, code_off);
			code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
			break;
		case IC_BASE_PTR:
			if (rpn->res.mode == MD_REG)
				into_reg = rpn->res.reg;
			else
				into_reg = 0;
			AIWNIOS_ADD_CODE(ARM_addImmX(into_reg, ARM_REG_FP, next->integer));
			goto restore_reg;
			break;
		case IC_STATIC:
			if (rpn->res.mode == MD_REG)
				into_reg = rpn->res.reg;
			else
				into_reg = 0;
			if (cctrl->code_ctrl->final_pass >= 2) {
				i = ARM_adrX(into_reg, (char*)rpn->integer - (bin + code_off));
				if (i != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(i);
					goto restore_reg;
				}
			}
			code_off = __ICMoveI64(cctrl, into_reg, rpn->integer, bin,
				code_off);
			goto restore_reg;
		case IC_GLOBAL:
		get_glbl_ptr:
			if (rpn->res.mode == MD_REG)
				into_reg = rpn->res.reg;
			else
				into_reg = 0;
			if (next->global_var->base.type & HTT_GLBL_VAR) {
				enter_addr = next->global_var->data_addr;
			} else if (next->global_var->base.type & HTT_FUN) {
				enter_addr = ((CHashFun*)next->global_var)->fun_ptr;
			} else
				abort();
			//Undefined?
			if (!enter_addr) {
				misc = AddRelocMisc(cctrl, next->global_var->base.str);
				if (cctrl->code_ctrl->final_pass >= 2)
					AIWNIOS_ADD_CODE(ARM_ldrLabelX(into_reg, (char*)misc->addr - (bin + code_off)))
				else
					AIWNIOS_ADD_CODE(0);
				goto restore_reg;
			} else if (cctrl->code_ctrl->final_pass >= 2) {
				i = ARM_adrX(into_reg, (char*)enter_addr - (bin + code_off));
				if (i != ARM_ERR_INV_OFF) {
					AIWNIOS_ADD_CODE(i);
					goto restore_reg;
				}
			} else
				AIWNIOS_ADD_CODE(0);
			code_off = __ICMoveI64(cctrl, into_reg, enter_addr, bin,
				code_off);
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
	case IC_XOR:
		BACKEND_BINOP_IMM(ARM_eorImmX,ARM_eorRegX);
		BACKEND_BIT_BINOP(ARM_eorRegX, 0);
		break;
	case IC_MOD:
		code_off = __CompileMod(cctrl, rpn, bin, code_off);
		break;
	case IC_OR:
    BACKEND_BINOP_IMM(ARM_orrImmX,ARM_orrRegX);
		BACKEND_BIT_BINOP(ARM_orrRegX, 0);
		break;
	case IC_LT:
	case IC_GT:
	case IC_LE:
	case IC_GE:
		//
		// Ask nroot how this works if you want to know
		//
		// X/V3 is for lefthand side
		// X/V4 is for righthand side
		range = NULL;
		range_args = NULL;
		range_fail_addrs = NULL;
		range_cmp_types = NULL;
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
			range_args = A_MALLOC(sizeof(CRPN*) * (cnt + 1), cctrl->hc);
			range = A_MALLOC(sizeof(CRPN*) * cnt, cctrl->hc);
			range_fail_addrs = A_MALLOC(sizeof(CRPN*) * cnt, cctrl->hc);
			range_cmp_types = A_MALLOC(sizeof(int64_t) * cnt, cctrl->hc);
			goto get_range_items;
		}
		for (i = 0; i <= cnt; i++) {
			// We compile front[cnt] to back[0]
			for (i2 = i - 1; i2 >= 0; i2--) {
				if (SpillsTmpRegs(range_args[i2])) {
					PushSpilledTmp(cctrl, range_args[i]);
					goto rppass;
				}
			}
			PushTmp(cctrl, range_args[i], NULL);
		rppass:;
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
			code_off = PutICArgIntoReg(cctrl, &tmp, i2, 3, bin, code_off);
			code_off = PutICArgIntoReg(cctrl, &tmp2, i2, 4, bin, code_off);
			if (use_flt_cmp) {
				AIWNIOS_ADD_CODE(ARM_fcmp(tmp.reg, tmp2.reg));
			} else {
				AIWNIOS_ADD_CODE(ARM_cmpRegX(tmp.reg, tmp2.reg));
			}
			//
			// We use the opposite compare because if fail we jump to the fail
			// zone
			//
			if (use_flt_cmp) {
			signed_cmp:
				switch (range[i]->type) {
					break;
				case IC_LT:
					range_cmp_types[i] = ARM_GE;
					break;
				case IC_GT:
					range_cmp_types[i] = ARM_LE;
					break;
				case IC_LE:
					range_cmp_types[i] = ARM_GT;
					break;
				case IC_GE:
					range_cmp_types[i] = ARM_LT;
				}
			} else if (next->raw_type == RT_U64i || next2->raw_type == RT_U64i) {
				switch (range[i]->type) {
					break;
				case IC_LT:
					range_cmp_types[i] = ARM_HS;
					break;
				case IC_GT:
					range_cmp_types[i] = ARM_LS;
					break;
				case IC_LE:
					range_cmp_types[i] = ARM_HI;
					break;
				case IC_GE:
					range_cmp_types[i] = ARM_LO;
				}
			} else {
				goto signed_cmp;
			}
			range_fail_addrs[i] = bin + code_off;
			AIWNIOS_ADD_CODE(0); // WILL BE FILLED IN LATER
			next = next2;
		}
		tmp.mode = MD_I64;
		tmp.off = 1;
		tmp.raw_type = RT_I64i;
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		exit_addr = bin + code_off;
		AIWNIOS_ADD_CODE(ARM_b(0));
		if (bin)
			for (i = 0; i != cnt; i++) {
				*(int32_t*)range_fail_addrs[i] = ARM_bcc(
					range_cmp_types[i],
					(bin + code_off) - (char*)range_fail_addrs[i]); // TODO unsigned
			}
		tmp.mode = MD_I64;
		tmp.off = 0;
		tmp.raw_type = RT_I64i;
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		if (bin)
			*(int32_t*)exit_addr = ARM_b((bin + code_off) - exit_addr);
		for (i = cnt; i >= 0; i--)
			PopTmp(cctrl, range_args[i]);
		A_FREE(range);
		A_FREE(range_args);
		A_FREE(range_fail_addrs);
		A_FREE(range_cmp_types);
		break;
	case IC_LNOT:
		next = rpn->base.next;
		PushTmp(cctrl, next, &rpn->res);
		code_off = __OptPassFinal(cctrl, next, bin, code_off);
		PopTmp(cctrl, next);
		code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 1, bin, code_off);
		AIWNIOS_ADD_CODE(ARM_cmpImmX(next->res.reg, 0));
		if (rpn->res.mode == MD_REG) {
			into_reg = rpn->res.reg;
		} else
			into_reg = 0; // Store into reg 0
		AIWNIOS_ADD_CODE(ARM_csetX(into_reg, ARM_EQ));
		if (rpn->res.mode != MD_REG) {
			tmp.raw_type = rpn->raw_type;
			tmp.reg = 0;
			tmp.mode = MD_REG;
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		}
		break;
	case IC_BNOT:
		if (rpn->raw_type == RT_F64)
			abort(); // TODO
		BACKEND_UNOP(ARM_mvnRegX, ARM_mvnRegX);
		break;
	case IC_POST_INC:
		code_off = __SexyPostOp(cctrl, rpn, ARM_addImmX, ARM_addRegX,ARM_faddReg, bin, code_off);
		break;
	case IC_POST_DEC:
		code_off = __SexyPostOp(cctrl, rpn, ARM_subImmX, ARM_subRegX,ARM_fsubReg, bin, code_off);
		break;
	case IC_PRE_INC:
		code_off = __SexyPreOp(cctrl, rpn, ARM_addImmX, ARM_addRegX,ARM_faddReg, bin, code_off);
		break;
	case IC_PRE_DEC:
		code_off = __SexyPreOp(cctrl, rpn, ARM_subImmX, ARM_subRegX,ARM_fsubReg, bin, code_off);
		break;
	case IC_AND_AND:
#define BACKEND_BOOLIFY(to, reg, rt)                        \
	if ((rt) != RT_F64) {                                   \
		AIWNIOS_ADD_CODE(ARM_cmpImmX(reg, 0));              \
		AIWNIOS_ADD_CODE(ARM_csetX(to, ARM_NE));            \
	} else {                                                \
		code_off = __ICMoveF64(cctrl, 2, 0, bin, code_off); \
		AIWNIOS_ADD_CODE(ARM_fcmp(reg, 2));                 \
		AIWNIOS_ADD_CODE(ARM_csetX(to, ARM_NE));            \
	}
		if (!rpn->code_misc)
			rpn->code_misc = CodeMiscNew(cctrl, CMT_LABEL);
		a = ICArgN(rpn, 1);
		b = ICArgN(rpn, 0);
		PushTmp(cctrl, a, &rpn->res);
		code_off = __OptPassFinal(cctrl, a, bin, code_off);
		PopTmp(cctrl, a);
		if (a->res.raw_type != RT_F64) {
			code_off = PutICArgIntoReg(cctrl, &a->res, RT_I64i, 0, bin, code_off);
		} else {
			//
			// avoid messing up a's reg
			//
			BACKEND_BOOLIFY(0, a->res.reg, RT_F64);
			a->res.reg = 0;
      a->res.raw_type=RT_I64i;
		}
		code_off = ICMov(cctrl, &rpn->res, &a->res, bin, code_off);
		if (cctrl->code_ctrl->final_pass > 2) {
			AIWNIOS_ADD_CODE(ARM_cbzX(a->res.reg, (char*)rpn->code_misc->addr - (bin + code_off)));
		} else
			AIWNIOS_ADD_CODE(0);

		PushTmp(cctrl, b, &rpn->res);
		code_off = __OptPassFinal(cctrl, b, bin, code_off);
		PopTmp(cctrl, b);
		code_off = PutICArgIntoReg(cctrl, &b->res, b->raw_type, 0, bin, code_off);
		//
		// avoid messing up b's reg
		//
		BACKEND_BOOLIFY(0, b->res.reg, b->raw_type);
		b->res.reg = 0;
    b->res.raw_type=RT_I64i;
		code_off = ICMov(cctrl, &rpn->res, &b->res, bin, code_off);
		if (cctrl->code_ctrl->final_pass >= 2) {
			rpn->code_misc->addr = bin + code_off;
		}
		break;
	case IC_OR_OR:
		if (!rpn->code_misc)
			rpn->code_misc = CodeMiscNew(cctrl, CMT_LABEL);
		a = ICArgN(rpn, 1);
		b = ICArgN(rpn, 0);
		PushTmp(cctrl, a, &rpn->res);
		code_off = __OptPassFinal(cctrl, a, bin, code_off);
		PopTmp(cctrl, a);
		code_off = PutICArgIntoReg(cctrl, &a->res, a->raw_type, 0, bin, code_off);
		//
		//Avoid messing up the value of a's register(use 0 instead)
		//
		BACKEND_BOOLIFY(0, a->res.reg, a->raw_type);
		a->res.reg = 0;
    a->res.raw_type=RT_I64i;
		code_off = ICMov(cctrl, &rpn->res, &a->res, bin, code_off);
		if (cctrl->code_ctrl->final_pass > 2) {
			AIWNIOS_ADD_CODE(ARM_cbnzX(a->res.reg, (char*)rpn->code_misc->addr - (bin + code_off)));
		} else
			AIWNIOS_ADD_CODE(0);

		PushTmp(cctrl, b, &rpn->res);
		code_off = __OptPassFinal(cctrl, b, bin, code_off);
		PopTmp(cctrl, b);
		code_off = PutICArgIntoReg(cctrl, &b->res, b->raw_type, 0, bin, code_off);
		//
		//Avoid messing up the value of a's register(use 0 instead)
		//
		BACKEND_BOOLIFY(0, b->res.reg, b->raw_type);
		b->res.reg = 0;
    b->res.raw_type=RT_I64i;
		code_off = ICMov(cctrl, &rpn->res, &b->res, bin, code_off);
		if (cctrl->code_ctrl->final_pass >= 2) {
			rpn->code_misc->addr = bin + code_off;
		}
		break;
	case IC_XOR_XOR:
#define BACKEND_LOGICAL_BINOP(op)                                                    \
	next = ICArgN(rpn, 1);                                                           \
	next2 = ICArgN(rpn, 0);                                                          \
	if (!SpillsTmpRegs(next2))                                                       \
		PushTmp(cctrl, next, NULL);                                                  \
	else                                                                             \
		PushSpilledTmp(cctrl, next);                                                 \
	PushTmp(cctrl, next2, &rpn->res);                                                \
	code_off = __OptPassFinal(cctrl, next, bin, code_off);                           \
	code_off = __OptPassFinal(cctrl, next2, bin, code_off);                          \
	PopTmp(cctrl, next2);                                                            \
	PopTmp(cctrl, next);                                                             \
	if (rpn->res.mode == MD_REG) {                                                   \
		into_reg = rpn->res.reg;                                                     \
	} else                                                                           \
		into_reg = 0;                                                                \
	code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 2, bin, code_off);  \
	code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type, 1, bin, code_off); \
	BACKEND_BOOLIFY(2, next->res.reg, next->res.raw_type);                           \
	BACKEND_BOOLIFY(1, next2->res.reg, next2->res.raw_type);                         \
	AIWNIOS_ADD_CODE(op(into_reg, 1, 2));                                            \
	if (rpn->res.mode != MD_REG) {                                                   \
		tmp.raw_type = rpn->raw_type;                                                \
		tmp.reg = 0;                                                                 \
		tmp.mode = MD_REG;                                                           \
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                     \
	}
		BACKEND_LOGICAL_BINOP(ARM_eorRegX);
		break;
	case IC_EQ_EQ:
#define BACKEND_CMP                                                                  \
	next = ICArgN(rpn, 1);                                                           \
	next2 = ICArgN(rpn, 0);                                                          \
	if (!SpillsTmpRegs(next2))                                                       \
		PushTmp(cctrl, next, NULL);                                                  \
	else                                                                             \
		PushSpilledTmp(cctrl, next);                                                 \
	PushTmp(cctrl, next2, &rpn->res);                                                \
	code_off = __OptPassFinal(cctrl, next, bin, code_off);                           \
	code_off = __OptPassFinal(cctrl, next2, bin, code_off);                          \
	PopTmp(cctrl, next2);                                                            \
	PopTmp(cctrl, next);                                                             \
	code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 2, bin, code_off);  \
	code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type, 1, bin, code_off); \
	if (rpn->raw_type == RT_F64) {                                                   \
		AIWNIOS_ADD_CODE(ARM_fcmp(next->res.reg, next2->res.reg));                   \
	} else {                                                                         \
		AIWNIOS_ADD_CODE(ARM_cmpRegX(next->res.reg, next2->res.reg));                \
	}
		BACKEND_CMP;
		if(use_flags) return code_off;
    if (rpn->res.mode == MD_REG) {
			into_reg = rpn->res.reg;
		} else
			into_reg = 0;
		AIWNIOS_ADD_CODE(ARM_csetX(into_reg, ARM_EQ));
		if (rpn->res.mode != MD_REG) {
			tmp.raw_type = rpn->raw_type;
			tmp.reg = 0;
			tmp.mode = MD_REG;
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		}
		break;
	case IC_NE:
		BACKEND_CMP;
    if(use_flags) return code_off;
		if (rpn->res.mode == MD_REG) {
			into_reg = rpn->res.reg;
		} else
			into_reg = 0;
		AIWNIOS_ADD_CODE(ARM_csetX(into_reg, ARM_NE));
		if (rpn->res.mode != MD_REG) {
			tmp.raw_type = rpn->raw_type;
			tmp.reg = 0;
			tmp.mode = MD_REG;
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		}
		break;
	case IC_LSH:
		BACKEND_BIT_BINOP(ARM_lslvRegX, 1);
		break;
	case IC_RSH:
		switch (rpn->raw_type) {
		case RT_U8i:
		case RT_U16i:
		case RT_U32i:
		case RT_U64i:
			BACKEND_BINOP(ARM_lsrvRegX, ARM_lsrvRegX);
			break;
		default:
			BACKEND_BIT_BINOP(ARM_asrvRegX, 1);
		}
		break;
	case __IC_CALL:
	case IC_CALL:
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
		code_off = __ICFCall(cctrl, rpn, bin, code_off);
		cctrl->backend_user_data2 = i;
		cctrl->backend_user_data3 = i2;
		break;
	case __IC_VARGS:
		//  ^^^^^^^^
		// (        \      ___________________
		// ( (*) (*)|     /                   \
    //  \   ^   |    |__IFCall            |
		//   \  <>  | <==| Will unwind for you|
		//    \    /      \__________________/
		//      \_/
		if (ARM_ERR_INV_OFF != ARM_subImmX(ARM_REG_SP, ARM_REG_SP, 8 * rpn->length)) {
			//Align to 16
			if (rpn->length & 1)
				AIWNIOS_ADD_CODE(ARM_subImmX(ARM_REG_SP, ARM_REG_SP, 8 + 8 * rpn->length))
			else
				AIWNIOS_ADD_CODE(ARM_subImmX(ARM_REG_SP, ARM_REG_SP, 8 * rpn->length))
		} else {
			if (rpn->length & 1) //Align to 16
				code_off = __ICMoveI64(cctrl, 0, 8 + 8 * rpn->length, bin, code_off);
			else
				code_off = __ICMoveI64(cctrl, 0, 8 * rpn->length, bin, code_off);
			AIWNIOS_ADD_CODE(ARM_subRegX(ARM_REG_SP, ARM_REG_SP, 0));
		}
		for (i = 0; i != rpn->length; i++) {
			next = ICArgN(rpn, rpn->length - i - 1);
			PushTmp(cctrl, next, &rpn->res);
			code_off = __OptPassFinal(cctrl, next, bin, code_off);
			PopTmp(cctrl, next);
			tmp.reg = ARM_REG_SP;
			tmp.off = 8 * i;
			tmp.mode = MD_INDIR_REG;
			tmp.raw_type = next->raw_type;
			if (tmp.raw_type < RT_I64i)
				tmp.raw_type = RT_I64i;
			code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);
		}
		tmp.reg = ARM_REG_SP;
		tmp.mode = MD_REG;
		tmp.raw_type = RT_I64i;
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		break;
	case IC_ADD_EQ:
		code_off = __SexyAssignOp(cctrl, rpn, &ARM_addRegX, ARM_addImmX,&ARM_faddReg, bin, code_off);
		break;
	case IC_SUB_EQ:
		code_off = __SexyAssignOp(cctrl, rpn, &ARM_subRegX, ARM_subImmX,&ARM_fsubReg, bin, code_off);
		break;
	case IC_MUL_EQ:
    if(ICArgN(rpn,1)->raw_type==RT_U64i) {
      code_off=__SexyAssignOp(cctrl, rpn, &ARM_umullX, NULL,&ARM_fmulReg, bin, code_off);;
    } else
      code_off = __SexyAssignOp(cctrl, rpn, &ARM_mulRegX, NULL,&ARM_fmulReg, bin, code_off);
		break;
	case IC_DIV_EQ:
    if(ICArgN(rpn,1)->raw_type==RT_U64i) {
      code_off=__SexyAssignOp(cctrl, rpn, &ARM_udivX, NULL,&ARM_fdivReg, bin, code_off);;
    } else
      code_off = __SexyAssignOp(cctrl, rpn, &ARM_sdivRegX,NULL, &ARM_fdivReg, bin, code_off);
		break;
	case IC_LSH_EQ:
		if (rpn->raw_type == RT_F64)
			abort();
		code_off = __SexyAssignOp(cctrl, rpn, &ARM_lslvRegX,ARM_lslImmX, &ARM_lslvRegX, bin, code_off);
		break;
	case IC_RSH_EQ:
		if (rpn->raw_type == RT_F64)
			abort();
		switch (rpn->raw_type) {
		case RT_U8i:
		case RT_U16i:
		case RT_U32i:
		case RT_U64i:
			code_off = __SexyAssignOp(cctrl, rpn, &ARM_lsrvRegX,ARM_lsrImmX, &ARM_lsrvRegX, bin,
				code_off);
			break;
		default:
			code_off = __SexyAssignOp(cctrl, rpn, &ARM_asrvRegX,ARM_asrImmX, &ARM_asrvRegX, bin,
				code_off);
		}
		break;
	case IC_AND_EQ:
		if (rpn->raw_type == RT_F64)
			abort(); // TODO
		code_off = __SexyAssignOp(cctrl, rpn, &ARM_andRegX, ARM_andImmX,&ARM_andRegX, bin, code_off);
		break;
	case IC_OR_EQ:
		if (rpn->raw_type == RT_F64)
			abort(); // TODO
		code_off = __SexyAssignOp(cctrl, rpn, &ARM_orrRegX, ARM_orrImmX,&ARM_orrRegX, bin, code_off);
		break;
	case IC_XOR_EQ:
		if (rpn->raw_type == RT_F64)
			abort(); // TODO
		code_off = __SexyAssignOp(cctrl, rpn, &ARM_eorRegX, ARM_eorImmX,&ARM_eorRegX, bin, code_off);
		break;
	case IC_ARROW:
		abort();
		break;
	case IC_DOT:
		abort();
		break;
	case IC_MOD_EQ:
		// Handles IC_MOD_EQ too
		code_off = __CompileMod(cctrl, rpn, bin, code_off);
		break;
	case IC_I64:
		rpn->res.mode = MD_I64;
		rpn->res.raw_type = RT_I64i;
		rpn->res.integer = rpn->integer;
		break;
	case IC_F64:
		rpn->res.mode = MD_F64;
		rpn->res.raw_type = RT_F64;
		rpn->res.flt = rpn->flt;
		break;
	case IC_ARRAY_ACC:
		abort();
		break;
	case IC_RET:
		next = ICArgN(rpn, 0);
		PushTmp(cctrl, next, NULL);
		code_off = __OptPassFinal(cctrl, next, bin, code_off);
		PopTmp(cctrl, next);
		if (cctrl->cur_fun)
			tmp.raw_type = cctrl->cur_fun->return_class->raw_type;
		else if (rpn->ic_class) {
			//No return type so just return what we have
			tmp.raw_type = rpn->ic_class->raw_type;
		} else
			//No return type so just return what we have
			tmp.raw_type = next->raw_type;
		tmp.reg = 0; // 0 is return register
		tmp.mode = MD_REG;
		code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);
		// TempleOS will always store F64 result in RAX(integer register)
		// Let's merge the two togheter
		if (tmp.raw_type == RT_F64) {
			AIWNIOS_ADD_CODE(ARM_fmovI64F64(0, 0));
		} else {
			//Vise versa
			AIWNIOS_ADD_CODE(ARM_fmovF64I64(0, 0));
		}
		// TODO  jump to return area,not generate epilog for each poo poo
		if (cctrl->code_ctrl->final_pass >= 2) {
			AIWNIOS_ADD_CODE(ARM_b(cctrl->epilog_offset - code_off));
		} else
			AIWNIOS_ADD_CODE(ARM_b(0));
		break;
	case IC_BASE_PTR:
		break;
	case IC_STATIC:
		tmp.raw_type = rpn->raw_type;
		tmp.off = rpn->integer;
		tmp.mode = MD_PTR;
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		break;
	case IC_IREG:
		tmp.raw_type = RT_I64i;
		tmp.reg = rpn->integer;
		tmp.mode = MD_REG;
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		break;
	case IC_FREG:
		tmp.raw_type = RT_F64;
		tmp.reg = rpn->integer;
		tmp.mode = MD_REG;
		code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		break;
	case IC_LOAD:
		abort();
		break;
	case IC_STORE:
		abort();
		break;
	case __IC_STATIC_REF:
		if (cctrl->code_ctrl->final_pass >= 2) {
			tmp.raw_type = rpn->raw_type;
			tmp.off = rpn->integer;
			tmp.mode = MD_STATIC;
			code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
		} else
			AIWNIOS_ADD_CODE(0);
		break;
	case __IC_SET_STATIC_DATA:
		//Final pass
		if (cctrl->code_ctrl->final_pass == 3) {
			memcpy(
				bin + cctrl->code_ctrl->statics_offset + rpn->code_misc->integer,
				rpn->code_misc->str,
				rpn->code_misc->str_len);
		}
	}
	return code_off;
}

// This dude calls __OptPassFinal 3 times.
// 1. Get size of WORST CASE compiled body,and generate any extra needed
// CCodeMisc's
//    This pass  will also asign CRPN->res with tmp registers/FRAME offsets
// 2. Fill in function body,accounting for not worst case jumps
// 3. Fill in the poo poo's
//
char* OptPassFinal(CCmpCtrl* cctrl, int64_t* res_sz, char** dbg_info)
{
	int64_t code_off, run, idx, cnt = 0, cnt2,final_size,is_terminal;
	int64_t min_ln = 0, max_ln = 0, statics_sz = 0;
	char* bin = NULL;
  char *ptr;
	CCodeMisc* misc;
	CHashImport* import;
	CRPN* r;
	for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code; r = r->base.next) {
		if (r->ic_line) {
			if (!min_ln)
				min_ln = r->ic_line;
			min_ln = (min_ln < r->ic_line) ? min_ln : r->ic_line;
			if (!max_ln)
				max_ln = r->ic_line;
			max_ln = (max_ln > r->ic_line) ? max_ln : r->ic_line;
		}
		//Move all frame offsets over by 16(for FP/LR) area on the frame
		if (r->type == IC_BASE_PTR) {
			r->integer += 16;
		} else if (r->type == __IC_STATICS_SIZE)
			statics_sz = r->integer;
	}
	for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code; r = ICFwd(r)) {
		cnt++;
	}
	if (dbg_info) {
    //Dont allocate on cctrl->hc heap ctrl as we want to share our datqa
		cctrl->code_ctrl->dbg_info = dbg_info;
		cctrl->code_ctrl->min_ln = min_ln;
	}
	CRPN* forwards[cnt2 = cnt];
	cnt = 0;
	for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code; r = ICFwd(r)) {
		forwards[cnt2 - ++cnt] = r;
	}
	// We DONT reset the wiggle room
	cctrl->backend_user_data0 = 0;
	//0 get maximum code size,
	//1 misc addrs
	//2 fill in label addresses
	//3 final in case anything changed
	for (run = 0; run != 4; run++) {
		cctrl->code_ctrl->final_pass = run;
		cctrl->backend_user_data1 = 0;
		cctrl->backend_user_data2 = 0;
		cctrl->backend_user_data3 = 0;
		if (run == 0) {
			code_off = 0;
			bin = NULL;
		} else if (run == 1) {
      //Dont allocate on cctrl->hc heap ctrl as we want to share our data
			bin = A_MALLOC(code_off, NULL);
			code_off = 0;
		} else {
			code_off = 0;
		}
		code_off = FuncProlog(cctrl, bin, code_off);
		for (cnt = 0; cnt < cnt2; cnt++) {
enter:
      r=forwards[cnt];
			code_off = __OptPassFinal(cctrl, r, bin, code_off);
      if(IsTerminalInst(r)) {
        cnt++;
        for(;cnt<cnt2;cnt++) {
          if(forwards[cnt]->type==IC_LABEL) 
            goto enter;
        }
      }
    }
		cctrl->epilog_offset = code_off;
		code_off = FuncEpilog(cctrl, bin, code_off);
		for (misc = cctrl->code_ctrl->code_misc->next;
			 misc != cctrl->code_ctrl->code_misc; misc = misc->base.next) {
			switch (misc->type) {
				break;
			case CMT_INT_CONST:
				if (code_off % 8)
					code_off += 8 - code_off % 8;
				misc->addr = bin + code_off;
				if (bin)
					*(int64_t*)(bin + code_off) = misc->integer;
				code_off += 8;
				break;
			case CMT_FLOAT_CONST:
				if (code_off % 8)
					code_off += 8 - code_off % 8;
				misc->addr = bin + code_off;
				if (bin)
					*(double*)(bin + code_off) = misc->flt;
				code_off += 8;
				break;
			case CMT_JMP_TAB:
				if (code_off % 8)
					code_off += 8 - code_off % 8;
				misc->addr = bin + code_off;
				if (bin) {
					// LAMA snail
					//
					//  ()     ()
					//   \\   //
					//   (O   O)      ()
					//(===____ )     (  )
					//   (\__/ )     / /
					//   (     )____/ /
					//   \           /
					//    \_________/
					// Here's the deal,I am doing position indepent code,so the offsets are
					// measured from the start of the function,these offsets are relative,not fixed
					// (this way we can write the jump table to a file and load it without problems)
					//
					for (idx = 0; idx <= misc->hi - misc->lo; idx++)
						*(void**)(bin + code_off + idx * 8) = (char*)misc->jmp_tab[idx]->addr - (char*)bin;
				}
				code_off += (misc->hi - misc->lo + 1) * 8;
				break;
			case CMT_LABEL:
				break;
			case CMT_SHORT_ADDR:
				if (run == 3 && misc->patch_addr) {
					*misc->patch_addr = misc->addr;
				}
				break;
			case CMT_RELOC_U64:
        //No need to make room for an unued misc
        if(run>=2&&!misc->use_cnt) {
          *misc->patch_addr=INVALID_PTR;
          break;
        }
				if (code_off % 8)
					code_off += 8 - code_off % 8;
				misc->addr = bin + code_off;
				if (bin)
					*(int64_t*)(bin + code_off) = 0xB00B1E5;
				code_off += 8;
				//
				// 3 is the final run,we add the relocation to the hash table
				//
				if (run == 3) {
					import = A_CALLOC(sizeof(CHashImport), NULL);
					import->base.type = HTT_IMPORT_SYS_SYM;
					import->base.str = A_STRDUP(misc->str, NULL);
					import->address = misc->addr;
					HashAdd(import, Fs->hash_table);
				}
				if (run == 3 && misc->patch_addr) {
					*misc->patch_addr = misc->addr;
				}
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
					if (run == 3) {
						//See IC_STR: We "steal" the ->str because it's already on the heap.
						// IC_STR steals the ->str point so we dont want to Free it
						misc->str = NULL;
					}
				}
			}
		}
    if(run<3) {
      if (code_off % 8) //Align to 8
        code_off += 8 - code_off % 8;
      cctrl->code_ctrl->statics_offset = code_off;
      if(statics_sz)
        code_off += statics_sz+8;
      if(run==2)
        final_size=code_off;
    } 
	}
	__builtin___clear_cache(bin, bin + MSize(bin));
  if(dbg_info) {
    cnt=MSize(dbg_info)/8;
    ptr=dbg_info[0]=bin;
    for(idx=1;idx<cnt;idx++) {
      if(!dbg_info[idx]) {
        dbg_info[idx]=NULL;
        continue;
      }
      if(ptr>dbg_info[idx]) {
        //No backwards jumps
        dbg_info[idx]=NULL;
        continue;
      }
      ptr=dbg_info[idx];
    }
  }
  if (res_sz)
      *res_sz = final_size;
	return bin;
}
