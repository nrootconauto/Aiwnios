extern class CBitSet;
class CRPN:CQue {
  I64 type,ic_line,ic_flags;
  CHashClass *ic_class;
  CHashFun *ic_fun;
  CArrayDim *ic_dim;  
  CICTreeLinks t;
  union {
    I64 length;
    I64 imm_i64;
    I64 inc_amt;
    F64 imm_f64;
    CHashGlblVar *glbl_var;
    CHashClass *cls;
    I64 ic_data;
  };
  union {
    CCodeMisc *misc;
  };
//For register allocater
  CBitSet *live_out;
  CBitSet *live_in;
  CBitSet *use_regs;
  CBitSet *kill_regs;
  U8 *user_data;
  U8 *user_data2;
  union {
  //Used for live variable anlaysis x freinds
    Bool visited;
    Bool changed;
  };
};

Bool IsPtrNode(CRPN *rpn) {
  return rpn->ic_class->ptr_stars_cnt||rpn->ic_dim;
}

U0 RPNDel(CRPN *rpn) {
  QueRem(rpn);
  Free(rpn);
}
//This accounts for CHashClass/CHashFun
CHashClass *DerefType(CHashClass *cls) {
  if(!cls->ptr_stars_cnt) return cmp.internal_types[RT_I64];
  if(cls->type&HTT_FUN)
    return cls(CHashFun*)-1;
  return cls-1;
}
CHashClass *AddrOfType(CHashClass *cls) {
  if(cls->type&HTT_FUN)
    return cls(CHashFun*)+1;
  return cls+1;
}
I64 RPNPtrWidth(CRPN *r) {
  I64 mul=1;
  if(r->ic_dim)
    mul=r->ic_dim->total_cnt;
  if(r->ic_class->type&HTT_CLASS&&r->ic_class->ptr_stars_cnt)
    return mul*DerefType(r->ic_class)->size;
  //Is a function pointer???
  return 1*mul;
}
CHashClass *AssignRawTypeToNode(CRPN *r) {
  CHashClass *cls;
  I64 rt1,rt2;
  if(r->ic_class)
    return r->ic_class;
  switch(r->type) {
    break;case IC_IMM_I64: return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_IMM_F64: return r->ic_class=cmp.internal_types[RT_F64];
    break;case IC_STR_CONST: return r->ic_class=cmp.internal_types[RT_PTR];
    break;case IC_ADDR_IMPORT:
    if(r->glbl_var->type&HTT_FUN) {
      return r->glbl_var;
    } else if(r->glbl_var->type&HTT_GLBL_VAR) {
      if(r->glbl_var->fun_ptr) {
        return r->ic_class=cmp.internal_types[RT_PTR];
      }
      return r->ic_class=AddrOfType(r->glbl_var->var_class);
    }
    throw('Compiler');
    break;case IC_HEAP_GLBL:
    r->ic_dim=r->glbl_var->dim.next;
    return r->ic_class=r->glbl_var->var_class;
    break;case IC_SIZEOF:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_TYPE:
    goto unimp;
    break;case IC_GET_LABEL:
    goto unimp;
    break;case IC_REG:
    goto unimp;
    break;case IC_TO_I64: return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_TO_F64: return r->ic_class=cmp.internal_types[RT_F64];
    break;case IC_TO_BOOL: return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_TOUPPER: return r->ic_class=cmp.internal_types[RT_U8];
    break;case IC_HOLYC_TYPECAST: return r->ic_class;
    break;case IC_COMMA:
    r->ic_dim=r->next(CRPN*)->ic_dim;
    return r->ic_class=AssignRawTypeToNode(r->next);
    break;case IC_ADDR:
    //Address of array reduces to itself
    if(r->t.arg1_tree->ic_dim) {
      r->ic_class=r->t.arg1_class;
      r->ic_dim=r->t.arg1_tree->ic_dim;
      return r->t.arg1_class;
    }
    return r->ic_class=AddrOfType(AssignRawTypeToNode(r->next));
    break;case IC_COM: //bitwiase COMMPLEMENT
    r->ic_class=AssignRawTypeToNode(r->next);
    r->ic_dim=r->next(CRPN*)->ic_dim;
    return r->ic_class;
    break;case IC_NOT:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_UNARY_MINUS:
    r->ic_class=AssignRawTypeToNode(r->next);
    r->ic_dim=r->next(CRPN*)->ic_dim;
    return r->ic_class;
    break;case IC_DEREF:
deref:
    r->ic_class=r->t.arg1_class;
    r->ic_dim=r->t.arg1_tree->ic_dim;
    if(r->ic_dim)
      r->ic_dim=r->ic_dim->next;
    else
      r->ic_class=DerefType(r->ic_class);
    return r->ic_class;
    break;case IC_DEREF_PP: goto deref;
    break;case IC__PP:
    return r->ic_class=r->t.arg1_class;
    break;case IC__MM:
    return r->ic_class=r->t.arg1_class;
    break;case IC_PP_:
    return r->ic_class=r->t.arg1_class;
    break;case IC_MM_:
    return r->ic_class=r->t.arg1_class;
    break;case IC_SHL:
binop:
    switch(r->type) {
      break;case IC_SUB:
      rt1=r->t.arg1_class->raw_type;
      rt2=r->t.arg2_class->raw_type;
      if(IsPtrNode(r->t.arg1_tree)&&IsPtrNode(r->t.arg2_tree))
        return r->ic_class=cmp.internal_types[RT_I64];
      default:
binop_dft:
      rt1=r->t.arg1_class->raw_type;
      rt2=r->t.arg2_class->raw_type;  
      if(rt1>rt2)
        return r->ic_class=r->t.arg1_class;
      return r->ic_class=r->t.arg2_class;    
      break;case IC_ADD:
      if(r->t.arg1_tree->ic_dim)
        r->ic_dim=r->t.arg1_tree->ic_dim;
      else if(r->t.arg2_tree->ic_dim)
        r->ic_dim=r->t.arg2_tree->ic_dim;
      else if(r->t.arg1_class->ptr_stars_cnt)
        return r->ic_class=r->t.arg1_class;
      else if(r->t.arg2_class->ptr_stars_cnt)
        return r->ic_class=r->t.arg2_class;
      goto binop_dft;
    }
    break;
    case IC_MAX_I64:
    case IC_MIN_I64:
    case IC_FS:
    case IC_GS:
    case IC_BT:
    case IC_BTS:
    case IC_BTR:
    case IC_BTC:
    case IC_LBTS:
    case IC_LBTR:
    case IC_LBTC:
    r->ic_class=cmp.internal_types[RT_I64];
    break;
    case IC_MAX_U64:
    case IC_MIN_U64:
    r->ic_class=cmp.internal_types[RT_U64];
    break;
    break;case IC_SHR:
    goto binop;
    break;case IC_SHL_CONST:
    goto binop;
    break;case IC_SHR_CONST:
    goto binop;
    break;case IC_POWER:
    case IC_SQR:
    return r->ic_class=cmp.internal_types[RT_F64];
    break;case IC_MUL:
    goto binop;
    break;case IC_DIV:
    goto binop;
    break;case IC_MOD:
    goto binop;
    break;case IC_AND:
    goto binop;
    break;case IC_OR:
    goto binop;
    break;case IC_XOR:
    goto binop;
    break;case IC_ADD:
    goto binop;
    break;case IC_SUB:
    goto binop;
    break;case IC_ADD_CONST:
    goto binop;
    break;case IC_SUB_CONST:
    goto binop;
    break;case IC_EQU_EQU:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_NOT_EQU:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_LESS:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_GREATER_EQU:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_GREATER:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_LESS_EQU:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_PUSH_CMP: //???
unimp:
    throw('Compiler');
    break;case IC_AND_AND:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_OR_OR:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_XOR_XOR:
    return r->ic_class=cmp.internal_types[RT_I64];
    break;case IC_ASSIGN:
assign:
    if(r->t.arg1_tree->ic_dim) {
      r->ic_dim=r->t.arg1_tree->ic_dim;
      return r->ic_class=r->t.arg1_class;
    }
    if(r->t.arg1_class->raw_type<RT_I64)
      return r->ic_class=cmp.internal_types[RT_I64];
    return r->ic_class=r->t.arg1_class;
    break;case IC_ASSIGN_PP:
    goto assign;
    break;case IC_ASSIGN_MM:
    goto assign;
    break;case IC_SHL_EQU:
    goto assign;
    break;case IC_SHR_EQU:
    goto assign;
    break;case IC_MUL_EQU:
    goto assign;
    break;case IC_DIV_EQU:
    goto assign;
    break;case IC_MOD_EQU:
    goto assign;
    break;case IC_AND_EQU:
    goto assign;
    break;case IC_OR_EQU:
    goto assign;
    break;case IC_XOR_EQU:
    goto assign;
    break;case IC_ADD_EQU:
    goto assign;
    break;case IC_SUB_EQU:
    goto assign;
    //TODO actually do something
    break;default: 
    return r->ic_class=cmp.internal_types[RT_U0];
  }
}

U0 PrsOffsetOf(CCmpCtrl *cc)
{
  CHashClass *tmpc;
  CMemberLst *tmpm;
  CRPN *rpn;
  I64 i;
  if (cc->token!=TK_IDENT)
    LexExcept(cc,"Invalid class at ");
  if (tmpm=cc->local_var_entry)
    tmpc=tmpm->member_class;
  else {
    tmpc=cc->hash_entry;
    if (!tmpc || !(tmpc->type & (HTT_CLASS|HTT_GLBL_VAR)))
      LexExcept(cc,"Invalid class at ");
    else if (tmpc->type & HTT_GLBL_VAR)
      tmpc=tmpc(CHashGlblVar *)->var_class;
  }
  if (Lex(cc)!='.')
    LexExcept(cc,"Expecting '.' at ");
  i=0;
  do {
    if (Lex(cc)!=TK_IDENT || !(tmpm=MemberFind(cc->cur_str,tmpc)))
      LexExcept(cc,"Invalid member at ");
    else if (cc->local_var_entry)
      cc->local_var_entry->use_cnt--;
    i+=tmpm->offset;
    tmpc=tmpm->member_class;
  } while (Lex(cc)=='.');
  RPNAddNew(
    cc,
    IC_IMM_I64,
    i,
    cmp.internal_types[RT_I64]
  );
}
U0 PrsSizeOf(CCmpCtrl *cc)
{
  CRPN *rpn;
  CHashClass *tmpc;
  CMemberLst *tmpm;
  CDbgInfo *dbg_info;
  I64 i;
  if (cc->token!=TK_IDENT)
    LexExcept(cc,"Invalid class at ");
  if (tmpm=cc->local_var_entry) {
    tmpc=tmpm->member_class;
    i=tmpc->size*tmpm->dim.total_cnt;
    if (Lex(cc)=='.')
      goto pu_sizeof_member;
  } else {
    if (!(tmpc=cc->hash_entry) || !(tmpc->type &
	  (HTT_CLASS|HTT_INTERNAL_TYPE|HTT_GLBL_VAR|
	  HTT_FUN|HTT_EXPORT_SYS_SYM)))
      LexExcept(cc,"Invalid class at ");
    if (tmpc->type&(HTT_FUN|HTT_EXPORT_SYS_SYM)) {
      if (!(dbg_info=tmpc(CHashFun *)->dbg_info))
	LexExcept(cc,"Size not defined at ");
      i=dbg_info->body[dbg_info->max_line+1-dbg_info->min_line]
	    -dbg_info->body[0];
      Lex(cc);
    } else {
      i=tmpc->size;
      while (Lex(cc)=='.') {
pu_sizeof_member:
	if (!(tmpc->type & (HTT_CLASS|HTT_GLBL_VAR)))
	  LexExcept(cc,"Invalid class at ");
	else if (tmpc->type & HTT_GLBL_VAR)
	  tmpc=tmpc(CHashGlblVar *)->var_class;
	if (Lex(cc)!=TK_IDENT || !(tmpm=MemberFind(cc->cur_str,tmpc)))
	  LexExcept(cc,"Invalid member at ");
	else if (cc->local_var_entry)
	  cc->local_var_entry->use_cnt--;
	tmpc=tmpm->member_class;
//Probably others like this:
	//#assert offset(CHashClass.size)==offset(CHashGlblVar.size)
	i=tmpc->size*tmpm->dim.total_cnt;
      }
    }
  }
  if (cc->token=='*') {
    while (Lex(cc)=='*');
    i=sizeof(U8 *);
  }
  RPNAddNew(
    cc,
    IC_IMM_I64,
    i,
    cmp.internal_types[RT_I64],
  );
}
extern U0 DumpRPN(CRPN*,I64 x=0);
CRPN *ICNext(CRPN *rpn) {
  CRPN *next=rpn->next;
  I64 cnt;
  switch(intermediate_code_table[rpn->type].arg_cnt) {
    case IS_V_ARG:
    cnt=rpn->length;
    while(cnt--)
      next=ICNext(next);
    break;
    case IS_2_ARG:
      next=ICNext(next);
      next=ICNext(next);
    break;
    case IS_1_ARG:
      next=ICNext(next);
    break;
    case IS_0_ARG:
  }
  return next;
}
//
// add_to_prs_stk will also not assign the tree args
//
U0 RPNAdd(CCmpCtrl *cc,CRPN *rpn,Bool add_to_prs_stk=TRUE) {
  CRPN *rpn2,*a,*b,*rpn3;
  I64 width;
  CHashClass *ic_class;
  CHashFun *powh;
  Bool divide=FALSE;
  if(!(cc->lex_include_stk->flags&LFSF_DEFINE))
    rpn->ic_line=cc->lex_include_stk->line_num;
  QueIns(rpn,&cc->coc.coc_head);
  if(add_to_prs_stk) {
    switch(intermediate_code_table[rpn->type].arg_cnt) {
        break;case IS_V_ARG:
        width=rpn->length;
        while(width--) {
	  if(!PrsPop3(cc->ps))
	    LexExcept(cc,"Unexpected end of expression at ");
        }
        break;case IS_2_ARG:
        rpn->t.arg2_tree=PrsPop3(cc->ps);
        rpn->t.arg1_tree=PrsPop3(cc->ps);
	if(!rpn->t.arg1_tree||!rpn->t.arg2_tree)
	  LexExcept(cc,"Unexpected end of expression at ");
        rpn->t.arg2_class=rpn->t.arg2_tree->ic_class;
        rpn->t.arg1_class=rpn->t.arg1_tree->ic_class;
        break;case IS_1_ARG:
        rpn->t.arg1_tree=PrsPop3(cc->ps);
	if(!rpn->t.arg1_tree)
	  LexExcept(cc,"Unexpected end of expression at ");
        rpn->t.arg1_class=rpn->t.arg1_tree->ic_class;
        break;case IS_0_ARG:
    }
    switch(rpn->type) {
        break;case IC_POWER:
	a=ICNext(rpn);
	if(rpn->t.arg1_class->raw_type!=RT_F64) {
	  b=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) b->ic_flags|=ICF_LOCK;
	  b->type=IC_TO_F64;
	  b->ic_class=cmp.internal_types[RT_F64];
	  QueInsRev(b,rpn->t.arg1_tree);
	  rpn->t.arg1_tree=b;
	  rpn->t.arg1_class=cmp.internal_types[RT_F64];
	}
	if(rpn->t.arg2_class->raw_type!=RT_F64) {
	  b=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) b->ic_flags|=ICF_LOCK;
	  b->type=IC_TO_F64;
	  b->ic_class=cmp.internal_types[RT_F64];
	  QueInsRev(b,rpn->t.arg2_tree);
	  rpn->t.arg2_tree=b;
	  rpn->t.arg2_class=cmp.internal_types[RT_F64];
	}
        rpn->type=IC_CALL_INDIRECT;
        rpn->length=3;
        powh=HashFind("Pow",cc->htc.hash_table_lst,HTT_FUN);
	if(!powh)
	  throw('Compiler');
        if(cc->flags&CCF_AOT_COMPILE||Bt(&powh->flags,Cf_EXTERN)) {
	  b=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) b->ic_flags|=ICF_LOCK;
	  b->type=IC_ADDR_IMPORT;
	  b->ic_data=CodeMiscHashNew(cc,powh);
	  b->ic_class=powh;
        } else {
	  b=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) b->ic_flags|=ICF_LOCK;
	  b->type=IC_IMM_I64;
	  b->ic_data=powh->exe_addr;
	  b->ic_class=powh;
        }
        QueInsRev(b,a);
	rpn->ic_class=powh->return_class;
        break;case IC_PP_:
      case IC_MM_:
      case IC__PP:
      case IC__MM:
        a=rpn->t.arg1_tree;
        if(a->ic_dim||a->ic_class->ptr_stars_cnt) {
	  rpn->ic_data=RPNPtrWidth(a);
        } else
	  rpn->ic_data=1;
        break;case IC_DEREF:
//
      //Dereferencing an array of an array does nothing.
      // Arrays are flat so we only take off a dim,derefence the pointer
      // I64i arr[2][2]={{1,2},{3,4}};
      //   is
      // [1,2,3,4]
      //
        a=rpn->t.arg1_tree;
        if(a->ic_dim)
	  if(a->ic_dim->next) {
	    a->ic_dim=a->ic_dim->next;
	    RPNDel(rpn);
	    PrsPush3(cc->ps,a); //Put back on the stack
	    return;
	  }
//Can't "Derefence" a function pointer,it is symbolic
        if(a->ic_class->ptr_stars_cnt==1&&(a->ic_class->type&HTT_FUN)) {
	  a->ic_class=DerefType(a->ic_class);
	  RPNDel(rpn);
	  PrsPush3(cc->ps,a); //Put back on the stack
	  return;
        }
        break;case IC_ADDR:
//Address of dereference does nothing.
      //&*a;
        if(rpn->t.arg1_tree->type==IC_DEREF) {
//rpn=='&'
	  //b=='*'
	  //a=='var'
	  b=rpn->t.arg1_tree;
	  a=b->t.arg1_tree;
	  RPNDel(rpn);
	  PrsPush3(cc->ps,a); //Put back on the stack
	  if(b->ic_dim) {
	    a->ic_class=b->ic_class;
	    a->ic_dim=b->ic_dim;
	  } else {
	    a->ic_dim=NULL;
	    a->ic_class=AddrOfType(b->ic_class);
	  }
	  RPNDel(b);
	  return;
        }
        break;case IC_SUB:
        a=rpn->t.arg1_tree;
        b=rpn->t.arg2_tree;
        if(IsPtrNode(a)) {
	  width=RPNPtrWidth(a);
	  rpn2=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) rpn2->ic_flags|=ICF_LOCK;
	  rpn2->type=IC_IMM_I64;
	  rpn2->imm_i64=width;
	  rpn2->ic_class=cmp.internal_types[RT_I64];
	  if(IsPtrNode(b)) {
	    divide=TRUE;
	  } else {
//Use a's class for rpn
	    rpn->ic_class=a->ic_class;          
mul:
	    rpn3=CAlloc(sizeof CRPN);
	    if(cc->lock_cnt) rpn3->ic_flags|=ICF_LOCK;
	    rpn3->type=IC_MUL;
	    rpn3->ic_class=cmp.internal_types[RT_I64];
	    rpn3->t.arg2_tree=rpn2;
	    rpn3->t.arg1_class=cmp.internal_types[RT_I64];
	    rpn3->t.arg2_class=cmp.internal_types[RT_I64];
	    rpn3->t.arg1_tree=b;
	    rpn->t.arg2_tree=rpn3;
	    rpn2->ic_line=rpn->ic_line;
	    rpn3->ic_line=rpn->ic_line;
	    QueIns(rpn3,rpn);
	    QueIns(rpn2,rpn3);
	  }
        }
        break;case IC_ADD_EQU:
      case IC_SUB_EQU:
        a=rpn->t.arg1_tree;
        b=rpn->t.arg2_tree;
        if(a->ic_dim) {
	  width=a->ic_class->size*a->ic_dim->total_cnt;
	  rpn2=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) rpn2->ic_flags|=ICF_LOCK;
	  rpn2->type=IC_IMM_I64;
	  rpn2->imm_i64=width;
	  rpn2->ic_class=cmp.internal_types[RT_I64];
	  rpn->ic_dim=a->ic_dim;
//Use a's class for rpn
	  rpn->ic_class=a->ic_class;
	  goto mul;
        } else if(IsPtrNode(a)) {
	  width=RPNPtrWidth(a);
	  rpn2=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) rpn2->ic_flags|=ICF_LOCK;
	  rpn2->type=IC_IMM_I64;
	  rpn2->imm_i64=width;
//Use a's class for rpn
	  rpn->ic_class=a->ic_class;
	  goto mul;    
        }
        break;case IC_ADD:
        a=rpn->t.arg1_tree;
        b=rpn->t.arg2_tree;
#ifdef TARGET_X86
        //I will implement this later for ARM64
        if(cc->flags&CCF_AOT_COMPILE) {
	  if(a->type==IC_ADDR_IMPORT&&b->type==IC_IMM_I64) {
	    width=1;
	    if(a->ic_dim)
	      width=a->ic_class->size*a->ic_dim->total_cnt;
	    else if(IsPtrNode(a))
	      width=RPNPtrWidth(a);
	    a->ic_data(CCodeMisc*)->offset+=b->imm_i64*width;
	    RPNDel(rpn);
	    RPNDel(b);
	    PrsPush3(cc->ps,a); //Put back on the stack
	    return;
	  }
	  if(b->type==IC_ADDR_IMPORT&&a->type==IC_IMM_I64) {
	    width=1;
	    if(b->ic_dim)
	      width=b->ic_class->size*b->ic_dim->total_cnt;
	    else if(IsPtrNode(b))
	      width=RPNPtrWidth(b);
	    b->ic_data(CCodeMisc*)->offset+=a->imm_i64*width;
	    RPNDel(rpn);
	    RPNDel(a);
	    PrsPush3(cc->ps,b); //Put back on the stack
	    return;
	  }
        }
#endif
#ifdef TARGET_RISCV
        //I will implement this later for ARM64
        if(cc->flags&CCF_AOT_COMPILE) {
	  if(a->type==IC_ADDR_IMPORT&&b->type==IC_IMM_I64) {
	    width=1;
	    if(a->ic_dim)
	      width=a->ic_class->size*a->ic_dim->total_cnt;
	    else if(IsPtrNode(a))
	      width=RPNPtrWidth(a);
	    a->ic_data(CCodeMisc*)->offset+=b->imm_i64*width;
	    RPNDel(rpn);
	    RPNDel(b);
	    PrsPush3(cc->ps,a); //Put back on the stack
	    return;
	  }
	  if(b->type==IC_ADDR_IMPORT&&a->type==IC_IMM_I64) {
	    width=1;
	    if(b->ic_dim)
	      width=b->ic_class->size*b->ic_dim->total_cnt;
	    else if(IsPtrNode(b))
	      width=RPNPtrWidth(b);
	    b->ic_data(CCodeMisc*)->offset+=a->imm_i64*width;
	    RPNDel(rpn);
	    RPNDel(a);
	    PrsPush3(cc->ps,b); //Put back on the stack
	    return;
	  }
        }
#endif
        if(a->ic_dim) {
	  width=a->ic_class->size*a->ic_dim->total_cnt;
	  rpn2=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) rpn2->ic_flags|=ICF_LOCK;
	  rpn2->type=IC_IMM_I64;
	  rpn2->imm_i64=width;
	  rpn2->ic_class=cmp.internal_types[RT_I64];
	  rpn->ic_dim=a->ic_dim;
//Use a's class for rpn
	  rpn->ic_class=a->ic_class;
	  goto mul;
        } else if(IsPtrNode(a)) {
	  width=RPNPtrWidth(a);
	  rpn2=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) rpn2->ic_flags|=ICF_LOCK;
	  rpn2->type=IC_IMM_I64;
	  rpn2->imm_i64=width;
//Use a's class for rpn
	  rpn->ic_class=a->ic_class;
	  goto mul;    
        }
        if(b->ic_dim) {
	  width=b->ic_class->size*b->ic_dim->total_cnt;
	  rpn2=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) rpn2->ic_flags|=ICF_LOCK;
	  rpn2->type=IC_IMM_I64;
	  rpn2->imm_i64=width;
	  rpn2->ic_class=cmp.internal_types[RT_PTR];
	  rpn->ic_dim=b->ic_dim;
	  rpn->ic_class=b->ic_class;
mul2:
	  rpn3=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) rpn3->ic_flags|=ICF_LOCK;
	  rpn3->type=IC_MUL;
	  rpn3->ic_class=cmp.internal_types[RT_I64];
	  rpn3->t.arg2_tree=rpn2;
	  rpn3->t.arg1_class=cmp.internal_types[RT_I64];
	  rpn3->t.arg2_class=cmp.internal_types[RT_I64];
	  rpn3->t.arg1_tree=a;
	  rpn->t.arg1_tree=rpn3;
	  rpn3->ic_line=rpn->ic_line;
	  QueInsRev(rpn3,a);
	  QueIns(rpn2,rpn3);
        } else if(IsPtrNode(b)) {
	  width=RPNPtrWidth(b);
	  rpn2=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) rpn2->ic_flags|=ICF_LOCK;
	  rpn2->type=IC_IMM_I64;
	  rpn2->imm_i64=width;
//Use b's class for rpn
	  rpn->ic_class=b->ic_class;
	  goto mul2;    
        }
    } 
  }
  if(add_to_prs_stk) {
    PrsPush3(cc->ps,rpn);
  }
  if(!rpn->ic_class)
    AssignRawTypeToNode(rpn);
  if(divide) {
    RPNAdd(cc,rpn2,add_to_prs_stk);
    rpn3=CAlloc(sizeof CRPN);
    if(cc->lock_cnt) rpn3->ic_flags|=ICF_LOCK;
    rpn3->type=IC_DIV;
    rpn3->ic_class=cmp.internal_types[RT_I64];
    RPNAdd(cc,rpn3,add_to_prs_stk);
  }
} 
CRPN *RPNAddNew(CCmpCtrl *cc,I64 type,I64 data=0,CHashClass *cls=NULL,Bool add_to_prs_stk=TRUE) {
  CRPN *rpn=CAlloc(sizeof CRPN);
  if(cc->lock_cnt) rpn->ic_flags|=ICF_LOCK;
  rpn->type=type;
  rpn->ic_data=data;
  rpn->ic_class=cls;
  RPNAdd(cc,rpn,add_to_prs_stk);
  return rpn;
}
U0 PushOpers(CCmpCtrl *cc,I64 cur_prec,Bool fin=FALSE) {
  CRPN *rpn,*r2;
  I64 v;
  while(cc->ps->ptr>0) { //cc->ps->stk starts at 1
    v=PrsPop(cc->ps);
    rpn=PrsPop2(cc->ps); //NULL if paren
    if(PREC_NULL==v.u32[1]&&!fin) {
      //Put back on stack
      PrsPush(cc->ps,v);
      PrsPush2(cc->ps,rpn);
      break;
    } else if(cur_prec.u32[1]==PREC_NULL&&!fin) {
      //PREC_NULL is a paren,so if we have a paren,pop until will get another paren
      goto pop;
    }
    if(v.u32[1]<cur_prec.u32[1]||fin) {
pop:
      if(!rpn) LexExcept(cc,"Invalid expression at ");
      if(rpn->type==IC_ADDR) {
        //
        //Address of array does nothing in terms of SYNTAX
        // But is used internaly so we check ONLY here.
        r2=cc->coc.coc_head.next;
	if(&cc->coc.coc_head==r2) LexExcept(cc,"Can't get address of nothing at ");
        if(r2->ic_dim) {
          Free(rpn);
          goto next;
        }
      } else if(rpn->type==IC_DEREF) {
         r2=cc->coc.coc_head.next;
	if(&cc->coc.coc_head==r2) LexExcept(cc,"Can't get derefernce nothing at ");
      }
      if(rpn)
        RPNAdd(cc,rpn);
next:;
    } else if(v.u32[1]==cur_prec.u32[1]&&!(cur_prec.u32[1]&ASSOCF_RIGHT)) {
      goto pop;
    } else {
      //Put it back on the stack
      PrsPush(cc->ps,v);
      PrsPush2(cc->ps,rpn);
      break;
    }
  }
}

//start_idx is the first argument
I64 ImplicitFunCall(CCmpCtrl *cc,CHashFun *fun,I64 start_idx=0) {
  I64 idx;
  CRPN *rpn;
  CMemberLst *lst=fun->member_lst_and_root;
  CHashClass *last_c=cc->coc.coc_head.next(CRPN*)->ic_class;
  CCodeMisc *cm;
  for(idx=0;idx!=fun->arg_cnt;idx++) {
    if(idx<start_idx)
      goto next;
    while(last_c->ptr_stars_cnt)
      last_c=DerefType(last_c);
    if(lst->flags&MLF_LASTCLASS&&last_c) {
      cm=COCMiscNew(cc,CMT_STR_CONST);
      cc->flags|=CCF_HAS_MISC_DATA;
      cm->str=StrNew(last_c->str);
      cm->st_len=StrLen(last_c->str)+1;
      RPNAddNew(
          cc,
          IC_STR_CONST,
          cm,
          last_c=cmp.internal_types[RT_U8]+1
        );
    } else if(lst->flags&(MLF_DFT_AVAILABLE|MLF_STR_DFT_AVAILABLE)) {
      last_c=lst->member_class;
      if(lst->member_class->raw_type==RT_F64) {
        RPNAddNew(
          cc,
          IC_IMM_F64,
          lst->dft_val,
          cmp.internal_types[RT_F64]
        );
      } else {
        RPNAddNew(
          cc,
          IC_IMM_I64,
          lst->dft_val,
          cmp.internal_types[RT_I64]
        );
      }
    } else if(lst->flags&MLF_DOT_DOT_DOT) {
      //Do nothing ,argc/argv are implicit
    } else {
      LexExcept(cc,"Missing dft value for argument at: ");
    }
next:
    lst=lst->next;
  }
  return idx;
}

CCodeMisc *CodeMiscHashNew(CCmpCtrl *cc,CHash* h) {
  CCodeMisc *head=&cc->coc.coc_next_misc,*cur;
  #ifdef TARGET_X86
  //In X86_64,I will do every relocation as a short one that isnt an import
  if(cc->flags&CCF_AOT_COMPILE) {
//Globals are on heap due to MacOS RW/X protections
    if(!(h->type&HTF_IMPORT)&&!(h->type&HTT_GLBL_VAR)) {
      cur=COCMiscNew(cc,CMT_HASH_ENTRY);
      cur->flags|=CMF_SHORT_ADDR;
      cur->h=h;
      return cur;
    }
  }
  #endif
  #ifdef TARGET_RISCV
  //In RISCV do the same
  if(cc->flags&CCF_AOT_COMPILE) {
    if(!(h->type&HTF_IMPORT)) {
      cur=COCMiscNew(cc,CMT_HASH_ENTRY);
      cur->flags|=CMF_SHORT_ADDR;
      cur->h=h;
      return cur;
    }
  }
  #endif

  /*for(cur=head->next;cur!=head;cur=cur->next) {
    if(cur->h==h)
      return cur;
  }*/
  cur=COCMiscNew(cc,CMT_HASH_ENTRY);
  cur->h=h;
  return cur;
}

U0 DbgPrint(U8 *fmt,...) {
  U8 *s=StrPrintJoin(NULL,fmt,argc,argv);
  DbgPutS(s);
  Free(s);
}
CRPN *DumpRPN(CRPN *rpn,I64 indent=0) {
  I64 idx=0;
  CHashClass *cls=rpn->ic_class;
  CRPN *next=rpn->next;
  if(cls) {
    while(cls->ptr_stars_cnt)
      cls=DerefType(cls);
    DbgPrint("%x%*c%s(%s*%d)",rpn,indent+1,' ',intermediate_code_table[rpn->type].name,cls->str,rpn->ic_class->ptr_stars_cnt);
  } else
	DbgPrint("%x%*c%s",rpn,indent+1,' ',intermediate_code_table[rpn->type].name);
  switch(rpn->type) {
    break;case IC_IMM_I64:
    case IC_FRAME:
    DbgPrint(",%d,",rpn->imm_i64);
    break;case IC_IMM_F64:
    ",%n,",rpn->imm_f64;
  }
  
  switch(intermediate_code_table[rpn->type].arg_cnt) {
    break;case IS_V_ARG:
    DbgPrint("(%d)\n",rpn->length);
    for(idx=0;idx!=rpn->length;idx++)
      next=DumpRPN(next,indent+1);
    break;case IS_2_ARG: 
    DbgPrint(",%x,%x\n",rpn->t.arg1_tree,rpn->t.arg2_tree);
    next=DumpRPN(next,indent+1);
    next=DumpRPN(next,indent+1);
    break;case IS_1_ARG:
    DbgPrint(",%x\n",rpn->t.arg1_tree);
    next=DumpRPN(next,indent+1);
    break;case IS_0_ARG:
    DbgPrint("\n");
  }
  return next;
}

U0 PrsMembers(CCmpCtrl *cc) {
  CRPN *head=cc->coc.coc_head.next,*added,*new;
  if(head==&cc->coc.coc_head.next)
    LexExcept(cc,"Expected to get the member of something!");
  CHashClass *cls=head->ic_class;
  CArrayDim *ic_dim=head->ic_dim;
  CMemberLst *mlst;
  CCodeMisc *cm;
  I64 off=0;
  Bool needs_addr=!(cls->ptr_stars_cnt||ic_dim);
  Bool needs_flush=FALSE;
  while(TRUE) {
    if(cc->token=='.') {
      if(needs_addr) {
        needs_addr=FALSE;
        RPNAddNew(cc,IC_ADDR,,cmp.internal_types[RT_I64],TRUE);
      }
      //
      // Nroot here: Here's the deal.
      //  If we get the address of an array access,the derefence gets removed
      //    &a[1]==a+1 //a has ic_dim
      //  So I will remove the ic_dim and make it a pointer as it will be "a pointer"
      //
      added=cc->coc.coc_head.next;
      if(added->ic_dim) {
        added->ic_dim=NULL;
        cls=added->ic_class;
      }
      Lex(cc);
      mlst=MemberFind(cc->cur_str,cls);
      if(!mlst||ic_dim)
        LexExcept(cc,"Invalid member at");
      off+=mlst->offset;
      cls=mlst->member_class;
      ic_dim=mlst->dim.next;
      Lex(cc);
      needs_flush=TRUE;
    } else if(cc->token==TK_DEREFERENCE) {
      if(needs_flush) {
        if(off) {
          //Prevent from expading ptr airth.
          cc->coc.coc_head.next(CRPN*)->ic_class=cmp.internal_types[RT_I64];
          RPNAddNew(cc,IC_IMM_I64,off,,TRUE);
          RPNAddNew(cc,IC_ADD,,,TRUE);
        }
        RPNAddNew(cc,IC_DEREF,off,cmp.internal_types[RT_I64],TRUE);
      }
      needs_flush=TRUE;
      off=0;
      if(Lex(cc)!=TK_IDENT)
	LexExcept(cc,"Expected an identifier at");
      needs_addr=FALSE; 
      if(ic_dim) {
        ic_dim=ic_dim->next;
        RPNAddNew(cc,IC_DEREF,,,TRUE);
        mlst=MemberFind(cc->cur_str,cls);
      } else if(DerefType(cls)) {
        mlst=MemberFind(cc->cur_str,DerefType(cls));
      } else {
        LexExcept(cc,"Use '.' at");
      }
      if(!mlst)
        LexExcept(cc,"Invalid member at");
      off+=mlst->offset;
      Lex(cc);
      cls=mlst->member_class;
      ic_dim=mlst->dim.next;
    } else
      break;
  }
  if(off) {
    //Prevent from expading ptr airth.
    cc->coc.coc_head.next(CRPN*)->ic_class=cmp.internal_types[RT_I64];
    RPNAddNew(cc,IC_IMM_I64,off,,TRUE);
    RPNAddNew(cc,IC_ADD,off,,TRUE);
  }
  if(mlst->flags&MLF_FUN) {
    //TempleOS stores Function class in ->fun_ptr for some reason(when MLF_FUN is set) 
    cls=mlst->fun_ptr;
  }
            
  if(!ic_dim)
    new=RPNAddNew(cc,IC_DEREF,off,cls,TRUE);
  else {
    added=cc->coc.coc_head.next;
    //Here's the deal
    // IC_DEREF's act on their ic_class,so if our last item is a IC_DEFEF
    //  Derefernece as usual,THEN use our dim
    //  a->some_class->array_u8 
    // a->some_class must derefnce to a pointer(8 bytes),then we switch the result type to
    // array of u8[0]. I do this by adding 0 to the result
    if(added->type==IC_DEREF) {
      RPNAddNew(cc,IC_IMM_I64,0,cmp.internal_types[RT_I64],TRUE);
      added=RPNAddNew(cc,IC_ADD,0,cls,TRUE);
    }
    added->ic_dim=ic_dim;
    added->ic_class=cls;
  }
}

Bool PrsExpression(CCmpCtrl *cc,U8 **ul=NULL,I64 flags=0,CPrsStk *_ps=NULL) { //TODO REMOVE Ul
  I64 cur_prec,stk_ptr,tok=cc->token,idx,arity,v,mode,str_len;
  CRPN *orig_next=cc->coc.coc_head.next,*argc,*argv,*next;
  CCodeMisc *misc;
  I64 intern_code;
  Bool finish=FALSE;
  CPrsStk *old=cc->ps;
  CArrayDim tmpad,*ic_dim;
  if(!_ps) {
    cc->ps=MAlloc(sizeof CPrsStk);
    cc->ps->ptr=0;
    cc->ps->ptr2=0;
    cc->ps->ptr3=0;
  } else {
    cc->ps=_ps;
  }
  I64 last_consumes_right=1;
  CHashFun *ic_fun=NULL,*last_c=NULL;
  CHashExport *tmpex;
  CHashClass *tmpc;
  CMemberLst *mlst;
  CAsmUndefHash *tmpauh;
//u32[0] is type
  //u32[1] is prec
  CRPN *rpn,*rpn2;
  while(!finish) {
    rpn=NULL;
    cur_prec=-1;
    tok=cc->token;
    if((flags&PEF_NO_COMMA)&&(tok==',')) {
      finish=TRUE;
      break;
    }
    switch(tok) {
        break;start:
        case TK_PLUS_PLUS:
	  if(last_consumes_right) {
	    cur_prec=((PREC_UNARY_PRE|ASSOCF_RIGHT)<<32|IC_PP_);
	    last_consumes_right=1;
	  } else {
	    cur_prec=(PREC_UNARY_POST<<32|IC__PP);
	    last_consumes_right=0;
	  }
	  break;case TK_MINUS_MINUS:
	  if(last_consumes_right) {
	    cur_prec=((PREC_UNARY_PRE|ASSOCF_RIGHT)<<32|IC_MM_);
	    last_consumes_right=1;
	  } else {
	    cur_prec=(PREC_UNARY_POST<<32|IC__MM);
	    last_consumes_right=0;
	  }
	  break;case '&':
	  if(last_consumes_right) {
	    cur_prec=((PREC_UNARY_PRE|ASSOCF_RIGHT)<<32|IC_ADDR);
	  } else {
	    cur_prec=(PREC_AND<<32|IC_AND);
	  }
	  last_consumes_right=1;
	  break;case '*':
	  if(last_consumes_right) {
	    cur_prec=((PREC_UNARY_PRE|ASSOCF_RIGHT)<<32|IC_DEREF);
	    last_consumes_right=1;
	  } else {
	    cur_prec=(PREC_MUL<<32|IC_MUL);
	    last_consumes_right=1;
	  }
	  break;case ',':
	  if(last_consumes_right)
	    LexExcept(cc,"Unexpected ',' at ");
	  cur_prec=(PREC_MAX<<32|IC_COMMA);
	  last_consumes_right=1;
	  break;case '+':
	  if(last_consumes_right) {
//Positive does nothing
	    cur_prec=-1;
	  } else {
	    cur_prec=(PREC_ADD<<32|IC_ADD);
	    last_consumes_right=1;
	  }
	  break;case '-':
	  if(last_consumes_right) {
	    cur_prec=((PREC_UNARY_PRE|ASSOCF_RIGHT)<<32|IC_UNARY_MINUS);
	    last_consumes_right=1;
	  } else {
	    cur_prec=(PREC_ADD<<32|IC_SUB);
	    last_consumes_right=1;
	  }
	  break;case '~':
	  if(last_consumes_right) {
	    cur_prec=((PREC_UNARY_PRE|ASSOCF_RIGHT)<<32|IC_COM);
	    last_consumes_right=1;
	  } else
	    LexExcept(cc,"Expected '~'.");
	  break;case '!':
	  if(last_consumes_right) {
	    cur_prec=((PREC_UNARY_PRE|ASSOCF_RIGHT)<<32|IC_NOT);
	    last_consumes_right=1;
	  } else
	    LexExcept(cc,"Expected '!'.");
	  break;end:
        if(cur_prec!=-1) { //See above '+' operator
	  if(cur_prec.u32[0]==IC_ADDR) {
	    flags|=PEF_ADDR_OF;
	  } else
	    flags&=~PEF_ADDR_OF;
	  PushOpers(cc,cur_prec);
	  rpn=CAlloc(sizeof CRPN);
	  if(cc->lock_cnt) rpn->ic_flags|=ICF_LOCK;
	  rpn->type=cur_prec.u32[0];
	  PrsPush(cc->ps,cur_prec);
	  PrsPush2(cc->ps,rpn);
	  QueInit(rpn);
        }
//After getting an operator,we are no longer accepting function calls for now
        flags&=~PEF_FUN_BEFORE;
        tok=Lex(cc);
        break;start:
        if(last_consumes_right)
	  LexExcept(cc,"Unexpected operator.");
        case '`':
	  cur_prec=(PREC_EXP<<32|IC_POWER);
	  last_consumes_right=1;
	  break;case TK_SHL:
	  cur_prec=(PREC_EXP<<32|IC_SHL);
	  last_consumes_right=1;
	  break;case TK_SHR:
	  cur_prec=(PREC_EXP<<32|IC_SHR);
	  last_consumes_right=1;
	  break;case '/':
	  cur_prec=(PREC_MUL<<32|IC_DIV);
	  last_consumes_right=1;
	  break;case '%':
	  cur_prec=(PREC_MUL<<32|IC_MOD);
	  last_consumes_right=1;
	  break;case '|':
	  cur_prec=(PREC_OR<<32|IC_OR);
	  last_consumes_right=1;
	  break;case '^':
	  cur_prec=(PREC_XOR<<32|IC_XOR);
	  last_consumes_right=1;
	  break;case '<':
	  cur_prec=(PREC_CMP<<32|IC_LESS);
	  last_consumes_right=1;
	  break;case '>':
	  cur_prec=(PREC_CMP<<32|IC_GREATER);
	  last_consumes_right=1;
	  break;case TK_GREATER_EQU:
	  cur_prec=(PREC_CMP<<32|IC_GREATER_EQU);
	  last_consumes_right=1;      
	  break;case TK_LESS_EQU:
	  cur_prec=(PREC_CMP<<32|IC_LESS_EQU);
	  last_consumes_right=1;
	  break;case TK_EQU_EQU:
	  cur_prec=(PREC_CMP2<<32|IC_EQU_EQU);
	  last_consumes_right=1;      
	  break;case TK_NOT_EQU:
	  cur_prec=(PREC_CMP2<<32|IC_NOT_EQU);
	  last_consumes_right=1;
	  break;case TK_AND_AND:
	  cur_prec=(PREC_AND_AND<<32|IC_AND_AND);
	  last_consumes_right=1;
	  break;case TK_XOR_XOR:
	  cur_prec=(PREC_XOR_XOR<<32|IC_XOR_XOR);
	  last_consumes_right=1;
	  break;case TK_OR_OR:
	  cur_prec=(PREC_OR_OR<<32|IC_OR_OR);
	  last_consumes_right=1;
	  break;case TK_SUB_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_SUB_EQU);
	  last_consumes_right=1;
	  break;case TK_ADD_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_ADD_EQU);
	  last_consumes_right=1;
	  break;case TK_XOR_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_XOR_EQU);
	  last_consumes_right=1;
	  break;case TK_OR_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_OR_EQU);
	  last_consumes_right=1;
	  break;case TK_AND_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_AND_EQU);
	  last_consumes_right=1;
	  break;case TK_DIV_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_DIV_EQU);
	  last_consumes_right=1;
	  break;case TK_MOD_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_MOD_EQU);
	  last_consumes_right=1;
	  break;case TK_MUL_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_MUL_EQU);
	  last_consumes_right=1;
	  break;case TK_SHL_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_SHL_EQU);
	  last_consumes_right=1;
	  break;case TK_SHR_EQU:
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_SHR_EQU);
	  last_consumes_right=1;
	  break;case '=':
	  cur_prec=((PREC_ASSIGN|ASSOCF_RIGHT)<<32|IC_ASSIGN);
	  last_consumes_right=1;
      end:
        PushOpers(cc,cur_prec);
        flags&=~PEF_ADDR_OF;
        rpn=CAlloc(sizeof CRPN);
        if(cc->lock_cnt) rpn->ic_flags|=ICF_LOCK;
        rpn->type=cur_prec.u32[0];
        PrsPush(cc->ps,cur_prec);
        PrsPush2(cc->ps,rpn);
        Lex(cc);
//After getting an operator,we are no longer accepting function calls for now
        flags&=~PEF_FUN_BEFORE;
        break;case TK_DEREFERENCE: //->,not *
      case '.':
        PrsMembers(cc);
        next=cc->coc.coc_head.next;
        if(next->ic_class->type&HTT_FUN) {
	  flags|=PEF_FUN_BEFORE;
        } else
	  flags&=~PEF_FUN_BEFORE;
        break;case '(':
        tok=Lex(cc);
        next=cc->coc.coc_head.next;
        if((flags&PEF_FUN_BEFORE)&&next->ic_class->type&HTT_FUN) {
	  last_consumes_right=0;
	  if(tok==TK_IDENT&&cc->hash_entry) {
//Check for typecast 
	    if(cc->hash_entry->type&(HTT_CLASS|HTT_INTERNAL_TYPE)) {
//
	      // Heres the Donald Trump deal. If we have a (implicit) function call before a typecast 
	      //  we will first call the function,it makes no sense to typecast a function
	      //
	      ic_fun=cc->ps->stk3[cc->ps->ptr3](CRPN*)->ic_class;
	      if(!ic_fun->ptr_stars_cnt) {
	        arity=ImplicitFunCall(cc,cc->coc.coc_head.next(CRPN*)->ic_class);
	        RPNAddNew(
		      cc,
		      IC_CALL_INDIRECT,
		      arity+1,
		      ic_fun->return_class
		      );
	      }
typecast:
	      tmpc=cc->hash_entry;
//TODO release tmpad
	      Lex(cc);
	      mode=PRS0_TYPECAST|PRS1_NULL;
	      tmpc=PrsType(cc,&tmpc,&mode,NULL,NULL,&ic_fun,NULL,&tmpad,0);
	      if(ic_fun) //ic_funs are extensions of CHashClass
	        tmpc=ic_fun;
	      rpn=RPNAddNew(cc,IC_HOLYC_TYPECAST,tmpc,tmpc);
	      rpn->ic_dim=tmpad.next;
	      if(cc->token!=')') {
	        LexExcept(cc,"Expected a ')'.");
	      } else
	        Lex(cc);
	      goto end_paren;
	    }
	  }
	  ic_fun=AssignRawTypeToNode(cc->coc.coc_head.next);
	  if(ic_fun->ptr_stars_cnt)
	    ic_fun=DerefType(ic_fun);
	  if(!(ic_fun->type&HTT_FUN)) {
	    LexExcept(cc,"Invalid type to call at ");
	  }
	  mlst=ic_fun->member_lst_and_root;
	  arity=0;
	  for(idx=0;idx!=ic_fun->arg_cnt;idx++) {
	    if(mlst->flags&MLF_DOT_DOT_DOT) goto vargs;
	    if(cc->token==',') {
	      if(mlst->flags&MLF_LASTCLASS) {
	        while(last_c->ptr_stars_cnt)
		  last_c=DerefType(last_c);
	        misc=COCMiscNew(cc,CMT_STR_CONST);
	        cc->flags|=CCF_HAS_MISC_DATA;
	        misc->str=StrNew(last_c->str);
	        misc->st_len=StrLen(last_c->str)+1;
	        RPNAddNew(
		      cc,
		      IC_STR_CONST,
		      misc,
		      last_c=cmp.internal_types[RT_U8]+1
		      );
	        arity++;
	      } else if(mlst->flags&(MLF_DFT_AVAILABLE|MLF_STR_DFT_AVAILABLE)) {
	        last_c=mlst->member_class;
	        if(mlst->member_class->raw_type==RT_F64) {
		  RPNAddNew(
		        cc,
		        IC_IMM_F64,
		        mlst->dft_val,
		        cmp.internal_types[RT_F64]
		        );
		  arity++;
	        } else {
		  RPNAddNew(
		        cc,
		        IC_IMM_I64,
		        mlst->dft_val,
		        cmp.internal_types[RT_I64]
		        );
		  arity++;
	        }
	      } else
	        LexExcept(cc,"Missing dft argument at:");
	      Lex(cc);
	    } else if(cc->token!=')') {
	      if(!PrsExpression(cc,,PEF_NO_COMMA))
	        LexExcept(cc,"Expected a function argument at:");
	      PrsPush3(cc->ps,cc->coc.coc_head.next);
	      arity++;
	      rpn=cc->coc.coc_head.next;
	      last_c=rpn->ic_class;
	      if(rpn->ic_class->raw_type==RT_F64&&mlst->member_class->raw_type!=RT_F64) {
	        RPNAddNew(cc,IC_TO_I64,,cmp.internal_types[RT_I64],TRUE);
	      } else if(rpn->ic_class->raw_type!=RT_F64&&mlst->member_class->raw_type==RT_F64) {
	        RPNAddNew(cc,IC_TO_F64,,cmp.internal_types[RT_F64],TRUE);
	      }
	      if(cc->token==',') {
	        Lex(cc);
	      }
	      else if(cc->token==')') {
	        goto end_call;
	      } else
	        LexExcept(cc,"Expected a ')'.");
	    } else {
end_call:
	      break;
	    }
	    mlst=mlst->next;
	  }
	  arity=ImplicitFunCall(cc,ic_fun,arity);
vargs:
	  if(Bt(&ic_fun->flags,Ff_DOT_DOT_DOT)) {
	    arity+=2;
	    argc=RPNAddNew(cc,IC_IMM_I64,0);
	    idx=0;
	    while(cc->token!=')') {
	      if(PrsExpression(cc,NULL,PEF_NO_COMMA)) {
	        PrsPush3(cc->ps,cc->coc.coc_head.next);
	        idx++;
	      }
	      if(cc->token==',')
	        Lex(cc);
	      else if(cc->token==')') {
	        break;
	      } else
	        LexExcept(cc,"Expected a ')'.");
	    }
	    argv=RPNAddNew(cc,IC_VARGS,idx);
	    argc->ic_data=idx;
	  }
	  if(cc->token!=')')
	    LexExcept(cc,"Expected a ')'.");
	  Lex(cc);
	  arity++; //Include the function in the arity
//Check for _intern functions 
	  if(!(ic_fun->flags&(1<<Ff_INTERNAL))) {
	    RPNAddNew(
		  cc,
		  IC_CALL_INDIRECT,
		  arity,
		  ic_fun->return_class
		  );
	  } else {
use_intern:
	    rpn=RPNAddNew(
		  cc,
		  IC_CALL_INDIRECT,
		  arity,
		  ic_fun->return_class
		  );
//"SWAP" out the new rpn type with the desired one
	    rpn->type=ic_fun->exe_addr; 		  
	    rpn->ic_data=0;
//next is the old dummy function,not needed as we are using an intermendiate
	    RPNDel(next);
	  }
        } else {
	  if(tok==TK_IDENT&&cc->hash_entry) {
	    if(cc->hash_entry->type&(HTT_CLASS|HTT_INTERNAL_TYPE))
	      goto typecast;
	  }
	  cur_prec=PREC_NULL<<32;
	  PrsPush(cc->ps,cur_prec);
	  PrsPush2(cc->ps,NULL);
	  last_consumes_right=1;
        }
end_paren:
        break;case ')':
        last_consumes_right=0;
//PrsPush starts at 1,and top is ->ptr
        for(idx=1;idx<=cc->ps->ptr;idx++) {
	  if(cc->ps->stk[idx].u32[1]==PREC_NULL) {
	    goto found_end_paren;
	  }
        }
//LexExcept(cc,"Unexpected ')'.");
        finish=TRUE;
        goto paren_unexp;
found_end_paren:;
        Lex(cc);
        PushOpers(cc,PREC_NULL<<32);
//PushOpers doesnt POP parens
        PrsPop(cc->ps);
        PrsPop2(cc->ps);
        next=cc->coc.coc_head.next;
        if(next!=&cc->coc.coc_head.next&&next->ic_class->type&HTT_FUN) {
	  flags|=PEF_FUN_BEFORE;
        } else
	  flags&=~PEF_FUN_BEFORE;
paren_unexp:;
        break;case '[':
        if(cc->flags&CCF_ASM_EXPRESSIONS) {
	  finish=TRUE;
	  break;
        }
        Lex(cc);
        PrsPush(cc->ps,(PREC_NULL<<32)|'[]');
        PrsPush2(cc->ps,NULL);
        last_consumes_right=1;
        break;case ']':
//PrsPush starts at 1,and top is ->ptr
        for(idx=1;idx<=cc->ps->ptr;idx++) {
	  if(cc->ps->stk[idx]==(PREC_NULL<<32)|'[]') {
	    goto found_end_square;
	  }
        }
        finish=TRUE;
        goto paren_unexp;
found_end_square:
//[] doesnt pop the (PREC_NULL<<32)|'[]'
        PushOpers(cc,PREC_NULL);
        PrsPop(cc->ps);
        PrsPop2(cc->ps);
        Lex(cc);
        next=cc->coc.coc_head.next;
        if(next==&cc->coc.coc_head.next)
	  LexExcept(cc,"Expected not empty '[]' at ");
        if(next->ic_class->raw_type==RT_F64)
	  RPNAddNew(cc,IC_TO_I64);
        RPNAddNew(cc,IC_ADD);
        RPNAddNew(cc,IC_DEREF);
        last_consumes_right=0;
        next=cc->coc.coc_head.next;
        if(next->ic_class->type&HTT_FUN) {
	  flags|=PEF_FUN_BEFORE;
        } else
	  flags&=~PEF_FUN_BEFORE;
        break;start:
        if(!last_consumes_right)
	  finish=TRUE;
        case TK_STR:
	  if(finish) break;
	  cc->flags|=CCF_HAS_MISC_DATA;
	  misc=COCMiscNew(cc,CMT_STR_CONST);
	  misc->str=LexExtStr(cc,&str_len);
	  misc->st_len=str_len;
	  RPNAddNew(cc,IC_STR_CONST,misc);
	  last_consumes_right=0;
	  break;case TK_INS_BIN:
	  if(finish) break;
	  cc->flags|=CCF_HAS_MISC_DATA;
	  misc=COCMiscNew(cc,CMT_STR_CONST);
	  misc->st_len=cc->cur_str_len;
	  misc->str=MAlloc(misc->st_len+1);
	  MemCpy(misc->str,cc->cur_str,cc->cur_str_len);
	  RPNAddNew(cc,IC_STR_CONST,misc);
	  last_consumes_right=0;
	  Lex(cc);
	  break;case TK_CHAR_CONST:
	  if(finish) break;
	  RPNAddNew(cc,IC_IMM_I64,cc->cur_i64,cmp.internal_types[RT_I64]);
	  last_consumes_right=0;
	  Lex(cc);
	  break;case TK_I64:
	  if(finish) break;
	  RPNAddNew(cc,IC_IMM_I64,cc->cur_i64,cmp.internal_types[RT_I64]);
	  last_consumes_right=0;
	  Lex(cc);
	  break;case TK_F64:
	  if(finish) break;
	  RPNAddNew(cc,IC_IMM_F64,cc->cur_f64(I64),cmp.internal_types[RT_F64]);
	  last_consumes_right=0;
	  Lex(cc);
	  break;case TK_IDENT:
	  if(finish) break;
	  last_consumes_right=0;
	  if(cc->flags&CCF_ASM_EXPRESSIONS) {
	    if(cc->local_var_entry&&cc->htc.fun) {
	      if(flags&PEF_ADDR_OF) {
	        PrsPop(cc->ps);
	        RPNDel(PrsPop2(cc->ps));
	      } else
	        LexExcept(cc,"Expected a '&' at ");
	      cc->local_var_entry->reg=REG_NONE;
	      tmpex=CAlloc(sizeof(CHashExport));
	      tmpex->str=MStrPrint("%s.%s",cc->htc.fun->str,cc->cur_str);
	      tmpex->type=HTT_EXPORT_SYS_SYM | HTF_UNRESOLVED;
	      goto ins_label;
	    } else if(cc->hash_entry&&cc->hash_entry->type&HTT_CLASS) {
//MOV RAX, CTask.addr[RBX]
	      PrsOffsetOf(cc);
	      break;
	    } else if(!cc->hash_entry) {
new:
	      tmpex=CAlloc(sizeof(CHashExport));
	      tmpex->str=StrNew(cc->cur_str);
	      if (*tmpex->str=='@' && tmpex->str[1]=='@') {
	        tmpex->type=HTT_EXPORT_SYS_SYM | HTF_UNRESOLVED | HTF_LOCAL;
//
	        //Nroot here,I turn local labels into scope.@@local
	        //
	        Free(tmpex->str);
	        tmpex->str=MStrPrint("%d.%s",cc->asm_local_scope,cc->cur_str);
//HashAdd(tmpex,cc->htc.local_hash_table);
	        cc->flags|=CCF_LOCAL;
	        goto ins_label;
	      } else {
	        tmpex->type=HTT_EXPORT_SYS_SYM | HTF_UNRESOLVED;
ins_label:
	        tmpc=HashFind(tmpex->str,cc->htc.glbl_hash_table,HTT_EXPORT_SYS_SYM);
	        if (tmpc)
		  HashAddAfter(tmpex,tmpc,cc->htc.glbl_hash_table);
	        else
		  HashAdd(tmpex,cc->htc.glbl_hash_table);
	      }
	      cc->flags|=CCF_UNRESOLVED;
	      rpn=RPNAddNew(
		    cc,
		    IC_ADDR_IMPORT,
		    CodeMiscHashNew(cc,tmpex),
		    cmp.internal_types[RT_I64],
		    );
	      Lex(cc);
	      tmpauh=CAlloc(sizeof CAsmUndefHash);
	      tmpauh->hash=tmpex;
	      tmpauh->next=cc->asm_undef_hash;
	      cc->asm_undef_hash=tmpauh;
	      goto fin_ident;
	    } else if(cc->hash_entry->type&HTT_EXPORT_SYS_SYM&&cc->hash_entry->type&HTF_RESOLVE) {
	      rpn=RPNAddNew(
		    cc,
		    IC_ADDR_IMPORT,
		    CodeMiscHashNew(cc,cc->hash_entry),
		    cmp.internal_types[RT_I64],
		    );
	      Lex(cc);
	      goto fin_ident;
	    } else if(cc->hash_entry->type&HTT_EXPORT_SYS_SYM) {
	      goto new;
	    }
	  }
	  if(cc->local_var_entry) {
	    if(cc->local_var_entry->flags&MLF_STATIC) {
	      misc=COCMiscNew(cc,CMT_HASH_ENTRY); //->h will be filled in AIWNIOS_CodeGen.HC
	      misc->offset=cc->local_var_entry;
	      misc->addr=INVALID_PTR;
	      misc->flags|=CMF_STATIC;
	      rpn=RPNAddNew(cc,IC_STATIC,
		    misc,
		    cc->local_var_entry->member_class
		    );
	    } else {
	      rpn=RPNAddNew(cc,IC_FRAME,
		    cc->local_var_entry->offset,
		    cc->local_var_entry->member_class
		    );
	    }
	    if(cc->local_var_entry->flags&MLF_FUN) {
//TempleOS stores Function class in ->fun_ptr for some reason(when MLF_FUN is set) 
	      rpn->ic_class=cc->local_var_entry->fun_ptr;
	    }
	    rpn->ic_dim=cc->local_var_entry->dim.next;
	    Lex(cc);
	  } else if(cc->hash_entry) {
	    if(cc->hash_entry->type&HTT_OPCODE||cc->hash_entry->type&HTT_REG) {
	      finish=TRUE;
	      goto fin_ident;
	    }
	    if(cc->hash_entry->type&HTT_FUN) {
	      ic_fun=cc->hash_entry;
	      if(Bt(&ic_fun->flags,Ff_INTERNAL)) {
	        rpn=RPNAddNew(
		      cc,
		      IC_IMM_I64,
		      cc->hash_entry(CHashFun*)->exe_addr,
		      cc->hash_entry,
		      );
	      } else if(Bt(&cc->hash_entry(CHashFun*)->flags,Cf_EXTERN)||cc->flags&CCF_AOT_COMPILE) {
		if(cc->flags&CCF_AOT_COMPILE) {
	          rpn=RPNAddNew(
		      cc,
		      IC_ADDR_IMPORT,
		      CodeMiscHashNew(cc,cc->hash_entry),
		      cc->hash_entry,
		      );
		      #ifdef TARGET_RISCV
		      rpn->ic_data(CCodeMisc*)->flags|=CMF_SHORT_ADDR;
		      #endif
		} else {
		  rpn=RPNAddNew(
		      cc,
		      IC_IMM_I64,
		      &cc->hash_entry(CHashFun*)->exe_addr,
		      cmp.internal_types[RT_I64]
		  );
	          rpn=RPNAddNew(
		      cc,
		      IC_DEREF,
		      0,
		      cc->hash_entry,
		      );
		}
	      } else
	        rpn=RPNAddNew(
		      cc,
		      IC_IMM_I64,
		      cc->hash_entry(CHashFun*)->exe_addr,
		      cc->hash_entry,
		      );
	      Lex(cc);
	      next=rpn;
	      if(cc->ps->ptr&&cc->ps->stk[cc->ps->ptr].u32[0]==IC_ADDR) {
	        if(ic_fun->flags&(1<<Ff_INTERNAL)) {
//Account for &Fs->task_flags
		  if(cc->token=='('||cc->token=='.'||cc->token==TK_DEREFERENCE) {
//All is good,not '(' so continue as usal
		    if(cc->token!='(')
		      goto implicit_call;
		  } else
		    LexExcept(cc,"Can't get address of internal function at ");
	        } else {
//Continue as usual
		  //Dont derefernce to function pointer if it is a call or "&Call()->member"
		  if(cc->token=='('||cc->token=='.'||cc->token==TK_DEREFERENCE) {
		    if(cc->token!='(')
		      goto implicit_call;
		  } else {
		    rpn->ic_class=AddrOfType(rpn->ic_class); //Emulate a pointer star
		    PrsPop(cc->ps);
		    Free(PrsPop2(cc->ps)); //Free node
		  }
	        }
	      } else if(cc->token!='(') {
//Is an implicit function call
	        implicit_call:
	        arity=ImplicitFunCall(cc,ic_fun,0);
	        if(ic_fun->flags&(1<<Ff_DOT_DOT_DOT)) {
		  RPNAddNew(cc,IC_IMM_I64,0); //Add argc,argv not needed
		  arity++;
	        }
	        arity++;//Include function
	        if(Bt(&ic_fun->flags,Ff_INTERNAL)) {
		  goto use_intern;
	        } else
		  RPNAddNew(
		        cc,
		        IC_CALL_INDIRECT,
		        arity,
		        ic_fun->return_class
		        );
	        goto fin_ident;
	      }
	    } else if(cc->hash_entry->type&HTT_GLBL_VAR) {
	      if(cc->hash_entry(CHashGlblVar*)->data_addr&&!(cc->flags&CCF_AOT_COMPILE)) {
	        rpn=RPNAddNew(
		      cc,
		      IC_IMM_I64,
		      cc->hash_entry(CHashGlblVar*)->data_addr,
		      cmp.internal_types[RT_I64],
		      );
	      } else if(cc->flags&CCF_AOT_COMPILE) {
	        rpn=RPNAddNew(
		      cc,
		      IC_ADDR_IMPORT,
		      CodeMiscHashNew(cc,cc->hash_entry),
		      cc->hash_entry(CHashGlblVar*)->var_class,
		      );
	      } else {
		  rpn=RPNAddNew(
		      cc,
		      IC_IMM_I64,
		      &cc->hash_entry(CHashGlblVar*)->data_addr,
		      cmp.internal_types[RT_I64]
		  );
	          rpn=RPNAddNew(
		      cc,
		      IC_DEREF,
		      0,
		      cmp.internal_types[RT_I64],
		      );
	      }
	      if(cc->hash_entry(CHashGlblVar*)->fun_ptr) {
	        rpn=RPNAddNew(
		      cc,
		      IC_DEREF,
		      0,
		      cc->hash_entry(CHashGlblVar*)->fun_ptr,
		      );
	      } else {
	        rpn=RPNAddNew(
		      cc,
		      IC_DEREF,
		      0,
		      cc->hash_entry(CHashGlblVar*)->var_class,
		      );
	      }
	      rpn->ic_dim=cc->hash_entry(CHashGlblVar*)->dim.next;
	      Lex(cc);
	    } else if(cc->hash_entry->type&HTT_KEYWORD) {
	      switch(cc->hash_entry(CHashGeneric *)->user_data0) {
		  break;case KW_SIZEOF:
		  idx=0;
		  while (Lex(cc)=='(')
		    idx++;
		  PrsSizeOf(cc);
		  while (idx--) {
		    if (cc->token!=')')
		      LexExcept(cc,"Missing ')' at ");
		    Lex(cc);
		  }
		  break;case KW_OFFSET:
		  idx=0;
		  while (Lex(cc)=='(')
		    idx++;
		  PrsOffsetOf(cc);
		  while (idx--) {
		    if (cc->token!=')')
		      LexExcept(cc,"Missing ')' at ");
		    Lex(cc);
		  }
		  break;default:
		  LexExcept(cc,"Unexpected keyword at: ");
	      }
	    } else {
	      LexExcept(cc,"Unknown symbol at ");
	    }
	  } else
	    LexExcept(cc,"Unknown symbol at ");
	  rpn=cc->coc.coc_head.next;
	  if(ic_dim=rpn->ic_dim) {
	    RPNAddNew(cc,IC_ADDR,,,TRUE);
//IC_ADDR of IC_DEREF will cancel each other out,so re-assign the dim
	    rpn=cc->coc.coc_head.next;
	    rpn->ic_dim=ic_dim;
	  }
fin_ident:
	  next=cc->coc.coc_head.next;
	  if(next!=&cc->coc.coc_head&&next->ic_class->type&HTT_FUN) {
	    flags|=PEF_FUN_BEFORE;
	  } else
	    flags&=~PEF_FUN_BEFORE;
fin_ident_final:;
	  break;end:
        flags&=~PEF_ADDR_OF;
        break;default:
        goto fin;
    }
  }
fin:
  PushOpers(cc,PREC_NULL<<32,TRUE);
  if(!_ps)
    Free(cc->ps);
  cc->ps=old;
  return cc->coc.coc_head.next!=orig_next;
}

U0 ParserTest() {
  CCmpCtrl *cctrl=CmpCtrlNew(
    "1+3/4+5 ",
    CCF_DONT_FREE_BUF,
    "TEST.HC"
  );
  Lex(cctrl);
  QueInit(&cctrl->coc.coc_head);
  PrsExpression(cctrl,NULL,0);
  DumpRPN(cctrl->coc.coc_head.next);
  cctrl=CmpCtrlNew(
    "(1+(3))/(4+5) ",
    CCF_DONT_FREE_BUF,
    "TEST.HC"
  );
  Lex(cctrl);
  QueInit(&cctrl->coc.coc_head);
  PrsExpression(cctrl,NULL,0);
  "FIN2\n";
  DumpRPN(cctrl->coc.coc_head.next);
  cctrl=CmpCtrlNew(
    "(1+2)(I64i*)[3+4] ",
    CCF_DONT_FREE_BUF,
    "TEST.HC"
  );
  Lex(cctrl);
  QueInit(&cctrl->coc.coc_head);
  PrsExpression(cctrl,NULL,0);
  DumpRPN(cctrl->coc.coc_head.next);  
}

U8 *LexExpression2Bin(CCmpCtrl *cc,I64 *_type=NULL)
{//Compile cc expression. You call the code.
  U8 *res;
  I64 size;
  CHashFun *old_fun;
  Bool old_trace=LBtr(&cc->flags,CCf_PASS_TRACE_PRESENT);
  COCPush(cc);
  COCInit(cc);
  if (PrsExpression(cc,NULL,PEF_NO_COMMA)) {
    RPNAddNew(cc,IC_RET,0,AssignRawTypeToNode(cc->coc.coc_head.next),FALSE);
    old_fun=cc->htc.fun; //Changed by nroot
    cc->htc.fun=NULL;
    res=AiwniosCompile(cc);
    cc->htc.fun=old_fun;
    if(_type) *_type=AssignRawTypeToNode(cc->coc.coc_head.next)->raw_type; 
  } else
    res=NULL;
  COCPop(cc);
  if(old_trace)
    LBts(&cc->flags,CCf_PASS_TRACE_PRESENT);
  else
    LBtr(&cc->flags,CCf_PASS_TRACE_PRESENT);
  return res;
}

Bool IsLexExpression2Bin(CCmpCtrl *cc,U8 **_machine_code)
{//Compile cc expression to bin. Return err status.
  return !!(*_machine_code=LexExpression2Bin(cc));
}

I64 LexExpressionI64(CCmpCtrl *cc)
{//Compile cc expression, forcing to I64 and eval.
  I64 (*machine_code)();
  I64 res,type;
  if (machine_code=LexExpression2Bin(cc,&type)) {
    res=(*machine_code)();
    Free(machine_code);
    if (type==RT_F64)
      res=ToI64(res(F64));
  } else
    res=0;
  return res;
}

F64 LexExpressionF64(CCmpCtrl *cc)
{//Compile cc expression, forcing to F64 and eval.
  I64 (*machine_code)();
  I64 res,type;
  if (machine_code=LexExpression2Bin(cc,&type)) {
    res=(*machine_code)();
    Free(machine_code);
    if (type!=RT_F64)
      res(F64)=ToF64(res);
  } else
    res=0;
  return res(F64);
}

I64 LexExpression(CCmpCtrl *cc)
{//Compile cc expression and eval.  Might be I64 or F64.
  I64 (*machine_code)();
  I64 res;
  if (machine_code=LexExpression2Bin(cc)) {
    res=(*machine_code)();
    Free(machine_code);
  } else
    res=0;
  return res;
}

CRPN *RPNNext(CRPN *rpn) {
  I64 i,to;
  switch(intermediate_code_table[rpn->type].arg_cnt) {
    break;case IS_0_ARG:
      return rpn->next;
    break;case IS_1_ARG:
      return RPNNext(rpn->next);
    break;case IS_2_ARG:
      return RPNNext(RPNNext(rpn->next));
    break;case IS_V_ARG:
    to=rpn->length;
    rpn=rpn->next;
    for(i=0;i!=to;i++)
      rpn=RPNNext(rpn);
    return rpn;
  }
}
CRPN *RPNArgN(CRPN *rpn,I64 arg) {
  I64 i;
  rpn=rpn->next;
  for(i=0;i!=arg;i++) {
    rpn=RPNNext(rpn);
  }
  return rpn;
}
