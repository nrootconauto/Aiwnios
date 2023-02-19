#include "KernelA.HH"
#include "CompilerA.HH"
#include "CExts.HC"
#include "CompilerB.HH"
#include "KGlbls.HC"
CCPU cpu_structs[64];
//AIWNIOS Boot stuff
U8 *StrNew(U8 *str,CTask *task=NULL) {
  if(!task)
    task=Fs;
  return __StrNew(str,task->data_heap);
}
U8 *MAlloc(I64 cnt,CTask *task=NULL) {
  U8 *hc;
  if(task)
    hc=task->data_heap;
  else
    hc=Fs->data_heap;
  return __MAlloc(cnt,hc);
}
U8 *CAlloc(I64 cnt,CTask *task=NULL) {
  U8 *hc;
  if(task)
    hc=task->data_heap;
  else
    hc=Fs->data_heap;
  return __CAlloc(cnt,hc);
}
U8 *AStrNew(U8 *str) {
  return StrNew(str,adam_task);
}
U8 *AMAlloc(I64 cnt,CTask *task=NULL) {
  return MAlloc(cnt,adam_task);
}
U8 *ACAlloc(I64 cnt) {
  return CAlloc(cnt,adam_task);
}
U8 *MAllocIdent(U8 *ptr,CTask *task=NULL) {
  U8 *hc,*ret;
  I64 cnt=MSize(ptr);
  if(task)
    hc=task->data_heap;
  else
    hc=Fs->data_heap;
  ret=__MAlloc(cnt,hc);
  MemCpy(ret,ptr,cnt);
  return ret;
}

U8 *AMAllocIdent(U8 *ptr) {
  return MAllocIdent(ptr,adam_task);
}

CTask *aiwnios_boot_task=__CAlloc(sizeof CTask);
SetFs(aiwnios_boot_task);
CCPU *Gs() {
  return &cpu_structs[0];
}
extern U0 Print(U8*,...);

U0 SwapU8(U8 *a,U8 *b) {
  U8 tmp=*a;
  *a=*b;
  *b=tmp;
}

U0 SwapU16(U16 *a,U16 *b) {
  U16 tmp=*a;
  *a=*b;
  *b=tmp;
}

U0 SwapU32(I32 *a,I32 *b) {
  I32 tmp=*a;
  *a=*b;
  *b=tmp;
}

U0 SwapI64(I64 *a,I64 *b) {
  I64 tmp=*a;
  *a=*b;
  *b=tmp;
}
#include "CQue.HC"
#include "KDate.HC"
#include "StrA.HC"
#include "KHashA.HC"
#include "KMath.HC"
#include "KMathB.HC"
#include "StrPrint.HC"
#include "StrA.HC"
#include "Sched.HC"
#include "Display.HC"
#include "KTask.HC"
#include "MultiProc.HC"
#include "KExcept.HC"
#include "Job.HC"
#include "KDataTypes.HC"
#include "KHashB.HC"
#include "Compress.HC"
#include "StrB.HC"
#include "KDefine.HC"
#include "StrScan.HC"
#include "KMisc.HC"
adam_task=aiwnios_boot_task;
TaskInit(adam_task,0);
#include "QSort.HC"
#include "MakeBlkDev.HC"
#include "KEnd.HC"
#include "FontStd.HC"
#include "FontCyrillic.HC"
#include "KLoadARM.HC"
#include "FunSeg.HC"
#include "Compiler.HC"
#include "KeyDev.HC"
#include "Keyboard.HC"
#include "Message.HC"
#include "Mouse.HC"
#include "EdLite.HC"
#include "KMain.HC"
U0 InitCPUs() {
  MemSet(&cpu_structs,0,sizeof(CCPU) *64);
  //TODO mp_cnt
  cpu_structs[0].addr=&cpu_structs[0];
  cpu_structs[0].seth_task=adam_task;
  //See MultiProc.HC
  CCPU *c=&cpu_structs;
  c->addr=c;
  c->num=0;
  c->idle_factor=0.01;
  QueInit(&c->next_dying);
  adam_task->gs=c;
  adam_task->task_signature=TASK_SIGNATURE_VAL;
  //cpu_structs[0].seth_task=Spawn(&CoreAPSethTask,,,,adam_task,,0); //TODO
}
InitCPUs;
class CHashNum:CHash {
  I64 value;
};
I64 StaticTest() {
  static I64 stat=123;
  return stat++;
}
I64 Foodle(I64 a,I64 b,I64 c) {
  return a+b+c;
} 
class CABC {
  I32 a,b,c;
};
U0 Main(I64 arg) {
  "I GOT ARGUMENT:%d\n",arg;
  U8 *r;
  I64 idx,(*fp)(I64,I64,I64);
  I64 u32=0;
  CHashNum *a;
  CHashTable *ht;
  "%d\n",1&&u32;
  "%d\n",u32&&1;
  "%d\n",1||u32;
  "%d\n",u32||1;
  u32=1;
  "%d\n",1&&u32;
  "%d\n",u32&&1;
  "%d\n",0||u32;
  "%d\n",u32||0;
  
  for(idx=0;idx!=32;idx++) {
    u32=1;
    BFieldOrU32(&u32,idx,0xf2345678);
    "PASS:%d,%d\n",idx,u32==(0xf2345678<<idx|1);
    if(idx) { //1st bit is set (1)
      if(BFieldExtU32(&u32,idx,32)!=0xf2345678)
        "FAIL,%x\n",BFieldExtU32(&u32,idx,32);
    }
  }
  PrintI("I32",sizeof I32);
  PrintI("I32",sizeof(I32));
  PrintI("I32*",sizeof(I32*));
  //
  //
  //
  ht=HashTableNew(0x10);
  for(idx=0;idx!=100;idx++) {
    a=CAlloc(sizeof(CHashNum));
    a->str=MStrPrint("HI:%d",idx);
    a->type=1;
    a->value=idx;
    HashAdd(a,ht);
  }
  for(idx=0;idx!=100;idx++) {
    r=MStrPrint("HI:%d",idx);
    "%s,%d\n",r,HashFind(r,ht,1)(CHashNum*)->value;
    Free(r);
  }
  U8 buf[8];
  buf[0](I64)=(idx+0)*1;
  "%c\n",'abcdef';
  "MAT:%d\n",WildMatch("POOP.TXT","*.???");
  //HashTableDel(ht);
  for(idx=0;idx!=3;idx++)
    "StaticTest:%d\n",StaticTest;
  Seed(123);
  "%n\n",Rand;
  "%n\n",Rand;
  "%n\n",Rand;
  "%d,%d,%d\n",offset(CABC.a),offset(CABC.b),offset(CABC.c);
  fp=&Foodle;
  "Foodle:%d\n",(*fp)(1,2,3);
  "Yielding\n";
  "HERE,%x,%x\n",Fs,Fs->task_flags;
  Yield;
  "Yield-Done\n";
  try {
    "In try1\n";
    throw('abcdef');
  } catch {
    "I cuaght a %c\n",Fs->except_ch;
    PutExcept;
  }
  try {
    "In try\n";
  } catch {
    "Oh poop,im not suppost to be here\n";
    PutExcept;
  }
  try {
    try {
      "Not catching this now\n";
      throw('gotit');
    } catch {
      "Fail to catch\n";
    }
    "Never reach here\n";
  } catch {
    "I cuaght a %c\n",Fs->except_ch;
    PutExcept;
  }
  "End try\n";
  "PARSER TEST\n";
  ParserTest;
  Poopacle;
  Exit;
}
#ifndef NO_BOOT
SetFs(Gs->seth_task);
#endif
//Cmp("TEST2.HC",,"HCRT2.BIN");
/*
U0 LexTests() {
I64 x;
  "ad:%xfs:%x,ah:%x,nxt:%x\n",adam_task,Fs,cmp.asm_hash,adam_task->hash_table->next;
  for(x=0;x!=16;x++)
    "%d,%x\n",x,char_bmp_alpha_numeric_no_at[x];
  CCmpCtrl *cctrl=CmpCtrlNew(
  "123\n123.456\n||\n_abcdef123\n"
  ,CCF_DONT_FREE_BUF,"Toad.HC");
  "POODLES:%x\n",HashFind("include",Fs->hash_table,-1);
  "%x:\n",123;
  if(Lex(cctrl)==TK_I64)
    "YAY:%d\n",cctrl->cur_i64;
  if(Lex(cctrl)==TK_F64)
    "YAY:%n\n",cctrl->cur_f64;
  if((x=Lex(cctrl))==TK_OR_OR)
    "YAY:%c\n",'||';
  else
    "POOPIE:%x,%c\n",x,x;
  "%x,%x,%x\n",Gs->seth_task->hash_table,Fs->hash_table->next,adam_task->hash_table->next;
  if(Lex(cctrl)==TK_IDENT)
    "YAY:%s\n",cctrl->cur_str;
  CmpCtrlDel(cctrl);
}
LexTests;
*/
#ifdef COMPONET_GR
#include "ADskA.HC"
#include "MakeGr.HC"
#include "Menu.HC"
#include "Win.HC"
#include "WinMgr.HC"
#include "GodExt.HC"
#include "MakeDoc.HC"
DrawWindowNew;
Gs->seth_task=adam_task;
sys_winmgr_task=Spawn(&WinMgrTask,NULL,"Window Mgr");
U0 Poodacle(I64) {
  I64 x=0,y=0;
  PaletteSetStd;
  WinMax(User);
  while(TRUE) {
    DCFill;
    gr.dc->color=RED;
    GrRect(,x,y,100,100);
    x+=5,y+=5;
    x%=GR_WIDTH;
    y%=GR_HEIGHT;
    Refresh;
  }
}
U0 UserStartUp()
{//Run each time a user a spawned
  DocTermNew;
  DocPrint(DocPut,"HEWLLO WORLD\n");
  //Type("::/Doc/Start.DD");
  LBts(&(Fs->display_flags),DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  //Dir;
  //"$TX,"82,150",D="DD_TEMPLEOS_LOC"$LOC:";
  //CPURep;
}
Spawn(&Poodacle);
CoreAPSethTask;
#endif
;
