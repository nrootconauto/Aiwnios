//Kanye West is lit

//http://ref.x86asm.net/coder64-abc.html
#define X86_OPCF_MODRM 0b01
#define X86_OPCF_ADD_REG 0b10
#define X86_OPCF_REXW 0b100
#define X86_OPCF_REX 0b1000

#define X86_OPERAND_R8 1
#define X86_OPERAND_RM8 2
#define X86_OPERAND_R16 3
#define X86_OPERAND_RM16 4
#define X86_OPERAND_R32 5
#define X86_OPERAND_RM32 6
#define X86_OPERAND_R64 7
#define X86_OPERAND_RM64 8
#define X86_OPERAND_IMM8 9
#define X86_OPERAND_IMM16 10
#define X86_OPERAND_IMM32 11
#define X86_OPERAND_IMM64 12
//Upper 8 bits in register is the register number
#define X86_OPERAND_CONST_R8 13
#define X86_OPERAND_CONST_R16 14
#define X86_OPERAND_CONST_R32 15
#define X86_OPERAND_CONST_R64 16
#define X86_OPERAND_RMx 17
#define X86_OPERAND_Rx 18
#define X86_OPERAND_EXT 19
#define X86_OPERAND_CONST_IMM 20
#define X86_OPERAND_IMM8 21
#define X86_OPERAND_IMMx 22
#define X86_OPERAND_CONST_Rx 23
#define X86_OPERAND_RXMM 24
#define X86_OPERAND_RMXMM 25
#define X86_OPERAND_RMXMM128 26
#define X86_OPERAND_RMXMM64 27
#define X86_OPERAND_RMXMM32 28
#define X86_OPERAND_RMM 29
#define X86_OPERAND_RMMM 30
#define X86_OPERAND_REL8 32
#define X86_OPERAND_REL32 33
#define X86_OPERAND_M8 34
#define X86_OPERAND_M16 35
#define X86_OPERAND_M32 36
#define X86_OPERAND_M64 37
#define X86_OPERAND_ST0 38
#define X86_OPERAND_STi 39
#define X86_OPERAND_FRM32 40
#define X86_OPERAND_FRM64 41
#define X86_OPERAND_FR 42
#define X86_OPERAND_Mx 43
#define X86_OPERAND_SREG 44
#define X86_OPERAND_MOFF8 45
#define X86_OPERAND_MOFFx 47
#define X86_OPERAND_RMMM64 48
//TODO,Nroot is not implementeing vector types now.
#define X86_OPERAND_M128 49

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

CQue asm_hash[0x10000];
CHashReg *AR(U8 *name,I64 idx,I64 rt) {
  CHashReg *r=CAlloc(sizeof CHashReg);
  r->str=StrNew(name);
  r->type=HTT_REG;
  r->reg_num=idx;
  r->reg_type=rt;
  HashAdd(r,cmp.asm_hash);
}
CX86Opcode *AO(U8 *name,I64 pf,I64 _0f,I64 po,I64 so,I64 flags,...) {
  I64 idx,op=0,at,idx2;
  CX86Opcode *opc=CAlloc(sizeof CX86Opcode),*opc2,*test;
  opc->type=HTT_OPCODE;
  if(pf!=-1)
    if(!(0<=pf<=U8_MAX))
      throw('UAsmInit');
  if(_0f!=-1)
    if(_0f!=0xf)
      throw('UAsmInit');
  if(po!=-1)
    if(!(0<=po<=U8_MAX))
      throw('UAsmInit');
  if(so!=-1)
    if(!(0<=so<=U8_MAX))
      throw('UAsmInit');
  opc->pf=pf;
  opc->_0f=_0f;
  opc->po=po;
  opc->so=so;
  opc->flags=flags;
  if(so==-1)
    at=po;
  else
    at=po|(so<<8);
  for(idx=0;idx!=argc;) {
    if(idx>argc) {
      throw('UAsmInit');
    }
    if(argv[idx]!=X86_OPERAND_EXT)
      opc->arity++;
    switch(argv[idx]) {
      start:
        opc->operands[op]=argv[idx];
        case X86_OPERAND_CONST_R8:
        case X86_OPERAND_CONST_R16:
        case X86_OPERAND_CONST_R32:
        case X86_OPERAND_CONST_R64:
        case X86_OPERAND_CONST_Rx:
        case X86_OPERAND_CONST_IMM:
        case X86_OPERAND_EXT:
	  opc->operands[op]|=argv[idx+1]<<8;
	  idx+=2;
	  break;
        default:
	  idx++;
      end:
        op++;
    }
  }
//"," seperates aliases 
  name=StrNew(name);
  idx=StrOcc(name,',')+1;
  for(op=0;idx-1!=op;op++)
    *StrLastOcc(name,",")=0;
  for(op=0;idx!=op;op++) {
    test=HashFind(LstSub(op,name),cmp.asm_hash,HTT_OPCODE);
    if(test) {
      if(test->arity!=opc->arity) {
	DbgPrint("ARITYMISSAMATCH:%s\n",LstSub(op,name));
	throw('AsmItrn');
      }
    }
    if(flags&X86_OPCF_ADD_REG) {
      idx2=8;
      while(--idx2>=0) {
        opc2=MAllocIdent(opc);
        opc2->addi_reg=idx2;
        opc2->str=StrNew(LstSub(op,name));
        HashAdd(opc2,cmp.asm_hash);
	//Move the ones without prefixes to the end as prefixes have higher precedence 
        if(pf==-1)
	  QueIns(&opc2->asm_hash_que,asm_hash[at+idx2].last);
        else
	  QueIns(&opc2->asm_hash_que,&asm_hash[at+idx2]);
      }
    } else {
      opc2=MAllocIdent(opc);
      opc2->str=StrNew(LstSub(op,name));
      HashAdd(opc2,cmp.asm_hash);
      //Move the ones without prefixes to the end as prefixes have higher precedence 
      if(pf==-1)
        QueIns(&opc2->asm_hash_que,asm_hash[at].last);
      else
        QueIns(&opc2->asm_hash_que,&asm_hash[at]);
    }
  }
  Free(opc);
  Free(name);
  return opc;
}
//Returns TRUE if just a register,or FALSE if indirect
Bool DecodeSib(U8 *data,I64 rex,I64 *r,I64 *s,I64 *i,I64 *b,I64 *o,I64 *sz=NULL) {
  I64 disp_sz=0,mod=(*data&0b11000000)>>6;
  I64 expanded;
  if(s) *s=-1;
  if(i) *i=-1;
  if(b) *b=-1;
  if(o) *o=0;
  if(r) *r=((*data&0b111000)>>3)|((rex&0b100)<<1);
  switch(mod) {
    case 0b00:
      disp_sz=0;
      if(*data&0b111==0b101) {
        disp_sz=4;
        if(b) *b=REG_RIP;
        data++;
        if(o) *o=data(I32*)[0];
        data+=4;
        if(sz) *sz=5;
        return FALSE;
      }
      if(*data&0b111==0b100) {
        data++;
        goto sib;
      }
      if(b) *b=(*data&0b111)|((rex&1)<<3);
      if(sz) *sz=1;
      return FALSE;
    case 0b01:
      disp_sz=1;
      if(*data&0b111==0b100) {
        data++;
        goto sib;
      }
      if(b) *b=(*data&0b111)|((rex&1)<<3);
      if(sz) *sz=2;
      data++;  
      if(o) *o=data(I8*)[0];
      return FALSE;
    case 0b10:
      disp_sz=4;
      if(*data&0b111==0b100) {
        data++;
        goto sib;
      }
      if(b) *b=(*data&0b111)|((rex&1)<<3);
      if(sz) *sz=5;
      data++;  
      if(o) *o=data(I32*)[0];
      return FALSE;
    case 0b11:
      if(b) *b=(*data&0b111)|((rex&1)<<3);
      if(sz) *sz=1;
      return TRUE;
  }
sib:
  if(s)
    switch((*data>>6)&0b11) {
      case 0: *s=1; break;
      case 1: *s=2; break;
      case 2: *s=4; break;
      case 3: *s=8; break;
    }
  switch(mod) {
    case 0:
      if(*data&0b111000==0b100000) {
        if(*data&0b111==0b101) {
	  if(sz) *sz=6;
	  if(o) *o=(data+1)(I32*)[0];
	  if(s) *s=-1;
	  //if(b) *b=REG_RIP;
	  return FALSE;
	}
        if(sz) *sz=2;
        if(b) *b=(*data&0b111)|((rex&0b1)<<3);      
        return FALSE;
      } else {
        if(*data&0b111==0b101) {
	  if(i) *i=((*data>>3)&0b111)|((rex&0b10)<<2);
	  if(o) *o=(data+1)(I32*)[0];
	  if(sz) *sz=6;
	  return FALSE;
        }
	if(sz) *sz=2;
        if(i) *i=((*data>>3)&0b111)|((rex&0b10)<<2);
        if(b) *b=(*data&0b111)|((rex&0b1)<<3);      
        return FALSE;
      }
    case 1:
	if(*data&0b111000==0b100000) {
	  if(s) *s=-1;
          if(b) *b=(*data&0b111)|((rex&0b1)<<3);
          if(o) *o=data(I8*)[1];
	  if(sz) *sz=3;
  	  return FALSE;  
        }
	if(b) *b=(*data&0b111)|((rex&0b1)<<3);      
        if(i) *i=((*data>>3)&0b111)|((rex&0b10)<<2);
        if(o) *o=data(I8*)[1];
        if(sz) *sz=3;
        return FALSE;
    case 2:
	if(*data&0b111000==0b100000) {
	  if(s) *s=-1;
          if(b) *b=(*data&0b111)|((rex&0b1)<<3);
          if(o) *o=(1+data)(I32*)[0];
          if(sz) *sz=6;
  	  return FALSE;  
        }
        if(b) *b=(*data&0b111)|((rex&0b1)<<3);
        if(i) *i=((*data>>3)&0b111)|((rex&0b10)<<2);
        if(o) *o=(1+data)(I32*)[0];
        if(sz) *sz=6;
        return FALSE;
  }
}
U8 *RegIdx2StrXmm(I64 r) {
  switch(r) {
    case 0: return "XMM0";
    case 1: return "XMM1";
    case 2: return "XMM2";
    case 3: return "XMM3";
    case 4: return "XMM4";
    case 5: return "XMM5";
    case 6: return "XMM6";
    case 7: return "XMM7";
    case 8: return "XMM8";
    case 9: return "XMM9";
    case 10: return "XMM10";
    case 11: return "XMM11";
    case 12: return "XMM12";
    case 13: return "XMM13";
    case 14: return "XMM14";
    case 15: return "XMM15";
    default: throw('Register');
  }
}
U8 *RegIdx2StrMm(I64 r) {
  switch(r) {
    case 0: return "MM0";
    case 1: return "MM1";
    case 2: return "MM2";
    case 3: return "MM3";
    case 4: return "MM4";
    case 5: return "MM5";
    case 6: return "MM6";
    case 7: return "MM7";
    case 8: return "MM8";
    case 9: return "MM9";
    case 10: return "MM10";
    case 11: return "MM11";
    case 12: return "MM12";
    case 13: return "MM13";
    case 14: return "MM14";
    case 15: return "MM15";
    default: throw('Register');
  }
}

U8 *RegIdx2StrX87(I64 r) {
  switch(r) {
    case 0: return "ST0";
    case 1: return "ST1";
    case 2: return "ST2";
    case 3: return "ST3";
    case 4: return "ST4";
    case 5: return "ST5";
    case 6: return "ST6";
    case 7: return "ST7";
    default: throw('Register');
  }
}

U8 *RegIdx2StrSReg(I64 r) {
  switch(r) {
    case 0: return "ES";
    case 1: return "CS";
    case 2: return "SS";
    case 3: return "DS";
    case 4: return "FS";
    case 5: return "GS";
    default: throw('Register');
  }
}


U8 *RegIdx2Str(I64 r,I64 sz=8,Bool rex=FALSE) {
  if(r==REG_RIP)
    return "rip";
  switch((sz<<4)|r) {
    case 0x10: return "al";
    case 0x20: return "ax";
    case 0x40: return "eax";
    case 0x80: return "rax";
    case 0x11: return "cl";
    case 0x21: return "cx";
    case 0x41: return "ecx";
    case 0x81: return "rcx";
    case 0x12: return "dl";
    case 0x22: return "dx";
    case 0x42: return "edx";
    case 0x82: return "rdx";
    case 0x13: return "bl";
    case 0x23: return "bx";
    case 0x43: return "ebx";
    case 0x83: return "rbx";
    case 0x14:
      if(rex)  return "spl";
      return "ah";
    case 0x24: return "sp";
    case 0x44: return "esp";
    case 0x84: return "rsp";
    case 0x15:
      if(rex)  return "bpl";
      return "ch";
    case 0x25: return "bp";
    case 0x45: return "ebp";
    case 0x85: return "rbp";
    case 0x16:
      if(rex)  return "sil";
      return "dh";
    case 0x26: return "si";
    case 0x46: return "esi";
    case 0x86: return "rsi";
    case 0x17:
      if(rex)  return "dil";
      return "bh";
    case 0x27: return "di";
    case 0x47: return "edi";
    case 0x87: return "rdi";
    case 0x18: return "r8l";
    case 0x28: return "r8w";
    case 0x48: return "r8d";
    case 0x88: return "r8";
    case 0x19: return "r9l";
    case 0x29: return "r9w";
    case 0x49: return "r9d";
    case 0x89: return "r9";
    case 0x1a: return "r10l";
    case 0x2a: return "r10w";
    case 0x4a: return "r10d";
    case 0x8a: return "r10";
    case 0x1b: return "r11l";
    case 0x2b: return "r11w";
    case 0x4b: return "r11d";
    case 0x8b: return "r11";
    case 0x1c: return "r12l";
    case 0x2c: return "r12w";
    case 0x4c: return "r12d";
    case 0x8c: return "r12";
    case 0x1d: return "r13l";
    case 0x2d: return "r13w";
    case 0x4d: return "r13d";
    case 0x8d: return "r13";
    case 0x1e: return "r14l";
    case 0x2e: return "r14w";
    case 0x4e: return "r14d";
    case 0x8e: return "r14";
    case 0x1f: return "r15l";
    case 0x2f: return "r15w";
    case 0x4f: return "r15d";
    case 0x8f: return "r15";
    default: throw('Register');
  }
}
#define RS_GP 1
#define RS_XMM 2
#define RS_X87 3
#define RS_MM 4
U0 PrintRMMode(CDoc *doc,U8 segment,I64 rex=0,U8 *data,I64 sz,Bool small=FALSE,I64 reg_set=RS_GP) {
  switch(sz) {
    case 1: DocPrint(doc,"U8 "); break;
    case 2: DocPrint(doc,"U16 "); break;
    case 4: DocPrint(doc,"U32 "); break;
    case 8: DocPrint(doc,"U64 "); break;
    case 16: DocPrint(doc,"U128 "); break;
  }
  I64 s,i,b,o;
  if(DecodeSib(data,rex,NULL,&s,&i,&b,&o)) {
    switch(reg_set) {
      case RS_GP:
        DocPrint(doc,"%s ",RegIdx2Str(b,sz,rex));
	return;
      case RS_XMM:
        DocPrint(doc,"%s ",RegIdx2StrXmm(b));
	return;
      case RS_X87:
        DocPrint(doc,"%s ",RegIdx2StrX87(b));
	return;
      case RS_MM:
        DocPrint(doc,"%s ",RegIdx2StrMm(b));
	return;
    }
  }
  switch(segment) {
    case 0x2E: DocPrint(doc,"CS ");break;
    case 0x36: DocPrint(doc,"SS ");break;
    case 0x3E: DocPrint(doc,"DS ");break;
    case 0x26: DocPrint(doc,"ES ");break;
    case 0x64: DocPrint(doc,"FS ");break;
    case 0x65: DocPrint(doc,"GS ");break;
  }

  if(o!=INVALID_PTR&&i==-1&&b==-1) {
    DocPrint(doc,"[");
    DocPrint(doc,"%X",o);
    DocPrint(doc,"] ");
    return;
  }
  if(o!=INVALID_PTR)
    DocPrint(doc,"%X",o);
  DocPrint(doc,"[");
  if(b!=-1) {
    DocPrint(doc,"%s",RegIdx2Str(b,8-4*small,rex));
    if(i!=-1) DocPrint(doc,"+");
  }
  if(i!=-1)
    DocPrint(doc,"%s",RegIdx2Str(i,8-4*small,rex));
  if(s!=-1)
    DocPrint(doc,"*%d",s);
  DocPrint(doc,"] ");
}
I64 GetOpcodeReg(CX86Opcode *opc,I64 rex,U8 *data) {
  I64 idx=0,m_reg,b_reg;
  Bool has_use_m_reg=FALSE;
  if(opc->flags&X86_OPCF_ADD_REG) {
    if(rex&0b001)
      return opc->addi_reg+8;
    return opc->addi_reg;
  }
  if(opc->flags&X86_OPCF_MODRM) {
    DecodeSib(data,rex,&m_reg,NULL,NULL,&b_reg,NULL);
    for(idx=0;idx!=4;idx++) {
      //If register is reserved for a fuckin EXTension,use the base register motherfucker
      if(opc->operands[idx].u8[0]==X86_OPERAND_EXT) {
        return b_reg;
      }
    }
    return ((*data&0b111000)>>3)|((rex&0b100)<<1);
  }
  throw('UAsmItrn');
}
U8 *DisasOpc(CDoc *doc_out,U8 *data) {
  CDoc *doc=DocNew;
  U8 *odata=data;
  I64 prefix_groups=0,opc,ext,i;
  Bool found_opc=FALSE;
  I64 pf1=-1,cnt,cnt2;
  Bool _66=FALSE,_67=FALSE,_0f=FALSE;
  I64 rex=0,len;
  U8 seg_prefix=0;
  Bool has_modrm=FALSE,wants_rexw=FALSE;
  CX86Opcode *cur,*head,*valid_opc=NULL;
  I64 imm_off=0;
  U8 *od;
prefix:
  while(TRUE) {
    if(!Bt(&prefix_groups,1)) {
      switch(*data) {
        case 0xf0:
        case 0xf1:
        case 0xf2:
        case 0xf3:
        case 0x9b:
	  Bts(&prefix_groups,1);
	  pf1=*data++;
	  goto prefix;
      }
    } 
    if(!Bt(&prefix_groups,2)) {
      switch(*data) {
        start:
	  Bts(&prefix_groups,2);
	  case 0x2e: break;
	  case 0x36: break;
	  case 0x3e: break;
	  case 0x26: break;
	  case 0x64: break;
	  case 0x65: break;
//AMD does have these,I am not checking for Branch instructions either
	    //          case 0x2e: DocPrint(doc,"BranchNotTaken "); break;
	    //          case 0x3e: DocPrint(doc,"BranchTaken "); break;
        end:
	  seg_prefix=*data++;
	  goto prefix;
      }
    }
    if(!Bt(&prefix_groups,3))
      if(*data==0x66){
        Bts(&prefix_groups,3);
        _66=TRUE;
        data++;
        goto prefix;
      }
    if(!Bt(&prefix_groups,4))
      if(*data==0x67){
        Bts(&prefix_groups,4);
        _67=TRUE;
        data++;
        goto prefix;
      }
    if(!Bt(&prefix_groups,5))
      if(*data&0xf0==0x40){
        Bts(&prefix_groups,5);
        rex=*data++;
        goto prefix;
      }
    if(!Bt(&prefix_groups,6))
      if(*data==0xf){
        Bts(&prefix_groups,6);
        _0f=TRUE;
        data++;
        break;
      }
    break;
  }
//First try 16bit opcode
  opc=data(U16*)[0],imm_off=2;
//Account for op,00
  if(!opc.u8[1])
    imm_off=1;
  if(!QueCnt(&asm_hash[opc]))
    opc=*data,imm_off=1;
  if(!QueCnt(&asm_hash[opc])) {
bad:
    DocPrint(doc,"(Bad,%X)",opc);
    return data+1;
  }
  data+=imm_off;
  od=data;
  imm_off=0;
  head=&asm_hash[opc];
//If one of our opcodes is promoted by X86_OPCF_REXW,require it if we have rex.W
  if(rex&0b1000)
    for(cur=head->next;cur!=head;cur=cur->next) {
      cur=cur(U8*)-offset(CX86Opcode.asm_hash_que);
      if(cur->flags&X86_OPCF_REXW) {
        wants_rexw=TRUE;
        break;
      }
      cur=cur(U8*)+offset(CX86Opcode.asm_hash_que);
    }
  for(cur=head->next;cur!=head;cur=cur->next) {
    cur=cur(U8*)-offset(CX86Opcode.asm_hash_que);
    data=od;
//If our old valid opcode wants(and has) a rex.w,ignore "newer" canidates that lack the need for a rex.
    if(valid_opc&&!(cur->flags&X86_OPCF_REXW))
      if(wants_rexw&&valid_opc->flags&X86_OPCF_REXW)
        goto next;
//If our valid opcode uses _66,ensure _66 is used by our "newer" one 
    if(valid_opc)
      switch(valid_opc->pf) {
        case -1:
	  break;
        case 0x66:
	  if(cur->pf!=0x66)
	    goto next;
	  break;
        default:
	  if(cur->pf==-1)
	    goto next;
      }
    switch(cur->pf) {
      case 0x66:
        if(!_66)
	  goto next;
        break;
      case 0xf0:
      case 0xf1:
      case 0xf2:
      case 0xf3:
      case 0x9b:
        if(pf1!=cur->pf)
	  goto next;
        break;
    }
    if((cur->_0f==0xf)!=_0f)
      goto next;
    if(cur->flags&X86_OPCF_REXW)
      if(!(rex&0b1000))
        goto next;
    if(cur->flags&X86_OPCF_MODRM) {
      DecodeSib(data,rex,&ext,NULL,NULL,NULL,NULL,&len);
      for(cnt=0;cnt!=4;cnt++)
        if(cur->operands[cnt].u8[0]==X86_OPERAND_EXT)
	  if(cur->operands[cnt].u8[1]!=ext&0x7)
	    goto next;
      imm_off=len;
//If Opcode wants a MEMORY only ass motherfucker,make sure ModRM/SIB
      //points to a memory address,AND NOT A FUCKIN register
      for(cnt=0;cnt!=4;cnt++) {
        switch(cur->operands[cnt].u8[0]) {
	  case X86_OPERAND_Mx:
	  case X86_OPERAND_M8:
	  case X86_OPERAND_M16:
	  case X86_OPERAND_M32:
	  case X86_OPERAND_M64:
//Returns TRUE if a fuckin register motherfucker
	    if(DecodeSib(data,rex,&ext,NULL,NULL,NULL,NULL))
	      goto next;
	    break;
	  case X86_OPERAND_Rx:
	  case X86_OPERAND_R8:
	  case X86_OPERAND_R16:
	  case X86_OPERAND_R32:
	  case X86_OPERAND_R64:
	  case X86_OPERAND_STi:
	  case X86_OPERAND_FR:
	  case X86_OPERAND_SREG:
//Check the converse Motherfucker ONLY if ext is used as the register
	  //This will place our fucker at ModRM base field
	    for(cnt2=0;cnt2!=4;cnt2++) {
	      if(cur->operands[cnt2].u8[0]==X86_OPERAND_EXT) {
	        if(!DecodeSib(data,rex,&ext,NULL,NULL,NULL,NULL))
		  goto next;
	      }
	    }
        }
      }
    } else
      imm_off=0;
    valid_opc=cur;
next:
    cur=cur(U8*)+offset(CX86Opcode.asm_hash_que);
  }
  if(valid_opc) {
    cur=valid_opc;
    switch(cur->pf) {
      case 0x66: //If 0x66 is part of the opcode,it may be an SSE prefix,so ignore it?
        _66=FALSE;
        break;
      case 0xf0:
      case 0xf1:
      case 0xf2:
      case 0xf3:
      case 0x9b:
        Btr(&prefix_groups,1);
    }
    if(Bt(&prefix_groups,1))
      switch(pf1) {
        case 0xf0:
	  DocPrint(doc,"LOCK ");
	  break; 
        case 0xf2:
	  DocPrint(doc,"REPNZ ");
	  break;
        case 0xf3:
	  DocPrint(doc,"REPZ ");
	  break;

      }
    DocPrint(doc,"%s ",cur->str);
    imm_off=0;
    if(cur->flags&X86_OPCF_MODRM)
      DecodeSib(data,rex,&ext,NULL,NULL,NULL,NULL,&imm_off);
    for(cnt=0;cnt!=4;cnt++) {
      switch(cur->operands[cnt]&0xff) {
        start:
	  if(cnt) DocPrint(doc,", ");
	  case X86_OPERAND_R8:
	    DocPrint(doc,"U8 %s ",RegIdx2Str(GetOpcodeReg(cur,rex,data),1));
	    break;
	  case X86_OPERAND_RM8:
	    PrintRMMode(doc,seg_prefix,rex,data,1,_67);
	    break;
	  case X86_OPERAND_R16:
	    DocPrint(doc,"U16 %s ",RegIdx2Str(GetOpcodeReg(cur,rex,data),2));
	    break;
	  case X86_OPERAND_RM16:
	    PrintRMMode(doc,seg_prefix,rex,data,2,_67);
	    break;
	  case X86_OPERAND_R32:
	    DocPrint(doc,"U32 %s ",RegIdx2Str(GetOpcodeReg(cur,rex,data),4));
	    break;
	  case X86_OPERAND_RM32:
	    PrintRMMode(doc,seg_prefix,rex,data,4,_67);
	    break;
	  case X86_OPERAND_R64:
	    DocPrint(doc,"U64 %s ",RegIdx2Str(GetOpcodeReg(cur,rex,data),8));
	    break;
	  case X86_OPERAND_RM64:
	    PrintRMMode(doc,seg_prefix,rex,data,8,_67);
	    break;
	  case X86_OPERAND_IMM8:
	    DocPrint(doc,"I8 %d ",data(U8*)[imm_off]);
	    imm_off++;
	    break;
	  case X86_OPERAND_IMM16:
	    DocPrint(doc,"I16 %d ",(data+imm_off)(I16*)[0]);
	    imm_off+=2;
	    break;
	  case X86_OPERAND_IMM32:
	    DocPrint(doc,"I32 %d ",(data+imm_off)(I32*)[0]);
	    imm_off+=4;
	    break;
	  case X86_OPERAND_IMM64:
	    DocPrint(doc,"I64 %d ",(data+imm_off)(U64*)[0]);
	    imm_off+=8;
	    break;
	  case X86_OPERAND_CONST_R8:
	    DocPrint(doc,"U8 %s ",RegIdx2Str(cur->operands[cnt].u8[1],1));
	    break;
	  case X86_OPERAND_CONST_R16:
	    DocPrint(doc,"U16 %s ",RegIdx2Str(cur->operands[cnt].u8[1],2));
	    break;
	  case X86_OPERAND_CONST_R32:
	    DocPrint(doc,"U32 %s ",RegIdx2Str(cur->operands[cnt].u8[1],4));
	    break;
	  case X86_OPERAND_CONST_R64:
	    DocPrint(doc,"U64 %s ",RegIdx2Str(cur->operands[cnt].u8[1],8));
	    break;
	  case X86_OPERAND_RMx:
	    if(_66) 
	      PrintRMMode(doc,seg_prefix,rex,data,2,_67);
	    else if(!(rex&0b1000))
	      PrintRMMode(doc,seg_prefix,rex,data,4,_67);
	    else
	      PrintRMMode(doc,seg_prefix,rex,data,8,_67);
	    break;
	  case X86_OPERAND_Rx:
	    if(_66) 
	      DocPrint(doc,"U16 %s ",RegIdx2Str(GetOpcodeReg(cur,rex,data),2));
	    else if(!(rex&0b1000))
	      DocPrint(doc,"U32 %s ",RegIdx2Str(GetOpcodeReg(cur,rex,data),4));
	    else
	      DocPrint(doc,"U64 %s ",RegIdx2Str(GetOpcodeReg(cur,rex,data),8));
	    break;
	  case X86_OPERAND_CONST_IMM:
	    DocPrint(doc,"I64 %d ",cur->operands[cnt].u8[1]);
	    break;
	  case X86_OPERAND_IMMx:
	    if(_66) {
	      DocPrint(doc,"I16 %d ",(data+imm_off)(I16*)[0]);
	      imm_off+=2;
	    } else {
	      DocPrint(doc,"I32 %d ",(data+imm_off)(I32*)[0]);
	      imm_off+=4;
	    }
	    break;
	  case X86_OPERAND_CONST_Rx:
	    if(_66) 
	      DocPrint(doc,"U16 %s ",RegIdx2Str(cur->operands[cnt].u8[1],2));
	    else if(!(rex&0b1000))
	      DocPrint(doc,"U32 %s ",RegIdx2Str(cur->operands[cnt].u8[1],4));
	    else
	      DocPrint(doc,"U64 %s ",RegIdx2Str(cur->operands[cnt].u8[1],8));
	    break;
	    break;
	  case X86_OPERAND_RXMM:
	    DocPrint(doc,"U128 %s ",RegIdx2StrXmm(GetOpcodeReg(cur,rex,data)));
	    break;
	  case X86_OPERAND_RMXMM:
	  case X86_OPERAND_RMXMM128:
	    PrintRMMode(doc,seg_prefix,rex,data,16,_67,RS_XMM);
	    break;
	  case X86_OPERAND_RMXMM64:
	    PrintRMMode(doc,seg_prefix,rex,data,8,_67,RS_XMM);
	    break;
	  case X86_OPERAND_RMXMM32:
	    PrintRMMode(doc,seg_prefix,rex,data,4,_67,RS_XMM);
	    break;
	  case X86_OPERAND_RMM:
	    DocPrint(doc,"U64 %s ",RegIdx2StrMm(GetOpcodeReg(cur,rex,data)));
	    break;
	  case X86_OPERAND_RMMM:
	    PrintRMMode(doc,seg_prefix,rex,data,8,_67,RS_MM);
	    break;
	  case X86_OPERAND_REL8:
	    DocPrint(doc,"I8 %P ",(data+1)+(data+imm_off)(I8*)[0]);
	    imm_off+=1;
	    break;
	  case X86_OPERAND_REL32:
	    DocPrint(doc,"I32 %P ",(data+4)+(data+imm_off)(I32*)[0]);
	    imm_off+=4;
	    break;
	  case X86_OPERAND_M8:
	    PrintRMMode(doc,seg_prefix,rex,data,1,_67);
	    break;
	  case X86_OPERAND_M16:
	    PrintRMMode(doc,seg_prefix,rex,data,2,_67);
	    break;
	  case X86_OPERAND_M32:
	    PrintRMMode(doc,seg_prefix,rex,data,4,_67);
	    break;
	  case X86_OPERAND_M64:
	    PrintRMMode(doc,seg_prefix,rex,data,8,_67);
	    break;
	  case X86_OPERAND_ST0:
	    DocPrint(doc,"F64 ST0");
	    break;
	  case X86_OPERAND_STi:
	    DocPrint(doc,"F64 ST%d",GetOpcodeReg(cur,rex,data));
	    break;
	  case X86_OPERAND_FRM32:
	    PrintRMMode(doc,seg_prefix,rex,data,4,_67,RS_X87);
	    break;
	  case X86_OPERAND_FRM64:
	    PrintRMMode(doc,seg_prefix,rex,data,8,_67,RS_X87);
	    break;
          case X86_OPERAND_M128:
	    PrintRMMode(doc,seg_prefix,rex,data,16,_67,RS_X87);
	    break;
	  case X86_OPERAND_Mx:
	    if(_66) 
	      PrintRMMode(doc,seg_prefix,rex,data,2,_67);
	    else if(!(rex&0b1000))
	      PrintRMMode(doc,seg_prefix,rex,data,4,_67);
	    else
	      PrintRMMode(doc,seg_prefix,rex,data,8,_67);
	    break;
	  case X86_OPERAND_SREG:
	    DocPrint(doc,"%s ",RegIdx2StrSReg(GetOpcodeReg(cur,rex,data)));
	    break;
	  start:
	    case X86_OPERAND_MOFF8:
	      DocPrint(doc,"I8  ");
	      imm_off+=8;
	      break;
	    case X86_OPERAND_MOFFx:
	      if(_66) 
	        DocPrint(doc,"I16 ");
	      else if(!(rex&0b1000))
	        DocPrint(doc,"I32 ");
	      else
	        DocPrint(doc,"I64 ");
	      imm_off+=8;
	      break;
	  end:
	    switch(seg_prefix) {
	      case 0x2E: DocPrint(doc,"CS:");break;
	      case 0x36: DocPrint(doc,"SS:");break;
	      case 0x3E: DocPrint(doc,"DS:");break;
	      case 0x26: DocPrint(doc,"ES:");break;
	      case 0x64: DocPrint(doc,"FS:");break;
	      case 0x65: DocPrint(doc,"GS:");break;
	    }
	    DocPrint(doc,"[%P] ",(data+imm_off)(I64*)[-1]);
	    break;
	  case X86_OPERAND_RMMM64:
	    PrintRMMode(doc,seg_prefix,rex,data,8,_67,RS_MM);
        end:
	  break;
        case X86_OPERAND_EXT:
	  break;
        case 0:
	  break;
        default:
	  throw("UAsmItrn");
      }
    }
    DocPrint(doc_out,"%P\t\t",odata);
    DocInsDoc(doc_out,doc);
    DocDel(doc);
    DocPrint(doc_out,"\n");
    return data+imm_off;
  }
//wut
  goto bad;
}
U0 UAsmInit() {
  I64 cnt=0;
  for(cnt=0;cnt!=0x10000;cnt++) 
    QueInit(&asm_hash[cnt]);
  AR("AL",0,REGT_R8);
  AR("CL",1,REGT_R8);
  AR("DL",2,REGT_R8);
  AR("BL",3,REGT_R8);
//With rex
  AR("SPL",4,REGT_R8);
  AR("BPL",5,REGT_R8);
  AR("SIL",6,REGT_R8);
  AR("DIL",7,REGT_R8);
//Without rex
  AR("AH",4,REGT_R8);
  AR("CH",5,REGT_R8);
  AR("DH",6,REGT_R8);
  AR("BH",7,REGT_R8);

  AR("R8L",8,REGT_R8);
  AR("R9L",9,REGT_R8);
  AR("R10L",10,REGT_R8);
  AR("R11L",11,REGT_R8);
  AR("R12L",12,REGT_R8);
  AR("R13L",13,REGT_R8);
  AR("R14L",14,REGT_R8);
  AR("R15L",15,REGT_R8);

  AR("AX",0,REGT_R16);
  AR("CX",1,REGT_R16);
  AR("DX",2,REGT_R16);
  AR("BX",3,REGT_R16);
  AR("SP",4,REGT_R16);
  AR("BP",5,REGT_R16);
  AR("SI",6,REGT_R16);
  AR("DI",7,REGT_R16);
  AR("R8",8,REGT_R16);
  AR("R9",9,REGT_R16);
  AR("R10W",10,REGT_R16);
  AR("R11W",11,REGT_R16);
  AR("R12W",12,REGT_R16);
  AR("R13W",13,REGT_R16);
  AR("R14W",14,REGT_R16);
  AR("R15W",15,REGT_R16);

  AR("EAX",0,REGT_R32);
  AR("ECX",1,REGT_R32);
  AR("EDX",2,REGT_R32);
  AR("EBX",3,REGT_R32);
  AR("ESP",4,REGT_R32);
  AR("EBP",5,REGT_R32);
  AR("ESI",6,REGT_R32);
  AR("EDI",7,REGT_R32);
  AR("R8D",8,REGT_R32);
  AR("R9D",9,REGT_R32);
  AR("R10D",10,REGT_R32);
  AR("R11D",11,REGT_R32);
  AR("R12D",12,REGT_R32);
  AR("R13D",13,REGT_R32);
  AR("R14D",14,REGT_R32);
  AR("R15D",15,REGT_R32);

  AR("RAX",0,REGT_R64);
  AR("RCX",1,REGT_R64);
  AR("RDX",2,REGT_R64);
  AR("RBX",3,REGT_R64);
  AR("RSP",4,REGT_R64);
  AR("RBP",5,REGT_R64);
  AR("RSI",6,REGT_R64);
  AR("RDI",7,REGT_R64);
  AR("R8",8,REGT_R64);
  AR("R9",9,REGT_R64);
  AR("R10",10,REGT_R64);
  AR("R11",11,REGT_R64);
  AR("R12",12,REGT_R64);
  AR("R13",13,REGT_R64);
  AR("R14",14,REGT_R64);
  AR("R15",15,REGT_R64);

  AR("MM0",0,REGT_MM);
  AR("MM1",1,REGT_MM);
  AR("MM2",2,REGT_MM);
  AR("MM3",3,REGT_MM);
  AR("MM4",4,REGT_MM);
  AR("MM5",5,REGT_MM);
  AR("MM6",6,REGT_MM);
  AR("MM7",7,REGT_MM);

  AR("XMM0",0,REGT_XMM);
  AR("XMM1",1,REGT_XMM);
  AR("XMM2",2,REGT_XMM);
  AR("XMM3",3,REGT_XMM);
  AR("XMM4",4,REGT_XMM);
  AR("XMM5",5,REGT_XMM);
  AR("XMM6",6,REGT_XMM);
  AR("XMM7",7,REGT_XMM);
  AR("XMM8",8,REGT_XMM);
  AR("XMM9",9,REGT_XMM);
  AR("XMM10",10,REGT_XMM);
  AR("XMM11",11,REGT_XMM);
  AR("XMM12",12,REGT_XMM);
  AR("XMM13",13,REGT_XMM);
  AR("XMM14",14,REGT_XMM);
  AR("XMM15",15,REGT_XMM);

  AR("ES",0,REGT_SEG);
  AR("CS",1,REGT_SEG);
  AR("SS",2,REGT_SEG);
  AR("DS",3,REGT_SEG);
  AR("FS",4,REGT_SEG);
  AR("GS",5,REGT_SEG);

  AR("ST0",0,REGT_FSTK);
  AR("ST1",1,REGT_FSTK);
  AR("ST2",2,REGT_FSTK);
  AR("ST3",3,REGT_FSTK);
  AR("ST4",4,REGT_FSTK);
  AR("ST5",5,REGT_FSTK);
  AR("ST6",6,REGT_FSTK);
  AR("ST7",7,REGT_FSTK);


  AO("ADC",-1,-1,0x10,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("ADC",-1,-1,0x11,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("ADC",-1,-1,0x12,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("ADC",-1,-1,0x13,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("ADC",-1,-1,0x14,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("ADC",-1,-1,0x15,-1,0,X86_OPERAND_CONST_Rx,REG_RAX,X86_OPERAND_IMMx);
  AO("ADC",-1,-1,0x80,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("ADC",-1,-1,0x81,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,2);
  AO("ADC",-1,-1,0x83,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);

  AO("ADD",-1,-1,0x00,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("ADD",-1,-1,0x01,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("ADD",-1,-1,0x02,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("ADD",-1,-1,0x03,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("ADD",-1,-1,0x04,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("ADD",-1,-1,0x05,-1,0,X86_OPERAND_CONST_Rx,REG_RAX,X86_OPERAND_IMMx);
  AO("ADD",-1,-1,0x80,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,0);
  AO("ADD",-1,-1,0x81,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,0);
  AO("ADD",-1,-1,0x83,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,0);

  AO("ADDPD",0x66,0xf,0x58,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("ADDPS",-1,0xf,0x58,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("ADDSD",0xf2,0xf,0x58,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("ADDSS",0xf3,0xf,0x58,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
  AO("ADDSUBPD",0x66,0xf,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("ADDSUBPS",0xf2,0xf,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);

  AO("AND",-1,-1,0x20,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("AND",-1,-1,0x21,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("AND",-1,-1,0x22,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("AND",-1,-1,0x23,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("AND",-1,-1,0x24,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("AND",-1,-1,0x25,-1,0,X86_OPERAND_CONST_Rx,REG_RAX,X86_OPERAND_IMMx);
  AO("AND",-1,-1,0x80,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,4);
  AO("AND",-1,-1,0x81,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,4);
  AO("AND",-1,-1,0x83,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,4);

  AO("ANDNPD",0x66,0xf,0x55,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("ANDNPS",-1,0xf,0x55,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("ANDPD",0x66,0xf,0x54,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("ANDPS",-1,0xf,0x54,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);

  AO("BLENDPD",-1,0xf,0x3a,0xd,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("BLENDPS",-1,0xf,0x3a,0xc,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);

  AO("BSF",-1,0xf,0xbc,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("BSR",-1,0xf,0xbd,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("BSWAP",-1,0xf,0xc8,-1,X86_OPCF_ADD_REG,X86_OPERAND_Rx);
  AO("BT",-1,0xf,0xa3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("BT",-1,0xf,0xba,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,4);
  AO("BTC",-1,0xf,0xba,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,7);
  AO("BTC",-1,0xf,0xbb,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("BTR",-1,0xf,0xb3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("BTR",-1,0xf,0xba,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("BTS",-1,0xf,0xab,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8);
  AO("BTS",-1,0xf,0xba,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,5);
  
  AO("CALL",-1,-1,0xe8,-1,0,X86_OPERAND_REL32);
  AO("CALL",-1,-1,0xff,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,2);
  AO("CALLF",-1,-1,0xff,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,3);

  AO("CBW",0x66,-1,0x98,-1,0);
  AO("CWDE",-1,-1,0x98,-1,0);
  AO("CDQE",-1,-1,0x98,-1,X86_OPCF_REXW);

  AO("CLC",-1,-1,0xf8,-1,0);
  AO("CLD",-1,-1,0xfc,-1,0);
  AO("CLFLUSH",-1,0xf,0xae,-1,X86_OPCF_MODRM,X86_OPERAND_M8,X86_OPERAND_EXT,7);
  AO("CLI",-1,-1,0xfa,-1,0);
  AO("CMC",-1,-1,0xf5,-1,0);

  AO("CMOVB,CMOVNAE,CMOVC",-1,0xf,0x42,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVBE,CMOVNA",-1,0xf,0x46,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVL,CMOVNGE",-1,0xf,0x4c,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVLE,CMOVNG",-1,0xf,0x4e,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVNB,CMOVAE,CMOVNC",-1,0xf,0x43,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVNBE,CMOVA",-1,0xf,0x47,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVNL,CMOVGE",-1,0xf,0x4d,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVNLE,CMOVG",-1,0xf,0x4f,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVNO",-1,0xf,0x41,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVNP,CMOVPO",-1,0xf,0x4b,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVNS",-1,0xf,0x49,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVNZ,CMOVNE",-1,0xf,0x45,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVO",-1,0xf,0x40,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVP,CMOVPE",-1,0xf,0x4a,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVS",-1,0xf,0x48,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMOVZ,CMOVE",-1,0xf,0x44,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);

  AO("CMP",-1,-1,0x38,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("CMP",-1,-1,0x39,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("CMP",-1,-1,0x3a,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("CMP",-1,-1,0x3b,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CMP",-1,-1,0x3c,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("CMP",-1,-1,0x3d,-1,0,X86_OPERAND_CONST_Rx,REG_RAX,X86_OPERAND_IMMx);
  AO("CMP",-1,-1,0x80,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,7,X86_OPERAND_IMM8);
  AO("CMP",-1,-1,0x81,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,7,X86_OPERAND_IMMx);
  AO("CMP",-1,-1,0x83,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,7,X86_OPERAND_IMM8);

  AO("CMPPD",0x66,0xf,0xc2,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("CMPPS",-1,0xf,0xc2,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);

  AO("CMPSB",-1,-1,0xa6,-1,0);
  AO("CMPSW",0x66,-1,0xa7,-1,0);
  AO("CMPSD",-1,-1,0xa7,-1,0);
  AO("CMPSQ",-1,-1,0xa7,-1,X86_OPCF_REXW);

  AO("CMPSD2",0xf2,0xf,0xc2,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64,X86_OPERAND_IMM8);
  AO("CMPSS2",0xf3,0xf,0xc2,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32,X86_OPERAND_IMM8);

  AO("CMPXCHG",-1,0xf,0xb0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_R8);
  AO("CMPXCHG",-1,0xf,0xb1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_Rx,REG_RAX,X86_OPERAND_Rx);
  AO("CMPXCHG8B",-1,0xf,0xc7,-1,X86_OPCF_MODRM,X86_OPERAND_RM64,X86_OPERAND_CONST_R32,REG_RAX,X86_OPERAND_CONST_R32,REG_RDX,X86_OPERAND_EXT,1);
  AO("CMPXCHG16B",-1,0xf,0xc7,-1,X86_OPCF_MODRM|X86_OPCF_REXW,X86_OPERAND_RM64,X86_OPERAND_CONST_R32,REG_RAX,X86_OPERAND_CONST_R32,REG_RDX,X86_OPERAND_EXT,1);

  AO("COMISD",0x66,0xf,0x2f,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("COMISS",-1,0xf,0x2f,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);

  AO("CPUID",-1,0xf,0xa2,-1,0);
  AO("CRC32",0xf2,0xf,0x38,0xf0,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RM8);
  AO("CRC32",0xf2,0xf,0x38,0xf1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("CVTDQ2PD",0xf3,0xf,0xe6,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("CVTDQ2PS",-1,0xf,0x5b,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("CVTPD2DQ",0xf2,0xf,0xe6,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("CVTPD2PI",0x66,0xf,0x2d,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMXMM128);
  AO("CVTPD2PS",0x66,0xf,0x5a,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("CVTPI2PD",0x66,0xf,0x2a,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("CVTPI2PS",-1,0xf,0x2a,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("CVTPS2DQ",0x66,0xf,0x5b,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("CVTPS2PD",-1,0xf,0x5a,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("CVTPS2PI",-1,0xf,0x2d,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMXMM64);
  AO("CVTSD2SI",0xf2,0xf,0x2d,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMXMM64);
  AO("CVTSD2SS",0xf2,0xf,0x5a,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("CVTSI2SD",0xf2,0xf,0x2a,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RM64);
  AO("CVTSI2SS",0xf3,0xf,0x2a,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RM64);
  AO("CVTSS2SD",0xf3,0xf,0x5a,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
  AO("CVTSS2SI",0xf3,0xf,0x2d,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMXMM32);
  AO("CVTPD2DQ",0x66,0xf,0xe6,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("CVTPD2PI",0x66,0xf,0x2c,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("CVTPS2DQ",0xf3,0xf,0x5b,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("CVTTPS2PI",-1,0xf,0x2c,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("CVTTSD2SI",0xf2,0xf,0x2c,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMXMM64);
  AO("CVTTSS2SI",0xf3,0xf,0x2c,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMXMM32);
  AO("CWD",0x66,-1,0x99,-1,0);
  AO("CDQ",-1,-1,0x99,-1,0);
  AO("CQO",-1,-1,0x99,-1,X86_OPCF_REXW);
  AO("DEC",-1,-1,0xfe,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,1);
  AO("DEC",-1,-1,0xff,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,1);
  AO("DIV",-1,-1,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,6);
  AO("DIV",-1,-1,0xf7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,6);
  AO("DIVPD",0x66,0xf,0x5e,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("DIVPS",-1,0xf,0x5e,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("DIVSD",0xf2,0xf,0x5e,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("DIVSS",0xf3,0xf,0x5e,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
  AO("DPPD",0x66,0xf,0x3a,0x41,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
  AO("DPPSS",0x66,0xf,0x3a,0x40,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("DPPSS",0x66,0xf,0x3a,0x40,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("EMMX",-1,0xf,0x77,-1,0);
  AO("ENTER",-1,-1,0xc8,-1,0,X86_OPERAND_IMM16,X86_OPERAND_IMM8);
  AO("EXTRACTPS",0x66,0xf,0x3a,0x17,X86_OPCF_MODRM,X86_OPERAND_RM32,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("F2XM1",-1,-1,0xd9,0xf0,0);
  AO("FABS",-1,-1,0xd9,0xe1,0);
  AO("FADD",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM32,X86_OPERAND_EXT,0);
  AO("FADD",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM64,X86_OPERAND_EXT,0);
  AO("FADD",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,0);
  AO("FADDP",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,0);
  AO("FBLD",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,4);
//  AO("FBSTP",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_FM80,X86_OPERAND_EXT,6);
  AO("FCHS",-1,-1,0xd9,0xe0,0);
  AO("FCLEX",0x9b,-1,0xdb,0xe2,0);
  AO("FCMOVB",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,0);
  AO("FCMOVBE",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,2);
  AO("FCMOVE",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,1);
  AO("FCMOVNB",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,0);
  AO("FCMOVNBE",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,2);
  AO("FCMOVNE",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,1);
  AO("FCMOVNU",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,3);
  AO("FCMOVU",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,3);
  AO("FCOM",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM32,X86_OPERAND_EXT,2);
  AO("FCOM",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM64,X86_OPERAND_EXT,2);
  AO("FCOM2",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,2);
  AO("FCOMI",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,6);
  AO("FCOMIP",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,6);
  AO("FCOMP",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM32,X86_OPERAND_EXT,3);
  AO("FCOMP",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M64,X86_OPERAND_EXT,3);
  AO("FCOMPP",-1,-1,0xde,0xd9,0);
  AO("FCOS",-1,-1,0xd9,0xff,0);
  AO("FDECSTP",-1,-1,0xd9,0xf6,0);
  AO("FDIV",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM32,X86_OPERAND_EXT,6);
  AO("FDIV",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M64,X86_OPERAND_EXT,6);
  AO("FDIV",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,7);
  AO("FDIVP",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,7);
  AO("FDIVR",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM32,X86_OPERAND_EXT,7);
  AO("FDIVR",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,6);
  AO("FDIVR",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M64,X86_OPERAND_EXT,7);
  AO("FDIVRP",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,6);
  AO("FFREE",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_EXT,0);
  AO("FIADD",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,0);
  AO("FIADD",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M16,X86_OPERAND_EXT,0);
  AO("FICOM",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,2);
  AO("FICOM",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M16,X86_OPERAND_EXT,2);
  AO("FICOMP",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,3);
  AO("FICOMP",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M16,X86_OPERAND_EXT,3);
  AO("FIDIV",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,6);
  AO("FIDIV",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M16,X86_OPERAND_EXT,6);
  AO("FIDIVR",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,7);
  AO("FIDIVR",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M16,X86_OPERAND_EXT,7);
  AO("FILD",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,0);
  AO("FILD",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M16,X86_OPERAND_EXT,0);
  AO("FILD",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M64,X86_OPERAND_EXT,5);
  AO("FIMUL",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,1);
  AO("FIMUL",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M16,X86_OPERAND_EXT,1);
  AO("FINCSTP",-1,-1,0xd9,0xf7,0);
  AO("FINIT",0x9b,-1,0xdb,0xe3,0);
  AO("FIST",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_M32,X86_OPERAND_EXT,2);
  AO("FIST",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,2);
  AO("FISTP",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_M32,X86_OPERAND_EXT,3);
  AO("FISTP",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,3);
  AO("FISTP",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,7);
  AO("FISTTP",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_M32,X86_OPERAND_EXT,1);
  AO("FISTTP",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,1);
  AO("FISTTP",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,1);
  AO("FISUB",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,4);
  AO("FISUB",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M16,X86_OPERAND_EXT,4);
  AO("FISUBR",-1,-1,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,5);
  AO("FISUBR",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M16,X86_OPERAND_EXT,5);
  AO("FLD",-1,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM32,X86_OPERAND_EXT,0);
//  AO("FLD",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M80,X86_OPERAND_EXT,5);
  AO("FLD",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M64,X86_OPERAND_EXT,0);
  AO("FLD1",-1,-1,0xd9,0xe8,0);
  AO("FLDCW",-1,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,5);
  AO("FLDENV",-1,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_M32,X86_OPERAND_EXT,4);
  AO("FLDL2E",-1,-1,0xd9,0xea,0);
  AO("FLDL2T",-1,-1,0xd9,0xe9,0);
  AO("FLDLG2",-1,-1,0xd9,0xec,0);
  AO("FLDLN2",-1,-1,0xd9,0xed,0);
  AO("FLDPI",-1,-1,0xd9,0xeb,0);
  AO("FLDZ",-1,-1,0xd9,0xee,0);
  AO("FMUL",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM32,X86_OPERAND_EXT,1);
  AO("FMUL",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M64,X86_OPERAND_EXT,1);
  AO("FMUL",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,1);
  AO("FMULP",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,1);
  AO("FNCLEX",-1,-1,0xdb,0xe2,0);
  AO("FNINIT",-1,-1,0xdb,0xe3,0);
  AO("FNSAVE",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,6);
  AO("FNSTCW",-1,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,7);
  AO("FNSTENV",-1,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,6);
  AO("FNSTSW",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,7);
  AO("FNSTSW",-1,-1,0xdf,0xe0,0,X86_OPERAND_CONST_R16,REG_RAX);
  AO("FPREM",-1,-1,0xd9,0xf8,0);
  AO("FPREM1",-1,-1,0xd9,0xf5,0);
  AO("FPATAN",-1,-1,0xd9,0xf3,0);
  AO("FPTAN",-1,-1,0xd9,0xf2,0);
  AO("FRNDINT",-1,-1,0xd9,0xfc,0);
  AO("FPRSTOR",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,4);
  AO("FSAVE",0x9b,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,6);
  AO("FSCALE",-1,-1,0xd9,0xfd,0);
  AO("FSIN",-1,-1,0xd9,0xfe,0);
  AO("FSINCOS",-1,-1,0xd9,0xfb,0);
  AO("FSQRT",-1,-1,0xd9,0xfa,0);
  AO("FST",-1,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_M32,X86_OPERAND_ST0,X86_OPERAND_EXT,2);
  AO("FST",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_ST0,X86_OPERAND_EXT,2);
  AO("FST",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,2);
  AO("FSTCW",0x9b,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,7);
  AO("FSTENV",0x9b,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,6);
  AO("FSTP",-1,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_M32,X86_OPERAND_ST0,X86_OPERAND_EXT,3);
//  AO("FSTP",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_M80,X86_OPERAND_ST0,X86_OPERAND_EXT,7);
  AO("FSTP",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_ST0,X86_OPERAND_EXT,3);
  AO("FSTP",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,3);
  AO("FSTSW",0x9b,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,7);
  AO("FSTSW",0x9b,-1,0xdf,0xe0,0,X86_OPERAND_CONST_R16,REG_RAX);
  AO("FSUB",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M32,X86_OPERAND_EXT,4);
  AO("FSUB",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,4);
  AO("FSUB",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M64,X86_OPERAND_EXT,4);
  AO("FSUB",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,5);
  AO("FSUBP",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,5);
  AO("FSUBR",-1,-1,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_FRM32,X86_OPERAND_EXT,5);
  AO("FSUBR",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,4);
  AO("FSUBR",-1,-1,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_M64,X86_OPERAND_EXT,5);
  AO("FSUBRP",-1,-1,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_STi,X86_OPERAND_ST0,X86_OPERAND_EXT,4);
  AO("FTST",-1,-1,0xd9,0xe4,0);
  AO("FUCOM",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,4);
  AO("FUCOMI",-1,-1,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,5);
  AO("FUCOMIP",-1,-1,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,5);
  AO("FUCOMP",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,5);
  AO("FUCOMP",-1,-1,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,5);
  AO("FUCOMPP",-1,-1,0xde,0xd9,0);
  AO("FXAM",-1,-1,0xd9,0xe5,0);
  AO("FXCH",-1,-1,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_ST0,X86_OPERAND_STi,X86_OPERAND_EXT,1);
  AO("FXRSTOR",-1,0xf,0xae,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,1);
  AO("FXSAVE",-1,0xf,0xae,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,0);
  AO("FXTRACT",-1,-1,0xd9,0xf4,0);
  AO("FYL2X",-1,-1,0xd9,0xf1,0);
  AO("FYL2XP1",-1,-1,0xd9,0xf9,0);


  AO("HADDPD",0x66,0x0f,0x7c,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("HADDPS",0xf2,0x0f,0x7c,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("HLT",-1,-1,0xf4,-1,0);
  AO("HSUBPD",0x66,0x0f,0x7d,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("HSUBPS",0xf2,0x0f,0x7d,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("IDIV",-1,-1,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,7);
  AO("IDIV",-1,-1,0xf7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,7);
  AO("IMUL3",-1,-1,0x69,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx,X86_OPERAND_IMMx);
  AO("IMUL3",-1,-1,0x6b,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx,X86_OPERAND_IMM8);
  AO("IMUL",-1,-1,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,5);
  AO("IMUL",-1,-1,0xf7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,5);
  AO("IMUL2",-1,0xf,0xaf,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);

  AO("IN",-1,-1,0xe4,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("IN",-1,-1,0xe5,-1,0,X86_OPERAND_CONST_R32,REG_RAX,X86_OPERAND_IMM8);
  AO("IN",-1,-1,0xec,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_CONST_R16,REG_RDX);
  AO("IN",-1,-1,0xed,-1,0,X86_OPERAND_CONST_R32,REG_RAX,X86_OPERAND_CONST_R16,REG_RDX);

  AO("INC",-1,-1,0xfe,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("INC",-1,-1,0xff,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,0);

  AO("INSB",-1,-1,0x6c,-1,0);
  AO("INSW",0x66,-1,0x6d,-1,0);
  AO("INSD",-1,-1,0x6d,-1,0);

  AO("INSERTPS",0x66,0xf,0x3a,0x21,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32,X86_OPERAND_IMM8);
  AO("INT3",-1,-1,0xcc,-1,0);
  AO("INT",-1,-1,0xcd,-1,0,X86_OPERAND_IMM8);
  AO("INT1",-1,-1,0xf1,-1,0,X86_OPERAND_IMM8);
  AO("ICEBP",-1,-1,0xf1,-1,0,X86_OPERAND_IMM8);
  AO("INTO",-1,-1,0xf1,-1,0);
  AO("INVD",-1,0xf,0x8,-1,0);
  AO("IRETD,IRET",-1,-1,0xcf,-1,0);
  AO("IRETQ",-1,-1,0xcf,-1,X86_OPCF_REXW);


  AO("JB,JNAE,JC",-1,-1,0x72,-1,0,X86_OPERAND_REL8);
  AO("JB,JNAE,JC",-1,0xf,0x82,-1,0,X86_OPERAND_REL32);
  AO("JBE,JNA",-1,-1,0x76,-1,0,X86_OPERAND_REL8);
  AO("JBE,JNA",-1,0xf,0x86,-1,0,X86_OPERAND_REL32);
  AO("JECXZ,JRCXZ",-1,-1,0xe3,-1,0,X86_OPERAND_REL8);
  AO("JL,JNGE",-1,-1,0x7c,-1,0,X86_OPERAND_REL8);
  AO("JL,JNGE",-1,0xf,0x8c,-1,0,X86_OPERAND_REL32);
  AO("JLE,JNG",-1,-1,0x7e,-1,0,X86_OPERAND_REL8);
  AO("JLE,JNG",-1,0xf,0x8e,-1,0,X86_OPERAND_REL32);
  AO("JMP",-1,-1,0xe9,-1,0,X86_OPERAND_REL32);
  AO("JMP",-1,-1,0xeb,-1,0,X86_OPERAND_REL8);
  AO("JMP",-1,-1,0xff,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,4);
  AO("JMPF",-1,-1,0xff,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,5);

  AO("JNB,JAE,JNC",-1,-1,0x73,-1,0,X86_OPERAND_REL8);
  AO("JNB,JAE,JNC",-1,0xf,0x83,-1,0,X86_OPERAND_REL32);
  AO("JNBE,JA",-1,-1,0x77,-1,0,X86_OPERAND_REL8);
  AO("JNBE,JA",-1,0xf,0x87,-1,0,X86_OPERAND_REL32);
  AO("JNL,JGE",-1,-1,0x7d,-1,0,X86_OPERAND_REL8);
  AO("JNL,JGE",-1,0xf,0x8d,-1,0,X86_OPERAND_REL32);
  AO("JNLE,JG",-1,-1,0x7f,-1,0,X86_OPERAND_REL8);
  AO("JNLE,JG",-1,0xf,0x8f,-1,0,X86_OPERAND_REL32);
  AO("JNO",-1,-1,0x71,-1,0,X86_OPERAND_REL8);
  AO("JNO",-1,0xf,0x81,-1,0,X86_OPERAND_REL32);
  AO("JNP,JPO",-1,-1,0x7b,-1,0,X86_OPERAND_REL8);
  AO("JNP,JPO",-1,0xf,0x8b,-1,0,X86_OPERAND_REL32);
  AO("JNZ,JNE",-1,-1,0x75,-1,0,X86_OPERAND_REL8);
  AO("JNZ,JNE",-1,0xf,0x85,-1,0,X86_OPERAND_REL32);
  AO("JO",-1,-1,0x70,-1,0,X86_OPERAND_REL8);
  AO("JO",-1,0xf,0x80,-1,0,X86_OPERAND_REL32);
  AO("JPE,JP",-1,-1,0x7a,-1,0,X86_OPERAND_REL8);
  AO("JPE,JP",-1,0xf,0x8a,-1,0,X86_OPERAND_REL32);
  AO("JNS",-1,-1,0x79,-1,0,X86_OPERAND_REL8);
  AO("JNS",-1,0xf,0x89,-1,0,X86_OPERAND_REL32);
  AO("JS",-1,-1,0x78,-1,0,X86_OPERAND_REL8);
  AO("JS",-1,0xf,0x88,-1,0,X86_OPERAND_REL32);
  AO("JZ,JE",-1,-1,0x74,-1,0,X86_OPERAND_REL8);
  AO("JZ,JE",-1,0xf,0x84,-1,0,X86_OPERAND_REL32);

  AO("LAHF",-1,-1,0x9f,-1,0);
  AO("LAR",-1,0xf,0x2,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_M16);
//  AO("LDDQU",-1,0xf,0x2,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("LDMXCSR",-1,0xf,0xae,-1,X86_OPCF_MODRM,X86_OPERAND_M32,X86_OPERAND_EXT,2);
  AO("LEA",-1,-1,0x8d,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("LEAVE",-1,-1,0xc9,-1,0);
  AO("MFENCE",-1,0xf,0xae,0xf0,0);
  AO("LFS",-1,-1,0x0f,0xb4,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("LGDT",-1,-1,0x0f,0x01,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_EXT,2);
  AO("LGS",-1,-1,0x0f,0xb5,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("LIDT",-1,-1,0x0f,0x01,X86_OPCF_MODRM,X86_OPERAND_Mx,X86_OPERAND_EXT,3);
  AO("LLDT",-1,-1,0x0f,0x00,X86_OPCF_MODRM,X86_OPERAND_Mx,X86_OPERAND_EXT,2);
  AO("LMSW",-1,-1,0x0f,0x01,X86_OPCF_MODRM,X86_OPERAND_Mx,X86_OPERAND_EXT,6);
  AO("LODSB",-1,-1,0xac,-1,0);
  AO("LODSW",0x66,-1,0xad,-1,0);
  AO("LODSD",-1,-1,0xad,-1,0);
  AO("LODSQ",-1,-1,0xad,-1,X86_OPCF_REXW);

  AO("LOOP",-1,-1,0xe2,-1,0,X86_OPERAND_REL8);
  AO("LOOPNZ,LOOPNE",-1,-1,0xe0,-1,0,X86_OPERAND_REL8);
  AO("LOOPZ,LOOPE",-1,-1,0xe1,-1,0,X86_OPERAND_REL8);

  AO("LSL",-1,0xf,0x3,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("LSS",-1,0xf,0xb2,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("LTR",-1,0xf,0xb2,-1,X86_OPCF_MODRM,X86_OPERAND_RM16,X86_OPERAND_EXT,3);
  AO("MAXPD",0x66,0xf,0x5f,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MAXPS",-1,0xf,0x5f,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MAXSD",0xf2,0xf,0x5f,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MAXSS",0xf3,0xf,0x5f,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
  AO("LFENCE",-1,0xf,0xae,0xe8,0);
  AO("MINPD",0x66,0xf,0x5d,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MINPS",-1,0xf,0x5d,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MINSD",0xf2,0xf,0x5d,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MINSS",0xf3,0xf,0x5d,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
  AO("MONITOR",-1,0xf,0x1,0xc8,X86_OPCF_MODRM,X86_OPERAND_M8,X86_OPERAND_EXT,1);

  AO("MOV",-1,-1,0x88,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("MOV",-1,-1,0x89,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("MOV",-1,-1,0x8a,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("MOV",-1,-1,0x8b,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("MOV",-1,-1,0x8c,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_SREG);
  AO("MOV",-1,-1,0x8e,-1,X86_OPCF_MODRM,X86_OPERAND_SREG,X86_OPERAND_RM16);
  AO("MOV",-1,-1,0xa0,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_MOFF8);
  AO("MOV",-1,-1,0xa1,-1,0,X86_OPERAND_CONST_Rx,REG_RAX,X86_OPERAND_MOFFx);
  AO("MOV",-1,-1,0xa2,-1,0,X86_OPERAND_MOFF8,X86_OPERAND_CONST_R8,REG_RAX);
  AO("MOV",-1,-1,0xa3,-1,0,X86_OPERAND_MOFFx,X86_OPERAND_CONST_Rx,REG_RAX);

  AO("MOV",-1,-1,0xb0,-1,X86_OPCF_ADD_REG,X86_OPERAND_R8,X86_OPERAND_IMM8);
  AO("MOV",-1,-1,0xb8,-1,X86_OPCF_ADD_REG|X86_OPCF_REXW,X86_OPERAND_R64,X86_OPERAND_IMM64);
  AO("MOV",0x66,-1,0xb8,-1,X86_OPCF_ADD_REG,X86_OPERAND_R16,X86_OPERAND_IMM16);
  AO("MOV",-1,-1,0xb8,-1,X86_OPCF_ADD_REG,X86_OPERAND_R32,X86_OPERAND_IMM32);
  AO("MOV",-1,-1,0xc6,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,0);
  AO("MOV",-1,-1,0xc7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,0);
/*
  TODO move from debug registes
*/
  AO("MOVAPD",0x66,0xf,0x28,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MOVAPD",0x66,0xf,0x29,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM128,X86_OPERAND_RXMM);
  AO("MOVAPS",-1,0xf,0x28,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MOVAPS",-1,0xf,0x29,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM128,X86_OPERAND_RXMM);
  AO("MOVBE",-1,0xf,0x38,0xf0,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("MOVBE",-1,0xf,0x38,0xf1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("MOVD",-1,0xf,0x6e,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RM32);
  AO("MOVQ",-1,0xf,0x6e,-1,X86_OPCF_MODRM|X86_OPCF_REXW,X86_OPERAND_RMM,X86_OPERAND_RM64);
  AO("MOVD",0x66,0xf,0x6e,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RM32);
  AO("MOVQ",0x66,0xf,0x6e,-1,X86_OPCF_MODRM|X86_OPCF_REXW,X86_OPERAND_RXMM,X86_OPERAND_RM64);
  AO("MOVD",-1,0xf,0x7e,-1,X86_OPCF_MODRM,X86_OPERAND_RM32,X86_OPERAND_RMM);
  AO("MOVQ",-1,0xf,0x7e,-1,X86_OPCF_MODRM|X86_OPCF_REXW,X86_OPERAND_RM64,X86_OPERAND_RMM);
  AO("MOVD",0x66,0xf,0x7e,-1,X86_OPCF_MODRM,X86_OPERAND_RM32,X86_OPERAND_RXMM);
  AO("MOVQ",0x66,0xf,0x7e,-1,X86_OPCF_MODRM|X86_OPCF_REXW,X86_OPERAND_RM64,X86_OPERAND_RXMM);
  AO("MOVDDUP",0xf2,0xf,0x12,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MOVDQ2Q",0xf2,0xf,0xd6,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMXMM64);
  AO("MOVSB",-1,-1,0xa4,-1,0);
  AO("MOVSW",0x66,-1,0xa5,-1,0);
  AO("MOVSD",-1,-1,0xa5,-1,0);
  AO("MOVSQ",-1,-1,0xa5,-1,X86_OPCF_REXW);
  AO("MOVSS",0xf3,0xf,0x10,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
  AO("MOVSS",0xf3,0xf,0x11,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM32	,X86_OPERAND_RXMM);
  AO("MOVSD2",0xf2,0xf,0x10,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MOVSD2",0xf2,0xf,0x11,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM64,X86_OPERAND_RXMM);
  AO("MOVDQA",0x66,0xf,0x6f,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MOVDQA",0x66,0xf,0x7f,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM128,X86_OPERAND_RXMM);
  AO("MOVDQU",0xf3,0xf,0x6f,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MOVDQU",0xf3,0xf,0x7f,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM128,X86_OPERAND_RXMM);
  AO("MOVHLPS",-1,0xf,0x12,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MOVHPD",0x66,0xf,0x16,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_M64);
  AO("MOVHPD",0x66,0xf,0x17,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_RXMM);
  AO("MOVHPS",-1,0xf,0x16,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_M64);
  AO("MOVHPS",-1,0xf,0x17,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_RXMM);
  AO("MOVLHPS",-1,0xf,0x16,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM);
  AO("MOVLPD",0x66,0xf,0x12,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_M64);
  AO("MOVLPD",0x66,0xf,0x13,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_RXMM);
  AO("MOVLPS",-1,0xf,0x12,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_M64);
  AO("MOVLPS",-1,0xf,0x13,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_RXMM);
  AO("MOVMSKPD",0x66,0xf,0x50,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RXMM);
  AO("MOVMSKPS",-1,0xf,0x50,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RXMM);
  AO("MOVNTDQ",0x66,0xf,0xe7,-1,X86_OPCF_MODRM,X86_OPERAND_M128,X86_OPERAND_RXMM);
  AO("MOVNTI",-1,0xf,0xc3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("MOVNTQ",-1,0xf,0xe7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_RMM);
  AO("MOVNTPD",-1,0xf,0x2b,-1,X86_OPCF_MODRM,X86_OPERAND_M128,X86_OPERAND_RXMM);
  AO("MOVNTPS",-1,0xf,0x2b,-1,X86_OPCF_MODRM,X86_OPERAND_M128,X86_OPERAND_RXMM);
  AO("MOVNTQ",-1,0xf,0xe7,-1,X86_OPCF_MODRM,X86_OPERAND_M64,X86_OPERAND_RMM);
  AO("MOVQ",-1,0xf,0x6f,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
  AO("MOVQ",0xf3,0xf,0x7e,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MOVQ",-1,0xf,0x7f,-1,X86_OPCF_MODRM,X86_OPERAND_RMMM64,X86_OPERAND_RMM);
  AO("MOVQ",0x66,0xf,0xd6,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM64,X86_OPERAND_RMM);
  AO("MOVQ2DQ",0xf3,0xf,0xd6,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMMM);
  AO("MOVSD2",0xf2,0xf,0x10,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MOVSD2",0xf2,0xf,0x11,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM64,X86_OPERAND_RXMM);
  AO("MOVSHDUP",0xf3,0xf,0x16,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MOVSLDUP",0xf3,0xf,0x12,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MOVSX",-1,0xf,0xbe,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RM8);
  AO("MOVSX",-1,0xf,0xbf,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RM16);
  AO("MOVSXD",-1,-1,0x63,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RM32);
  AO("MOVUPD",0x66,0xf,0x10,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MOVUPD",0x66,0xf,0x11,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM128,X86_OPERAND_RXMM);
  AO("MOVUPS",-1,0xf,0x10,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MOVUPS",-1,0xf,0x11,-1,X86_OPCF_MODRM,X86_OPERAND_RMXMM128,X86_OPERAND_RXMM);
  AO("MOVZX",-1,0xf,0xb6,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RM8);
  AO("MOVZX",-1,0xf,0xb7,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RM16);
  AO("MPSADBW",0x66,0xf,0x3a,0x42,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MUL",-1,-1,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,4);
  AO("MUL",-1,-1,0xf7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,4);
  AO("MULPD",0x66,0xf,0x59,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MULPS",-1,0xf,0x59,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("MULSD",0xf2,0xf,0x59,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("MULSS",0xf3,0xf,0x59,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
  AO("MWAIT",-1,0xf,0xf7,0xc9,0);
  AO("NEG",-1,-1,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,3);
  AO("NEG",-1,-1,0xf7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,3);
  AO("NOP",-1,-1,0x90,-1,0);
  AO("NOT",-1,-1,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,2);
  AO("NOT",-1,-1,0xf7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,2);
  AO("OR",-1,-1,0x08,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("OR",-1,-1,0x09,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("OR",-1,-1,0x0a,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("OR",-1,-1,0x0b,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("OR",-1,-1,0x0c,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("OR",-1,-1,0x0d,-1,0,X86_OPERAND_CONST_R64,REG_RAX,X86_OPERAND_IMMx);
  AO("OR",-1,-1,0x80,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,1);
  AO("OR",-1,-1,0x81,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,1);
  AO("OR",-1,-1,0x83,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,1);
  AO("ORPD",0x66,0xf,0x56,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("ORPS",-1,0xf,0x56,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("OUT",-1,-1,0xe6,-1,0,X86_OPERAND_IMM8,X86_OPERAND_CONST_R8,REG_RAX);
  AO("OUT",-1,-1,0xe7,-1,0,X86_OPERAND_IMM8,X86_OPERAND_CONST_R32,REG_RAX);
  AO("OUT",-1,-1,0xee,-1,0,X86_OPERAND_CONST_R16,REG_RDX,X86_OPERAND_CONST_R8,REG_RAX);
  AO("OUT",-1,-1,0xef,-1,0,X86_OPERAND_CONST_R16,REG_RDX,X86_OPERAND_CONST_R32,REG_RAX);
  AO("OUTSB",-1,-1,0x6e,-1,0);
  AO("OUTSW",0x66,-1,0x6f,-1,0);
  AO("OUTSD",-1,-1,0x6f,-1,0);

  AO("PACKSSDW",-1,0xf,0x6b,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PACKSSDW",0x66,0xf,0x6b,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PACKSSDB",-1,0xf,0x63,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PACKSSDB",0x66,0xf,0x63,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PACKUSWB",-1,0xf,0x67,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PACKUSWB",0x66,0xf,0x67,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PADDB",-1,0xf,0xfc,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PADDB",0x66,0xf,0xfc,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PADDD",-1,0xf,0xfe,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PADDD",0x66,0xf,0xfe,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PADDQ",-1,0xf,0xd4,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PADDQ",0x66,0xf,0xd4,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PADDSB",-1,0xf,0xec,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PADDSB",0x66,0xf,0xec,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PADDSW",-1,0xf,0xed,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PADDSW",0x66,0xf,0xed,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PADDUSB",-1,0xf,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PADDUSB",0x66,0xf,0xdc,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PADDUSW",-1,0xf,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PADDUSW",0x66,0xf,0xdd,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PADDW",-1,0xf,0xfd,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PADDW",0x66,0xf,0xfd,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PADDW",-1,0xf,0x3a,0xf,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PADDW",0x66,0xf,0x3a,0xf,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PAND",-1,0xf,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PAND",0x66,0xf,0xdb,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PANDN",-1,0xf,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PANDN",0x66,0xf,0xdf,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PAUSE",0xf3,-1,0x90,-1,0);
  AO("PAVGB",-1,0xf,0xe0,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PAVGB",0x66,0xf,0xe0,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PAVGW",-1,0xf,0xe3,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PAVGW",0x66,0xf,0xe3,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
//  AO("PBLENDW",0x66,0xf,0x3a,0xe,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("PCMPEQB",-1,0xf,0x74,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PCMPEQB",0x66,0xf,0x74,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PCMPEQD",-1,0xf,0x76,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PCMPEQD",0x66,0xf,0x76,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PCMPEQW",-1,0xf,0x75,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PCMPEQW",0x66,0xf,0x75,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
//PCMPESTRI
//PCMPESTRM
  AO("PCMPGTB",-1,0xf,0x64,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PCMPGTB",0x66,0xf,0x64,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PCMPGTD",-1,0xf,0x66,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PCMPGTD",0x66,0xf,0x66,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PCMPGTW",-1,0xf,0x65,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PCMPGTW",0x66,0xf,0x65,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
//PCMPISTRI
//PCMPISTRM
  AO("PEXTRB",0x66,0xf,0x3a,0x14,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PEXTRB",0x66,0xf,0x3a,0x14,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PEXTRD",0x66,0xf,0x3a,0x16,X86_OPCF_MODRM,X86_OPERAND_RM32,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PEXTRQ",0x66,0xf,0x3a,0x16,X86_OPCF_MODRM|X86_OPCF_REXW,X86_OPERAND_RM64,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PEXTRW",0x66,0xf,0x3a,0x15,X86_OPCF_MODRM,X86_OPERAND_RM16,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PEXTRW",-1,0xf,0xc5,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PEXTRW",0x66,0xf,0xc5,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMM,X86_OPERAND_IMM8);
  AO("PINSRB",0x66,0xf,0x3a,0x20,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_M8,X86_OPERAND_IMM8);
  AO("PINSRB",0x66,0xf,0x3a,0x20,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_Rx,X86_OPERAND_IMM8);
  AO("PINSRD",0x66,0xf,0x3a,0x22,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_M32,X86_OPERAND_IMM8);
  AO("PINSRD",0x66,0xf,0x3a,0x22,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_Rx,X86_OPERAND_IMM8);
  AO("PINSRQ",0x66,0xf,0x3a,0x22,X86_OPCF_MODRM|X86_OPCF_REXW,X86_OPERAND_RXMM,X86_OPERAND_RM64,X86_OPERAND_IMM8);
  AO("PINSRW",-1,0xf,0xc4,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_M16,X86_OPERAND_IMM8);
  AO("PINSRW",0x66,0xf,0xc4,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_Rx,X86_OPERAND_IMM8);
  AO("PMADDWD",-1,0xf,0xf5,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PMADDWD",0x66,0xf,0xf5,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("PMAXSW",-1,0xf,0xee,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PMAXSW",0x66,0xf,0xee,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("PMAXUB",-1,0xf,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PMAXUB",0x66,0xf,0xde,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("PMINSW",-1,0xf,0xea,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PMINSW",0x66,0xf,0xea,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("PMINUB",-1,0xf,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PMINUB",0x66,0xf,0xda,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("PMOVMSKB",-1,0xf,0xd7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_RMM);
  AO("PMOVMSKB",0x66,0xf,0xd7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_RXMM);
  AO("PMULHUW",-1,0xf,0xe4,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PMULHUW",0x66,0xf,0xe4,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_RMXMM128);
  AO("PMULHW",-1,0xf,0xe5,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PMULHW",0x66,0xf,0xe5,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_RMXMM128);
  AO("PMULLW",-1,0xf,0xd5,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PMULLW",0x66,0xf,0xd5,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_RMXMM128);
  AO("PMULUDQ",-1,0xf,0xf4,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PMULUDW",0x66,0xf,0xf4,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_RMXMM128);

  AO("POP",-1,-1,0x58,-1,X86_OPCF_ADD_REG,X86_OPERAND_R64);
  AO("POP",-1,-1,0x8f,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_EXT,0);
//POP GS
//POP FS
  AO("POPCNT",0xf3,0xf,0xb8,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("POPF",-1,-1,0x9d,-1,0);
  AO("POR",-1,0xf,0xeb,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
  AO("POR",0x66,0xf,0xeb,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM);
//  AO("PREFETCHNTA",-1,0xf,0x18,-1,X86_OPCF_MODRM,X86_OPERAND_M8,X86_OPERAND_EXT,0);
//  AO("PREFETCHT0",-1,0xf,0x18,-1,X86_OPCF_MODRM,X86_OPERAND_M8,X86_OPERAND_EXT,1);
//  AO("PREFETCHT1",-1,0xf,0x18,-1,X86_OPCF_MODRM,X86_OPERAND_M8,X86_OPERAND_EXT,2);
//  AO("PREFETCHT2",-1,0xf,0x18,-1,X86_OPCF_MODRM,X86_OPERAND_M8,X86_OPERAND_EXT,3);
  AO("PSADBW",-1,0xf,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
  AO("PSADBW",0x66,0xf,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RXMM);
  AO("PSHUFD",0x66,0xf,0x70,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PSHUFHW",0xf3,0xf,0x70,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PSHUFLW",0xf2,0xf,070,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PSHUFW",0xf2,0xf,0x70,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RXMM,X86_OPERAND_IMM8);
  AO("PSHUFW",-1,0xf,0x70,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64,X86_OPERAND_IMM8);
  AO("PSLLD",-1,0xf,0x72,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("PSLLD",0x66,0xf,0x72,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("PSLLD",-1,0xf,0xf2,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSLLD",0x66,0xf,0xf2,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RXMM);
  AO("PSLLDQ",0x66,0xf,0x73,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_EXT,7);
  AO("PSLLQ",-1,0xf,0x73,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("PSLLQ",0x66,0xf,0x73,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("PSLLQ",-1,0xf,0xf3,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSLLQ",0x66,0xf,0xf3,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSLLW",-1,0xf,0x71,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("PSLLW",0x66,0xf,0x71,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("PSLLW",-1,0xf,0xf1,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSLLW",0x66,0xf,0xf1,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSRAD",-1,0xf,0x72,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,4);
  AO("PSRAD",0x66,0xf,0x72,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,4);
  AO("PSRAD",-1,0xf,0xe2,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSRAD",0x66,0xf,0xe2,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSRAW",-1,0xf,0x71,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("PSRAW",0x66,0xf,0x71,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("PSRAW",-1,0xf,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSRAW",0x66,0xf,0xd61,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSRLD",-1,0xf,0x72,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("PSRLD",0x66,0xf,0x72,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("PSRLD",-1,0xf,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSRLD",0x66,0xf,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSRLDQ",0x66,0xf,0x73,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_EXT,3,X86_OPERAND_IMM8);
  AO("PSRLQ",-1,0xf,0x73,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("PSRLQ",0x66,0xf,0x73,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("PSRLQ",-1,0xf,0xd3,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
  AO("PSRLW",-1,0xf,0x71,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("PSRLW",0x66,0xf,0x71,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("PSRLW",-1,0xf,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
  AO("PSUBB",-1,0xf,0xf8,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSUBB",0x66,0xf,0xf8,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSUBD",-1,0xf,0xfa,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSUBD",0x66,0xf,0xfa,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSUBQ",-1,0xf,0xfb,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSUBQ",0x66,0xf,0xfb,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSUBSB",-1,0xf,0xe8,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSUBSB",0x66,0xf,0xe8,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSUBSW",-1,0xf,0xe9,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSUBSW",0x66,0xf,0xe9,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSUBUSB",-1,0xf,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSUBUSB",0x66,0xf,0xd8,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSUBUSW",-1,0xf,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSUBUSW",0x66,0xf,0xd9,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PSUBW",-1,0xf,0xf9,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PSUBW",0x66,0xf,0xf9,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PUNPCKHBW",-1,0xf,0x68,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PUNPCKHBW",0x66,0xf,0x68,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PUNPCKHDQ",-1,0xf,0x6a,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PUNPCKHDQ",0x66,0xf,0x6a,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
//  AO("PUNPCKHQDQ",0x66,0xf,0x6d,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PUNPCKHWD",-1,0xf,0x69,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PUNPCKHWD",0x66,0xf,0x69,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PUNPCKLBW",-1,0xf,0x60,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PUNPCKLBW",0x66,0xf,0x60,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PUNPCKLDQ",-1,0xf,0x62,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PUNPCKLDQ",0x66,0xf,0x62,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("PUNPCKLQDQ",0x66,0xf,0x6c,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
  AO("PUNPCKLWD",-1,0xf,0x61,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
//  AO("PUNPCKLWD",0x66,0xf,0x61,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
 
  AO("PUSH",-1,-1,0x50,-1,X86_OPCF_ADD_REG,X86_OPERAND_R64);
  AO("PUSH",-1,-1,0x68,-1,0,X86_OPERAND_IMM32);
  AO("PUSH",-1,-1,0x6a,-1,0,X86_OPERAND_IMM8);
  AO("PUSH",-1,-1,0xff,-1,X86_OPCF_MODRM,X86_OPERAND_EXT,6,X86_OPERAND_RMx);
//PUSH FS
//PUSH Gs
  AO("PUSHFQ,PUSHF",-1,-1,0x9c,-1,0);

  AO("PXOR",-1,0xf,0xef,-1,X86_OPCF_MODRM,X86_OPERAND_RMM,X86_OPERAND_RMMM64);
  AO("PXOR",0x66,0xf,0xef,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);

  AO("RCL",-1,-1,0xc0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("RCL",-1,-1,0xc1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,2);
  AO("RCL",-1,-1,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,2);
  AO("RCL",-1,-1,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,2);
  AO("RCL",-1,-1,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,2);
  AO("RCL",-1,-1,0xd3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,2);

//  AO("RCPPS",-1,0xf,0x53,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("RCPSS",0xf3,0xf,0x53,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);

  AO("RCR",-1,-1,0xc0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,3);
  AO("RCR",-1,-1,0xc1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,3);
  AO("RCR",-1,-1,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,3);
  AO("RCR",-1,-1,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,3);
  AO("RCR",-1,-1,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,3);
  AO("RCR",-1,-1,0xd3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,3);

  AO("RDMSR",-1,0xf,0x32,-1,0);
  AO("RDPMC",-1,0xf,0x33,-1,0);
  AO("RDTSC",-1,0xf,0x31,-1,0);
  AO("RDTSCP",-1,0xf,0x1,0xf9,0);

  AO("RETF1",-1,-1,0xca,-1,0,X86_OPERAND_IMM16);
  AO("RETF",-1,-1,0xcb,-1,0);
  AO("RET1",-1,-1,0xc2,-1,0,X86_OPERAND_IMM16);
  AO("RET",-1,-1,0xc3,-1,0);

  AO("ROL",-1,-1,0xc0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,0);
  AO("ROL",-1,-1,0xc1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,0);
  AO("ROL",-1,-1,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,0);
  AO("ROL",-1,-1,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,0);
  AO("ROL",-1,-1,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,0);
  AO("ROL",-1,-1,0xd3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,0);
  AO("ROR",-1,-1,0xc0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,1);
  AO("ROR",-1,-1,0xc1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,1);
  AO("ROR",-1,-1,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,1);
  AO("ROR",-1,-1,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,1);
  AO("ROR",-1,-1,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,1);
  AO("ROR",-1,-1,0xd3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,1);

  AO("ROUNDPD",0x66,0xF,0x3A,0x9,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("ROUNDPS",0x66,0xF,0x3A,0x8,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("ROUNDSD",0x66,0xF,0x3A,0xa,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64,X86_OPERAND_IMM8);
  AO("ROUNDSS",0x66,0xF,0x3A,0xb,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32,X86_OPERAND_IMM8);

  AO("RSM",-1,0xf,0xaa,-1,0);

  AO("RSQRTPS",-1,0xf,0x52,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("RSQRTSS",0xf3,0xf,0x52,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);

  AO("SAHF",-1,-1,0x9e,-1,0);

  AO("SAL",-1,-1,0xc0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("SAL",-1,-1,0xc1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("SAL",-1,-1,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,6);
  AO("SAL",-1,-1,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,6);
  AO("SAL",-1,-1,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,6);
  AO("SAL",-1,-1,0xd3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,6);
  AO("SAR",-1,-1,0xc0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,7);
  AO("SAR",-1,-1,0xc1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,7);
  AO("SAR",-1,-1,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,7);
  AO("SAR",-1,-1,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,7);
  AO("SAR",-1,-1,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,7);
  AO("SAR",-1,-1,0xd3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,7);

  AO("SBB",-1,-1,0x18,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("SBB",-1,-1,0x19,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("SBB",-1,-1,0x1a,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("SBB",-1,-1,0x1b,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("SBB",-1,-1,0x1c,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("SBB",-1,-1,0x1d,-1,0,X86_OPERAND_CONST_R64,REG_RAX,X86_OPERAND_IMMx);
  AO("SBB",-1,-1,0x80,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,3);
  AO("SBB",-1,-1,0x81,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,3);
  AO("SBB",-1,-1,0x83,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,3);


  AO("SETB,SETNAE,SETC",-1,0xf,0x92,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETBE,SETNA",-1,0xf,0x96,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETL,SETNGE",-1,0xf,0x9c,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETLE,SETNG",-1,0xf,0x9e,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETNB,SETAE,SETNC",-1,0xf,0x93,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETNBE,SETA",-1,0xf,0x97,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETNL,SETGE",-1,0xf,0x9d,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETNLE,SETG",-1,0xf,0x9f,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETNO",-1,0xf,0x91,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETNP,SETPO",-1,0xf,0x9b,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETNS",-1,0xf,0x99,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETNZ,SETNE",-1,0xf,0x95,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETO",-1,0xf,0x90,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETP,SETPE",-1,0xf,0x9a,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETS",-1,0xf,0x98,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);
  AO("SETZ,SETE",-1,0xf,0x94,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_EXT,0);

  AO("SCASB",-1,-1,0xae,-1,0);
  AO("SCASW",0x66,-1,0xaf,-1,0);
  AO("SCASD",-1,-1,0xaf,-1,0);
  AO("SCASQ",-1,-1,0xaf,-1,X86_OPCF_REXW);

//SGDT
  AO("SHL,SAL",-1,-1,0xc0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,4);
  AO("SHL,SAL",-1,-1,0xc1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,4);
  AO("SHL,SAL",-1,-1,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,4);
  AO("SHL,SAL",-1,-1,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,4);
  AO("SHL,SAL",-1,-1,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,4);
  AO("SHL,SAL",-1,-1,0xd3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,4);

  AO("SHLD",-1,0xf,0xa4,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx,X86_OPERAND_IMM8);
  AO("SHLD",-1,0xf,0xa5,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx,X86_OPERAND_CONST_R8,REG_RCX);

  AO("SHR",-1,-1,0xc0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,5);
  AO("SHR",-1,-1,0xc1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,5);
  AO("SHR",-1,-1,0xd0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,5);
  AO("SHR",-1,-1,0xd1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_IMM,1,X86_OPERAND_EXT,5);
  AO("SHR",-1,-1,0xd2,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,5);
  AO("SHR",-1,-1,0xd3,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_CONST_R8,REG_RCX,X86_OPERAND_EXT,5);

  AO("SHRD",-1,0xf,0xac,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx,X86_OPERAND_IMM8);
  AO("SHRD",-1,0xf,0xad,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx,X86_OPERAND_CONST_R8,REG_RCX);

  AO("SHUFPD",0x66,0xF,0xc6,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);
  AO("SHUFPS",-1,0xF,0xc6,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128,X86_OPERAND_IMM8);

//SIDT
//SLDT
//SMSW
  AO("SQRTPD",0x66,0xF,0x51,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("SQRTPS",-1,0xF,0x51,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("SQRTSD",0xf2,0xF,0x51,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("SQRTSS",0xf3,0xF,0x51,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
  
  AO("STC",-1,-1,0xf9,-1,0);  
  AO("STD",-1,-1,0xfd,-1,0);  
  AO("STI",-1,-1,0xfb,-1,0);  
//STMXCSR
  AO("STOSB",-1,-1,0xaa,-1,0);
  AO("STOSW",0x66,-1,0xaa,-1,0);
  AO("STOSD",-1,-1,0xab,-1,0);
  AO("STOSQ",-1,-1,0xab,-1,X86_OPCF_REXW);
//STR
  AO("SUB",-1,-1,0x28,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("SUB",-1,-1,0x29,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("SUB",-1,-1,0x2a,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("SUB",-1,-1,0x2b,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("SUB",-1,-1,0x2c,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("SUB",-1,-1,0x2d,-1,0,X86_OPERAND_CONST_Rx,REG_RAX,X86_OPERAND_IMMx);
  AO("SUB",-1,-1,0x80,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,5);
  AO("SUB",-1,-1,0x81,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,5);
  AO("SUB",-1,-1,0x83,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,5);

  AO("SUBPD",0x66,0xF,0x5c,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("SUBPS",-1,0xF,0x5c,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("SUBSD",0xf2,0xF,0x5c,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("SUBSS",0xf3,0xF,0x5c,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);

  AO("VERR",-1,0xf,0x00,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,4);
  AO("VERW",-1,0xf,0x00,-1,X86_OPCF_MODRM,X86_OPERAND_M16,X86_OPERAND_EXT,5);

//SWAPGS
  //SYSCAL
  //SYSENTER
  //SYSRET
  AO("TEST",-1,-1,0x84,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("TEST",-1,-1,0x85,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("TEST",-1,-1,0xa8,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("TEST",-1,-1,0xa9,-1,0,X86_OPERAND_CONST_Rx,REG_RAX,X86_OPERAND_IMMx);
  AO("TEST",-1,-1,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,0);
  AO("TEST",-1,-1,0xf6,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,1);
  AO("TEST",-1,-1,0xf7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,0);
  AO("TEST",-1,-1,0xf7,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,1);

  AO("UD",-1,0x0F,0xB9,-1,0);
  AO("UD2",-1,0x0F,0xB,-1,0);

  AO("UCOMISD",0x66,0xF,0x2e,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("UCOMISS",-1,0xF,0x2e,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM32);
//  AO("UNPCKHPD",0x66,0xF,0x15,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("UNPCKHPS",-1,0xF,0x15,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
//  AO("UNPCKLPD",0x66,0xF,0x14,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("UNPCKLPS",-1,0xF,0x14,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM64);
  AO("WBINVD",-1,0xF,0x09,-1,0);
  AO("WRMSR",-1,0xf,0x30,-1,0);
  AO("XADD",-1,0xf,0xc0,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("XADD",-1,0xf,0xc1,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("XCHG",-1,-1,0x86,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("XCHG",-1,-1,0x87,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("XCHG",-1,-1,0x90,-1,X86_OPCF_ADD_REG,X86_OPERAND_Rx,X86_OPERAND_CONST_Rx,REG_RAX);
  AO("XGETBV",-1,0xF,0x1,0xD0,0);
  AO("XLAT,XLATB",-1,-1,0xd7,-1,0);

  AO("XOR",-1,-1,0x30,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_R8);
  AO("XOR",-1,-1,0x31,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_Rx);
  AO("XOR",-1,-1,0x32,-1,X86_OPCF_MODRM,X86_OPERAND_R8,X86_OPERAND_RM8);
  AO("XOR",-1,-1,0x33,-1,X86_OPCF_MODRM,X86_OPERAND_Rx,X86_OPERAND_RMx);
  AO("XOR",-1,-1,0x34,-1,0,X86_OPERAND_CONST_R8,REG_RAX,X86_OPERAND_IMM8);
  AO("XOR",-1,-1,0x35,-1,0,X86_OPERAND_CONST_Rx,REG_RAX,X86_OPERAND_IMMx);
  AO("XOR",-1,-1,0x80,-1,X86_OPCF_MODRM,X86_OPERAND_RM8,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);
  AO("XOR",-1,-1,0x81,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMMx,X86_OPERAND_EXT,6);
  AO("XOR",-1,-1,0x83,-1,X86_OPCF_MODRM,X86_OPERAND_RMx,X86_OPERAND_IMM8,X86_OPERAND_EXT,6);

  AO("LOCK",-1,-1,0xf0,-1,0);
  AO("REPNE,REPNZ",-1,-1,0xf2,-1,0);
  AO("REP,REPE,REPZ",-1,-1,0xf3,-1,0);
  AO("XORPD",0x66,0xF,0x57,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
  AO("XORPS",-1,0xF,0x57,-1,X86_OPCF_MODRM,X86_OPERAND_RXMM,X86_OPERAND_RMXMM128);
//XRSTOR
//XSAVE
//XSAVE
//XSETBV
}
I8 SSE_ShuffleByte(I64 a,I64 b,I64 c,I64 d) {
//I8 noreg  forces no promition
  I8 noreg r=(a&3)|((b&3)<<2)|((c&3)<<4)|((d&3)<<6);
  return r;
}
UAsmInit;
#ifdef ASM_TEST
U8 *d="\x55\x48\x89\xE5\xC9\xC3";
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);

d=
"\x55\x48\x89\xE5\x48\xC7\xC0\x00\x00\x00\x00\xF0\x48\x0F\xBA\x2F\x01\x72\x07\x48\xC7\xC0\x01\x00\x00\x00\xC9\xC3";
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);
U8 *od;
d="\xF3\x0F\x7E\x04\xB0\x14\xfe";
od=d;
d=DisasOpc(DocPut,d);
d=DisasOpc(DocPut,d);

/*
//ModRM/SIB Fuzz test
U8 buf[STR_LEN];
CDoc *doc=DocNew("fuzzy.DD");
I64 modrm=0,sib,sz;
for(modrm=0;modrm<=0xff;modrm++) {
  switch(modrm&011000000) {
    case 0: sz=0;
      break;
    case 0b01000000:
      sz=1;
      break;
    case 0b10000000:
      sz=4;
      break;
  }
  buf[0]=0x11;
  buf[1]=modrm;
  if(modrm&0b111==0b100) {
    for(sib=0;sib<=0xff;sib++) {
      buf[2]=sib;
      buf[3](I32)=0x11223344;
      DocPrint(doc,"MODRM/SIB:%X/%X is ",modrm,sib);
      DisasOpc(doc,buf);
    }
  } else {
    DocPrint(doc,"MODRM:%X is ",modrm,sib);
    buf[2](I32)=0x11223344;
    DisasOpc(doc,buf);
  }
}
DocWrite(doc);
DocDel(doc);*/
/*
ADC BYTE PTR [RAX],BL
ADC QWORD PTR [RAX+RBX*8],RBX
ADC BL,BYTE PTR [RAX]
ADC RBX,QWORD PTR [RAX+RBX*8]
ADC AL,0xfe
ADC RAX,0xf00f1e
ADC BYTE PTR [RAX],0xfe
ADC QWORD PTR [RAX+RBX*8],0xf00f1e
ADC QWORD PTR [RAX+RBX*8],0xfe

ADD BYTE PTR [RAX],BL
ADD QWORD PTR [RAX+RBX*8],RBX
ADD BL,BYTE PTR [RAX]
ADD RBX,QWORD PTR [RAX+RBX*8]
ADD AL,0xfe
ADD RAX,0xf00f1e
ADD BYTE PTR [RAX],0xfe
ADD QWORD PTR [RAX+RBX*8],0xf00f1e
ADD QWORD PTR [RAX+RBX*8],0xfe
*/
d="\x10\x18\x48\x11\x1C\xD8\x12\x18\x48\x13"
"\x1C\xD8\x14\xFE\x48\x15\x1E\x0F\xF0\x00"
"\x80\x10\xFE\x48\x81\x14\xD8\x1E\x0F\xF0"
"\x00\x48\x81\x14\xD8\xFE\x00\x00\x00\x00"
"\x18\x48\x01\x1C\xD8\x02\x18\x48\x03\x1C"
"\xD8\x04\xFE\x48\x05\x1E\x0F\xF0\x00\x80"
"\x00\xFE\x48\x81\x04\xD8\x1E\x0F\xF0\x00"
"\x48\x81\x04\xD8\xFE\x00\x00\x00";
I64 cnt=9*2;
while(cnt--)
  d=DisasOpc(DocPut,d);
/*
ADDPD XMM0, [RAX]
ADDPS XMM0, [RAX]
ADDSD XMM0, [RAX]
ADDSS XMM0, [RAX]
ADDSUBPD XMM0, [RAX]
ADDSUBPS XMM0, [RAX]
*/
d="\x66\x0F\x58\x00\x0F\x58\x00\xF2\x0F\x58"
"\x00\xF3\x0F\x58\x00\x66\x0F\xD0\x00\xF2"
"\x0F\xD0\x00";
cnt=6;
while(cnt--)
  d=DisasOpc(DocPut,d);
/*
AND BYTE PTR [RAX],BH
AND QWORD PTR [RAX],RBX
AND BH,BYTE PTR [RAX]
AND RBX,QWORD PTR [RAX]
AND AL,0xfe
AND RAX,0xf00f1e
AND BYTE PTR [RAX],0xfe
AND QWORD PTR [RAX],0xf00f1e
ANDNPD XMM0,[RAX]
ANDNPS XMM0,[RAX]
ANDPD XMM0,[RAX]
ANDPS XMM0,[RAX]
BLENDPS XMM0,[RAX],1
BLENDPD XMM0,[RAX],1
BSR RAX,[RAX]
BSF RAX,[RAX]
BSWAP RAX
BT QWORD PTR [RAX],1
BT QWORD PTR [RAX],RBX
BTS QWORD PTR [RAX],1
BTS QWORD PTR [RAX],RBX
BTR QWORD PTR [RAX],1
BTR QWORD PTR [RAX],RBX
BTC QWORD PTR [RAX],1
BTC QWORD PTR [RAX],RBX
CBW
CWDE
CDQE
CLC
CLD
CLFLUSH [RAX]
CLI
CMC
CMOVB RAX,[RAX]
CMOVBE R14,[R14]
CMOVL R14,[R14]
CMOVLE R14,[R14]
CMOVNB R14,[R14]
CMOVNBE R14,[R14+R15]
CMOVNL R14,[R14+R15]
CMOVNLE R14,[R14+R15]
CMOVNO R14,[R14+R15]
CMOVNP R14,[R14+R15]
CMOVNS R14,[R14+R15]
CMOVNZ R14,[R14+R15]
CMOVO R14,[R14+R15]
CMOVP R14,[R14+R15]
CMOVS R14,[R14+R15]
CMOVZ R14,[R14+R15]
CMP BYTE PTR [RAX],BH
CMP QWORD PTR [RAX],RBX
CMP BH,BYTE PTR [RAX]
CMP RBX,QWORD PTR [RAX]
CMP AL,0xfe
CMP RAX,0xf00f1e
CMP BYTE PTR [RAX],0xfe
CMP QWORD PTR [RAX],0xf00f1e
CMPSB
CMPSW
CMPSD
CMPSQ
CMPSD XMM0,QWORD PTR [RAX],1
CMPSS XMM0,DWORD PTR [RAX],1
CMPXCHG BYTE PTR [RAX],BH
CMPXCHG QWORD PTR [RAX],RBX
CMPXCHG8B QWORD PTR [RAX]
CMPXCHG16B [RAX]
COMISD XMM0,QWORD PTR [RAX]
COMISS XMM0,DWORD PTR [RAX]
CRC32 RAX,BYTE PTR [RAX]
CRC32 RAX,RBX
CVTPI2PD XMM0,[RAX]
CVTPI2PS XMM0,[RAX]
CVTPD2PI MM0,[RAX]
CVTPS2PI MM0,[RAX]
CVTSD2SS XMM0,[RAX]
CVTSS2SD XMM0,[RAX]
CVTSS2SI RAX,[RAX]
CVTTPS2PI MM0,XMM0
CVTTSS2SI RAX,[RAX]
CVTTSS2SI RAX,[RAX]
CWD
CDQ
CQO
DEC BL
DEC QWORD PTR [RCX]
DIV DIL
DIV RDX
DIVSD XMM0,[RAX]
DIVSS XMM0,[RAX]
ENTER 16,7
EXTRACTPS EDX,XMM0,1
F2xM1
FABS
FADD ST(1)
FADD QWORD PTR [RAX]
FADD DWORD PTR [RAX]
FADD ST(1),ST(0)
FADDP ST(2),ST(0)
FCHS
FCLEX
FCMOVB ST(0),ST(1)
FCMOVBE ST(0),ST(1)
FCMOVE ST(0),ST(1)
FCMOVNB ST(0),ST(1)
FCMOVNBE ST(0),ST(1)
FCMOVNE ST(0),ST(1)
FCMOVNU ST(0),ST(1)
FCMOVU ST(0),ST(1)
FCOM ST(2)
FCOM DWORD PTR [RAX]
FCOM QWORD PTR [RAX]
FCOMI ST(2)
FCOMP ST(2)
FCOMP DWORD PTR [RAX]
FCOMP QWORD PTR [RAX]
FCOMIP ST(2)
FCOMPP
FCOS
FDECSTP
FDIV ST(2)
FDIV QWORD PTR [RAX]
FDIV DWORD PTR [RAX]
FDIV ST(2),ST
FDIVP ST(2)
FDIVR ST(2)
FDIVR QWORD PTR [RAX]
FDIVR DWORD PTR [RAX]
FDIVR ST(2),ST
FFREE ST(1)
FIADD DWORD PTR [RAX]
FIADD WORD PTR [RAX]
FICOM DWORD PTR [RAX]
FICOM WORD PTR [RAX]
FICOMP DWORD PTR [RAX]
FICOMP WORD PTR [RAX]
FIDIV DWORD PTR [RAX]
FIDIV WORD PTR [RAX]
FIDIVR DWORD PTR [RAX]
FIDIVR WORD PTR [RAX]
FILD DWORD PTR [RAX]
FILD WORD PTR [RAX]
FILD QWORD PTR [RAX]
FIMUL DWORD PTR [RAX]
FIMUL WORD PTR [RAX]
FINCSTP
FINIT
FIST DWORD PTR [RAX]
FIST WORD PTR [RAX]
FISTP DWORD PTR [RAX]
FISTP WORD PTR [RAX]
FISTP QWORD PTR [RAX]
FISTTP DWORD PTR [RAX]
FISTTP WORD PTR [RAX]
FISTTP QWORD PTR [RAX]
FISUB DWORD PTR [RAX] 
FISUB WORD PTR [RAX] 
FISUBR DWORD PTR [RAX] 
FISUBR WORD PTR [RAX] 
FLD ST(1)
FLD DWORD PTR [RAX]
FLD QWORD PTR [RAX]
FLD1
FLDCW [RAX]
FLDENV [RAX]
FLDL2E
FLDL2T
FLDLG2
FLDLN2
FLDPI
FLDZ
FMUL ST,ST(1)
FMUL DWORD PTR [RAX]
FMUL QWORD PTR [RAX]
FMUL ST(1),ST
FMULP ST(1),ST
FNCLEX
FNINIT
FNOP
FNSTCW [RAX]
FNSTENV [RAX]
FNSTSW [RAX]
FNSTSW AX
FPATAN
FPREM
FPREM1
FPTAN
FRNDINT
FSCALE
FSIN
FSINCOS
FSQRT
FST DWORD PTR [RAX]
FST QWORD PTR [RAX]
FST ST(1)
FSTCW [rAX]
FSTENV [RAX]
FSTP DWORD PTR [RAX]
FSTP QWORD PTR [RAX]
FSTP ST(1)
FSTSW [RAX]
FSTSW AX
FSUB DWORD PTR [RAX]
FSUB QWORD PTR [RAX]
FSUB ST(1),ST
FSUB ST,ST(1)
FSUBP ST(1),ST
FSUBR DWORD PTR [RAX]
FSUBR QWORD PTR  [RAX]
FSUBR ST(1),ST
FUCOM ST(1)
FUCOMI ST(1)
FUCOMIP ST(1)
FUCOMP ST(1)
FXAM
FXCH ST(1)
HLT
IDIV BYTE PTR [RAX]
IDIV QWORD PTR [RAX]
IMUL RDX,RBX,3
IMUL RDX,RBX,0x100
IMUL DIL
IMUL RDI
FXTRACT
FYL2X
FYL2XP1
IN AL,1
IN EAX,1
IN AL,DX
IN EAX,Dx
INC BYTE PTR [R15]
INC R15
INSB
INSW
INSD
INSERTPS XMM0,XMM1,1
INSERTPS XMM0,DWORD PTR [RAX],1
INT 3
INT 123
INVD
IRET
IRETD
IRETQ
JB _jb
_jb:
JBE _jbe
_jbe:
JECXZ _jecxz
_jecxz:
JL _jl
_jl:
JLE _jle
_jle:
JMP _jmp
_jmp:
JMP RAX
JNB _jnb
_jnb:
JNBE _jnbe
_jnbe:
JNL _jnl
_jnl:
JNLE _jnle
_jnle:
JNO _jno
_jno:
JNP _jnp
_jnp:
JNS _jns
_jns:
JNZ _jnz
_jnz:
JO _jo
_jo:
JP _jp
_jp:
JS _js
_js:
JZ _jz
_jz:
LAHF
LEA R15,[R14+R15*8]
LEAVE
LFENCE
LODSB
LODSW
LODSD
LODSQ
l1:
LOOP l1
l2:
LOOPNE l2
l3:
LOOPZ l3
MAXSD XMM0,XMM15
MAXSS XMM0,XMM15
MFENCE
MINSD XMM0,XMM15
MINSS XMM0,XMM15
MOV BH,[RAX]
MOV RBX,[RAX]
MOV [RAX],BH
MOV [RAX],RBX
MOV [0x112233445566],AL
MOV [0x112233445566],RAX
MOV AL,[0x112233445566]
MOV RAX,[0x112233445566]
MOV DIL,1
MOV RDI,1
MOV RDI,0x112233
MOV BYTE PTR [RAX],1
MOV QWORD PTR [RAX],1
MOV QWORD PTR[ RAX],0x112233
MOVBE [RAX],RBX
MOVBE RBX,[RAX];
MOVQ XMM1,RAX
MOVD XMM1,EAX
MOVQ RAX,XMM1
MOVD EAX,XMM1
MOVDDUP XMM0,XMM1
MOVDQ2Q MM0,XMM1
MOVHLPS XMM0,XMM1
MOVHPD XMM0,QWORD PTR [RAX]
MOVHPD QWORD PTR [RAX],XMM0
MOVHPS XMM0,QWORD PTR [RAX]
MOVHPS QWORD PTR [RAX],XMM0
MOVLHPS XMM0,XMM1
MOVLPD XMM0,QWORD PTR [RAX]
MOVLPD QWORD PTR [RAX],XMM0
MOVLPS XMM0,QWORD PTR [RAX]
MOVLPS QWORD PTR [RAX],XMM0
MOVMSKPD RAX,XMM0
MOVMSKPS RAX,XMM0
MOVNTI QWORD PTR [RAX],RAX
MOVNTQ QWORD PTR [RAX],MM0
MOVQ2DQ XMM0,MM0
MOVSB
MOVSW
MOVSD
MOVSQ
MOVSD XMM0,QWORD PTR [RAX]
MOVSD QWORD PTR[RAX],XMM0
MOVSHDUP XMM0,XMM1
MOVSLDUP XMM0,XMM1
MOVSS DWORD PTR [RAX],XMM0
MOVSS XMM0,DWORD PTR [RAX]
MOVSX RBX,BL
MOVSX RBX,BX
MOVSXD RBX,EBX
MOVZX RBX,BL
MOVZX RBX,BX
MUL BH
MUL RBX
MULSD XMM0,QWORD PTR [RAX]
MULSS XMM0,DWORD PTR [RAX]
NEG BH
NEG RBX
NOT BH
NOT RBX
OR BYTE PTR [RAX],BL
OR QWORD PTR [RAX],RBX
OR BL,BYTE PTR [RAX]
OR RBX,QWORD PTR [RAX]
OR AL,1
OR RAX,0x112233
OR BL,1
OR RBX,0x112233
OR RBX,1
OUT 1,AL
OUT 1,EAX
OUT DX,EAX
OUTSB
OUTSW
OUTSD
PACKSSDW MM0,QWORD PTR [RAX]
PACKSSWB MM0,QWORD PTR [RAX]
PACKUSWB MM0,QWORD PTR [RAX]
PADDB MM0,QWORD PTR [RAX]
PADDD MM0,QWORD PTR [RAX]
PADDQ MM0,QWORD PTR [RAX]
PADDSB MM0,QWORD PTR [RAX]
PADDSW MM0,QWORD PTR [RAX]
PADDUSB MM0,QWORD PTR [RAX]
PADDUSW MM0,QWORD PTR [RAX]
PADDW MM0,QWORD PTR [RAX]
PAND MM0,QWORD PTR [RAX]
PANDN MM0,QWORD PTR [RAX]
PAUSE
PAVGB MM0,QWORD PTR [RAX]
PAVGW MM0,QWORD PTR [RAX]
PCMPEQB MM0,QWORD PTR [RAX]
PCMPEQD MM0,QWORD PTR [RAX]
PCMPEQW MM0,QWORD PTR [RAX]
PCMPGTB MM0,QWORD PTR [RAX]
PCMPGTD MM0,QWORD PTR [RAX]
PCMPGTW MM0,QWORD PTR [RAX]
PEXTRB BYTE PTR [RAX],XMM0,1
PEXTRD DWORD PTR[RAX],XMM0,1
PEXTRQ QWORD PTR[RAX],XMM0,1
PEXTRW WORD PTR[RAX],XMM0,1
PINSRB XMM0,EAX,1
PINSRW XMM0,EAX,1
PINSRD XMM0,EAX,1
PINSRQ XMM0,RAX,1
PMADDWD MM0,QWORD PTR [RAX]
PMAXSW MM0,QWORD PTR [RAX]
PMAXUB MM0,QWORD PTR [RAX]
PMINSW MM0,QWORD PTR [RAX]
PMINUB MM0,QWORD PTR [RAX]
PMOVMSKB EAX,MM0
PMOVMSKB EAX,XMM0
PMULHUW MM0,QWORD PTR [RAX]
PMULHW MM0,QWORD PTR [RAX]
PMULLW MM0,QWORD PTR [RAX]
PMULUDQ MM0,QWORD PTR [RAX]
POP R15
POP WORD PTR [RAX]
POP WORD PTR [RAX]
POPCNT RBX,QWORD PTR [RAX]
POPF
POPFQ
POR MM0,MM1
PREFETCHNTA BYTE PTR [RAX]
PREFETCHT0  BYTE PTR [RAX]
PREFETCHT1  BYTE PTR [RAX]
PREFETCHT2  BYTE PTR [RAX]
PSADBW MM0,MM7
PSHUFW MM0,MM1,2
PSLLD MM0,2
PSLLD XMM15,2
PSLLD MM0,QWORD PTR [RAX]
PSLLQ MM0,2
PSLLQ XMM15,2
PSLLW MM0,2
PSLLW XMM15,2
PSLLW MM0,QWORD PTR [RAX]
PSRAD MM0,2
PSRAD XMM15,2
PSRAD MM0,QWORD PTR [RAX]
PSRAW MM0,2
PSRAW XMM15,2
PSRAW MM0,QWORD PTR [RAX]
PSRLD MM0,2
PSRLD XMM15,2
PSRLD MM0,QWORD PTR [RAX]
PSRLQ MM0,2
PSRLQ XMM15,2
PSRLW MM0,2
PSRLW XMM15,2
PSRLW MM0,QWORD PTR [RAX]
PSUBB MM0,QWORD PTR [RAX]
PSUBD MM0,QWORD PTR [RAX]
PSUBQ MM0,QWORD PTR [RAX]
PSUBSB MM0,QWORD PTR [RAX]
PSUBSW MM0,QWORD PTR [RAX]
PSUBUSB MM0,QWORD PTR [RAX]
PSUBW MM0,QWORD PTR [RAX]
PUNPCKHBW MM0,QWORD PTR [RAX]
PUNPCKHDQ MM0,QWORD PTR [RAX]
PUNPCKHWD MM0,QWORD PTR [RAX]
PUSH R15
PUSH 0x11ff
PUSH 0x11223344
PUSH 0x1
PUSHF
PXOR MM0,MM7
RCL RBX,3
RCL BL,3
RCL RBX,1
RCL BL,1
RCL RBX,CL
RCL BL,CL
RCL RBX,3
RCR BL,3
RCR RBX,1
RCR BL,1
RCR RBX,CL
RCR BL,CL
RET 15
ROL RBX,3
ROL BL,3
ROL RBX,1
ROL BL,1
ROL RBX,CL
ROL BL,CL
ROL RBX,3
ROR BL,3
ROR RBX,1
ROR BL,1
ROR RBX,CL
ROR BL,CL
ROUNDSD XMM0,QWORD PTR [RAX],1
ROUNDSS XMM0,DWORD PTR [RAX],1
SAHF
SAL BL,3
SAL RBX,3
SAL BL,1
SAL RBX,1
SAL BL,CL
SAL RBX,CL
SAR BL,3
SAR RBX,3
SAR BL,1
SAR RBX,1
SAR BL,CL
SAR RBX,CL

SBB [RBX],RBX
SBB BL,BL 
SBB RBX,RBX
SBB BL,1
SBB RBX,0x1122
SBB RBX,1
SBB AL,1
SBB RAX,0x1122

SCASB
SCASD
SCASW
SCASQ

SETB AL
SETBE AL
SETL AL
SETLE AL
SETNB AL
SETNBE AL
SETNL AL
SETNO AL
SETNP AL
SETNS AL
SETS AL
SETP AL
SETO AL
SETNZ AL
SETZ AL

SHLD RAX,RBX,1
SHLD RAX,RBX,CL
SHRD RAX,RBX,1
SHRD RAX,RBX,CL
SHR BL,1
SHR RBX,1
SHR BL,3
SHR RBX,3
SHR BL,CL
SHR RBX,CL
SQRTSS XMM0,XMM1
SQRTSD XMM0,XMM1
STD
STI
STC
STOSB
STOSD
STOSW
STOSQ
SUB BL,BYTE PTR [RAX]
SUB EBX,DWORD PTR [RAX]
SUB BYTE PTR [RAX],BL
SUB DWORD PTR [RAX],EBX
SUB AL,1
SUB EAX,1
SUB BL,1
SUB EBX,0x1122
SUB EBX,1
SUBSD XMM0,[RAX]
SUBSS XMM0,[RAX]
TEST R8b,BYTE PTR[R15]
TEST R8, [R15]
TEST AL,1
TEST RAX,0x1122
TEST RAX,1
TEST R8b,1
TEST R8,0x1122
TEST R8,1
UCOMISD XMM0,[RAX]
UCOMISS XMM0,[RAX]
UD2
UNPCKHPS XMM0,XMM1
UNPCKLPS XMM0,XMM9
XADD R8b,R9b
XADD R8,R9
XCHG R8b,R9b
XCHG R8,RAX
XLATB
XOR BL,BYTE PTR [RAX]
XOR EBX,DWORD PTR [RAX]
XOR BYTE PTR [RAX],BL
XOR DWORD PTR [RAX],EBX
XOR AL,1
XOR EAX,1
XOR BL,1
XOR EBX,0x1122
XOR EBX,1
*/
d="\x20\x38\x48\x21\x18\x22\x38\x48\x23\x18"
"\x24\xFE\x48\x25\x1E\x0F\xF0\x00\x80\x20"
"\xFE\x48\x81\x20\x1E\x0F\xF0\x00"
"\x66\x0F\x55\x00\x0F\x55\x00\x66\x0F\x54"
"\x00\x0F\x54\x00""\x66\x0F\x3A\x0C\x00\x01"
"\x66\x0F\x3A\x0D\x00\x01"
"\x48\x0F\xBD\x00\x48\x0F\xBC\x00\x48\x0F"
"\xC8\x48\x0F\xBA\x20\x01\x48\x0F\xA3\x18"
"\x48\x0F\xBA\x28\x01\x48\x0F\xAB\x18\x48"
"\x0F\xBA\x30\x01\x48\x0F\xB3\x18\x48\x0F"
"\xBA\x38\x01\x48\x0F\xBB\x18"
"\x66\x98\x98\x48\x98\xF8\xFC\x0F\xAE\x38\xFA\xF5"
"\x48\x0F\x42\x00\x4D\x0F\x46\x36\x4D\x0F\x4C\x36"
"\x4D\x0F\x4E\x36\x4D\x0F\x43\x36\x4F\x0F\x47\x34"
"\x3E\x4F\x0F\x4D\x34\x3E\x4F\x0F\x4F\x34\x3E\x4F"
"\x0F\x41\x34\x3E\x4F\x0F\x4B\x34\x3E\x4F\x0F\x49"
"\x34\x3E\x4F\x0F\x45\x34\x3E\x4F\x0F\x40\x34\x3E"
"\x4F\x0F\x4A\x34\x3E\x4F\x0F\x48\x34\x3E\x4F\x0F"
"\x44\x34\x3E""\x38\x38\x48\x39\x18\x3A\x38\x48"
"\x3B\x18\x3C\xFE\x48\x3D\x1E\x0F\xF0\x00\x80\x38"
"\xFE\x48\x81\x38\x1E\x0F\xF0\x00"
"\xA6\x66\xA7\xA7\x48\xA7\xF2\x0F\xC2\x00\x01\xF3"
"\x0F\xC2\x00\x01""\x0F\xB0\x38\x48\x0F\xB1\x18"
"\x0F\xC7\x08\x48\x0F\xC7\x08"
"\x66\x0F\x2F\x00\x0F\x2F\x00\xF2\x48\x0F\x38"
"\xF0\x00\xF2\x48\x0F\x38\xF1\xC3"
"\x66\x0F\x2A\x00\x0F\x2A\x00\x66\x0F\x2D\x00"
"\x0F\x2D\x00\xF2\x0F\x5A\x00\xF3\x0F\x5A\x00"
"\xF3\x48\x0F\x2D\x00\x0F\x2C\xC0\xF3\x48\x0F"
"\x2C\x00\xF3\x48\x0F\x2C\x00""\x66\x99\x99"
"\x48\x99"
"\xFE\xCB\x48\xFF\x09\x40\xF6\xF7\x48\xF7\xF2"
"\xF2\x0F\x5E\x00\xF3\x0F\x5E\x00""\xC8\x10\x00"
"\x07\x66\x0F\x3A\x17\xC2\x01\xD9\xF0\xD9\xE1"
"\xD8\xC1\xDC\x00\xD8\x00"
"\xDC\xC1\xDE\xC2\xD9\xE0\x9B\xDB\xE2\xDA\xC1"
"\xDA\xD1\xDA\xC9\xDB\xC1\xDB\xD1\xDB\xC9\xDB"
"\xD9\xDA\xD9""\xD8\xD2\xD8\x10\xDC\x10\xDB\xF2"
"\xD8\xDA\xD8\x18\xDC\x18\xDF\xF2"
"\xDE\xD9""\xD9\xFF\xD9\xF6\xD8\xF2\xDC\x30\xD8"
"\x30\xDC\xFA\xDE\xFA\xD8\xFA\xDC\x38\xD8\x38"
"\xDC\xF2"
"\xDD\xC1\xDA\x00\xDE\x00\xDA\x10\xDE\x10\xDA"
"\x18\xDE\x18\xDA\x30\xDE\x30\xDA\x38\xDE\x38"
"\xDB\x00\xDF\x00\xDF\x28\xDA\x08\xDE\x08"
"\xD9\xF7\x9B\xDB\xE3\xDB\x10\xDF\x10\xDB\x18"
"\xDF\x18\xDF\x38\xDB\x08\xDF\x08\xDD\x08\xDA"
"\x20\xDE\x20\xDA\x28\xDE\x28"
"\xD9\xC1\xD9\x00\xDD\x00\xD9\xE8\xD9\x28\xD9"
"\x20\xD9\xEA\xD9\xE9\xD9\xEC\xD9\xED\xD9\xEB"
"\xD9\xEE\xD8\xC9\xD8\x08\xDC\x08\xDC\xC9\xDE\xC9"
"\xDB\xE2\xDB\xE3\xD9\xD0\xD9\x38\xD9\x30\xDD\x38"
"\xDF\xE0\xD9\xF3\xD9\xF8\xD9\xF5\xD9\xF2\xD9\xFC"
"\xD9\xFD\xD9\xFE\xD9\xFB\xD9\xFA\xD9\x10\xDD\x10"
"\xDD\xD1""\x9B\xD9\x38\x9B\xD9\x30\xD9\x18\xDD"
"\x18\xDD\xD9\x9B\xDD\x38\x9B\xDF\xE0\xD8\x20"
"\xDC\x20\xDC\xE9\xD8\xE1\xDE\xE9\xD8\x28\xDC"
"\x28\xDC\xE1\xDE\xE1\xD9\xE4""\xDD\xE1\xDB\xE9"
"\xDF\xE9\xDD\xE9\xD9\xE5\xD9\xC9\xF4\xF6\x38"
"\x48\xF7\x38\x48\x6B\xD3\x03\x48\x69\xD3\x00"
"\x01\x00\x00\x40\xF6\xEF\x48\xF7\xEF"
"\xD9\xF4\xD9\xF1\xD9\xF9\xE4\x01\xE5\x01\xEC"
"\xED\x41\xFE\x07\x49\xFF\xC7\x6C\x66\x6D\x6D"
"\x66\x0F\x3A\x21\xC1\x01\x66\x0F\x3A\x21\x00"
"\x01""\xCC\xCD\x7B""\x0F\x08\xCF\xCF\x48\xCF"
"\x72\x00\x76\x00\x67\xE3\x00\x7C\x00\x7E\x00"
"\xEB\x00\xFF\xE0\x73\x00\x77\x00\x7D\x00\x7F"
"\x00\x71\x00\x7B\x00\x79\x00\x75\x00\x70\x00"
"\x7A\x00\x78\x00\x74\x00"
"\x9F\x4F\x8D\x3C\xFE\xC9\x0F\xAE\xE8\xAC\x66"
"\xAD\xAD\x48\xAD\xE2\xFE\xE0\xFE\xE1\xFE\xF2"
"\x41\x0F\x5F\xC7\xF3\x41\x0F\x5F\xC7\x0F\xAE"
"\xF0\xF2\x41\x0F\x5D\xC7\xF3\x41\x0F\x5D\xC7"
"\x8A\x38\x48\x8B\x18\x88\x38\x48\x89\x18\xA2"
"\x66\x55\x44\x33\x22\x11\x00\x00\x48\xA3\x66"
"\x55\x44\x33\x22\x11\x00\x00\xA0\x66\x55\x44"
"\x33\x22\x11\x00\x00\x48\xA1\x66\x55\x44\x33"
"\x22\x11\x00\x00\x40\xB7\x01\x48\xC7\xC7\x01"
"\x00\x00\x00\x48\xC7\xC7\x33\x22\x11\x00\xC6"
"\x00\x01\x48\xC7\x00\x01\x00\x00\x00\x48\xC7"
"\x00\x33\x22\x11\x00\x48\x0F\x38\xF1\x18\x48"
"\x0F\x38\xF0\x18""\x66\x48\x0F\x6E\xC8\x66"
"\x0F\x6E\xC8\x66\x48\x0F\x7E\xC8\x66\x0F\x7E"
"\xC8\xF2\x0F\x12\xC1\xF2\x0F\xD6\xC1\x0F\x12"
"\xC1\x66\x0F\x16\x00\x66\x0F\x17\x00\x0F\x16"
"\x00\x0F\x17\x00\x0F\x16\xC1\x66\x0F\x12\x00"
"\x66\x0F\x13\x00\x0F\x12\x00\x0F\x13\x00\x66"
"\x0F\x50\xC0\x0F\x50\xC0\x48\x0F\xC3\x00\x0F"
"\xE7\x00\xF3\x0F\xD6\xC0\xA4\x66\xA5\xA5\x48"
"\xA5\xF2\x0F\x10\x00\xF2\x0F\x11\x00\xF3\x0F"
"\x16\xC1\xF3\x0F\x12\xC1"
"\xF3\x0F\x11\x00\xF3\x0F\x10\x00\x48\x0F\xBE"
"\xDB\x48\x0F\xBF\xDB\x48\x63\xDB\x48\x0F\xB6"
"\xDB\x48\x0F\xB7\xDB"
"\xF6\xE7\x48\xF7\xE3\xF2\x0F\x59\x00\xF3\x0F"
"\x59\x00\xF6\xDF\x48\xF7\xDB\xF6\xD7\x48\xF7"
"\xD3\x08\x18\x48\x09\x18\x0A\x18\x48\x0B\x18"
"\x0C\x01\x48\x0D\x33\x22\x11\x00\x80\xCB\x01"
"\x48\x81\xCB\x33\x22\x11\x00\x48\x83\xCB\x01"
"\xE6\x01\xE7\x01\xEF\x6E\x66\x6F\x6F\x0F\x6B"
"\x00\x0F\x63\x00\x0F\x67\x00\x0F\xFC\x00\x0F"
"\xFE\x00\x0F\xD4\x00\x0F\xEC\x00\x0F\xED\x00"
"\x0F\xDC\x00\x0F\xDD\x00\x0F\xFD\x00\x0F\xDB"
"\x00\x0F\xDF\x00\xF3\x90\x0F\xE0\x00\x0F\xE3"
"\x00\x0F\x74\x00\x0F\x76\x00\x0F\x75\x00\x0F"
"\x64\x00\x0F\x66\x00\x0F\x65\x00\x66\x0F\x3A"
"\x14\x00\x01\x66\x0F\x3A\x16\x00\x01\x66\x48"
"\x0F\x3A\x16\x00\x01\x66\x0F\x3A\x15\x00\x01"
"\x66\x0F\x3A\x20\xC0\x01\x66\x0F\xC4\xC0\x01"
"\x66\x0F\x3A\x22\xC0\x01\x66\x48\x0F\x3A\x22"
"\xC0\x01\x0F\xF5\x00\x0F\xEE\x00\x0F\xDE\x00"
"\x0F\xEA\x00\x0F\xDA\x00\x0F\xD7\xC0\x66\x0F"
"\xD7\xC0"
"\x0F\xE4\x00\x0F\xE5\x00\x0F\xD5\x00\x0F\xF4"
"\x00\x41\x5F\x66\x8F\x00\x66\x8F\x00\xF3\x48"
"\x0F\xB8\x18\x9D\x9D\x0F\xEB\xC1\x0F\x18\x00"
"\x0F\x18\x08\x0F\x18\x10\x0F\x18\x18\x0F\xF6"
"\xC7\x0F\x70\xC1\x02\x0F\x72\xF0\x02\x66\x41"
"\x0F\x72\xF7\x02\x0F\xF2\x00\x0F\x73\xF0\x02"
"\x66\x41\x0F\x73\xF7\x02\x0F\x71\xF0\x02\x66"
"\x41\x0F\x71\xF7\x02\x0F\xF1\x00\x0F\x72\xE0"
"\x02\x66\x41\x0F\x72\xE7\x02\x0F\xE2\x00\x0F"
"\x71\xE0\x02\x66\x41\x0F\x71\xE7\x02\x0F\xE1"
"\x00\x0F\x72\xD0\x02\x66\x41\x0F\x72\xD7\x02"
"\x0F\xD2\x00\x0F\x73\xD0\x02\x66\x41\x0F\x73"
"\xD7\x02\x0F\x71\xD0\x02\x66\x41\x0F\x71\xD7"
"\x02\x0F\xD1\x00""\x0F\xF8\x00\x0F\xFA\x00"
"\x0F\xFB\x00\x0F\xE8\x00\x0F\xE9\x00\x0F\xD8"
"\x00\x0F\xF9\x00\x0F\x68\x00\x0F\x6A\x00\x0F"
"\x69\x00\x41\x57\x68\xFF\x11\x00\x00\x68\x44"
"\x33\x22\x11\x6A\x01\x9C\x0F\xEF\xC7\x48\xC1"
"\xD3\x03\xC0\xD3\x03\x48\xD1\xD3\xD0\xD3\x48"
"\xD3\xD3\xD2\xD3\x48\xC1\xD3\x03\xC0\xDB\x03"
"\x48\xD1\xDB\xD0\xDB\x48\xD3\xDB\xD2\xDB\xC2"
"\x0F\x00\x48\xC1\xC3\x03\xC0\xC3\x03\x48\xD1"
"\xC3\xD0\xC3\x48\xD3\xC3\xD2\xC3\x48\xC1\xC3"
"\x03\xC0\xCB\x03\x48\xD1\xCB\xD0\xCB\x48\xD3"
"\xCB\xD2\xCB"
"\x66\x0F\x3A\x0B\x00\x01\x66\x0F\x3A\x0A\x00"
"\x01\x9E\xC0\xE3\x03\x48\xC1\xE3\x03\xD0\xE3"
"\x48\xD1\xE3\xD2\xE3\x48\xD3\xE3\xC0\xFB\x03"
"\x48\xC1\xFB\x03\xD0\xFB\x48\xD1\xFB\xD2\xFB"
"\x48\xD3\xFB\x48\x19\x1B\x18\xDB\x48\x19\xDB"
"\x80\xDB\x01\x48\x81\xDB\x22\x11\x00\x00\x48"
"\x83\xDB\x01\x1C\x01\x48\x1D\x22\x11\x00\x00"
"\xAE\xAF\x66\xAF\x48\xAF\x0F\x92\xC0\x0F\x96"
"\xC0\x0F\x9C\xC0\x0F\x9E\xC0\x0F\x93\xC0\x0F"
"\x97\xC0\x0F\x9D\xC0\x0F\x91\xC0\x0F\x9B\xC0"
"\x0F\x99\xC0\x0F\x98\xC0\x0F\x9A\xC0\x0F\x90"
"\xC0\x0F\x95\xC0\x0F\x94\xC0""\x48\x0F\xA4"
"\xD8\x01\x48\x0F\xA5\xD8\x48\x0F\xAC\xD8\x01"
"\x48\x0F\xAD\xD8\xD0\xEB\x48\xD1\xEB\xC0\xEB"
"\x03\x48\xC1\xEB\x03\xD2\xEB\x48\xD3\xEB"
"\xF3\x0F\x51\xC1\xF2\x0F\x51\xC1\xFD\xFB\xF9"
"\xAA\xAB\x66\xAB\x48\xAB\x2A\x18\x2B\x18\x28"
"\x18\x29\x18\x2C\x01\x83\xE8\x01\x80\xEB\x01"
"\x81\xEB\x22\x11\x00\x00\x83\xEB\x01\xF2\x0F"
"\x5C\x00\xF3\x0F\x5C\x00\x45\x84\x07\x4D\x85"
"\x07\xA8\x01\x48\xA9\x22\x11\x00\x00\x48\xA9"
"\x01\x00\x00\x00\x41\xF6\xC0\x01\x49\xF7\xC0"
"\x22\x11\x00\x00\x49\xF7\xC0\x01\x00\x00\x00"
"\x66\x0F\x2E\x00\x0F\x2E\x00\x0F\x0B\x0F\x15"
"\xC1\x41\x0F\x14\xC1\x45\x0F\xC0\xC8\x4D\x0F"
"\xC1\xC8\x45\x86\xC8\x49\x90\xD7\x32\x18\x33"
"\x18\x30\x18\x31\x18\x34\x01\x83\xF0\x01\x80"
"\xF3\x01\x81\xF3\x22\x11\x00\x00\x83\xF3\x01";
cnt=600;
while(cnt--)
  d=DisasOpc(DocPut,d);
#endif
