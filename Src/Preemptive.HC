U0 AiwniosMutexLock(I64 *ptr,I64 bit=0,I64 sleep=300) {
  I64 v,at;
  while(LBts(ptr,bit)) {
    at=__GetTicksHP;
    v=*ptr;
    AiwniosPreemptWait(ptr,v,sleep);
    Fs->slept_jiffies+=__GetTicksHP-at;
  }
}
U0 AiwniosMutexUnlock(I64 *ptr,I64 bit=0) {
  LBtr(ptr,bit);
  AiwniosPreemptWake(ptr,1);
}
U0 AiwniosMutexWakeAlll(I64 *ptr,I64 bit) {
  LBtr(ptr,bit);
  AiwniosPreemptWake(ptr,I32_MAX);
}
U0 AiwniosSleep(I64 sleep_hp) {
  I64 at=__GetTicksHP;
  LBts(&(Fs->wake_futex),0);
  AiwniosPreemptWait(&(Fs->wake_futex),1,sleep_hp);
  LBtr(&(Fs->wake_futex),0);
  Fs->slept_jiffies+=__GetTicksHP-at;
}
U0 AiwniosAwakeTask(CTask *t) {
  LBtr(&(Fs->wake_futex),0);
  AiwniosPreemptWake(&(Fs->wake_futex),1);
}