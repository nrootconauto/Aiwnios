U0 CoreAPSethTask()
{
  CJobCtrl *ctrl=&(Fs->srv_ctrl);
  Bool bl;
  U8 *ul;
  CTask *task,task1;
  I64 mS,t;
  while (TRUE) {
    bl=BreakLock;
    do {
      TaskKillDying;
      //Dirty fix by nroot.
      //Messages will return a JOBT_DONE(0) from a JobRunOne
      //Seth task's dont need messages so i will flush them
      FlushMsgs;
      while (LBts(&ctrl->flags,JOBCf_LOCKED))
        ; //TODO atomic lock
    } while (ctrl->next_waiting!=ctrl && JobRunOne(0,ctrl));
    if(bl)
      BreakUnlock;
    LBts(&(Fs->task_flags),TASKf_AWAITING_MSG);
    LBtr(&(ctrl->flags),JOBCf_LOCKED);
    LBts(&(Fs->task_flags),TASKf_IDLE);
    mS=0.1*JIFFY_FREQ,t=__GetTicks;
    task1=Fs;
    for(task=Fs->next_task;task!=task1;task=task->next_task) {
      if(Bt(&task->task_flags,TASKf_SUSPENDED)||Bt(&task->task_flags,TASKf_AWAITING_MSG))
        goto next;
      if(task->wake_jiffy-t<mS)
        mS=task->wake_jiffy-t;
next:;
    }
    if(mS>0) {
      Gs->idle_pt_hits+=mS;
      __Sleep(mS);
    }
    Gs->total_jiffies=__GetTicks;
    Yield;
    LBtr(&(Fs->task_flags),TASKf_IDLE);
  }
}
