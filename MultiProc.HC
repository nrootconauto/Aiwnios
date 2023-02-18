U0 CoreAPSethTask()
{
  CJobCtrl *ctrl=&(Fs->srv_ctrl);
  Bool bl;
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
    Yield;
    LBtr(&(Fs->task_flags),TASKf_IDLE);
  }
}
