U0 PrsVarInit(CCmpCtrl *cc,U8 **_dst,CHashClass *tmpc,CArrayDim *tmpad,
	U8 *data_addr_rip,U8 **_base,Bool data_heap,I64 pass,CHash *to)
{
  U8 *dst=*_dst;
  I64 (*machine_code)();
  I64 i,j,r,old_flags,type,size;
  CMemberLst *tmpm;
  CRPN *rpn;
  CIntermediateCode *tmpi;
  CAOTCtrl *aotc=cc->aotc;
  CAOTAbsAddr *tmpa;
  CCodeMisc *cm;
  CAOTImportExport *tmpie;
  Bool is_str;
  	
  tmpc=OptClassFwd(tmpc);
  if (tmpm=tmpc->member_lst_and_root) {
    if (cc->token!='{')
      LexExcept(cc,"Expecting '{' at ");
    LexPopNoRestore(cc);
    LexPush(cc);
    Lex(cc);
    while (tmpm) {
      PrsVarInit2(cc,&dst,tmpm->member_class,&tmpm->dim,
	    data_addr_rip,_base,data_heap,pass,to);
      if (cc->token==',')
	Lex(cc);
      tmpm=tmpm->next;
    }
    LexPopNoRestore(cc);
    if (cc->token!='}')
      LexExcept(cc,"Missing '}' at ");
    Lex(cc);
  } else {
    /*if (tmpc->ptr_stars_cnt==1 &&
	  ((tmpc-1)->raw_type==RT_I8 || (tmpc-1)->raw_type==RT_U8) &&
	  !tmpad && cc->token==TK_STR)
      is_str=TRUE;
    else
      is_str=FALSE;
    if (cc->flags&CCF_AOT_COMPILE  && is_str && 0) {
      LexPopNoRestore(cc);
      machine_code=LexExtStr(cc,&i);
      if (pass==2) {
	tmpa=CAlloc(sizeof(CAOTAbsAddr));
	tmpa->next=aotc->abss;
	tmpa->type=AAT_ADD_U64;
	aotc->abss=tmpa;
	tmpa->rip=data_addr_rip+dst-*_base;
	*dst(I64 *)=aotc->rip;
	for (j=0;j<i;j++)
	  AOTStoreCodeU8(cc,machine_code(U8*)[j]);
      }
      Free(machine_code);
    } else */{
      old_flags=cc->flags;
      cc->flags=CCF_NO_ABSS | cc->flags &
	    ~(CCF_AOT_COMPILE|CCF_HAS_MISC_DATA|CCF_NOT_CONST);
      machine_code=LexExpression2Bin(cc,&type);
      if (old_flags&CCF_AOT_COMPILE
	    /*cc->flags&CCF_NOT_CONST && */ //Const-folding is done in the C end
	//Nroot forces all AOT globals to go on the heap for Apple hardedned runtime
	    /*!Bt(&cc->opts,OPTf_GLBLS_ON_DATA_HEAP)*/
	) {
	cc->flags=cc->flags&~CCF_NO_ABSS|CCF_AOT_COMPILE;
	Free(machine_code);
	if (pass==2) {
	   MemSet(dst,0,tmpc->size);
	   LexPopRestore(cc);
	   Lex(cc);
	   COCPush(cc);
	   COCInit(cc);
	   if(to) {
	     cm=CodeMiscHashNew(cc,to);
	     cm->offset=dst-*_base;
	     RPNAddNew(cc,IC_ADDR_IMPORT,cm,tmpc+1,FALSE);
	     RPNAddNew(cc,IC_DEREF,0,tmpc,FALSE);
           }
          if(cc->token==',') //DUMB FIX by nroot
            Lex(cc);
	  if (!PrsExpression(cc,NULL,PEF_NO_COMMA)) //Changed by nroot
	    throw('Compiler');
	  if(to) {RPNAddNew(cc,IC_ASSIGN,,tmpc,FALSE);
	  RPNAddNew(cc,IC_RET,0,tmpc,FALSE);
	  while(cc->aotc->rip&3)
	    AOTStoreCodeU8(cc,0);
	  if (machine_code=COCCompile(cc,&size,NULL,NULL)) {
	    tmpie=CAlloc(sizeof(CAOTImportExport));
	    tmpie->type=IET_MAIN;
	    QueIns(tmpie,cc->aot->last_ie);
      tmpie->rip=cc->aotc->rip;
	    for (i=0;i<size;i++) {
	      AOTStoreCodeU8(cc,machine_code(U8*)[i]);
	    }
	    Free(machine_code);
	    }
	  }
	  COCPop(cc);
	} else
	  LexPopNoRestore(cc);
      } else {
	LexPopNoRestore(cc);
	if (!machine_code)
	  throw('Compiler');
	r=(*machine_code)();
	if (!(cc->flags & CCF_HAS_MISC_DATA)||pass==1)
	  Free(machine_code);

	if (type==RT_F64 &&
	      tmpc->raw_type!=RT_F64)
	  r=r(F64);
	else if (type!=RT_F64 &&
	      tmpc->raw_type==RT_F64)
	  r(F64)=r;
	MemCpy(dst,&r,tmpc->size);
      }
    }
    dst+=tmpc->size;
    cc->flags=cc->flags&
	  ~CCF_NO_ABSS|old_flags&(CCF_HAS_MISC_DATA|CCF_AOT_COMPILE);
  }
  *_dst=dst;
}

class CVI2
{
  CVI2 *next,*last;
  U0 base;
};

U0 PrsVarInit2(CCmpCtrl *cc,U8 **_dst,CHashClass *tmpc,
	CArrayDim *tmpad,U8 *data_addr_rip,U8 **_base,Bool data_heap,I64 pass,CHash *to)
{
  I64 i,j,cnt;
  U8 *st,*_b;
  CVI2 head,*tmpvi,*tmpvi1;
  CArrayDim *tmpad1;
  tmpc=OptClassFwd(tmpc);
  if (tmpad1=tmpad->next) {
    if (!tmpc->ptr_stars_cnt &&
	  (tmpc->raw_type==RT_I8 || tmpc->raw_type==RT_U8) &&
	  cc->token==TK_STR) {
      LexPopNoRestore(cc);
      st=LexExtStr(cc,&i);
      if (tmpad1->cnt<0) {//[]
	tmpad1->cnt=i;
	tmpad->total_cnt=i*tmpad1->total_cnt;
	Free(*_base);
	if (data_heap)
	  *_base=MAlloc(i);
	else
	  *_base=MAlloc(i /*,Fs->code_heap*/ );
      MemCpy(*_base,st,i);
	*_dst=*_base+i;
      } else {
	MemCpy(*_dst,st,tmpad1->cnt);
	*_dst+=tmpad1->cnt;
      }
      Free(st);
      LexPush(cc);
    } else {
      if (cc->token=='{') {
	LexPopNoRestore(cc);
	LexPush(cc);
	Lex(cc);
      }
      if (tmpad1->cnt<0) {//[]
	QueInit(&head);
	cnt=0;
	while (cc->token!='}') {
	  tmpvi=MAlloc(offset(CVI2.base)+tmpad1->total_cnt*tmpc->size);
	  _b=&tmpvi->base;
	  PrsVarInit2(cc,&_b,tmpc,tmpad1,data_addr_rip,_base,data_heap,pass,to);
	  QueIns(tmpvi,head.last);
	  if (cc->token==',')
	    Lex(cc);
	  cnt++;
	}
	Lex(cc); //skip '}'
	tmpad1->cnt=cnt;
	tmpad->total_cnt=cnt*tmpad1->total_cnt;
	j=tmpad1->total_cnt*tmpc->size;
	i=cnt*j;
	Free(*_base);
	if (data_heap)
	  *_base=_b=MAlloc(i);
	else
	  *_base=_b=MAlloc(i /*,Fs->code_heap*/);
	tmpvi=head.next;
	while (tmpvi!=&head) {
	  tmpvi1=tmpvi->next;
	  MemCpy(_b,&tmpvi->base,j);
	  _b+=j;
	  Free(tmpvi);
	  tmpvi=tmpvi1;
	}
	*_dst=_b;
      } else {
	for (i=0;i<tmpad1->cnt;i++) {
	  PrsVarInit2(cc,_dst,tmpc,tmpad1,data_addr_rip,_base,data_heap,pass,to);
	  if (tmpad1->cnt>1 && cc->token==',')
	    Lex(cc);
	}
	if (cc->token=='}')
	  Lex(cc);
      }
    }
  } else {
    PrsVarInit(cc,_dst,tmpc,tmpad1,data_addr_rip,_base,data_heap,pass,to);
    LexPush(cc);
  }
}

U0 PrsGlblInit(CCmpCtrl *cc,CHashGlblVar *tmpg,I64 pass)
{
  U8 *dst=tmpg->data_addr;
  PrsVarInit2(cc,&dst,tmpg->var_class,&tmpg->dim,
	tmpg->data_addr_rip,&tmpg->data_addr,
	Bt(&cc->opts,OPTf_GLBLS_ON_DATA_HEAP)||
	Bt(&cc->flags,CCf_AOT_COMPILE),pass,tmpg);
}

U0 PrsStaticInit(CCmpCtrl *cc,CMemberLst *tmpm,I64 pass)
{
  U8 *dst=tmpm->static_data;
  CHashClass *tmpc=tmpm->member_class;
  CHashFun *old_fun=cc->htc.fun; //Added by nroot,DONT ALLOW COCCompile to pick up function args
  I64 (*machine_code)();
  I64 i,size;
  CAOTImportExport *tmpie;

  cc->htc.fun=NULL;

  if (cc->flags&CCF_AOT_COMPILE && pass==2) {
    COCPush(cc);
    COCInit(cc);
  }
  PrsVarInit2(cc,&dst,tmpc,&tmpm->dim,tmpm->static_data_rip,
	&tmpm->static_data,Bt(&cc->flags,CCf_AOT_COMPILE),pass,NULL);
  if (cc->flags&CCF_AOT_COMPILE && pass==2) {
    if (cc->coc.coc_head.next!=&cc->coc.coc_head) {
      RPNAddNew(cc,IC_RET,,AssignRawTypeToNode(cc->coc.coc_head.next),FALSE);
      //ARM likes 4byte align for instrcutions
      while(cc->aotc->rip%4)
        AOTStoreCodeU8(cc,0);
      if (machine_code=COCCompile(cc,&size,NULL,NULL)) {
	if (pass==2) {
	  tmpie=CAlloc(sizeof(CAOTImportExport));
	  tmpie->type=IET_MAIN;
	  tmpie->rip=cc->aotc->rip;
	  QueIns(tmpie,cc->aot->last_ie);
    for (i=0;i<size;i++)
	    AOTStoreCodeU8(cc,machine_code(U8*)[i]);
	}
	Free(machine_code);
      }
    } //TODO: else del misc?
    COCPop(cc);
  }
  
  cc->htc.fun=old_fun;
}

U0 PrsArrayDims(CCmpCtrl *cc,I64 mode,CArrayDim *dim)
{//dim->next!=0 for array
  CArrayDim *tmpad,*tmpad1;
  I64 j;
  dim->next=NULL;
  dim->cnt=0;
  dim->total_cnt=1;
  tmpad1=&dim->next;
  if (cc->token=='[') {
    if (mode.u8[1]==PRS1B_FUN_ARG)
      LexExcept(cc,"No arrays in fun args at ");
    do {
      if (Lex(cc)==']' && !dim->next)
	j=0;
      else {
	if ((j=LexExpressionI64(cc))<0)
	  LexExcept(cc,"Invalid array size at ");
      }
      tmpad=MAlloc(sizeof(CArrayDim));
      tmpad->next=NULL;
      tmpad1=&dim;
      do {
      //
      //Nroot Fix,getting addres of &dim is an 8byte ptr,SO DONT WRITE INTO IT IT
      // [RBP]
      // [RBP-8] &dim(writing a member may corrupt RBP)
      //
      if(&dim!=tmpad1)
        tmpad1->total_cnt*=j;
	if (!tmpad1->next) {
	  tmpad1->next=tmpad;
	  break;
	}
	tmpad1=tmpad1->next;
      } while (tmpad1);
      tmpad1=tmpad;
      tmpad->cnt=j;
      tmpad->total_cnt=1;
      if (cc->token!=']')
	LexExcept(cc,"Missing ']' at ");
    } while (Lex(cc)=='[');
  }
}

CHashClass *PrsType(CCmpCtrl *cc,CHashClass **_tmpc1,
  I64 *_mode,CMemberLst *tmpm,U8 **_ident,CHashFun **_fun_ptr,
  CHashExport **_tmpex,CArrayDim *tmpad,I64 fsp_flags)
{
  I64 k,ptr_stars_cnt,mode=*_mode;
  CHashClass *tmpc1=*_tmpc1,*tmpc2;
  CHashFun *fun_ptr=NULL;
  CHashExport *tmpex=NULL;

  pt_start:
  if (!tmpc1 || !(tmpc1->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
    LexExcept(cc,"Invalid class at ");

  ptr_stars_cnt=0;
  while (cc->token=='*') {
    if (mode.u8[1]) {
      LexPopNoRestore(cc);
      LexPush(cc);
    }
    Lex(cc);
    tmpc1++;
    if (++ptr_stars_cnt>PTR_STARS_NUM)
      LexExcept(cc,"Too many *'s at ");
  }

  k=PrsKeyWord(cc);
  if (k==KW_UNION || k==KW_CLASS) {
    Lex(cc);
    tmpc2=PrsClass(cc,k,fsp_flags,mode&255==PRS0_EXTERN);
    tmpc2->fwd_class=tmpc1;
    tmpc1=tmpc2;
    if (_tmpc1) *_tmpc1=tmpc1;
    mode=PRS0_NULL|PRS1_NULL;
    goto pt_start;
  }

  if (cc->token=='(') {
    if (Lex(cc)!='*')
      LexExcept(cc,"Expecting '*' at ");
    ptr_stars_cnt=1; //fun_ptr
    while (Lex(cc)=='*')
      ptr_stars_cnt++; //fun_ptr
    if (ptr_stars_cnt>PTR_STARS_NUM)
      LexExcept(cc,"Too many *'s at ");
  } else
    ptr_stars_cnt=-1; //fun_ptr

  if (_ident) {
    if (cc->token==TK_IDENT) {
      tmpex=cc->hash_entry;
      *_ident=cc->cur_str;
      cc->cur_str=NULL;
      Lex(cc);
    } else {
      if (!mode.u8[1])
	*_ident=NULL;
      else if (cc->token==',' || cc->token==';' || cc->token==')') {
	tmpex=NULL;
	*_ident=StrNew("_anon_");
	tmpm->flags|=MLF_NO_UNUSED_WARN;
      } else
	LexExcept(cc,"Expecting identifier at ");
    }
  }

  if (ptr_stars_cnt>=0) { //fun_ptr
    if (cc->token!=')')
      LexExcept(cc,"Missing ')' at ");
    if (Lex(cc)!='(')
      LexExcept(cc,"Expecting '(' at ");
    fun_ptr=PrsFunJoin(cc,tmpc1,NULL,fsp_flags)+ptr_stars_cnt;
    tmpc1=cmp.internal_types[RT_PTR]+ptr_stars_cnt;
  }
  PrsArrayDims(cc,mode,tmpad);

  tmpc2=OptClassFwd(tmpc1);
  if (tmpc2->ptr_stars_cnt) {
    tmpc2-=tmpc2->ptr_stars_cnt;
    if (tmpc2->type&HTT_INTERNAL_TYPE && !tmpc2->size)
      LexWarn(cc,"use \"U8 *\" instead of \"U0 *\" at ");
  }

  if (_mode)	*_mode=mode;
  if (_fun_ptr)	*_fun_ptr=fun_ptr;
  if (_tmpex)	*_tmpex=tmpex;
  return tmpc1;
}

U0 PrsDotDotDot(CCmpCtrl *cc,CHashFun *tmpf,I64 _reg)
{
  CMemberLst *tmpm;

  LBts(&tmpf->flags,Ff_DOT_DOT_DOT);

  Lex(cc);
  tmpm=MemberLstNew(_reg);
  tmpm->flags=MLF_DOT_DOT_DOT;
  tmpm->member_class=cmp.internal_types[RT_I64];
  tmpm->str=StrNew("argc");
  tmpm->offset=tmpf->size;
  tmpm->size=8;
  tmpf->size+=8;
  tmpm->dim.total_cnt=1; //Added by nroot
  MemberAdd(cc,tmpm,tmpf,PRS1B_FUN_ARG);

  tmpm=MemberLstNew(_reg);
  tmpm->flags=MLF_DOT_DOT_DOT;
  tmpm->member_class=cmp.internal_types[RT_I64]; //Changed by nroot,argv is a pointer,not an array
  tmpm->member_class++;
  tmpm->str=StrNew("argv");
  tmpm->offset=tmpf->size;
  tmpm->size=1; //Close enough(1 count of an I8* ???)
  tmpf->size+=8;//Close enough
  tmpm->dim.total_cnt=1; //Added by nroot
  MemberAdd(cc,tmpm,tmpf,PRS1B_FUN_ARG);

  if (cc->token==')')
    Lex(cc);
}

U0 PrsVarLst(CCmpCtrl *cc,CHashClass *tmpc,I64 mode,I64 union_base=0)
{
  I64 i,k,old_flags=cc->flags,old_flags2,type,_reg;
  CHashClass *tmpc1,*tmpc2;
  CHash *tmph;
  CMemberLst *tmpm;
  CMemberLstMeta *tmp_meta;
  U8 *(*machine_code)();
  Bool undef_array_size,first;
  cc->flags|=CCF_DONT_MAKE_RES;
  if (mode.u8[1]==PRS1B_CLASS)
    cc->flags|=CCF_CLASS_DOL_OFFSET;
  if ((mode.u8[1]!=PRS1B_LOCAL_VAR && mode.u8[1]!=PRS1B_STATIC_LOCAL_VAR ||
	mode&PRSF_UNION) && (cc->token=='(' || cc->token=='{'))
    Lex(cc);
  while (TRUE) {
    if (mode&PRSF_UNION)
      cc->class_dol_offset=union_base;
    else
      cc->class_dol_offset=tmpc->size;
    while (cc->token==';')
      Lex(cc);
    while (cc->token=='\d') {
      if (Lex(cc)!='=') //skip $$
	LexExcept(cc,"Expecting '=' at ");
      Lex(cc); //skip =
      cc->class_dol_offset=LexExpression(cc);
      if (-cc->class_dol_offset>tmpc->neg_offset)
	tmpc->neg_offset=-cc->class_dol_offset;
      if (mode&PRSF_UNION)
	union_base=cc->class_dol_offset;
      else
	tmpc->size=cc->class_dol_offset;
      if (cc->token!=';')
	LexExcept(cc,"Missing ';' at");
      Lex(cc); //skip ;
    }
    if (cc->token==')' || cc->token=='}') {
      Lex(cc);
      goto pvl_done;
    }
    _reg=REG_UNDEF;
pvl_restart1:
    switch (PrsKeyWord(cc)) {
      case KW_REG:
	_reg=REG_ALLOC;
	if (Lex(cc)==TK_IDENT) {
	  k=DefineMatch(cc->cur_str,"ST_U64_REGS");
	  if (k>=0) {
	    _reg=k;
	    Lex(cc);
	  }
	}
	goto pvl_restart1;
      case KW_NOREG:
	_reg=REG_NONE;
	Lex(cc);
	goto pvl_restart1;
    }

    if (cc->token==TK_ELLIPSIS && mode.u8[1]==PRS1B_FUN_ARG) {
      PrsDotDotDot(cc,tmpc,_reg);
      goto pvl_done;
    }
    if (cc->token==TK_IDENT)
      tmph=cc->hash_entry;
    else
      tmph=NULL;
    if (!tmph)
      LexExcept(cc,"Expecting type at ");
    k=PrsKeyWord(cc);
    if (k==KW_UNION) {
      Lex(cc);
      PrsVarLst(cc,tmpc,mode|PRSF_UNION,tmpc->size);
    } else {
      if (!(tmph->type & (HTT_CLASS|HTT_INTERNAL_TYPE)))
	LexExcept(cc,"Expecting type at ");
      first=TRUE;
pvl_restart2:
      tmpc1=tmph;
      LexPush(cc);
      Lex(cc); //skip type or ','
      tmpm=MemberLstNew(_reg);
      _reg=REG_UNDEF;
      if (mode.u8[1]==PRS1B_STATIC_LOCAL_VAR) {
	tmpm->flags|=MLF_STATIC;
	tmpm->reg=REG_NONE;
      }
      if (mode.u8[1]==PRS1B_FUN_ARG || mode.u8[1]==PRS1B_LOCAL_VAR) {
pvl_restart3:
	switch (PrsKeyWord(cc)) {
	  case KW_REG:
	    tmpm->reg=REG_ALLOC;
	    LexPopNoRestore(cc);
	    LexPush(cc);
	    if (Lex(cc)==TK_IDENT) {
	      k=DefineMatch(cc->cur_str,"ST_U64_REGS");
	      if (k>=0) {
		tmpm->reg=k;
		LexPopNoRestore(cc);
		LexPush(cc);
		Lex(cc);
	      }
	    }
	    goto pvl_restart3;
	  case KW_NOREG:
	    tmpm->reg=REG_NONE;
	    LexPopNoRestore(cc);
	    LexPush(cc);
	    Lex(cc);
	    goto pvl_restart3;
	}
      }
      tmpm->member_class=PrsType(cc,&tmpc1,&mode,tmpm,&tmpm->str,
	    &tmpm->fun_ptr,NULL,&tmpm->dim,0);
      if (tmpm->fun_ptr)
	tmpm->flags|=MLF_FUN;
      if (first)
	MemberAdd(cc,tmpm,tmpc,mode.u8[1]);
      else
	MemberAdd(cc,tmpm,tmpc,PRS1B_NULL);
      tmpc->member_cnt++;

      tmpc2=tmpm->member_class;
      i=tmpc2->size*tmpm->dim.total_cnt;
      switch (mode.u8[1]) {
	case PRS1B_STATIC_LOCAL_VAR:
	  if (i<0) {
	    i=0;
	    undef_array_size=TRUE;
	  } else
	    undef_array_size=FALSE;
	  if (mode&PRSF_UNION)
	    LexExcept(cc,"Static unions are not implemented ");
	  k=(i+7)&~7;
	  if (cc->flags&CCF_AOT_COMPILE)
	    tmpm->static_data=MAlloc(k);
	  else
	    tmpm->static_data=MAlloc(k /*,Fs->code_heap*/);
	  if (cc->flags&CCF_AOT_COMPILE)	{ //TODO gutted by nroot
	    tmpm->static_data_rip=cc->aotc->rip;
	    k>>=3;
	    while (k--)
	      AOTStoreCodeU64(cc,0);
	  }/* else TODO RESTORE
	    if (sys_var_init_flag)
	      MemSet(tmpm->static_data,sys_var_init_val,k);
    */
    else
      MemSet(tmpm->static_data,0,k);
	  LexPopNoRestore(cc);
	  if (cc->token=='=') {
	    cc->flags=cc->flags&
		  ~CCF_DONT_MAKE_RES|old_flags&CCF_DONT_MAKE_RES;
	    if (undef_array_size) {
	      LexPush(cc);
	      LexPush(cc);
	      Lex(cc); //skip =
	      PrsStaticInit(cc,tmpm,1);
	      LexPopNoRestore(cc);
	      i=tmpc2->size*tmpm->dim.total_cnt;
	      k=(i+7)&~7;
	      if (cc->flags&CCF_AOT_COMPILE)	{
		k>>=3;
		while (k--)
		  AOTStoreCodeU64(cc,0);
	      } else
		/*if (sys_var_init_flag) TODO RESTORE
		  MemSet(tmpm->static_data,sys_var_init_val,k);*/
        MemSet(tmpm->static_data,0,k); //TODO remove
	      LexPopRestore(cc);
	    }
	    LexPush(cc);
	    Lex(cc); //skip =
	    PrsStaticInit(cc,tmpm,2);
	    LexPopNoRestore(cc);
	    if (cc->flags&CCF_AOT_COMPILE)
	      for (k=0;k<i;k++)
		AOTStoreCodeU8At(cc,tmpm->static_data_rip+k,
		      tmpm->static_data[k]);
	    tmpm->use_cnt=0;
	    cc->flags|=CCF_DONT_MAKE_RES;
	  }
	  if (cc->flags&CCF_AOT_COMPILE)
	    Free(tmpm->static_data);
	  break;
	case PRS1B_LOCAL_VAR:
	  if (mode&PRSF_UNION) {
	    if (union_base-tmpc->size<i)
	      i=union_base-i-tmpc->size;
	    else
	      i=0;
	  }
	  if (i>=8)
	    tmpc->size=(tmpc->size-i)&~7;
	  else if (i>=4)
	    tmpc->size=(tmpc->size-i)&~3;
	  else if (i>=2)
	    tmpc->size=(tmpc->size-i)&~1;
	  else
	    tmpc->size-=i;
	  tmpm->offset=tmpc->size;
	  tmpm->size=i;
	  if (cc->token=='=') {
	    cc->flags=cc->flags&~CCF_DONT_MAKE_RES|
		  old_flags&CCF_DONT_MAKE_RES;
	    LexPopRestore(cc);
	    Lex(cc);
	    if (!PrsExpression(cc,NULL,PEF_NO_COMMA))
	      throw('Compiler');
	    tmpm->use_cnt=0;
	    cc->flags|=CCF_DONT_MAKE_RES;
	  } else
	    LexPopNoRestore(cc);
	  break;
	case PRS1B_FUN_ARG:
	  if (mode&PRSF_UNION) {
	    tmpm->offset=union_base;
	    if (tmpc->size-union_base<8)
	      tmpc->size=8+union_base;
	  } else {
	    tmpm->offset=tmpc->size;
	    tmpc->size+=8;
	  }
	  tmpm->size=8;
	  if (cc->token=='=') {
	    Lex(cc);
	    if (PrsKeyWord(cc)==KW_LASTCLASS) {
	      tmpm->flags|=MLF_LASTCLASS;
	      Lex(cc);
	    } else {
	      old_flags2=cc->flags;
	      cc->flags&=~CCF_HAS_MISC_DATA;
	      machine_code=LexExpression2Bin(cc,&type);
	      if (!machine_code)
		throw('Compiler');
	      tmpm->dft_val=(*machine_code)();
	      tmpc2=OptClassFwd(tmpc2);
	      if (tmpc2->raw_type==RT_F64) {
		if (type!=RT_F64)
		  tmpm->dft_val(F64)=tmpm->dft_val;
	      } else {
		if (type==RT_F64)
		  tmpm->dft_val=tmpm->dft_val(F64);
	      }
	      if (cc->flags & CCF_HAS_MISC_DATA) {
		tmpm->dft_val=StrNew(tmpm->dft_val);
		tmpm->flags|=MLF_STR_DFT_AVAILABLE;
	      }
	      Free(machine_code);
	      cc->flags|=old_flags2&CCF_HAS_MISC_DATA;
	    }
	    tmpm->flags|=MLF_DFT_AVAILABLE;
	  }
	  LexPopNoRestore(cc);
	  break;
	case PRS1B_CLASS:
	  if (mode&PRSF_UNION) {
	    tmpm->offset=union_base;
	    if (tmpc->size-union_base<i)
	      tmpc->size=i+union_base;
	  } else {
	    tmpm->offset=tmpc->size;
	    tmpc->size+=i;
	  }
	  tmpm->size=i;
	  if (mode&PRSF_UNION)
	    cc->class_dol_offset=union_base;
	  else
	    cc->class_dol_offset=tmpc->size;

	  while (cc->token==TK_IDENT) {
	    tmp_meta=MAlloc(sizeof(CMemberLstMeta));
	    tmp_meta->next=tmpm->meta;
	    tmpm->meta=tmp_meta;
	    tmp_meta->str=cc->cur_str;
	    tmp_meta->flags=0;
	    cc->cur_str=NULL;
	    if (Lex(cc)==TK_STR) {
	      tmp_meta->user_data=LexExtStr(cc);
	      tmp_meta->flags|=MLMF_IS_STR;
	    } else
	      tmp_meta->user_data=LexExpression(cc);
	  }
	  LexPopNoRestore(cc);
	  break;
      }
      switch (cc->token) {
	case ',':
	  if (mode.u8[1]==PRS1B_FUN_ARG && !(mode&PRSF_UNION))
	    Lex(cc);
	  else {
	    first=FALSE;
	    goto pvl_restart2;
	  }
	  break;
	case ')':
	case '}':
	  Lex(cc);
	  goto pvl_done;
	case ';':
	  cc->flags=cc->flags&~CCF_DONT_MAKE_RES|
		old_flags&CCF_DONT_MAKE_RES;
	  Lex(cc);
	  cc->flags|=CCF_DONT_MAKE_RES;
	  if ((mode.u8[1]==PRS1B_LOCAL_VAR||mode.u8[1]==
		PRS1B_STATIC_LOCAL_VAR) && !(mode&PRSF_UNION))
	    goto pvl_done;
	  break;
	default:
	  LexExcept(cc,"Missing ';' at");
      }
    }
  }
pvl_done:
  cc->flags=cc->flags&~(CCF_CLASS_DOL_OFFSET|CCF_DONT_MAKE_RES)|
	old_flags&(CCF_CLASS_DOL_OFFSET|CCF_DONT_MAKE_RES);
}
