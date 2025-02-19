//Added by nroot,used for Bounds checker
U8 *WhineOOB(U8 *ptr) {
  I64 oob=0;
  if(!BoundsCheck(ptr,&oob)) {
     DbgPrint("Memory access is %d bytes Out of Bounds!!!\n",oob);
     DbgPrint("%P\n%P\n%P\n%P\n%P\n",Caller(3),Caller(4),Caller(5),
     Caller(6),Caller(7));
  }
  return ptr;
}

public Bool ChkOnStk(U8 *ptr,CTask *task=NULL) {
  if(!task) task=Fs;
  return task->stk<=ptr<task->stk+MSize(task->stk);
}
U8 *__CallerBP(U8 **rbp,I64 num=1)
{//Returns the addr of the fun which called this one,
//or the caller of the caller, etc.
  U8 **ptr;
  while (num--) {
    if(!IsValidPtr(rbp)) return NULL;
    if (rbp>=*rbp)
      return NULL;
    rbp=*rbp;
    if (!ChkOnStk(rbp,Fs))
      return NULL;
  }
  ptr=rbp+1;
  if(!IsValidPtr(ptr)) return NULL;
  return *ptr;
}
U8 *TaskCaller(CTask *task=NULL,I64 num=0,Bool saved_context=FALSE)
{//Fetches addr of Nth caller on task's stk.
  U8 **ptr,**x29;
  if (!task) task=Fs;
  if (!saved_context && task==Fs)
    return Caller(num+1);
  if (!TaskValidate(task))
    return NULL;
#ifdef TARGET_X86
  x29=task->context_save_area[2];
#endif
#ifdef TARGET_AARCH64
  x29=task->context_save_area[11];
#endif
#ifdef TARGET_RISCV
  x29=task->context_save_area[2];
  if (num) {
    while (x29) {
      if(!ChkOnStk(x29,task))
        break;
      ptr=x29-1;
      if(!ptr) break;
      if (! --num)
	return *ptr;
      if (x29>=x29[-2])
	break;
      x29=x29[-2];
    }
    return NULL;
  } else {
  return task->context_save_area[0];
  }
#endif
  if (num) {
    while (x29) {
      if(!ChkOnStk(x29,task))
        break;
      ptr=x29+1;
      if(!ptr) break;
      if (! --num)
	return *ptr;
      if (x29>=*x29)
	break;
      x29=*x29;
    }
    return NULL;
  } else {
#ifdef TARGET_X86
    return task->context_save_area[0];
#else    
    return task->context_save_area[12];
#endif
  }
}
CTask *GetDebuggedProcess() {
  CHashGlblVar *dtask=HashFind("DFs",Fs->hash_table,HTT_GLBL_VAR);
  if(dtask) return dtask->data_addr(CTask**)[0];
  return Fs;
} 
U0 CallerRep(U8 **x29=NULL,CTask *task=NULL)
{//Prints a report of calling routines.
  I64 **ptr;
  if (!task) task=GetDebuggedProcess;
  #ifdef TARGET_X86
  if (!x29) {
    if (task==Fs)
      x29=Fs->context_save_area[2];
    else
      x29=task->context_save_area[2];
  }
  #endif
  #ifdef TARGET_AARCH64
  if (!x29) {
    if (task==Fs)
      x29=Fs->context_save_area[11];
    else
      x29=task->context_save_area[11];
  }
  #endif
  #ifdef TARGET_RISCV
  if (!x29) {
    if (task==Fs)
      x29=Fs->context_save_area[2];
    else
      x29=task->context_save_area[2];
  }
  while (ChkOnStk(x29,task)) {
    ptr=x29-1;
    "%08X:%08tX:%P\n",ptr,*ptr,*ptr;
    if (x29>=x29[-2])
      break;
    x29=x29[-2];
  }
  return;
  #endif

  "CallerRep:\n";
  while (ChkOnStk(x29,task)) {
    ptr=x29+1;
    "%08X:%08tX:%P\n",ptr,*ptr,*ptr;
    if (x29>=*x29)
      break;
    x29=*x29;
  }
}
U0 DebuggerSpawn(CTask *debugee) {  
  U8 *src_link;
  Fs->win_inhibit=WIG_USER_TASK_DFT;
  DocTermNew;
  LBts(&(Fs->display_flags),DISPLAYf_SHOW);
  "$$BK,1$$Debugging DFs(0x%X)$$BK,0$$\n",debugee->addr;
  "Type 'DbgHelp;' for help,it's a bit different than TempleOS.\n";
  #ifdef TARGET_X86
  "Oh,and welcome to %P(%s)\n",debugee->context_save_area[0],src_link=SrcEdLink(debugee->context_save_area[0]);
  #endif
  #ifdef TARGET_RISCV
  "Oh,and welcome to %P(%s)\n",debugee->context_save_area[0],src_link=SrcEdLink(debugee->context_save_area[0]);
  #endif
  #ifdef TARGET_AARCH64
  "Oh,and welcome to %P(%s)\n",debugee->context_save_area[22],src_link=SrcEdLink(debugee->context_save_area[22]);
  #endif
  "\n";
  Free(src_link);
  ExePrint("CTask *DFs=0x%X;;\n",debugee);
  WinToTop;
  WinZBufUpdate;
  UserTaskCont;
}

U0 DumbStackTrace(U8 **rbp,U8 *rip) {
  I64 cnt=1;
  DbgPrint("[0]::%P\n",rip);
#ifdef TARGET_RISCV
  while(ChkOnStk(rbp,Fs)) {
    DbgPrint("[%d]::%P\n",cnt++,rbp[-1]);
    rbp=rbp[-2];
  }

#else
  while(ChkOnStk(rbp,Fs)) {
    DbgPrint("[%d]::%P\n",cnt++,rbp[1]);
    rbp=*rbp;
  }
#endif
}
U0 AiwniosDbgCB(I64 sig,I64 *_gps) {
  AIWNIOS_SetCaptureMouse(FALSE); //If you freeze you are on your own
  I64 gps[33];
  MemCpy(gps,_gps,8*33);
  DebuggerClientStart(Fs,gps);
  CBpt *bp;
  CTask clone;
  CHashTable *old;
  I64 idx;
  #ifdef TARGET_X86
  I64 oip=gps[0],rbp=gps[2]; 
  if(HOST_ABI=='OpenBSD') {
    rbp=GetRBP;
    oip=Caller(0);
    gps[0]=oip;
    gps[2]=rbp;
  }
  #endif
  #ifdef TARGET_AARCH64
  I64 oip=gps[22],rbp=gps[11]; 
  #endif
  #ifdef TARGET_RISCV
  I64 oip=gps[0],rbp=gps[2]; 
  #endif
  #ifdef TARGET_RISCV
  I64 trap=4;
  #else
  I64 trap=5;
  #endif
  Fs->is_single_step=FALSE;
  if(Fs==sys_winmgr_task) {
    //Oh no,you crashed the window manager
    DbgPutS("You crashed the window manager,Ill try to restart it\n");
    old=Fs->hash_table;
    Fs->hash_table=sys_task_being_scrn_updated->hash_table;
    DumbStackTrace(rbp,oip);
    Fs->hash_table=old;
    sys_task_being_scrn_updated->draw_it=NULL;
    LBts(&sys_task_being_scrn_updated->task_flags,TASKf_KILL_TASK);
    sys_winmgr_task=Spawn(&WinMgrTask,NULL,"Window Mgr");
    Exit;
  }
  DumbStackTrace(rbp,oip);
  if(IsFastFail) ExitAiwnios(sig);
  if(Gs->num) Exit;
  MemCpy(Fs->context_save_area,gps,23*8);
  MemCpy(&clone,Fs,sizeof CTask);
  CTask *task=Spawn(&DebuggerSpawn,&clone,"Debugger",0,Fs);
  DeathWait(&task);
  if(sig!=trap)
    Exit;
  if(sig==trap) { //SIGTRAP
    //Allow us to move past the breakpoint
    bp=BptFind(oip,Fs);
    if(bp)
      *bp->addr=bp->val;
#ifdef TARGET_X86
//On some platforms RIP is put at the INT3 so dont move past it(if it's just sitting there)
    if(oip==clone.context_save_area[0]&&IsValidPtr(oip)) {
      if(oip(U8*)[0]==OC_BPT)
	clone.context_save_area[0]++; //Move past INT3
    }
#endif
  }
fin:
  Fs->task_flags=clone.task_flags;
  Fs->bpt_lst=clone.bpt_lst;
  for(idx=0;idx!=24;idx++)
     DebuggerClientSetGreg(Fs,idx,clone.context_save_area[idx]);
  WinRefocus;
  DebuggerClientEnd(Fs,clone.is_single_step);
}
CBpt *BptFind(U8 *needle_addr,CTask *haystack_task=NULL,Bool rem=FALSE)
{
  CBpt *res=NULL,*tmpb,*tmpb1,*tmpb2;
  Bool bl;
  if (!haystack_task) haystack_task=GetDebuggedProcess;
  bl=BreakLock;
  tmpb1=&haystack_task->bpt_lst;
  tmpb=haystack_task->bpt_lst;
  while (tmpb) {
    tmpb2=tmpb->next;
    if (tmpb->addr==needle_addr) {
      res=tmpb;
      if (rem)
	tmpb1->next=tmpb2;
      else
	tmpb1=&tmpb->next;
    } else
      tmpb1=&tmpb->next;
    tmpb=tmpb2;
  }
  if(bl)
    BreakUnlock;
  return res;
}

Bool BptS(U32 *addr,CTask *task=NULL,Bool live=TRUE)
{//Set breakpoint.
  CBpt *tmpb;
  Bool res=TRUE,bl;
  if (!task) task=GetDebuggedProcess;
  bl=BreakLock;
  if (!(tmpb=BptFind(addr,task,FALSE))) {
    tmpb=CAlloc(sizeof(CBpt),task);
    tmpb->addr=addr;
    tmpb->val=*addr;
    res=FALSE;
    tmpb->next=task->bpt_lst;
    task->bpt_lst=tmpb;
    if (task==GetDebuggedProcess && live)
#ifdef TARGET_X86
      addr(U8*)[0]=OC_BPT;
#endif
#ifdef TARGET_AARCH64
      //https://github.com/scottt/debugbreak
      *addr=0xd4200000;
#endif
#ifdef TARGET_RISCV
	  ;//*addr=0x00100073; TODO TODO
#endif
  }
  if(bl)
    BreakUnlock;
  return res;
}

Bool BptR(U8 *addr,CTask *task=NULL,Bool live=TRUE,Bool rem=TRUE)
{//Rem breakpoint.
  CBpt *tmpb;
  Bool res=FALSE,bl;
  if (!task) task=GetDebuggedProcess;
  bl=BreakLock;
  if (tmpb=BptFind(addr,task,rem)) {
    if (task==GetDebuggedProcess && live)
      *tmpb->addr=tmpb->val;
    res=TRUE;
    if (rem)
      Free(tmpb);
  }
  if(bl)
    BreakUnlock;
  return res;
}

Bool B(U8 *addr,CTask *task=NULL,Bool live=TRUE)
{//Toggle breakpoint.
//Return: TRUE if removed.
  Bool res=FALSE,bl;
  bl=BreakLock;
  if (BptFind(addr,task,FALSE)) {
    BptR(addr,task,live,TRUE);
    res=TRUE;
  } else
    BptS(addr,task,live);
  if(bl)
    BreakUnlock;
  return res;
}

I64 B2(CTask *task=NULL,Bool live=TRUE)
{//Rem all breakpoints.
//Return: cnt of removed.
  I64 res=0,bl;
  CBpt *tmpb,*tmpb1;
  if (!task) task=GetDebuggedProcess;
  bl=BreakLock;
  tmpb=task->bpt_lst;
  task->bpt_lst=NULL;
  while (tmpb) {
    tmpb1=tmpb->next;
    if (task==GetDebuggedProcess && live)
      *tmpb->addr=tmpb->val;
    Free(tmpb);
    tmpb=tmpb1;
    res++;
  }
  if(bl)
    BreakUnlock;
  return res;
}

U0 G(U8 *ip=INVALID_PTR,CTask *task=NULL)
{//Go
  if (!task) task=GetDebuggedProcess;
  if (ip!=INVALID_PTR) task->context_save_area[12]=ip;
  if (BptFind(task->context_save_area[12],task))
    "\nDo one of the following, first:\n"
	  ">S;\t\t\t//Single step\n"
	  ">B2;\t\t\t//Clear all break points\n"
	  ">G2;\t\t\t//Clear all break points and Go\n\n";
  else {
    LBtr(&task->task_flags,TASKf_DISABLE_BPTS);
    Exit;
  }
}

U0 G2(U8 *ip=INVALID_PTR,CTask *task=NULL)
{//Rem all breakpoints and Go.
  if (!task) task=GetDebuggedProcess;
  B2(task);
  G(ip,task);
}

U0 BList(CTask *task=NULL) {
  if(!task) task=GetDebuggedProcess;
  CBpt *bp=&task->bpt_lst;
  U8 *slink;
  for(bp=bp->next;bp;bp=bp->next) {
    "\t%P(%s)\n",bp->addr,slink=SrcEdLink(bp->addr);
    Free(slink);
  }
}

public Bool ViewFrameVars(I64 frame=0,I64 depth=2) {
  U8 **rbp;
  CTask *t=GetDebuggedProcess;
  CHash *fun;
  U8 *rip;
  I64 oframe=frame,dummy;
#ifdef TARGET_X86
  rbp=t->context_save_area[2];
  rip=t->context_save_area[0];
#endif
#ifdef TARGET_AARCH64
  rbp=t->context_save_area[11];
  rip=t->context_save_area[22];
#endif
  while(frame>0) {
    rip=rbp[1];
    rbp=*rbp;
    if(!ChkOnStk(rbp,t)) return FALSE;
    frame--;
  }
  fun=FunSegFind(rip,&dummy);
  if(!fun) return FALSE;
  FunRep(fun->str,rbp,depth,t);
  return TRUE;
}
U0 DbgHelp()
{
  "\n"
        "The cmd line is basically the same as TempleOS.	Here are some common\n"
        "debugging commands.\n\n"
        ">ClassRep(DFs,\"CTask\",1);\t//Dump current task record.\n"
        ">ClassRep(DFs,,1);\t\t//(It knows lastclass.)\n"
        ">CallerRep;\t\t\t//Stack trace report.\n"
        ">Da(DFs->rsp);\t\t\t//Dump stk.\n"
        ">Dr;\t\t\t\t//Dump Regs.\n"
        ">1+2*3+&Print;\t\t\t//Show calculation res.\n"
        ">*(0x70000)(I64 *)=0x123456789;\t//Assign value to 0x70000-0x70007.\n"
        ">DFs->_rax=0x1234;\t\t\t//Set RAX to 0x1234.\n"
        ">DFs->_rip=&Break;\t\t//Set RIP.\n"
        ">I64 i;\t\t\t\t//Declare variable.\n"
        ">i=DFs->rax+DFs->rbx;\t\t\t//Assign to variable.\n"
        ">U(&Print+0x8);\t\t\t//Unassemble Print.\n"
        ">Uf(\"Print\");\t\t\t//Unassembler function \"Print\".\n"
        ">Man(\"Print\");\t\t\t//Edit Src for \"Print\".\n"
        ">E(DFs->rip);\t\t\t//Edit Src Code.\n"
        ">B(&Main+0x20);\t\t\t//Toggle break point.\n"
        ">B2;\t\t\t\t//Clear all break points.\n"
        ">S;\t\t\t\t//Single step.\n"
        ">G;\t\t\t\t//Resume execution.\n"
        ">G2;\t\t\t\t//B2;VGAFlush;WinFocus;G;\n"
        ">ViewFrameVars;\t\t\t//Examine the variables at a frame\n"
        ">Exit;\t\t\t\t//Exit debugger.\n\n";
}
public U0 S(U8 *ip=INVALID_PTR,CTask *task=NULL) //Single-step.
{//Single step.
  Bool bl=BreakLock;
  if (!task) task=GetDebuggedProcess;
  if (ip!=INVALID_PTR) task->context_save_area[0]=ip;
  task->is_single_step=TRUE;
  if(bl) BreakUnlock;
  Exit;
}
/*U0 StkRep(CTask *task=NULL)
{//Reports whats on the stk.
  I64 i,j,addr,
	**rbp,**rsp,*my_rsp[STK_REP_LEN],bl;
  CHashTable *old_hash=Fs->hash_table;
  if (!task) task=GetDebuggedProcess;
  if (!TaskValidate(task))
    return;
  bl=BreakLock;
  if (task==Fs) {
    rbp=GetRBP;
    rsp=rbp+3;
#ifndef TARGET_RISCV
    rbp=*rbp;
#endif
#ifdef TARGET_RISCV
    rbp=rbp[1];
#endif
  } else {
    rsp=task->rsp;
    rbp=task->rbp;
  }
  if (task->rip==_RET)
    addr=*rsp;
  else
    addr=task->rip;
  MemCpy(my_rsp,rsp,STK_REP_LEN*sizeof(U8 *));
  if(bl)
    BreakUnlock;
  Fs->hash_table=task->hash_table;
  for (i=0;i<STK_REP_LEN;i++) {
    "%08X [RSP+%04X]: %016X ",rsp+i,
	  i*sizeof(U8 *),my_rsp[i];
    while (TRUE) {
      if (!(&task->stk->stk_base<=rbp<
	    (&task->stk->stk_base)(U8 *)+task->stk->stk_size))
	break;
      j=rbp-rsp;
      if (j>=i)
	break;
      addr=my_rsp[j+1];
      if (rbp>=my_rsp[j])
	break;
      rbp=my_rsp[j];
    }
    if (my_rsp[i]==addr)
      "$$RED$$";
    "%P$$FG$$\n",my_rsp[i];
  }
  '\n';
  Fs->hash_table=old_hash;
}*/

U0 Dr(CTask *task=NULL)
{//Dump regs
  I64 i;
  U64 *ptr;
  if (!task) task=GetDebuggedProcess;
#ifdef TARGET_X86
  for (i=0;i<16;i++)
#endif
#ifdef TARGET_AARCH64
  for (i=0;i<32;i++)
#endif
#ifdef TARGET_RISCV
  for (i=0;i<32;i++)
#endif
    if(ptr=TaskRegAddr(task,i)){
      "%3Z:%016X\n",i,"ST_U64_REGS",*ptr;
    }
#ifdef TARGET_AARCH64
  "RIP:%016X\n",task->context_save_area[22];
#else
  "RIP:%016X\n",task->context_save_area[0];
#endif
}
U0 D(U8 *addr,I64 cnt=0x80,Bool show_offset=TRUE)
{//Dump mem, showing offsets.
//See $LK,"DocD",A="MN:DocD"$() for a live dump.
  I64 i,j,ch;
  U8 *ptr=addr;
  while (cnt) {
    if (show_offset)
      "%08X",ptr-addr;
    else
      "%010X",ptr;
    if (cnt>16)
      j=16;
    else
      j=cnt;
    for (i=0;i<j;i++)
      "%02X ",ptr[i];
    for (;i<16;i++)
      "";
    for (i=0;i<j;i++) {
      ch=ptr[i];
      if (ch<CH_SHIFT_SPACE || ch==CH_BACKSPACE)
	ch='.';
      '' ch;
      if (ch=='$$')
	'' ch;
    }
    '\n';
    cnt-=j;
    ptr+=j;
  }
}

U0 Dm(U8 *addr,I64 cnt=0x80)
{//Show mem addr, not offsets.
  D(addr,cnt,FALSE);
}

U0 Da(U8 **addr,I64 cnt=0x10)
{//Dump mem, showing symbolic addresses.
  while (cnt-->0) {
    "%08X:%08X,%P\n",addr,*addr,*addr;
    addr++;
  }
}

U0 HeapWho() {
//Dont use current heapctrl for the poo poo suace(we will modify the heap possibly during print)
  CHeapCtrl *old=Fs->data_heap;
  CHeapCtrl *dummy=HeapCtrlInit(NULL,Fs,0);
  CHeapCtrlArena *arena;
  I64 a;
  CDoc *poop=DocNew; //MAke a dummy document because we are "swapping" CHeapCtrls
  CMemUsed *used,*head;
  Fs->data_heap=dummy;
  while(LBts(&old->locked_flags,1))
    Sleep(1);
  for(a=0;a!=16;a++) {
    arena=&old->arenas[a];
    head=&arena->used_mem;
    used=head->next;
    while(used!=head) {
      DocPrint(poop,"%X(%d) from %P,%P\n",used+1,used->size,used->caller1,used->caller2);
      used=used->next;
    }
  }
  LBtr(&old->locked_flags,1);
  Fs->data_heap=old;
  DocInsDoc(DocPut(Fs),poop);
  HeapCtrlDel(dummy);
}
