U0 CoreAPSethTask()
{
  CJobCtrl *ctrl=&(Fs->srv_ctrl);
  Bool bl;
  U8 *ul;
  CTask *task,task1;
  I64 t,last_load,slept,total,last_jiffies;
  F64 ns;
  while (TRUE) {
    last_jiffies=__GetTicksHP;
    bl=BreakLock;
    do {
      TaskKillDying;
      //Dirty fix by nroot.
      //Messages will return a JOBT_DONE(0) from a JobRunOne
      //Seth task's dont need messages so i will flush them
      FlushMsgs;
      AiwniosMutexLock(&ctrl->flags,JOBCf_LOCKED);
    } while (ctrl->next_waiting!=ctrl && JobRunOne(0,ctrl));
    if(bl)
      BreakUnlock;
    LBts(&(Fs->task_flags),TASKf_AWAITING_MSG);
    AiwniosMutexUnlock(&ctrl->flags,JOBCf_LOCKED);
    LBts(&(Fs->task_flags),TASKf_IDLE);
    ns=0.1*JIFFY_FREQ*1000,t=__GetTicksHP;
#ifndef MP_PREEMPTIVE
    task1=Fs;
    for(task=Fs->next_task;task!=task1;task=task->next_task) {
      if(Bt(&task->task_flags,TASKf_SUSPENDED)||Bt(&task->task_flags,TASKf_AWAITING_MSG))
        goto next;
      if(task->wake_jiffy-t<ns)
        ns=task->wake_jiffy-t;
next:;
    }
    if(ns>0) {
      Gs->idle_pt_hits+=ns;
      MPSleepHP(ns);
    }
    Gs->total_jiffies=__GetTicksHP;
    Yield;
#else
    task1=Fs;
    MPSleepHP(10000);
    ns=1.;
    for(task=Fs->next_task;task!=task1;task=task->next_task) {
      if(task->born_jiffy) {
	total=__GetTicksHP-task->born_jiffy;
        ns=(ns+ToF64(task->slept_jiffies)/total)/2.;
      }
    }
    Gs->total_jiffies=__GetTicksHP;
//Approximate
    Gs->idle_pt_hits=ns*Gs->total_jiffies;
#endif
    LBtr(&(Fs->task_flags),TASKf_IDLE);
  }
}
