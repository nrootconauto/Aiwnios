U0 AOTGlblsResolve(CCmpCtrl *cc,CAOT *tmpaot)
{
  CAOTCtrl *aotc=cc->aotc;
  CHashFun *tmpf;
  CAsmUnresolvedRef *tmpu=aotc->glbl_unresolved,*tmpu1;
  I64 i,j;
  CAOTImportExport *tmpie,*tmpie1;
  CAsmUndefHash *tmpauh;
  CHashExport *tmpex;
  U8 *label;
  Bool undefined;
  CExternUsage *tmpeu,*tmpeu8;

  while (tmpu) {
    label=NULL;
    undefined=FALSE;
    tmpu1=tmpu->next;
    tmpauh=tmpu->asm_undef_hash;
    while (tmpauh) {
      if (tmpauh->hash->type & HTF_UNRESOLVED) {
	tmpex=tmpauh->hash;
	if (tmpex->type & HTT_EXPORT_SYS_SYM&& tmpex->type & HTF_UNRESOLVED &&
	      !(tmpex->type & HTF_IMPORT) &&
	      (tmpf=HashFind(tmpex->str,cc->htc.hash_table_lst,HTT_FUN)) &&
	      !Bt(&tmpf->flags,Cf_EXTERN)) {
	  tmpex->val=tmpf->exe_addr;
	  tmpex->type&=~HTF_UNRESOLVED;
	  label=tmpauh->hash->str;
	} else {
	  if (!(tmpex->type & HTF_IMPORT)) {
	    if (cc->htc.local_var_lst) {
	      tmpex->type|=HTF_GOTO_LABEL;
	      tmpex->use_cnt++;
	    } else {
	      PrintErr("Undefined sym at line:%04d %s\n",
		    tmpu->line_num,tmpex->str);
	      LexExcept(cc);
	    }
	  } else if (undefined) {
	    PrintErr("Two imports in same expression "
		  "not allowed at line:%04d %s\n",
		  tmpu->line_num,tmpex->str);
	    LexExcept(cc);
	  }
	  undefined=TRUE;
	}
      } else
	label=tmpauh->hash->str;
      tmpauh=tmpauh->next;
    }
    //AsmResolve(cc,tmpu,label,undefined); TODO restore ???
    Free(tmpu->machine_code);
    LinkedLstDel(tmpu->asm_undef_hash);
    Free(tmpu);
    tmpu=tmpu1;
  }

  for (i=0;i<=cc->htc.glbl_hash_table->mask;i++) {
    tmpex=cc->htc.glbl_hash_table->body[i];
    while (tmpex) {
      if (tmpex->type & (HTF_IMPORT|HTF_GOTO_LABEL)) {
	if (tmpex->use_cnt && (tmpie=tmpex->ie_lst)) {
	  if (tmpex->type&HTF_GOTO_LABEL)
	    tmpie->flags|=IEF_GOTO_LABEL;
	  if (tmpex->import_name)
	    tmpie->str=StrNew(tmpex->import_name);
	  else
	    tmpie->str=StrNew(tmpex->str);
	  do {
	    tmpie1=tmpie->next;
	    QueIns(tmpie,tmpaot->last_ie);
	  } while (tmpie=tmpie1);
	  tmpex->ie_lst=NULL;
	}
      } else if (tmpex->type & (HTF_EXPORT|HTF_RESOLVE)) {
	if (tmpex->type & HTF_UNRESOLVED) {
	  PrintErr("Undefined sym at %s\n",tmpex->str);
	  LexExcept(cc);
	}
	if (tmpex->type & HTF_RESOLVE) {
	  tmpf=tmpex;
	  tmpeu=tmpf->ext_lst;
	  while (tmpeu) {
	    tmpeu8=tmpeu->next;
	    j=tmpf->exe_addr-(tmpeu->rip+4);
	    AOTStoreCodeU8At(cc,tmpeu->rip,j.u8[0],j.u8[1],j.u8[2],j.u8[3]);
	    Free(tmpeu);
	    tmpeu=tmpeu8;
	  }
	}
	if (tmpex->type & HTF_EXPORT) {
	  tmpie=CAlloc(sizeof(CAOTImportExport));
	  tmpie->type=IET_REL32_EXPORT;
	  if (tmpex->type & HTT_FUN)
	    tmpie->rip=tmpf->exe_addr;
	  else if (tmpex->type & HTT_GLBL_VAR)
	    tmpie->rip=tmpex(CHashGlblVar *)->data_addr_rip;
	  else
	    tmpie->rip=tmpex->val;
	  tmpie->aot=cc->aot;
	  if (tmpex->type & HTF_IMM)
	    tmpie->type++;
	  tmpie->str=StrNew(tmpex->str);
	  tmpie->src_link=StrNew(tmpex->src_link);
	  QueIns(tmpie,tmpaot->last_ie);
	}
      }
      tmpex=tmpex->next;
    }
  }
}
