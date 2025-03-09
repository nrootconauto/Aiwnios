U8 *LexStmt2Bin(CCmpCtrl *cc,I64 *_type,I64 cmp_flags=0)
{//Compile one cc stmt to bin code.
  I64 size,i,j,k,*res=INVALID_PTR;
  CCodeCtrl *tmpcbh;
  CRPN *rpn;
  LBtr(&cc->flags,CCf_PASS_TRACE_PRESENT);
  if (cc->aot_depth==2)
    COCPush(cc);
  COCInit(cc);
  if (!PrsStmt(cc,,,cmp_flags)) {
    if (cc->coc.coc_head.next!=&cc->coc.coc_head) {
      rpn=cc->coc.coc_head.next;
      switch(rpn->type) {
        case IC_TO_I64:
        case IC_TO_F64:
        case IC_HOLYC_TYPECAST:
        case IC_ABS_ADDR:
        case IC_ADDR_IMPORT:
        case IC_STR_CONST:
        case IC_IMM_I64:
        case IC_IMM_F64:
        case IC_DEREF:
        case IC_UNARY_MINUS:
        case IC_NOT:
        case IC_COM:
        case IC_SHL:
        case IC__MM:
        case IC__PP:
        case IC_MM_:
        case IC_PP_:
        case IC_SHR:
        case IC_SHL_CONST:
        case IC_SHR_CONST:
        case IC_POWER:
        case IC_DIV:
        case IC_MUL:
        case IC_ADD:
        case IC_SUB:
        case IC_XOR:
        case IC_OR:
        case IC_AND:
        case IC_MOD:
        case IC_EQU_EQU:
        case IC_LESS:
        case IC_NOT_EQU:
        case IC_GREATER_EQU:
        case IC_GREATER:
        case IC_LESS_EQU:
        case IC_AND_AND:
        case IC_OR_OR:
        case IC_XOR_XOR:
        case IC_ASSIGN:
        case IC_ASSIGN_MM:
        case IC_ASSIGN_PP:
        case IC_SHL_EQU:
        case IC_SHR_EQU:
        case IC_MUL_EQU:
        case IC_DIV_EQU:
        case IC_MOD_EQU:
        case IC_AND_EQU:
        case IC_OR_EQU:
        case IC_ADD_EQU:
        case IC_XOR_EQU:
        case IC_SUB_EQU:
        case IC_CALL:
        case IC_CALL_INDIRECT:
        case IC_CALL_INDIRECT2:
        case IC_CALL_IMPORT:
        case IC_CALL_EXTERN:
        case IC_COMMA:
        case IC_TO_BOOL:
	case IC_SQR:
	  RPNAddNew(cc,IC_RET,0,AssignRawTypeToNode(rpn),FALSE);
	  break;default:
	  RPNAddNew(cc,IC_IMM_I64,0,cmp.internal_types[RT_I64],FALSE);
	  RPNAddNew(cc,IC_RET,0,cmp.internal_types[RT_I64],FALSE);
      }
      if(cc->flags&CCF_AOT_COMPILE)
//ARM likes 4byte align for instrcutions
        while(cc->aotc->rip%4)
	  AOTStoreCodeU8(cc,0);
      if (res=COCCompile(cc,&size,NULL,_type)) {
        if (cc->flags&CCF_AOT_COMPILE) {
	  j=cc->aotc->rip;
	  k=(size+7)>>3;
	  for (i=0;i<k;i++)
	    AOTStoreCodeU64(cc,res[i]);
	  Free(res);
	  res=j;
        }
      }
    } //TODO: else del misc?
  } else //TODO: too dangerous to del Misc?
    QueDel(&cc->coc.coc_head.next);
  if (cc->aot_depth==2) {
    tmpcbh=COCPopNoFree(cc);
    COCAppend(cc,tmpcbh);
  }
  return res;
}

CAOT *CmpJoin(CCmpCtrl *cc,I64 cmp_flags,U8 *map_name=NULL,U8 mapfile_drv_let=0)
{
  CAOTCtrl *aotc,*old_aot=cc->aotc;
  I64 i,j,l;
  U8 *buf;
  CAOTBinBlk *tmpbin;
  CAOTImportExport *tmpie;
  Bool okay=TRUE;
  CLexHashTableContext *htc=MAlloc(sizeof(CLexHashTableContext));
  CAOT *res=CAlloc(sizeof(CAOT)),*parent;
  if (parent=cc->aot) {
    res->parent_aot=parent;
    QueIns(res,parent->last);
  } else
    QueInit(res);
  cc->aot=res;

  res->next_ie=res->last_ie=&res->next_ie;
  cc->aotc=aotc=CAlloc(sizeof(CAOTCtrl));
  cc->aot_depth++;

  aotc->bin=CAlloc(sizeof(CAOTBinBlk));
  aotc->max_align_bits=0;
  aotc->org=INVALID_PTR;

  MemCpy(htc,&cc->htc,sizeof(CLexHashTableContext));
  if (cc->htc.fun)
    cc->htc.glbl_hash_table=HashTableNew(128);
  else
    cc->htc.glbl_hash_table=HashTableNew(1024);
  if (cc->flags&CCF_AOT_COMPILE) {
    cc->htc.define_hash_table=cc->htc.glbl_hash_table;
    if (cc->aot_depth<=1)
      cc->htc.glbl_hash_table->next=cmp.asm_hash;
    else
      cc->htc.glbl_hash_table->next=htc->glbl_hash_table;
  } else
    cc->htc.glbl_hash_table->next=Fs->hash_table;
  cc->htc.hash_table_lst=cc->htc.local_hash_table=HashTableNew(16);
  cc->htc.local_hash_table->next=cc->htc.glbl_hash_table;
  cc->htc.local_var_lst=cc->htc.fun; //HolyC local vars
  cc->htc.fun=NULL;
  try {
    if (cmp_flags&CMPF_LEX_FIRST)
      Lex(cc);
    if (!(cmp_flags&CMPF_ONE_ASM_INS))
      cmp_flags|=CMPF_PRS_SEMICOLON;
    if (cc->flags&CCF_AOT_COMPILE) {
      while (cc->token!=TK_EOF) {
	buf=LexStmt2Bin(cc,NULL,cmp_flags);
  if (buf!=INVALID_PTR) {
	  tmpie=CAlloc(sizeof(CAOTImportExport));
	  tmpie->type=IET_MAIN;
	  tmpie->rip=buf;
	  QueIns(tmpie,res->last_ie);
	}
	if (cmp_flags&CMPF_ASM_BLK)
	  break;
      }
    } else
      PrsStmt(cc,,,cmp_flags);
    AOTGlblsResolve(cc,res);
  } catch {
    if (Fs->except_ch=='Compiler' && !(cmp_flags&CMPF_ASM_BLK)) {
      PutExcept; //Added by nroot
      LexPutPos(cc);
      Fs->catch_except=TRUE;
    }
    okay=FALSE;
  }
  if (!okay) {
    if (cc->error_cnt<1)
      cc->error_cnt=1;
    cc->aot=res->parent_aot;
    Free(res);
    LinkedLstDel(aotc->bin);
    res=NULL;
  } else {
    if (map_name)
      DbgInfoWrite(cc->htc.glbl_hash_table,map_name);
    FillInShortCalls(cc);
    HashTableDel(cc->htc.local_hash_table);
    HashTableDel(cc->htc.glbl_hash_table);

    if (!aotc->num_bin_U8s)
      res->buf=NULL;
    else {
      if (cc->flags&CCF_AOT_COMPILE)
	res->buf=MAlloc(aotc->num_bin_U8s);
      else {
	if (aotc->org==INVALID_PTR)
	  res->buf=MAlloc(aotc->num_bin_U8s,Fs->code_heap);
	else
	  res->buf=aotc->org;
      }
      res->aot_U8s=aotc->num_bin_U8s;
      tmpbin=aotc->bin;
      j=0;
      l=aotc->num_bin_U8s;
      while (tmpbin) {
	i=l;
	if (i>AOT_BIN_BLK_SIZE)
	  i=AOT_BIN_BLK_SIZE;
	MemCpy(res->buf+j,tmpbin->body,i);
	j+=i;
	l-=i;
	tmpbin=tmpbin->next;
      }
    }
    LinkedLstDel(aotc->bin);
    res->abss=aotc->abss;
    res->heap_glbls=aotc->heap_glbls;
    res->max_align_bits=aotc->max_align_bits;
    res->org=aotc->org;
  }
  cc->aot=parent;
  MemCpy(&cc->htc,htc,sizeof(CLexHashTableContext));
  Free(htc);
  Free(aotc);
  cc->aotc=old_aot;
  cc->aot_depth--;
  return res;
}

CAOT *CmpBuf(U8 *buf,U8 *map_name=NULL,
  I64 *error_cnt=NULL, I64 *warning_cnt=NULL,U8 mapfile_drv_let=0)
{
  CCmpCtrl *cc;
  CAOT *res=NULL;
  cc=CmpCtrlNew(buf,CCF_DONT_FREE_BUF);
  cc->flags|=CCF_AOT_COMPILE;
  #ifdef TARGET_AARCH64
  //For MacOS RWX protections. 
  cc->opts|=1<<OPTf_GLBLS_ON_DATA_HEAP;
  #endif
  #ifdef TARGET_X86
  //For OpenBSD
  cc->opts|=1<<OPTf_GLBLS_ON_DATA_HEAP;
  #endif
  QueIns(cc,Fs->last_cc);
  res=CmpJoin(cc,CMPF_LEX_FIRST,map_name,mapfile_drv_let);
  if (error_cnt)   *error_cnt=cc->error_cnt;
  if (warning_cnt) *warning_cnt=cc->warning_cnt;
  QueRem(cc);
  if (res)
    CmpCtrlDel(cc);
  return res;
}

I64 Cmp(U8 *filename,U8 *map_name=NULL,U8 *out_name=NULL,U8 mapfile_drv_let=0)
{//AOT Compile HC or PRJ file a and output BIN file. Returns err_cnt.
  U8 *ptr,*fbuf=NULL,*fbuf2=NULL,*fbuf3=NULL,
	*patch_table=MAlloc(0x800000);
  CAOT *tmpaot;
  I64 i,cnt,size=0,error_cnt=0,warning_cnt=0,aot_U8s=0;
  CBinFile *bfh;
  CAOTImportExport *tmpie,*tmpie1;
  CAOTAbsAddr *tmpa,*tmpa1;
  CAOTHeapGlblRef *tmphgr,*tmphgr1;
  CAOTHeapGlbl *tmphg,*tmphg1;

  fbuf=ExtDft(filename,"PRJ.Z");
  fbuf2=MStrPrint("#include \"%s\"",fbuf);
  if (map_name)
    fbuf3=ExtDft(map_name,"MAP.Z");

  if (tmpaot=CmpBuf(fbuf2,fbuf3,&error_cnt,&warning_cnt,mapfile_drv_let)) {
    aot_U8s=tmpaot->aot_U8s;
    ptr=patch_table;
//See $LK,"Load",A="MN:Load"$()
    cnt=0;
    tmpa=tmpaot->abss;
    while (tmpa) {
      if (!(tmpa->type&IEF_IMM_NOT_REL))
	cnt++;
      tmpa=tmpa->next;
    }
    if (cnt) {
      *ptr++=IET_ABS_ADDR;
      *ptr(U32 *)++=cnt;
      *ptr++=0;
      tmpa=tmpaot->abss;
      while (tmpa) {
	tmpa1=tmpa->next;
	if (!(tmpa->type&IEF_IMM_NOT_REL))
	  *ptr(U32 *)++ =tmpa->rip;
	Free(tmpa);
	tmpa=tmpa1;
      }
    }
    tmphg=tmpaot->heap_glbls;
    while (tmphg) {
      tmphg1=tmphg->next;
      cnt=0;
      tmphgr=tmphg->references;
      while (tmphgr) {
	cnt++;
	tmphgr=tmphgr->next;
      }
      if (cnt) {
	*ptr++=IET_DATA_HEAP;
	*ptr(U32 *)++=cnt;
	if (tmphg->str) {
	  i=StrLen(tmphg->str);
	  MemCpy(ptr,tmphg->str,i+1);
	  Free(tmphg->str);
	  ptr+=i+1;
	} else
	  *ptr++=0;
	*ptr(I64 *)++=tmphg->size;
	if(!tmphg->data)
	  tmphg->data=CAlloc(tmphg->size);
	MemCpy(ptr,tmphg->data,tmphg->size);
	ptr+=tmphg->size;
	Free(tmphg->data);
	tmphgr=tmphg->references;
	while (tmphgr) {
	  tmphgr1=tmphgr->next;
	  *ptr(U32 *)++=tmphgr->rip;
	  *ptr(U32 *)++=tmphgr->offset; //Added by nroot
	  Free(tmphgr);
	  tmphgr=tmphgr1;
	}
      }
      Free(tmphg);
      tmphg=tmphg1;
    }

    //Do exports first
    tmpie=tmpaot->next_ie;
    while (tmpie!=&tmpaot->next_ie) {
      tmpie1=tmpie->next;
      if (!tmpie->type || IET_REL32_EXPORT<=tmpie->type<=IET_IMM64_EXPORT) {
	QueRem(tmpie);
	*ptr++=tmpie->type;
	*ptr(U32 *)++=tmpie->rip;
	if (tmpie->str) {
	  i=StrLen(tmpie->str);
	  MemCpy(ptr,tmpie->str,i+1);
	  Free(tmpie->str);
	  ptr+=i+1;
	} else
	  *ptr++=0;
	Free(tmpie->src_link);
	Free(tmpie);
      }
      tmpie=tmpie1;
    }

    //Do imports second
    tmpie=tmpaot->next_ie;
    while (tmpie!=&tmpaot->next_ie) {
      tmpie1=tmpie->next;
      QueRem(tmpie);
      *ptr++=tmpie->type;
      if (tmpie->aot)
	tmpie->rip+=tmpie->aot->rip2;
      *ptr(U32 *)++=tmpie->rip;
      switch(tmpie->type) {
      case IET_REL_I0 ... IET_REL_RISCV:
		*ptr(I32 *)++=tmpie->offset;
      }
      if (tmpie->str) {
	i=StrLen(tmpie->str);
	MemCpy(ptr,tmpie->str,i+1);
	Free(tmpie->str);
	ptr+=i+1;
      } else
	*ptr++=0;
      Free(tmpie->src_link);
      Free(tmpie);
      tmpie=tmpie1;
    }

    *ptr++=IET_END;
    MemSet(ptr,0,16);
    i=ptr-patch_table;
//Needs 16 ALIGN
    size=(sizeof(CBinFile)+aot_U8s+i+15)+16;
    bfh=MAlloc(size);
    bfh->jmp=0xEB+256*(sizeof(CBinFile)-2);
    bfh->reserved=0;
    bfh->org=tmpaot->org;
    bfh->module_align_bits=tmpaot->max_align_bits;
    bfh->patch_table_offset=sizeof(CBinFile)+aot_U8s;
    bfh->file_size=size;
#ifdef TARGET_X86
    bfh->bin_signature='X86';
    if(HOST_ABI=='OpenBSD')
      bfh->bin_signature='OX86';
#endif
#ifdef TARGET_AARCH64
    bfh->bin_signature='ARM';
#endif
#ifdef TARGET_RISCV
    bfh->bin_signature='RV64';
#endif
    MemCpy(bfh(U8 *)+sizeof(CBinFile),tmpaot->buf,aot_U8s);
    MemCpy(bfh(U8 *)+sizeof(CBinFile)+aot_U8s,patch_table,
	  size-aot_U8s-sizeof(CBinFile));
	Free(fbuf2);
    if (out_name)
      fbuf2=ExtDft(out_name,"BIN.Z");
    else
      fbuf2=ExtChg(fbuf,"BIN.Z");
    FileWrite(fbuf2,bfh,size);
    Free(bfh);
    Free(tmpaot->buf);
    QueDel(tmpaot);
    Free(tmpaot);
  }
  Free(patch_table);
  Free(fbuf);
  Free(fbuf2);
  Free(fbuf3);
  Print("Errs:%d Warns:%d Code:%X Size:%X\n",
	error_cnt,warning_cnt,aot_U8s,size);
  return error_cnt;
}

I64 ExePutS(U8 *buf,U8 *filename=NULL,
	I64 ccf_flags=0,CLexHashTableContext *htc=NULL)
{//JIT Compile and execute text from a puts("").
  I64 res;
  Bool okay=TRUE;
  CCmpCtrl *cc;
  if (!filename)
    filename=blkdev.tmp_filename;
  cc=CmpCtrlNew(buf,ccf_flags|CCF_DONT_FREE_BUF,filename);
  if (Fs->last_cc!=&(Fs->next_cc)) {
    cc->opts=Fs->last_cc->opts;
    if (htc) {
      cc->flags=cc->flags &~CCF_ASM_EXPRESSIONS |
	    htc->old_flags&CCF_ASM_EXPRESSIONS;
      MemCpy(&cc->htc,htc,sizeof(CLexHashTableContext));
    }
  }
  QueIns(cc,Fs->last_cc);
  try {
    Lex(cc);
    res=ExeCmdLine(cc);
  } catch {
    if (Fs->except_ch=='Compiler' || Fs->except_ch=='Break') {
      Fs->catch_except=TRUE;
      okay=FALSE;
      res=0;
    }
  }
  QueRem(cc);
  if (okay)
    CmpCtrlDel(cc); //TODO: can crash
  return res;
}

I64 ExePrint(U8 *fmt,...)
{//JIT Compile and execute text from a printf().
  I64 res;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  res=ExePutS(buf);
  Free(buf);
  return res;
}

I64 ExeFile(U8 *name,I64 ccf_flags=0)
{//JIT Compile and execute a file.
  I64 res;
  U8 *name2=ExtDft(name,"HC.Z"),
	*st=MStrPrint("#include \"%s\";",name2);
  res=ExePutS(st,name,ccf_flags);
  Free(st);
  Free(name2);
  return res;
}

I64 RunFile(U8 *name,I64 ccf_flags=0,...)
{//$LK,"ExeFile",A="MN:ExeFile"$() with args using $LK,"LastFun",A="MN:LastFun"$().
  ExeFile(name,ccf_flags);
  return 0;
  //return LastFun(argc,argv); TODO RESTORE
}

I64 ExePutS2(U8 *buf,U8 *filename=NULL,I64 ccf_flags=0)
{//throws exceptions
  I64 res;
  CCmpCtrl *cc;
  if (!filename)
    filename=blkdev.tmp_filename;
  cc=CmpCtrlNew(buf,ccf_flags|CCF_DONT_FREE_BUF,filename);
  if (Fs->last_cc!=&(Fs->next_cc))
    cc->opts=Fs->last_cc->opts;
  QueIns(cc,Fs->last_cc);
  Lex(cc);
  res=ExeCmdLine(cc);
  QueRem(cc);
  CmpCtrlDel(cc);
  return res;
}

I64 ExePrint2(U8 *fmt,...)
{//throws exceptions
  I64 res;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  res=ExePutS2(buf);
  Free(buf);
  return res;
}

I64 ExeFile2(U8 *name,I64 ccf_flags=0)
{//throws exceptions
  I64 res;
  U8 *name2=ExtDft(name,"HC.Z"),*st=MStrPrint("#include \"%s\";",name2);
  res=ExePutS2(st,name,ccf_flags);
  Free(st);
  Free(name2);
  return res;
}

I64 RunFile2(U8 *name,I64 ccf_flags=0,...)
{//$LK,"ExeFile2",A="MN:ExeFile2"$() with args using $LK,"LastFun",A="MN:LastFun"$(). throws exceptions.
  ExeFile2(name,ccf_flags);
  return 0;
  // TODO RESTORE
  //return LastFun(argc,argv);
}

I64 StreamExePrint(U8 *fmt,...)
{//Causes value from stream to be used in an #exe{} block.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  I64 res=0;
  CLexHashTableContext *htc;
  CCmpCtrl *cc=Fs->last_cc;
  if (cc==&(Fs->next_cc))
    PrintErr("Not Compiling\n");
  else {
    if (!(cc->flags&CCF_EXE_BLK))
      LexExcept(cc,"StreamExePrint only allowed in AOT compiled #exe{} mode.");
    if (htc=cc->htc.next)
      res=ExePutS(buf,,,htc);
  }
  Free(buf);
  return res;
}

//Added by nroot
U0 FillInShortCalls(CCmpCtrl *cc) {
  CAOTImportExport *tmpaotie,*tmpaotie1;
  CHashFun *fun;
  I64 i,ptr,low12;
  for(tmpaotie=cc->aot->next_ie;tmpaotie!=&cc->aot->next_ie;tmpaotie=tmpaotie1) {
    tmpaotie1=tmpaotie->next;
    if(tmpaotie->short_jmp) {		  
      fun=HashSingleTableFind(tmpaotie->str,cc->htc.glbl_hash_table,HTT_FUN|HTT_GLBL_VAR);
      if(fun&&fun->type&HTT_FUN) {
		if(fun->type&HTF_RESOLVE) {
		  #ifdef TARGET_RISCV
		  ptr=fun->exe_addr+tmpaotie->offset;
riscv_patch:
		  i=ptr-tmpaotie->rip;
	      low12=i-(i&~(1<<12-1));
	      //In riscv_backend,I use 1/2/etc to tell what kind of reloction it is,Use mask (0xfff) to presereve the low12 bits(used by opcode)
	      AOTAndBytesU8At(cc,tmpaotie->rip,0xff,0x0f,0,0);
		  if(-(1<<11)<=low12<(1<<11-1)) {/*Chekc for bit 12 being set*/
		    //AUIPC imm20
		    i=((ptr-tmpaotie->rip)>>12)<<12;
            AOTOrBytesU8At(cc,tmpaotie->rip,i.u8[0],i.u8[1],i.u8[2],i.u8[3]);
            //JALR imm12
		    i=low12<<20;
			AOTOrBytesU8At(cc,tmpaotie->rip(I64)+4,i.u8[0],i.u8[1],i.u8[2],i.u8[3]);
          } else {
			i=ptr-tmpaotie->rip;
			i=(i>>12+1)<<12;
            AOTOrBytesU8At(cc,tmpaotie->rip,i.u8[0],i.u8[1],i.u8[2],i.u8[3]);
		    i=low12<<20;
            AOTOrBytesU8At(cc,tmpaotie->rip(I64)+4,i.u8[0],i.u8[1],i.u8[2],i.u8[3]);
	      }
		  #endif
		  #ifdef TARGET_AARCH64
          i=((fun->exe_addr-tmpaotie->rip)>>2)&(1<<26-1);
          AOTOrBytesU8At(cc,tmpaotie->rip,i.u8[0],i.u8[1],i.u8[2],i.u8[3]);
          #endif
          #ifdef TARGET_X86
          i=fun->exe_addr-(tmpaotie->rip+4)+tmpaotie->offset;
          AOTStoreCodeU8At(cc,tmpaotie->rip,i.u8[0],i.u8[1],i.u8[2],i.u8[3]);
          #endif
          QueRem(tmpaotie);
          Free(tmpaotie->str);
          Free(tmpaotie);
        }
      } else if(fun&&fun->type&HTT_GLBL_VAR) {
			#ifdef TARGET_X86
		if(fun->type&HTF_EXPORT) {
			i=fun(CHashGlblVar*)->data_addr_rip-(tmpaotie->rip+4)+tmpaotie->offset;
			AOTStoreCodeU8At(cc,tmpaotie->rip,i.u8[0],i.u8[1],i.u8[2],i.u8[3]);
} else goto skip;
			#endif
			#ifdef TARGET_RISCV
		//if(fun->type&HTF_EXPORT) {
			if(fun(CHashGlblVar*)->data_addr_rip) {
				ptr=fun(CHashGlblVar*)->data_addr_rip+tmpaotie->offset;
				goto riscv_patch;
			} else goto skip;
//}
			#endif
			QueRem(tmpaotie);
			Free(tmpaotie->str);
			Free(tmpaotie);
skip:;
      }
    }
  }
}

