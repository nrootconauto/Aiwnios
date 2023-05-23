//
// Binds the silly sauce from parser.c to HolyC. AIWNIOS is built so when
//  you write a backend,HolyC will use it too,that way you dont have to re-port
//  your backend again when you port to another arch
//
#ifdef TARGET_X86_SYSV
#define TARGET_X86 1
#endif
#ifdef TARGET_X86_WIN
#define TARGET_X86 1
#endif
#ifdef IMPORT_AIWNIOS_SYMS
import U0 __HC_SetAOTRelocBeforeRIP(U8 *,I64); //See aiwn.h(TAKES number of bytes before current RIP the symbol is located at)
import U8 *__HC_CmpCtrlDel(U8 *);
import U8 *__HC_ICAdd_ToF64(U8 *);
import U8 *__HC_ICAdd_ToI64(U8 *);
import U0 __HC_ICSetLine(U8*,I64);
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
import U8 * __HC_ICAdd_Eq(U8*);
import U8 * __HC_ICAdd_Div(U8*);
import U8 * __HC_ICAdd_Sub(U8*);
import U8 * __HC_ICAdd_Mul(U8*);
import U8 * __HC_ICAdd_Add(U8*);
import U8 * __HC_ICAdd_Deref(U8*,I64 rt,I64 ptrs);
import U8 * __HC_ICAdd_Comma(U8*);
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
import U8 *__HC_CodeMiscLabelNew(U8*);
import U8 *__HC_CmpCtrlNew();
import U8 *__HC_CodeCtrlPush(U8*);
import U0 __HC_CodeCtrlPop(U8*);
import U8 *__HC_Compile(U8*,I64 *sz=NULL,U8 **dbg_info=NULL);
import U8 *__HC_CodeMiscLabelNew(U8 *);
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


#else
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
extern U8 * __HC_ICAdd_Vargs(U8 *,I64);
extern U8 * __HC_ICAdd_IReg(U8*,I64 r,I64 rt,I64 ptrs);
extern U8 * __HC_ICAdd_Frame(U8*,I64 off,I64 rt,I64 ptrs);
extern U8 *__HC_CodeMiscJmpTableNew(U8 *,U8 **cmts,I64 lo,I64 hi);
extern U8 *__HC_CodeMiscStrNew(U8*,U8*,I64 sz);
extern U8 *__HC_CodeMiscLabelNew(U8*);
extern U8 *__HC_CmpCtrlNew();
extern U8 *__HC_CodeCtrlPush(U8*);
extern U0 __HC_CodeCtrlPop(U8*);
extern U8 *__HC_Compile(U8*,I64 *sz=NULL,U8 **dbg_info=NULL);
extern U8 *__HC_CodeMiscLabelNew(U8 *);
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
#endif
//
// See arm_backend.c
//
#ifdef TARGET_AARCH64
#define AIWNIOS_IREG_START 19
#define AIWNIOS_IREG_CNT (28 - 19 + 1)
#define AIWNIOS_FREG_START 8
#define AIWNIOS_FREG_CNT (15 - 8 + 1)
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
#endif
#ifdef TARGET_X86_WIN
#define AIWNIOS_IREG_START 9
#define AIWNIOS_IREG_CNT 6
#define AIWNIOS_REG_FP RBP
#define AIWNIOS_REG_SP RSP
#define AIWNIOS_TMP_IREG_POOP R8
#define AIWNIOS_TMP_IREG_POOP2 RCX
#define AIWNIOS_TMP_IREG_START 0
#define AIWNIOS_TMP_IREG_CNT 3
#define AIWNIOS_FREG_START 6
#define AIWNIOS_FREG_CNT (15-6+1)
#define AIWNIOS_TMP_FREG_START 3
#define AIWNIOS_TMP_FREG_CNT (5-AIWNIOS_TMP_FREG_START+1)
#endif
#ifdef TARGET_X86_SYSV
#define AIWNIOS_IREG_START 12
#define AIWNIOS_IREG_CNT 4
#define AIWNIOS_TMP_IREG_CNT 3
#define AIWNIOS_REG_FP REG_RBP
#define AIWNIOS_REG_SP REG_RSP
#define AIWNIOS_TMP_IREG_START 0
#define AIWNIOS_FREG_START 0
#define AIWNIOS_FREG_CNT 0
#define AIWNIOS_TMP_FREG_START 2
#define AIWNIOS_TMP_FREG_CNT 14
#endif
//
// Replaces IC_CALL_INDIRECT(IC_ADDR_IMPORT) with Short jumps if not import
//
U0 AiwniosMakeShortJmps(CCmpCtrl *cc) {
  CRPN *head=&cc->coc.coc_head,*cur,*next;
  CCodeMisc *misc;
  for(cur=head->next;cur!=head;cur=cur->next) {
    if(cur->type==IC_CALL_INDIRECT) {
      if((next=RPNArgN(cur,cur->length-1))->type==IC_ADDR_IMPORT) {
        misc=next->ic_data;
        if((misc->h->type&HTT_FUN)&&!(misc->h->type&HTF_IMPORT)) {
          next->misc=COCMiscNew(cc,CMT_HASH_ENTRY);
          next->misc->str=StrNew(misc->h->str);
          next->misc->flags|=CMF_SHORT_ADDR;
          next->misc->h=misc->h;
          next->ic_data=next->misc;
          next->ic_flags|=ICF_SHORT_JMP;
        }
      }
    }
  }
}
I64 VarIRegToReg(I64 r)  {
	switch(r) {
		break;case 0: return REG_RBX;
		break;case 1: return REG_R12;
		break;case 2: return REG_R14;
		break;case 3: return REG_R15;
		#ifdef TARGET_X86_WIN
		break;case 4: return REG_RDI;
		break;case 5: return REG_RSI;
		#endif
	}
	throw('Compile');
}
//
// Turns my CRPN from the parser into the IC for the C side
//
U8 *AiwniosCompile(CCmpCtrl *cc,I64 *res_sz=NULL,CDbgInfo **info) {
  if(cc->flags&CCF_AOT_COMPILE)
    AiwniosMakeShortJmps(cc);
  CRPN *head=&cc->coc.coc_head,*cur,*new;
  CCodeMisc *misc,**table;
  CMemberLst *mlst;
  COptMemberVar *mv=NULL;
  CHashImport *h;
  CAOTImportExport *tmpaie;
  CAOTAbsAddr *tmpabs;
  I64 idx,argc,member_cnt=0,ir=AIWNIOS_IREG_START,fr=AIWNIOS_FREG_START,foff,*foff_lookup=NULL;
  U8 **aiwnios_dbg_info=NULL;
  U8 *acc=__HC_CmpCtrlNew(),*machine_code;
  U8 *cc2=__HC_CodeCtrlPush(acc);
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
    member_cnt=cc->htc.fun->member_cnt;
    mlst=cc->htc.fun->member_lst_and_root;
    if(cc->htc.fun->flags&(1<<Ff_DOT_DOT_DOT)) {
      member_cnt+=2;
    }
    mv=CAlloc(member_cnt*sizeof COptMemberVar);
    foff_lookup=CAlloc(member_cnt*sizeof I64);
    for(idx=0;idx!=member_cnt;idx++) {
      mv[idx].m=mlst;
      mlst=mlst->next;
    }
    for(cur=head->last;cur!=head;cur=cur->last) {
      if(cur->type==IC_FRAME) {
        mlst=cc->htc.fun->member_lst_and_root;
        for(idx=0;idx!=member_cnt;idx++) {
          if(cur->ic_data==mv[idx].m->offset) {
            if(cur->last(CRPN*)->type==IC_ADDR) {
              mv[idx].addrof_cnt++;
              mv[idx].score=0;
            }
            if(!mv[idx].addrof_cnt)
              mv[idx].score++;
            break;
          }
          mlst=mlst->next;
        }
      }
    }
    //Sort by score
    QSort(mv,member_cnt,sizeof(COptMemberVar),&OptMVCompare);
    for(idx=0;idx!=member_cnt;idx++) {
      if(mv[idx].score&&!mv[idx].m->dim.next) {
        if(mv[idx].m->member_class->raw_type==RT_F64) {
          if(AIWNIOS_FREG_START<=fr<AIWNIOS_FREG_START+AIWNIOS_FREG_CNT) {
            mv[idx].m->reg=fr++;
          } else
            mv[idx].m->reg=REG_NONE;
        } else {
          if(AIWNIOS_IREG_START<=ir<AIWNIOS_IREG_START+AIWNIOS_IREG_CNT) {
            mv[idx].m->reg=VarIRegToReg(ir++ -AIWNIOS_IREG_START);
          } else {
            mv[idx].m->reg=REG_NONE;
          }
        }
      } else
        mv[idx].m->reg=REG_NONE;
    }
    
    //Assign registers
    for(cur=head->last;cur!=head;cur=cur->last) {
      if(cur->type==IC_FRAME) {
        for(idx=0;idx!=member_cnt;idx++) {
          if(cur->ic_data==mv[idx].m->offset) {          
            if(mv[idx].m->reg!=REG_NONE) {
              cur->type=IC_REG;
              cur->ic_data=mv[idx].m->reg;
            }
            break;
          }
        }
      }
    }
    //In FunRep,the function is assumes to store registers somewhere
    //So i "store" them at the start of the function
    foff=0;
    for(idx=0;idx!=member_cnt;idx++) {
      if(mv[idx].m->reg!=REG_NONE) {
        mv[idx].m->offset=foff;
        foff+=8;
      }
    }
    //Assign offsets
    for(idx=0;idx!=member_cnt;idx++) {
      mlst=mv[idx].m;
      if(mlst->reg==REG_NONE) {
        #ifdef TARGET_AARCH64
        foff_lookup[idx].i32[1]=foff;
        foff_lookup[idx].i32[0]=mlst->offset;
        mlst->offset=foff;
        foff+=mlst->member_class->size*mlst->dim.total_cnt;  
        if(foff&7)
          foff+=8-(foff&7); //Align to 8#endifg
        #endif
        #ifdef TARGET_X86
        foff_lookup[idx].i32[0]=mlst->offset;
        //In X86_64 the stack grows down and the RBP is above the stack pointer,
        //SO MOVE THE OFFSETS DOWN TO THE BOTTOM
        //RBP
        //x0
        //x1
        //....
        //x8 <<=offset
        foff+=mlst->member_class->size*mlst->dim.total_cnt;  
        if(foff&7)
          foff+=8-(foff&7); //Align to 8
        mlst->offset=foff;
        foff_lookup[idx].i32[1]=foff;
        #endif
      }
    }
    
    cc->htc.fun->size=foff;
  }
  
  //Replace the old frame address with the new one 
  for(cur=head->last;cur!=head;cur=cur->last)
    if(cur->type==IC_FRAME)
      for(idx=0;idx!=member_cnt;idx++) {
        foff=foff_lookup[idx].i32[0];
        if(foff==cur->ic_data) {
          cur->ic_data=foff_lookup[idx].u32[1];
          break;
        }
      }
  Free(foff_lookup);
  
  //
  // Step 1.5,Assign the variables from the arguments
  //
  if(cc->htc.fun) {
    __HC_ICAdd_SetFrameSize(cc2,cc->htc.fun->size+16);
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
      __HC_ICAdd_Arg(cc2,idx);
      mlst=mlst->next;
    }
    Free(mv);
  } else {
    //
    // Add MANDITORY room for FP/LR pair
    //
    __HC_ICAdd_SetFrameSize(cc2,16);
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
      misc->addr=__HC_CodeMiscLabelNew(acc);
    break;case CMT_GOTO_LABEL:
      misc->addr=__HC_CodeMiscLabelNew(acc);
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
      misc->addr=__HC_CodeMiscJmpTableNew(acc,table,0,misc->range);
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
    foff_lookup=CAlloc(2*member_cnt*8);
    mlst=cc->htc.fun->member_lst_and_root;
    for(idx=0;idx!=member_cnt;idx++) {
      if(mlst->flags&MLF_STATIC) {
        __HC_ICAdd_StaticData(acc,cc2,foff,mlst->static_data,mlst->member_class->size*mlst->dim.total_cnt);
        foff_lookup[idx*2]=mlst->static_data;
        foff_lookup[idx*2+1]=foff;
        foff+=mlst->member_class->size*mlst->dim.total_cnt;
        if(foff%8)
          foff+=8-foff%8;
      }
      mlst=mlst->next;
    }
    if(foff) {
      for(cur=head->last;cur!=head;cur=cur->last) {
        if(cur->type==IC_STATIC) {
          mlst=cc->htc.fun->member_lst_and_root;
          for(idx=0;idx!=member_cnt;idx++) {
            if(foff_lookup[idx*2]==cur->ic_data) {
              cur->ic_data=foff_lookup[idx*2+1];
              break;
            }
            mlst=mlst->next;
          }
        }
      }
      __HC_ICAdd_SetStaticsSize(cc2,foff);
    }
    Free(foff_lookup);
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
  // Step 5,convert the CRPN's into AIWNIOS rpns
  //
  for(cur=head->last;cur!=head;cur=cur->last) {
    switch(cur->type) {
    break;case IC_FRAME:
    if(cur->last(CRPN*)->type==IC_ADDR) {
      //Our HolyC part accounts for the pointers for us
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,RT_U8,0);
    } else if(cur->ic_class->ptr_stars_cnt)
      //All pointer arithmetic is pre-computed
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,RT_PTR,0);
    else
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,cur->ic_class->raw_type,0);
    break;case IC_BR_ZERO:
    new=__HC_ICAdd_LNot(cc2);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_NOT_ZERO:
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    //break;case IC_BR_CARRY: ???
    //break;case IC_BR_NOT_CARRY: ???
    break;case IC_BR_EQU_EQU:
    new=__HC_ICAdd_EqEq(cc2);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_NOT_EQU:
    new=__HC_ICAdd_Ne(cc2);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_LESS:
    new=__HC_ICAdd_Lt(cc2);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_GREATER_EQU:
    new=__HC_ICAdd_Ge(cc2);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_GREATER:
    new=__HC_ICAdd_Gt(cc2);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr);
    break;case IC_BR_LESS_EQU:
    new=__HC_ICAdd_Le(cc2);
    __HC_ICSetLine(new,cur->ic_line);
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
      new=__HC_ICAdd_ShortAddr(acc,cc2,h->str,&cur->ic_data(CCodeMisc*)->addr);
      break;
    }
    if(h->type&HTT_GLBL_VAR) {
      if((h(CHashGlblVar*)->flags&GVF_EXTERN)||cc->flags&CCF_AOT_COMPILE) {
        new=__HC_ICAdd_Reloc(
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
    }
    if(cc->flags&CCF_AOT_COMPILE) {
      if(h->type&HTT_GLBL_VAR&&h->type&HTF_EXPORT) {
        __HC_SetAOTRelocBeforeRIP(new,h(CHashGlblVar*)->data_addr_rip-cc->aotc->rip);
      } else if(h->type&HTT_FUN&&!Bt(&(h(CHashFun*)->flags),Cf_EXTERN)) {
        __HC_SetAOTRelocBeforeRIP(new,h(CHashFun*)->exe_addr-cc->aotc->rip);
      }
    }
    break;case IC_REG:
    if(cur->ic_class->raw_type==RT_F64) {
      new=__HC_ICAdd_FReg(cc2,cur->imm_i64);
    } else {
      new=__HC_ICAdd_IReg(cc2,cur->imm_i64,cur->ic_class->raw_type,0);
    }
    break;case IC_TO_I64:
    new=__HC_ICAdd_ToI64(cc2);
    break;case IC_TO_F64:
    new=__HC_ICAdd_ToF64(cc2);
    break;case IC_TO_BOOL: //TODO
    break;case IC_TOUPPER:  //TODO
    break;case IC_HOLYC_TYPECAST:
    new=__HC_ICAdd_Typecast(cc2,cur->ic_class->raw_type,0);
    break;case IC_ADDR:
    new=__HC_ICAdd_Addr(cc2);
    break;case IC_COMMA:
    new=__HC_ICAdd_Comma(cc2);
    break;case IC_COM:
    new=__HC_ICAdd_BNot(cc2);
    break;case IC_NOT:
    new=__HC_ICAdd_LNot(cc2);
    break;case IC_UNARY_MINUS:
    new=__HC_ICAdd_Neg(cc2);
    break;case IC_DEREF:
    if(cur->ic_class->raw_type==RT_PTR)
      new=__HC_ICAdd_Deref(cc2,cur->ic_class->raw_type,0);
    else
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
    new=__HC_ICAdd_Pow(cc2);
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
    if(cur->last(CRPN*)->type==IC_ADDR)
      new=__HC_ICAdd_StaticRef(cc2,cur->ic_data,RT_U8,0);
    else
      new=__HC_ICAdd_StaticRef(cc2,cur->ic_data,cur->ic_class->raw_type,0);
    }
    __HC_ICSetLine(new,cur->ic_line);
  }
  aiwnios_dbg_info=CAlloc(8*(cc->max_line-cc->min_line+1));
  machine_code=__HC_Compile(acc,res_sz,aiwnios_dbg_info);
  __HC_CodeCtrlPop(acc);
  if(info) {
    info[0]->body[0]=machine_code;
    MemCpy(&info[0]->body[1],aiwnios_dbg_info,8*(cc->max_line-cc->min_line+1));
  }
  Free(aiwnios_dbg_info);
  
  //
  //Turn the CMT_HASH_ENTRY to CMT_LABEL to avoid freeing the CHash.
  //Things work different in AIWNIOS
  //
  // Also be sure to turn the relocations to CHashImport's
  //
  for(misc=cc->coc.coc_next_misc;misc!=&cc->coc.coc_next_misc;misc=misc->next) {
    if(misc->type==CMT_HASH_ENTRY) {
      misc->type=CMT_LABEL;
      //
      // AIWNIOS will always use IC_ADDR_IMPORT,but if we find a existing symbol
      //   to bind we use it's address (instead of a relocation).
      // So if we have relocations that aren't used (have an ->addr of INVALID_PTR)
      // dont add a "fill-in-later" for it
      //
      if(cc->flags&CCF_AOT_COMPILE) {
		if(misc->addr!=INVALID_PTR) {
          tmpaie=CAlloc(sizeof(CAOTImportExport));
          tmpaie->type=IET_IMM_I64;
          tmpaie->str=StrNew(misc->h->str);
          tmpaie->rip=cc->aotc->rip+(misc->addr(U8*)-machine_code);
		  //All non-imported functions are short calls
          if(misc->flags&CMF_SHORT_ADDR) {
            tmpaie->short_jmp=TRUE;
          }
          QueIns(tmpaie,cc->aot->last_ie);
        }
      } else {
        if(misc->addr!=INVALID_PTR) {
          h=CAlloc(sizeof CHashImport);
          h->type=HTT_IMPORT_SYS_SYM;
          h->str=StrNew(misc->h->str);
          h->module_base=machine_code;
          //[reloc type 1][reloc offset 4][name?]NULL]
          //Nroot adds an aditional byte to specify NULL after we load the silly sauce 
          h->module_header_entry=CAlloc(StrLen(h->str)+5+1+1);
          h->module_header_entry[0]=IET_IMM_I64;
          h->module_header_entry[1](U32)=misc->addr(U8*)-machine_code;
          StrCpy(h->module_header_entry+5,h->str);
          HashAdd(h,Fs->hash_table);
        }
      }
    } else if(misc->addr!=INVALID_PTR&&misc->type==CMT_LABEL&&misc->str&&!StrNCmp("0x",misc->str,2)) {
      tmpabs=CAlloc(sizeof(CAOTAbsAddr));
      tmpabs->next=cc->aotc->abss;
      tmpabs->type=AAT_ADD_U64;
      tmpabs->rip=cc->aotc->rip+(misc->addr(U8*)-machine_code);
      cc->aotc->abss=tmpabs;	
      idx=misc->addr(U64*)[0]=Str2I64(misc->str);
    }
  }
  __HC_CmpCtrlDel(acc);
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
	    } else
	      if(to) (to+size)(U32*)[0]=0;
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
U0 DbgInfoRead(U8 *name) {
  I64 len,min,max,ln,type;
  CHashSrcSym *tmph;
  CMemberLst *tmpm=NULL,*tmpm2=NULL;
  CDbgInfo *dbg_info;
  CHashClass *tmpc;
  U8 *body=FileRead(name,&len),*ptr=body,*fn,*idx;
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
      for(min=0;min!=max;min++) {
        tmpm2=tmpm;
        if(tmph) tmpm=CAlloc(sizeof(CMemberLst));
        if(tmpm2) tmpm2->next=tmpm;
        if(tmph) tmpm->str=StrNew(ptr);
        ptr+=StrLen(ptr)+1;
        tmpc=HashFind(ptr,Fs->hash_table,HTT_CLASS|HTT_INTERNAL_TYPE);
        if(tmph) tmpm->member_class=tmpc;
        ptr+=StrLen(ptr)+1;
        //If we couldn't find a class,continue
        if(!tmpc) {
          if(tmpm) Free(tmpm->str);
          Free(tmpm);
          if(tmpm2) tmpm2->next=NULL;
          tmpm=tmpm2;
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
        next:;
      }
      if(tmph) {
        tmpm=tmph(CHashFun*)->member_lst_and_root;
        for(;tmpm&&tmpm->next;tmpm=tmpm->next);
        if(tmpm)
	  tmpm->next=tmpm2;
        else
	  tmph(CHashFun*)->member_lst_and_root=tmpm2;
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
  Free(body);
}

