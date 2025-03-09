class CAiwniosAOTBlob {
  CQue *asm_patches;
  CQue *to_fill;
  I64 len;
  U8 *data;
  //private,used when placing ->data into it's final location
  U8 **final_start_addr;
};
class CRiscVOpcode:CHashOpcode  {
  U32 opc,f3,f7,extra;
  U64 args[4];
  U64 op_type;
  Bool is_flt_style;
};
class CAddrMode {
  U64 type;
  U64 value;
  U64 reg;
  CCodeMisc *label;
  CAsmUndefHash *local_asm_undef_hash;
  CAsmUndefHash *glbl_asm_undef_hash;
  I64 (*machine_code)();
  Bool resolved;
};
class CAsmPatch:CQue  {
  CAddrMode *args [4];
  CAddrMode *imm;
  CRiscVOpcode *opcode;
  I64 addr,sz;
};

class CToFill:CQue {
  CHashExport *h;
};
extern I64 AsmOpcode0(CAiwniosAOTBlob *blob,CRiscVOpcode *o,Bool add=FALSE);

U0 AsmResolvePatches(CCmpCtrl *cc,CQue *patches) {
  CAsmPatch *patch;
  I64 b,i;
  CAsmUndefHash *tab,*cur;
  CHashImport *h;
  CMemberLst *mlst;
  CHashExport *exp;
  CCodeMisc *cm;
  CRiscVOpcode *opc;
  Bool local;
  patch=patches->next;
  while(patch!=patches) {
    if(!patch->imm) goto nxt;
    if(!patch->imm->machine_code) goto nxt;
    if(tab=patch->imm->local_asm_undef_hash) {
      local=TRUE;
    } else {
      tab=patch->imm->glbl_asm_undef_hash;
      local=FALSE;
    }
    if(!tab) goto nxt;
    cur=tab;
    while(cur) {
      if(cc->coc.coc_next_misc)
        for(cm=cc->coc.coc_next_misc;cm!=&cc->coc.coc_next_misc;cm=cm->next) {
	  if(cm->addr2!=INVALID_PTR&&
	        cm->type==CMT_GOTO_LABEL&&cm->str&&
	        !StrCmp(cm->str,cur->hash->str)) {
	    exp=CAlloc(sizeof CHashExport);
	    exp->type=HTT_EXPORT_SYS_SYM|HTF_RESOLVE;
	    exp->str=StrNew(cm->str);
	    exp->val=cm->addr2;
	    HashAdd(exp,Fs->hash_table);
	    SysSymImportsResolve(cur->hash->str,0);
	    HashRemDel(exp,Fs->hash_table);
	    goto resolved;
	  }
        }
//See AIWNIOS_PrsExp.HC
      if(cc->htc.fun) {
	i=StrLen(cc->htc.fun->str);
        if(!StrNCmp(cur->hash->str,cc->htc.fun->str,i)) {
	  if(cur->hash->str[i]=='.'&&
		MemberFind(cur->hash->str+i+1,cc->htc.fun)) {
	    exp=CAlloc(sizeof CHashExport);
	    exp->type=HTT_EXPORT_SYS_SYM|HTF_RESOLVE;
	    exp->str=StrNew(cur->hash->str);
//Stack grows down,I should make the offsets be accordingly
	    exp->val=-MemberFind(cur->hash->str+i+1,cc->htc.fun)->offset;
	    HashAdd(exp,Fs->hash_table);
	    SysSymImportsResolve(cur->hash->str,0);
	    HashRemDel(exp,Fs->hash_table);
	    goto resolved;
	  }
        }
      }
      SysSymImportsResolve(cur->hash->str,0);
resolved:
      cur=cur->next;
    }
    patch->imm->value=(*patch->imm->machine_code)();
    Free(patch->imm->machine_code);
    patch->imm->machine_code=NULL;
    LinkedLstDel(tab);
nxt:
    patch=patch->next;
  }
}
U0 JITFixupAiwniosAOTBlob(CCmpCtrl *cc,CAiwniosAOTBlob *blob,U8 *buf) {
  CToFill *to_fill=blob->to_fill,*tmp_to_fill;
  CAsmPatch *patch,*next_patch;
  Bool rel;
  I64 tmp,sz;
  CRiscVOpcode *opc;
  tmp_to_fill=to_fill->next;
  while(tmp_to_fill!=to_fill) {
    tmp_to_fill->h->type&=~HTF_UNRESOLVED;
    tmp_to_fill->h->type|=HTF_RESOLVE;
    tmp_to_fill->h->val+=buf;

    tmp_to_fill=tmp_to_fill->next;
  }
  AsmResolvePatches(cc,blob->asm_patches);
//If our relocations are not realtive,add the ptr to result
  patch=blob->asm_patches->next;
  while(patch!=blob->asm_patches) {
    next_patch=patch->next;
    rel=FALSE;
    if(opc=patch->opcode) {
      if(!StrCmp(opc->str,"JAL"))
        rel=TRUE;
      else if(!StrCmp(opc->str,"AUIPC"))
        rel=TRUE;
      if(opc->op_type.u8[0]=='B')
        rel=TRUE;
    }
    if(patch->imm) {
      patch->imm->resolved=TRUE;
      if(rel)
	patch->imm->value-=ToI64(&buf[patch->addr]);
      buf[patch->addr](U32)=AsmOpcode0(blob,patch->opcode,patch->args);
      Free(patch->imm);
      patch->imm=NULL;
    }
    patch=next_patch;
  }
  tmp_to_fill=to_fill->next;
  while(tmp_to_fill!=to_fill) {
    if(!(tmp_to_fill->h->type&HTF_EXPORT))
      HashRemDel(tmp_to_fill->h,Fs->hash_table);
    tmp_to_fill=tmp_to_fill->next;
  }
}



U0 AiwniosAOTBlobDel(CAiwniosAOTBlob *b) {
  QueDel(b->asm_patches);
  QueDel(b->to_fill);
  Free(b->asm_patches);
  Free(b->to_fill);
  Free(b->data);
  Free(b);
}

U0 AddBytesToBuf(U8 **_buf,I64 *len,I64 cnt,U8 *bytes) {
  U8 *buf2,*buf=*_buf;
  if(MSize(buf)<*len+cnt+16) {
    buf2=MAlloc(*len+64+cnt);
    MemCpy(buf2,buf,*len);
    Free(buf);
    buf=buf2;
  }
  MemCpy(buf+*len,bytes,cnt);
  *len+=cnt;
  *_buf=buf;
}


//Copied from c/riscv_asm.c
I64 MaskN(U64 data,I64 shift,I64 width) {
  return (data&(1<<width-1))<<shift;
}
I64 RISCV_R(I64 f7, I64 s2, I64 s1, I64 f3, I64 d,
                I64 opc) {
  return MaskN(f7, 25, 7) | MaskN(s2, 20, 5) | MaskN(s1, 15, 5) |
         MaskN(f3, 12, 3) | MaskN(d, 7, 5) | MaskN(opc, 0, 7);
}
I64 RISCV_I(I64 imm, I64 s1, I64 f3, I64 d, I64 opc) {
  return MaskN(imm, 20, 12) | MaskN(s1, 15, 5) | MaskN(f3, 12, 3) |
         MaskN(d, 7, 5) | MaskN(opc, 0, 7);
}
I64 RISCV_S(I64 imm115, I64 s1, I64 f3,I64 d,
                I64 opc) {
  return MaskN(imm115 >> 5, 25, 12) | MaskN(s1, 20, 5) | MaskN(d, 15, 5) |
         MaskN(f3, 12, 3) | MaskN(imm115, 7, 5) | MaskN(opc, 0, 7);
}
I64 RISCV_B(I64 imm, I64 s1, I64 s2, I64 f3, I64 opc) {
  return MaskN(imm >> 12, 31, 1) | MaskN(imm >> 5, 25, 6) | MaskN(s2, 20, 5) |
         MaskN(s1, 15, 5) | MaskN(f3, 12, 3) | MaskN(imm >> 1, 8, 4) |
         MaskN(imm >> 11, 7, 1) | MaskN(opc, 0, 7);
}
I64 RISCV_U(I64 imm3112, I64 d, I64 opc) {
  I64 r31_12 = imm3112 >> 12;
  return MaskN(imm3112, 12, 20) | MaskN(d, 7, 5) | MaskN(opc, 0, 7);
}
I64 RISCV_J(I64 imm_wtf, I64 d, I64 opc) {
  I64 b20 = (imm_wtf >> 20) & 1;
  I64 b10_1 = MaskN(imm_wtf >> 1, 0, 10);
  I64 b11 = MaskN(imm_wtf >> 11, 0, 1);
  I64 b19_12 = MaskN(imm_wtf >> 12, 0, 19 - 12 + 1);
  imm_wtf = (b20 << 31) | (b10_1 << 21) | (b11 << (20)) | (b19_12 << 12);
  return imm_wtf | MaskN(d, 7, 5) | MaskN(opc, 0, 7);
}
I64 RISCV_FENCE(I64 fm,I64 pred,I64 succ,I64 s1,I64 f3,I64 d,I64 opc) {
  return MaskN(fm, 28, 4) | MaskN(pred, 24, 4) | MaskN(succ, 20, 4) |
         MaskN(s1, 15, 5) | MaskN(f3, 12, 3) | MaskN(d, 7, 5) | MaskN(opc, 0, 7);
}

U0 AddrModeDel(CAddrMode *md) {
  if(md->local_asm_undef_hash)
    LinkedLstDel(md->local_asm_undef_hash);
  if(md->glbl_asm_undef_hash)
    LinkedLstDel(md->glbl_asm_undef_hash);
  Free(md->machine_code);
  Free(md);
}



Bool PrsAsmImm(CCmpCtrl *cc,CAddrMode *arg)
{
  I64 old_flags=cc->flags;
  arg->local_asm_undef_hash=NULL;
  arg->glbl_asm_undef_hash=NULL;
  cc->asm_undef_hash=NULL;
  cc->abs_cnts=0;
  cc->flags&=~(CCF_UNRESOLVED+CCF_LOCAL);
  cc->flags|=CCF_ASM_EXPRESSIONS;
  if (!IsLexExpression2Bin(cc,&arg->machine_code)) {
    return FALSE;
  } else {
    if (cc->abs_cnts.externs)
      LexExcept(cc,"Extern Not Allowed at ");
    if (cc->flags & CCF_UNRESOLVED) {
      if (cc->flags & CCF_LOCAL) {
	arg->local_asm_undef_hash=cc->asm_undef_hash;
	cc->asm_undef_hash=NULL;
      } else {
	arg->glbl_asm_undef_hash=cc->asm_undef_hash;
	cc->asm_undef_hash=NULL;
      }
    } else {
      arg->resolved=TRUE;
      arg->value=(*arg->machine_code)();
      arg->glbl_asm_undef_hash=cc->asm_undef_hash;
      cc->asm_undef_hash=NULL;
      Free(arg->machine_code);
      arg->machine_code=NULL;
    }
  }
  cc->flags=old_flags;
 return TRUE;
}
I64 Name2Reg(U8 *name) {
  if(!StrCmp(name,"zero"))
    return 0;
  if(!StrCmp(name,"ra"))
    return 1;
  if(!StrCmp(name,"sp"))
    return 2;
  if(!StrCmp(name,"gp"))
    return 3;
  if(!StrCmp(name,"tp"))
    return 4;
  if(!StrCmp(name,"t0"))
    return 5;
  if(!StrCmp(name,"t1"))
    return 6;
  if(!StrCmp(name,"t2"))
    return 7;
  if(!StrCmp(name,"fp"))
    return 8;
  if(!StrCmp(name,"s0"))
    return 8;
  if(!StrCmp(name,"s1"))
    return 9;
  if(!StrCmp(name,"a0"))
    return 10;
  if(!StrCmp(name,"a1"))
    return 11;
  if(!StrCmp(name,"a2"))
    return 12;
  if(!StrCmp(name,"a3"))
    return 13;
  if(!StrCmp(name,"a4"))
    return 14;
  if(!StrCmp(name,"a5"))
    return 15;
  if(!StrCmp(name,"a6"))
    return 16;
  if(!StrCmp(name,"a7"))
    return 17;

  if(!StrCmp(name,"s2"))
    return 18;
  if(!StrCmp(name,"s3"))
    return 19;
  if(!StrCmp(name,"s4"))
    return 20;
  if(!StrCmp(name,"s5"))
    return 21;
  if(!StrCmp(name,"s6"))
    return 22;
  if(!StrCmp(name,"s7"))
    return 23;
  if(!StrCmp(name,"s8"))
    return 24;
  if(!StrCmp(name,"s9"))
    return 25;
  if(!StrCmp(name,"s10"))
    return 26;
  if(!StrCmp(name,"s11"))
    return 27;

  if(!StrCmp(name,"t3"))
    return 28;
  if(!StrCmp(name,"t4"))
   return 29;
  if(!StrCmp(name,"t5"))
    return 30;
  if(!StrCmp(name,"t6"))
    return 31;
  return -1;
}

I64 Name2FReg(U8 *name) {
  if(!StrCmp(name,"ft0"))
    return 0;
  if(!StrCmp(name,"ft1"))
    return 1;
  if(!StrCmp(name,"ft2"))
    return 2;
  if(!StrCmp(name,"ft3"))
    return 3;
  if(!StrCmp(name,"ft4"))
    return 4;
  if(!StrCmp(name,"ft5"))
    return 5;
  if(!StrCmp(name,"ft6"))
    return 6;
  if(!StrCmp(name,"ft7"))
    return 7;
  if(!StrCmp(name,"fs0"))
    return 8;
  if(!StrCmp(name,"fs1"))
    return 9;
  if(!StrCmp(name,"fa0"))
    return 10;
  if(!StrCmp(name,"fa1"))
    return 11;
  if(!StrCmp(name,"fa2"))
    return 12;
  if(!StrCmp(name,"fa3"))
    return 13;
  if(!StrCmp(name,"fa4"))
    return 14;
  if(!StrCmp(name,"fa5"))
    return 15;
  if(!StrCmp(name,"fa6"))
    return 16;
  if(!StrCmp(name,"fa7"))
    return 17;

  if(!StrCmp(name,"fs2"))
    return 18;
  if(!StrCmp(name,"fs3"))
    return 19;
  if(!StrCmp(name,"fs4"))
    return 20;
  if(!StrCmp(name,"fs5"))
    return 21;
  if(!StrCmp(name,"fs6"))
    return 22;
  if(!StrCmp(name,"fs7"))
    return 23;
  if(!StrCmp(name,"fs8"))
    return 24;
  if(!StrCmp(name,"fs9"))
    return 25;
  if(!StrCmp(name,"fs10"))
    return 26;
  if(!StrCmp(name,"fs11"))
    return 27;

  if(!StrCmp(name,"ft8"))
    return 28;
  if(!StrCmp(name,"ft9"))
   return 29;
  if(!StrCmp(name,"ft10"))
    return 30;
  if(!StrCmp(name,"ft11"))
    return 31;
  return -1;
}

CAddrMode *PrsAddrMode(CCmpCtrl *cc,Bool flt_style=FALSE) { 
  CAddrMode *am=CAlloc(sizeof CAddrMode);
  I64 r,i;
  U8 *s;
  if(cc->token==TK_IDENT) {
    if(flt_style) {
      if(0<=(r=Name2FReg(cc->cur_str))) {
        am->type='Reg';
        am->reg=r;
        Lex(cc);
        return am;
      }
    } else {
      if(0<=(r=Name2Reg(cc->cur_str))) {
        am->type='Reg';
        am->reg=r;
        Lex(cc);
        return am;
      }
      s=cc->cur_str;
      while(*s&&StrOcc("iorw",*s++));
      if(!*s) {
        am->type='Iorw';
        am->reg=(!!StrOcc(cc->cur_str,'i'))<<3|
		(!!StrOcc(cc->cur_str,'o'))<<2|
		(!!StrOcc(cc->cur_str,'r'))<<1|
		(!!StrOcc(cc->cur_str,'w'))<<0;
	Lex(cc);
	return am;
      }
    }
  }  
  if(PrsAsmImm(cc,am)) {
    am->type='Imm';
indir:
    if(cc->token=='[') {
      Lex(cc);
      if(0<=(r=Name2Reg(cc->cur_str))) {
        am->type='Indir';
        am->reg=r;
        if(Lex(cc)!=']')
	  LexExcept(cc,"Expected a ']' here.");
        Lex(cc);
      } else
        LexExcept(cc,"Expected a register here here.");
    }
    return am;
  } else if(cc->token=='[')
    goto indir;
  Free(am);
  return NULL;
}
CHashTable *riscv_opcs=adam_task->hash_table;
CRiscVOpcode *AO(U8 *name,U32 o,U64 f3,U64 f7,U64 type,U64 r2_style=0) {
  CRiscVOpcode *opc=CAlloc(sizeof CRiscVOpcode);
  opc->str=StrNew(name);
  opc->type=HTT_OPCODE;
  opc->op_type=type;
  opc->opc=o;
  opc->f3=f3;
  opc->f7=f7;
  opc->extra=r2_style;
  HashAdd(opc,riscv_opcs);
  return opc;
}
CRiscVOpcode *APause(U8 *name,U32 o) {
  CRiscVOpcode *opc=CAlloc(sizeof CRiscVOpcode);
  opc->str=StrNew(name);
  opc->type=HTT_OPCODE;
  opc->op_type='PS';
  opc->opc=o;
  HashAdd(opc,riscv_opcs);
  return opc;
}
CRiscVOpcode *AFence(U8 *name,U32 o) {
  CRiscVOpcode *opc=CAlloc(sizeof CRiscVOpcode);
  opc->str=StrNew(name);
  opc->type=HTT_OPCODE;
  opc->op_type='FE';
  opc->opc=o;
  HashAdd(opc,riscv_opcs);
  return opc;
}
CRiscVOpcode *AOf(U8 *name,U32 o,U64 f3,U64 f7,U64 type,U64 r2_style=0) {
  CRiscVOpcode *opc=AO(name,o,f3,f7,type,r2_style);
  opc->is_flt_style=TRUE;
  return opc;
}
U0 InitOpcodes()  {
  AO("LUI",0b0110111,0,0,'U');
  AO("AUIPC",0b0010111,0,0,'U');
  AO("JAL",0b1101111,0,0,'J');
  AO("JALR",0b1100111,0,0,'I');
  AO("BEQ",0b1100011,0b1,0,'B');
  AO("BNE",0b1100011,0,0,'B');
  AO("BLT",0b1100011,0b100,0,'B');
  AO("BGE",0b1100011,0b101,0,'B');
  AO("BLTU",0b1100011,0b110,0,'B');
  AO("BGEU",0b1100011,0b111,0,'B');
  AO("LB",0b0000011,0,0,'Il');
  AO("LH",0b0000011,0b001,0,'Il');
  AO("LW",0b0000011,0b010,0,'Il');
  AO("LBU",0b0000011,0b100,0,'Il');
  AO("LHU",0b0000011,0b101,0,'Il');

  AO("SB",0b0100011,0,0,'Ss');
  AO("SH",0b0100011,0b001,0,'Ss');
  AO("SW",0b0100011,0b010,0,'Ss');

  AO("ADDI",0b0010011,0,0,'I');
  AO("SLTI",0b0010011,0b010,0,'I');
  AO("SLTIU",0b0010011,0b011,0,'I');
  AO("XORI",0b0010011,0b100,0,'I');
  AO("ORI",0b0010011,0b110,0,'I');
  AO("ANDI",0b0010011,0b111,0,'I');

  AO("SLLI",0b0010011,0b001,0,'Rs');
  AO("SRLI",0b0010011,0b101,0,'Rs');
  AO("SRAI",0b0010011,0b101,0b100000,'Rs',0b010000);

  AO("ADD",0b0110011,0b000,0,'R');
  AO("SUB",0b0110011,0b000,0b100000,'R');
  AO("SLL",0b0110011,0b001,0b000000,'R');
  AO("SLT",0b0110011,0b010,0b000000,'R');
  AO("SLTU",0b0110011,0b011,0b000000,'R');
  AO("XOR",0b0110011,0b100,0b000000,'R');
  AO("SRL",0b0110011,0b101,0b000000,'R');
  AO("SRA",0b0110011,0b101,0b100000,'R');
  AO("OR",0b0110011,0b110,0b000000,'R');
  AO("AND",0b0110011,0b111,0b000000,'R');

  APause("PAUSE",0b0001111);
  AFence("FENCE",0b0001111);

//ECALL
//EBREAK

  AO("LWU",0b0000011,0b110,0,'Il');
  AO("LD",0b0000011,0b011,0,'Il');
  AO("SD",0b0100011,0b011,0,'Ss');
  AO("SLLI",0b0010011,0b001,0,'I');
  AO("SRLI",0b0010011,0b101,0,'I');
  AO("SRAI",0b0010011,0b101,0b100000,'I');
  AO("ADDIW",0b0011011,0b000,0,'I');
  AO("SLLIW",0b0011011,0b000,0,'I');
  AO("SRLIW",0b0011011,0b000,0,'I');
  AO("SRAIW",0b0011011,0b000,0b0100000,'I');
  AO("ADDW",0b0111011,0b000,0,'R');
  AO("SUBW",0b0111011,0b000,0b0100000,'R');
  AO("SLLW",0b0111011,0b001,0,'R');
  AO("SRLW",0b0111011,0b101,0,'R');
  AO("SRAW",0b0111011,0b101,0b0100000,'R');

  AO("MUL",0b0110011,0,1,'R');
  AO("MULH",0b0110011,1,1,'R');
  AO("MULHS",0b0110011,0b10,1,'R');
  AO("MULHSU",0b0110011,0b11,1,'R');
  AO("DIV",0b0110011,0b100,1,'R');
  AO("DIVU",0b0110011,0b101,1,'R');
  AO("REM",0b0110011,0b110,1,'R');
  AO("REMU",0b0110011,0b111,1,'R');
  AO("MULW",0b0110011,0,1,'R');
  AO("DIVW",0b0110011,0b100,1,'R');
  AO("DIVUW",0b0110011,0b101,1,'R');
  AO("REMW",0b0110011,0b110,1,'R');
  AO("REMUW",0b0110011,0b111,1,'R');

  AO("LRW",0b0101111,0b010,0b0001011,'LR');
  AO("SCW",0b0101111,0b010,0b0001111,'SC');
  AO("AMOSWAPW",0b0101111,0b010,0b0000111,'SC');
  AO("AMOADDW",0b0101111,0b010,0b0000011,'SC');
  AO("AMOXORW",0b0101111,0b010,0b0010011,'SC');
  AO("AMOANDW",0b0101111,0b010,0b0110011,'SC');
  AO("AMOANDW",0b0101111,0b010,0b0100011,'SC');
  AO("AMOMINW",0b0101111,0b010,0b1000011,'SC');
  AO("AMOMAXW",0b0101111,0b010,0b1010011,'SC');
  AO("AMOMINUW",0b0101111,0b010,0b1100011,'SC');
  AO("AMOMAXUW",0b0101111,0b010,0b1110011,'SC');
 
  AO("LRD",0b0101111,0b011,0b0001011,'LR');
  AO("SCD",0b0101111,0b011,0b0001111,'SC');
  AO("AMOSWAPD",0b0101111,0b011,0b0000111,'SC');
  AO("AMOADDD",0b0101111,0b011,0b0000011,'SC');
  AO("AMOXORD",0b0101111,0b011,0b0010011,'SC');
  AO("AMOANDD",0b0101111,0b011,0b0110011,'SC');
  AO("AMOORD",0b0101111,0b011,0b0100011,'SC');
  AO("AMOMIND",0b0101111,0b011,0b1000011,'SC');
  AO("AMOMAXD",0b0101111,0b011,0b1010011,'SC');
  AO("AMOMINUD",0b0101111,0b011,0b1100011,'SC');
  AO("AMOMAXUD",0b0101111,0b011,0b1110011,'SC');

//RV32F
  I64 rm=1; //Round towards Z
  AOf("FLW",0b0000111,0b010,0,'Il');
  AOf("FSW",0b0100111,0b010,0,'Ss');
  AOf("FADDS",0b1010011,rm,0b0000000,'R');
  AOf("FSUBS",0b1010011,rm,0b0000100,'R');
  AOf("FMULS",0b1010011,rm,0b0001000,'R');
  AOf("FDIVS",0b1010011,rm,0b0001100,'R');
  AOf("FSQRTS",0b1010011,rm,0b0101100,'RS');
  AOf("FSGNJS",0b1010011,0b0010000,0b001,'R');
  AOf("FSGNJNS",0b1010011,0b0010000,0b001,'R');
  AOf("FSGNJXS",0b1010011,0b0010000,0b010,'R');
  AOf("FMINS",0b1010011,0b0010100,0b000,'R');
  AOf("FMAXS",0b1010011,0b0010100,0b001,'R');
  AOf("FCVTWS",0b1010011,0b1100000,0b000,'R2',0);
  AOf("FCVTWUS",0b1010011,0b1100000,0b000,'R2',1);
  AOf("FMVXS",0b1010011,0b1110000,0b000,'R2',1);
  AOf("FEQS",0b1010011,0b1010000,0b010,'R',1);
  AOf("FLTS",0b1010011,0b1010000,0b001,'R',1);
  AOf("FLES",0b1010011,0b1010000,0b000,'R',1);
  AOf("FCLASSS",0b1010011,0b001,0b1110000,'R2',0);
  AOf("FCVTSW",0b1010011,rm,0b1100000,'R2',0b00010);
  AOf("FCVTSWU",0b1010011,rm,0b1100000,'R2',0b00011);
  AOf("FCVTSL",0b1010011,rm,0b1101000,'R2',0b00010);
  AOf("FCVTSLU",0b1010011,rm,0b1101000,'R2',0b00011);
//RV32D
  AOf("FLD",0b0000111,0b011,0,'Il');
  AOf("FSD",0b0100111,0b011,0,'Il');
  AOf("FADD",0b1010011,rm,0b0000001,'R');
  AOf("FSUB",0b1010011,rm,0b0000101,'R');
  AOf("FMUL",0b1010011,rm,0b0001001,'R');
  AOf("FDIV",0b1010011,rm,0b0001101,'R');
  AOf("FSQRT",0b1010011,rm,0b0101101,'RS');
  AOf("FSGNJ",0b1010011,0b000,0b0010001,'R');
  AOf("FSGNJN",0b1010011,0b001,0b0010001,'R');
  AOf("FSGNJX",0b1010011,0b010,0b0010001,'R');
  AOf("FMIN",0b1010011,0b000,0b0010101,'R');
  AOf("FMAX",0b1010011,0b001,0b0010101,'R');
  AOf("FCVTSD",0b1010011,rm,0b100000,'R2',1);
  AOf("FCVTDS",0b1010011,rm,0b100001,'R2',0);
  AOf("FEQ",0b1010011,0b010,0b1010001,'R',1);
  AOf("FLT",0b1010011,0b001,0b1010001,'R',1);
  AOf("FLE",0b1010011,0b000,0b1010001,'R',1);
  AOf("FCLASS",0b1010011,0b001,0b1110000,'R2',0);
  AOf("FCVTLD",0b1010011,rm,0b1100001,'R2',0b00010);
  AOf("FCVTLUD",0b1010011,rm,0b1100001,'R2',0b00011);
  AOf("FCVTDL",0b1010011,rm,0b1101001,'R2',0b00010);
  AOf("FCVTDLU",0b1010011,rm,0b1101001,'R2',0b00011);


  AOf("FMVXD",0b1010011,0,0b1110001,'R2',0b000000);
  AOf("FMVDX",0b1010011,0,0b1111001,'R2',0b000000);
  AOf("FCVTWD",0b1010011,rm,0b1100001,'R2',0b000000);
  AOf("FCVTWUD",0b1010011,rm,0b1100001,'R2',0b00001);
  AOf("FCVTDW",0b1010011,rm,0b1101001,'R2',0b00010);
  AOf("FCVTDWU",0b1010011,rm,0b1101001,'R2',0b00011);

}
InitOpcodes;
I64 AsmOpcode(CCmpCtrl *cc,CAiwniosAOTBlob *blob) {
  I64 want,argc,ret=-1;
  Bool flt_style;
  CAsmPatch *patch;
  CAddrMode *have,*poo[4];
  CRiscVOpcode *opc;
  if(cc->token!=TK_IDENT) 
    return ret;
  opc=HashFind(cc->cur_str,riscv_opcs,HTT_OPCODE);
  if(!opc)
    return ret;
  switch(opc->op_type) {
    case 'SC':
    case 'R':
      want=3;
      break;
    case 'LR':
    case 'RS':
    case 'R2':
      want=2;
      break;
    case 'Il':
      want=2;
      break;
    case 'I':
      want=3;
      break;
    case 'S':
      want=3;
      break;
    case 'Ss':
      want=2;
      break;
    case 'Rs':
      want=2;
      break;
    case 'B':
      want=3;
      break;
    case 'U':
      want=2;
      break;
    case 'J':
      want=2;
      break;
    case 'PS':
      want=0;
      break;
    case 'FE':
      want=2;
      break;
  }
  Lex(cc);
  for(argc=0;argc!=want;argc++) {
    flt_style=opc->is_flt_style;
    if(!argc) {
      if(!StrCmp("FEQ",opc->str))
	flt_style=0;
      else if(!StrCmp("FLT",opc->str))
	flt_style=0;
      else if(!StrCmp("FLE",opc->str))
	flt_style=0;
      else if(!StrCmp("FEQS",opc->str))
	flt_style=0;
      else if(!StrCmp("FLTS",opc->str))
	flt_style=0;
      else if(!StrCmp("FLES",opc->str))
	flt_style=0;
      else if(!StrCmp("FCLASS",opc->str))
	flt_style=0;
      else if(!StrCmp("FCLASSS",opc->str))
	flt_style=0;
      else if(!StrCmp("FCVTLD",opc->str))
	flt_style=0;
      else if(!StrCmp("FCVTLUD",opc->str))
	flt_style=0;
      else if(!StrCmp("FMVXD",opc->str))
	flt_style=0;
      else if(!StrCmp("FCVTWD",opc->str))
	flt_style=0;
      else if(!StrCmp("FCVTWUD",opc->str))
	flt_style=0;
    } else if(argc==1) {
      if(!StrCmp("FCVTDL",opc->str))
	flt_style=0;
      else if(!StrCmp("FCVTDLU",opc->str))
	flt_style=0;
      else if(!StrCmp("FCVTDL",opc->str))
	flt_style=0;
      else if(!StrCmp("FMVDX",opc->str))
	flt_style=0;
      else if(!StrCmp("FCVTDW",opc->str))
	flt_style=0;
      else if(!StrCmp("FCVTDWU",opc->str))
	flt_style=0;
    }
    poo[argc]=PrsAddrMode(cc,flt_style);
    if(argc+1<want) {
      if(cc->token!=',')
        LexExcept(cc,"Expected a ','.");
      Lex(cc);
    }
  }
  switch(opc->op_type) {
    case 'SC':
      have=poo[0];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[1];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[2];
      if(have->type!='Indir')
        LexExcept(cc,"Expected a indirect operand.");
      ret=RISCV_R(opc->f7,poo[1]->reg,poo[2]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'LR':
      have=poo[0];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[1];
      if(have->type!='Indir')
        LexExcept(cc,"Expected a indirect operand.");
      ret=RISCV_R(opc->f7,0,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'Rs': //Shift
      for(argc=0;argc!=want;argc++) {
        have=poo[argc];
      }
      have=poo[0];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[1];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[2];
      if(have->type!='Imm')
        LexExcept(cc,"Expected a immediater operand.");
      ret=RISCV_I((poo[2]->value&(1<<6-1))|opc->extra<<6,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'RS': //Sqrt
      for(argc=0;argc!=want;argc++) {
        have=poo[argc];
        if(have->type!='Reg')
	  LexExcept(cc,"Expected a register operand.");
      }
      ret=RISCV_I(opc->f7<<5|opc->extra,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'R2':
      for(argc=0;argc!=want;argc++) {
        have=poo[argc];
        if(have->type!='Reg')
	  LexExcept(cc,"Expected a register operand.");
      }
      ret=RISCV_R(opc->f7,opc->extra,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'R':
      for(argc=0;argc!=want;argc++) {
        have=poo[argc];
        if(have->type!='Reg')
	  LexExcept(cc,"Expected a register operand.");
      }
      ret=RISCV_R(opc->f7,poo[2]->reg,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'Il':
      have=poo[0];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[1];
      if(have->type!='Indir')
        LexExcept(cc,"Expected an indirect operand.");
      ret=RISCV_I(poo[1]->value,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      goto imm;
    case 'I':
      have=poo[0];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[1];
      if(have->type!='Reg')
        LexExcept(cc,"Expected register operand.");
      have=poo[2];
      if(have->type!='Imm')
        LexExcept(cc,"Expected a number operand.");
      ret=RISCV_I(poo[2]->value,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      goto imm;
    case 'Ss':
      have=poo[0];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[1];
      if(have->type!='Indir')
        LexExcept(cc,"Expected a Indirect operand.");
      ret=RISCV_S(poo[1]->value,poo[0]->reg,opc->f3,poo[1]->reg,opc->opc);
      goto imm;
    case 'B':
      have=poo[0];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[1];
      if(have->type!='Reg')
        LexExcept(cc,"Expected register operand.");
      have=poo[2];
      if(have->type!='Imm')
        LexExcept(cc,"Expected a number operand.");
      ret=RISCV_B(poo[2]->value,poo[0]->reg,poo[1]->reg,opc->f3,opc->opc);
      goto imm;
    case 'U':
      have=poo[0];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[1];
      if(have->type!='Imm')
        LexExcept(cc,"Expected a number operand.");
      ret=RISCV_U(poo[1]->value,poo[0]->reg,opc->opc);
      goto imm;
    case 'J':
      have=poo[0];
      if(have->type!='Reg')
        LexExcept(cc,"Expected a register operand.");
      have=poo[1];
      if(have->type!='Imm')
        LexExcept(cc,"Expected a number operand.");
      ret=RISCV_J(poo[1]->value,poo[0]->reg,opc->opc);
      goto imm;
    case 'FE':
      have=poo[0];
      if(have->type!='Iorw')
        LexExcept(cc,"Expected an iorw operand.");
      have=poo[1];
      if(have->type!='Iorw')
        LexExcept(cc,"Expected an iorw operand.");
      ret=RISCV_FENCE(0,poo[0]->reg,poo[1]->reg,0,0,0,opc->opc);
      goto imm;
    case 'PS':
      ret=RISCV_FENCE(0,1,0,0,0,0,opc->opc);
imm:
      for(argc=0;argc!=want;argc++) {
        have=poo[argc];
        if(have->type=='Imm'||have->type=='Indir') {
	  if(!have->resolved) {
	    patch=CAlloc(sizeof CAsmPatch);
	    patch->opcode=opc;
	    MemCpy(patch->args,poo,want*sizeof(CAddrMode*));
	    patch->imm=have;
	    patch->addr=blob->len;
	    QueIns(patch,blob->asm_patches);
	    break;
	  }
        }
      }
      break;
  } 
  if(ret!=-1)
    AddBytesToBuf(&blob->data,&blob->len,4,&ret);
  return ret;
}

I64 AsmOpcode0(CAiwniosAOTBlob *blob,CRiscVOpcode *opc,CAddrMode **poo) {
  I64 want,argc,ret=-1;
  CAsmPatch *patch;
  switch(opc->op_type) {
    case 'LR':
      ret=RISCV_R(opc->f7,0,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'R':
      ret=RISCV_R(opc->f7,poo[2]->reg,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'Il':
      ret=RISCV_I(poo[1]->value,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'I':
      ret=RISCV_I(poo[2]->value,poo[1]->reg,opc->f3,poo[0]->reg,opc->opc);
      break;
    case 'Ss':
      ret=RISCV_S(poo[1]->value,poo[0]->reg,opc->f3,poo[1]->reg,opc->opc);
      break;
    case 'B':
      ret=RISCV_B(poo[2]->value,poo[0]->reg,poo[1]->reg,opc->f3,opc->opc);
      break;
    case 'U':
      ret=RISCV_U(poo[1]->value,poo[0]->reg,opc->opc);
      break;
    case 'J':
      ret=RISCV_J(poo[1]->value,poo[0]->reg,opc->opc);
      break;
    case 'FE':
      ret=RISCV_FENCE(0,poo[0]->reg,poo[1]->reg,0,0,0,opc->opc);
      break;
    case 'PS':
      ret=RISCV_FENCE(0,1,0,0,0,0,opc->opc);
      break;
  } 
  return ret;
}



CAiwniosAOTBlob *PrsAsmBlk(CCmpCtrl *cc,I64 flags=0) {
  if(cc->flags&CCF_AOT_COMPILE)
    LexExcept(cc,"AOT assembler not implemented yet");
  Bool one_ins=FALSE;
  if(flags&CMPF_ONE_ASM_INS) {
    one_ins=TRUE;
    goto enter0;
  }
  if(cc->token==TK_IDENT&&cc->hash_entry&&!StrCmp(cc->hash_entry->str,"asm")) {
    if(Lex(cc)!='{')
      LexExcept(cc,"Expected a '{' at");
    Lex(cc);
    goto enter0;
  } else if(cc->token==TK_IDENT) {
    one_ins=TRUE;
    if(cc->hash_entry&&cc->hash_entry->type&HTT_OPCODE)
      goto enter0;
  }
  return NULL;
enter0:
  CAiwniosAOTBlob *ret_blob;
  I64 scope=0;
  CQue *asm_patches=MAlloc(sizeof CQue),*to_fill=MAlloc(sizeof CQue);
  I64 arg_cnt,best_sz,sz,tmp,idx;
  U8 *buf=MAlloc(64),dummy[STR_LEN],*fbuf;

  CCodeMisc *cm;
  CHashExport *exp;
  CHashImport *imp;
  CToFill *tmp_to_fill;
  CAddrMode *tmp_mode;
  CAsmPatch *patch,*next_patch;
  QueInit(asm_patches);
  QueInit(to_fill);
  ret_blob=CAlloc(sizeof CAiwniosAOTBlob);
  ret_blob->to_fill=to_fill;
  ret_blob->asm_patches=asm_patches;
  ret_blob->len=0;  
  ret_blob->data=buf;
enter:
  if(AsmOpcode(cc,ret_blob)>=0) {
  } else if(cc->token==TK_IDENT&&cc->hash_entry&&cc->hash_entry->type&(HTT_ASM_KEYWORD|HTT_KEYWORD)) {
    if(!StrCmp(cc->hash_entry->str,"IMPORT")) {
//TODO
    } else if(!StrCmp(cc->hash_entry->str,"BINFILE")) {
      if(Lex(cc)!=TK_STR)
        LexExcept(cc,"Expected a filename.");
      if(!FileFind(cc->cur_str))
        LexExcept(cc,"File not found");
      fbuf=FileRead(cc->cur_str,&sz);
      AddBytesToBuf(&buf,&ret_blob->len,sz,fbuf);
      Free(fbuf);
    } else if(!StrCmp(cc->hash_entry->str,"DU8")) {
      sz=1;
duxx:
      Lex(cc);
//Perhaps is an unlinked label
      while(TRUE) {
        if(cc->token==TK_STR) {
	  idx=0;
	  while(tmp=cc->cur_str[idx++])
	    AddBytesToBuf(&buf,&ret_blob->len,sz,&tmp);
	  Lex(cc);
        } else {
	  tmp_mode=CAlloc(sizeof CAddrMode);
	  PrsAsmImm(cc,tmp_mode);
	  patch=CAlloc(sizeof CAsmPatch);
	  patch->imm=tmp_mode;
	  patch->addr=ret_blob->len;
	  patch->sz=sz;
	  QueIns(patch,asm_patches);
	  tmp=0;
	  AddBytesToBuf(&buf,&ret_blob->len,sz,&tmp);
        }
        if(cc->token==',') {
	  Lex(cc);
        } else if(cc->token==';') {
	  Lex(cc);
	  break;
        } else {
	  LexExcept(cc,"Expected a ',' or ';'.");
        }
      }
    } else if(!StrCmp(cc->hash_entry->str,"DU16")) {
      sz=2;
      goto duxx;
    } else if(!StrCmp(cc->hash_entry->str,"DU32")) {
      sz=4;
      goto duxx;
    } else if(!StrCmp(cc->hash_entry->str,"DU64")) {
      sz=8;
      goto duxx;
    }
  } else if(cc->token==TK_IDENT) {
    StrCpy(dummy,cc->cur_str);
    switch(Lex(cc)) {
      start:
        exp=CAlloc(sizeof(CHashExport));
        HashSrcFileSet(cc,exp);
        exp->type=HTT_EXPORT_SYS_SYM;
        exp->val=ret_blob->len;
        exp->str=StrNew(dummy);
        tmp_to_fill=CAlloc(sizeof CToFill);
        tmp_to_fill->h=exp;
        QueIns(tmp_to_fill,to_fill);
        case ':':
	  break;
        case TK_DBL_COLON:
	  exp->type|=HTF_EXPORT;
	  break;
        default:
	  LexExcept(cc,"Expected a ':'");
      end:;
        Lex(cc);
        if(StrNCmp("@@",dummy,2)) {
	  cc->asm_local_scope++;
        } else {
	  tmp=exp->str;
	  exp->str=MStrPrint("%d.%s",cc->asm_local_scope,tmp);
	  Free(tmp);
        }
        HashAdd(exp,cc->htc.glbl_hash_table);
    }
  } else if(cc->token==';') {
    Lex(cc);
  } else {
    QueDel(to_fill);
    Free(buf);
    LexExcept(cc,"Expected a '}' at ");
  }
  if(!one_ins) {
    if(cc->token!='}')
      goto enter;
    if(Lex(cc)!=';') {
      Lex(cc);
      QueDel(to_fill);
      Free(buf);
      LexExcept(cc,"Expected a ';'.");
    }
  }
fin:
  return ret_blob;
}
#if 0
U8 *AsmTest() {
  U8 *ret,*oret;
  CAiwniosAOTBlob *blob;
  CCmpCtrl *cctrl=CmpCtrlNew(
"asm {\n"
"PAUSE\n"
"LUI a0,123\n"
"AUIPC a0,123\n"
"JAL a0,POO\n"
"JALR ra,a0,123\n"
"BEQ a0,a1,POO\n"
"BNE a0,a1,POO\n"
"BLT a0,a1,POO\n"
"BGE a0,a1,POO\n"
"BLTU a0,a1,POO\n"
"BGEU a0,a1,POO\n"
"POO::\n"
"LB a0,1[a1]\n"
"LH a0,1[a1]\n"
"LW a0,1[a1]\n"
"LD a0,1[a1]\n"
"LBU a0,1[a1]\n"
"LHU a0,1[a1]\n"
"LWU a0,1[a1]\n"
"SB a0,1[a1]\n"
"SH a0,1[a1]\n"
"SW a0,1[a1]\n"
"SD a0,1[a1]\n"
"ADDI a0,a1,123\n"
"SLTI a0,a1,123\n"
"SLTIU a0,a1,123\n"
"XORI a0,a1,123\n"
"ORI a0,a1,123\n"
"ANDI a0,a1,123\n"
"SLLI a0,a1,1\n"
"SRLI a0,a1,1\n"
"SRAI a0,a1,1\n"
"ADDa0,a1,a2\n"
"SUBa0,a1,a2\n"
"SLL a0,a1,a2\n"
"SLT a0,a1,a2\n"
"SLTU a0,a1,a2\n"
"XOR a0,a1,a2\n"
"SRL a0,a1,a2\n"
"SRA a0,a1,a2\n"
"OR a0,a1,a2\n"
"AND a0,a1,a2\n"
//"FENCE\n"
"MUL a0,a1,a2\n"
"DIV a0,a1,a2\n"
"DIVU a0,a1,a2\n"
"REM a0,a1,a2\n"
"REMU a0,a1,a2\n"
"LRD a0, [a1]\n"
"SCD a0,a1,[a2]\n"
"AMOSWAPD a0,a1,[a2]\n"
"AMOADDD a0,a1,[a2]\n"
"AMOXORD a0,a1,[a2]\n"
"AMOANDD a0,a1,[a2]\n"
"AMOORD a0,a1,[a2]\n"
"AMOMIND a0,a1,[a2]\n"
"AMOMAXD a0,a1,[a2]\n"
"AMOMINUD a0,a1,[a2]\n"
"AMOMAXUD a0,a1,[a2]\n"

"FLD ft1,123[a0]\n"
"FSD ft1,123[a0]\n"
"FADD fa0,fa1,fa2\n"
"FSUB fa0,fa1,fa2\n"
"FMUL fa0,fa1,fa2\n"
"FDIV fa0,fa1,fa2\n"
"FSQRT fa0,fa1\n"
"FSGNJ fa0,fa1,fa2\n"
"FSGNJN fa0,fa1,fa2\n"
"FSGNJX fa0,fa1,fa2\n"
"FEQ a0,fa1,fa2\n"
"FLT a0,fa1,fa2\n"
"FLE a0,fa1,fa2\n"
"FCLASS a0,fa1\n"
"FCVTLD a1,fa2\n"
"FCVTLUD a1,fa2\n"
"FCVTDL fa1,a2\n"
"FCVTDLU fa1,a2\n"
"_FIB::\n"
"  ADDI sp,sp,-16\n"
"  SD ra,8[sp]\n"
"  SD a0,[sp]\n"
"  ADDI t0,zero,2\n"
"  BLT a0,t0,@@ret\n"
"  ADDI a0,a0,-1\n"
"  JAL ra,_FIB\n"
"  LD t0,[sp]\n"
"  SD a0,[sp]\n"
"  ADDI a0,t0,-2\n"
"  JAL ra,_FIB\n"
"  LD t0,[sp]\n"
"  ADD a0,a0,t0\n"
"@@ret:\n"
"  LD ra,8[sp]\n"
"  ADDI sp,sp,16\n"
"  JALR zero,ra,0\n"
"};\n"
,
CCF_DONT_FREE_BUF
  );
  Lex(cctrl);
  blob=PrsAsmBlk(cctrl);
  oret=ret=MAllocIdent(blob->data,Fs->code_heap);
PrintI("ret",ret);
  JITFixupAiwniosAOTBlob(cctrl,blob,ret);
  CmpCtrlDel(cctrl);
  while(ret<oret+blob->len) {
    ret=DisasOpc(DocPut,ret);
 }
  AiwniosAOTBlobDel(blob);
}
AsmTest;
_extern _FIB I64 Fib(I64);
"%d\n",Fib(10);
#endif
