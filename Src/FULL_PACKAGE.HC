#include "KernelA.HH"
#ifdef BOOTSTRAP
#define mp_cnt 1
#endif
#include "CompilerA.HH"
#include "CExts.HC"
#include "CompilerB.HH"
#include "KGlbls.HC"
#include "KMisc.HC"

CCPU cpu_structs[64];
SetGs(&cpu_structs[0]);
//AIWNIOS Boot stuff
U8 *StrNew(U8 *str,CTask *task=NULL) {
  if(task) {
//Heap?
    if(task->task_signature.u8[0]=='H') {
      task=task;
      goto do_it;
    }
  }
  if(task)
    task=task->data_heap;
  else
    task=Fs->data_heap;
do_it:
  if(!str) str="";
  return __StrNew(str,task);
}
U8 *MAlloc(I64 cnt,CTask *task=NULL) {
  U8 *hc;
  CMemUnused *m;
  if(task) {
//Heap?
    if(task->task_signature.u8[0]=='H') {
      hc=task;
      goto do_it;
    }
  }
  if(task)
    hc=task->data_heap;
  else
    hc=Fs->data_heap;

do_it:
  m=__MAlloc(cnt,hc);
  if(m) {
    m[-1].caller1=Caller(2);
    m[-1].caller2=Caller(3);
  }
  return m;
}
U8 *CAlloc(I64 cnt,CTask *task=NULL) {
  U8 *hc;
  CMemUnused *m;
  if(task) {
//Heap?
    if(task->task_signature.u8[0]=='H') {
      hc=task;
      goto do_it;
    }
  }  if(task)
    hc=task->data_heap;
  else
    hc=Fs->data_heap;
do_it:
  m=__CAlloc(cnt,hc);
  if(m) {
    m[-1].caller1=Caller(2);
    m[-1].caller2=Caller(3);
  }
  return m;
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
  CMemUnused *hc,*ret;
  I64 cnt=MSize(ptr);
  if(task) {
//Heap?
    if(task->task_signature.u8[0]=='H') {
      hc=task;
      goto do_it;
    }
  }  if(task)
    hc=task->data_heap;
  else
    hc=Fs->data_heap;
do_it:
  ret=__MAlloc(cnt,hc);
  MemCpy(ret,ptr,cnt);
  if(ret) {
    ret[-1].caller1=Caller(2);
    ret[-1].caller2=Caller(3);
  }
  return ret;
}

U8 *AMAllocIdent(U8 *ptr) {
  return MAllocIdent(ptr,adam_task);
}

U8 *MAllocAligned(I64 size,I64 alignment,
       CTask *mem_task=NULL,I64 misalignment=0)
{//Only powers of two alignment. This is awful.
  I64 mask=alignment-1;
  U8 *ptr=MAlloc(size+mask+sizeof(I64)+misalignment,mem_task),
       *res=(ptr+sizeof(I64)+mask)&~mask+misalignment;
  res(I64 *)[-1]=ptr-res;
  return res;
}

CTask *aiwnios_boot_task=__CAlloc(sizeof CTask);
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
#include "KHashB.HC"
#include "KTask.HC"
#include "MultiProc.HC"
#include "KExcept.HC"
#include "Job.HC"
#include "KDataTypes.HC"
#include "Compress.HC"
#include "StrB.HC"
#include "KDefine.HC"
#include "StrScan.HC"
#include "QSort.HC"
#include "MakeBlkDev.HC"
adam_task=aiwnios_boot_task;
SetFs(adam_task);
TaskInit(adam_task,0);
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
#include "Profiler.HC"
#ifdef TARGET_X86
#include "UAsmX64.HC"
#include "AsmX64.HC"
#endif
#include "KUtils.HC"
#include "KMain.HC"
U0 SCRoutine() {
  CTask *self=CAlloc(sizeof CTask,adam_task);
  SetFs(self);
  TaskInit(Fs,0);
  Fs->task_signature=TASK_SIGNATURE_VAL;
  Gs->seth_task=Fs;
  LBts(&(Gs->ready),0);
  CoreAPSethTask;
}
MakeContext(adam_task->context_save_area,NULL,adam_task->stk);
U0 InitCPUs() {
  I64 idx;
  MemSet(&cpu_structs,0,sizeof(CCPU) *64);
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
  for(idx=1;idx<mp_cnt;idx++) {
    c=&cpu_structs[idx];
    c->addr=c;
    c->num=idx;
    c->idle_factor=0.01;
    QueInit(&c->next_dying);
    SpawnCore(&SCRoutine,c,idx);
    do __Sleep(2);
    while (!Bt(&c->ready,0));
  }
 }
InitCPUs;
#ifndef NO_BOOT
SetFs(Gs->seth_task);
#endif
#ifdef STAGE1
#ifndef BOOTSTRAP
//Embed them so the user cant mess it up
U8 *KernelAHH;
//Seperate to avoid weird bug with running #exe block twice
KernelAHH=(#exe {ExeEmbedFile("/Src/KernelA.HH");});;
#endif
#endif
//For use later(ABOVE)
U0 ExeEmbedFile(U8 *fn) {
//During Aiwnios bootstapping,only Functions  are imported and not macros
  DbgPrint("Embedding File \"%Q\"(may take some time)\n",fn);
  U8 *ptr,*buf,*ptr2;
  CArcCompress *comp;
  I64 len,len2,b;
  ptr=FileRead(fn,&len);
  comp=CompressBuf(ptr,len+1); //+1 for NULL terminator
  Free(ptr);
  ptr=comp;
  len=comp->compressed_size;
  ptr2=buf=MAlloc(4*len+1); //\xff(4 chars )
  StreamPrint("ExpandBuf(");
  while(len>0) {
    StrPrint(ptr2,"\\x%02x",*ptr++);
    ptr2+=4;
    len--;
  }
  *ptr2=0;
  StreamPrint("\"%s\"\n",buf);
  StreamPrint(")");
  Free(comp);
}

U0 ImportSymbolsCB(U8 *name,U8 *addr) {
  CHashExport *find=HashFind(name,Fs->hash_table,-1);
  I64 cnt=1;
  if(find&&find->type&HTT_GLBL_VAR) {
    find(CHashGlblVar*)->data_addr=addr;
  } else if(find&&find->type&HTT_FUN) {
    if(!(find(CHashFun*)->flags&1<<Ff_INTERNAL)) {
      find(CHashFun*)->exe_addr=addr;
      LBtr(&find(CHashFun*)->flags,Cf_EXTERN);
    }
  } 
  #ifdef GEN_HEADERS
  else if(1) {
    DocPrint(unfound,"%s\n",name);
  }
  #endif
  else {
    find=CAlloc(sizeof(CHashExport),adam_task);
    find->str=AStrNew(name);
    find->type=HTT_EXPORT_SYS_SYM;
    find->val=addr;
    HashAdd(find,Fs->hash_table);
  }
}
#ifdef GEN_HEADERS
CDoc *unfound=DocNew("unfound.DD");
#endif
U0 InitSymbols() {
#ifdef TARGET_X86
  ExePrint("#define TARGET_X86\n;");
#endif
#ifdef TARGET_AARCH64
  ExePrint("#define TARGET_AARCH64\n;");
#endif
#ifdef TARGET_RISCV
  ExePrint("#define TARGET_RISCV\n;");
#endif
#ifdef STAGE1
  ExePutS(KernelAHH,"/Src/KernelA.HH");
  ImportSymbolsToHolyC(&ImportSymbolsCB);
  if(FileFind("/HCRT2.DBG.Z")) DbgInfoRead("/HCRT2.DBG.Z");
#else
//BOOTSTRAP is defined
  ExeFile("T:/Src/KernelA.HH");
  ImportSymbolsToHolyC(&ImportSymbolsCB);
#endif
  #ifdef GEN_HEADERS
  DocWrite(unfound);
  DocDel(unfound);
  #endif
}
InitSymbols;

#ifdef BOOTSTRAP
Cd("Src");
U8 *body=
	MStrPrint("#define IMPORT_AIWNIOS_SYMS 1\n"
	"#define TEXT_MODE 0\n"
	#ifdef TARGET_X86
	"#define TARGET_X86 1\n"
	#endif
	#ifdef TARGET_AARCH64
	"#define TARGET_AARCH64 1\n"
	#endif
	#ifdef TARGET_RISCV
	"#define TARGET_RISCV 1\n"
	#endif
	"#define HOST_ABI '%c'\n"
	"#define STAGE1 1\n"
	"#include \"HCRT_TOS.HC\";;\n",
	HOST_ABI
);
//Put this in a scope to do this all at once
{
	FileWrite("STAGE1.HC",body,StrLen(body));
	Cmp("STAGE1.HC","../HCRT2.DBG.Z","../HCRT2.BIN");
	ExitAiwnios;
}
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
#include "KDbg.HC"
#include "ADbg.HC"
#include "ARegistry.HC"
#include "CHash.HC"
#include "AHash.HC"
#include "AMath.HC"
#include "AMathODE.HC" 
#include "InFile.HC" 
#include "TaskRep.HC"
#include "Diff.HC" 
#include "Merge.HC" 
#include "DocUtils.HC" 
#include "MakeISOFile.HC";
#ifdef TARGET_AARCH64
#include "UAsmARM64.HC"
#endif
#ifdef TARGET_RISCV
#include "UAsmRiscV.HC"
#include "AsmRISCV.HC"
#endif
#include "HomeKeyPlugIns.HC"
if(!IsCmdLineMode&&!IsCmdLineMode2)
  DrawWindowNew;
#ifdef GEN_HEADERS
#include "GenHeader.HC";;
#endif
Gs->seth_task=adam_task;
if(IsCmdLineMode2)
  AiwniosTUIEnable;
if(!IsCmdLineMode)
  sys_winmgr_task=Spawn(&WinMgrTask,NULL,"Window Mgr");
U0 UserStartUp()
{//Run each time a user a spawned
  DocTermNew;
  Type("::/Doc/Start.DD");
  LBts(&(Fs->display_flags),DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  //Dir;
  //"$TX,"",D="DD_TEMPLEOS_LOC"$LOC:";
  //CPURep;
}
PaletteSetStd;
U8 *_CmdLineGetStr(U8 *st) {
  U8 *ret,*dc=DirCur,*pmt=MStrPrint("%s> ",dc);
  ret=CmdLineGetStr(pmt); 
  Free(dc),Free(pmt);
  return ret;
}
U0 Exit0() {
  ExitAiwnios(0);
}
U0 CmdLine(I64) {
  CTask *cmd_line;
  I64 cnt=0,old=-1;
  Fs->task_end_cb=&Exit0;
  for(cnt=0;cnt!=CmdLineBootFileCnt;cnt++)
     ExePrint2(
//TRick into thinking it is a valid CMD_LINE
       "Fs->last_cc->flags|=CCF_CMD_LINE;"
       "#include \"%s\";;",CmdLineBootFiles[cnt]);
  Fs->task_end_cb=NULL;
//
//  Properly handle __CMD_LINE__
//
  Spawn(&UserCmdLine)->task_end_cb=&Exit0;
}
#include "AiwniosPack.HC";
AiwniosPackLoad; //Does nothing if not aiwnios-packed
if(IsCmdLineMode) {
  fp_getstr2=&_CmdLineGetStr;
  Spawn(&CmdLine);
  CoreAPSethTask;
} else
  WinMax(User);
if(!IsCmdLineMode) {
  ExePrint2("#include \"~:/Src/WallPaper.HC\";WallPaperInit;;");
  ExePrint2("#include \"~:/Src/AMouse.HC\";");
  if (_SixtyFPS)
    SetFPS(60.);
  ACInit("~:/Src/*.HC*;*.HH*");
  OnceExe;
}
CoreAPSethTask;
#endif;
