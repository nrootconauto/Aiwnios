#include"ArmImmTab.HC";

class CToFill:CQue {
  CHashExport *h;
};
class CARMAddrMode {
  U64 imm;
  CCodeMisc *label;
  CAsmUndefHash *local_asm_undef_hash;
  CAsmUndefHash *glbl_asm_undef_hash;
  I64 (*machine_code)();
};

class CAsmPatch:CQue  {
  I64 args[16];
  CARMAddrMode imodes[16];  
  CArm64Opc *opcode;
  I64 addr,sz;
};
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

I64 Val2ImmSImmR(I64 val,I64 width=64) {
  I64 idx=5334;
  I64 mask=-1;
  if(width<64)
    mask=1<<width-1;
  while((idx-=2)>=0) {
    if(arm_imm_table[idx]&mask==val)
      break;
  }
  if(idx<0)
    return -1;
  return arm_imm_table[idx+1];
}
I64 Arm64SetValue(CArm64Opc *opc,U32 inst,I64 which,I64 value,I64 occ=1) {
  I64 ret=inst;
  I64 o;
  I64 off;
  for(o=0;o!=16;o++) {
    if(opc->operands[o].type==which) {
      if(!--occ) {
        if(opc->operands[o].offset)
	  off=opc->operands[o].offset;
        else
	  off=0;
        switch(which) {
	  case ARM64_OP_IMM9:
	    value&=1<<9-1;
	    break;
	  case ARM64_OP_IMM7:
	    value&=1<<7-1;
	    break;
	  case ARM64_OP_IMM19:
	    value&=1<<19-1;
	    break;
	  case ARM64_OP_REG:
	    value&=1<<5-1;
	    break;
	  case ARM64_OP_B5:
bt:
	    if(value)
	      Bts(&ret,off);
	    else
	      Btr(&ret,off);
	    goto fin;
	  case ARM64_OP_B40:
	    value&=0b1111;
	    break;
	  case ARM64_OP_IMM14:
	    value&=1<<14-1;
	    break;
	  case ARM64_OP_COND:
	    value&=0b1111;
	    break;
	  case ARM64_OP_R:
	  case ARM64_OP_IMM26:
	    value&=1<<26-1;
	    break;
	  case ARM64_OP_IMMLO:
	    value&=1<<2-1;
	    break;
	  case ARM64_OP_IMMHI:
	    value&=1<<19-1;
	    break;
	  case ARM64_OP_IMMS:
	  case ARM64_OP_IMMR:
	  case ARM64_OP_OPTION:
	  case ARM64_OP_SHIFT:
	    value&=0b11;
	    break;
	  case ARM64_OP_S:
	    goto bt;
	  case ARM64_OP_IMM5:
	    value&=1<<5-1;
	    break;
	  case ARM64_OP_NZCV:
	  case ARM64_OP_IMM4:
	    value&=1<<4-1;
	    break;
	  case ARM64_OP_Q:
	  case ARM64_OP_IMM16:
	    value&=1<<16-1;
	    break;
	  case ARM64_OP_IMM12:
	    value&=1<<12-1;
	    break;
        }
        ret|=value<<off;
      }
    }
  }
fin:;
  return ret;
}

U32 *AsmOpc(CArm64Opc *opc,U32 *ptr,I64 shmod,I64 *argv) {
  U32 ret=opc->value;
  I64 mul=1;
  switch(opc->num) {
    case 6 ... 9:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0]);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM19,argv[1]);
      break;
    case 11 ... 12:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0]);
      ret=Arm64SetValue(opc,ret,ARM64_OP_B5,argv[1]>>5);
      ret=Arm64SetValue(opc,ret,ARM64_OP_B40,argv[1]&(1<<5-1));
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM14,argv[2]);
      break;
    case 14:
      ret=Arm64SetValue(opc,ret,ARM64_OP_COND,argv[0]);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM19,argv[1]>>2);
      break;
    case 16 ... 23:
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM16,argv[0]);
      break;
    case 36 ... 40:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0]);
      break;
    case 41 ... 43:
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM26,argv[0]);
      break;
    case 46 ... 77:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[3],4);
      break;
    case 79 ... 85:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0]);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM19,argv[1]);
      break;
    case 87 ... 88:
    case 89 ... 90:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/4);
      break;
    case 91 ... 94:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/8);
      break;
    case 95 ... 96:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/16);
      break;
    case 98 ... 102:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/4);
      break;
    case 103 ... 106:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/8);
      break;
    case 107 ... 108:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/16);
      break;
    case 110 ... 114:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/4);
      break;
    case 115 ... 118:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/8);
      break;
    case 119 ... 120:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/16);
      break;
    case 122 ... 126:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/4);
      break;
    case 127 ... 130:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/8);
      break;
    case 131 ... 132:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM7,argv[3]/16);
      break;
    case 134 ... 157:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM9,argv[2]/8);
      break;
    case 158 ... 172:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM9,argv[2]);
      break;
    case 173 ... 177:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM9,argv[2]);
      break;
    case 178 ... 181:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM9,argv[2]);
      break;

    case 183 ... 196:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM9,argv[2]);
      break;
    case 197 ... 219:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM9,argv[2]);
      break;
    case 220 ... 245:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      ret|=(argv[3]&0b111)<<13;
      if(argv[4]) {
        ret|=1<<12;
        switch(opc->num) {
	  case 220 ... 226:
	    mul=1;
	    break;
	  case 227 ... 228:
	    mul=16;
	    break;
	  case 229 ... 234:
	    mul=2;
	    break;
	  case 235 ... 239:
	    mul=4;
	    break;
	  case 240 ... 245:
	    mul=8;
	    break;
        }
        if(argv[4]!=mul)
	  return NULL;
      }
      break;
    start:
      case 246 ... 251:
        mul=1;
        break;
      case 252 ... 253:
        mul=16;
        break;
      case 254 ... 259:
        mul=2;
        break;
      case 260 ... 264:
        mul=4;
        break;
      case 265 ... 269:
        mul=8;
        break;
    end:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM12,argv[2]/mul);
      break;
    case 272:
      argv[1]-=ToI64(ptr);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMMLO,argv[1]);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMMHI,argv[1]>>2);
      break;
    case 273:
      argv[1]-=ToI64(ptr);
      argv[1]>>=12;
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMMLO,argv[1]);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMMHI,argv[1]>>2);
      break;
    case 275 ... 282:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM12,argv[2]);
      break;
    case 284 ... 287:
      ret|=Val2ImmSImmR(argv[2],32);
      if(0>Val2ImmSImmR(argv[2],32))
        return NULL;
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      break;
    case 288 ... 291:
      ret|=Val2ImmSImmR(argv[2]);
      if(0>Val2ImmSImmR(argv[2],32))
        return NULL;
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      break;

    case 293 ... 298:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_IMM16,argv[1],1);
      break;
    case 300 ... 302:
      ret|=Val2ImmSImmR(argv[2],32);
      if(0>Val2ImmSImmR(argv[2],32))
        return NULL;
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      break;
    case 303 ... 305:
      ret|=Val2ImmSImmR(argv[2]);
      if(0>Val2ImmSImmR(argv[2]))
        return NULL;
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      break;
    case 307 ... 308:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      break;
    case 310 ... 318:
    case 319 ... 326:
    case 328 ... 331:
    case 332 ... 335:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_SHIFT,shmod,1);
      ret|=argv[3]&(1<<6-1);
      break;
    case 337 ... 344:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_OPTION,shmod,1);
      break;
    case 346 ... 353:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      break;
//TODO conditional compare
    case 364 ... 372:
      ret=Arm64SetValue(opc,ret,ARM64_OP_COND,argv[0],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[3],1);
      break;
    case 374 ... 383:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],4);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[3],2);
      break;
    case 385 ... 404:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      break;
    case 406 ... 434:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      break;
    case 441 ... 458:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      break;
    case 460 ... 461:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_COND,shmod,1);
      break;
    case 466 ... 473:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      break;
    case 475 ... 557:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      break;
    case 559 ... 566:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],4);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[3],2);
      break;
    case 568 ... 603:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],3);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[2],1);
      break;
    case 604 ... 714:
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[0],2);
      ret=Arm64SetValue(opc,ret,ARM64_OP_REG,argv[1],1);
      break;
  }
  *ptr=ret;
  return ptr+1;
}
Bool PrsAsmImm(CCmpCtrl *cc,CARMAddrMode *arg,U8 reg_type='X')
{
  I64 old_flags=cc->flags;
  arg->local_asm_undef_hash=NULL;
  arg->glbl_asm_undef_hash=NULL;
  cc->asm_undef_hash=NULL;
  cc->abs_cnts=0;
  cc->flags&=~(CCF_UNRESOLVED+CCF_LOCAL);
  cc->flags|=CCF_ASM_EXPRESSIONS;
  if(cc->token=='[')
      LexExcept(cc,"Expected an expression at ");
//Ignore register expressions
  if(cc->token==TK_IDENT) {
    if(cc->cur_str[0]==reg_type&&0<=Str2I64(cc->cur_str+1)<=31)
      LexExcept(cc,"Expected an expression at ");
  }
  if (!IsLexExpression2Bin(cc,&arg->machine_code)) {
    LexSkipEol(cc);
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
      arg->imm=(*arg->machine_code)();
      arg->glbl_asm_undef_hash=cc->asm_undef_hash;
      cc->asm_undef_hash=NULL;
      Free(arg->machine_code);
      arg->machine_code=NULL;
    }
  }
  cc->flags=old_flags;
  return arg->imm;
}
I64 PrsAsmShift(CCmpCtrl *cc) {
  U8 *str=cc->cur_str;
  if(cc->token!=TK_IDENT)
    LexExcept(cc,"Expected Shmod at ");

  if(!StrCmp(str,"LSL")) {
    Lex(cc);
    return 0b00;
  }
  if(!StrCmp(str,"LSR")) {
    Lex(cc);
    return 0b01;
  }
  if(!StrCmp(str,"ASR")) {
    Lex(cc);
    return 0b10;
  }
  if(!StrCmp(str,"ROR")) {
    Lex(cc);
    return 0b11;
  }
  LexExcept(cc,"Expected Shmod at ");
}
I64 PrsAsmOption(CCmpCtrl *cc) {
  U8 *str=cc->cur_str;
  I64 ret=-1;
  if(cc->token!=TK_IDENT)
    LexExcept(cc,"Expected Shmod at ");
  
  if(!StrCmp(str,"UXTB")) {
    ret=0;
  pass:
    Lex(cc);
    return ret;
  }
  if(!StrCmp(str,"UXTH")) {
    ret=1;
    goto pass;
  }
  if(!StrCmp(str,"LSL")) {
    ret=0b10;
    goto pass;
  }
  if(!StrCmp(str,"UXTX")) {
    ret=0b11;
    goto pass;
  }

  if(!StrCmp(str,"SXTB")) {
    ret=0b100;
    goto  pass;
  }
  if(!StrCmp(str,"SXTH")) {
    ret=0b101;
    goto pass;
  }
  if(!StrCmp(str,"SXTW")) {
    ret=0b110;
    goto pass;
  }
  if(!StrCmp(str,"SXTX")) {
    ret=0b111;
    goto pass;
  }
  LexExcept(cc,"Expected a valid option at ");
  return 0;
}

I64 PrsAsmCond(CCmpCtrl *cc) {
  I64 tmp;
  if(cc->token==TK_IDENT) {
    if(!StrCmp("EQ",cc->cur_str))
      tmp=0;
    else if(!StrCmp("NE",cc->cur_str))
      tmp=1;
    else if(!StrCmp("CS",cc->cur_str))
      tmp=2;
    else if(!StrCmp("CC",cc->cur_str))
      tmp=3;
    else if(!StrCmp("MI",cc->cur_str))
      tmp=4;
    else if(!StrCmp("PL",cc->cur_str))
      tmp=5;
    else if(!StrCmp("VS",cc->cur_str))
      tmp=6;
    else if(!StrCmp("VC",cc->cur_str))
      tmp=7;
    else if(!StrCmp("HI",cc->cur_str))
      tmp=8;
    else if(!StrCmp("LS",cc->cur_str))
      tmp=9;
    else if(!StrCmp("GE",cc->cur_str))
      tmp=0xa;
    else if(!StrCmp("LT",cc->cur_str))
      tmp=0xb;
    else if(!StrCmp("GT",cc->cur_str))
      tmp=0xc;
    else if(!StrCmp("LE",cc->cur_str))
      tmp=0xd;
    else if(!StrCmp("AL",cc->cur_str))
      tmp=0xe;
    else if(!StrCmp("NV",cc->cur_str))
      tmp=0xf;
    else 
      goto fail;
    Lex(cc);
  } else {
    goto fail;
  }
  return tmp;
fail:;
  LexExcept(cc,"Expected valid condition at ");
}
I64 PrsAsmReg(CCmpCtrl *cc,I64 type='X') {
  I64 ret=-1;
  if(cc->token==TK_IDENT) {
    if(cc->cur_str[0]==type) {
      ret=Str2I64(cc->cur_str+1);
      if(ret>=32||ret<0)
        goto fail;
    } else
      goto fail;
  } else {
    goto fail;
  }
  Lex(cc);
  return ret;
fail:;
  LexExcept(cc,"Expected a register at");
}
U0 LexExpect(CCmpCtrl *cc,U64 t) {
  if(cc->token!=t)
    LexExcept(cc,"Unexpected token at ");
  Lex(cc);
}
I64 PrsARMInst(CCmpCtrl *cc,CAiwniosAOTBlob *blob) {
  I64 old_flags=cc->flags;
  Bool old_silent=Silent(TRUE);
  CArm64Opc *opc;
  CARMAddrMode imodes[16];
  CAsmPatch *patch;
  MemSet(imodes,0,sizeof(CARMAddrMode)*16);
  I64 argc=0,argv[16],mod=0,type,tmp;
  I64 ret_opc=-1;
  I64 attempt=1;
  I64 passed=0;
  if(cc->token==TK_IDENT) {
    if(opc=HashFind(cc->cur_str,Fs->hash_table,HTT_OPCODE)) {
again:;
      argc=0;
      LexPush(cc);
      Lex(cc);
      while(opc=HashFind(opc->str,Fs->hash_table,HTT_OPCODE,attempt++)) {
        U64 specific=opc->specific;
        if(!specific)
	  specific='X';
        try {
	  passed=1;
	  switch(opc->num) {
	    case 6 ... 9:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmImm(cc,&imodes[1],specific);
	      break;
	    case 11 ... 12:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmImm(cc,&imodes[1],specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmImm(cc,&imodes[2],specific);
	      break;
	    case 14:
	      LexExpect(cc,'.');
	      argv[0]=PrsAsmCond(cc);
	      argv[1]=PrsAsmImm(cc,&imodes[1],specific);
	      break;
	    case  16 ... 23:
	      LexExpect(cc,',');
	      argv[0]=PrsAsmImm(cc,&imodes[0],specific);
	      break;
	    case 36 ... 40:
	      argv[0]=PrsAsmReg(cc,specific);
	      break;
	    case 41 ... 43:
	      argv[0]=PrsAsmImm(cc,&imodes[0],specific)>>2;
	      break;
	    case 46 ... 77:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      LexExpect(cc,','); //,
	      LexExpect(cc,'['); //[
	      argv[3]=PrsAsmReg(cc,specific);
	      LexExpect(cc,']'); //]
	      break;
	    case 79 ... 85:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmImm(cc,&imodes[1],specific);
	      break;
	    case 87 ... 88:
	    case 89 ... 90:
	    case 91 ... 94:
	    case 95 ... 96:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      LexExpect(cc,'['); //[
	      argv[2]=PrsAsmReg(cc,'X');
	      LexExpect(cc,'+');// :
	      argv[3]=PrsAsmImm(cc,&imodes[3],'X');
	      LexExpect(cc,']'); //]
	      break;
	    case 110 ... 114:
	    case 115 ... 118:
	    case 119 ... 120:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,','); //,
	      LexExpect(cc,'['); //[
	      argv[2]=PrsAsmReg(cc,'X');
	      LexExpect(cc,'+');// +=
	      argv[3]=PrsAsmImm(cc,&imodes[3],'X');
	      LexExpect(cc,']'); //]
	      break;
	    case 122 ... 132:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      LexExpect(cc,'['); //[
	      argv[2]=PrsAsmReg(cc,'X');
	      LexExpect(cc,TK_ADD_EQU);// +=
	      argv[3]=PrsAsmImm(cc,&imodes[3],'X');
	      LexExpect(cc,']'); //]
	      break;
	    case 98 ... 108:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      LexExpect(cc,'['); //[
	      argv[2]=PrsAsmReg(cc,'X');
	      LexExpect(cc,','); //,
	      LexExpect(cc,TK_ADD_EQU);// +=
	      argv[3]=PrsAsmImm(cc,&imodes[3],'X');
	      LexExpect(cc,']'); //]
	      break;
	    case 134 ... 157:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      LexExpect(cc,'['); //[
	      argv[1]=PrsAsmReg(cc,'X');
	      LexExpect(cc,'+');
	      argv[2]=PrsAsmImm(cc,&imodes[2],'X');
	      LexExpect(cc,']'); //]
	      break;
	    case 183 ... 196:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      LexExpect(cc,'['); //[
	      argv[1]=PrsAsmReg(cc,'X');
	      LexExpect(cc,'+');
	      argv[2]=PrsAsmImm(cc,&imodes[2],'X');
	      LexExpect(cc,']'); //]
	      break;
	    case 158 ... 181:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      LexExpect(cc,'['); //[
	      argv[1]=PrsAsmReg(cc,'X');
	      LexExpect(cc,',');
	      LexExpect(cc,TK_ADD_EQU);
	      argv[2]=PrsAsmImm(cc,&imodes[2],'X');
	      LexExpect(cc,']'); //]
	      break;
	    case 197 ... 219:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      LexExpect(cc,'['); //[
	      argv[1]=PrsAsmReg(cc,'X');
	      LexExpect(cc,TK_ADD_EQU);
	      argv[2]=PrsAsmImm(cc,&imodes[2],'X');
	      LexExpect(cc,']'); //]
	      break;
	    case 220 ... 245:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      LexExpect(cc,'['); //[
	      argv[1]=PrsAsmReg(cc,'X');
	      LexExpect(cc,'+');
	      argv[2]=PrsAsmReg(cc,'X');
	      if(cc->token==',') {
		Lex(cc);
		argv[3]=PrsAsmOption(cc);
		if(cc->token==',') {
		  Lex(cc);
		  argv[4]=PrsAsmImm(cc,&imodes[3],'X');
		} else
		  argv[4]=0;
	      } else {
		argv[3]=0b111;
		argv[4]=0;
	      }
	      LexExpect(cc,']'); //]
	      break;
	    case 246 ... 270:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      LexExpect(cc,'['); //[
	      argv[1]=PrsAsmReg(cc,'X');
	      LexExpect(cc,'+');
	      argv[2]=PrsAsmImm(cc,&imodes[2],'X');
	      LexExpect(cc,']'); //]
	      break;
	    case 272:
	    case 273:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmImm(cc,&imodes[1],specific);
	      break;
	    case 275 ... 282:
	    case 284 ... 287:
	    case 288 ... 291:
	    case 300 ... 302:
	    case 303 ... 305:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmImm(cc,&imodes[2],specific);
	      break;
	    case 293 ... 298:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmImm(cc,&imodes[1],specific);
	      LexExpect(cc,',');
	      if(cc->token!=TK_I64)
		LexExcept(cc,"Expected a shift amount at ");
	      argv[2]=cc->cur_i64;
	      Lex(cc);
	      break;
	    case 307 ... 308:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      break;
	    case 310 ... 318:
	    case 319 ... 326:
	    case 328 ... 331:
	    case 332 ... 335:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      if(cc->token==',') {
	        LexExpect(cc,',');
	        argv[3]=PrsAsmShift(cc);
	        LexExpect(cc,',');
	        argv[4]=PrsAsmImm(cc,&imodes[4],specific);
	      } else {
	        argv[3]=0;
	        argv[4]=0;
	      }
	      break;
	    case 337 ... 344:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[3]=PrsAsmOption(cc);
	      break;
	    case 346 ... 353:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      break;
	    case 364 ... 372:
	      LexExpect(cc,'.');
	      argv[0]=PrsAsmCond(cc);
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[3]=PrsAsmReg(cc,specific);
	      break;
	    case 374 ... 383:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[3]=PrsAsmReg(cc,specific);
	      break;
	    case 385 ... 404:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      break;
	    case 406 ... 434:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      break;
	    case 441 ... 458:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      break;
	    case 460 ... 461:
	      LexExpect(cc,'.');
	      argv[0]=PrsAsmCond(cc);
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[3]=PrsAsmReg(cc,specific);
	      break;
	    case 466 ... 473:
	    case 475 ... 557:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      break;
	    case 559 ... 566:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[3]=PrsAsmReg(cc,specific);
	      break;
	    case 568 ... 603:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[2]=PrsAsmReg(cc,specific);
	      break;
	    case 604 ... 714:
	      argv[0]=PrsAsmReg(cc,specific);
	      LexExpect(cc,',');
	      argv[1]=PrsAsmReg(cc,specific);
	      break;
	    default:
	      LexExcept(cc,"Un-implemented Opcode at ");
	  }
        } catch {
	  Fs->catch_except=TRUE;
	  passed=0;
        }
        if(passed) {
	  ret_opc=0; //Clear -1 bit
	  if(!AsmOpc(opc,&ret_opc,mod,argv)) {
	    ret_opc=-1;
	    goto fail;
	  }
	  LexPopNoRestore(cc);
	  break;
        } else {
fail:;
	  LexPopRestore(cc);
	  goto again;
        }
      }
    }
  }
  if(ret_opc>-1) {
    patch=CAlloc(sizeof CAsmPatch);
    patch->addr=blob->len;
    patch->sz=4;
    patch->opcode=opc;
    MemCpy(patch->args,argv,16*8);
    MemCpy(patch->imodes,imodes,16*sizeof(CARMAddrMode));
    AddBytesToBuf(&blob->data,&blob->len,4,&ret_opc);
    QueIns(patch,blob->asm_patches->last);
  }
  cc->flags=old_flags;
  Silent(old_silent);
  return ret_opc;
}


U0 AsmResolvePatches(CCmpCtrl *cc,CQue *patches) {
  CAsmPatch *patch;
  I64 b,i,a;
  CAsmUndefHash *tab,*cur;
  CHashImport *h;
  CMemberLst *mlst;
  CHashExport *exp;
  CCodeMisc *cm;
  CArm64Opc *opc;
  CARMAddrMode *imode;
  Bool local;
  patch=patches->next;
  while(patch!=patches) {
    for(a=0;a!=16;++a) {
      imode=&patch->imodes[a];
      if(!imode->machine_code) goto nxt;
      if(tab=imode->local_asm_undef_hash) {
        local=TRUE;
      } else {
        tab=imode->glbl_asm_undef_hash;
        local=FALSE;
      }
      if(!tab) {
	goto do_it;
      }
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
do_it:
      imode->imm=(*imode->machine_code)();
      Free(imode->machine_code);
      imode->machine_code=NULL;
      if(tab) LinkedLstDel(tab);
      patch->args[a]=imode->imm;
nxt:;
    }
    patch=patch->next;
  }
}
U0 JITFixupAiwniosAOTBlob(CCmpCtrl *cc,CAiwniosAOTBlob *blob,U8 *buf) {
  CToFill *to_fill=blob->to_fill,*tmp_to_fill;
  CAsmPatch *patch,*next_patch;
  Bool rel;
  I64 tmp,sz;
  CArm64Opc *opc,a;
  CARMAddrMode *amode;
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
    for(a=0;a!=16;++a) {
      amode=&patch->imodes[a];
      if(opc=patch->opcode) {
        switch(opc->num) {
	  case 6 ... 9:
	  case 14:
	  case 79 ... 85:
	    if(a==1) {
	      amode->imm-=ToI64(&buf[patch->addr]);
	      patch->args[a]=amode->imm;
	    }
	    break;
	  case 11 ... 12:
	    if(a==2) {
	      amode->imm-=ToI64(&buf[patch->addr]);
	      patch->args[a]=amode->imm;
	    }
	    break;
	  case 41 ... 43:
	    if(a==0) {
	      amode->imm-=ToI64(&buf[patch->addr]);
	      patch->args[a]=amode->imm>>2;
	    }
	    break;
        }
      }
    }
    if(patch->opcode)
      AsmOpc(patch->opcode,&buf[patch->addr],0,patch->args);
    else
      MemCpy(&buf[patch->addr],&patch->args[a],patch->sz);
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
  CARMAddrMode *tmp_mode;
  CAsmPatch *patch,*next_patch;
  QueInit(asm_patches);
  QueInit(to_fill);
  ret_blob=CAlloc(sizeof CAiwniosAOTBlob);
  ret_blob->to_fill=to_fill;
  ret_blob->asm_patches=asm_patches;
  ret_blob->len=0;  
  ret_blob->data=buf;
enter:
  if(PrsARMInst(cc,ret_blob)>=0) {
  } else if(cc->token==TK_IDENT&&cc->hash_entry&&cc->hash_entry->type&(HTT_ASM_KEYWORD|HTT_KEYWORD)) {
    if(!StrCmp(cc->hash_entry->str,"IMPORT")) {
//TODO
    } else if(!StrCmp(cc->hash_entry->str,"BINFILE")) {
      if(Lex(cc)!=TK_STR)
        LexExcept(cc,"Expected a filename.");
      if(!FileFind(cc->cur_str))
        LexExcept(cc,"File not found");
      fbuf=FileRead(cc->cur_str,&sz);
      AddBytesToBuf(&ret_blob->data,&ret_blob->len,sz,fbuf);
      Free(fbuf);
      goto enter;
    } else if(!StrCmp(cc->hash_entry->str,"DU8")) {
      sz=1;
duxx:
      Lex(cc);
//Perhaps is an unlinked label
      while(TRUE) {
        if(cc->token==TK_STR) {
	  idx=0;
	  while(tmp=cc->cur_str[idx++])
	    AddBytesToBuf(&ret_blob->data,&ret_blob->len,sz,&tmp);
	  Lex(cc);
        } else {
	  tmp_mode=CAlloc(sizeof CARMAddrMode);
	  patch=CAlloc(sizeof CAsmPatch);
	  tmp=PrsAsmImm(cc,&patch->imodes[0]);
	  patch->addr=ret_blob->len;
	  patch->sz=sz;
	  QueIns(patch,asm_patches);
	  AddBytesToBuf(&ret_blob->data,&ret_blob->len,sz,&tmp);
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
      goto enter;
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
    goto enter;
  } else if(cc->token==';') {
    Lex(cc);
    goto enter;
  } else if(cc->token!='}') {
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
