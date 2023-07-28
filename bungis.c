#pragma once
#include "aiwn.h"
#include <setjmp.h>
#include <stdlib.h>
// This doesn't 1:1 mirror TempleOS,when things get bootstaped , things may get
// a bit
// more likley
//
// Dec 4,2022. My phone ran out of minutes so I can't get tea
//
struct CHeapCtrl;
struct CHashTable;
__thread struct CTask *Fs;
__thread struct CTask *HolyFs;
void TaskExit() {
  // TODO implement me
  exit(0);
}

void TaskInit(CTask *task, void *addr, int64_t stk_sz) {
  if (!stk_sz)
    stk_sz = 512 * (1 << 9);
  CExcept *except;
  memset(task, 0, sizeof(CTask));
  task->heap = HeapCtrlInit(NULL, task, 1);
  task->except = A_MALLOC(sizeof(CQue), task);
  task->stack = A_MALLOC(stk_sz, task);
  QueInit(task->except);
  except = A_MALLOC(sizeof(CExcept), task);
  AIWNIOS_makecontext(
      &task->ctx, addr,
      (void *)(-0x10 & (int64_t)(task->stack + MSize(task->stack))));
  QueIns(except, task->except);
  task->hash_table = HashTableNew(1 << 10, task);
  if (Fs && Fs != task)
    task->hash_table->next = Fs->hash_table;
}
