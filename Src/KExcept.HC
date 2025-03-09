U0 PutExcept(Bool catch_it=TRUE)
{//Print exception msg and catch exception.
  "Except:%c:",Fs->except_ch;
  "%P:%P:%P:%P:%P:%P\n",Fs->except_callers[0],Fs->except_callers[1],
	Fs->except_callers[2],Fs->except_callers[3],Fs->except_callers[4],
	Fs->except_callers[5],Fs->except_callers[6],Fs->except_callers[7];
  Fs->catch_except=catch_it;
}

//This returns a pointer to the except pad
U8 *SysTry()
{
  CExcept *tmpt=MAlloc(sizeof(CExcept));
  QueIns(tmpt,Fs->last_except);
  return &tmpt->AARCH64_except_pad;
}

U0 SysUntry()
{
  CExcept *tmpt=Fs->last_except;
  QueRem(tmpt);
  Free(tmpt);
}

U0 EndCatch() {
  if(!Fs->catch_except)
    throw(Fs->except_ch);
}

//
// Nroot here,See PrsTry in parser.c
//
//try 
//  A;
//catch
//  B;
//
//if(!AIWNIOS_SetJmp(SysTry)) {
//  A;
//  SysUntry;
//} else {
//  B;
//  EndTry;
//}
U0 throw(I64 ch=0,Bool no_log=FALSE)
{//ch can be up to 8 chars like $LK,"PutChars",A="MN:PutChars"$().
//In the catcher, fetch ch from Fs->except_ch.
  CExcept *tmpt=Fs->last_except;
  I64 except_pad[2048];
  if(tmpt==&(Fs->next_except)) goto panic;
  MemCpy(&except_pad,tmpt->AARCH64_except_pad,2048);
  SysUntry;
  Bool was_raw;
  I64 i;
  Fs->except_ch=ch;
  for (i=0;i<TASK_EXCEPT_CALLERS;i++)
    Fs->except_callers[i]=Caller(i+2);
  Fs->catch_except=FALSE;
  if (!no_log)
    ;
    /*AdamLog("Except:%c:%p:%p:%p:%p:%p:%p\n",ch,Fs->except_callers[0], TODO RESTORE
    Fs->except_callers[1],Fs->except_callers[2],Fs->except_callers[3],
	  Fs->except_callers[4],Fs->except_callers[5],Fs->except_callers[6],
	  Fs->except_callers[7]);*/
  AIWNIOS_LongJmp(&except_pad);
panic:
  PutExcept(FALSE);
  //Panic("Unhandled Exception"); TODO RESTORE
  //Raw(was_raw);
}

Bool BreakLock(CTask *task=NULL)
{//Disables <CTRL-ALT-c>.
  if (!task) task=Fs;
  return !LBts(&task->task_flags,TASKf_BREAK_LOCKED);
}

Bool BreakUnlock(CTask *task=NULL)
{//Reenables <CTRL-ALT-c> and issues any pending breaks.
  Bool res;
  if (!task) task=Fs;
  res=LBtr(&task->task_flags,TASKf_BREAK_LOCKED);
  if (LBtr(&task->task_flags,TASKf_PENDING_BREAK)) {
    if (task==Fs)
      Break;
    else
      task->context_save_area[12]=&Break;
  }
  return res;
}
