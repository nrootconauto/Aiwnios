//
// Binds the silly sauce from parser.c to HolyC. AIWNIOS is built so when
//  you write a backend,HolyC will use it too,that way you dont have to re-port
//  your backend again when you port to another arch
//
#ifdef IMPORT_AIWNIOS_SYMS
import U8 *__HC_ICAdd_ToF64(U8 *,CHeapCtrl*);
import U8 *__HC_ICAdd_ToI64(U8 *,CHeapCtrl*);
import U0 __HC_ICSetLine(U8*,I64);
import U8 *__HC_ICAdd_Typecast(U8 *,I64,I64,CHeapCtrl*);
import U8 *__HC_ICAdd_SubCall(U8 *,U8 *,CHeapCtrl*);
import U8 *__HC_ICAdd_SubProlog(U8 *,CHeapCtrl*);
import U8 *__HC_ICAdd_SubRet(U8 *,CHeapCtrl*);
import U8 *__HC_ICAdd_BoundedSwitch(U8*,U8*,U8*,CHeapCtrl*);
import U8 *__HC_ICAdd_UnboundedSwitch(U8 *,U8 *,CHeapCtrl *);
import U8 *__HC_ICAdd_PreInc(U8 *,I64,CHeapCtrl *);
import U8 *__HC_ICAdd_Call(U8*,I64,I64 rt,I64 ptrs,CHeapCtrl *);
import U8 *__HC_ICAdd_F64(U8*,F64,CHeapCtrl *);
import U8 *__HC_ICAdd_I64(U8*,I64,CHeapCtrl *);
import U8 * __HC_ICAdd_PreDec(U8*,I64,CHeapCtrl *);
import U8 * __HC_ICAdd_PostDec(U8*,I64,CHeapCtrl *);
import U8 * __HC_ICAdd_PostInc(U8*,I64,CHeapCtrl *);
import U8 * __HC_ICAdd_Pow(U8 *,CHeapCtrl*);
import U8 * __HC_ICAdd_Eq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Div(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Sub(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Mul(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Add(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Deref(U8*,I64 rt,I64 ptrs,CHeapCtrl *);
import U8 * __HC_ICAdd_Comma(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Addr(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Xor(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Mod(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Or(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Lt(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Gt(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Le(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Ge(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_LNot(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_BNot(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_AndAnd(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_And(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_OrOr(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_XorXor(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Ne(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_EqEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Lsh(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_Rsh(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_AddEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_SubEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_MulEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_DivEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_LshEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_RshEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_AndEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_OrEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_XorEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_ModEq(U8*,CHeapCtrl *);
import U8 * __HC_ICAdd_FReg(U8*,I64,CHeapCtrl *);
import U8 * __HC_ICAdd_Vargs(U8 *,I64,CHeapCtrl *);
import U8 * __HC_ICAdd_IReg(U8*,I64 r,I64 rt,I64 ptrs,CHeapCtrl *);
import U8 * __HC_ICAdd_Frame(U8*,I64 off,I64 rt,I64 ptrs,CHeapCtrl *);
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
import U8 *__HC_ICAdd_Label(U8*,U8*,CHeapCtrl*);
import U8 *__HC_ICAdd_Goto(U8*,U8*,CHeapCtrl*);
import U8 *__HC_ICAdd_GotoIf(U8*,U8*,CHeapCtrl*)
import U8 *__HC_ICAdd_Str(U8*,U8*,CHeapCtrl*); //Takes a AIWNIOS code misc
import U8 *__HC_ICAdd_Neg(U8*,CHeapCtrl*);
import U8 *__HC_ICAdd_Ret(U8*,CHeapCtrl*);
import U8 *__HC_ICAdd_Arg(U8*,I64,CHeapCtrl *);
import U8 *__HC_ICAdd_SetFrameSize(U8*,I64,CHeapCtrl *);
import U8 *__HC_ICAdd_Reloc(U8 *ccmp,U8 *cc,U64*,U8 *sym,I64 rt,I64 ptrs,CHeapCtrl *ctrl);
import U8 *__HC_ICAdd_StaticData(U8 *ccmp,U8 *cc,I64 at,U8 *d,I64 l,CHeapCtrl *ctrl);
import U8 *__HC_ICAdd_SetStaticsSize(U8 *,I64 sz,CHeapCtrl *ctrl);
import U8 *__HC_ICAdd_StaticRef(U8 *,I64 off,I64 rt,I64 ptrs,CHeapCtrl *ctrl);
#else
extern U8 * __HC_ICAdd_ToF64(U8 *,CHeapCtrl*);
extern U8 * __HC_ICAdd_ToI64(U8 *,CHeapCtrl*);
extern U0 __HC_ICSetLine(U8*,I64);
extern U8 * __HC_ICAdd_Typecast(U8 *,I64,I64,CHeapCtrl*);
extern U8 * __HC_ICAdd_SubCall(U8 *,U8 *,CHeapCtrl*);
extern U8 * __HC_ICAdd_SubProlog(U8 *,CHeapCtrl*);
extern U8 * __HC_ICAdd_SubRet(U8 *,CHeapCtrl*);
extern U8 * __HC_ICAdd_BoundedSwitch(U8*,U8*,U8*,CHeapCtrl*);
extern U8 * __HC_ICAdd_UnboundedSwitch(U8 *,U8 *,CHeapCtrl *);
extern U8 * __HC_ICAdd_PreInc(U8 *,I64,CHeapCtrl *);
extern U8 * __HC_ICAdd_Call(U8*,I64,I64 rt,I64 ptrs,CHeapCtrl *);
extern U8 * __HC_ICAdd_F64(U8*,F64,CHeapCtrl *);
extern U8 * __HC_ICAdd_I64(U8*,I64,CHeapCtrl *);
extern U8 * __HC_ICAdd_PreDec(U8*,I64,CHeapCtrl *);
extern U8 * __HC_ICAdd_PostDec(U8*,I64,CHeapCtrl *);
extern U8 * __HC_ICAdd_PostInc(U8*,I64,CHeapCtrl *);
extern U8 * __HC_ICAdd_Pow(U8 *,CHeapCtrl*);
extern U8 * __HC_ICAdd_Eq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Div(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Sub(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Mul(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Add(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Deref(U8*,I64 rt,I64 ptrs,CHeapCtrl *);
extern U8 * __HC_ICAdd_Comma(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Addr(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Xor(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Mod(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Or(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Lt(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Gt(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Le(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Ge(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_LNot(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_BNot(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_AndAnd(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_And(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_OrOr(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_XorXor(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Ne(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_EqEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Lsh(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_Rsh(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_AddEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_SubEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_MulEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_DivEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_LshEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_RshEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_AndEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_OrEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_XorEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_ModEq(U8*,CHeapCtrl *);
extern U8 * __HC_ICAdd_FReg(U8*,I64,CHeapCtrl *);
extern U8 * __HC_ICAdd_Vargs(U8 *,I64,CHeapCtrl *);
extern U8 * __HC_ICAdd_IReg(U8*,I64 r,I64 rt,I64 ptrs,CHeapCtrl *);
extern U8 * __HC_ICAdd_Frame(U8*,I64 off,I64 rt,I64 ptrs,CHeapCtrl *);
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
extern U8 *__HC_ICAdd_Label(U8*,U8*,CHeapCtrl*);
extern U8 *__HC_ICAdd_Goto(U8*,U8*,CHeapCtrl*);
extern U8 *__HC_ICAdd_GotoIf(U8*,U8*,CHeapCtrl*)
extern U8 *__HC_ICAdd_Str(U8*,U8*,CHeapCtrl*); //Takes a AIWNIOS code misc
extern U8 *__HC_ICAdd_Neg(U8*,CHeapCtrl*);
extern U8 *__HC_ICAdd_Ret(U8*,CHeapCtrl*);
extern U8 *__HC_ICAdd_Arg(U8*,I64,CHeapCtrl *);
extern U8 *__HC_ICAdd_SetFrameSize(U8*,I64,CHeapCtrl *);
extern U8 *__HC_ICAdd_Reloc(U8 *ccmp,U8 *cc,U64*,U8 *sym,I64 rt,I64 ptrs,CHeapCtrl *ctrl);
extern U8 *__HC_ICAdd_StaticData(U8 *ccmp,U8 *cc,I64 at,U8 *d,I64 l,CHeapCtrl *ctrl);
extern U8 *__HC_ICAdd_SetStaticsSize(U8 *,I64 sz,CHeapCtrl *ctrl);
extern U8 *__HC_ICAdd_StaticRef(U8 *,I64 off,I64 rt,I64 ptrs,CHeapCtrl *ctrl);
#endif
//
// See arm_backend.c
//
#define AIWNIOS_IREG_START 19
#define AIWNIOS_IREG_CNT (28 - 19 + 1)
#define AIWNIOS_FREG_START 8
#define AIWNIOS_FREG_CNT (15 - 8 + 1)

//
// Turns my CRPN from the parser into the IC for the C side
//

U8 *AiwniosCompile(CCmpCtrl *cc,I64 *res_sz=NULL,CDbgInfo **info) {
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
  if(info) {
    idx=cc->max_line-cc->min_line;
    *info=MAlloc((idx+1)*8+sizeof(CDbgInfo));
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
            mv[idx].m->reg=ir++;
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
    //Assign offsets
    foff=0;
    for(idx=0;idx!=member_cnt;idx++) {
      mlst=mv[idx].m;
      if(mlst->reg==REG_NONE) {
      if(!StrCmp(cc->htc.fun->str,"DocPrintPartial")) {
        "%s,foff:%x,mo:%x\n",mlst->str,foff,mlst->offset;
      }
        foff_lookup[idx].i32[1]=foff;
        foff_lookup[idx].i32[0]=mlst->offset;
        mlst->offset=foff;
        foff+=mlst->member_class->size*mlst->dim.total_cnt;  
        if(foff&7)
          foff+=8-(foff&7); //Align to 8
      }
    }
    
    if(!StrCmp(cc->htc.fun->str,"DocPrintPartial")) {
      for(idx=0;idx!=member_cnt;idx++) {
        mlst=mv[idx].m;
        if(mlst->reg==REG_NONE) {
          "%s(FOFF:%x)\n",mlst->str,mlst->offset;
        } else {
          "%s(REG:%d)\n",mlst->str,mlst->reg;
        }
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
    __HC_ICAdd_SetFrameSize(cc2,cc->htc.fun->size+16,Fs->data_heap);
    mlst=cc->htc.fun->member_lst_and_root;
    argc=cc->htc.fun->arg_cnt;
    if(cc->htc.fun->flags&(1<<Ff_DOT_DOT_DOT))
      argc+=2;
    for(idx=0;idx!=argc;idx++) {
      if(mlst->reg==REG_NONE)
        __HC_ICAdd_Frame(cc2,mlst->offset,mlst->member_class->raw_type,0,Fs->data_heap);
      else if(mlst->member_class->raw_type!=RT_F64) {
        __HC_ICAdd_IReg(cc2,mlst->reg,mlst->member_class->raw_type,0,Fs->data_heap);
      } else {
        __HC_ICAdd_FReg(cc2,mlst->reg,Fs->data_heap);      
      }
      __HC_ICAdd_Arg(cc2,idx,Fs->data_heap);
      mlst=mlst->next;
    }
    Free(mv);
  } else {
    //
    // Add MANDITORY room for FP/LR pair
    //
    __HC_ICAdd_SetFrameSize(cc2,16,Fs->data_heap);
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
        __HC_ICAdd_StaticData(acc,cc2,foff,mlst->static_data,mlst->member_class->size*mlst->dim.total_cnt,Fs->data_heap);
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
      __HC_ICAdd_SetStaticsSize(cc2,foff,Fs->data_heap);
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
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,RT_U8,0,Fs->data_heap);
    } else if(cur->ic_class->ptr_stars_cnt)
      //All pointer arithmetic is pre-computed
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,RT_PTR,0,Fs->data_heap);
    else
      new=__HC_ICAdd_Frame(cc2,cur->ic_data,cur->ic_class->raw_type,0,Fs->data_heap);
    break;case IC_BR_ZERO:
    new=__HC_ICAdd_LNot(cc2,Fs->data_heap);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_BR_NOT_ZERO:
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    //break;case IC_BR_CARRY: ???
    //break;case IC_BR_NOT_CARRY: ???
    break;case IC_BR_EQU_EQU:
    new=__HC_ICAdd_EqEq(cc2,Fs->data_heap);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_BR_NOT_EQU:
    new=__HC_ICAdd_Ne(cc2,Fs->data_heap);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_BR_LESS:
    new=__HC_ICAdd_Lt(cc2,Fs->data_heap);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_BR_GREATER_EQU:
    new=__HC_ICAdd_Ge(cc2,Fs->data_heap);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_BR_GREATER:
    new=__HC_ICAdd_Gt(cc2,Fs->data_heap);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_BR_LESS_EQU:
    new=__HC_ICAdd_Le(cc2,Fs->data_heap);
    __HC_ICSetLine(new,cur->ic_line);
    new=__HC_ICAdd_GotoIf(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
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
      new=__HC_ICAdd_Label(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    //break;case IC_CALL_START: AIWNIOS DOESNT USE THESE
    //break;case IC_CALL_END:
    //break;case IC_CALL_END2:
    //break;case IC_RETURN_VAL:
    //break;case IC_RETURN_VAL2:
    break;case IC_IMM_I64:
    new=__HC_ICAdd_I64(cc2,cur->imm_i64,Fs->data_heap);
    break;case IC_IMM_F64:
    new=__HC_ICAdd_F64(cc2,cur->imm_f64,Fs->data_heap);
    break;case IC_STR_CONST:
    new=__HC_ICAdd_Str(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_ABS_ADDR:
    misc=COCMiscNew(cc,CMT_LABEL);
    misc->str=MStrPrint("0x%x",cur->imm_i64);
    new=__HC_ICAdd_Reloc(
          acc,
          cc2,
          &misc->addr,
          misc->str,
          RT_U8,
          1,
          Fs->data_heap
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
    if(h->type&HTT_GLBL_VAR) {
      if((h(CHashGlblVar*)->flags&GVF_EXTERN)||cc->flags&CCF_AOT_COMPILE) {
        new=__HC_ICAdd_Reloc(
          acc,
          cc2,
          &cur->ic_data(CCodeMisc*)->addr,
          h->str,
          RT_U8,
          1,
          Fs->data_heap
        );
      } else {
        new=__HC_ICAdd_I64(
          cc2,
          h(CHashGlblVar*)->data_addr,
          Fs->data_heap
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
            1,
            Fs->data_heap
          );
      } else {
        new=__HC_ICAdd_I64(
          cc2,
          h(CHashFun*)->exe_addr,
          Fs->data_heap
        );
      }
    }
    break;case IC_REG:
    if(cur->ic_class->raw_type==RT_F64) {
      new=__HC_ICAdd_FReg(cc2,cur->imm_i64,Fs->data_heap);
    } else {
      new=__HC_ICAdd_IReg(cc2,cur->imm_i64,cur->ic_class->raw_type,0,Fs->data_heap);
    }
    break;case IC_TO_I64:
    new=__HC_ICAdd_ToI64(cc2,Fs->data_heap);
    break;case IC_TO_F64:
    new=__HC_ICAdd_ToF64(cc2,Fs->data_heap);
    break;case IC_TO_BOOL: //TODO
    break;case IC_TOUPPER:  //TODO
    break;case IC_HOLYC_TYPECAST:
    new=__HC_ICAdd_Typecast(cc2,cur->ic_class->raw_type,0,Fs->data_heap);
    break;case IC_ADDR:
    new=__HC_ICAdd_Addr(cc2,Fs->data_heap);
    break;case IC_COMMA:
    new=__HC_ICAdd_Comma(cc2,Fs->data_heap);
    break;case IC_COM:
    new=__HC_ICAdd_BNot(cc2,Fs->data_heap);
    break;case IC_NOT:
    new=__HC_ICAdd_LNot(cc2,Fs->data_heap);
    break;case IC_UNARY_MINUS:
    new=__HC_ICAdd_Neg(cc2,Fs->data_heap);
    break;case IC_DEREF:
    if(cur->ic_class->raw_type==RT_PTR)
      new=__HC_ICAdd_Deref(cc2,cur->ic_class->raw_type,0,Fs->data_heap);
    else
      new=__HC_ICAdd_Deref(cc2,cur->ic_class->raw_type,0,Fs->data_heap);
    //break;case IC_DEREF_PP //NOT USED BY AIWINIOS
    //break;case IC_DEREF_MM //DITTO
    break;case IC__PP:
    new=__HC_ICAdd_PostInc(cc2,cur->ic_data,Fs->data_heap);
    break;case IC__MM:
    new=__HC_ICAdd_PostDec(cc2,cur->ic_data,Fs->data_heap);
    break;case IC_PP_:
    new=__HC_ICAdd_PreInc(cc2,cur->ic_data,Fs->data_heap);
    break;case IC_MM_:
    new=__HC_ICAdd_PreDec(cc2,cur->ic_data,Fs->data_heap);
    break;case IC_SHL:
    new=__HC_ICAdd_Lsh(cc2,Fs->data_heap);
    break;case IC_SHR:
    new=__HC_ICAdd_Rsh(cc2,Fs->data_heap);
    //break;case IC_SHL_CONST: NOT USED BY AIWNIOS
    //break;case IC_SHR_CONST: NOT USED BY AIWNIOS
    break;case IC_POWER:
    new=__HC_ICAdd_Pow(cc2,Fs->data_heap);
    break;case IC_MUL:
    new=__HC_ICAdd_Mul(cc2,Fs->data_heap);
    break;case IC_DIV:
    new=__HC_ICAdd_Div(cc2,Fs->data_heap);
    break;case IC_MOD:
    new=__HC_ICAdd_Mod(cc2,Fs->data_heap);
    break;case IC_AND:
    new=__HC_ICAdd_And(cc2,Fs->data_heap);
    break;case IC_OR:
    new=__HC_ICAdd_Or(cc2,Fs->data_heap);
    break;case IC_XOR:
    new=__HC_ICAdd_Xor(cc2,Fs->data_heap);
    break;case IC_ADD:
    new=__HC_ICAdd_Add(cc2,Fs->data_heap);
    break;case IC_SUB:
    new=__HC_ICAdd_Sub(cc2,Fs->data_heap);
    //break;case IC_ADD_CONST NOT USED BY AIWNIOS
    //break;case IC_SUB_CONST NOT USED BY AIWNIOS
    break;case IC_EQU_EQU:
    new=__HC_ICAdd_EqEq(cc2,Fs->data_heap);
    break;case IC_NOT_EQU:
    new=__HC_ICAdd_Ne(cc2,Fs->data_heap);
    break;case IC_LESS:
    new=__HC_ICAdd_Lt(cc2,Fs->data_heap);
    break;case IC_GREATER_EQU:
    new=__HC_ICAdd_Ge(cc2,Fs->data_heap);
    break;case IC_GREATER:
    new=__HC_ICAdd_Gt(cc2,Fs->data_heap);
    break;case IC_LESS_EQU:
    new=__HC_ICAdd_Le(cc2,Fs->data_heap);
    //break;case IC_PUSH_CMP NOT USED BY AIWNIOS
    break;case IC_AND_AND:
    new=__HC_ICAdd_AndAnd(cc2,Fs->data_heap);
    break;case IC_OR_OR:
    new=__HC_ICAdd_OrOr(cc2,Fs->data_heap);
    break;case IC_XOR_XOR:
    new=__HC_ICAdd_XorXor(cc2,Fs->data_heap);
    break;case IC_ASSIGN:
    new=__HC_ICAdd_Eq(cc2,Fs->data_heap);
    //break;case IC_ASSIGN_PP //NOT USED BY AIWNIOS
    //break;case IC_ASSIGN_MM //DITTO
    break;case IC_SHL_EQU:
    new=__HC_ICAdd_LshEq(cc2,Fs->data_heap);
    break;case IC_SHR_EQU:
    new=__HC_ICAdd_RshEq(cc2,Fs->data_heap);
    break;case IC_MUL_EQU:
    new=__HC_ICAdd_MulEq(cc2,Fs->data_heap);
    break;case IC_DIV_EQU:
    new=__HC_ICAdd_DivEq(cc2,Fs->data_heap);
    break;case IC_MOD_EQU:
    new=__HC_ICAdd_ModEq(cc2,Fs->data_heap);
    break;case IC_AND_EQU:
    new=__HC_ICAdd_AndEq(cc2,Fs->data_heap);
    break;case IC_OR_EQU:
    new=__HC_ICAdd_OrEq(cc2,Fs->data_heap);
    break;case IC_XOR_EQU:
    new=__HC_ICAdd_XorEq(cc2,Fs->data_heap);
    break;case IC_ADD_EQU:
    new=__HC_ICAdd_AddEq(cc2,Fs->data_heap);
    break;case IC_SUB_EQU:
    new=__HC_ICAdd_SubEq(cc2,Fs->data_heap);
    break;case IC_JMP:
    new=__HC_ICAdd_Goto(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_SUB_RET:
    new=__HC_ICAdd_SubRet(cc2,Fs->data_heap);
    break;case IC_SUB_CALL:
    new=__HC_ICAdd_SubCall(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_SWITCH:
    misc=cur->ic_data(CCodeMisc*);
    __HC_ICAdd_BoundedSwitch(cc2,misc->addr,misc->dft->addr,Fs->data_heap);
    break;case IC_NOBOUND_SWITCH:
    __HC_ICAdd_UnboundedSwitch(cc2,cur->ic_data(CCodeMisc*)->addr,Fs->data_heap);
    break;case IC_RET:
    //TODO CHECK IF CUR FUN IS I64/F64
    new=__HC_ICAdd_Ret(cc2,Fs->data_heap);
    break;case IC_CALL_INDIRECT:
    new=__HC_ICAdd_Call(
      cc2,
      cur->length-1,
      cur->ic_class->raw_type,
      0,
      Fs->data_heap);
    /*TODO 
    IC_CALL
    
    IC_CALL_INDIRECT2
    IC_CALL_IMPORT
    IC_CALL_EXTERN
    */
    break;case IC_VARGS:
    new=__HC_ICAdd_Vargs(cc2,cur->ic_data,Fs->data_heap);
    break;case IC_STATIC:
    new=__HC_ICAdd_StaticRef(cc2,cur->ic_data,cur->ic_class->raw_type,0,Fs->data_heap);
    }
    __HC_ICSetLine(new,cur->ic_line);
  }
  //TODO free AIWNIOS CmpCtrl
  machine_code=__HC_Compile(acc,res_sz,&aiwnios_dbg_info);
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
          tmpaie->rip=cc->aotc->rip+misc->addr(U8*)-machine_code;
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
    } else if(misc->type==CMT_LABEL&&misc->str&&!StrNCmp("0x",misc->str,2)) {
      tmpabs=CAlloc(sizeof(CAOTAbsAddr));
      tmpabs->next=cc->aotc->abss;
      tmpabs->type=AAT_ADD_U64;
      tmpabs->rip=cc->aotc->rip+misc->addr(U8*)-machine_code;
      cc->aotc->abss=tmpabs;	
      idx=misc->addr(U64*)[0]=Str2I64(misc->str);
    }
  }
  return machine_code;
}

U8 *COCCompile(CCmpCtrl *cc,I64 *_code_size,CDbgInfo **_dbg,I64 *_type) {
//TODO fill more
  U8 *mc=AiwniosCompile(cc,_code_size,_dbg);
  return mc;
}

U0 PrsBindCSymbol(U8 *as,U8 *fptr) {
  CHashFun *fun=HashFind(as,Fs->hash_table,HTT_FUN);
  if(fun&&fun->flags&(1<<Cf_EXTERN)) {
    fun->flags&=~(1<<Cf_EXTERN);
    fun->exe_addr=fptr;
  }
}

I64 poly_fill_fs=NULL;
CTask *PolyFillFs() {
  return poly_fill_fs;
}
U0 PolyFillSetFs(CTask *t) {
  poly_fill_fs=t;
}

U0 PrsBindCSymbols() {
  PrsBindCSymbol("HeapCtrlInit", &HeapCtrlInit);
  PrsBindCSymbol("HeapCtrlDel", &HeapCtrlDel);
  PrsBindCSymbol("__MAlloc", &__MAlloc);
  PrsBindCSymbol("__CAlloc", &__CAlloc);
  PrsBindCSymbol("Free", &Free);
  PrsBindCSymbol("MSize", &MSize);
  PrsBindCSymbol("__StrNew", &__StrNew);
  PrsBindCSymbol("MemCpy", &MemCpy);
  PrsBindCSymbol("MemSet", &MemSet);
  PrsBindCSymbol("MemSetU16", &MemSetU16);
  PrsBindCSymbol("MemSetU32", &MemSetU32);
  PrsBindCSymbol("MemSetU64", &MemSetU64);
  PrsBindCSymbol("StrLen", &StrLen);
  PrsBindCSymbol("StrCmp", &StrCmp);
  PrsBindCSymbol("ToUpper", &ToUpper);
  PrsBindCSymbol("Log10", &Log10);
  PrsBindCSymbol("Log2", &Log2);
  PrsBindCSymbol("Pow10", &Pow10);
  PrsBindCSymbol("PrintI", &PrintI);
  PrsBindCSymbol("PrintF", &PrintF);
  PrsBindCSymbol("Round", &Round);
  PrsBindCSymbol("Floor", &Floor);
  PrsBindCSymbol("Ceil", &Ceil);
  PrsBindCSymbol("Bt", &Bt);
  PrsBindCSymbol("LBts", &LBts);
  PrsBindCSymbol("LBtr", &LBtr);
  PrsBindCSymbol("Bsf", &Bsf);
  PrsBindCSymbol("Bsr", &Bsr);
  PrsBindCSymbol("PutS", &PutS);
  PrsBindCSymbol("SetFs", &PolyFillSetFs);
  PrsBindCSymbol("Fs", &PolyFillFs);
  PrsBindCSymbol("__GetTicks", &__GetTicks);
  PrsBindCSymbol("AIWNIOS_SetJmp", &AIWNIOS_SetJmp);
  PrsBindCSymbol("AIWNIOS_LongJmp", &AIWNIOS_LongJmp);
  PrsBindCSymbol("IsValidPtr", &IsValidPtr);
  PrsBindCSymbol("Caller",&Caller);
    PrsBindCSymbol("VFsSetPwd",&VFsSetPwd);
    PrsBindCSymbol("VFsExists",&VFsExists);
    PrsBindCSymbol("VFsIsDir",&VFsIsDir);
    PrsBindCSymbol("VFsFSize",&VFsFSize);
    PrsBindCSymbol("VFsFRead",&VFsFRead);
    PrsBindCSymbol("VFsFWrite",&VFsFWrite);
    PrsBindCSymbol("VFsDel",&VFsDel);
    PrsBindCSymbol("VFsDir",&VFsDir);
    PrsBindCSymbol("VFsDirMk",&VFsDirMk);
    PrsBindCSymbol("VFsFBlkRead",&VFsFBlkRead);
    PrsBindCSymbol("VFsFBlkWrite",&VFsFBlkWrite);
    PrsBindCSymbol("VFsFOpenW",&VFsFOpenW);
    PrsBindCSymbol("VFsFOpenR",&VFsFOpenR);
    PrsBindCSymbol("VFsFClose",&VFsFClose);
    PrsBindCSymbol("VFsFSeek",&VFsFSeek);
    PrsBindCSymbol("VFsSetDrv",&VFsSetDrv);
    PrsBindCSymbol("FUnixTime",&FUnixTime);
    PrsBindCSymbol("FSize",&FSize);
    PrsBindCSymbol("VFsFTrunc",&VFsFTrunc);
    PrsBindCSymbol("UnixNow",&UnixNow);
}

U0 Poopacle() {
  I64 (*foo)(I64);
  U8 *src;
  I64 idx;
  CHashFun *h;
  CCmpCtrl *cmp=CmpCtrlNew(
    src=FileRead("TEST1.HC",NULL),
    CCF_DONT_FREE_BUF,
    "TEST.HC"
  );
  QueIns(cmp,Fs->last_cc);
  Lex(cmp);
  while(cmp->token) {
    PrsStmt(cmp);
  }
  QueRem(cmp);
  foo=HashFind("Foo1",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo1(8) is %d\n",foo(8);
  foo=HashFind("Foo2",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo2(8) is %d\n",foo(8);
  foo=HashFind("Foo3",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo3(8) is %d\n",foo(8);
  foo=HashFind("Foo4",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo4(8) is %d\n",foo(8);
  foo=HashFind("Foo5",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo5(8) is %d\n",foo(8);
  foo=HashFind("Foo6",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo6(8) is %d\n",foo(8);
  foo=HashFind("Foo7",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo7(8) is %d\n",foo(8);
  foo=HashFind("Foo8",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo8(8) is %d\n",foo(8);
  h=HashFind("PutS",Fs->hash_table,HTT_FUN);
  h->exe_addr=&PutS;  
  HashFind("PutS",Fs->hash_table,HTT_FUN)(CHashFun *)->flags&=~(1<<Cf_EXTERN);
  foo=HashFind("Foo9",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  SysSymImportsResolve("PutS",0);
  "Foo9 is at %s\n",SrcEdLink(foo);
  "Foo9(8) is %d\n",foo(8);
  foo=HashFind("Foo10",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo10(8) is %d\n",foo(8);
  foo=HashFind("Foo11",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  for(idx=0;idx!=8;idx++)
    "Foo11(8) is %d\n",foo(8);
  foo=HashFind("Foo12",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo12(8) is %d\n",foo(8);
  foo=HashFind("Foo13",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo13(8) is %d\n",foo(8);
  foo=HashFind("Foo14",Fs->hash_table,HTT_FUN)(CHashFun *)->exe_addr;
  "Foo14(8) is %p\n",foo(8);
  QueRem(cmp);
  CmpCtrlDel(cmp);
  cmp=CmpCtrlNew(
    src=MStrPrint(
      "#include \"HCRT_TOS.HC\";;\n"
    ),
    0,
    "TEST.HC"
  );
  QueIns(cmp,Fs->last_cc);
  Lex(cmp);
  PutS(src);
  while(cmp->token) {
    COCPush(cmp);
    COCInit(cmp);
    PrsStmt(cmp);
    foo=AiwniosCompile(cmp,NULL,NULL);
    (*foo)(0);
    Free(foo);
    COCPop(cmp);
    //TODO better way of doing this
    PrsBindCSymbols;
  }
}

/**
 * Has format
 * U8 type
 * U8 name[]
 * U8 src_file[]
 * U8 help_idx[]
 * I32 min_ln
 * I32 max_ln
 * I32 line_offsets[max_ln-min_ln+1]
 */
I64 __AiwniosWriteDbgInfo(CTask *universe=NULL,U8 *write_to=NULL) {
  I64 len=0,idx,ln,lns;
  CHashFun *tmpf;
  CHash *h;
  U8 *fn;
  U32 *offs;
  for(idx=0;idx<=universe->hash_table->mask;idx++) {
    for(h=universe->hash_table->body[idx];h;h=h->next) {
      if(h->type&HTT_FUN) {
        tmpf=h;
        if(write_to)
          write_to[len]=HTt_FUN;
        len++;
        if(write_to)
          StrCpy(write_to+len,h->str);
        len+=StrLen(h->str)+1;
        if(tmpf->idx) {
          if(write_to)
            StrCpy(write_to+len,tmpf->idx);
          len+=StrLen(tmpf->idx)+1;
        } else {
          if(write_to)
            write_to[len]=0;
          len++;
        }
        fn=StrNew(tmpf->src_link);
        if(!fn) fn=StrNew("");
        StrFirstRem(fn,":");
        StrLastRem(fn,",");
        if(write_to)
          StrCpy(write_to+len,fn);
        len+=StrLen(fn)+1;
        Free(fn);
        if(tmpf->dbg_info) {
          lns=1+tmpf->dbg_info->max_line-tmpf->dbg_info->min_line;
          if(write_to)
            write_to[len](U32)=tmpf->dbg_info->min_line;
          len+=4;
          if(write_to)
            write_to[len](U32)=tmpf->dbg_info->max_line;
          len+=4;
          if(write_to) {
            offs=write_to+len;
            for(ln=0;ln!=lns;ln++) {
              //dbg_info[0] is the function address always
              offs[ln]=tmpf->dbg_info[ln+1]-tmpf->dbg_info[0];
            }
          }
          len+=4*lns;
        } else {
          //Min_ln and Max_ln
          if(write_to)
            write_to[len](U64)=0;
          len+=8;
          //Dummy body field
          if(write_to)
            write_to[len](U32)=0;
          len+=4;
        }
      }
    }
  }
  return len;
}
U0 AiwniosWriteDbgInfo(CTask *universe,U8 *to) {
  I64 len=__AiwniosWriteDbgInfo(universe,NULL);
  U8 *data=MAlloc(len);
  __AiwniosWriteDbgInfo(universe,data);
  FileWrite(to,data,len);
  Free(data);
}

/**
 * Has format
 * U8 type
 * U8 name[]
 * U8 src_file[]
 * U8 help_idx[]
 * I32 first_ln
 * I32 last_ln
 * I32 line_offsets[last_ln-first_ln+1]
 */
/*
U0 AiwniosLoadDbgInfo(CTask *universe,U8 *from) {
  U8 *ftxt=FileRead(from),*src=ftxt,*name,*help_idx,*fn;
  I64 type,max_ln,min_ln;
  CHashFun *tmpf;
  while(type=*src++) {
    switch(type) {
    start:
      name=src;
      src+=StrLen(name)+1;
      fn=src;
      src +=StrLen(fn)+1;
      help_idx=src;
      src+=StrLen(help_idx)+1;
      min_ln=*src(U32*)++;
      max_ln=*src(U32*)++;
    case HTt_FUN:
      tmpf=HashFind(name,universe,HTT_FUN);
      if(tmpf) {
        if(min_ln==max_ln&&!min_ln) {
        }
      }
      break;
    end:
    ;
    }
  }
}
*/
/*
//Compile universe's symbols with CCF_AOT
U0 AiwniosAotCompile(U8 *file,U8 *write_to,U8 *dbg_info=NULL) {
  U8 *ret;
  I64 len;
  CHashTable *old=Fs->hash_table;
  Fs->hash_table=HashTableNew(0x1000);
  Fs->hash_table->next=old;
  V
  CCmpCtrl *cc=CmpCtrlNew(
    MStrPrint("#include \"%s\";;",file),
    CCF_AOT_COMPILE,
    "Tmp"
  );
  Lex(cc);
  COCPush(cc);
  while(cc->token) {
    PrsStmt(cc);
  }
  //TODO compile main code
  COCPop(cc);
  len=__AiwniosAotCmp(Fs,NULL);
  ret=MAlloc(len);
  __AiwniosAotCmp(Fs,ret);
  FileWrite(write_to,ret,len);
  Free(ret);
  HashTableDel(Fs->hash_table);
  Fs->hash_table=old;
}
*/
