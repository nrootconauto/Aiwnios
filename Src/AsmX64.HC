//Include UAsmX64.HC first

//	TODO
// * DIL
// * JMP I8 rel

class CAiwniosAOTBlob {
  CQue *asm_patches;
  CQue *to_fill;
  I64 len;
  U8 *data;
  //private,used when placing ->data into it's final location
  U8 **final_start_addr;
};
U0 AiwniosAOTBlobDel(CAiwniosAOTBlob *b) {
  QueDel(b->asm_patches);
  QueDel(b->to_fill);
  Free(b->asm_patches);
  Free(b->to_fill);
  Free(b->data);
  Free(b);
}
#define X86MT_REG 1
#define X86MT_INDIR 2
#define X86MT_IMM 3
class CX86AddrMode {
  I8 type;
  I8 scale;
  I8 idx;
  I8 seg;
  union {
    I8 r;
    I8 base;
  }
  I8 short_addr;
  I8 is_upper_reg;
  I8 operand_sz;
  U8 reg_type;
  union {
    I64 off;
    I64 imm;
 }
  CCodeMisc *label;
  CAsmUndefHash *local_asm_undef_hash;
  CAsmUndefHash *glbl_asm_undef_hash;
  I64 (*machine_code)();
};
U0 X86AddrModeDel(CX86AddrMode *md) {
  if(md->local_asm_undef_hash)
    LinkedLstDel(md->local_asm_undef_hash);
  if(md->glbl_asm_undef_hash)
    LinkedLstDel(md->glbl_asm_undef_hash);
  Free(md->machine_code);
  Free(md);
}
class CAsmPatch:CQue  {
  CX86AddrMode *imm;
  I64 sz;
  I64 rip;
  Bool is_rel;
};
Bool PrsAsmImm(CCmpCtrl *cc,CX86AddrMode *arg)
{
  I64 old_flags=cc->flags;
  arg->local_asm_undef_hash=NULL;
  arg->glbl_asm_undef_hash=NULL;
  cc->asm_undef_hash=NULL;
  cc->abs_cnts=0;
  cc->flags&=~(CCF_UNRESOLVED+CCF_LOCAL);
  cc->flags|=CCF_ASM_EXPRESSIONS;
  if (!IsLexExpression2Bin(cc,&arg->machine_code))
    LexSkipEol(cc);
  else {
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
      arg->imm=(*arg->machine_code)();
      arg->glbl_asm_undef_hash=cc->asm_undef_hash;
      cc->asm_undef_hash=NULL;
      Free(arg->machine_code);
      arg->machine_code=NULL;
    }
  }
  cc->flags=old_flags;
  return TRUE;
}

CX86AddrMode *PrsX86AddrMode(CCmpCtrl *cc) {
  CX86AddrMode *mode=CAlloc(sizeof CX86AddrMode);
  CHashReg *rg;
  I64 (*machine_code)(),cur_i64;
  mode->scale=-1;
  mode->type=-1;
  mode->idx=-1;
  mode->base=-1;
  mode->r=-1;
  mode->short_addr=-1; //Will be set later
  mode->operand_sz=-1;
  mode->seg=-1;
  mode->imm=0;
  mode->label=NULL;
  if(cc->token==TK_IDENT) {
    if(cc->hash_entry&&cc->hash_entry->type&(HTT_CLASS|HTT_INTERNAL_TYPE)) {
      mode->operand_sz=cc->hash_entry(CHashClass*)->size;
      Lex(cc);
      goto indir;
    }
    if(cc->hash_entry&&cc->hash_entry->type&HTT_REG) {
      rg=cc->hash_entry;
      Lex(cc);
      if(cc->token==':') {
        Lex(cc);
        if(rg->reg_type!=REGT_SEG) {
	  Free(mode);
	  LexExcept(cc,"Expected a segment register at");
        }
        mode->seg=rg->reg_num;
        goto indir;
      }
      mode->r=rg->reg_num;
      mode->reg_type=rg->reg_type;
      mode->type=X86MT_REG;
      goto ret;
    }
  }
indir:
  if(cc->token=='[') {
    Lex(cc);
    mode->type=X86MT_INDIR;
indir_loop:
    switch(cc->token) {
      case ']':
        Lex(cc);
        break;
      default:
        goto check_imm;
      case TK_IDENT:
        if(cc->hash_entry&&cc->hash_entry->type&HTT_REG) {
	  rg=cc->hash_entry;
	  Lex(cc);
	  if(cc->token=='+') {
	    Lex(cc);
	    if(mode->base==-1)
	      mode->base=rg->reg_num;
	    else if(mode->idx==-1)
	      mode->idx=rg->reg_num;
	    else {
	      Free(mode);
	      LexExcept(cc,"Already has a base/index.");
	    }
addr_reg_chk:
	    if(rg->reg_type==REGT_R64) {
	      if(mode->short_addr==1) {
	        Free(mode);
	        LexExcept(cc,"Both address registers must be the same size.");
	      }
	      mode->short_addr=0;
	    } else if(rg->reg_type==REGT_R32) {
	      if(mode->short_addr==0) {
	        Free(mode);
	        LexExcept(cc,"Both address registers must be the same size.");
	      }	      
	      mode->short_addr=1;
	    } else if(mode->short_addr==1) {
	      Free(mode);
	      LexExcept(cc,"Invalid address register.");
	    }
	    mode->short_addr=rg->reg_type==REGT_R32;
	  } else if(cc->token=='*') {
	    if(mode->idx!=-1) {
	      Free(mode);
	      LexExcept(cc,"Already has a index.");
	    }
	    Lex(cc);
	    if(cc->token!=TK_I64) {
idx_fail:
	      Free(mode);
	      LexExcept(cc,"Expected an index value of 1/2/4/8.");
	    }
	    cur_i64=cc->cur_i64;
set_idx:
	    switch(cur_i64) {
	      case 1:
	      case 2:
	      case 4:
	      case 8:
	        mode->scale=cc->cur_i64;
	        mode->idx=rg->reg_num;
	        Lex(cc);
	        goto addr_reg_chk;
	      default:
	        goto idx_fail;
	    }
	  } else {
	    mode->short_addr=rg->reg_type==REGT_R32;
	    if(mode->base==-1)
	      mode->base=rg->reg_num;
	    else if(mode->idx==-1)
	      mode->idx=rg->reg_num;
	    else {
	      Free(mode);
	      LexExcept(cc,"Only 2 indirect registers allowed at");
	    }
	  }
        } else 
	  goto check_imm;
        goto indir_loop;
      case TK_I64: 
        cur_i64=cc->cur_i64;
        LexPush(cc);
        if(Lex(cc)=='*')
	  if(Lex(cc)==TK_IDENT&&cc->hash_entry&&cc->hash_entry->type&HTT_REG) {
	    rg=cc->hash_entry;
	    LexPopNoRestore(cc);
	    Lex(cc);
	    goto indir_loop;
	  }
        LexPopRestore(cc);
//Restore token too
        cc->cur_i64=cur_i64;
        cc->token=TK_I64;
check_imm:
        PrsAsmImm(cc,mode);
        if(cc->token!=']') {
exp_indir_end:
	  Free(mode);
	  LexExcept(cc,"Expected a ']'.");
        }
        goto indir_loop;
    }
    goto ret;
  }
  if(mode->type==-1) {
//Check for immdiate
    mode->type=X86MT_IMM;
    PrsAsmImm(cc,mode);
    if(cc->token=='[')
      goto indir;
    goto ret;
  }
  if(mode->type==-1) {
    Free(mode);
    mode=NULL;
    goto ret;
  }
ret:
  return mode;
}
class CX86Opcode:CHash {
  //-1 for absent
  I16 pf,_0f,po,so;
  I32 flags;
//low 8 is type,high 8 is value(what register)
  U16 operands[4];
//private
  CQue asm_hash_que;
//addi reg is used in conjuntion to X86_OPCF_ADD_REG
//It is the added reg to the opcode
  I8 addi_reg,arity,pad[6];
};
Bool ModeMatch(U16 md,CX86AddrMode *mode,I64 mem_sz,Bool mem_only=FALSE) {
  switch(md.u8[0]) {
    case X86_OPERAND_R8:
r8:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_R8)
	  goto pass;
      return FALSE;
    case X86_OPERAND_R16:
r16:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_R16)
	  goto pass;
      return FALSE;
    case X86_OPERAND_R32:
r32:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_R32)
	  goto pass;
      return FALSE;
    case X86_OPERAND_R64:
r64:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_R64)
	  goto pass;
      return FALSE;
    case X86_OPERAND_RM8:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==1||mode->operand_sz==-1)
	  goto pass;
      if(mem_only) return FALSE;
      goto r8;
    case X86_OPERAND_RM16:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==2||mode->operand_sz==-1)
	  goto pass;
      if(mem_only) return FALSE;
      goto r16;
    case X86_OPERAND_RM32:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==4||mode->operand_sz==-1)
	  goto pass;
      if(mem_only) return FALSE;
      goto r32;
    case X86_OPERAND_RM64:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==8||mode->operand_sz==-1)
	  goto pass;
      if(mem_only) return FALSE;
      goto r64;
    case X86_OPERAND_IMM8:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_IMM)
        if(I8_MIN<=mode->imm<=I8_MAX)
	  goto pass;
      return FALSE;
    case X86_OPERAND_IMM16:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_IMM)
        if(I16_MIN<=mode->imm<=I16_MAX)
	  goto pass;
      return FALSE;
    case X86_OPERAND_IMM32:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_IMM)
        if(I32_MIN<=mode->imm<=I32_MAX)
	  goto pass;
      return FALSE;
    case X86_OPERAND_IMM64:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_IMM)
        goto pass;
      return FALSE;
    case X86_OPERAND_CONST_R8:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_R8
	      &&mode->r==md.u8[1])
	  goto pass;
      return FALSE;
    case X86_OPERAND_CONST_R16:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_R16
	      &&mode->r==md.u8[1])
	  goto pass;
      return FALSE;
    case X86_OPERAND_CONST_R32:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_R32
	      &&mode->r==md.u8[1])
	  goto pass;
      return FALSE;
    case X86_OPERAND_CONST_R64:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_R64
	      &&mode->r==md.u8[1])
	  goto pass;
      return FALSE;
    case X86_OPERAND_Rx:
rx:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG) {
        switch(mode->reg_type) {
	  case REGT_R16:
	    if(mem_sz!=2) return FALSE;
	    goto pass;
	  case REGT_R32:
	    if(mem_sz!=4) return FALSE;
	    goto pass;
	  case REGT_R64:
	    if(mem_sz!=8) return FALSE;
	    goto pass;
        }
      }
      return FALSE;
    case X86_OPERAND_RMx:
      if(mode->type==X86MT_INDIR) {
        if(mode->operand_sz==mem_sz||mode->operand_sz==-1) {
	  goto pass;
	}
      }
      if(mem_only) return FALSE;
      goto rx;
    case X86_OPERAND_CONST_IMM:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_IMM)
        if(mode->imm==md.u8[1])
	  goto pass;
      return FALSE;
    case X86_OPERAND_IMMx:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_IMM) {
        switch(mem_sz) {
	  case 8:
	  case 4:
	    if(I32_MIN<=mode->imm<=I32_MAX)
	      goto pass;
	  case 2:
	    if(I16_MIN<=mode->imm<=I16_MAX)
	      goto pass;
        } 
      }
      return FALSE;
    case X86_OPERAND_CONST_Rx:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        switch(mode->reg_type) {
	case REGT_R16:
	case REGT_R32:
	case REGT_R64:
        if(mode->r==md.u8[1])
	  return TRUE;
	}
      return FALSE;
    case X86_OPERAND_RXMM:
rxmm:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_XMM)
	  goto pass;
      return FALSE;
    case X86_OPERAND_RMXMM:
    case X86_OPERAND_RMXMM128:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==16||mode->operand_sz==-1)
	  goto pass;
      if(mem_only) return FALSE;
      goto rxmm;
    case X86_OPERAND_RMXMM64:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==8||mode->operand_sz==-1)
	  goto pass;
      if(mem_only) return FALSE;
      goto rxmm;
    case X86_OPERAND_RMXMM32:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==4||mode->operand_sz==-1)
	  goto pass;
      if(mem_only) return FALSE;
      goto rxmm;
    case X86_OPERAND_RMMM:
      if(mode->type==X86MT_INDIR)
        goto pass;
      if(mem_only) return FALSE;
      goto rmm;
    case X86_OPERAND_RMM:
rmm:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_MM)
	  goto pass;
      return FALSE;
    case X86_OPERAND_FR:
    case X86_OPERAND_STi:
fr:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_FSTK)
	  goto pass;
      return FALSE;
    case X86_OPERAND_REL8:
/*
      if(mode->type==X86MT_IMM)
        goto pass;
*/
      return FALSE;
    case X86_OPERAND_REL32:
      if(mode->type==X86MT_IMM)
        goto pass;
      return FALSE;
    case X86_OPERAND_M8:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==1||mode->operand_sz==-1)
	  goto pass;
      return FALSE;
    case X86_OPERAND_M16:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==2||mode->operand_sz==-1)
	  goto pass;
      return FALSE;
    case X86_OPERAND_M32:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==4||mode->operand_sz==-1)
	  goto pass;
      return FALSE;
    case X86_OPERAND_M64:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==8||mode->operand_sz==-1)
	  goto pass;
      return FALSE;
    case X86_OPERAND_ST0:
      if(mem_only) return FALSE;
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_FSTK&&mode->r==0)
	  goto pass;
      return FALSE;
    case X86_OPERAND_FRM32:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==4||mode->operand_sz==-1)
	  goto pass;
      if(mem_only) return FALSE;
      goto fr;
    case X86_OPERAND_FRM64:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==8||mode->operand_sz==-1)
	  goto pass;
      if(mem_only) return FALSE;
      goto fr;
    case X86_OPERAND_Mx:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==mem_sz||mode->operand_sz==-1)
	  goto pass;
      return FALSE;
    case X86_OPERAND_SREG:
      if(mode->type==X86MT_REG)
        if(mode->reg_type==REGT_SEG)
	  goto pass;
      if(mem_only) return FALSE;
      return FALSE;
    case X86_OPERAND_MOFF8:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==1||mode->operand_sz==-1)
	  if(mode->base==-1&&
	        mode->idx==-1)
	    goto pass;
      return FALSE;
    case X86_OPERAND_MOFFx:
      if(mode->operand_sz!=mem_sz&&mode->operand_sz!=-1)
        return FALSE;
      if(mode->type==X86MT_INDIR)
        if(mode->base==-1&&
	      mode->idx==-1)
	  goto pass;
      return FALSE;
    case X86_OPERAND_EXT:
      if(mem_only) return FALSE;
      return FALSE;
    case X86_OPERAND_RMMM64:
      if(mode->type==X86MT_INDIR)
        if(mode->operand_sz==8||mode->operand_sz==-1)
	  goto pass;
      goto rmm;
    default:
      throw('AsmItrn');
  }
pass:
  return TRUE;
}
Bool OpcodePassesModes(CX86Opcode *opc,CX86AddrMode **modes) {
  I64 a=0,mem_sz,mask,idx;
  if(!opc->arity) return TRUE;
//Start at 16 bits,multiply to 32,then 64
  for(mem_sz=2;mem_sz<=8;mem_sz*=2) {
    a=0;
    for(idx=0;opc->operands[idx].u8[0];idx++) {
      if(opc->operands[idx].u8[0]==X86_OPERAND_EXT) goto nxt_arg;
      if(a>=opc->arity) break;
      if(ModeMatch(opc->operands[idx],modes[a],mem_sz)) {
	a++;
	goto nxt_arg;
      }
      break;
nxt_arg:;
    }
    if(a==opc->arity)
      return TRUE;
  }
  return FALSE;
}
//Returns error code
I64 EncodeSib(U8 *to,I64 r,I64 s,I64 i,I64 b,I64 off=0,I64 *_disp_sz=NULL) {
  I64 disp_sz=0;
  I64 modrm=-1;
  I64 sib=-1;
  if(b==REG_RIP) {
    disp_sz=4;
    modrm=0b101|r<<3;
    goto fin;
  }
  if(b==-1&&i!=-1) {
    disp_sz=4;
    modrm=0b00000100|(r&0b111)<<3;
    switch(s) {
      case -1:
      case 1: sib=0b00000101|((i&0b111)<<3); break;
      case 2: sib=0b01000101|((i&0b111)<<3); break;
      case 4: sib=0b10000101|((i&0b111)<<3); break;
      case 8: sib=0b11000101|((i&0b111)<<3); break;
    }
    goto fin;
  }
  if(i==-1&&b==-1) {
    disp_sz=4;
    modrm=0b00000100|((r&0b111)<<3);
    sib=0b00100101;
    goto fin;
  }
  if(b==REG_RSP&&i==-1) {
    sib=0b00100100;
    if(!off) {
      modrm=0b00000100|((r&0b111)<<3);
    } else if(I8_MIN<=off<=I8_MAX) {
      modrm=0b01000100|((r&0b111)<<3);
      disp_sz=1;
    }
    modrm=0b10000100|(r&0b111)<<3;
    disp_sz=4;
    goto fin;
  }
  if(!off&&i==-1) {
    if(b&0b111==0b100) {
      modrm=0b01000101|((r&0b111)<<3);
      sib=0b00100000|(b&0b111);
      goto fin;
    } else if(b&0b111==0b101) {
      disp_sz=1;
      modrm=0b01000101|((r&0b111)<<3);
      goto fin;
    }
    modrm=(b&0b111)|((r&0b111)<<3);
    goto fin;
  }
  if(i==-1) {
    if(I8_MIN<=off<=I8_MAX) {
      disp_sz=1;
      modrm=0b01000000;
    } else {
      disp_sz=4;
      modrm=0b10000000;
    }
    if(b&0b111==0b100) {
      modrm=0b01000101|((r&0b111)<<3);
      sib=0b00100000|(b&0b111);
      if(b&0b111==0b101)
        disp_sz=1;
      goto fin;
    }
    modrm|=(b&0b111)|((r&0b111)<<3);
    goto fin;
  }
  if(i!=-1&&b==-1) {
    disp_sz=4;
    modrm=0b01000100|((r&0b111)<<3);
    sib=0b00000101|((i&0b111)<<3);
    goto fin;
  }
  if(i!=-1||b!=-1) {
    switch(s) {
      case -1:
      case 1: sib=0b00000000; break;
      case 2: sib=0b01000000; break;
      case 4: sib=0b10000000; break;
      case 8: sib=0b11000000; break;
    }
    if((b==REG_RBP||b==13)&&I8_MIN<=off<=I8_MAX) {
      disp_sz=1;
      modrm=0b01000100|(r&0b111)<<3;
      sib|=(b&0b111)|((i&0b111)<<3);
      goto fin;
    } else if(!(b==REG_RBP||b==13)&&!off) {
      modrm=0b00000100|(r&0b111)<<3;
      sib|=(b&0b111)|((i&0b111)<<3);
      goto fin;
    }
    sib|=(b&0b111)|((i&0b111)<<3);
    if(I8_MIN<=off<=I8_MAX) {
      disp_sz=1;
      modrm=0b01000100|((r&0b111)<<3);
      goto fin;
    } else {
      disp_sz=4;
      modrm=0b10000100|((r&0b111)<<3);
      goto fin;
    }
    goto fin;
  }
  throw('AsmItrn');
fin:
  if(to) *to++=modrm;
  if(to&&sib!=-1) *to++=sib;
  switch(disp_sz) {
    case 0:
      if(off) return -1;
      break;
    case 1:
      if(!(I8_MIN<=off<=I8_MAX)) return -1;
      if(to)
        *to++=off;
      break;
    case 4:
      if(!(I32_MIN<=off<=I32_MAX)) return -1;
      if(to) {
        to(U32*)[0]=off;
        to+=4;       
      }
      break;
  }
  if(_disp_sz) *_disp_sz=disp_sz;
  return 1+(sib!=-1)+disp_sz;
}
I64 X86EncodeOpcode(I64 rip,CX86Opcode *opc,CX86AddrMode **modes,U8 *to,CQue *asm_patches) {
  U8 dummy[STR_LEN];
  I64 segment_override=-1;
  I64 i,ext=-1,a;
  I64 operand_sz=-1,len=0,sz,indir_mode_idx=-1;
  I64 mask=1<<opc->arity-1;
  Bool is_x_sz=FALSE,wants_modrm_sib=FALSE;
  I64 rex=0,rel_sz=0; //Upper 4 bits will be 0x4 if needed
  if(!to) to=dummy;
  CX86AddrMode *indir_mode=NULL;
  CX86AddrMode *reg_mode=NULL;
  CAsmPatch *patch;
  for(a=0;a!=4;a++)
    switch(opc->operands[a].u8[0]) {
      case X86_OPERAND_REL8:
        rel_sz=1;
        break;
      case X86_OPERAND_REL32:
        rel_sz=4;
        break;
    }
//Check for MOFFx/8
  for(i=0;opc->operands[i].u8[0];i++) {
    switch(opc->operands[i].u8[0]) {
      case X86_OPERAND_MOFF8:
      case X86_OPERAND_MOFFx:
        for(a=0;a!=opc->arity;a++)
	  if(modes[a]->type==X86MT_INDIR)
	    indir_mode=modes[a];
    }
  }
//Check for segment overides
  for(a=0;a!=opc->arity;a++)
    if(modes[a]->type==X86MT_INDIR)
      if(-1!=modes[a]->seg) {
        switch(modes[a]->r) {
	  case 0: //ES
	    segment_override=0x26;
	    break;
	  case 1: //CS
	    segment_override=0x3e;
	    break;
	  case 2: //SS
	    segment_override=0x36;
	    break;
	  case 3: //DS
	    segment_override=0x3e;
	    break;
	  case 4: //FS
	    segment_override=0x64;
	    break;
	  case 6: //FS
	    segment_override=0x65;
	    break;
	  default:
	    throw('Asm');
        }
      }
//Chect RMx/Rx/IMMx to see if we need to supply 0x66/REX.w/nothing
  for(i=0;i!=4;i++)
    switch(opc->operands[i].u8[0]) {
      start:
        case X86_OPERAND_MOFFx:
        case X86_OPERAND_Mx:
        case X86_OPERAND_RMx:
//Try memory size
	  for(a=0;a!=opc->arity;a++)
	    if(modes[a]->type==X86MT_INDIR)
	      operand_sz=modes[a]->operand_sz;
        case X86_OPERAND_CONST_Rx:
        case X86_OPERAND_Rx:
//Then guess from register size
	  for(a=0;a!=opc->arity;a++)
	    if(modes[a]->type==X86MT_REG&&operand_sz==-1) {
	      switch(modes[a]->reg_type) {
	        case REGT_R8: operand_sz=1; break;
	        case REGT_R16: operand_sz=2; break;
	        case REGT_R32: operand_sz=4; break;
	        case REGT_R64: operand_sz=8; break;
	      }
	    }
	  break;
        case X86_OPERAND_IMMx:
//If nothing matched,check immediate size
	  for(a=0;a!=opc->arity;a++)
	    if(modes[a]->type==X86MT_IMM&&operand_sz==-1)
	      if(opc->operands[i].u8[0]==X86_OPERAND_IMMx) {
	        if(I16_MIN<=modes[a]->imm<=I16_MAX&&!modes[a]->machine_code)
		  operand_sz=2;
	        else
		  operand_sz=4;
	        break;
	      }
      end:
        is_x_sz=TRUE;
    }
  if(operand_sz==-1&&opc->operands[0].u8[0]) //Make sure our opcode takes arguments
    operand_sz=8;

  if(X86_OPCF_MODRM&opc->flags) {
    for(a=0,i=0;i!=4;i++)
      if(opc->operands[i].u8[0]==X86_OPERAND_EXT)
        ext=opc->operands[i].u8[1];
    for(a=0,i=0;i!=4&&a<opc->arity;i++) {
      if(opc->operands[i].u8[0]) {
        switch(opc->operands[i].u8[0]) {
	  case 0: goto mdrm_fin;
	  case X86_OPERAND_M8:
	  case X86_OPERAND_M16:
	  case X86_OPERAND_M32:
	  case X86_OPERAND_M64:
	  case X86_OPERAND_Mx:
	  case X86_OPERAND_RMMM64:
	  case X86_OPERAND_MOFFx:
	  case X86_OPERAND_MOFF8:
	    if(!indir_mode) {
	      indir_mode_idx=a;
	      indir_mode=modes[a];
	      a++;
	    }
	    break;
	  case X86_OPERAND_FRM32:
	  case X86_OPERAND_FRM64:
	  case X86_OPERAND_RM8:
	  case X86_OPERAND_RM16:
	  case X86_OPERAND_RM32:
	  case X86_OPERAND_RM64:
	  case X86_OPERAND_RMx:
	  case X86_OPERAND_RMXMM:
	  case X86_OPERAND_RMXMM128:
	  case X86_OPERAND_RMXMM64:
	  case X86_OPERAND_RMXMM32:
	  case X86_OPERAND_RMMM:
	    if(!indir_mode) {
	      indir_mode_idx=a;
	      indir_mode=modes[a];
	      a++;
	      break;
	    }
	  case X86_OPERAND_R8:
	  case X86_OPERAND_R16:
	  case X86_OPERAND_R32:
	  case X86_OPERAND_R64:
	  case X86_OPERAND_Rx:
	  case X86_OPERAND_RXMM:
	  case X86_OPERAND_RMM:
	  case X86_OPERAND_FR:
	  case X86_OPERAND_SREG:
	  case X86_OPERAND_STi:
	    if(!reg_mode&&ext==-1) {
	      reg_mode=modes[a];
	      a++;
	    } else if(!indir_mode) {
	      indir_mode_idx=a;
	      indir_mode=modes[a];
	      a++;
	    }
	    break;
	  case X86_OPERAND_ST0:
//This is implicit
	    a++;
        }
      }
    }
mdrm_fin:
    wants_modrm_sib=TRUE;
  } else if(opc->flags&X86_OPCF_ADD_REG) {
    for(a=0;a!=opc->arity&&!reg_mode;a++)
      for(i=0;i!=4;i++)
        if(opc->operands[i].u8[0]&&ModeMatch(opc->operands[i],modes[a],operand_sz)&&Bt(&mask,a))
	  switch(opc->operands[i].u8[0]) {
	    case X86_OPERAND_R8:
	    case X86_OPERAND_R16:
	    case X86_OPERAND_R32:
	    case X86_OPERAND_R64:
	    case X86_OPERAND_Rx:
	    case X86_OPERAND_RXMM:
	    case X86_OPERAND_RMM:
	    case X86_OPERAND_ST0:
	    case X86_OPERAND_STi:
	    case X86_OPERAND_FR:
	    case X86_OPERAND_SREG:
	      if(!reg_mode) {
	        Btr(&mask,a);
	        reg_mode=modes[a];
	      }
	  }
  }
  if(operand_sz==8)
    rex|=0x48;
  if(opc->flags&X86_OPCF_REXW)
    rex|=0x48;
  if(X86_OPCF_REX&opc->flags)
    rex|=0x40;
  if(opc->flags&X86_OPCF_ADD_REG) {
    if(reg_mode&&reg_mode->r&0b1000) 
      rex|=0x41;
  } else if(reg_mode&&reg_mode->r&0b1000)
    rex|=0x40|0b100;
  if(indir_mode&&indir_mode->type==X86MT_INDIR) {
    rex|=0x40;
    if(indir_mode->base!=-1&&indir_mode->base&0b1000)
      rex|=1;
    if(indir_mode->idx!=-1&&indir_mode->idx&0b1000)
      rex|=0b10;
  }
  if(indir_mode&&indir_mode->type==X86MT_REG) {
    if(indir_mode->base!=-1&&indir_mode->base&0b1000)
      rex|=0x41;
  }
/*
//SPL,BPL,SIL,DIL require REX
if(reg_mode&&reg_mode->type==X86MT_REG)
switch(reg_mode->r) {
case REG_RSP:
case REG_RBP:
case REG_RSI:
case REG_RDI:
if(reg_mode->reg_type==REGT_R8)
rex|=0x40;
}
//SPL,BPL,SIL,DIL require REX
if(indir_mode&&indir_mode->type==X86MT_REG)
switch(indir_mode->r) {
case REG_RSP:
case REG_RBP:
case REG_RSI:
case REG_RDI:
if(indir_mode->reg_type==REGT_R8)
rex|=0x40;
}
*/

  //Prefix group 2
  if(segment_override!=-1)
    to[len++]=segment_override;

  if(opc->pf!=-1) {
    to[len++]=opc->pf;  
  }
//Prefix group 3(Operand size prefix)
  if(is_x_sz&&operand_sz==2) {
    to[len++]=0x66;
  }
//Prefix group 4(Address size ovveride)
  if(rex&0x40) {
    to[len++]=rex;    
  }
  if(opc->_0f!=-1) {
    to[len++]=0x0f;
  }
  if(opc->flags&X86_OPCF_ADD_REG) {
    to[len++]=opc->po+(reg_mode->r&0b111);
  } else {
    to[len++]=opc->po;
  }
  if(opc->so!=-1) {
    to[len++]=opc->so;
  }
  if(wants_modrm_sib) {
    if(reg_mode)
      ext=reg_mode->r&0b111;
    if(indir_mode->type==X86MT_INDIR) {
      if(indir_mode->machine_code) {
        if(!asm_patches)
	  len+=EncodeSib(to+len,ext,indir_mode->scale,indir_mode->idx,indir_mode->base,
	        I32_MAX);
        else {
	  patch=CAlloc(sizeof CAsmPatch);
	  patch->imm=MAlloc(sizeof CX86AddrMode);
	  MemCpy(patch->imm,modes[indir_mode_idx],sizeof CX86AddrMode);
	  QueIns(patch,asm_patches);
	  len+=EncodeSib(to+len,ext,indir_mode->scale,indir_mode->idx,indir_mode->base,
	        I32_MAX,&patch->sz);
	  patch->rip=rip+len-patch->sz;
	  MemSet(modes[indir_mode_idx],0,sizeof(CX86AddrMode));  //"Steal" the machine code by NULLing
        }
      } else {
        len+=EncodeSib(to+len,ext,indir_mode->scale,indir_mode->idx,indir_mode->base,
	      indir_mode->off);
      }
    } else if(indir_mode->type==X86MT_REG)
      to[len++]=0b11000000|ext<<3|(indir_mode->r&0b111);
  }
  if(!rel_sz) {
    for(a=0;a<opc->arity;)
      for(i=0;i!=4;i++)
        if(opc->operands[i].u8[0]) {
	  if(opc->operands[i].u8[0]!=X86_OPERAND_EXT) {
	    if(modes[a]->machine_code) {
	      switch(opc->operands[i].u8[0]) {
	        start:
		  case X86_OPERAND_IMMx:
		    if(operand_sz==2)
		      sz=2;
		    else
		      sz=4;
		    break;
		  case X86_OPERAND_IMM8:
		    sz=1;
		    break;
		  case X86_OPERAND_IMM16:
		    sz=2;
		    break;
		  case X86_OPERAND_IMM32:
		    sz=4;
		    break;
		  case X86_OPERAND_IMM64:
		    sz=8;
		    break;
	        end:
		  if(asm_patches) {
		    patch=CAlloc(sizeof CAsmPatch);
		    patch->imm=MAlloc(sizeof CX86AddrMode);
		    patch->rip=rip+len;
		    patch->sz=sz;
		    MemCpy(patch->imm,modes[a],sizeof CX86AddrMode);
		    QueIns(patch,asm_patches);
		    MemSet(modes[a],0,sizeof CX86AddrMode); //"Steal" the machine code by nulling
		  }
		  len+=sz;
	      }
	    } else {
	      switch(opc->operands[i].u8[0]) {
	        case X86_OPERAND_IMMx:
		  if(operand_sz==2) {
		    to[len](I16)=modes[a]->imm;
		    len+=2;
		  } else {
		    to[len](I32)=modes[a]->imm;
		    len+=4;
		  }
		  break;
	        case X86_OPERAND_IMM8:
		  to[len++]=modes[a]->imm;
		  break;
	        case X86_OPERAND_IMM16:
		  to[len](I16)=modes[a]->imm;
		  len+=2;
		  break;
	        case X86_OPERAND_IMM32:
		  to[len](I32)=modes[a]->imm;
		  len+=4;
		  break;
	        case X86_OPERAND_IMM64:
		  to[len](I64)=modes[a]->imm;
		  len+=8;
	      }
	    }
	    if(opc->operands[i].u8[0]!=X86_OPERAND_EXT)
	      a++;
	  }
        } else
	  break;
  }
  if(rel_sz) {
    for(a=0;a!=opc->arity;a++)
      if(modes[a]->type==X86MT_IMM) {
        if(asm_patches) {
	  patch=CAlloc(sizeof CAsmPatch);
	  patch->imm=MAlloc(sizeof CX86AddrMode);
	  patch->rip=rip+len;
	  patch->is_rel=TRUE;
	  patch->sz=rel_sz;
	  MemCpy(patch->imm,modes[a],sizeof CX86AddrMode);
	  QueIns(patch,asm_patches);
	  MemSet(modes[a],0,sizeof CX86AddrMode); //"Steal" the machine code by nulling
	  len+=rel_sz;
        } else {
	  len+=rel_sz;
        }
        break;
      }
  }
  for(a=0;opc->operands[a].u8[0];a++) {
    switch(opc->operands[a].u8[0]) {
      case X86_OPERAND_MOFFx:
      case X86_OPERAND_MOFF8:
        if(asm_patches&&indir_mode->machine_code) {
	  patch=CAlloc(sizeof CAsmPatch);
	  patch->imm=MAlloc(sizeof CX86AddrMode);
	  patch->sz=8;
	  MemCpy(patch->imm,indir_mode,sizeof CX86AddrMode);
	  QueIns(patch,asm_patches);
	  MemSet(indir_mode,0,sizeof(CX86AddrMode));  //"Steal" the machine code by NULLing
	  len+=8;
        } else {
	  to[len](I64)=indir_mode->off;
	  len+=8;
        }
    }
  }
fin:
  return len;
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
class CToFill:CQue {
  CHashExport *h;
};
U0 AsmResolvePatches(CCmpCtrl *cc,CQue *patches) {
  CAsmPatch *patch;
  I64 b,i;
  CAsmUndefHash *tab,*cur;
  CHashImport *h;
  CMemberLst *mlst;
  CHashExport *exp;
  CCodeMisc *cm;
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
    patch->imm->off=(*patch->imm->machine_code)();
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
  I64 tmp,sz;
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
    if(patch->imm) {
      tmp=patch->imm->off;
      switch(sz=patch->sz) {
        case 1:
	  if(patch->is_rel)
	    buf[patch->rip]=tmp-(buf+sz+patch->rip);
	  else
	    buf[patch->rip]=tmp;
	  break;
        case 2:
	  if(patch->is_rel)
	    buf[patch->rip](U16)=tmp-(buf+sz+patch->rip);
	  else
	    buf[patch->rip](U16)=tmp;
	  break;
        case 4:
	  if(patch->is_rel)
	    buf[patch->rip](U32)=tmp-(buf+sz+patch->rip);
	  else
	    buf[patch->rip](U32)=tmp;
	  break;
        case 8:
	  if(patch->is_rel)
	    buf[patch->rip](U64)=tmp-(buf+sz+patch->rip);
	  else
	    buf[patch->rip](U64)=tmp;
	  break;
      }
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
  CX86Opcode *opc,*first_opc,*best;
  CQue *asm_patches=MAlloc(sizeof CQue),*to_fill=MAlloc(sizeof CQue);
  I64 arg_cnt,best_sz,sz,len=0,tmp,idx;
  U8 *buf=MAlloc(64),dummy[STR_LEN],*fbuf;
  CCodeMisc *cm;
  CHashExport *exp;
  CHashImport *imp;
  CToFill *tmp_to_fill;
  CX86AddrMode *modes[4],*tmp_mode;
  CAsmPatch *patch,*next_patch;
  QueInit(asm_patches);
  QueInit(to_fill);
enter:
  MemSetU64(modes,NULL,4);
  if(cc->hash_entry&&cc->hash_entry->type&HTT_OPCODE) {
    first_opc=cc->hash_entry;
    Lex(cc);
    for(arg_cnt=0;arg_cnt!=first_opc->arity;arg_cnt++) {
      if(arg_cnt) {
        if(cc->token!=',') {
	  LexExcept(cc,"Expected ',' at");
	  Free(buf);
	  throw('Asm');
        }
        Lex(cc);
      }
      modes[arg_cnt]=PrsX86AddrMode(cc);
      if(!modes[arg_cnt]) {
        while(--arg_cnt>=0)
	  Free(modes[arg_cnt]);
        LexExcept(cc,"Expected addressing mode at");
        Free(buf);
        throw('Asm');
      }
    }
    best=NULL,best_sz=I64_MAX;
    for(opc=first_opc;opc;opc=opc->next) {
      if(!StrCmp(first_opc->str,opc->str))
       if(OpcodePassesModes(opc,modes)) {
	  sz=X86EncodeOpcode(len,opc,modes,NULL,NULL);
	  if(sz<best_sz) {
	    best_sz=sz;
	    best=opc;
	  }
        }
    }
    if(!best) {
      LexExcept(cc,"Invalid usage of opcode:");
      Free(buf);
      throw('Asm');
    }
    sz=X86EncodeOpcode(len,best,modes,dummy,asm_patches);
    AddBytesToBuf(&buf,&len,sz,dummy);
  } else if(cc->token==TK_IDENT&&cc->hash_entry&&cc->hash_entry->type&(HTT_ASM_KEYWORD|HTT_KEYWORD)) {
    if(!StrCmp(cc->hash_entry->str,"IMPORT")) {
//TODO
    } else if(!StrCmp(cc->hash_entry->str,"BINFILE")) {
      if(Lex(cc)!=TK_STR)
        LexExcept(cc,"Expected a filename.");
      if(!FileFind(cc->cur_str))
        LexExcept(cc,"File not found");
      fbuf=FileRead(cc->cur_str,&sz);
      AddBytesToBuf(&buf,&len,sz,fbuf);
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
	    AddBytesToBuf(&buf,&len,sz,&tmp);
	  Lex(cc);
        } else {
	  tmp_mode=CAlloc(sizeof CX86AddrMode);
	  tmp_mode->type=X86MT_IMM;
	  PrsAsmImm(cc,tmp_mode);
	  patch=CAlloc(sizeof CAsmPatch);
	  patch->imm=tmp_mode;
	  patch->rip=len;
	  patch->sz=sz;
	  QueIns(patch,asm_patches);
	  tmp=0;
	  AddBytesToBuf(&buf,&len,sz,&tmp);
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
        exp->val=len;
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
  for(idx=0;idx!=4;idx++) {
    if(modes[idx]) {
      X86AddrModeDel(modes[idx]);
      modes[idx]=NULL;
    }
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
  ret_blob=CAlloc(sizeof CAiwniosAOTBlob);
  ret_blob->to_fill=QueCopy(to_fill);
  ret_blob->asm_patches=QueCopy(asm_patches);
  Free(to_fill);
  Free(asm_patches);
  ret_blob->len=len;  
  ret_blob->data=buf;
  return ret_blob;
}
U8 *AsmTest() {
  U8 *ret,*oret;
  CAiwniosAOTBlob *blob;
  CCmpCtrl *cctrl=CmpCtrlNew(
//Mirrors http://ref.x86asm.net/coder64-abc.html
"asm {\n"
"NOTHING0::"
"MOV RBX,[RAX]\n"
"NOTHING::\n"
"ADC U8 [RAX],AL\n"
"ADC U16 [RAX],AX\n"
"ADC AL,U8 [RAX]\n"
"ADC AX,U16 [RAX]\n"
"ADC AL,1\n"
"ADC RAX,0x123\n"
"ADC U8 [RAX],1\n"
"ADC U16 [RAX],0x123\n"
"ADC U16 [RAX],1\n"

"ADD U8 [RAX],AL\n"
"ADD U16 [RAX],AX\n"
"ADD AL,U8 [RAX]\n"
"ADD AX,U16 [RAX]\n"
"ADD AL,1\n"
"ADD RAX,0x123\n"
"ADD U8 [RAX],1\n"
"ADD U16 [RAX],0x123\n"
"ADD U16 [RAX],1\n"

"ADDSD XMM0,U64 [RAX]\n"
"ADDSS XMM0,U32 [RAX]\n"
"ADDSD XMM0,XMM14\n"

"AND U8 [RAX],AL\n"
"AND U16 [RAX],AX\n"
"AND AL,U8 [RAX]\n"
"AND AX,U16 [RAX]\n"
"AND AL,1\n"
"AND RAX,0x123\n"
"AND U8 [RAX],1\n"
"AND U16 [RAX],0x123\n"
"AND U16 [RAX],1\n"

"BSR R15,U64 [RAX]\n"
"BSF R15,U64 [RAX]\n"
"BSWAP R15\n"
"BT U64 [RAX],R15\n"
"BT U64 [RAX],3\n" //TODOlock check
"BTR U64 [RAX],R15\n"
"BTR U64 [RAX],3\n"
"BTC U64 [RAX],R15\n"
"BTC U64 [RAX],3\n"
"CALL NOTHING\n"
"CALL U64 [RAX]\n"
"CBW\n"
"CWDE\n"
"CDQE\n"
"CLC\n"
"CLD\n"
"CLFLUSH U8 [RAX]\n"
"CLI\n"
"CMC\n"

"CMOVB RAX,U64 [RAX]\n"
"CMOVNA RAX,U64 [RAX]\n"
"CMOVNGE RAX,U64 [RAX]\n"
"CMOVNC RAX,U64 [RAX]\n"
"CMOVA RAX,U64 [RAX]\n"
"CMOVGE RAX,U64 [RAX]\n"
"CMOVG RAX,U64 [RAX]\n"
"CMOVNO RAX,U64 [RAX]\n"
"CMOVNP RAX,U64 [RAX]\n"
"CMOVNS RAX,U64 [RAX]\n"
"CMOVNZ RAX,U64 [RAX]\n"
"CMOVO RAX,U64 [RAX]\n"
"CMOVP RAX,U64 [RAX]\n"
"CMOVS RAX,U64 [RAX]\n"
"CMOVZ RAX,U64 [RAX]\n"

"CMPU8 [RAX],AL\n"
"CMPU16 [RAX],AX\n"
"CMPAL,U8 [RAX]\n"
"CMPAX,U16 [RAX]\n"
"CMPAL,1\n"
"CMPRAX,0x123\n"
"CMPRAX,1\n"

"CMPSB\n"
"CMPSW\n"
"CMPSD\n"
"CMPSQ\n"

"CMPSD2 XMM1,XMM2,3\n"
"CMPSS2 XMM1,XMM2,3\n"

"CMPXCHG U8 [RAX],AL,BL\n"
"CMPXCHG U16 [RAX],AX,BX\n"

"CMPXCHG8B U64 [RAX],EAX,EDX\n"

"COMISD XMM15,U64 [RAX]\n"
"COMISS XMM15,U32 [RAX]\n"
"CVTSD2SI R15,XMM15\n"
"CVTSD2SS XMM15,XMM15\n"
"CVTSI2SS XMM15,R15\n"
"CVTSI2SD XMM15,R15\n"
"CVTSS2SI R15,XMM15\n"
"CVTTSD2SI R15,XMM15\n"
"CVTTSS2SI R15,XMM15\n"
"CWD\n"
"CDQ\n"
"CQO\n"
"DEC U8 0x123[RAX]\n"
"DEC U64 0x123[RAX]\n"
"DIV U8 0x123[RAX]\n"
"DIV U64 0x123[RAX]\n"
"DIVSD XMM15,XMM15\n"
"DIVSS XMM15,XMM15\n"
"ENTER 100,3\n"
"EXTRACTPS EAX,XMM15,3\n"
"F2XM1\n"
"FABS \n"
"FADD ST0,ST1\n"
"FADD ST0,U32 [RBX]\n"
"FADD ST0,U64 [RBX]\n"
"FADD ST2,ST0\n"
"FADDP ST2,ST0\n"
"FADDP ST1,ST0\n"
"FCHS\n"
"FCLEX\n"
"FCMOVB ST0,ST1\n"
"FCMOVBE ST0,ST1\n"
"FCMOVNB ST0,ST1\n"
"FCMOVNBE ST0,ST1\n"
"FCMOVNE ST0,ST1\n"
"FCMOVNU ST0,ST1\n"
"FCMOVU ST0,ST1\n"
"FCOM ST0,ST2\n"
"FCOM2 ST0,ST2\n"
"FCOMI ST0,ST2\n"
"FCOMIP ST0,ST2\n"
"FCOMP ST0,ST2\n"
"FCOMP ST0,U64 [RAX]\n"
"FCOMPP\n"
"FCOS\n"
"FDECSTP\n"
"FDIV ST0,U32 [RAX]\n"
"FDIV ST0,U64 [RAX]\n"
"FDIV ST0,ST2\n"
"FDIVR ST0,U32 [RAX]\n"
"FDIVR ST0,U64 [RAX]\n"
"FDIVR ST2,ST0\n"
"FFREE ST2\n"
"FIADD ST0,U16 [RAX]\n"
"FIADD ST0,U32 [RAX]\n"
"FIDIVR ST0,U16 [RAX]\n"
"FIDIVR ST0,U32 [RAX]\n"
"FIDIV ST0,U16 [RAX]\n"
"FIDIV ST0,U32 [RAX]\n"
"FILD ST0,U32 [RAX]\n"
"FILD ST0,U16 [RAX]\n"
"FILD ST0,U64 [RAX]\n"
"FIMUL ST0,U16 [RAX]\n"
"FIMUL ST0,U32 [RAX]\n"
"FINCSTP\n"
"FINIT\n"
"FISTP U16 [RAX]\n"
"FISTP U32 [RAX]\n"
"FISTP U64 [RAX]\n"
"FIST U16 [RAX]\n"
"FIST U32 [RAX]\n"
"FISUB ST0,U16 [RAX]\n"
"FISUB ST0,U32 [RAX]\n"
"FISUBR ST0,U16 [RAX]\n"
"FISUBR ST0,U32 [RAX]\n"
"FLD ST0,U32 [RAX]\n"
"FLD ST0,U64 [RAX]\n"
"FLDCW U16[RAX]\n"
"FLDL2E\n"
"FLDL2T\n"
"FLDLG2\n"
"FLDLN2\n"
"FLDPI\n"
"FLDZ\n"
"FMUL ST0,U32 [RAX]\n"
"FMUL ST0,ST1\n"
"FMUL ST0,U64 [RAX]\n"
"FNCLEX\n"
"FNINIT\n"
"FNSTCW U16 [RAX]\n" 
"FNSTSW U16 [RAX]\n"
"FPATAN\n"
"FPREM\n"
"FPREM1\n"
"FPTAN\n"
"FRNDINT\n"
"FSIN\n"
"FSINCOS\n"
"FSQRT\n"
"FST U64 [RAX],ST0\n"
"FST U64 [RAX],ST0\n"
"FST ST0,ST1\n"
"FSTCW U16 [RAX]\n"
"FSTP U32 [RAX],ST0\n"
"FSTP U64 [RAX],ST0\n"
"FSTP ST0,ST1\n"
"FSTSW U16 [RAX]\n"
"FSUB ST0,U32[RAX]\n"
"FSUB ST0,U64 [RAX]\n"
"FSUB ST0,ST2\n"
"FSUBP ST2,ST0\n"
"FSUBR ST0,U32[RAX]\n"
"FSUBR ST0,U64 [RAX]\n"
"FSUBR ST0,ST2\n"
"FSUBRP ST2,ST0\n"
"FTST\n"
"FUCOM ST0,ST2\n"
"FUCOMI ST0,ST2\n"
"FUCOMIP ST0,ST2\n"
"FUCOMP ST0,ST2\n"
"FUCOMPP\n"
"FXCH ST0,ST2\n"
"FXTRACT\n"
"IDIV U8[RAX]\n"
"IDIV U64[RAX]\n"
"IMUL3 RDX,RBX,3\n"
"IMUL3 RDX,RBX,300\n"
"IMUL U8 [RAX]\n"
"IMUL U64 [RAX]\n"
"IMUL2 RBX,U64 [RAX]\n"
"IN AL,3\n"
"IN EAX,3\n"
"IN AL,DX\n"
"IN EAX,DX\n"
"INC U8 [RAX]\n"
"INC U64 [RAX]\n"
"INSB\n"
"INSW\n"
"INSD\n"
"INSERTPS XMM15,U32 [RAX],3\n"
"INSERTPS XMM15,XMM14,3\n"
"INT3\n"
"INT 100\n"
"INVD\n"
"IRET\n"
"IRETD\n"
"IRETQ\n"
"@@jc: JC@@jc\n"
"@@jna: JNA @@jna\n"
"@@jnge: JNGE @@jnge\n"
"@@jng: JNG @@jng\n"
"@@jmp: JMP @@jmp\n"
"JMP U32 [RAX]\n"
"JMP U64 [RAX]\n"
"@@jnc: JNC@@jnc\n"
"@@ja: JA@@ja\n"
"@@jge: JGE@@jge\n"
"@@jg: JG@@jg\n"
"@@jno: JNO@@jno\n"
"@@jpo: JPO@@jpo\n"
"@@jns: JNS@@jns\n"
"@@jnz: JNZ@@jnz\n"
"@@jo: JO@@jo\n"
"@@je: JE@@je\n"
"LAHF\n"
"LAR AX,U16 [RAX]\n"
"LEARAX,U64[R15+R15*8]\n"
"LEAVE\n"
"LFENCE\n"
"LODSB\n"
"LODSW\n"
"LODSD\n"
"LODSQ\n"
"LSLRBX,RDX\n"
"MAXSD XMM14,XMM1\n"
"MAXSS XMM14,XMM1\n"
"MFENCE\n"
"MINSD XMM14,XMM1\n"
"MINSS XMM14,XMM1\n"
"MOVU8 [RAX],R15L\n"
"MOVU64 [RAX],R15\n"
"MOVU16 [RAX],FS\n"
"MOVFS,U16 [RAX]\n"
"MOVAL,U8 [0x1122334]\n"
"MOVRAX,U64 [0x1122334]\n"
"MOVU8 [0x1122334],AL\n"
"MOVU64 [0x1122334],RAX\n"
"MOVR15L,1\n"
"MOVR15,0x123\n"
"MOVR15,0x1\n"
"MOVR15D,0x12\n"
"MOVBEU64 [R15],R14\n"
"MOVBER14,U64 [R15]\n"
"MOVQ MM0,RAX\n"
"MOVD MM0,EAX\n"
"MOVQ XMM15,RAX\n"
"MOVD MM7,EAX\n"
"MOVQ U64 [RAX],MM0\n"
"MOVD U32 [RAX],MM0\n"
"MOVQ U64 [RAX],XMM15\n"
"MOVD U32 [RAX],XMM15\n"
"MOVDDUP XMM15,U64 [RAX]\n"
"MOVDQ2Q MM7,XMM15\n"
"MOVHLPS XMM0,XMM1\n"
"MOVHPD U64 [RAX],XMM15\n"
"MOVHPD XMM15,U64 [RAX]\n"
"MOVHPS U64 [RAX],XMM15\n"
"MOVHPS XMM15,U64 [RAX]\n"
"MOVLPD XMM15,U64 [RAX]\n"
"MOVLPD U64 [RAX],XMM15\n"
"MOVLPS XMM15,U64 [RAX]\n"
"MOVLPS U64 [RAX],XMM15\n"
"MOVMSKPD RAX,XMM15\n"
"MOVMSKPS RAX,XMM15\n"
"MOVNTI  U64[RAX],R15\n"
"MOVQ2DQ XMM10,MM5\n"
"MOVSB\n"
"MOVSW\n"
"MOVSD\n"
"MOVSQ\n"
"MOVSD2 XMM15,U64 [RAX]\n"
"MOVSD2 U64 [RAX],XMM15\n"
"MOVSHDUP XMM15,U64 [RAX]\n"
"MOVSLDUP XMM15,U64 [RAX]\n"
"MOVSS XMM15,U32 [RAX]\n"
"MOVSS U32 [RAX],XMM15\n"
"MOVSX RAX,U8 [RAX]\n"
"MOVSX EAX,U16 [RAX]\n"
"MOVSXD RAX,U32 [RAX]\n"
"MOVZX RAX,U8 [RAX]\n"
"MOVZX EAX,U16 [RAX]\n"
"MUL U8 [RAX]\n"
"MUL U64 [RAX]\n"
"MULSS XMM15,U32 [RAX]\n"
"MULSD XMM15,U64 [RAX]\n"
"NEG U8 [RAX]\n"
"NEG U64 [RAX]\n"
"NOP\n"
"OR U8 [RAX],AL\n"
"OR U64 [RAX],RAX\n"
"OR AL,U8 [RAX]\n"
"OR RAX,U64 [RAX]\n"
"OR AL,1\n"
"OR RAX,0x123\n"
"OR RAX,1\n"
"OR BL,1\n"
"OR RBX,0x123\n"
"OR RBX,1\n"
"OUT 1,AL\n"
"OUT 1,EAX\n"
"OUT DX,AL\n"
"OUT DX,EAX\n"
"OUTSB\n"
"OUTSW\n"
"OUTSD\n"
"PACKSSDW MM7,MM7\n"
"PACKUSWB MM7,MM7\n"
"PADDB MM7,U64 [RAX]\n"
"PADDD MM7,U64 [RAX]\n"
"PADDQ MM7,U64 [RAX]\n"
"PADDSB MM7,U64 [RAX]\n"
"PADDSW MM7,U64 [RAX]\n"
"PADDW MM7,U64 [RAX]\n"
//"PALIGNR MM7,U64 [RAX]\n"
"PAND MM7,U64 [RAX]\n"
"PANDN MM7,U64 [RAX]\n"
"PAUSE\n"
"PAVGB MM7,U64 [RAX]\n"
"PAVGW MM7,U64 [RAX]\n"
"PCMPEQB MM7,U64 [RAX]\n"
"PCMPEQD MM7,U64 [RAX]\n"
"PCMPEQW MM7,U64 [RAX]\n"
"PCMPGTB MM7,U64 [RAX]\n"
"PCMPGTDMM7,U64 [RAX]\n"
"PCMPGTWMM7,U64 [RAX]\n"
"PEXTRB U8 [RAX],XMM15,3\n"
"PEXTRD EAX,XMM15,3\n"
"PEXTRQ RAX,XMM15,3\n"
"PINSRB XMM15,U8 [RAX],3\n"
"PINSRD XMM15,EAX,3\n"
"PINSRQ XMM15,RAX,3\n"
"PINSRW XMM15,U16[RAX],3\n"
"PMADDWD MM7,U64 [RAX]\n"
"PMAXSW MM7,U64 [RAX]\n"
"PMAXUB MM7,U64 [RAX]\n"
"PMINSW MM7,U64 [RAX]\n"
"PMINUB MM7,U64 [RAX]\n"
"PMOVMSKB RAX,MM7\n"
"PMULHUW MM7,U64[RAX]\n"
"PMULHW MM7,U64[RAX]\n"
"PMULLW MM7,U64[RAX]\n"
"PMULUDQ MM7,U64 [RAX]\n"
"POP R15\n"
"POP U32 [RAX]\n"
"POP U64 [RAX]\n"
"POPCNT RAX,U64[RAX]\n"
"POPF\n"
"PORMM7,U64[RAX]\n"
"PREFETCHNTA U8 [RAX]\n"
//"PREFETCHNT0 U8 [RAX]\n"
//"PREFETCHNT1 U8 [RAX]\n"
//"PREFETCHNT2 U8 [RAX]\n"
"PSADBW MM7,U64 [RAX]\n"
"PSADBW MM7,U64 [RAX]\n"
"PSLLD MM7,3\n"
"PSLLD XMM7,3\n"
"PSLLD MM7,U64 [RAX]\n"
"PSLLQ MM7,3\n"
"PSLLQ XMM7,3\n"
"PSLLQ MM7,U64 [RAX]\n"
"PSLLW MM7,3\n"
"PSLLW XMM7,3\n"
"PSLLW MM7,U64 [RAX]\n"
"PSRAD MM7,3\n"
"PSRAD XMM7,3\n"
"PSRAD MM7,U64 [RAX]\n"
"PSRAW MM7,3\n"
"PSRAW XMM7,3\n"
"PSRAW MM7,U64 [RAX]\n"
"PSRLD MM7,3\n"
"PSRLD XMM7,3\n"
"PSRLD MM7,U64 [RAX]\n"
"PSRLDQ XMM7,3\n"
"PSRLQ MM7,3\n"
"PSRLQ XMM7,3\n"
"PSRLQ MM7,U64 [RAX]\n"
"PSRLW MM7,3\n"
"PSRLW XMM7,3\n"
"PSRLW MM7,U64 [RAX]\n"
"PSUBB MM7,U64 [RAX]\n"
"PSUBW MM7,U64 [RAX]\n"
"PSUBD MM7,U64 [RAX]\n"
"PSUBQ MM7,U64 [RAX]\n"
"PSUBSB MM7,U64 [RAX]\n"
"PSUBSW MM7,U64 [RAX]\n"
"PSUBUSB MM7,U64 [RAX]\n"
"PSUBUSW MM7,U64 [RAX]\n"
"PUNPCKHBW MM7,U64 [RAX]\n"
"PUNPCKHDQ MM7,U64 [RAX]\n"
"PUNPCKHWD MM7,U64 [RAX]\n"
"PUNPCKLBW MM7,U64 [RAX]\n"
"PUNPCKLDQ MM7,U64 [RAX]\n"
"PUNPCKLWD MM7,U64 [RAX]\n"
"PUSH U64[RAX]\n"
"PUSH 0x1122\n"
"PUSH 0x11223344\n"
"PUSH 8\n"
"PUSH U32[RAX]\n"
"PUSHFQ\n"
"PXOR MM7,U64 [RAX]\n"
"RCL BL,2\n"
"RCL BX,2\n"
"RCL BL,1\n"
"RCL BX,1\n"
"RCL BL,CL\n"
"RCL BX,CL\n"
"RCPSS XMM1,XMM2\n"
"RCR BL,2\n"
"RCR BX,2\n"
"RCR BL,1\n"
"RCR BX,1\n"
"RCR BL,CL\n"
"RCR BX,CL\n"
"RET1 16\n"
"ROL BL,2\n"
"ROL BX,2\n"
"ROL BL,1\n"
"ROL BX,1\n"
"ROL BL,CL\n"
"ROL BX,CL\n"
"ROR BL,2\n"
"ROR BX,2\n"
"ROR BL,1\n"
"ROR BX,1\n"
"ROR BL,CL\n"
"ROR BX,CL\n"
"ROUNDSD XMM1,XMM2,3\n"
"ROUNDSS XMM1,U32 [RAX],3\n"
"RSQRTSS XMM1,U32 [RAX]\n"
"SAL BL,3\n"
"SAL BX,3\n"
"SAL BL,1\n"
"SAL BX,1\n"
"SAL BL,CL\n"
"SAL BX,CL\n"
"SAR BL,3\n"
"SAR BX,3\n"
"SAR BL,1\n"
"SAR BX,1\n"
"SAR BL,CL\n"
"SAR BX,CL\n"
"SBB U8 [RAX],AL\n"
"SBB U16 [RAX],AX\n"
"SBB AL,U8 [RAX]\n"
"SBB AX,U16 [RAX]\n"
"SBB AL,1\n"
"SBB RAX,0x123\n"
"SBB U8 [RAX],1\n"
"SBB U16 [RAX],0x123\n"
"SBB U16 [RAX],1\n"
"SCASB\n"
"SCASD\n"
"SCASQ\n"
"SCASW\n"
"SETC AL\n"
"SETNA AL\n"
"SETNGE AL\n"
"SETNG AL\n"
"SETNC AL\n"
"SETA AL\n"
"SETGE AL\n"
"SETG AL\n"
"SETNO AL\n"
"SETPO AL\n"
"SETNS AL\n"
"SETNE AL\n"
"SETO AL\n"
"SETPE AL\n"
"SETS AL\n"
"SETE AL\n"
//"SFENCE\n"
"SHLD U16 [RAX],AX,3\n"
"SHLD U16 [RAX],AX,CL\n"
"SHR BL,3\n"
"SHR BX,3\n"
"SHR BL,1\n"
"SHR BX,1\n"
"SHR BL,CL\n"
"SHR BX,CL\n"
"SQRTSD XMM7,U64[RAX]\n"
"SQRTSS XMM7,U32[RAX]\n"
"STI\n"
"STD\n"
"STC\n"
"STOSB\n"
"STOSD\n"
"STOSW\n"
"STOSQ\n"
"SUB U8 [RAX],AL\n"
"SUB U16 [RAX],AX\n"
"SUB AL,U8 [RAX]\n"
"SUB AX,U16 [RAX]\n"
"SUB AL,1\n"
"SUB RAX,0x123\n"
"SUB U8 [RAX],1\n"
"SUB U16 [RAX],0x123\n"
"SUB U16 [RAX],1\n"
"SUBSD XMM15,U64 [RAX]\n"
"SUBSS XMM15,U32 [RAX]\n"
"TEST U8 [RAX],AL\n"
"TEST U16 [RAX],AX\n"
"TEST AL,1\n"
"TEST RAX,0x123\n"
"TEST U8 [RAX],1\n"
"TEST U16 [RAX],0x123\n"
"TEST U16 [RAX],1\n"
"UCOMISD XMM15,U64 [RAX]\n"
"UCOMISS XMM15,U32 [RAX]\n"
"UNPCKHPS XMM15,U64 [RAX]"
"VERR U16 [RAX]\n"
"VERW U16 [RAX]\n"
"WBINVD\n"
"XADDU8 [RAX],BL\n"
"XADDU16 [RAX],BX\n"
"XCHG BL,U8 [RAX]\n"
"XCHG AX,U16 [RAX]\n"
"XCHG R15,RAX\n"
"XLATB\n"
"XOR U8 [RAX],AL\n"
"XOR U16 [RAX],AX\n"
"XOR AL,U8 [RAX]\n"
"XOR AX,U16 [RAX]\n"
"XOR AL,1\n"
"XOR RAX,0x123\n"
"XOR U8 [RAX],1\n"
"XOR U16 [RAX],0x123\n"
"XOR U16 [RAX],1\n"
"_ONION::\n"
"MOV RAX,123\n"
"RET\n"
"_NOTHING2::"
"MOVRAX,U64 0[RSP]\n"
"MOVRAX,U64 0x10[RSP]\n"
"MOVRAX,U64 0x100[RSP]\n"
"MOVRAX,U64 [RAX]\n"
"MOVRAX,U64 0x10[RAX]\n"
"MOVRAX,U64 0x100[RAX]\n"
"MOVRAX,U64 [R8*8]\n"
"MOVRAX,U64 0x10[R8*8]\n"
"MOVRAX,U64 0x100[R8*8]\n"
"MOVRAX,U64 [0x1122]\n"
"};;",
    CCF_DONT_FREE_BUF,
    "TEST.HC"
  );
  Lex(cctrl);
  blob=PrsAsmBlk(cctrl);
  oret=ret=blob->data;
  JITFixupAiwniosAOTBlob(cctrl,blob,ret);
  CmpCtrlDel(cctrl);
  while(ret<oret+blob->len)
    ret=DisasOpc(DocPut,ret);
  AiwniosAOTBlobDel(blob);
  return oret;
}
//AsmTest;
