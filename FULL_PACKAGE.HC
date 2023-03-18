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
  if(!str) str="";
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
#ifndef NO_BOOT
SetFs(Gs->seth_task);
#endif
#ifdef COMPONET_GR
#include "ADskA.HC"
#include "ADskB.HC"
#include "MakeGr.HC"
#include "Menu.HC"
#include "Win.HC"
#include "WinMgr.HC"
#include "GodExt.HC"
#include "MakeDoc.HC"
#include "Training.HC"
#include "MakeAC.HC"
#include "Mount.HC"
#include "MakeGod.HC"
#include "ASnd.HC"
#include "TaskSettings.HC"
#include "MakeCtrls.HC"
#include "Utils.HC"
#include "FileMgr.HC"
//Cmp("TEST2.HC",,"HCRT2.BIN");
#ifdef GEN_HEADERS
CDoc *unfound=DocNew("unfound.DD");
#endif
U0 ImportSymbolsCB(U8 *name,U8 *addr) {
  CHash *find=HashFind(name,Fs->hash_table,-1);
  I64 cnt=1;
  if(find&&find->type&HTT_GLBL_VAR) {
    find(CHashGlblVar*)->data_addr=addr;
  } else if(find&&find->type&HTT_FUN&&!(find(CHashFun*)->flags&1<<Ff_INTERNAL)) {
    find(CHashFun*)->exe_addr=addr;
    LBtr(&find(CHashFun*)->flags,Cf_EXTERN);
  } 
  #ifdef GEN_HEADERS
  else if(1) {
    DocPrint(unfound,"%s\n",name);
  }
  #endif
}
U0 InitSymbols() {
  ExeFile("KernelA.HH");
  ImportSymbolsToHolyC(&ImportSymbolsCB);
  if(FileFind("HCRT2.DBG.Z"))
    DbgInfoRead("HCRT2.DBG.Z");
  #ifdef GEN_HEADERS
  DocWrite(unfound);
  DocDel(unfound);
  #endif
}
InitSymbols;
DrawWindowNew;
#ifdef GEN_HEADERS
#include "GenHeader.HC";;
#endif
Gs->seth_task=adam_task;
sys_winmgr_task=Spawn(&WinMgrTask,NULL,"Window Mgr");
U0 UserStartUp()
{//Run each time a user a spawned
  DocTermNew;
  //Type("::/Doc/Start.DD");
  LBts(&(Fs->display_flags),DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  //Dir;
  //"$TX,"82,150",D="DD_TEMPLEOS_LOC"$LOC:";
  //CPURep;
}
U0 Poodacle(I64) {
  CTask *t; 
  I64 x=0,y=0;
  PaletteSetStd;
  WinMax(t=User);
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
Spawn(&Poodacle);
ACInit("T:/Kernel*.HH*");
CoreAPSethTask;
#endif;
