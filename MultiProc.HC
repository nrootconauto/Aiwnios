U0 CoreAPSethTask()
{
  CJobCtrl *ctrl=&(Fs->srv_ctrl);
  Bool bl;
  CTask *task,task1;
  I64 ns,t;
  while (TRUE) {
    bl=BreakLock;
    do {
      TaskKillDying;
      while (LBts(&ctrl->flags,JOBCf_LOCKED))
        ; //TODO atomic lock
    } while (ctrl->next_waiting!=ctrl && JobRunOne(0,ctrl)); //TODO RESTORE
    if(bl)
      BreakUnlock;
    LBts(&(Fs->task_flags),TASKf_AWAITING_MSG);
    LBtr(&(ctrl->flags),JOBCf_LOCKED);
    LBts(&(Fs->task_flags),TASKf_IDLE);
    ns=0.1*JIFFY_FREQ,t=__GetTicks;
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
      __Sleep(ns);
    }
    Yield;
    LBtr(&(Fs->task_flags),TASKf_IDLE);
  }
}
