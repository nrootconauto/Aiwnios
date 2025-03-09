//
// Binds the silly sauce from parser.c to HolyC. AIWNIOS is built so when
//  you write a backend,HolyC will use it too,that way you dont have to re-port
//  your backend again when you port to another arch
//
#ifdef IMPORT_AIWNIOS_SYMS
import U0 __HC_SetAOTRelocBeforeRIP(U8 *,I64); //See aiwn.h(TAKES number of bytes before current RIP the symbol is located at)
import U8 *__HC_CmpCtrlDel(U8 *);
import U8 *__HC_ICAdd_Sqr(U8 *);
import U8 *__HC_ICAdd_RawBytes(U8*,U8*,I64);
import U8 *__HC_ICAdd_GetVargsPtr(U8*);
import U8 *__HC_ICAdd_ToF64(U8 *);
import U8 *__HC_ICAdd_ToI64(U8 *);
import U0 __HC_ICSetLine(U8*,I64);
import U8 *__HC_ICAdd_Sqrt(U8 *);
import U8 *__HC_ICAdd_Max_I64(U8 *);
import U8 *__HC_ICAdd_Min_I64(U8 *);
import U8 *__HC_ICAdd_Max_U64(U8 *);
import U8 *__HC_ICAdd_Min_U64(U8 *);
import U8 *__HC_ICAdd_Max_F64(U8 *);
import U8 *__HC_ICAdd_Min_F64(U8 *);
import U8 *__HC_ICAdd_Typecast(U8 *,I64,I64);
import U8 *__HC_ICAdd_SubCall(U8 *,U8 *);
import U8 *__HC_ICAdd_SubProlog(U8 *);
import U8 *__HC_ICAdd_SubRet(U8 *);
import U8 *__HC_ICAdd_BoundedSwitch(U8*,U8*,U8*);
import U8 *__HC_ICAdd_UnboundedSwitch(U8 *,U8 *);
import U8 *__HC_ICAdd_PreInc(U8 *,I64);
import U8 *__HC_ICAdd_Call(U8*,I64,I64 rt,I64 ptrs);
import U8 *__HC_ICAdd_F64(U8*,F64);
import U8 *__HC_ICAdd_I64(U8*,I64);
import U8 * __HC_ICAdd_PreDec(U8*,I64);
import U8 * __HC_ICAdd_PostDec(U8*,I64);
import U8 * __HC_ICAdd_PostInc(U8*,I64);
import U8 * __HC_ICAdd_Pow(U8 *);
import U8 * __HC_ICAdd_Lock(U8 *);
import U8 * __HC_ICAdd_Eq(U8*);
import U8 * __HC_ICAdd_Div(U8*);
import U8 * __HC_ICAdd_Sub(U8*);
import U8 * __HC_ICAdd_Mul(U8*);
import U8 * __HC_ICAdd_Add(U8*);
import U8 * __HC_ICAdd_Deref(U8*,I64 rt,I64 ptrs);
import U8 * __HC_ICAdd_Comma(U8*);
import U0 __HC_ICSetLock(U8*);
import U8 * __HC_ICAdd_ToBool(U8*);
import U8 * __HC_ICAdd_Addr(U8*);
import U8 * __HC_ICAdd_Xor(U8*);
import U8 * __HC_ICAdd_Mod(U8*);
import U8 * __HC_ICAdd_Or(U8*);
import U8 * __HC_ICAdd_Lt(U8*);
import U8 * __HC_ICAdd_Gt(U8*);
import U8 * __HC_ICAdd_Le(U8*);
import U8 * __HC_ICAdd_Ge(U8*);
import U8 * __HC_ICAdd_LNot(U8*);
import U8 * __HC_ICAdd_BNot(U8*);
import U8 * __HC_ICAdd_AndAnd(U8*);
import U8 * __HC_ICAdd_And(U8*);
import U8 * __HC_ICAdd_OrOr(U8*);
import U8 * __HC_ICAdd_XorXor(U8*);
import U8 * __HC_ICAdd_Ne(U8*);
import U8 * __HC_ICAdd_EqEq(U8*);
import U8 * __HC_ICAdd_Lsh(U8*);
import U8 * __HC_ICAdd_Rsh(U8*);
import U8 * __HC_ICAdd_AddEq(U8*);
import U8 * __HC_ICAdd_SubEq(U8*);
import U8 * __HC_ICAdd_MulEq(U8*);
import U8 * __HC_ICAdd_DivEq(U8*);
import U8 * __HC_ICAdd_LshEq(U8*);
import U8 * __HC_ICAdd_RshEq(U8*);
import U8 * __HC_ICAdd_AndEq(U8*);
import U8 * __HC_ICAdd_OrEq(U8*);
import U8 * __HC_ICAdd_XorEq(U8*);
import U8 * __HC_ICAdd_ModEq(U8*);
import U8 * __HC_ICAdd_FReg(U8*,I64);
import U8 * __HC_ICAdd_Vargs(U8 *,I64);
import U8 * __HC_ICAdd_IReg(U8*,I64 r,I64 rt,I64 ptrs);
import U8 * __HC_ICAdd_Frame(U8*,I64 off,I64 rt,I64 ptrs);
import U8 *__HC_CodeMiscJmpTableNew(U8 *,U8 **cmts,I64 lo,I64 hi);
import U8 *__HC_CodeMiscStrNew(U8*,U8*,I64 sz);
import U8 *__HC_CodeMiscLabelNew(U8*,U8**patch_addr=NULL);
import U8 *__HC_CmpCtrlNew();
import U8 *__HC_CodeCtrlPush(U8*);
import U0 __HC_CodeCtrlPop(U8*);
import U8 *__HC_Compile(U8*,I64 *sz=NULL,U8 **dbg_info=NULL,CHeapCtrl *);
import U8 *__HC_CodeMiscStrNew(U8 *,U8*,I64);
import U8 *__HC_CodeMiscJmpTableNew(U8*,U8**,I64,I64);
import U8 *__HC_ICAdd_Label(U8*,U8*);
import U8 *__HC_ICAdd_Goto(U8*,U8*);
import U8 *__HC_ICAdd_GotoIf(U8*,U8*)
import U8 *__HC_ICAdd_Str(U8*,U8*); //Takes a AIWNIOS code misc
import U8 *__HC_ICAdd_Neg(U8*);
import U8 *__HC_ICAdd_Ret(U8*);
import U8 *__HC_ICAdd_Arg(U8*,I64);
import U8 *__HC_ICAdd_SetFrameSize(U8*,I64);
import U8 *__HC_ICAdd_Reloc(U8 *ccmp,U8 *cc,U64*,U8 *sym,I64 rt,I64 ptrsctrl);
import U8 *__HC_ICAdd_RelocUnique(U8 *ccmp,U8 *cc,U64*,U8 *sym,I64 rt,I64 ptrsctrl);
import U8 *__HC_ICAdd_StaticData(U8 *ccmp,U8 *cc,I64 at,U8 *d,I64 lctrl);
import U8 *__HC_ICAdd_SetStaticsSize(U8 *,I64 szctrl);
import U8 *__HC_ICAdd_StaticRef(U8 *,I64 off,I64 rt,I64 ptrsctrl);
import U8 *__HC_ICAdd_ShortAddr(U8 *cc,U8*cctrl,U8*,U8**);
import U0 __HC_CmpCtrlSetRip(U8*,I64);
import U8 * __HC_ICAdd_BT(U8*);
import U8 * __HC_ICAdd_BTC(U8*);
import U8 * __HC_ICAdd_BTR(U8*);
import U8 * __HC_ICAdd_BTS(U8*);
import U8 * __HC_ICAdd_LBTC(U8*);
import U8 * __HC_ICAdd_LBTR(U8*);
import U8 * __HC_ICAdd_LBTS(U8*);
import U0 __HC_CodeMiscInterateThroughRefs(U8 *,U0(*fptr)(U8*addr,U8*ud),U8*user_data);
import U8 * __HC_ICAdd_Fs(U8*);
import U8 * __HC_ICAdd_Gs(U8*);
#else
extern  U8 *__HC_ICAdd_Max_I64(U8 *);
extern U8 *__HC_ICAdd_Min_I64(U8 *);
extern U8 *__HC_ICAdd_Max_U64(U8 *);
extern U8 *__HC_ICAdd_Min_U64(U8 *);
extern U8 *__HC_ICAdd_Max_F64(U8 *);
extern U8 *__HC_ICAdd_Min_F64(U8 *);
extern U8 *__HC_ICAdd_GetVargsPtr(U8*);
extern U8 *__HC_ICAdd_RawBytes(U8*,U8*,I64);
extern U0 __HC_CodeMiscInterateThroughRefs(U8 *,U0(*fptr)(U8*addr,U8*ud),U8*user_data);
extern U8 * __HC_ICAdd_ToF64(U8 *);
extern U8 * __HC_ICAdd_ToI64(U8 *);
extern U0 __HC_ICSetLine(U8*,I64);
extern U8 * __HC_ICAdd_Typecast(U8 *,I64,I64);
extern U8 * __HC_ICAdd_SubCall(U8 *,U8 *);
extern U8 * __HC_ICAdd_SubProlog(U8 *);
extern U8 * __HC_ICAdd_SubRet(U8 *);
extern U8 * __HC_ICAdd_BoundedSwitch(U8*,U8*,U8*);
extern U8 * __HC_ICAdd_UnboundedSwitch(U8 *,U8 *);
extern U8 * __HC_ICAdd_PreInc(U8 *,I64);
extern U8 * __HC_ICAdd_Call(U8*,I64,I64 rt,I64 ptrs);
extern U8 * __HC_ICAdd_F64(U8*,F64);
extern U8 * __HC_ICAdd_I64(U8*,I64);
extern U8 * __HC_ICAdd_PreDec(U8*,I64);
extern U8 * __HC_ICAdd_PostDec(U8*,I64);
extern U8 * __HC_ICAdd_PostInc(U8*,I64);
extern U8 * __HC_ICAdd_Pow(U8 *);
extern U8 * __HC_ICAdd_Eq(U8*);
extern U8 * __HC_ICAdd_Div(U8*);
extern U8 * __HC_ICAdd_Sub(U8*);
extern U8 * __HC_ICAdd_Mul(U8*);
extern U8 * __HC_ICAdd_Add(U8*);
extern U8 * __HC_ICAdd_Deref(U8*,I64 rt,I64 ptrs);
extern U8 * __HC_ICAdd_Comma(U8*);
extern U8 * __HC_ICAdd_Addr(U8*);
extern U8 * __HC_ICAdd_Xor(U8*);
extern U8 * __HC_ICAdd_Mod(U8*);
extern U8 * __HC_ICAdd_Or(U8*);
extern U8 * __HC_ICAdd_Lt(U8*);
extern U8 * __HC_ICAdd_Gt(U8*);
extern U8 * __HC_ICAdd_Le(U8*);
extern U8 * __HC_ICAdd_Ge(U8*);
extern U8 * __HC_ICAdd_LNot(U8*);
extern U8 * __HC_ICAdd_BNot(U8*);
extern U8 * __HC_ICAdd_AndAnd(U8*);
extern U8 * __HC_ICAdd_And(U8*);
extern U8 * __HC_ICAdd_OrOr(U8*);
extern U8 * __HC_ICAdd_XorXor(U8*);
extern U8 * __HC_ICAdd_Ne(U8*);
extern U8 * __HC_ICAdd_EqEq(U8*);
extern U8 * __HC_ICAdd_Lsh(U8*);
extern U8 * __HC_ICAdd_Rsh(U8*);
extern U8 * __HC_ICAdd_AddEq(U8*);
extern U8 * __HC_ICAdd_SubEq(U8*);
extern U8 * __HC_ICAdd_MulEq(U8*);
extern U8 * __HC_ICAdd_DivEq(U8*);
extern U8 * __HC_ICAdd_LshEq(U8*);
extern U8 * __HC_ICAdd_RshEq(U8*);
extern U8 * __HC_ICAdd_AndEq(U8*);
extern U8 * __HC_ICAdd_OrEq(U8*);
extern U8 * __HC_ICAdd_XorEq(U8*);
extern U8 * __HC_ICAdd_ModEq(U8*);
extern U8 * __HC_ICAdd_FReg(U8*,I64);
extern U8 * __HC_ICAdd_Ds(U8 *,I64);
extern U8 * __HC_ICAdd_IReg(U8*,I64 r,I64 rt,I64 ptrs);
extern U8 * __HC_ICAdd_Frame(U8*,I64 off,I64 rt,I64 ptrs);
extern U8 *__HC_CodeMiscJmpTableNew(U8 *,U8 **cmts,I64 lo,I64 hi);
extern U8 *__HC_CodeMiscStrNew(U8*,U8*,I64 sz);
extern U8 *__HC_CodeMiscLabelNew(U8*,U8**);
extern U8 *__HC_CmpCtrlNew();
extern U8 *__HC_CodeCtrlPush(U8*);
extern U0 __HC_CodeCtrlPop(U8*);
extern U8 *__HC_Compile(U8*,I64 *sz=NULL,U8 **dbg_info=NULL,CHeapCtrl *c);
extern U8 *__HC_CodeMiscLabelNew(U8 *,U8 **patch_addr=NULL);
extern U8 *__HC_CodeMiscStrNew(U8 *,U8*,I64);
extern U8 *__HC_CodeMiscJmpTableNew(U8*,U8**,I64,I64);
extern U8 *__HC_ICAdd_Label(U8*,U8*);
extern U8 *__HC_ICAdd_Goto(U8*,U8*);
extern U8 *__HC_ICAdd_GotoIf(U8*,U8*)
extern U8 *__HC_ICAdd_Str(U8*,U8*); //Takes a AIWNIOS code misc
extern U8 *__HC_ICAdd_Neg(U8*);
extern U8 *__HC_ICAdd_Ret(U8*);
extern U8 *__HC_ICAdd_Arg(U8*,I64);
extern U8 *__HC_ICAdd_SetFrameSize(U8*,I64);
extern U8 *__HC_ICAdd_Reloc(U8 *ccmp,U8 *cc,U64*,U8 *sym,I64 rt,I64 ptrsctrl);
extern U8 *__HC_ICAdd_StaticData(U8 *ccmp,U8 *cc,I64 at,U8 *d,I64 lctrl);
extern U8 *__HC_ICAdd_SetStaticsSize(U8 *,I64 szctrl);
extern U8 *__HC_ICAdd_StaticRef(U8 *,I64 off,I64 rt,I64 ptrsctrl);
extern U8 *__HC_ICAdd_ShortAddr(U8 *cc,U8*cctrl,U8*,U8**);
extern U0 __HC_CmpCtrlSetRip(U8*,I64);
extern U8 * __HC_ICAdd_BT(U8*);
extern U8 * __HC_ICAdd_BTC(U8*);
extern U8 * __HC_ICAdd_BTR(U8*);
extern U8 * __HC_ICAdd_BTS(U8*);
extern U8 * __HC_ICAdd_LBTC(U8*);
extern U8 * __HC_ICAdd_LBTR(U8*);
extern U8 * __HC_ICAdd_LBTS(U8*);
extern U8 * __HC_ICAdd_Fs(U8*);
extern U8 * __HC_ICAdd_Gs(U8*)
#endif
//
// See arm_backend.c
//
#ifdef TARGET_AARCH64
#define AIWNIOS_IREG_START 19
#define AIWNIOS_IREG_CNT (27 - 19 + 1)
#define AIWNIOS_FREG_START 8
#define AIWNIOS_FREG_CNT (15 - 8 + 1)
I64 VarIRegToReg(I64 r) {return r+AIWNIOS_IREG_START;}
I64 VarFRegToReg(I64 r) {return r+AIWNIOS_FREG_START;}
#endif

#ifdef TARGET_RISCV
#define AIWNIOS_IREG_START 0
#define AIWNIOS_FREG_START 0
#define AIWNIOS_IREG_CNT (27-18+1+1)
#define AIWNIOS_FREG_CNT (27-18+1+2)
I64 VarFRegToReg(I64 r)  {
  switch(r) {
     case 0:
       return 9;
     case 1:
       return 8;
     case 2:
       return 18;
     case 3:
       return 19;
     case 4:
       return 20;
     case 5:
       return 21;
     case 6:
       return 22;
     case 7:
       return 23;
     case 8:
       return 24;
     case 9:
       return 25;
     case 10:
       return 26;
     case 11:
       return 27;
   }
   throw('Compiler');
}
I64 VarIRegToReg(I64 r)  {
  switch(r) {
     case 0:
       return 9;
     case 1:
       return 18;
     case 2:
       return 19;
     case 3:
       return 20;
     case 4:
       return 21;
     case 5:
       return 22;
     case 6:
       return 23;
     case 7:
       return 24;
     case 8:
       return 25;
     case 9:
       return 26;
     case 10:
       return 27;
   }
   throw('Compiler');
}

#endif


#ifdef TARGET_X86
#define	REG_RAX 0
#define	REG_RCX 1
#define	REG_RDX 2
#define	REG_RBX 3
#define	REG_RSP 4
#define	REG_RBP 5
#define	REG_RSI 6
#define	REG_RDI 7
#define	REG_R8 8
#define	REG_R9 9
#define	REG_R10 10
#define	REG_R11 11
#define	REG_R12 12
#define	REG_R13 13
#define	REG_R14 14
#define	REG_R15 15
#define	REG_RIP 16
#define AIWNIOS_IREG_START 9
#define AIWNIOS_IREG_CNT 7
#define AIWNIOS_TMP_IREG_CNT 3
#define AIWNIOS_REG_FP REG_RBP
#define AIWNIOS_REG_SP REG_RSP
#define AIWNIOS_TMP_IREG_START 0
#define AIWNIOS_FREG_START 6
#define AIWNIOS_FREG_CNT (15-6+1)
#define AIWNIOS_TMP_FREG_START 2
#define AIWNIOS_TMP_FREG_CNT 14
I64 VarFRegToReg(I64 r)  {
  return AIWNIOS_FREG_START+r;
}

#endif
//
// Replaces IC_CALL_INDIRECT(IC_ADDR_IMPORT) with Short jumps if not import
//
U0 AiwniosMakeShortJmps(CCmpCtrl *cc,U8 *cc2) {
  CRPN *head=&cc->coc.coc_head,*cur,*new;
  CCodeMisc *misc;
  for(cur=head->next;cur!=head;cur=cur->next) {
    if(cur->type==IC_ADDR_IMPORT) {
      misc=cur->ic_data;
      if(!(misc->h->type&HTF_IMPORT)&&(misc->flags&CMF_SHORT_ADDR)) {
force:
        misc->addr=__HC_CodeMiscLabelNew(cc2);
        cur->ic_flags|=ICF_SHORT_JMP;
      } else {
      //No use imports as short addresses
        misc->flags&=~CMF_SHORT_ADDR;
/* Nroot removed this as not all platforms have easy access to lower 32bits
#ifdef TARGET_X86
//In X86_64,i will put all functions in low 32bits to avoid viener wfunctions.
goto force;
#endif
*/
      }
    }
next:;
  }
}




I64 IRegIsAvail(CHashFun *fun,I64 r) {
  CMemberLst *mlst=fun->member_lst_and_root;
  while(mlst) {
    if(mlst->reg==r&&mlst->member_class->raw_type!=RT_F64)
      return FALSE;
    mlst=mlst->next;
  }
  return TRUE;
}

I64 FRegIsAvail(CHashFun *fun,I64 r) {
  CMemberLst *mlst=fun->member_lst_and_root;
  while(mlst) {
    if(mlst->reg==r&&mlst->member_class->raw_type==RT_F64)
      return FALSE;
    mlst=mlst->next;
  }
  return TRUE;
}
class CBitSet {
  union {
    U8 *body;
    U64 *body_u64;
  }
  I64 cnt,body_sz;
};

CBitSet BitSetNew() {
  CBitSet *bs=CAlloc(sizeof CBitSet);
  bs->body=CAlloc(8);
  bs->body_sz=8;
  return bs;
}
U0 BitSetDel(CBitSet *bs) {
  Free(bs->body);
  Free(bs);
} 
U0 BitSetAdd(CBitSet *bs,I64 b) {
  I64 *new;
  if(0x10000<b) throw('Compiler');
  if(bs->body_sz<=b>>3) {
    new=CAlloc(8+b>>3);
    MemCpy(new,bs->body,bs->body_sz);
    Free(bs->body);
    bs->body=new;
    bs->body_sz=8+b>>3;
  }
  Bts(bs->body,b);
}
U0 BitSetRem(CBitSet *bs,I64 b) {
  I64 *new;
  if(0x10000<b) throw('Compiler');
  if(bs->body_sz<=b>>3)
    return;
  Btr(bs->body,b);
}
I64 BitSetCnt(CBitSet *bs) {
  I64 c=0,i=bs->body_sz;
  while(--i>=0)
    c+=set_bits_table[bs->body[i]];
  return c;
}
U0 BitSetRemAll(CBitSet *bs,CBitSet *bs2) {
//Sizes are always aligned to 8
  I64 min=MinI64(bs->body_sz,bs2->body_sz)>>3;
  while(--min>=0) {
    bs->body_u64[min]&=~bs2->body_u64[min];
  }
}
U0 BitSetAddAll(CBitSet *bs,CBitSet *bs2) {
  I64 *new,idx;
  if(bs->body_sz<bs2->body_sz) {
    new=CAlloc(bs2->body_sz);
    MemCpy(new,bs->body,bs->body_sz);
    Free(bs->body);
    bs->body=new;
    bs->body_sz=bs2->body_sz;
  }
//Sizes are always aligned to 8
  idx=bs2->body_sz>>3;
  while(--idx>=0) {
   bs->body_u64[idx]|=bs2->body_u64[idx];
  }
}
I64 BitSetWidth(CBitSet *bs) {
  return bs->body_sz*8;
}
Bool BitSetHas(CBitSet *bs,I64 b) {
  if(bs->body_sz<=b>>3)
    return FALSE;
  if(bs->body[b>>3]&(1<<(b&7))) {
    return TRUE;
  }
  return FALSE;
}
class CI64Set {
  I64 cnt,cap;
  I64 *body;
};
U0 I64SetDel(CI64Set *s) {
  Free(s->body);
  Free(s);
}
CI64Set *I64SetNew() {
  CI64Set *s=CAlloc(sizeof(CI64Set));
  s->cap=16;
  s->body=CAlloc(16*8);
  return s;
}
Bool I64SetHas(CI64Set *s,I64 v) {
  I64 i=s->cnt;
  I64 *body=s->body;
  while(i>0)
    if(body[--i]==v)
      return TRUE;
  return FALSE;
}
U0 I64SetAdd(CI64Set *s,I64 v,Bool allow_repeats=FALSE) {
  if(!allow_repeats)
    if(I64SetHas(s,v))
      return;
  I64 *new;
  if(s->cnt+1>=s->cap) {
     new=CAlloc(MSize(s->body)+16*8);
     s->cap+=16;
     MemCpy(new,s->body,s->cnt*8);
     Free(s->body);
     s->body=new;
  }
  s->body[s->cnt++]=v;
}
U0 I64SetAddAll(CI64Set *s,CI64Set *from) {
  I64 idx=from->cnt;
  while(--idx>=0)
   I64SetAdd(s,from->body[idx]);
}
U0 I64SetRem(CI64Set *s,I64 v) {
  I64 idx,ptr=0,del=0;
  for(idx=0;idx!=s->cnt;idx++) {
    if(s->body[idx]!=v)
      s->body[ptr++]=s->body[idx];
    else
      del++;
  }
  s->cnt-=del;
}
U0 I64SetRemAll(CI64Set *s,CI64Set *from) {
  I64 idx=from->cnt;
  while(--idx>=0)
   I64SetRem(s,from->body[idx]);
}
#ifdef TARGET_X86
I64 VarIRegToReg(I64 r)  {
  switch(r) {
      break;case 0: return REG_R10;
      break;case 1: return REG_R11;
      break;case 2: return REG_R12;
      break;case 3: return REG_R14;
      break;case 4: return REG_R15;
      break;case 5: return REG_RDI;
//AIWNIOS_IREG_CNT ends at 6,NOT 7,$BK,1$SEE BELOW NOTE$BK,0$
      break;case 6: return REG_RSI;
//DO NOT USE THIS WITH ANYTHING RELATED TO IC_DEREF
      break;case 7: return REG_R13;
  }
  throw('Compile');
}
#endif
class CAddShortRelocData {
	CCmpCtrl *cc;
	CCodeMisc *cm;
	U8 *machine_code;
	Bool is_fun_call;
};
Bool Globalify(U8 *addr,CAddShortRelocData *data) {
  CCodeMisc *cm=data->cm;
  CCmpCtrl *cc=data->cc;
  if(!(cm->h->type&HTT_GLBL_VAR))
    return FALSE;
  CAOTHeapGlbl *glbl_gh=cc->aotc->heap_glbls;
  CAOTHeapGlblRef *tmpghr;
  while(glbl_gh&&StrCmp(glbl_gh->str,cm->h->str)) {
    glbl_gh=glbl_gh->next;
  }
  if(glbl_gh) {
    tmpghr=CAlloc(sizeof CAOTHeapGlblRef);
    tmpghr->next=glbl_gh->references;
    glbl_gh->references=tmpghr;
    tmpghr->offset=cm->offset;
    tmpghr->rip=cc->aotc->rip+(addr-data->machine_code);
    return TRUE;
  }
  return FALSE;
}
U0 AddShortReloc(U8 *addr,CAddShortRelocData *data) {
  CAOTImportExport *tmpaie=CAlloc(sizeof(CAOTImportExport));
  #ifdef TARGET_RISCV
  tmpaie->type=IET_REL_RISCV;
  #else
  tmpaie->type=IET_REL_I32;
  #endif
  tmpaie->str=StrNew(data->cm->h->str);
  tmpaie->rip=data->cc->aotc->rip+(addr-data->machine_code);
  
  tmpaie->offset=data->cm->offset;
  tmpaie->short_jmp=TRUE;
  QueIns(tmpaie,data->cc->aot->last_ie);
}
static Bool IsTLSReloc(CCodeMisc *misc) {
  if(misc->h&&(!StrCmp(misc->h->str,"__Fs")||!StrCmp(misc->h->str,"__Gs")))
	return TRUE;
  return FALSE;
}
static Bool IsFunArg(CHashFun *fun,CMemberLst *lst) {
  if(!fun) return FALSE;
  CMemberLst *mlst=fun->member_lst_and_root;
  I64 idx,off=0;
  if(fun->flags&(1<<Ff_DOT_DOT_DOT))
    off+=2;
  for(idx=0;idx!=fun->arg_cnt+off;idx++) {
    if(lst==mlst) return TRUE;
    mlst=mlst->next;
  }
  return FALSE;
}
static I64 FunArgIdx(CHashFun *fun,CMemberLst *lst) {
  if(!fun) return FALSE;
  CMemberLst *mlst=fun->member_lst_and_root;
  I64 idx,off=0;
  if(fun->flags&(1<<Ff_DOT_DOT_DOT))
    off+=2;
  for(idx=0;idx!=fun->arg_cnt+off;idx++) {
    if(lst==mlst) return idx;
    mlst=mlst->next;
  }
  return -1;
}
Bool IsKill(I64 t) {
  switch(t) {
    case IC_SHL_EQU:
    case IC_MUL_EQU:
    case IC_SHR_EQU:
    case IC_DIV_EQU:
    case IC_MOD_EQU:
    case IC_AND_EQU:
    case IC_OR_EQU:
    case IC_XOR_EQU:
    case IC_ADD_EQU:
    case IC_SUB_EQU:
    case IC_ASSIGN:
    case IC__PP:
    case IC__MM:
    case IC_PP_:
    case IC_MM_:
      return TRUE;
  }
  return FALSE;
}
CRPN *NthArgument(CRPN *r,I64 a) {
  r=r->next;
  while(--a>=0) {
    r=ICNext(r);
  }
  return r;
}
I64 FoffToMemberIdx(CCmpCtrl *cc,I64 foff) {
  CMemberLst *mlst=cc->htc.fun->member_lst_and_root;
  I64 idx=0;
  while(mlst) {
    if(mlst->offset==foff)
      return idx;
    mlst=mlst->next;
    idx++;
  }
 
 return -1;
}
CRPN *LabelToRPN(CCmpCtrl *cc,CCodeMisc *cm) {
  CRPN *head=&cc->coc.coc_head,*c;
  if(cm->rpn) return cm->rpn;
  for(c=head->next;c!=head;c=c->next) {
    if(c->type==IC_LABEL&&c->ic_data==cm) {
       cm->rpn=c;
       return c;
    }
  }
  return NULL;
}
CMemberLst *NthMember(CCmpCtrl *cc,I64 i) {
  CMemberLst *lst=cc->htc.fun->member_lst_and_root;
  while(--i>=0) {
    if(!lst) return NULL;
    lst=lst->next;
  }
  return lst;
} 
U0 DumpNthVar(CCmpCtrl *cc,I64 live) {
  CMemberLst *lst=cc->htc.fun->member_lst_and_root;
  while(--live>=0) {
    if(!lst) return;
    lst=lst->next;
  }
  if(!lst) return;
  DbgPrint("%s,",lst->str);
}
U0 DumpLiveSet(CCmpCtrl *cc,CI64Set * set) {
  I64 bit;
  for(bit=0;bit!=set->cnt;bit++)
    DumpNthVar(cc,set->body[bit]);
}
//Kind of like SSA
class CVarRenumber {
  CI64Set *nodes;
  I64 idx;
  I64 new_foffset;
};
class CVarRenumbers:CI64Set {
  I64 foffset;
};
class CGraphColoring {
  I64 *colors;
};
class CGraph {
  CI64Set **connects; //Use Bt
};
//This will restore the original IC_FRAME offsets
U0 VarRenumbersDel(CVarRenumbers *s) { 
  I64 o_foff=s->foffset;
  I64 cnt=s->cnt,cnt2;
  CVarRenumber *rn;
  CRPN *cur,*next;
  while(--cnt>=0) {
    rn=s->body[cnt];
    cnt2=rn->nodes->cnt;
    while(--cnt2>=0) {
      cur=rn->nodes->body[cnt2];
      next=ICNext(cur);
      while(next!=cur) {
	if(cur->type==IC_FRAME&&cur->ic_data==rn->new_foffset)
	  cur->ic_data=o_foff;
	cur=cur->next;
      }
    }
    I64SetDel(rn->nodes);
    Free(rn);
  }
  Free(s->body);
  Free(s);
}
Bool InfectAdjacent(CI64Set *add_to,CRPN *a,I64 midx) {
  CI64Set *stack;
  CI64Set *set;
  I64 cnt;
  CRPN *tmp;
  if(!BitSetHas(a->live_in,midx)&&!BitSetHas(a->kill_regs,midx))
    return FALSE;
  if(a->visited) return FALSE;
  stack=I64SetNew;
  I64SetAdd(stack,a,TRUE);
  while(stack->cnt) {
    a=stack->body[--stack->cnt];
    a->visited=TRUE;
    if(!BitSetHas(a->live_in,midx)&&!BitSetHas(a->kill_regs,midx)) goto skip;
    I64SetAdd(add_to,a,TRUE);  
    set=a->user_data;
    for(cnt=0;cnt!=set->cnt;cnt++) {
      tmp=set->body[cnt];
      if(!tmp->visited)
        I64SetAdd(stack,tmp,TRUE);
    }
    set=a->user_data2;
    for(cnt=0;cnt!=set->cnt;cnt++) {
      tmp=set->body[cnt];
      if(!tmp->visited)
        I64SetAdd(stack,tmp,TRUE);
    }
skip:;
  }
  I64SetDel(stack);
  return TRUE;
}
I64 MakeNumberedMember(CCmpCtrl *cc,I64 from_member,I64 num,I64 tmp_off,CMemberLst **who=NULL) {
  I64 mc;
  CMemberLst *nth=NthMember(cc,from_member),*new;
  new=CAlloc(sizeof CMemberLst);
  new->str=MStrPrint("%s.%d",nth->str,num);
  new->member_class=nth->member_class;
  new->reg=REG_ALLOC;
  new->use_cnt=1; //Avoid "unused" var warnings
  new->offset=tmp_off;
  MemberAdd(cc,new,cc->htc.fun,PRS1B_LOCAL_VAR);
  if(who) *who=new;
  new=cc->htc.fun->member_lst_and_root;
  cc->htc.fun->member_cnt++;
  mc=0;
  while(new) {
   mc++;
   new=new->next;
  }
  return mc-1;
}
CVarRenumbers *GenerateRenumbersForVar(CCmpCtrl *cc,I64 member,I64 *foff) {
  I64 new_foff=*foff,cnt=0,idx;
  CRPN *head=&cc->coc.coc_head,*cur;
  CMemberLst *mlst=NthMember(cc,member),*who;
  CVarRenumbers *ret=CAlloc(sizeof CVarRenumbers);
  CVarRenumber *renum;
  CI64Set *tset;
  CRPN *cur2,*next2,cnt2;
  ret->body=CAlloc(16*8);
  ret->cap=16;
  ret->foffset=mlst->offset;
  tset=I64SetNew;
  for(cur=head->next;cur!=head;cur=ICNext(cur)) {
    cur->visited=FALSE;
  }
  for(cur=head->next;cur!=head;cur=ICNext(cur)) {
    if(InfectAdjacent(tset,cur,member)) {
      *foff-=8;
      
      renum=CAlloc(sizeof CVarRenumber);
      renum->idx=MakeNumberedMember(cc,member,cnt++,*foff,&who);
      renum->nodes=I64SetNew;
      renum->new_foffset=*foff;
      for(idx=0;idx!=tset->cnt;idx++)
        I64SetAdd(renum->nodes,tset->body[idx],TRUE);

      //Renumber the offsets
      for(cnt2=tset->cnt-1;cnt2>=0;cnt2--) {
        cur2=tset->body[cnt2];
        next2=ICNext(cur2);

	BitSetRem(cur2->live_in,member);
	BitSetRem(cur2->live_out,member);
	BitSetRem(cur2->kill_regs,member);

	BitSetAdd(cur2->live_in,renum->idx);
	BitSetAdd(cur2->live_out,renum->idx);
	BitSetAdd(cur2->kill_regs,renum->idx);

        while(next2!=cur2) {
	  if(cur2->type==IC_FRAME&&cur2->ic_data==ret->foffset) {
	    who->use_cnt++;
	    cur2->ic_data=renum->new_foffset;
	  }
	  cur2=cur2->next;
        }
      }
      tset->cnt=0;
      I64SetAdd(ret,renum);
    }
  }
  I64SetDel(tset);
  return ret;
}
U0 GraphColoringDel(CGraphColoring *c) {
  Free(c->colors);
  Free(c);
}
Bool IsPtrVar(CCmpCtrl *cc,I64 foff) {
  CRPN *rpn,*head=&cc->coc.coc_head,*rpn2;
  for(rpn=head->next;rpn!=head;) { 
    if(rpn->type==IC_DEREF) {
      rpn2=ICNext(rpn);
      while(rpn!=rpn2) {
	if(rpn->type==IC_FRAME&&rpn->ic_data==foff) {
	  return TRUE;
	}
	rpn=rpn->next;
      }
    } else
      rpn=rpn->next;
  }
  return FALSE;
}
CGraphColoring *GraphColor(CCmpCtrl *cc,Bool ireg=TRUE) {
  CGraph *graph=CAlloc(sizeof CGraph);
  CGraphColoring *coloring=CAlloc(sizeof CGraphColoring);
  CRPN *rpn,*head=&cc->coc.coc_head;
  I64 bit,tmp,bit2,tmp2,mcnt=0,color;
  I64 reg_cnt,nth_best,penalty,best_penalty;
  CMemberLst *lst=cc->htc.fun->member_lst_and_root,**flat;
  COptMemberVar *mv; 
  I64 foff=cc->htc.fun->size,run,adj_score;
  CI64Set *renumbers=I64SetNew,*adjacent;
  U8 *canidates;
  while(lst) {
    mcnt++;
    lst=lst->next;
  }
  mv=CAlloc(sizeof(COptMemberVar)*mcnt);
  lst=cc->htc.fun->member_lst_and_root;
  for(tmp=0;tmp!=mcnt;tmp++) {
    mv[tmp].m=lst;
    lst=lst->next;
  }
  tmp2=mcnt;
  for(tmp=0;tmp!=mcnt;tmp++) {
    lst=mv[tmp].m;
    if(!IsFunArg(cc->htc.fun,lst)) {
      if(!(lst->reg==REG_NONE||0<=lst->reg<REG_REGS_NUM)) {
        if(lst->member_class->raw_type==RT_F64&&!ireg) {
re_number:;
	  I64SetAdd(renumbers,GenerateRenumbersForVar(cc,tmp,&foff));
        } else if(lst->member_class->raw_type!=RT_F64&&ireg) {
	  goto re_number;
        }
      }
    }
  }
  Free(mv);
  //Recompute with the splits
  lst=cc->htc.fun->member_lst_and_root;
  mcnt=0;
  while(lst) {
    mcnt++;
    lst=lst->next;
  }
  flat=CAlloc(8*mcnt);
  mv=CAlloc(sizeof(COptMemberVar)*mcnt);
  lst=cc->htc.fun->member_lst_and_root;
  for(tmp=0;tmp!=mcnt;tmp++) {
    flat[tmp]=lst;
    mv[tmp].m=lst;
    mv[tmp].score=lst->use_cnt;
    lst=lst->next;
  }

  for(rpn=head->next;rpn!=head;rpn=rpn->next) {
    if(rpn->type==IC_FRAME) {
      for(tmp=0;tmp!=mcnt;tmp++) {
        if(mv[tmp].m->offset==rpn->ic_data) {
	  if(rpn->last(CRPN*)->type==IC_ADDR) {
	    mv[tmp].addrof_cnt++;
	    mv[tmp].score=0;
	    mv[tmp].m->reg=REG_NONE;
	  } else if(cc->flags&CCF_NO_REG_OPT) {
	    mv[tmp].score=0;
	    mv[tmp].m->reg=REG_NONE;
	  } else if(!mv[tmp].addrof_cnt)
	    mv[tmp].score++;
        }
      }
    }
  }


  coloring->colors=CAlloc(8*mcnt);
  canidates=MAlloc(mcnt+64);
  graph->connects=CAlloc(8*mcnt);
  for(tmp=0;tmp!=mcnt;tmp++) {
    graph->connects[tmp]=I64SetNew;
  }
  for(rpn=head->next;rpn!=head;rpn=rpn->next) {
    if(rpn->live_in&&rpn->kill_regs) {
      for(tmp=BitSetWidth(rpn->live_in)-1;tmp>=0;tmp--) {
        if(BitSetHas(rpn->live_in,tmp)) {
          bit=tmp;
          if(!(flat[bit]->member_class->raw_type==RT_F64^^!ireg)&&flat[bit]->reg!=REG_NONE) {
	    for(tmp2=BitSetWidth(rpn->live_in)-1;tmp2>=0;tmp2--) {
	      if(BitSetHas(rpn->live_in,tmp2)) {
	        bit2=tmp2;
	        if(!(flat[bit2]->member_class->raw_type==RT_F64^^!ireg)&&flat[bit2]->reg!=REG_NONE) {
	          I64SetAdd(graph->connects[bit],bit2);
	          I64SetAdd(graph->connects[bit2],bit);
	        }
	      }
	    }
//Include kill regs
	    for(tmp2=BitSetWidth(rpn->kill_regs)-1;tmp2>=0;tmp2--) {
              if(BitSetHas(rpn->kill_regs,tmp2)) {
	        bit2=tmp2;
	        if(!(flat[bit2]->member_class->raw_type==RT_F64^^!ireg)&&flat[bit2]->reg!=REG_NONE) {
	          I64SetAdd(graph->connects[bit],bit2);
	          I64SetAdd(graph->connects[bit2],bit);
	        }
	      }
	    }
	  }
        }
      }
    }
  }
  if(ireg)
    reg_cnt=AIWNIOS_IREG_CNT;
  else
    reg_cnt=AIWNIOS_FREG_CNT;
//Pre-color varaibles that are defined via "reg" keyword
  for(tmp=0;tmp!=mcnt;tmp++) {
    if(0<=mv[tmp].m->reg<REG_REGS_NUM) {
      coloring->colors[tmp]=mv[tmp].m->reg;
    }
  }

  best_penalty=I64_MAX;
  I64 *best_colors=CAlloc(8*mcnt);
  QSort(mv,mcnt,sizeof (COptMemberVar),&OptMVCompare);

  MemSetI64(coloring->colors,-1,mcnt);
  for(run=2;run>=0;--run) {
    for(nth_best=mcnt-1;nth_best>=0;nth_best--) {
      for(tmp=0;tmp!=mcnt;tmp++) {
        if(mv[nth_best].m==flat[tmp]) {//Unused var?
          if(!graph->connects[tmp]->cnt) {
	    coloring->colors[tmp]=-1;
	    break;
          }
          MemSet(canidates,TRUE,reg_cnt);
          for(tmp2=0;tmp2!=mcnt;tmp2++) {
	    if(I64SetHas(graph->connects[tmp],tmp2)) {
	      if(tmp!=tmp2) {
	        color=coloring->colors[tmp2];
	        canidates[color]=FALSE;
	      }
	    }
          }

	  color=coloring->colors[tmp];
	  if(color!=-1) {
	    if(canidates[color]) {
	      goto skip;
	    }
	  }
          coloring->colors[tmp]=-1;
          for(color=0;color!=reg_cnt;color++) {
	    if(canidates[color]) {
	      coloring->colors[tmp]=color;
	      break;
	    }
          }
#ifdef TARGET_X86
//Right here our color points to R13(VarIRegToReg(color)==R13)
	  if(ireg&&canidates[color]) { 
//Try register R13(has quircks with being used with SIBs)
//https://wiki.osdev.org/X86-64_Instruction_Encoding
            if(!IsPtrVar(cc,flat[tmp]->offset)) {
	      coloring->colors[tmp]=color;
	      break;
	    }
	  }
#endif
skip:;
        }
      }
    }

    penalty=0;
    for(tmp=0;tmp!=mcnt;tmp++) {
      if(coloring->colors[tmp]==-1&&flat[tmp]->reg!=REG_NONE) {
        adjacent=graph->connects[tmp];
	if(adjacent->cnt)
          penalty+=flat[tmp]->use_cnt;
      }
    }
    if(penalty<best_penalty) {
      best_penalty=penalty;
      MemCpy(best_colors,coloring->colors,8*mcnt);
    }
    if(!penalty)
      break;

//Make spilled registers unspilled to check if they are good.
      for(tmp=0;tmp!=mcnt;tmp++) {
        adjacent=graph->connects[tmp];
	if(!adjacent->cnt)
	  goto skip2;
        if(flat[tmp]->reg!=REG_NONE&&coloring->colors[tmp]==-1)  {  //Not Predefined
          MemSet(canidates,TRUE,reg_cnt);
	  adj_score=I16_MAX;
	  for(tmp2=0;tmp2!=adjacent->cnt;tmp2++) {
	    color=coloring->colors[adjacent->body[tmp2]];
	    if(color!=-1) {
	      canidates[color]=FALSE;
	      adj_score=MinI64(adj_score,flat[tmp2]->use_cnt);
	    }
          }
	  for(tmp2=0;tmp2!=reg_cnt;tmp2++) {
	    if(canidates[tmp2]) {
	      coloring->colors[tmp]=tmp2;
	      goto skip2;
	    }
	  }
	  for(tmp2=0;tmp2!=adjacent->cnt;tmp2++) {
	    if(adj_score==flat[tmp2]->use_cnt) {
	      coloring->colors[tmp2]=-1;
	      break;
	    }
	  }
skip2:;
        }
      }
  }
  MemCpy(coloring->colors,best_colors,8*mcnt);
  Free(best_colors);

  for(tmp=0;tmp!=mcnt;tmp++) {
    if(!(flat[tmp]->member_class->raw_type==RT_F64^^!ireg))
      if(flat[tmp]->reg==REG_ALLOC||flat[tmp]->reg==REG_UNDEF) {
        if(coloring->colors[tmp]==-1)
	  flat[tmp]->reg=REG_NONE;
        else if(ireg)
	  flat[tmp]->reg=VarIRegToReg(coloring->colors[tmp]);
	        else
	  flat[tmp]->reg=VarFRegToReg(coloring->colors[tmp]);
      }
    I64SetDel(graph->connects[tmp]);
  }

  for(rpn=head->last;rpn!=head;rpn=rpn->last) {
    if(rpn->type==IC_FRAME) {
      for(tmp=0;tmp!=mcnt;tmp++) {
        if(rpn->ic_data==mv[tmp].m->offset) {
//I hope your arhcitectue of choice has less than REG_NONE registers
	  if(0<=mv[tmp].m->reg<REG_NONE) { //REG_NONE is 32,other buddies are larger
	    rpn->type=IC_REG;
	    rpn->ic_data=mv[tmp].m->reg;
	  }
	  break;
        }
      }
    }
  }

//Keep area for the poo poo tempraries for FunRep
  cc->htc.fun->size=foff;

  tmp=renumbers->cnt;
  while(--tmp>=0)
    VarRenumbersDel(renumbers->body[tmp]);
  I64SetDel(renumbers);
  Free(graph->connects);
  Free(canidates);
  Free(flat);
  Free(graph);
  Free(mv);
  return coloring;
}
U0 _RPNConnect(CRPN *to,CRPN *from) {
  I64SetAdd(to->user_data,from);
  I64SetAdd(from->user_data2,to);
}
U0 LiveVariableAnalysis(CCmpCtrl *cc) {
  CRPN *rpn,*head=&cc->coc.coc_head,**flat,*end_rpn,*start_rpn,*arg,*jump_to,*rpn2;
  CCodeMisc *tab;
//On first_run I generate the graph connections,no need to recompute each time
  Bool changed=FALSE,first_run=TRUE;
  I64 cnt=0,idx,idx2;
  I64 which;
  I64 *old_live,old_in,old_out;
  CI64Set *changed1=I64SetNew,*changed2=I64SetNew,*tmp_set;
  rpn=head->next;
  while(rpn!=head) {
    rpn=ICNext(rpn);
    cnt++;
  }
  I64 run=0;
  flat=CAlloc(cnt*8);
  idx=0;
  rpn=head->next;
  while(rpn!=head) {
//REVERSE polish notation
    flat[cnt-idx-1]=rpn;
    idx++;
    rpn->user_data=I64SetNew;
    rpn->user_data2=I64SetNew;
    rpn->live_in=BitSetNew;
    rpn->live_out=BitSetNew;
    rpn->kill_regs=BitSetNew;
    rpn->use_regs=BitSetNew;
    I64SetAdd(changed1,rpn,TRUE);
    rpn=ICNext(rpn);
  }
  do {
    for(idx=0;idx!=cnt;idx++) {
      flat[idx]->visited=FALSE;
      flat[idx]->changed=FALSE;
    }
    changed=FALSE;
    for(idx=0;idx!=changed1->cnt;idx++) {
      start_rpn=changed1->body[idx];
      end_rpn=ICNext(start_rpn);

      old_in=BitSetCnt(start_rpn->live_in);
      old_out=BitSetCnt(start_rpn->live_out);

      if(first_run)
        for(rpn=start_rpn;rpn!=end_rpn;rpn=rpn->next) {
	  idx2=intermediate_code_table[rpn->type].arg_cnt;
	  switch(idx2) {
	    case IS_V_ARG:
	      idx2=rpn->length;
	      break;
	    case IS_2_ARG:
	      idx2=2;
	      break;
	    case IS_1_ARG:
	      idx2=1;
	      break;
	    case IS_0_ARG:
	      idx2=0;
	      break;
	  }
	  if(IsKill(rpn->type)) {
	    arg=NthArgument(rpn,--idx2);
	    if(arg->type==IC_FRAME) {
	      which=FoffToMemberIdx(cc,arg->ic_data);
	      if(rpn->type==IC__MM||rpn->type==IC__PP||
		    rpn->type==IC_MM_||rpn->type==IC_PP_||
		    rpn->type==IC_SHL_EQU||
		    rpn->type==IC_MUL_EQU||
		    rpn->type==IC_SHR_EQU||
		    rpn->type==IC_DIV_EQU||
		    rpn->type==IC_MOD_EQU||
		    rpn->type==IC_AND_EQU||
		    rpn->type==IC_OR_EQU||
		    rpn->type==IC_XOR_EQU||
		    rpn->type==IC_SUB_EQU||
		    rpn->type==IC_ADD_EQU)
	        BitSetAdd(start_rpn->use_regs,which);
	      BitSetAdd(start_rpn->kill_regs,which);
	    }
	  }
          while(--idx2>=0) {
	    arg=NthArgument(rpn,idx2);
	    if(arg->type==IC_FRAME) {
	      which=FoffToMemberIdx(cc,arg->ic_data);
	      BitSetAdd(start_rpn->use_regs,which);
	    }
	  }
        }

      rpn=start_rpn;
      switch(rpn->type) {
        case IC_BR_ZERO ... IC_BR_NOT_BTC:
	  if(first_run) {
	    jump_to=LabelToRPN(cc,rpn->ic_data);
	    if(!jump_to) throw('Compiler');
	    _RPNConnect(jump_to,rpn);
	    BitSetAddAll(rpn->live_out,jump_to->live_in);
	    goto next_ic;
	  } else {
use_outgoing:
	    for(idx2=0;idx2!=rpn->user_data2(CI64Set*)->cnt;idx2++) {
	      jump_to=rpn->user_data2(CI64Set*)->body[idx2];
	      BitSetAddAll(rpn->live_out,jump_to->live_in);
	    }
	    break;
	  }
        case IC_NOBOUND_SWITCH:
        case IC_SWITCH:
	  if(first_run) {
	    tab=rpn->ic_data;
	    for(idx2=0;idx2!=tab->range;idx2++) {
	      if(tab->jmp_table[idx2]) {
	        jump_to=LabelToRPN(cc,tab->jmp_table[idx2]);
	        if(!jump_to) throw('Compiler');
	        BitSetAddAll(rpn->live_out,jump_to->live_in);
	        _RPNConnect(jump_to,rpn);
	      }
	    }
	    if(tab->dft) {
	      jump_to=LabelToRPN(cc,tab->dft);
	      BitSetAddAll(rpn->live_out,jump_to->live_in);
	      _RPNConnect(jump_to,rpn);
	    }
	  } else
	    goto use_outgoing;
	  break;
        case IC_RET:
	  break;
	    case IC_JMP:
	  if(first_run) {
	    jump_to=LabelToRPN(cc,rpn->ic_data);
	    if(jump_to) {
	      BitSetAddAll(rpn->live_out,jump_to->live_in);
	      _RPNConnect(jump_to,rpn);
	    }
	  } else
	    goto use_outgoing;
	  break;
        case IC_SUB_CALL:
//Special behavior,we are kind of linking the ends(IC_SUB_CALL/IC_SUB_RET) in a weird way
//DO NOT first_run CHECK THIS
	  jump_to=LabelToRPN(cc,rpn->ic_data);
	  if(jump_to) {
	    BitSetAddAll(rpn->live_out,jump_to->live_in);
	    _RPNConnect(jump_to,rpn);
//Also connect to end of SUB_CALL
	    for(idx2=0;idx2!=cnt;idx2++) {
	     if(flat[idx2]->type==IC_SUB_RET&&flat[idx2]->ic_data==rpn->ic_data) {
	        jump_to=flat[idx2];
	        _RPNConnect(rpn,jump_to);
	        BitSetAddAll(jump_to->live_out,rpn->live_in);
	      }
	    }
	  }
	  goto next_ic;
	  break;
        case IC_SUB_RET:
	  break;
        default:
next_ic:
	  if(first_run) {
	    for(idx2=0;idx2!=cnt;idx2++) {
	      if(flat[idx2]==rpn&&idx2+1<cnt) {
	        BitSetAddAll(rpn->live_out,flat[idx2+1]->live_in);
	        _RPNConnect(flat[idx2+1],rpn);
	        break;
	      }
	    }
	  } else
	    goto use_outgoing;
      }
      BitSetRemAll(rpn->live_in,rpn->live_in);
      BitSetAddAll(rpn->live_in,rpn->live_out);
      BitSetRemAll(rpn->live_in,rpn->kill_regs);
      BitSetAddAll(rpn->live_in,rpn->use_regs);

      if(old_in!=BitSetCnt(rpn->live_in)||BitSetCnt(rpn->live_out)!=old_out) {
        changed|=TRUE;
        I64SetAdd(changed2,rpn,TRUE);
        tmp_set=rpn->user_data;
        for(idx2=0;idx2!=tmp_set->cnt;idx2++) {
          rpn2=tmp_set->body[idx2];
          if(!rpn2->changed)
            I64SetAdd(changed2,rpn2,TRUE);
        }
      }
    }
    first_run=FALSE;
    SwapI64(&changed1,&changed2);
    changed2->cnt=0;
  } while(changed);

  Free(flat);
  I64SetDel(changed1);
  I64SetDel(changed2);
}
//
// Turns my CRPN from the parser into the IC for the C side
//
U8 *AiwniosCompile(CCmpCtrl *cc,I64 *res_sz=NULL,CDbgInfo **info) {
  CRPN *head=&cc->coc.coc_head,*cur,*new;
  CCodeMisc *misc,**table;
  CMemberLst *mlst,*mlst2;
  COptMemberVar *mv=NULL;
  CHashImport *h;
  CAOTImportExport *tmpaie;
  CAOTAbsAddr *tmpabs;
  CAOTHeapGlbl *tmpgh,*glbl_gh;
  CAOTHeapGlblRef *tmpghr;
  CAddShortRelocData fill_user_data;
  I64 idx,argc,member_cnt=0,ir=AIWNIOS_IREG_START,fr=AIWNIOS_FREG_START,foff,*foff_lookup=NULL,align,idx2;
  I64 reported_min_ln=I16_MAX,reported_max_ln=I16_MIN;
  U8 **aiwnios_dbg_info=NULL;
  U8 *acc=__HC_CmpCtrlNew(),*machine_code,*stat_nam,*dumb_crap_apple;
  I64 *table_ptr;
  U8 *cc2=__HC_CodeCtrlPush(acc);
  U8 *dumb_static;
  U8 name[STR_LEN],name2[STR_LEN];
  U8 *name_ptr=name,*name2_ptr=name2;
  Bool force_segment=FALSE;
  I64 statics_sz=0;
  static I64 static_cnt=0;
  stat_nam=MStrPrint(".STATIC_%d",static_cnt++);
//Turn all IC_IMM_I64s that are used as F64's to IC_IMM_I64
  for(cur=head->next;cur!=head;cur=cur->next) {
    if(cur->ic_class&&cur->type!=IC_HOLYC_TYPECAST) {
      if(!cur->ic_dim&&!cur->ic_class->ptr_stars_cnt&&OptClassFwd(cur->ic_class)->raw_type==RT_F64) {
        idx=intermediate_code_table[cur->type].arg_cnt;
        switch(idx) {
	  case IS_V_ARG:
	    idx=cur->length;
	    break;
	  case IS_2_ARG:
	    idx=2;
	    break;
	  case IS_1_ARG:
	    idx=1;
	    break;
	  default:
	    idx=0;
        }
	new=cur->next;
	switch(cur->type) {
	  case IC_CALL:
	  case IC_CALL_INDIRECT:
	  case IC_CALL_INDIRECT2:
	  case IC_CALL_EXTERN:
	  case IC_CALL_IMPORT:
	    //Dont convert function address(I64) into a F64
	    goto skip_convert;
	}
        while(--idx>=0) {
	  if(new->type==IC_IMM_I64) {
	    new->type=IC_IMM_F64;
	    new->imm_f64=new->imm_i64;
	    new->ic_class=cmp.internal_types[RT_F64];
	  }
	  new=ICNext(new);
	}
skip_convert:;
      }
    }
  }
  if(GetOption(OPTf_BOUNDS_CHECK)&&HashFind("WhineOOB",cc->htc.glbl_hash_table,HTT_FUN)) {
    for(cur=head->next;cur!=head;cur=cur->next) {
      if(cur->type==IC_DEREF) {
        new=CAlloc(sizeof CRPN);
        new->type=IC_ADDR_IMPORT;
        new->ic_class=cmp.internal_types[RT_I64];
        new->ic_data=CodeMiscHashNew(cc,HashFind("WhineOOB",cc->htc.glbl_hash_table,HTT_FUN));
        QueInsRev(new,RPNNext(cur)); 
        new=CAlloc(sizeof CRPN);
        new->type=IC_CALL_INDIRECT;
        new->length=2;
        new->ic_class=cmp.internal_types[RT_I64];
        QueIns(new,cur);
      }
    }    
  }
  #ifdef TARGET_X86
  if(cc->flags&CCF_AOT_COMPILE)
    AiwniosMakeShortJmps(cc,acc);
  #endif
  #ifdef TARGET_RISCV
  if(cc->flags&CCF_AOT_COMPILE)
    AiwniosMakeShortJmps(cc,acc);
  #endif
  if(cc->flags&CCF_AOT_COMPILE)
    __HC_CmpCtrl_SetAOT(acc);
  if(info) {
    idx=cc->max_line-cc->min_line;
    *info=MAlloc((idx+2)*8+sizeof(CDbgInfo));
    info[0]->min_line=cc->min_line;
    info[0]->max_line=cc->max_line;
  }
  //
  // Step 1,do register allocation stuff by counting the occuraces of IC_FRAME,and making
  //  sure arent getting the address of said location
  //
  if(cc->htc.fun) {
    if(GetOption(OPTf_NO_REG_VAR)) {
      mlst=cc->htc.fun->member_lst_and_root;
      while(mlst) {
        mlst->reg=REG_NONE;
	mlst=mlst->next;
      }
    } else {
      LiveVariableAnalysis(cc);
      GraphColoringDel(GraphColor(cc,TRUE));
      GraphColoringDel(GraphColor(cc,FALSE));
    }
    member_cnt=cc->htc.fun->member_cnt;
    mlst=cc->htc.fun->member_lst_and_root;
    if(cc->htc.fun->flags&(1<<Ff_DOT_DOT_DOT)) {
      member_cnt+=2;
    }
    mv=CAlloc(member_cnt*sizeof(COptMemberVar));
    foff_lookup=CAlloc(member_cnt*sizeof I64);
    for(idx=0;idx!=member_cnt;idx++) {
      mv[idx].m=mlst;
      mlst=mlst->next;
    }
    
    //In FunRep,the function is assumes to store registers somewhere
    //So i "store" them at the start of the function
#ifdef TARGET_RISCV
    //In RiscV
    // s0    -> TOP
    // s0-8  -> return_address
    // s0-16 -> old s0
    foff=16;
#else
    foff=0;
#endif
    for(idx=0;idx!=member_cnt;idx++) {
      if(mv[idx].m->reg!=REG_NONE) {
        mv[idx].m->offset=foff;
        foff+=8;
      }
    }
    //Assign offsets
    for(idx=0;idx!=member_cnt;idx++) {
      mlst=mv[idx].m;
//If spilled(to stack REG_NONE) reuse location  for same vairable
      if(mlst->reg==REG_NONE&&StrOcc(mlst->str,'.')) {
	StrScan(mlst->str,"%s.",&name_ptr);
	  mlst2=MemberFind(name,cc->htc.fun);
	  if(mlst2) {
              foff_lookup[idx].i32[1]=mlst2->offset;
              foff_lookup[idx].i32[0]=mlst->offset;              
	      mlst->offset=mlst2->offset;
	      goto found_offset;
	  }
      }
//idx here is the alignment
      align=8;
      if(mlst->reg==REG_NONE) {
        #ifdef TARGET_AARCH64
        foff_lookup[idx].i32[1]=foff;
        foff_lookup[idx].i32[0]=mlst->offset;
        mlst->offset=foff;
        foff+=mlst->member_class->size*mlst->dim.total_cnt;  
        if(foff&(align-1))
          foff+=align-(foff&(align-1)); //Align to 8
        #else 
	if(IsFunArg(cc->htc.fun,mlst)&&!(mlst->flags&MLF_DOT_DOT_DOT&&!StrCmp(mlst->str,"argv"))) {
	  //Arguments are passed on stack so no need to mode
	  foff_lookup[idx].i32[0]=mlst->offset;
	  //16 to offset [PC,RBP] pair at base 
	  #ifdef TARGET_AARCH64
	  //ARM now uses a native SysV ABI srot of
	  goto no_farg;
	  #endif
	  #ifdef TARGET_RISCV
	  //RISCV now uses a native SysV ABI srot of
	  goto no_farg;
	  #else
	  mlst->offset=-16-8*FunArgIdx(cc->htc.fun,mlst);
	  #endif
          foff_lookup[idx].i32[1]=mlst->offset;
	} else {
	no_farg:
          foff_lookup[idx].i32[0]=mlst->offset;
          //In X86_64 the stack grows down and the RBP is above the stack pointer,
          //SO MOVE THE OFFSETS DOWN TO THE BOTTOM
          //RBP
          //x0
          //x1
          //....
          //x8 <<=offset
          foff+=mlst->member_class->size*mlst->dim.total_cnt;  
          if(foff&(align-1))
            foff+=align-(foff&(align-1)); //Align to 8
          mlst->offset=foff;
          foff_lookup[idx].i32[1]=foff;
	}
        #endif
      } else {
//Assign  them a spot on the stack for debugging purposes
	foff+=8;
        if(foff&7)
          foff+=8-(foff&7); //Align to 8
	mlst->offset=foff;
found_offset:;;
      }
    }
    if(foff&7)
      foff+=8-(foff&7); //Align to 8    
    cc->htc.fun->size=foff+16;
  }

  //Replace the old frame address with the new one 
  for(cur=head->last;cur!=head;cur=cur->last)
    if(cur->type==IC_FRAME)
      for(idx=0;idx!=member_cnt;idx++) {
        foff=foff_lookup[idx].i32[0];
        if(foff==cur->ic_data) {
          cur->ic_data=foff_lookup[idx].i32[1];
          break;
        }
      }
  Free(foff_lookup);
  
  //
  // Step 1.5,Assign the variables from the arguments
  //
  if(cc->htc.fun) {
    __HC_ICAdd_SetFrameSize(cc2,cc->htc.fun->size);
    mlst=cc->htc.fun->member_lst_and_root;
    argc=cc->htc.fun->arg_cnt;
   if(cc->htc.fun->flags&(1<<Ff_DOT_DOT_DOT))
      argc+=2;
    for(idx=0;idx!=argc;idx++) {
      if(mlst->reg==REG_NONE)
        __HC_ICAdd_Frame(cc2,mlst->offset,mlst->member_class->raw_type,0);
      else if(mlst->member_class->raw_type!=RT_F64) {
        __HC_ICAdd_IReg(cc2,mlst->reg,mlst->member_class->raw_type,0);
      } else {
        __HC_ICAdd_FReg(cc2,mlst->reg);      
      }

#ifdef TARGET_X86
//Not yet implemented for AARCH64
      if(mlst->flags&MLF_DOT_DOT_DOT&&!StrCmp(mlst->str,"argv")) {
        __HC_ICAdd_GetVargsPtr(cc2);
      } else
        __HC_ICAdd_Arg(cc2,idx);
#endif
#ifdef TARGET_RISCV
//Not yet implemented for AARCH64
      if(mlst->flags&MLF_DOT_DOT_DOT&&!StrCmp(mlst->str,"argv")) {
        __HC_ICAdd_GetVargsPtr(cc2);
      } else
        __HC_ICAdd_Arg(cc2,idx);
#endif
#ifdef TARGET_AARCH64
        __HC_ICAdd_Arg(cc2,idx);
#endif
      mlst=mlst->next;
    }
    Free(mv);
  } else {
    #ifdef TARGET_X86
    __HC_ICAdd_SetFrameSize(cc2,0);
    #endif
    #ifdef TARGET_AARCH64
    __HC_ICAdd_SetFrameSize(cc2,16);
    #endif
    #ifdef TARGET_RISCV
    __HC_ICAdd_SetFrameSize(cc2,16);
    #endif
    
  }
  //
  //
  // Step 2,convert CCodeMisc's to AIWNIOS codegen CCodeMiscs
  //
  for(misc=cc->coc.coc_next_misc;misc!=&cc->coc.coc_next_misc;misc=misc->next) {
    switch(misc->type) {
    break;case CMT_HASH_ENTRY:
      //See spider thing for more notes 
    break;case CMT_LABEL:
    misc->addr=__HC_CodeMiscLabelNew(acc,&misc->addr2);
    break;case CMT_GOTO_LABEL:
      if(!(misc->flags&CMF_DEFINED)&&misc->str) {
	PrintErr("Undefined reference to label \"%s\".\n",misc->str);
	LexExcept(cc,"Undefined label reference");
      }
      misc->addr=__HC_CodeMiscLabelNew(acc,&misc->addr2);
    break;case CMT_STR_CONST:
      misc->addr=__HC_CodeMiscStrNew(acc,misc->str,misc->st_len);
    //
    // Skip this for now,we want to assign the labels AIWNIOS codegen CCodeMiscs to
    //  the labels and we will pass the labels'->addr to __HC_CodeMiscJmpTableNew
    //break;case CMT_JMP_TABLE:
    //  misc->addr=__HC_CodeMiscJmpTableNew(cc2);
    //
    }
  }
  //See above note
  for(misc=cc->coc.coc_next_misc;misc!=&cc->coc.coc_next_misc;misc=misc->next) {
    if(misc->type==CMT_JMP_TABLE) {
      table=CAlloc(misc->range*8);
      for(idx=0;idx!=misc->range;idx++)
        table[idx]=misc->jmp_table[idx]->addr;
      misc->addr=__HC_CodeMiscJmpTableNew(acc,table,&misc->addr2,misc->range);
      Free(table);
    }
  }
  //
  // Step 3,Convert static addresses to static offsets and compute statics size
  //
  if(cc->htc.fun) {
    //
    // foff_lookup will have form [orig_static_data1,offset1,...,orig_static_dataN,offsetN]
    //
    foff=0;
    mlst=cc->htc.fun->member_lst_and_root;
    for(idx=0;idx!=member_cnt;idx++) {
      if(mlst->flags&MLF_STATIC) {
//TODO
//        __HC_ICAdd_StaticData(acc,cc2,foff,mlst->static_data,mlst->member_class->size*mlst->dim.total_cnt);
		mlst->offset=foff;
        foff+=mlst->member_class->size*mlst->dim.total_cnt;
        if(foff%8)
          foff+=8-foff%8;
      }
      mlst=mlst->next;
    }
    if(foff) {
      statics_sz=foff;
    }
  }
  //
  // Step 4,OptClassFwd the ic_class'es to reduce to I64 to I64i's etc
  //
  // Classes with base type's need to be OptClassFwd'ed to reduce down to thier base type,otherwise
  //   it is probably an RT_I64
  for(cur=head->last;cur!=head;cur=cur->last) {
    if(cur->ic_class)
      cur->ic_class=OptClassFwd(cur->ic_class);
  }
  //
  // Step 6,convert the CRPN's into AIWNIOS rpns
  //
  for(cur=head->last;cur!=head;cur=cur->last) {
    switch(cur->type) {
    break;case IC_MAX_I64:
      new=__HC_ICAdd_Max_I64(cc2);
    break;case IC_MAX_U64:
      new=__HC_ICAdd_Max_U64(cc2);
    break;case IC_MIN_I64:
      new=__HC_ICAdd_Min_I64(cc2);
    break;case IC_MIN_U64:
      new=__HC_ICAdd_Min_U64(cc2);
    break;case IC_FS:
    if(!cur->ic_data) {
    //Unique references to the TLS offset(unlike normal functions whose pointer is stored at the end of the function and loaded from there)
      misc=cur->ic_data=COCMiscNew(cc,CMT_HASH_ENTRY);
      misc->h=HashFind("__Fs",cc->htc.glbl_hash_table,HTT_FUN);
      if(!misc->h)
        misc->h=HashFind("__Fs",Fs->hash_table,HTT_FUN); //Maybe we arent AOT Compiling
    }
    misc=cur->ic_data;
    misc->addr=INVALID_PTR;
    misc->flags&=~CMF_SHORT_ADDR;
    #ifdef TARGET_X86
    force_segment=TRUE;
    #else
    force_segment=FALSE;
    #endif
    if(force_segment) {
      //Bypass import and use raw one on X86,they are predictable
      misc->h=HashFind("__Fs",Fs->hash_table,HTT_FUN);
      new=__HC_ICAdd_I64(cc2,misc->h(CHashFun*)->exe_addr);      
    } else if(!(misc->h->type&HTF_IMPORT)) {
      new=__HC_ICAdd_I64(cc2,misc->h(CHashFun*)->exe_addr);
    } else
      new=__HC_ICAdd_RelocUnique(
          acc,
          cc2,
          &misc->addr,
          "__Fs",
          RT_U8,
          1
        );
    new=__HC_ICAdd_Fs(cc2);
	break;case IC_GS:
	
	if(!cur->ic_data) {
    //Unique references to the TLS offset(unlike normal functions whose pointer is stored at the end of the function and loaded from there)
      misc=cur->ic_data=COCMiscNew(cc,CMT_HASH_ENTRY);
      misc->h=HashFind("__Gs",cc->htc.glbl_hash_table,HTT_FUN);
      if(!misc->h)
        misc->h=HashFind("__Gs",Fs->hash_table,HTT_FUN); //Maybe we arent AOT Compiling
    }
    misc=cur->ic_data;
    misc->flags&=~CMF_SHORT_ADDR;
    misc->addr=INVALID_PTR;
    
    #ifdef TARGET_X86
    force_segment=TRUE;
    #else
    force_segment=FALSE;
    #endif
    if(force_segment) {
      //Bypass import and use raw one on X86,they are predictable
      misc->h=HashFind("__Gs",Fs->hash_table,HTT_FUN);
      new=__HC_ICAdd_I64(cc2,misc->h(CHashFun*)->exe_addr);      
    } else if(!(misc->h->type&HTF_IMPORT)) {
      new=__HC_ICAdd_I64(cc2,misc->h(CHashFun*)->exe_addr);
    } else
      new=__HC_ICAdd_RelocUnique(
          acc,
          cc2,
          &misc->addr,
          "__Gs",
          RT_U8,
          1
        );
    new=__HC_ICAdd_Gs(cc2);
	    break;case IC_LOCK:
    new=__HC_ICAdd_Lock(cc2);    
    break;case IC_FRAME:
    if(cur->last(CRPN*)->type==IC_ADDR) {
      //Our HolyC part accounts for the pointers for us
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,RT_U8,0);
    } else if(cur->ic_class->ptr_stars_cnt)
      //All pointer arithmetic is pre-computed
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,RT_PTR,0);
    else if(!cur->ic_dim)
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,cur->ic_class->raw_type,0);
    else
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,RT_I64,0);
    break;case IC_BR_ZERO:
    new=__HC_ICAdd_LNot(cc2);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_NOT_ZERO:
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    //break;case IC_BR_CARRY: ???
    //break;case IC_BR_NOT_CARRY: ???
    break;case IC_BR_EQU_EQU:
    new=__HC_ICAdd_EqEq(cc2);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_NOT_EQU:
    new=__HC_ICAdd_Ne(cc2);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_LESS:
    new=__HC_ICAdd_Lt(cc2);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_GREATER_EQU:
    new=__HC_ICAdd_Ge(cc2);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_GREATER:
    new=__HC_ICAdd_Gt(cc2);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_LESS_EQU:
    new=__HC_ICAdd_Le(cc2);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    //break;case IC_BR_EQU_EQU2: ???
    //break;case IC_BR_NOT_EQU2:  ???
    //break;case IC_BR_LESS2 ???
    //break;case IC_BR_GREATER_EQU2 ???
    //break;case IC_BR_GREATER2 ???
    //break;case IC_BR_LESS_EQU2 ???
    //break;case IC_BR_AND_ZERO ???
    //break;case IC_BR_AND_NOT_ZERO ???
    //break;case IC_BR_MM_ZERO ???
    //break;case IC_BR_MM_NOT_ZERO ???
    //break;case IC_BR_AND_AND_ZERO ???
    //break;case IC_BR_AND_AND_NOT_ZERO ???
    //break;case IC_BR_OR_OR_ZERO ???
    //break;case IC_BR_OR_OR_NOT_ZERO ???
    break;case IC_END: //This is symbolic 
    break;case IC_NOP1:
    break;case IC_END_EXP:
    break;case IC_NOP2:
    break;case IC_LABEL:
      new=__HC_ICAdd_Label(cc2,cur->ic_data(CCodeMisc*)->addr);
break;case IC_ASM:
  __HC_ICAdd_Label(cc2,__HC_CodeMiscLabelNew(acc,&cur->ic_data(CAiwniosAOTBlob*)->final_start_addr));
   new=__HC_ICAdd_RawBytes(cc2,cur->ic_data(CAiwniosAOTBlob*)->data,cur->ic_data(CAiwniosAOTBlob*)->len);
            
//break;case IC_CALL_START: AIWNIOS DOESNT USE THESE
    //break;case IC_CALL_END:
    //break;case IC_CALL_END2:
    //break;case IC_RETURN_VAL:
    //break;case IC_RETURN_VAL2:
    break;case IC_IMM_I64:
    new=__HC_ICAdd_I64(cc2,cur->imm_i64);
    break;case IC_IMM_F64:
    new=__HC_ICAdd_F64(cc2,cur->imm_f64);
    break;case IC_STR_CONST:
    new=__HC_ICAdd_Str(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_ABS_ADDR:
    misc=COCMiscNew(cc,CMT_LABEL);
    misc->str=MStrPrint("0x%x",cur->imm_i64);
    misc->flags&=~CMF_SHORT_ADDR;
    misc->addr=INVALID_PTR;
    new=__HC_ICAdd_Reloc(
          acc,
          cc2,
          &misc->addr,
          misc->str,
          RT_U8,
          1
        );
    break;case IC_ADDR_IMPORT:
    //
    //Aiwnios ALWAYS takes 1 import misc for each IMPORTED symbol(which can
    //  be looked up multiple times by the function)
    //          _____
    // ________/     \________
    // | |   /( ^___^ )\   | |
    // | |  | /( \_/ )\ |  | |
    // | |  | | \___/ | |  | |
    //_/_| _|_|       |_|_ |_ \_
    //
    h=cur->ic_data(CCodeMisc*)->h;
    if(cur->ic_flags&ICF_SHORT_JMP) {
      new=__HC_ICAdd_ShortAddr(acc,cc2,h->str,cur->ic_data(CCodeMisc*)->addr);
      break;
    }
    if(h->type&HTT_GLBL_VAR) {
      if((h(CHashGlblVar*)->flags&GVF_EXTERN)||cc->flags&CCF_AOT_COMPILE) {
        new=__HC_ICAdd_RelocUnique( //Unique because of ->offset
          acc,
          cc2,
          &cur->ic_data(CCodeMisc*)->addr,
          h->str,
          RT_U8,
          1
        );
      } else {
        new=__HC_ICAdd_I64(
          cc2,
          h(CHashGlblVar*)->data_addr
        );
      }
    } else if(h->type&HTT_FUN) {
      if((h(CHashFun*)->flags&(1<<Cf_EXTERN))||cc->flags&CCF_AOT_COMPILE) {
        new=__HC_ICAdd_Reloc(
            acc,
            cc2,
            &cur ->ic_data(CCodeMisc*)->addr,
            h->str,
            RT_U8,
            1
          );
      } else {
        new=__HC_ICAdd_I64(
          cc2,
          h(CHashFun*)->exe_addr
        );
      }
    } else if(!(h->type&HTF_RESOLVE)) {
        new=__HC_ICAdd_Reloc(
            acc,
            cc2,
            &cur ->ic_data(CCodeMisc*)->addr,
            h->str,
            RT_U8,
            1
          );
    } else {
        new=__HC_ICAdd_I64(
          cc2,
          h(CHashExport*)->val
	);
    }
    if(cc->flags&CCF_AOT_COMPILE) {
      if(h->type&HTT_GLBL_VAR&&h->type&HTF_EXPORT) {
        __HC_SetAOTRelocBeforeRIP(new,h(CHashGlblVar*)->data_addr_rip-cc->aotc->rip);
      } else if(h->type&HTT_FUN&&!Bt(&(h(CHashFun*)->flags),Cf_EXTERN)) {
        __HC_SetAOTRelocBeforeRIP(new,h(CHashFun*)->exe_addr-cc->aotc->rip);
      }
    }
    break;case IC_REG:
    if(cur->ic_dim) {
        new=__HC_ICAdd_IReg(cc2,cur->imm_i64,cmp.internal_types[RT_I64],0);
    } else {
      if(cur->ic_class->raw_type==RT_F64) {
        new=__HC_ICAdd_FReg(cc2,cur->imm_i64);
      } else {
        new=__HC_ICAdd_IReg(cc2,cur->imm_i64,cur->ic_class->raw_type,0);
      }
    }
    break;case IC_TO_I64:
    new=__HC_ICAdd_ToI64(cc2);
    break;case IC_TO_F64:
    new=__HC_ICAdd_ToF64(cc2);
    break;case IC_TO_BOOL:
    new=__HC_ICAdd_ToBool(cc2);
    break;case IC_TOUPPER:  //TODO
    break;case IC_HOLYC_TYPECAST:
    new=__HC_ICAdd_Typecast(cc2,cur->ic_class->raw_type,0);
    break;case IC_ADDR:
//Change of plans,I am making statics Relocations to be filled on the heap
    if(cur->next(CRPN*)->type!=IC_STATIC)
      new=__HC_ICAdd_Addr(cc2);
    break;case IC_COMMA:
    new=__HC_ICAdd_Comma(cc2);
    break;case IC_COM:
    new=__HC_ICAdd_BNot(cc2);
    break;case IC_NOT:
    new=__HC_ICAdd_LNot(cc2);
    break;case IC_UNARY_MINUS:
    new=__HC_ICAdd_Neg(cc2);
    break;case IC_SQRT:
    new=__HC_ICAdd_Sqrt(cc2);
    break;case IC_DEREF:
    new=__HC_ICAdd_Deref(cc2,cur->ic_class->raw_type,0);
    //break;case IC_DEREF_PP //NOT USED BY AIWINIOS
    //break;case IC_DEREF_MM //DITTO
    break;case IC__PP:
    new=__HC_ICAdd_PostInc(cc2,cur->ic_data);
    break;case IC__MM:
    new=__HC_ICAdd_PostDec(cc2,cur->ic_data);
    break;case IC_PP_:
    new=__HC_ICAdd_PreInc(cc2,cur->ic_data);
    break;case IC_MM_:
    new=__HC_ICAdd_PreDec(cc2,cur->ic_data);
    break;case IC_SHL:
    new=__HC_ICAdd_Lsh(cc2);
    break;case IC_SHR:
    new=__HC_ICAdd_Rsh(cc2);
    //break;case IC_SHL_CONST: NOT USED BY AIWNIOS
    //break;case IC_SHR_CONST: NOT USED BY AIWNIOS
    break;case IC_POWER:
    throw("Compiler");
    break;case IC_MUL:
    new=__HC_ICAdd_Mul(cc2);
    break;case IC_DIV:
    new=__HC_ICAdd_Div(cc2);
    break;case IC_MOD:
    new=__HC_ICAdd_Mod(cc2);
    break;case IC_AND:
    new=__HC_ICAdd_And(cc2);
    break;case IC_OR:
    new=__HC_ICAdd_Or(cc2);
    break;case IC_XOR:
    new=__HC_ICAdd_Xor(cc2);
    break;case IC_BT:
    new=__HC_ICAdd_BT(cc2);
    break;case IC_BTS:
    new=__HC_ICAdd_BTS(cc2);
    break;case IC_BTC:
    new=__HC_ICAdd_BTC(cc2);
    break;case IC_BTR:
    new=__HC_ICAdd_BTR(cc2);
    break;case IC_LBTS:
    new=__HC_ICAdd_LBTS(cc2);
    break;case IC_LBTC:
    new=__HC_ICAdd_LBTC(cc2);
    break;case IC_LBTR:
    new=__HC_ICAdd_LBTR(cc2);
    break;
    case IC_SQR:
	  new=__HC_ICAdd_Sqr(cc2);
	  break;
	break;case IC_ADD:
    new=__HC_ICAdd_Add(cc2);
    break;case IC_SUB:
    new=__HC_ICAdd_Sub(cc2);
    //break;case IC_ADD_CONST NOT USED BY AIWNIOS
    //break;case IC_SUB_CONST NOT USED BY AIWNIOS
    break;case IC_EQU_EQU:
    new=__HC_ICAdd_EqEq(cc2);
    break;case IC_NOT_EQU:
    new=__HC_ICAdd_Ne(cc2);
    break;case IC_LESS:
    new=__HC_ICAdd_Lt(cc2);
    break;case IC_GREATER_EQU:
    new=__HC_ICAdd_Ge(cc2);
    break;case IC_GREATER:
    new=__HC_ICAdd_Gt(cc2);
    break;case IC_LESS_EQU:
    new=__HC_ICAdd_Le(cc2);
    //break;case IC_PUSH_CMP NOT USED BY AIWNIOS
    break;case IC_AND_AND:
    new=__HC_ICAdd_AndAnd(cc2);
    break;case IC_OR_OR:
    new=__HC_ICAdd_OrOr(cc2);
    break;case IC_XOR_XOR:
    new=__HC_ICAdd_XorXor(cc2);
    break;case IC_ASSIGN:
    new=__HC_ICAdd_Eq(cc2);
    //break;case IC_ASSIGN_PP //NOT USED BY AIWNIOS
    //break;case IC_ASSIGN_MM //DITTO
    break;case IC_SHL_EQU:
    new=__HC_ICAdd_LshEq(cc2);
    break;case IC_SHR_EQU:
    new=__HC_ICAdd_RshEq(cc2);
    break;case IC_MUL_EQU:
    new=__HC_ICAdd_MulEq(cc2);
    break;case IC_DIV_EQU:
    new=__HC_ICAdd_DivEq(cc2);
    break;case IC_MOD_EQU:
    new=__HC_ICAdd_ModEq(cc2);
    break;case IC_AND_EQU:
    new=__HC_ICAdd_AndEq(cc2);
    break;case IC_OR_EQU:
    new=__HC_ICAdd_OrEq(cc2);
    break;case IC_XOR_EQU:
    new=__HC_ICAdd_XorEq(cc2);
    break;case IC_ADD_EQU:
    new=__HC_ICAdd_AddEq(cc2);
    break;case IC_SUB_EQU:
    new=__HC_ICAdd_SubEq(cc2);
    break;case IC_JMP:
    new=__HC_ICAdd_Goto(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_SUB_RET:
    new=__HC_ICAdd_SubRet(cc2);
    break;case IC_SUB_CALL:
    new=__HC_ICAdd_SubCall(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_SWITCH:
    misc=cur->ic_data(CCodeMisc*);
    __HC_ICAdd_BoundedSwitch(cc2,misc->addr,misc->dft->addr);
    break;case IC_NOBOUND_SWITCH:
    __HC_ICAdd_UnboundedSwitch(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_RET:
    new=__HC_ICAdd_Ret(cc2);
    break;case IC_CALL_INDIRECT:
    new=__HC_ICAdd_Call(
      cc2,
      cur->length-1,
      cur->ic_class->raw_type,
      0);
    /*TODO 
    IC_CALL
    IC_CALL_INDIRECT2
    IC_CALL_IMPORT
    IC_CALL_EXTERN
    */
    break;case IC_VARGS:
    new=__HC_ICAdd_Vargs(cc2,cur->ic_data);
    break;case IC_STATIC:   
    //HolyC side accounts for the ptr arithmetic for us
    new=__HC_ICAdd_Reloc(
          acc,
          cc2,
          &cur->ic_data(CCodeMisc*)->addr,
          stat_nam,
          RT_I64,
          0
    );
    if(cur->ic_data(CCodeMisc*)->offset) {
      misc=cur->ic_data;
      mlst=misc->offset; //See AIWNIOS_PrsExp.HC
      __HC_ICAdd_I64(cc2,mlst->offset);
      __HC_ICAdd_Add(cc2);
    }
    if(cur->last(CRPN*)->type!=IC_ADDR) {
      __HC_ICAdd_Deref(cc2,cur->ic_class->raw_type,0);
    }
  }
  if(cur->ic_line&&cur->ic_line>=cc->min_line) { //check for oddities(maybe includes accross a file boundary)
      reported_max_ln=MaxI64(reported_max_ln,cur->ic_line);
      reported_min_ln=MinI64(reported_min_ln,cur->ic_line);
      __HC_ICSetLine(new,cur->ic_line);
    }
    if(cur->ic_flags&ICF_LOCK)
	__HC_ICSetLock(new);
}
  if(reported_min_ln!=I16_MAX) {
    aiwnios_dbg_info=CAlloc(8*(reported_max_ln-reported_min_ln+2));
  } else 
	aiwnios_dbg_info=NULL;
  machine_code=__HC_Compile(acc,&idx,aiwnios_dbg_info,Fs->code_heap);

  if(cc->pass_trace&&cc->htc.fun) {
     U8 *dumb_dump=NULL;
     I64 dump_len=DolDocDumpIR(NULL,0,acc);
     dumb_dump=CAlloc(dump_len);
     DocPrint(DocPut,"$$GREEN$$FUNC::%s$$FD$$\n$$ID,2$$",cc->htc.fun->str);
     dump_len=DolDocDumpIR(dumb_dump,0,acc);
     DocPutS(DocPut,dumb_dump);
     DocPrint(DocPut,"$$ID,-2$$\n");
     Free(dumb_dump);
  }



  if(res_sz) *res_sz=idx;
  if(info&&aiwnios_dbg_info) {
    if(reported_min_ln!=I16_MAX&&reported_min_ln>=cc->min_line) {
      foff=reported_min_ln-cc->min_line+1;
      MemCpy(info[0]->body+foff,aiwnios_dbg_info,8*ClampI64(reported_max_ln-reported_min_ln-foff+1,0,MaxI64(cc->max_line-cc->min_line-foff-1,0)));
    }
    info[0]->body[0]=machine_code;
    info[0]->body[cc->max_line-cc->min_line+1]=idx+machine_code;
  }
  Free(aiwnios_dbg_info);
  
  //
  //Turn the CMT_HASH_ENTRY to CMT_LABEL to avoid freeing the CHash.
  //Things work different in AIWNIOS
  //
  // Also be sure to turn the relocations to CHashImport's
  //
  if(statics_sz&&!(cc->flags&CCF_AOT_COMPILE)) {
     dumb_static=CAlloc(statics_sz);
     mlst=cc->htc.fun->member_lst_and_root;
     for(idx=0;idx!=member_cnt;idx++) {
       if(mlst->flags&MLF_STATIC) {
	     if(mlst->static_data) {
		    MemCpy(dumb_static+mlst->offset,mlst->static_data,mlst->member_class->size*mlst->dim.total_cnt);
	     }
       }
       mlst=mlst->next;
     }
  }
  tmpgh=NULL;
  for(misc=cc->coc.coc_next_misc;misc!=&cc->coc.coc_next_misc;misc=misc->next) {
    if(misc->flags&CMF_STATIC) {
      if(misc->addr!=INVALID_PTR) {
	if(cc->flags&CCF_AOT_COMPILE) {
          if(!tmpgh) {
            tmpgh=CAlloc(sizeof CAOTHeapGlbl);
            tmpgh->size=statics_sz;
            tmpgh->str=StrNew(stat_nam);
            tmpgh->next=cc->aotc->heap_glbls;
            tmpgh->data=CAlloc(statics_sz);
            cc->aotc->heap_glbls=tmpgh;
            mlst=cc->htc.fun->member_lst_and_root;
			for(idx=0;idx!=member_cnt;idx++) {
			   if(mlst->flags&MLF_STATIC) {
				 if(mlst->static_data) {
					MemCpy(tmpgh->data+mlst->offset,mlst->static_data,mlst->member_class->size*mlst->dim.total_cnt);
				 }
			   }
			   mlst=mlst->next;
			 }
          }
          tmpghr=CAlloc(sizeof CAOTHeapGlblRef);
          tmpghr->next=tmpgh->references;
          tmpgh->references=tmpghr;
	      //tmpghr->offset=misc->offset;
          tmpghr->rip=cc->aotc->rip+(misc->addr(U8*)-machine_code);
	} else {
	      WriteProtectMemCpy(misc->addr,&dumb_static,8);
	}
      }
    } else if(misc->type==CMT_HASH_ENTRY) {
      misc->type=CMT_LABEL;
      //
      // AIWNIOS will always use IC_ADDR_IMPORT,but if we find a existing symbol
      //   to bind we use it's address (instead of a relocation).
      // So if we have relocations that aren't used (have an ->addr of INVALID_PTR)
      // dont add a "fill-in-later" for it
      //
      if(cc->flags&CCF_AOT_COMPILE) {
		if(misc->addr!=INVALID_PTR) {
          //All non-imported functions are short calls
          if(misc->flags&CMF_SHORT_ADDR) {
            fill_user_data.cc=cc;
	    fill_user_data.cm=misc;
	    fill_user_data.machine_code=machine_code;
	    __HC_CodeMiscInterateThroughRefs(misc->addr,&AddShortReloc,&fill_user_data);
          }else if(IsTLSReloc(misc)) {
            tmpaie=CAlloc(sizeof(CAOTImportExport));
            tmpaie->type=IET_IMM_I64;
            tmpaie->str=StrNew(misc->h->str);
            tmpaie->rip=cc->aotc->rip+(misc->addr(U8*)-machine_code);
            tmpaie->offset=misc->offset;
            QueIns(tmpaie,cc->aot->last_ie);
          } else if(misc->h->type&HTT_GLBL_VAR){
            fill_user_data.cc=cc;
	    fill_user_data.cm=misc;
	    fill_user_data.machine_code=machine_code;
	    if(!Globalify(misc->addr,&fill_user_data))
		  goto norm_reloc;
	  } else {
	  norm_reloc:
	    tmpaie=CAlloc(sizeof(CAOTImportExport));
            tmpaie->type=IET_IMM_I64;
            tmpaie->str=StrNew(misc->h->str);
            tmpaie->rip=cc->aotc->rip+(misc->addr(U8*)-machine_code);
            tmpaie->offset=misc->offset;
            QueIns(tmpaie,cc->aot->last_ie);
	  }
        }
      } else {
        if(misc->addr!=INVALID_PTR) {
          h=CAlloc(sizeof CHashImport);
          h->type=HTT_IMPORT_SYS_SYM;
          h->str=StrNew(misc->h->str);
          h->module_base=machine_code;
          //[reloc type 1][reloc offset 4][target-offset 4][name?]NULL]
          //Nroot adds an aditional byte to specify NULL after we load the silly sauce 
          h->module_header_entry=CAlloc(StrLen(h->str)+8+1+1);
          if(IsTLSReloc(misc))
	    h->module_header_entry[0]=IET_IMM_I64;
          else 
	    h->module_header_entry[0]=IET_IMM_I64;
          h->module_header_entry[1](U32)=misc->addr(U8*)-machine_code;
          h->module_header_entry[5](U32)=0;
          StrCpy(h->module_header_entry+8+1,h->str);
          HashAdd(h,Fs->hash_table);
        }
      }
    } else if(misc->addr!=INVALID_PTR&&misc->type==CMT_LABEL&&misc->str&&!StrNCmp("0x",misc->str,2)) {
      tmpabs=CAlloc(sizeof(CAOTAbsAddr));
      tmpabs->next=cc->aotc->abss;
      tmpabs->type=AAT_ADD_U64;
      tmpabs->rip=cc->aotc->rip+(misc->addr(U8*)-machine_code);
      cc->aotc->abss=tmpabs;	
      misc->addr(U64*)[0]=Str2I64(misc->str);
    } else if(misc->addr!=INVALID_PTR&&misc->type==CMT_JMP_TABLE&&cc->flags&CCF_AOT_COMPILE) {
      for(idx=0;idx<misc->range;idx++) {
        tmpabs=CAlloc(sizeof(CAOTAbsAddr));
        tmpabs->next=cc->aotc->abss;
        tmpabs->type=AAT_ADD_U64;
        table_ptr=misc->addr2(U8*)+8*idx;
        foff=cc->aotc->rip+(*table_ptr-machine_code(I64));
        WriteProtectMemCpy(table_ptr,&foff,8);
        tmpabs->rip=cc->aotc->rip+((misc->addr2(U8*)+8*idx)-machine_code);
        cc->aotc->abss=tmpabs;	
      }
    }
  }
  
  Free(stat_nam);
  __HC_CodeCtrlPop(acc);
  
  //
  // For CAiwniosAOTBlob,Fill in the relocations
  //
  for(cur=head->last;cur!=head;cur=cur->last)  {
    if(cur->live_in) BitSetDel(cur->live_in);
    if(cur->live_out) BitSetDel(cur->live_out);
    if(cur->use_regs) BitSetDel(cur->use_regs);
    if(cur->kill_regs) BitSetDel(cur->kill_regs);
    if(cur->user_data) I64SetDel(cur->user_data);
    if(cur->user_data2) I64SetDel(cur->user_data2);
    if(cur->type==IC_ASM) {
//TODO AOT
      JITFixupAiwniosAOTBlob(cc,cur->ic_data,cur->ic_data(CAiwniosAOTBlob*)->final_start_addr);
      AiwniosAOTBlobDel(cur->ic_data);
    }
  }
  __HC_CmpCtrlDel(acc);
  COCDel(cc,&cc->coc);
  return machine_code;
}

U8 *COCCompile(CCmpCtrl *cc,I64 *_code_size,CDbgInfo **_dbg,I64 *_type) {
  CRPN *rpn=cc->coc.coc_head.next;
  if(rpn->type==IC_RET&&_type)
    *_type=AssignRawTypeToNode(rpn->next)->raw_type;
  U8 *mc=AiwniosCompile(cc,_code_size,_dbg);
  return mc;
}

/**
 * Has format 
 * if(HTT_FUN)
 *     FUNC_NAME(U8[])
 *     FUNC_FILE(U8[])
 *     FUNC_HELP_IDX(U8[])
 *     U32 s,e;
 *     U32 line_offsets[]
 *     U32 stk_size
 *     U32 var_cnt
 *          VAR_NAME(U8[])
 *          VAR_TYPE(U8[])
 *          U32 reg
 *          U32 offset
 *          U32 star_cnt
 *          U32 total_dim //I64 array[3][4] would have total_dim of 12
 *
 * if(HTT_HELP_FILE)
 *     POINTS_TO_FILE(U8[])
 *     HELP_IDX(U8[])
 * ...
 */
I64 __DbgInfoWrite(CHashTable *h,U8 *to) {
  I64 i,ln,size=0,min,max;
  CHashSrcSym *tmph;
  CHashFun *,*pub_tmph;
  CHashClass *tmpc;
  CMemberLst *fun_mem;
  CDbgInfo *dbg_info;
  U32 start_ptr,cnt;
  U8 *src_link,buffer[1024],*bptr=&buffer;
  for (i=0;i<=h->mask;i++) {
    for(tmph=h->body[i];tmph;tmph=tmph->next) {
      if (tmph->src_link && !(tmph->type & (HTF_IMPORT | HTF_PRIVATE)) && (dbg_info=tmph->dbg_info)) {
        if((tmph->type&HTT_FUN)&&!(tmph(CHashFun*)->flags&(1<<Cf_EXTERN))) {
	  if(to) (to+size)(I64*)[0]=tmph->type&(HTT_FUN|HTF_PUBLIC);
	  size+=sizeof(I64);
	  StrScan(tmph->src_link,"FL:%s,%d",&bptr,&ln);
	  if(to) StrCpy(to+size,tmph->str);
	  size+=StrLen(tmph->str)+1;
	  if(to) StrCpy(to+size,buffer);
	  size+=StrLen(buffer)+1;
	  pub_tmph=HashFind(tmph->str,h,HTF_PUBLIC);
          if(pub_tmph) {
	    if(pub_tmph->idx) {
	      if(to) StrCpy(to+size,pub_tmph->idx);
	      size+=StrLen(pub_tmph->idx)+1;
	      goto wrote_help_idx;
	    }
	  }
	  if(tmph->idx) {
	    if(to) StrCpy(to+size,tmph->idx);
	    size+=StrLen(tmph->idx)+1;
	  } else {
	    if(to) StrCpy(to+size,"");
	    size+=StrLen("")+1;
	  }
wrote_help_idx:
	  min=dbg_info->min_line;
	  max=dbg_info->max_line;
	  if(to) (to+size)(U32*)[0]=min;
	  if(to) (to+size)(U32*)[1]=max;
	  size+=2*sizeof(U32);
	  max-=min;
	  min-=min;
	  start_ptr=dbg_info->body[0];
	  for(ln=0;ln<=max+1;ln++) {
      if(dbg_info->body[ln]) {
	      if(to) (to+size)(U32*)[0]=dbg_info->body[ln]-start_ptr;
	  } else {
	      if(to) (to+size)(U32*)[0]=0;
	  }
      size+=4;      
	  }
	  if(to) (to+size)(U32*)[0]=tmph(CHashFun*)->size;
	  size+=4;
	  fun_mem=tmph(CHashFun*)->member_lst_and_root;
	  for(cnt=0;fun_mem;fun_mem=fun_mem->next)
	    cnt++;
	  if(to) (to+size)(U32*)[0]=cnt;
	  size+=4;
	  fun_mem=tmph(CHashFun*)->member_lst_and_root;
	  for(;fun_mem;fun_mem=fun_mem->next) {
	    if(to) StrCpy(to+size,fun_mem->str);
	    size+=StrLen(fun_mem->str)+1;
            tmpc=fun_mem->member_class;
            tmpc-=tmpc->ptr_stars_cnt;
            if(tmpc->str) {
	      if(to) StrCpy(to+size,tmpc->str);
	      size+=StrLen(tmpc->str)+1;
            } else {
              if(to) to[size]=0;
              size++;
            }
	    if(to) (to+size)(U32*)[0]=fun_mem->reg;
	    size+=4;
	    if(to) (to+size)(U32*)[0]=fun_mem->offset;
	    size+=4;
            if(to) (to+size)(U32*)[0]=fun_mem->member_class->ptr_stars_cnt;
            size+=4;
            if(to) (to+size)(U32*)[0]=fun_mem->dim.total_cnt;
            size+=4;
	  }
        }
      } else if(tmph->type&HTT_HELP_FILE){
        if(to) (to+size)(I64*)[0]=HTT_HELP_FILE;
        size+=sizeof(I64);
        if(to) StrCpy(to+size,tmph->str);
        size+=StrLen(tmph->str)+1;
        if(to) StrCpy(to+size,tmph->idx);
        size+=StrLen(tmph->idx)+1;
      }
  }
  }
  return size;
}
U0 DbgInfoWrite(CHashTable *h,U8 *name) {
	I64 len=__DbgInfoWrite(h,NULL);
	U8 *buf=MAlloc(len);
	__DbgInfoWrite(h,buf);
	FileWrite(name,buf,len);
	Free(buf);
}
U0 DbgInfoRead(U8 *file) {
  I64 min,max,ln,type,len;
  CHashSrcSym *tmph;
  CMemberLst *tmpm=NULL,*tmpm2=NULL,*tmpm_start;
  CDbgInfo *dbg_info;
  CHashClass *tmpc;
  U8 *body=FileRead(file,&len),*ptr=body,*fn,*idx;
  while(ptr<body+len) {
    type=ptr(I64*)[0];
    ptr+=8;
    if(type&HTT_FUN) {
      tmph=HashFind(ptr,Fs->hash_table,HTT_FUN);
      ptr+=StrLen(ptr)+1;
      fn=ptr;
      ptr+=StrLen(ptr)+1;
      idx=ptr;
      ptr+=StrLen(ptr)+1;
      min=ptr(U32*)[0];
      max=ptr(U32*)[1];
      ptr+=2*sizeof(U32);
      if(tmph&&tmph->type&HTT_FUN) {
        tmph->type|=type; //type may include HTF_PUBLIC
        Free(tmph->src_link);
        Free(tmph->idx);
        if(StrLen(idx))
	  tmph->idx=StrNew(idx);
        else
	  tmph->idx=NULL;
        tmph->src_link=MStrPrint("FL:%s,%d",fn,min);
        tmph->dbg_info=CAlloc(sizeof(CDbgInfo)+sizeof(I64)*(max-min+1));
        tmph->dbg_info->min_line=min;
        tmph->dbg_info->max_line=max;
        for(ln=min;ln<=max+1;ln++) {
	  tmph->dbg_info->body[ln-min]=tmph(CHashFun*)->exe_addr(U8*)+ptr(U32*)[ln-min];
	  if(ln-min&&tmph->dbg_info->body[ln-min]==tmph(CHashFun*)->exe_addr)
	    tmph->dbg_info->body[ln-min]=NULL;
        }
      }
      ptr+=sizeof(U32)*(max-min+1+1);
      if(tmph) tmph(CHashFun*)->size=ptr(I32*)[0];
      ptr+=sizeof(U32);
      max=ptr(U32*)[0];
      ptr+=sizeof(U32);
      tmpm=NULL;
      tmpm2=NULL;
      tmpm_start=NULL;
      for(min=0;min!=max;min++) {
        if(tmph) tmpm=CAlloc(sizeof(CMemberLst));
        if(tmph) tmpm->str=StrNew(ptr);
        ptr+=StrLen(ptr)+1;
        tmpc=HashFind(ptr,Fs->hash_table,HTT_CLASS|HTT_INTERNAL_TYPE);
        if(tmph) tmpm->member_class=tmpc;
        ptr+=StrLen(ptr)+1;
        //If we couldn't find a class,continue
        if(!tmpc) {
          if(tmpm) Free(tmpm->str);
          Free(tmpm);
          ptr+=16;
          goto next;
        } 
	if(tmph) tmpm->reg=ptr(U32*)[0];
	ptr+=4;
        if(tmph) tmpm->offset=ptr(I32*)[0];
        ptr+=4;
        if(tmpm) tmpm->member_class+=ptr(U32*)[0]; //In TempleOS,the next class is a "pointer star"
        ptr+=4;
        if(tmpm) tmpm->dim.total_cnt=tmpm->dim.cnt=ptr(U32*)[0];
        ptr+=4;
        if(!tmpm_start) tmpm_start=tmpm;
        if(tmpm2) tmpm2->next=tmpm;
        tmpm2=tmpm;
        next:;
      }
      if(tmph) {
        tmpm=tmph(CHashFun*)->member_lst_and_root;
        for(;tmpm&&tmpm->next;tmpm=tmpm->next);
        if(tmpm)
	  tmpm->next=tmpm_start;
        else
	  tmph(CHashFun*)->member_lst_and_root=tmpm_start;
      }
    } else if(type&HTT_HELP_FILE) {
      tmph=CAlloc(sizeof(CHashSrcSym));
      tmph->type=HTF_PUBLIC|HTT_HELP_FILE;
      tmph->str=StrNew(ptr);
      ptr+=StrLen(ptr)+1;
      tmph->idx=StrNew(ptr);
      ptr+=StrLen(ptr)+1;
      HashAdd(tmph,Fs->hash_table);
    }
  }
}
