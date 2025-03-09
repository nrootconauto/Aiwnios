#ifndef BOOTSTRAP
I64 Sext(I64 v,I64 bits) {
  if(v&(1<<(bits-1)))
    return -(1<<bits-v);
  return v;
}
I64 Arm64GetValue(CArm64Opc *opc,U32 inst,I64 which,I64 occ=1) {
  I64 o;
  for(o=0;o!=16;o++) {
    if(opc->operands[o].type==which) {
      if(!--occ) {
        if(opc->operands[o].offset)
	  inst>>=opc->operands[o].offset;
        switch(which) {
	  case ARM64_OP_IMM9:
	    return Sext(inst&(1<<9-1),9);
	  case ARM64_OP_IMM7:
	    return Sext(inst&(1<<7-1),7);
	  case ARM64_OP_IMM19:
	    return Sext(inst&(1<<19-1),19);
	  case ARM64_OP_REG:
	    return inst&(1<<5-1);
	  case ARM64_OP_B5:
	    return inst&1;
	  case ARM64_OP_B40:
	    return inst&0b1111;
	  case ARM64_OP_IMM14:
	    return Sext(inst&(1<<14-1),14);
	  case ARM64_OP_COND:
	    return inst&0b1111;
	  case ARM64_OP_R:
	  case ARM64_OP_IMM26:
	    return Sext(inst&(1<<26-1),26);
	  case ARM64_OP_IMMLO:
	    return inst&(1<<2-1);
	  case ARM64_OP_IMMHI:
	    return inst&(1<<19-1);
	  case ARM64_OP_IMMS:
	  case ARM64_OP_IMMR:
	  case ARM64_OP_OPTION:
	  case ARM64_OP_SHIFT:
	    return inst&0b11;
	  case ARM64_OP_S:
	    return inst&0b1;
	  case ARM64_OP_IMM5:
	    return Sext(inst&(1<<5-1),5);
	  case ARM64_OP_NZCV:
	  case ARM64_OP_IMM4:
	    return Sext(inst&(1<<4-1),4);
	  case ARM64_OP_Q:
	  case ARM64_OP_IMM16:
	    return Sext(inst&(1<<16-1),16);
	  case ARM64_OP_IMM12:
	    return Sext(inst&(1<<12-1),12);
        }
      }
    }
  }
  return -1;
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
	    value=1<<19-1;
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
	  case ARM64_OP_IMM12:
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
U8 *ReadALine(U8 **_ptr) {
  U8 *ptr=*_ptr;
  if(!*ptr) return NULL;
  U8 *en=StrFirstOcc(ptr,"\n"),*ret;
  if(!en) {
    en=StrLen(ptr)+ptr;
  }
  ret=MAlloc(en-ptr+1);
  MemCpy(ret,ptr,en-ptr);
  ret[en-ptr]=0;
  if(*en=='\n')
    en++;
  *_ptr=en;
  return ret;
}
U8 *ReadAFeild(U8 **_ptr) {
  U8 *ptr=*_ptr;
  U8 *en=StrFirstOcc(ptr,","),*ret;
  if(!en) {
    en=StrLen(ptr)+ptr;
  }
  ret=MAlloc(en-ptr+1);
  MemCpy(ret,ptr,en-ptr);
  ret[en-ptr]=0;
  if(*en==',')
    en++;
  *_ptr=en;
  return ret;
}
static CArm64Opc **arm64_opcodes_flat;
U0 LoadOpcodes() {
  if(!FileFind("/Src/AArch64_ops.csv"))
    return;
  U8 *optr=FileRead("/Src/AArch64_ops.csv"),*ptr=optr;
  StrUtil(optr,SUF_REM_CTRL_CHARS);
  I64 max_opc_cnt=StrOcc(optr,'\n')+1;
  U8 *oln,*ofeild,*ln,*field;
  U8 *name,*prepend,*append;
  CArm64Opc *opc,**store_at;
  I64 operand,bit,num,bit2;
  store_at=arm64_opcodes_flat=CAlloc((max_opc_cnt+1)*8);
  Free(ReadALine(&ptr)); //Skip table header line
  while(oln=ReadALine(&ptr)) {
    ln=oln;
//num
    field=ReadAFeild(&ln);
    num=Str2I64(field);
    Free(field);
//in use
    field=ReadAFeild(&ln);
    Free(field);
//blah*2
    field=ReadAFeild(&ln);
    Free(field);
    field=ReadAFeild(&ln);
    Free(field);
//Opcode name
    name=ReadAFeild(&ln);
    if(!StrLen(name)) {
      Free(name);
      goto next;
    }
//Prepend
    prepend=ReadAFeild(&ln);
//Append
    append=ReadAFeild(&ln);
    opc=CAlloc(sizeof CArm64Opc);
    opc->num=num;
    opc->str=MStrPrint("%s",name);
    opc->type=HTT_OPCODE;
    *store_at++=opc;
    HashAdd(opc,cmp.asm_hash);
    Free(name);
    Free(prepend);
    Free(append);
//blah*2
    field=ReadAFeild(&ln);
    Free(field);
    field=ReadAFeild(&ln);
    Free(field);
//Comment
    field=ReadAFeild(&ln);
    Free(field);
//useless
    field=ReadAFeild(&ln);
    Free(field);
//Get first 9 bits
    operand=0;
    for(bit=31;bit>=0;--bit) {
      field=ReadAFeild(&ln);
      for(bit2=0;ln[bit2]==',';bit2++)
        ;
      bit2=bit-bit2;
      if(bit2<0) bit2=0;
      if(!StrLen(field)) {
      } else if(!StrCmp(field,"1")) {
        Bts(&opc->value,bit);
        Bts(&opc->mask,bit);
      } else if(!StrCmp(field,"0")) {
        Bts(&opc->mask,bit);
      } else if(!StrCmp(field,"-")) {
      } else if(!StrCmp(field,"b5")) {
	opc->operands[operand].type=ARM64_OP_B5;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"b40")) {
	opc->operands[operand].type=ARM64_OP_B40;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imm14")) {
	opc->operands[operand].type=ARM64_OP_IMM14;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"cond")) {
	opc->operands[operand].type=ARM64_OP_COND;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imm19")) {
	opc->operands[operand].type=ARM64_OP_IMM19;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imm16")) {
	opc->operands[operand].type=ARM64_OP_IMM16;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"Rt2")
	||!StrCmp(field,"Rt")
	||!StrCmp(field,"Rn")
	||!StrCmp(field,"Rs")
	||!StrCmp(field,"Rm")
	||!StrCmp(field,"Rd")
	||!StrCmp(field,"Ra")) {
	opc->operands[operand].type=ARM64_OP_REG;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imm26")) {
	opc->operands[operand].type=ARM64_OP_IMM26;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imm7")) {
	opc->operands[operand].type=ARM64_OP_IMM7;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imm9")) {
	opc->operands[operand].type=ARM64_OP_IMM9;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imm12")) {
	opc->operands[operand].type=ARM64_OP_IMM12;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"immlo")) {
	opc->operands[operand].type=ARM64_OP_IMMLO;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"immhi")) {
	opc->operands[operand].type=ARM64_OP_IMMHI;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imms")) {
	opc->operands[operand].type=ARM64_OP_IMMS;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"immr")) {
	opc->operands[operand].type=ARM64_OP_IMMR;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"option")) {
	opc->operands[operand].type=ARM64_OP_OPTION;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"shift")) {
	opc->operands[operand].type=ARM64_OP_SHIFT;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imm5")) {
	opc->operands[operand].type=ARM64_OP_IMM5;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"imm4")) {
	opc->operands[operand].type=ARM64_OP_IMM4;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"nzcv")) {
	opc->operands[operand].type=ARM64_OP_NZCV;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"Q")) {
	opc->operands[operand].type=ARM64_OP_Q;
        opc->operands[operand++].offset=bit2;
      } else if(!StrCmp(field,"Q")) {
	opc->operands[operand].type=ARM64_OP_S;
        opc->operands[operand++].offset=bit2;
      }
      Free(field);     
    }
next:;
    Free(oln);
  }
  Free(optr);
}
U8 *ArmCond2Str(I64 cond) {
  switch(cond) {
    case 0:return "EQ";
    case 1:return "NE";
    case 2:return "CS";
    case 3:return "CC";
    case 4:return "MI";
    case 5:return "PL";
    case 6:return "VS";
    case 7:return "VC";
    case 8:return "HI";
    case 9:return "LS";
    case 0xa:return "GE";
    case 0xb:return "LT";
    case 0xc:return "GT";
    case 0xd:return "LE";
    case 0xe:return "AL";
    case 0xf:return "NV";
  }
  return NULL;
}
I64 ImmSImmR2Val(U32 inst) {
  inst=(inst>>10)&(1<<13-1);
  Bool n=!!(inst&(1<<12));
  I64 s=inst&(1<<6-1),r=(inst>>6)&(1<<6-1);
  I64 consec,base_pat,width,idx,idx2,final;
  if(n) {
    consec=s+1;
    width=64;
  } else if(s&0b111110==0b111100) {
    consec=1+(s&1);
    width=2;
  } else if(s&0b111100==0b111000) {
    consec=1+(s&0b11);
    width=4;
  } else if(s&0b111000==0b110000) {
    consec=1+(s&0b111);
    width=8;
  } else if(s&0b110000==0b100000) {
    consec=1+(s&0b1111);
    width=16;
  } else if(s&0b100000==0b000000) {
    consec=1+(s&0b11111);
    width=32;
  }
  final=0;
  for(idx=0;idx!=64/width;idx++) {
    for(idx2=0;idx2!=consec;idx2++) {
      Bts(&final,(idx*width+idx2-r)&63);
    }
  }
  return final;
}
U8 *ArmShmod2Str(I64 opt) {
  switch(opt) {
    case 0b00: return "LSL";
    case 0b01: return "LSR";
    case 0b10: return "ASR";
    case 0b11: return "ROR";
  }
  return NULL;
}
U8 *ArmOption2Str(I64 opt) {
  switch(opt) {
case 0b000: return "UXTB";
case 0b001: return "UXTH";
case 0b010: return "LSL";
case 0b011: return "UXTX";
case 0b100: return "SXTB";
case 0b101: return "SXTH";
case 0b110: return "SXTW";
case 0b111: return "SXTX";
  }
  return NULL;
}
U8 *DisasOpc(CDoc *doc,U32 *ptr) {
  U32 inst=*ptr;
  CArm64Opc **opcs,*opc;
  I64 off;
  opcs=arm64_opcodes_flat;
  if(!opcs) return ptr+1;
  while(opc=*opcs) {
    if(opc->mask&inst==opc->value) {
      DocPrint(doc,"%P\t%s ",ptr,opc->str);
      switch(opc->num) {
        case 6 ... 9:
	  DocPrint(doc,"R%d,%P",
	        Arm64GetValue(opc,inst,ARM64_OP_REG),
	        ptr+Arm64GetValue(opc,inst,ARM64_OP_IMM19));
	  break;
        case 11 ... 12:
	  DocPrint(doc,"R%d,%d,%P",
	        Arm64GetValue(opc,inst,ARM64_OP_REG),
	        (Arm64GetValue(opc,inst,ARM64_OP_B5)<<5)|Arm64GetValue(opc,inst,ARM64_OP_B40),
	        ptr+Arm64GetValue(opc,inst,ARM64_OP_IMM14));
	  break;
        case 14:
	  DocPrint(doc,"(%s),%P",
	        ArmCond2Str(Arm64GetValue(opc,inst,ARM64_OP_COND)),
	        ptr+Arm64GetValue(opc,inst,ARM64_OP_IMM19));
	  break;
        case 16 ... 23:
	  DocPrint(doc,"%P",
	        Arm64GetValue(opc,inst,ARM64_OP_IMM16));
	  break;
        case 36 ... 40:
	  DocPrint(doc,"R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG));
	  break;
        case 41 ... 43:
	  DocPrint(doc,"%P",
	        ptr+Arm64GetValue(opc,inst,ARM64_OP_IMM26));
	  break;
        case 46 ... 77:
	  DocPrint(doc,"R%d,R%d,R%d,[R%d]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,4),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3));
	  break;
        case 79 ... 85:
	  DocPrint(doc,"R%d,%P",
	        Arm64GetValue(opc,inst,ARM64_OP_REG),
	        ptr+Arm64GetValue(opc,inst,ARM64_OP_IMM19));
	  break;
        case 87 ... 88:
        case 89 ... 90:
	  DocPrint(doc,"R%d,R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*4);
	  break;
        case 91 ... 94:
	  DocPrint(doc,"R%d,R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*8);
	  break;
        case 95 ... 96:
	  DocPrint(doc,"R%d,R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*16);
	  break;
        case 98 ... 102:
	  DocPrint(doc,"R%d,R%d,[R%d,+=%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*4);
	  break;
        case 103 ... 106:
	  DocPrint(doc,"R%d,R%d,[R%d,+=%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*8);
	  break;
        case 107 ... 108:
	  DocPrint(doc,"R%d,R%d,[R%d,+=%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*16);
	  break;
        case 110 ... 114:
	  DocPrint(doc,"R%d,R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*4);
	  break;
        case 115 ... 118:
	  DocPrint(doc,"R%d,R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*8);
	  break;
        case 119 ... 120:
	  DocPrint(doc,"R%d,R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*16);
	  break;
        case 122 ... 126:
	  DocPrint(doc,"R%d,R%d,[R%d+=%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*4);
	  break;
        case 127 ... 130:
	  DocPrint(doc,"R%d,R%d,[R%d+=%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*8);
	  break;
        case 131 ... 132:
	  DocPrint(doc,"R%d,R%d,[R%d+=%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM7)*16);
	  break;
        case 134 ... 157:
	  DocPrint(doc,"R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM9)*8);
	  break;
        case 158 ... 172:
	  DocPrint(doc,"R%d,[R%,d+=%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM9));
	  break;
        case 173 ... 177:
	  DocPrint(doc,"R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM9)*4);
	  break;
        case 178 ... 181:
	  DocPrint(doc,"R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM9)*8);
	  break;
        case 183 ... 196:
	  DocPrint(doc,"R%d,[R%d+%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM9));
	  break;
        case 197 ... 219:
	  DocPrint(doc,"R%d,[R%d+=%X]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM9));
	  break;
        case 229 ... 242:
	  DocPrint(doc,"R%d,[R%d+R%d]",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        start:
	  off=Arm64GetValue(opc,inst,ARM64_OP_IMM12);
	  if(off<0) off=(-(-off-1<<12))&(1<<12-1);
	  case 246 ... 251:
	    DocPrint(doc,"R%d,[R%d+%X]",
		  Arm64GetValue(opc,inst,ARM64_OP_REG,2),
		  Arm64GetValue(opc,inst,ARM64_OP_REG,1),
		  off);
	    break;
	  case 252 ... 253:
	    DocPrint(doc,"R%d,[R%d+%X]",
		  Arm64GetValue(opc,inst,ARM64_OP_REG,2),
		  Arm64GetValue(opc,inst,ARM64_OP_REG,1),
		  off*16);
	    break;

	  case 254 ... 259:
	    DocPrint(doc,"R%d,[R%d+%X]",
		  Arm64GetValue(opc,inst,ARM64_OP_REG,2),
		  Arm64GetValue(opc,inst,ARM64_OP_REG,1),
		  off*2);
	    break;
	  case 260 ... 264:
	    DocPrint(doc,"R%d,[R%d+%X]",
		  Arm64GetValue(opc,inst,ARM64_OP_REG,2),
		  Arm64GetValue(opc,inst,ARM64_OP_REG,1),
		  off*4);
	    break;
	  case 265 ... 267:
	    DocPrint(doc,"R%d,[R%d+%X]",
		  Arm64GetValue(opc,inst,ARM64_OP_REG,2),
		  Arm64GetValue(opc,inst,ARM64_OP_REG,1),
		  off*8);
	    break;
        end:;
	  break;
        case 272:
	  off=Arm64GetValue(opc,inst,ARM64_OP_IMMLO)|
	    (Arm64GetValue(opc,inst,ARM64_OP_IMMHI)<<2);
	  off=Sext(off,19+2);
	  DocPrint(doc,"R%d,%P",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        ptr(U8*)+off);
	  break;
        case 273:
	  off=Arm64GetValue(opc,inst,ARM64_OP_IMMLO)|
	    (Arm64GetValue(opc,inst,ARM64_OP_IMMHI)<<2);
	  off=Sext(off,19+2);
	  off<<=12;
	  DocPrint(doc,"R%d,%P",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        ptr(U8*)+off);
	  break;
        case 275 ... 282:
	  off=Arm64GetValue(opc,inst,ARM64_OP_IMM12);
	  if(off<0) off=(-(-off-1<<12))&(1<<12-1);
	  DocPrint(doc,"R%d,R%d,%X",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        off);
	  break;
        case 284 ... 287:
	  DocPrint(doc,"R%d,R%d,%X",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        ImmSImmR2Val(inst)&(1<<32-1));
	  break;
        case 288 ... 291:
	  DocPrint(doc,"R%d,R%d,%X",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        ImmSImmR2Val(inst));
	  break;

        case 293 ... 298:
	  DocPrint(doc,"R%d,%X",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        Arm64GetValue(opc,inst,ARM64_OP_IMM16));
	  break;
        case 300 ... 302:
	  DocPrint(doc,"R%d,R%d,%X",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        ImmSImmR2Val(inst)&(1<<32-1));
	  break;
        case 303 ... 305:
	  DocPrint(doc,"R%d,R%d,%X",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        ImmSImmR2Val(inst));
	  break;
        case 307 ... 308:
	  DocPrint(doc,"R%d,R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 310 ... 318:
        case 319 ... 326:
        case 328 ... 331:
        case 332 ... 335:
	  DocPrint(doc,"R%d,R%d,R%d,%s,%X",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1),
	        ArmShmod2Str(Arm64GetValue(opc,inst,ARM64_OP_SHIFT)),
	        (inst>>10)&(1<<6-1));
	  break;
        case 337 ... 344:
	  DocPrint(doc,"R%d,R%d,%s R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        ArmOption2Str(Arm64GetValue(opc,inst,ARM64_OP_OPTION)),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 346 ... 353:
	  DocPrint(doc,"R%d,R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
//TODO conditional compare
        case 364 ... 372:
	  DocPrint(doc,"(%s) R%d,R%d,R%d",
	        ArmCond2Str(Arm64GetValue(opc,inst,ARM64_OP_COND)),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 374 ... 383:
	  DocPrint(doc,"R%d,R%d,R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,4),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2));
	  break;
        case 385 ... 404:
	  DocPrint(doc,"R%d,R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 406 ... 434:
	  DocPrint(doc,"R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 441 ... 458:
	  DocPrint(doc,"R%d,R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 460 ... 461:
	  DocPrint(doc,"(%s) R%d,R%d,R%d",
	        ArmCond2Str(Arm64GetValue(opc,inst,ARM64_OP_COND)),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 466 ... 473:
	  DocPrint(doc,"R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 475 ... 557:
	  DocPrint(doc,"R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 559 ... 566:
	  DocPrint(doc,"R%d,R%d,R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,4),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2));
	  break;
        case 568 ... 603:
	  DocPrint(doc,"R%d,R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,3),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;
        case 604 ... 714:
	  DocPrint(doc,"R%d,R%d",
	        Arm64GetValue(opc,inst,ARM64_OP_REG,2),
	        Arm64GetValue(opc,inst,ARM64_OP_REG,1));
	  break;

	  //TODO conditional compare flaoting point
      }
      "\n";
      goto pass;
    }
    opcs++;
  }
  "???\n";
pass:;
  return ptr+1;
}
U32 *GenTests(I64 from,I64 to) {
  U32 *bytes=CAlloc(to*4),*ptr=bytes;
  I64 idx,idx2;
  I64 o,v;
  CArm64Opc *opc;
  for(idx=from;idx<=to;idx++) {
    for(idx2=0;opc=arm64_opcodes_flat[idx2];idx2++) {
      if(opc->num==idx) {
        *ptr=opc->value;
        for(o=0;opc->operands[o].type;o++) {
	  v=0;
	  switch(opc->operands[o].type) {
	    case ARM64_OP_IMM19:
	      v=-1&(1<<19-1);
	      break;
	    case ARM64_OP_IMM9:
	      v=-1&(1<<9-1);
	      break;
	    case ARM64_OP_IMM7:
	      v=-1&(1<<7-1);
	      break;
	    case ARM64_OP_REG:
	      v=o;
	      break;
	    case ARM64_OP_B5:
	      v=1;
	      break;
	    case ARM64_OP_B40:
	      v=0b1011;
	      break;
	    case ARM64_OP_IMM14:
	      v=-1&(1<<7-1);
	      break;
	    case ARM64_OP_COND:
	      v=1;
	      break;
	    case ARM64_OP_IMM26:
	      v=-1&(1<<26-1);
	      break;
	    case ARM64_OP_IMMLO:
	      v=0;
	      break;
	    case ARM64_OP_IMMHI:
	      v=0x123;
	      break;
	    case ARM64_OP_IMMS:
	    case ARM64_OP_IMMR:
	      v=0;
	      break;
	    case ARM64_OP_OPTION:
	      v=o;
	      break;
	    case ARM64_OP_SHIFT:
	      v=3;
	      break;
	    case ARM64_OP_IMM5:
	      v=-1&(1<<5-1);
	      break;
	    case ARM64_OP_NZCV:
	    case ARM64_OP_IMM4:
	      v=-1&(1<<4-1);
	      break;
	    case ARM64_OP_IMM16:
	      v=-1&(1<<16-1);
	      break;
	    case ARM64_OP_IMM12:
	      v=-1&(1<<12-1);
	      break;
	    case ARM64_OP_S:
	      v=1;
	      break;
	  }
	  *ptr|=v<<opc->operands[o].offset;
        }
        ptr++;
      }
    }
  }
"0x%X\n",bytes;
  to=ptr-bytes;
  for(idx=0;idx<to;idx++) {
"[%d]\t",idx+from;
   DisasOpc(DocPut,bytes+idx);
  }
  return bytes;
}

LoadOpcodes;
/*
I64 Fib(I64 f} {
  if(f<=2)
    return f;
  return Fib(f-1)+Fib(f-2);
}
Disas(&Fib,MSize(&Fib));
//Pass
GenTests(6,9);
//Pass
GenTests(11,12);
//Pass
GenTests(14,14);
//Pass
GenTests(16,23);
//Pass
GenTests(36,40);
//Pass
GenTests(41,43);
//Pass?
GenTests(46,77);
//Pass
GenTests(79,85);
//Pass
GenTests(87,90);
//Pass
GenTests(91,94);
//Pass
GenTests(95,96);
//Pass
GenTests(98,102);
//Pass
GenTests(122,126);
//Pass
GenTests(119,120);
//Pass
GenTests(134,166);
//Pass
GenTests(167,172);
//Pass
GenTests(187,196);
//Pass
GenTests(197,219);
//????
GenTests(229,242);
//Pass
GenTests(246,253);
//Pass
GenTests(254,259);
//Pass
GenTests(260,267);
//Pass
GenTests(275,282);
//Pass
GenTests(284,291);
//Pass
GenTests(293,298);
//Pass
GenTests(300,305);
//Pass
GenTests(306,308);
//Pass
GenTests(310,335);
//Pass
GenTests(337,344);
//Pass
GenTests(346,353);
//Pass
GenTests(364,372);
//Pass
GenTests(373,382);
//Pass
GenTests(385,404);
//Pass
GenTests(406,434);
//Pass
GenTests(441,458);
//Pass
GenTests(460,462);
//Pass
GenTests(466,473);
//Pass
GenTests(475,557);
//Pass
GenTests(559,705);
//Pass
GenTests(707,714);
*/
#endif
