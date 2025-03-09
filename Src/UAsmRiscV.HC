#define FP_UPPER_BITS "RiscV.FP.upper"
Bool HasUpperBits() {
  if(HashFind(FP_UPPER_BITS,Fs->hash_table,HTT_FRAME_PTR))
    return TRUE;
  return FALSE;
}
I64 MaskN(U64 data,I64 shift,I64 width) {
  return (data>>shift)&(1<<width-1);
}
U8 *RegType2Str(I64 idx,I64 type) {
  switch(type) {
    case 'i':
      switch(idx) {
        case 0:
	  return "zero";
        case 1:
	  return "ra";
        case 2:
	  return "sp";
        case 3:
	  return "gp";
        case 4:
	  return "tp";
        case 5:
	  return "t0";
        case 6:
	  return "t1";
        case 7:
	  return "t2";
        case 8:
	  return "fp";
        case 9:
	  return "s1";
        case 10:
	  return "a0";
        case 11:
	  return "a1";
        case 12:
	  return "a2";
        case 13:
	  return "a3";
        case 14:
	  return "a4";
        case 15:
	  return "a5";
        case 16:
	  return "a6";
        case 17:
	  return "a7";
        case 18:
	  return "s2";
        case 19:
	  return "s3";
        case 20:
	  return "s4";
        case 21:
	  return "s5";
        case 22:
	  return "s6";
        case 23:
	  return "s7";
        case 24:
	  return "s8";
        case 25:
	  return "s9";
        case 26:
	  return "s10";
        case 27:
	  return "s11";
        case 28:
	  return "t3";
        case 29:
	  return "t4";
        case 30:
	  return "t5";
        case 31:
	  return "t6";
      }
    case 'f':
      switch(idx) {
        case 0:
	  return "ft0";
        case 1:
	  return "ft1";
        case 2:
	  return "ft2";
        case 3:
	  return "ft3";
        case 4:
	  return "ft4";
        case 5:
	  return "ft5";
        case 6:
	  return "ft6";
        case 7:
	  return "ft7";
        case 8:
	  return "fs0";
        case 9:
	  return "fs1";
        case 10:
	  return "fa0";
        case 11:
	  return "fa1";
        case 12:
	  return "fa2";
        case 13:
	  return "fa3";
        case 14:
	  return "fa4";
        case 15:
	  return "fa5";
        case 16:
	  return "fa6";
        case 17:
	  return "fa7";
        case 18:
	  return "fs2";
        case 19:
	  return "fs3";
        case 20:
	  return "fs4";
        case 21:
	  return "fs5";
        case 22:
	  return "fs6";
        case 23:
	  return "fs7";
        case 24:
	  return "fs8";
        case 25:
	  return "fs9";
        case 26:
	  return "fs10";
        case 27:
	  return "fs11";
        case 28:
	  return "ft8";
        case 29:
	  return "ft9";
        case 30:
	  return "ft10";
        case 31:
	  return "ft11";
      }
  }
}
Bool DisasRType(U32 data,U8 *name,I64 f7,I64 f3,I64 opcode,I64 opt1='i',I64 opt2='i',I64 opt3='i') {
  if(MaskN(data,25,7)!=f7||
	MaskN(data,12,3)!=f3||
	MaskN(data,0,7)!=opcode)
    return FALSE;
  "%s %s,%s,%s\n",name,
	RegType2Str(MaskN(data,7,5),opt1),
	RegType2Str(MaskN(data,15,5),opt2),
	RegType2Str(MaskN(data,20,5),opt3);
  return TRUE;
}
Bool DisasIType(U32 data,U8 *name,I64 f3,I64 opcode,I64 opt1='i',I64 opt2='i') {
  if(MaskN(data,12,3)!=f3||
	MaskN(data,0,7)!=opcode)
    return FALSE;
  I64 imm=MaskN(data,20,12);
  if(imm&(1<<11)) imm|=~(1<<12-1);
  "%s %s,%s,%X",name,
	RegType2Str(MaskN(data,7,5),opt1),
	RegType2Str(MaskN(data,15,5),opt2),imm;
  if(HasUpperBits&&FramePtr(FP_UPPER_BITS)) {
    " #%P\n",FramePtr(FP_UPPER_BITS)+imm;
  } else 
    "\n";
  FramePtrDel(FP_UPPER_BITS);
  return TRUE;
}
Bool DisasI2Type(U32 data,U8 *name,I64 eimm,I64 f3,I64 opcode,I64 opt1='i',I64 opt2='i') {
  if(MaskN(data,12,3)!=f3||
	MaskN(data,0,7)!=opcode||
	eimm!=MaskN(data,20,12))
    return FALSE;
  "%s %s,%s\n",name,
	RegType2Str(MaskN(data,7,5),opt1),
	RegType2Str(MaskN(data,15,5),opt2);
  FramePtrDel(FP_UPPER_BITS);
  return TRUE;
}

Bool DisasSType(U32 data,U8 *name,I64 f3,I64 opcode,I64 opt1='i',I64 opt2='i') {
  if(MaskN(data,12,3)!=f3||
	MaskN(data,0,7)!=opcode)
    return FALSE;
  I64 imm=MaskN(data,25,12)<<5|
    MaskN(data,7,5);
  if(imm&(1<<11)) imm|=~(1<<12-1);
  "%s %s,%s,%X",name,
	RegType2Str(MaskN(data,20,5),opt1),
	RegType2Str(MaskN(data,15,5),opt2),imm;
  if(HasUpperBits&&FramePtr(FP_UPPER_BITS)) {
    " #%P\n",FramePtr(FP_UPPER_BITS)+imm;
  } else 
    "\n";
  FramePtrDel(FP_UPPER_BITS);
  return TRUE;
}
Bool DisasBType(U32 data,U8 *name,I64 f3,I64 opcode,I64 opt1='i',I64 opt2='i') {
  if(MaskN(data,12,3)!=f3||
	MaskN(data,0,7)!=opcode)
    return FALSE;
  I64 imm=MaskN(data,30,1)<<12|MaskN(data,25,6)<<5|
   MaskN(data,8,4)<<1|MaskN(data,7,1)<<11;
    MaskN(data,7,5);
  if(imm&(1<<12)) imm|=~(1<<13-1);
  "%s %s,%s,%X",name,
	RegType2Str(MaskN(data,15,5),opt1),
	RegType2Str(MaskN(data,20,5),opt2),imm;
  if(FramePtr(FP_UPPER_BITS)) {
    " #%P\n",FramePtr(FP_UPPER_BITS)+imm;
  } else 
    "\n";
  FramePtrDel(FP_UPPER_BITS);
  return TRUE;
}

Bool DisasUType(U32 data,U8 *name,I64 opcode,I64 opt1='i') {
  if(MaskN(data,0,7)!=opcode)
    return FALSE;
  I64 imm=MaskN(data,12,20)<<12;
  if(imm&(1<<31)) imm|=~(1<<32-1);
  FramePtrDel(FP_UPPER_BITS);
  FramePtrAdd(FP_UPPER_BITS,imm);
  "%s %s,%X\n",name,
	RegType2Str(MaskN(data,7,5),opt1),imm;
  return TRUE;
}
Bool DisasJType(U32 data,U8 *name,I64 opcode,I64 opt1='i') {
  if(MaskN(data,0,7)!=opcode)
    return FALSE;
  I64 imm=MaskN(data,31,1)<<20|MaskN(data,21,10)<<1|
	MaskN(data,20,1)<<1|MaskN(data,19-12+1,12)<<12;
  if(imm&(1<<20)) imm|=~(1<<21-1);
  "%s %s,%X",name,RegType2Str(MaskN(data,7,5),opt1),
    imm;
  if(HasUpperBits&&FramePtr(FP_UPPER_BITS)) {
    " #%P\n",FramePtr(FP_UPPER_BITS)+imm;
  } else 
    "\n";
  FramePtrDel(FP_UPPER_BITS);
  return TRUE;
}
Bool DisasFence(U32 data,I64 opcode) {
  I64 i;
  if(MaskN(data,0,7)!=opcode)
    return FALSE;
  if(MaskN(data,28,4)!=0||MaskN(data,15,5)!=0||MaskN(data,12,3)!=0||MaskN(data,7,5)!=0)
    return FALSE;
  I64 pred=MaskN(data,24,4),succ=MaskN(data,20,4);
  if(pred==1&&!succ) {
    "PAUSE\n";
    return TRUE;
  }
  if(!data&&!succ) {
    return FALSE;
  }
  "FENCE ";
  if(pred&1<<3)
    "i";
  if(pred&1<<2)
    "o";
  if(pred&1<<1)
    "r";
  if(pred&1)
    "w";
  ",";
  if(succ&1<<3)
    "i";
  if(succ&1<<2)
    "o";
  if(succ&1<<1)
    "r";
  if(succ&1)
    "w";
  "\n";
  return TRUE;
}
U32 *DisasOpc(CDoc *doc,U32 *data) {
  "%P\t",data;
  U32 d=*data;
  if(DisasUType(d,"LUI", 0b110111)) return data+1; 
  if(DisasUType(d,"AUIPC", 0b10111)) {
//Add pc to upper bits
     FramePtrSet(FP_UPPER_BITS,FramePtr(FP_UPPER_BITS)+ToI64(data));
     return data+1;
  }
  if(DisasJType(d,"JAL", 0b1101111)) return data+1; 
  if(DisasIType(d,"JALR",0b000,0b1100111)) return data+1; 
  if(DisasBType(d,"BNE",0b001,0b1100011)) return data+1; 
  if(DisasBType(d,"BEQ",0b000,0b1100011)) return data+1; 
  if(DisasBType(d,"BLT",0b100,0b1100011)) return data+1; 
  if(DisasBType(d,"BGE",0b101,0b1100011)) return data+1; 
  if(DisasBType(d,"BLTU",0b110,0b1100011)) return data+1; 
  if(DisasBType(d,"BGEU",0b111,0b1100011)) return data+1; 
  if(DisasIType(d,"LB",0,0b11)) return data+1;
  if(DisasIType(d,"LH",0b1,0b11)) return data+1;
  if(DisasIType(d,"LW",0b10,0b11)) return data+1;
  if(DisasIType(d,"LBU",0b100,0b11)) return data+1;
  if(DisasIType(d,"LHU",0b101,0b11)) return data+1;
  if(DisasIType(d,"LWU",0b110,0b11)) return data+1;  
  if(DisasIType(d,"SB",0,0b100011)) return data+1;
  if(DisasIType(d,"SH",0b1,0b100011)) return data+1;
  if(DisasIType(d,"SW",0b10,0b100011)) return data+1;
  if(DisasIType(d,"LBU",0b100,0b100011)) return data+1;
  if(DisasIType(d,"LHU",0b101,0b100011)) return data+1;
  if(DisasIType(d,"LWU",0b110,0b100011)) return data+1;
  if(DisasIType(d,"ADDI",0b000,0b10011)) return data+1;
  if(DisasIType(d,"SLTI",0b010,0b10011)) return data+1;
  if(DisasIType(d,"SLTIU",0b011,0b10011)) return data+1;
  if(DisasIType(d,"XORI",0b0100,0b10011)) return data+1;
  if(DisasIType(d,"ORI",0b0110,0b10011)) return data+1;
  if(DisasIType(d,"ANDI",0b0111,0b10011)) return data+1;
  if(DisasRType(d,"ADD",0b0,0b000,0b110011)) return data+1;
  if(DisasRType(d,"SUB",0b0100000,0b000,0b110011)) return data+1;
  if(DisasRType(d,"SLL",0b0000000,0b001,0b110011)) return data+1;
  if(DisasRType(d,"SLT",0b0000000,0b010,0b110011)) return data+1;
  if(DisasRType(d,"SLTU",0b0000000,0b011,0b110011)) return data+1;
  if(DisasRType(d,"XOR",0b0000000,0b100,0b110011)) return data+1;
  if(DisasRType(d,"SRL",0b0000000,0b101,0b110011)) return data+1;
  if(DisasRType(d,"SRA",0b0100000,0b101,0b110011)) return data+1;
  if(DisasRType(d,"OR",0b0000000,0b110,0b110011)) return data+1;
  if(DisasRType(d,"AND",0b0000000,0b111,0b110011)) return data+1;
  if(DisasIType(d,"LD",0b11,0b11)) return data+1;
  if(DisasSType(d,"SD",0b11,0b100011)) return data+1;
  if(DisasIType(d,"SLLI",0b1,0b10011)) return data+1;
  if(d&(1<<30)){
    if(DisasIType(d,"SRAI",0b101,0b10011)) return data+1;
  } else if(DisasIType(d,"SRLI",0b101,0b10011)) return data+1;
  if(DisasRType(d,"MUL",0b1,0b000,0b110011)) return data+1;
  if(DisasRType(d,"MULH",0b1,0b001,0b110011)) return data+1;
  if(DisasRType(d,"MULHSU",0b1,0b010,0b110011)) return data+1;
  if(DisasRType(d,"MULHU",0b1,0b011,0b110011)) return data+1;
  if(DisasRType(d,"DIV",0b1,0b100,0b110011)) return data+1;
  if(DisasRType(d,"DIVU",0b1,0b101,0b110011)) return data+1;
  if(DisasRType(d,"REM",0b1,0b110,0b110011)) return data+1;
  if(DisasRType(d,"REMU",0b1,0b111,0b110011)) return data+1;
  if(DisasIType(d,"FLD",0b11,0b111)) return data+1;
  if(DisasIType(d,"FSD",0b11,0b100111)) return data+1;
#define ROUND_MODE 1
  if(DisasRType(d,"FADD",0b1,ROUND_MODE,0b1010011)) return data+1;
  if(DisasRType(d,"FSUB",0b101,ROUND_MODE,0b1010011)) return data+1;
  if(DisasRType(d,"FMUL",0b1001,ROUND_MODE,0b1010011)) return data+1;
  if(DisasRType(d,"FDIV",0b1101,ROUND_MODE,0b1010011)) return data+1;
  if(DisasRType(d,"FSQRT",0b101101,ROUND_MODE,0b1010011)) return data+1;
  if(DisasRType(d,"FMIN",0b10101,0,0b1010011)) return data+1;
  if(DisasRType(d,"FMAX",0b10101,1,0b1010011)) return data+1;
  if(DisasRType(d,"FEQ",0b1010001,0b10,0b1010011)) return data+1;
  if(DisasRType(d,"FLT",0b1010001,0b1,0b1010011)) return data+1;
  if(DisasRType(d,"FLE",0b1010001,0b0,0b1010011)) return data+1;
  if(DisasRType(d,"SGNJ",0b10001,0b0,0b1010011)) return data+1;
  if(DisasRType(d,"SGNJN",0b10001,0b1,0b1010011)) return data+1;
  if(DisasRType(d,"SGNJX",0b10001,0b10,0b1010011)) return data+1;
  if(DisasI2Type(d,"FMV.D.X",0b111100100000,0,0b1010011)) return data+1;
  if(DisasI2Type(d,"FMV.X.D",0b111000100000,0,0b1010011)) return data+1;
  if(DisasI2Type(d,"FCVT.L.D",0b110000100010,ROUND_MODE,0b1010011)) return data+1;
  if(DisasI2Type(d,"FCVT.D.L",0b110100100010,ROUND_MODE,0b1010011)) return data+1;
  if(DisasFence(d,0b0001111)) return data+1;
  "BAD\n";
  return data+1;
}
