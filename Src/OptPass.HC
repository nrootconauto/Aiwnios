U0 IsConst(CRPN *r) {
  return r->type==IC_IMM_I64||r->type==IC_IMM_F64;
}
F64 ConstF64(CRPN *r) {
  if(r->type==IC_IMM_F64)
    return r->imm_f64;
  return r->imm_i64;
}
I64 ConstI64(CRPN *r) {
  if(r->type==IC_IMM_F64)
    return r->imm_f64;
  return r->imm_i64;
}
U0 OptPassConstFold(CCmpCtrl *cc) {
  CRPN *rpn,*a,*b,*next;
  Bool changed=TRUE;
  while(changed) {
    for(rpn=cc->coc.coc_head.next;rpn!=&cc->coc.coc_head;rpn=next) {
      next=rpn->next;
      if(rpn->ic_class->raw_type==RT_F64) {
        switch(rpn->type) {
        break;case IC_HOLYC_TYPECAST:
        a=rpn->next;
        if(IsConst(a)) {
          if(a->type==IC_IMM_I64) {
            rpn->type=IC_IMM_F64;
            rpn->imm_i64=a->imm_i64; //These two are unioned
          } else if(a->type==IC_IMM_F64) {
            rpn->type=IC_IMM_F64;
            rpn->imm_f64=a->imm_f64;
          }
          changed=TRUE;
          RPNDel(a);
          next=rpn->next;
        }
        break;case IC_NOT:
        a=rpn->next;
        if(IsConst(a)) {
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=~ConstF64(a);
          changed=TRUE;
          RPNDel(a);
          next=rpn->next;
        }
        break;case IC_UNARY_MINUS:
        a=rpn->next;
        if(IsConst(a)) {
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=-ConstF64(a);
          changed=TRUE;
          RPNDel(a);
          next=rpn->next;
        }
        break;case IC_SHL:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          RPNDel(a);
          RPNDel(b);
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)<<ConstI64(b);
          next=rpn->next;
        }
        break;case IC_SHR:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)>>ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_POWER:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)`ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_MUL:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)*ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_DIV:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)/ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_MOD:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)%ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_AND:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)&ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_OR:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)|ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_XOR:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)^ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_ADD:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)+ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_SUB:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_f64=ConstF64(a)-ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        /* THESE ARE SPECIAL
        break;case IC_EQU_EQU:
        break;case IC_NOT_EQU:
        break;case IC_LESS:
        break;case IC_GREATER_EQU:
        break;case IC_GREATER:
        break;case IC_LESS_EQU:
        break;case IC_AND_AND:
        break;case IC_OR_OR:
        break;case IC_XOR_XOR:
        */
        }
      } else {
        switch(rpn->type) {
        break;case IC_HOLYC_TYPECAST:
        a=rpn->next;
        if(IsConst(a)) {
          if(a->type==IC_IMM_I64) {
            rpn->type=IC_IMM_I64;
            rpn->imm_i64=a->imm_i64;
          } else if(a->type==IC_IMM_F64) {
            rpn->type=IC_IMM_I64;
            rpn->imm_i64=a->imm_i64;  //These two are unioned
          }
          changed=TRUE;
          RPNDel(a);
          next=rpn->next;
        }
        break;case IC_NOT:
        a=rpn->next;
        if(IsConst(a)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=~ConstI64(a);
          RPNDel(a);
          next=rpn->next;
        }
        break;case IC_UNARY_MINUS:
        a=rpn->next;
        if(IsConst(a)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=-ConstI64(a);
          RPNDel(a);
          next=rpn->next;
        }
        break;case IC_SHL:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)+ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_SHR:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)-ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_POWER:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_F64;
          rpn->imm_i64=ConstF64(a)`ConstF64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_MUL:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)*ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_DIV:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)/ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_MOD:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)%ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_AND:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)&ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_OR:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)|ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_XOR:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)^ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_ADD:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)+ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        break;case IC_SUB:
        b=rpn->next;
        a=rpn->next->next;
        if(IsConst(a)&&IsConst(b)) {
          changed=TRUE;
          rpn->type=IC_IMM_I64;
          rpn->imm_i64=ConstI64(a)-ConstI64(b);
          RPNDel(a);
          RPNDel(b);
          next=rpn->next;
        }
        /* THESE ARE SPECIAL
        break;case IC_EQU_EQU:
        break;case IC_NOT_EQU:
        break;case IC_LESS:
        break;case IC_GREATER_EQU:
        break;case IC_GREATER:
        break;case IC_LESS_EQU:
        break;case IC_AND_AND:
        break;case IC_OR_OR:
        break;case IC_XOR_XOR:
        */
        }
      }
    }
  }
}
#ifdef TARGET_AARCH64
#define AIWNIOS_IREG_START 19
#define AIWNIOS_IREG_CNT (28 - 19 + 1)
#define AIWNIOS_REG_FP ARM_REG_FP
#define AIWNIOS_REG_SP ARM_REG_SP
#define AIWNIOS_TMP_IREG_POOP 18 //Platform register,I use it as a poo poo
#define AIWNIOS_TMP_IREG_POOP2 17 //I use it as a second poo poo ALWAYS
#define AIWNIOS_TMP_IREG_START 8
#define AIWNIOS_TMP_IREG_CNT (16 - 8 + 1)
#define AIWNIOS_FREG_START 8
#define AIWNIOS_FREG_CNT (15 - 8 + 1)
#define AIWNIOS_TMP_FREG_START 16
#define AIWNIOS_TMP_FREG_CNT (31 - 16 + 1)
#endif

CRPN *CompSetFlags(CCmpCtrl *cc,CRPN *r) {
  CRPN *ret,*arg;
  switch(r->type) {
    break;case IC_END:
    break;case IC_NOP1:
    break;case IC_END_EXP:
    break;case IC_NOP2:
    break;case IC_LABEL:
    break;case IC_CALL_START: //NOT used by AIWNIOS
    break;case IC_CALL_END: //NOT used by AIWNIOS
    break;case IC_CALL_END2: //NOT used by AIWNIOS
    break;case IC_RETURN_VAL: //NOT used by AIWNIOS
    break;case IC_RETURN_VAL2: //NOT used by AIWNIOS
    break;case IC_IMM_I64:
    break;case IC_IMM_F64:
    break;case IC_STR_CONST:
    break;case IC_ABS_ADDR:
    break;case IC_ADDR_IMPORT:
    break;case IC_HEAP_GLBL: //???
    break;case IC_SIZEOF: //???
    break;case IC_GET_LABEL:
    break;case IC_REG:
    break;case IC_MOV:
    break;case IC_TO_I64:
iunop:
    ret=CompSetFlags(cc,arg=r->next);
    arg->flags&=~ICF_RES_NOT_USED;
    arg->flags|=ICF_RES_TO_INT;
    r->flags|=(r->next->flags&ICF_FUNC_CALL); //inherit dirtyness
    break;case IC_TO_F64:
    ret=CompSetFlags(cc,r->next);
    r->next->flags&=~ICF_RES_NOT_USED;
    r->next->flags|=ICF_RES_TO_F64;
    r->flags|=(r->next->flags&ICF_FUNC_CALL); //inherit dirtyness
    break;case IC_TO_BOOL:
    goto iunop;
    break;case IC_TOUPPER: //TODO???
    break;case IC_HOLYC_TYPECAST:
    ret=CompSetFlags(cc,r->next);
    r->next->flags&=~ICF_RES_NOT_USED;
    r->flags|=(r->next->flags&ICF_FUNC_CALL); //inherit dirtyness
    break;case IC_ADDR:
    goto iunop;
    break;case IC_COM:
binop:
    ret=CompSetFlags(cc,r->next);
    r->next->flags&=~ICF_RES_NOT_USED;
    r->flags|=(r->next->flags&ICF_FUNC_CALL); //inherit dirtyness
    ret=CompSetFlags(cc,ret);
    r->next->flags&=~ICF_RES_NOT_USED;
    r->flags|=(r->next->flags&ICF_FUNC_CALL); //inherit dirtyness
    break;case IC_NOT:
unop:
    ret=CompSetFlags(cc,r->next);
    r->next->flags&=~ICF_RES_NOT_USED;
    r->flags|=(r->next->flags&ICF_FUNC_CALL); //inherit dirtyness
    break;case IC_UNARY_MINUS: goto unop;
    break;case IC_DEREF:
    goto iunop;
    break;case IC_DEREF_PP:
    goto iunop;
    break;case IC_DEREF_MM:
    goto iunop;
    break;case IC__PP:
    goto unop;
    break;case IC__MM:
    goto unop;
    break;case IC_PP_:
    goto unop;
    break;case IC_MM_:
    goto unop;
    break;case IC_SHL:
shbinop:
    ret=CompSetFlags(cc,r->next);
    r->next->flags&=~ICF_RES_NOT_USED;
    r->flags|=(r->next->flags&ICF_FUNC_CALL); //inherit dirtyness
    ret=CompSetFlags(cc,ret);
    ret->next->flags&=~ICF_RES_NOT_USED;
    ret->next->flags|=ICF_RES_TO_I64;
    ret->flags|=(r->next->flags&ICF_FUNC_CALL); //inherit dirtyness
    break;case IC_SHR:
    break;case IC_SHL_CONST:
    break;case IC_SHR_CONST:
    break;case IC_POWER:
    break;case IC_MUL:
    break;case IC_DIV:
    break;case IC_MOD:
    break;case IC_AND:
    break;case IC_OR:
    break;case IC_XOR:
    break;case IC_ADD:
    break;case IC_SUB:
    break;case IC_EQU_EQU:
    break;case IC_NOT_EQU:
    break;case IC_LESS:
    break;case IC_GREATER_EQU:
    break;case IC_GREATER:
    break;case IC_LESS_EQU:
    break;case IC_PUSH_CMP:
    break;case IC_AND_AND:
    break;case IC_OR_OR:
    break;case IC_XOR_XOR:
    break;case IC_ASSIGN:
    break;case IC_ASSIGN_PP:
    break;case IC_ASSIGN_MM:
    break;case IC_SHL_EQU:
    break;case IC_SHR_EQU:
    break;case IC_MUL_EQU:
    break;case IC_DIV_EQU:
    break;case IC_MOD_EQU:
    break;case IC_AND_EQU:
    break;case IC_OR_EQU:
    break;case IC_XOR_EQU:
    break;case IC_ADD_EQU:
    break;case IC_SUB_EQU:
    break;case IC_JMP:
    break;case IC_SUB_CALL:
    break;case IC_SWITCH:
    break;case IC_NOBOUND_SWITCH:
    break;case IC_ADD_RSP:
    break;case IC_ADD_RSP1:
    break;case IC_ENTER:
    break;case IC_PUSH_REGS:
    break;case IC_POP_REGS:
    break;case IC_LEAVE:
    break;case IC_RET:
    break;case IC_CALL:
    break;case IC_CALL_INDIRECT:
    break;case IC_CALL_INDIRECT2:
    break;case IC_CALL_IMPORT:
    break;case IC_CALL_EXTERN:
  }
  return ret;
}

U0 FuncCall(CRPN *r) {
}
I64 CompileNode(CCmpCtrl *cc,CRPN *r,U8 *bin,I64 bin_sz) {
  switch(r->type) {
  break;case IC_END:
    return bin_sz;
  break;case IC_NOP1:
next:
    return CompileNode(cc,r->next,bin,bin_sz);
  break;case IC_END_EXP:
    goto next;
  break;case IC_NOP2:
  goto next;
  break;case IC_LABEL:
    if(bin)
      r->addr=bin+bin_sz;
    goto next;
  break;case IC_CALL_START: //NOT used by AIWNIOS
  break;case IC_CALL_END: //NOT used by AIWNIOS
  break;case IC_CALL_END2: //NOT used by AIWNIOS
  break;case IC_RETURN_VAL: //NOT used by AIWNIOS
  break;case IC_RETURN_VAL2: //NOT used by AIWNIOS
  break;case IC_IMM_I64:
  break;case IC_IMM_F64:
  break;case IC_STR_CONST:
  break;case IC_ABS_ADDR:
  break;case IC_ADDR_IMPORT:
  break;case IC_HEAP_GLBL:
  break;case IC_SIZEOF:
  break;case IC_GET_LABEL:
  break;case IC_REG:
  break;case IC_MOV:
  break;case IC_TO_I64:
  break;case IC_TO_F64:
  break;case IC_TO_BOOL:
  break;case IC_TOUPPER:
  break;case IC_HOLYC_TYPECAST:
  break;case IC_ADDR:
  break;case IC_COM:
  break;case IC_NOT:
  break;case IC_UNARY_MINUS:
  break;case IC_DEREF:
  break;case IC_DEREF_PP:
  break;case IC_DEREF_MM:
  break;case IC__PP:
  break;case IC__MM:
  break;case IC_PP_:
  break;case IC_MM_:
  break;case IC_SHL:
  break;case IC_SHR:
  break;case IC_SHL_CONST:
  break;case IC_SHR_CONST:
  break;case IC_POWER:
  break;case IC_MUL:
  break;case IC_DIV:
  break;case IC_MOD:
  break;case IC_AND:
  break;case IC_OR:
  break;case IC_XOR:
  break;case IC_ADD:
  break;case IC_SUB:
  break;case IC_EQU_EQU:
  break;case IC_NOT_EQU:
  break;case IC_LESS:
  break;case IC_GREATER_EQU:
  break;case IC_GREATER:
  break;case IC_LESS_EQU:
  break;case IC_PUSH_CMP:
  break;case IC_AND_AND:
  break;case IC_OR_OR:
  break;case IC_XOR_XOR:
  break;case IC_ASSIGN:
  break;case IC_ASSIGN_PP:
  break;case IC_ASSIGN_MM:
  break;case IC_SHL_EQU:
  break;case IC_SHR_EQU:
  break;case IC_MUL_EQU:
  break;case IC_DIV_EQU:
  break;case IC_MOD_EQU:
  break;case IC_AND_EQU:
  break;case IC_OR_EQU:
  break;case IC_XOR_EQU:
  break;case IC_ADD_EQU:
  break;case IC_SUB_EQU:
  break;case IC_JMP:
  break;case IC_SUB_CALL:
  break;case IC_SWITCH:
  break;case IC_NOBOUND_SWITCH:
  break;case IC_ADD_RSP:
  break;case IC_ADD_RSP1:
  break;case IC_ENTER:
  break;case IC_PUSH_REGS:
  break;case IC_POP_REGS:
  break;case IC_LEAVE:
  break;case IC_RET:
  break;case IC_CALL:
  break;case IC_CALL_INDIRECT:
  break;case IC_CALL_INDIRECT2:
  break;case IC_CALL_IMPORT:
  break;case IC_CALL_EXTERN:
}
