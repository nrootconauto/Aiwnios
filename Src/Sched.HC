U64 SYS_CTRL_ALT_FLAGS=0; //In KGlbls?

//Sched.
//This is put in Sched.HC as it use it as a replacement for CLI
Bool BreakLock(CTask *task=NULL)
{//Disables <CTRL-ALT-c>.
  if (!task) task=Fs;
  return !LBts(&task->task_flags,TASKf_BREAK_LOCKED);
}

//Ditto
U0 Break()
{//Send <CTRL-ALT-c>.
  if (Bt(&(Fs->task_flags),TASKf_BREAK_TO_SHIFT_ESC))
    Msg(MSG_KEY_DOWN,CH_SHIFT_ESC,0x20100000201);
  else {
    Fs->wake_jiffy=0;
    TaskRstAwaitingMsg;
    DrvsRelease();
    BlkDevsRelease();
    FlushMsgs;
    throw('Break');
  }
}

//Ditto
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


U0 TaskFocusNext()
{
  CTask *task,*_task=sys_focus_task;
  sys_focus_task=NULL;
  if (!_task) _task=adam_task;
  task=_task->next_task;
  do {
    if (!Bt(&task->win_inhibit,WIf_SELF_FOCUS)) {
      sys_focus_task=task;
      WinToTop(sys_focus_task);
      return;
    }
    task=task->next_task;
  } while (task!=_task);
}
#define TaskContextSave AIWNIOS_SetJmp(Fs->context_save_area)
U0 TaskContextRestore(CTask *task) {
  CBpt *bp;
  //TODO increase Gs->swap_center;
  if(!Bt(&task->task_flags,TASKf_DISABLE_BPTS)) {
    for(bp=task->bpt_lst;bp;bp=bp->next) {
      //https://github.com/scottt/debugbreak 
      *bp->addr=0xd4200000;
    }
  }
  task->swap_cnter++;
  SetFs(task);
  AIWNIOS_LongJmp(task->context_save_area);
}
extern CTask *TaskEnd(); 
U0 Yield() {
  CTask *task=Fs;
  CBpt *bp;
  Gs->last_yield=cnts.jiffies=__GetTicks;
  if(!Bt(&task->task_flags,TASKf_DISABLE_BPTS)) {
    for(bp=task->bpt_lst;bp;bp=bp->next) {
      *bp->addr=bp->val;
    }
  }
  if(TaskContextSave) {
    return;
  }  
  if(SYS_CTRL_ALT_FLAGS
    &(1<<CTRL_ALT_DEL|1<<CTRL_ALT_TAB|1<<CTRL_ALT_X|1<<CTRL_ALT_C)) {
    if(Gs->num) {
      goto restore;
    }
    if(LBtr(&SYS_CTRL_ALT_FLAGS,CTRL_ALT_TAB)) {
      if(sys_focus_task) {
        TaskFocusNext;
        goto restore;
      }
    }
    if(LBtr(&SYS_CTRL_ALT_FLAGS,CTRL_ALT_X)&&sys_focus_task) {
      LBts(&sys_focus_task->task_flags,TASKf_KILL_TASK);
      task=sys_focus_task;
      SetFs(sys_focus_task);
      task=TaskEnd();
      goto restore;
    }
    if(LBtr(&SYS_CTRL_ALT_FLAGS,CTRL_ALT_C)) {
      if(!sys_focus_task) {
        goto restore;
      }
      task=sys_focus_task;
      if(Bt(&task->win_inhibit,WIf_SELF_FOCUS)) {
        goto restore;
      }
      if(Bt(&task->task_flags,TASKf_BREAK_LOCKED)) {
        LBts(&task->task_flags,TASKf_PENDING_BREAK);
        goto restore;
      }
      TaskContextSetRIP(task->context_save_area,&Break);
      if(Bt(&task->task_flags,TASKf_BREAK_TO_SHIFT_ESC)) {
        goto restore;
      }
      goto restore;
    }
  }
not_seth:
next:
  task=task->next_task;
  if(task==Gs->seth_task)
    goto restore;
  if(Bt(&task->task_flags,TASKf_KILL_TASK)) {
    SetFs(task);
    task=TaskEnd();
    goto next;
  }
  if(task->wake_jiffy>__GetTicks) {
    goto next;
  }

  if(!(task->task_flags&(1<<TASKf_AWAITING_MSG|1<<TASKf_SUSPENDED))) {
    goto restore;
  } else
    goto next;
restore:
  SetFs(task);
  TaskContextRestore(task);
}

U0 TaskQueIns(CTask *task,CTask *pred=NULL)
{//Insert a task in the scheduler running task queue.
//You have no business with this, probably.
  CTask *last;
  Bool bl=BreakLock;
  if (!pred) pred=Fs;
  last=pred->last_task;
  last->next_task=pred->last_task=task;
  task->last_task=last;
  task->next_task=pred;
  if(bl)
    BreakUnlock;
}

U0 TaskQueRem(CTask *task)
{//Remove a task from the scheduler running task queue.
//Use $LK,"Suspend",A="MN:Suspend"$().
  CTask *next,*last;
  Bool bl=BreakLock;
  next=task->next_task;
  last=task->last_task;
  last->next_task=next;
  next->last_task=last;
  if(bl)
    BreakUnlock;
}

U0 TaskQueInsChild(CTask *task)
{
  CTask *last,*pred;
  Bool bl=BreakLock;
  pred=task->parent_task->last_child_task;
  last=pred->last_sibling_task;
  last->next_sibling_task=pred->last_sibling_task=task;
  task->last_sibling_task=last;
  task->next_sibling_task=pred;
  if(bl)
    BreakLock;
}
