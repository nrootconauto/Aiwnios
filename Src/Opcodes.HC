//https://github.com/fujitsu/xbyak_aarch64
#define ARM_LSL 0
#define ARM_LSR 1
#define ARM_ASR 2
#define ARM_ROR 3
#define ARM_MSL 4
#define ARM_NONE 5

#define ARM_UXTB 0
#define ARM_UXTH 1
#define ARM_UXTW 2
#define ARM_UXTX 3
#define ARM_SXTB 4
#define ARM_SXTH 5
#define ARM_SXTW 6
#define ARM_SXTX 7
#define ARM_UXT 8
#define ARM_SXT 9
#define ARM_MUL 10
#define ARM_MUL_VL 11
#define ARM_EXT_LSL 12
Bool ARMEnsureRange(I64 val,I64 bits,I64 chop) {
  if(val&(1<<chop-1)) return FALSE;
  val>>=chop;
  if(val>=0) {
    if(val>1<<(bits-1)-1) return FALSE;
  } else {
    if(-val>1<<(bits-1)) return FALSE;
  }
  return TRUE;
} 
I64 FpDataProc1Reg(I64 m,I64 s,I64 type,I64 op,I64 d,I64 src) {
  return (m<<31)|(s<<29)|(0xf<<25)|(type<<22)|(1<<21)|(op<<15)
	|(1<<14)|(src<<5)|d;
}
I64 MASKn(I64 val,I64 bits,I64 off) {
  return (val&(1<<bits-1))<<off;
}
I64 ConversionFpInt(I64 sf,I64 s,I64 type,I64 rmode,I64 opc,I64 d,I64 n) {
  return (sf<<31)|(s<<29)|(0xf<<25)|(type<<22)|(1<<21)|(rmode<<19)|
	(opc<<16)|(n<<5)|d;
}
I64 DataProc3Reg(I64 opc54,I64 opc31,I64 o0,I64 n,I64 m,I64 d,I64 a) {
  return (1<<31)|(opc54<<29)|(0x1b<<24)|(opc31<<21)|(m<<16)|(o0<<15)
	|(a<<10)|(n<<5)|d;
}
I64 DataProc2Src(I64 opc,I64 d,I64 a,I64 b) {
  return (1<<31)|(0xd6<<21)|(b<<16)|(opc<<10)|(a<<5)|d;
}
I64 DataProc1Src(I64 opc,I64 opc2,I64 d,I64 n) {
  return (1<<31)|(1<<30)|(0xd6<<21)|(opc2<<16)|(opc<<10)|(n<<5)|d;
}
I64 CondSel(I64 opc,I64 s,I64 opc2,I64 d,I64 n,I64 m,I64 cond ) {
  return (1<<31)|(opc<<30)|(s<<29)|(0xd4<<21)|(m<<16)|(cond<<12)
	|(opc2<<10)|(n<<5)|d;
}
I64 PCRelAddrEncX(I64 opc,I64 d,I64 adr) {
  if(!ARMEnsureRange(adr,21,2)) return -1;
  return (opc<<31)|((adr&3)<<29)|(0x10<<24)|
	MASKn(adr>>2,19,5)|d;
}
I64 AddSubImmX(I64 op,I64 s,I64 d,I64 n,I64 imm,I64 sh) {
  I64 sh_f=0;
  if(imm&~(1<<12-1)) {
    return -1;
  }
  sh_f=sh==12;  
  return (sh<<31)|(op<<30)|(0x11<<24)|(sh_f<<22)|(imm<<10)|
	n<<4|d;
}
I64 MvWideX(I64 opc,I64 rd,I64 imm,I64 sh) {
  I64 hw=sh&(1<<4);
  if(imm&~0xffff)
     return -1;
  return (1<<31)|(opc<<29)|(0x25<<23)|(hw<<21)|(imm<<5)|
	rd;
}
I64 BitfieldX(I64 opc,I64 d,I64 n,I64 immr,I64 imms) {
  return (1<<31)|(opc<<29)|(0x26<<23)|(1<<22)|(immr<<16)|
	(imms<<10)|(n<<5)|d;
}
I64 CondBrImmEnc(I64 cond,I64 label_off) {
  if(!ARMEnsureRange(label_off,19,2)) return -1;
  return (0x2a<<25)|((label_off>>2)<<5)|cond;
}
I64 UncondBrImmEnc(I64 op,I64 label_off) {
  if(!ARMEnsureRange(label_off,26,2)) return -1;
  return (op<<31)|(5<<26)|(label_off>>2);
}
I64 CompareBrEncX(I64 op,I64 r,I64 label_off) {
  if(!ARMEnsureRange(label_off,19,2)) return -1;
  return (1<<31)|(0x1a<<25)|(op<<24)|MASKn(label_off,19,5)|r;
}
I64 TestBrEnc(I64 op,I64 r,I64 imm,I64 label_off) {
  if(!ARMEnsureRange(label_off,14,2)) return -1;
  if(imm&~0b111111) return -1;
  I64 i5=(imm>>5)&1;
  I64 i40=imm&0b1111;
  return (i5<<31)|(0x1b<<25)|(op<<24)|(i40<<19)|MASKn(label_off,14,5)|r;
}
I64 LdRegLiteralEnc(I64 opc,I64 v,I64 rt,I64 label_off) {
  if(!ARMEnsureRange(label_off,19,2)) return -1;
  return (opc<<30)|(3<<27)|(v<<26)|MASKn(label_off>>2,19,5)|rt;
}

I64 LdFpRegLiteralEnc(I64 rt,I64 label_off) {
  if(!ARMEnsureRange(label_off,19,2)) return -1;
  return (1<<30)|(3<<27)|(0<<26)|MASKn(label_off>>2,19,5)|rt;
}

I64 LdStRegReg(I64 sz,I64 opc,I64 rt,I64 a,I64 b) {
  I64 opt=3;
  I64 s=0;
  return (sz<<30)|(7<<27)|(opc<<22)|(1<<21)|(a<<16)|(opt<<13)|(s<<12)|
	(b<<5)|rt;
}
I64 LdStFpRegReg(I64 opc,I64 rt,I64 a,I64 b) {
  return (1<<31)|(7<<27)|(1<<26)|(opc<<22)|(1<<21)|(a<<16)|(3<<13)|
	(2<<10)|(b<<5)|rt;
}
I64 LdStRegUnImm(I64 sz,I64 opc,I64 rt,I64 a,U64 off) {
  if((off>>sz)&(1<<12-1)) return -1;
  return (sz<<30)|(0x7<<27)|(1<<24)|(opc<<22)|(off>>sz<<10)|(a<<5)|rt;
  
}
I64 LogicalShiftRegX(I64 opc,I64 N,I64 rd,I64 rn,I64 rm,I64 shmod,U64 sh) {
  if(sh&~(1<<6-1)) return -1;
  return (1<<31)|(opc<<29)|(0xa<<24)|(shmod<<22)|(N<<21)|(rm<<16)|(shmod<<10)|(rn<<5)|rd;
}
I64 MvRegX(I64 d,I64 s) {
  if(d==31||s==31)
    return AddSubImmX(0,0,d,s,0,0);
  return LogicalShiftRegX(1,0,d,31,s,ARM_LSL,0);
}
I64 AddSubExtRegX(I64 opc,I64 s,I64 d,I64 n,I64 m,I64 extmod,I64 sh) {
  if(!(0<=sh<=3))
    return -1;
  I64 option=extmod;
  if(extmod==ARM_EXT_LSL)
    option=3;
  return (1<<31)|(opc<<30)|(s<<29)|(0xb<<24)|(1<<21)|(m<<16)|
	(option<<13)|(sh<<10)|(n<<5)|d;
}
I64 AddSubShiftRegX(I64 opc,I64 s,I64 d,I64 n,I64 m,I64 shmod,I64 sh,I64 alias) {
  Bool rd_sp=d==31;
  Bool rn_sp=n==31;
  if(rd_sp+rn_sp>=1+alias)
    return AddSubExtRegX(opc,s,d,n,m,ARM_EXT_LSL,sh);
  I64 sf=1;
  if(sh&~0b111111)
    return -1;
  return (sf<<31)|(opc<<30)|(0xb<<24)|(shmod<<22)|(m<<16)|(sh<<10)|(n<<5)|d;
}

I64 FpComp(I64 m,I64 s,I64 type,I64 op,I64 op2,I64 n,I64 m2) {
  return (m<<31)|(s<<29)|(0xf<<25)|(type<<22)|(1<<21)|(m2<<16)|
	(op<<4)|(1<<13)|(n<<5)|op2;
}
I64 FpDataProc2Reg(I64 m,I64 s,I64 type,I64 opc,I64 vd,I64 vn,I64 vm) {
  return (m<<31)|(s<<29)|(0xf<<25)|(type<<22)|(1<<21)|(vm<<16)|(opc<<12)|
	(2<<10)|(vn<<5)|vd;
}
I64 MvWideImmX(I64 opc,I64 d,I64 imm,I64 sh) {
  if(imm&~0xffff) return -1;
  return (1<<31)|(opc<<29)|(0x25<<23)|(sh>>4<<21)|(imm<<5)|d;
}
I64 LdStRegPairImmX(I64 opc,I64 l,I64 r,I64 r2,I64 adr_n,I64 imm=0) {
  I64 times=opc==2;
  times+=1;
  if(!(-256*times<=imm<=252*times))
    return -1;
  if(imm%(4*times)!=0)
    return -1;
  imm=(imm>>(times+1))&0b1111111;
  return (opc<<30)|(0x5<<27)|(2<<23)|(imm<<15)|(r2<<10)|(adr_n<<5)|r;
}
I64 UncondBrNoReg(I64 opc,I64 opc2,I64 opc3,I64 rn,I64 opc4) {
  return (0x6b<<25)|(opc<<21)|(opc2<<16)|(opc3<<10)|(rn<<5)|(opc4);
}
I64 ARM_Adr(I64 d,I64 off) {return PCRelAddrEncX(0,d,off);} 
I64 ARM_AddImm(I64 d,I64 n,I64 imm,I64 sh) {return AddSubImmX(0,0,d,n,imm,sh);}
I64 ARM_AddsImm(I64 d,I64 n,I64 imm,I64 sh) {return AddSubImmX(0,1,d,n,imm,sh);}
I64 ARM_SubImm(I64 d,I64 n,I64 imm,I64 sh) {return AddSubImmX(0,0,d,n,imm,sh);}
I64 ARM_SubsImm(I64 d,I64 n,I64 imm,I64 sh) {return AddSubImmX(0,1,d,n,imm,sh);}
I64 ARM_CmpImm(I64 d,I64 n,I64 imm,I64 sh) {return AddSubImmX(1,1,31,n,imm,sh);}
I64 ARM_Movn(I64 d,I64 imm,I64 sh) {return MvWideImmX(0,d,imm,sh);}
I64 ARM_Movz(I64 d,I64 imm,I64 sh) {return MvWideImmX(2,d,imm,sh);}
I64 ARM_Movk(I64 d,I64 imm,I64 sh) {return MvWideImmX(3,d,imm,sh);}
I64 ARM_Sxtb(I64 rd,I64 rn) {return BitfieldX(0, rd, rn, 0, 7);}
I64 ARM_Sxth(I64 rd,I64 rn) {return BitfieldX(0, rd, rn, 0, 15);}
I64 ARM_Sxtw(I64 rd,I64 rn) {return BitfieldX(0, rd, rn, 0, 31);}
I64 ARM_AsrImm(I64 rd,I64 rn,I64 amt) {return BitfieldX(0, rd, rn, amt, 63);};
I64 ARM_LslImm(I64 rd,I64 rn,I64 sh) {return BitfieldX(2,rd,rn,(((-1)*sh)%64)&0b111111,63-sh);}
I64 ARM_LsrImm(I64 rd,I64 rn,I64 sh) {return BitfieldX(2,rd,rn,sh,63);}
I64 ARM_Uxtb(I64 rd,I64 rn) {return BitfieldX(2,rd,rn,0,7);}
I64 ARM_Uxth(I64 rd,I64 rn) {return BitfieldX(2,rd,rn,0,15);}
I64 ARM_Bcc(I64 cond,I64 label) {return CondBrImmEnc(cond,label);}
I64 ARM_Ret() {return UncondBrNoReg(2,31,0,30,0);}
I64 ARM_Br(I64 r) {return UncondBrNoReg(0,31,0,r,0);}
I64 ARM_Blr(I64 r) {return UncondBrNoReg(1,31,0,r,0);}
I64 ARM_B(I64 label) {return UncondBrImmEnc(0, label);}
I64 ARM_Cbz(I64 z,I64 label) {return CompareBrEncX(0,z,label);}
I64 ARM_Cbz(I64 nz,I64 label) {return CompareBrEncX(1,nz,label);}
I64 ARM_LdrX(I64 r,I64 label) {return LdRegLiteralEnc(1,0,r,label);}
I64 ARM_Ldr(I64 r,I64 label) {return LdRegLiteralEnc(0,0,r,label);}
I64 ARM_LdrF64(I64 r,I64 label) {return LdFpRegLiteralEnc(r,label);}
I64 ARM_StrRegRegb(I64 r,I64 a,I64 b) {return LdStRegReg(0,0,r,a,b);}
I64 ARM_LdrRegRegb(I64 r,I64 a,I64 b) {return LdStRegReg(0,1,r,a,b);}
I64 ARM_LdrRegRegh(I64 r,I64 a,I64 b) {return LdStRegReg(1,1,r,a,b);}
I64 ARM_StrRegReg(I64 r,I64 a,I64 b) {return LdStRegReg(2,1,r,a,b);}
I64 ARM_LdrRegReg(I64 r,I64 a,I64 b) {return LdStRegReg(2,1,r,a,b);}
I64 ARM_StrRegRegX(I64 r,I64 a,I64 b) {return LdStRegReg(3,1,r,a,b);}
I64 ARM_LdrRegRegX(I64 r,I64 a,I64 b) {return LdStRegReg(3,1,r,a,b);}
I64 ARM_StrRegRegF64(I64 r,I64 a,I64 b) {return LdStFpRegReg(0,r,a,b);}
I64 ARM_LdrRegRegF64(I64 r,I64 a,I64 b) {return LdStFpRegReg(1,r,a,b);}
I64 ARM_StrRegUImmX(I64 r,I64 a,I64 off) {return LdStRegUnImm(3,0,r,a,off);}
I64 ARM_LdrRegUImmX(I64 r,I64 a,I64 off) {return LdStRegUnImm(3,1,r,a,off);}
I64 ARM_StrRegUImm(I64 r,I64 a,I64 off) {return LdStRegUnImm(2,0,r,a,off);}
I64 ARM_LdrRegUImm(I64 r,I64 a,I64 off) {return LdStRegUnImm(2,1,r,a,off);}
I64 ARM_StrRegUImmh(I64 r,I64 a,I64 off) {return LdStRegUnImm(1,0,r,a,off);}
I64 ARM_LdrRegUImmh(I64 r,I64 a,I64 off) {return LdStRegUnImm(1,1,r,a,off);}
I64 ARM_StrRegUImmb(I64 r,I64 a,I64 off) {return LdStRegUnImm(0,0,r,a,off);}
I64 ARM_LdrRegUImmb(I64 r,I64 a,I64 off) {return LdStRegUnImm(0,1,r,a,off);}
I64 ARM_LslvX(I64 r,I64 a,I64 b) {return DataProc2Src(8,r,a,b);}
I64 ARM_LsrvX(I64 r,I64 a,I64 b) {return DataProc2Src(9,r,a,b);}
I64 ARM_AsrvX(I64 r,I64 a,I64 b) {return DataProc2Src(10,r,a,b);}
I64 ARM_AndReg(I64 r,I64 a,I64 b,I64 shmod,I64 sh) {return LogicalShiftRegX(0,0,r,a,b,shmod,sh);}
I64 ARM_OrrReg(I64 r,I64 a,I64 b,I64 shmod,I64 sh) {return LogicalShiftRegX(1,0,r,a,b,shmod,sh);}
I64 ARM_MvnReg(I64 r,I64 a,I64 shmod,I64 sh) {return LogicalShiftRegX(1,1,r,31,a,shmod,sh);}
I64 ARM_EorReg(I64 r,I64 a,I64 b,I64 shmod,I64 sh) {return LogicalShiftRegX(2,0,r,a,b,shmod,sh);}
I64 ARM_AndsReg(I64 r,I64 a,I64 b,I64 shmod,I64 sh) {return LogicalShiftRegX(3,0,r,a,b,shmod,sh);}
I64 ARM_TstReg(I64 r,I64 a,I64 b,I64 shmod,I64 sh) {return LogicalShiftRegX(3,0,31,a,b,shmod,sh);}
I64 ARM_MovReg(I64 r,I64 a) {return MvRegX(r,a);}
I64 ARM_AddReg(I64 r,I64 a,I64 b,I64 shmod,I64 sh) {return AddSubShiftRegX(0,0,r,a,b,shmod,sh,FALSE);}
I64 ARM_AddsReg(I64 r,I64 a,I64 b,I64 shmod,I64 sh) {return AddSubShiftRegX(0,1,r,a,b,shmod,sh,FALSE);}
I64 ARM_SubReg(I64 r,I64 a,I64 b,I64 shmod,I64 sh) {return AddSubShiftRegX(1,0,r,a,b,shmod,sh,FALSE);}
I64 ARM_SubsReg(I64 r,I64 a,I64 b,I64 shmod,I64 sh) {return AddSubShiftRegX(1,1,r,a,b,shmod,sh,FALSE);}
I64 ARM_NegReg(I64 r,I64 a,I64 shmod,I64 sh) {return AddSubShiftRegX(1,0,r,31,a,shmod,sh,TRUE);}
I64 ARM_NegsReg(I64 r,I64 a,I64 shmod,I64 sh) {return AddSubShiftRegX(1,1,r,31,a,shmod,sh,TRUE);}
I64 ARM_CmpReg(I64 a,I64 b,I64 shmod,I64 sh) {return AddSubShiftRegX(1,1,31,a,b,shmod,sh,TRUE);}
I64 ARM_Cset(I64 r,I64 cond) {return CondSel(0,0,1,r,31,31,cond^1);}
I64 ARM_Mul(I64 d,I64 a,I64 b) {return DataProc3Reg(0,0,0,d,a,b,31);}
I64 ARM_Fcvtzs(I64 d,I64 s) {return ConversionFpInt(1,0,1,3,0,d,s);}
I64 ARM_Fcvtzu(I64 d,I64 s) {return ConversionFpInt(1,0,1,3,1,d,s);}
I64 ARM_Scvtf(I64 d,I64 s) {return ConversionFpInt(1,0,1,0,2,d,s);}
I64 ARM_Ucvtf(I64 d,I64 s) {return ConversionFpInt(1,0,1,0,3,d,s);}
I64 ARM_Fmov(I64 d,I64 s) {return FpDataProc1Reg(0,0,1,0,d,s);}
I64 ARM_Fabs(I64 d,I64 s) {return FpDataProc1Reg(0,0,1,1,d,s);}
I64 ARM_Fneg(I64 d,I64 s) {return FpDataProc1Reg(0,0,1,2,d,s);}
I64 ARM_Fsqrt(I64 d,I64 s) {return FpDataProc1Reg(0,0,1,3,d,s);}
I64 ARM_Fcmp(I64 d,I64 s) {return FpComp(0,0,1,0,0,d,s);}
I64 ARM_Fcmpe(I64 d,I64 s) {return FpComp(0,0,1,0,16,d,s);}
I64 ARM_Fmul(I64 d,I64 a,I64 b) {return FpDataProc2Reg(0,0,1,0,d,a,b);}
I64 ARM_Fdiv(I64 d,I64 a,I64 b) {return FpDataProc2Reg(0,0,1,1,d,a,b);}
I64 ARM_Fadd(I64 d,I64 a,I64 b) {return FpDataProc2Reg(0,0,1,2,d,a,b);}
I64 ARM_Fsub(I64 d,I64 a,I64 b) {return FpDataProc2Reg(0,0,1,3,d,a,b);}
I64 ARM_Fmax(I64 d,I64 a,I64 b) {return FpDataProc2Reg(0,0,1,4,d,a,b);}
I64 ARM_Fmin(I64 d,I64 a,I64 b) {return FpDataProc2Reg(0,0,1,5,d,a,b);}
I64 ARM_Stp(I64 a,I64 b,I64 adr_n,I64 off) { return LdStRegPairImmX(2,0,a,b,adr_n,off);}
I64 ARM_Ldp(I64 a,I64 b,I64 adr_n,I64 off) { return LdStRegPairImmX(2,1,a,b,adr_n,off);}
