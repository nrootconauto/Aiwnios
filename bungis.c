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
#include <ucontext.h>
struct CHeapCtrl;
struct CHashTable;
__thread struct CTask* Fs;
__thread struct CTask* HolyFs;
void TaskExit()
{
	// TODO implement me
	exit(0);
}

extern struct CHeapCtrl* HeapCtrlInit(struct CHeapCtrl* ct, CTask* task);
void TaskInit(CTask* task, void* addr, int64_t stk_sz)
{
	if (!stk_sz)
		stk_sz = 512 * (1 << 9);
	CExcept* except;
	memset(task, 0, sizeof(CTask));
	task->heap = HeapCtrlInit(NULL, task);
	task->except = A_MALLOC(sizeof(CQue), task);
	task->stack = A_MALLOC(stk_sz, task);
	QueInit(task->except);
	except = A_MALLOC(sizeof(CExcept), task);
	task->ctx.uc_stack.ss_sp = task->stack;
	task->ctx.uc_stack.ss_size = stk_sz;
	task->exit_ctx.uc_stack.ss_sp = task->stack;
	task->exit_ctx.uc_stack.ss_size = stk_sz;
	makecontext(&task->exit_ctx, &TaskExit, 0);
	makecontext(&task->ctx, addr, 0);
	memcpy(&except->ctx, &task->exit_ctx, sizeof(ucontext_t));
	QueIns(except, task->except);
	task->hash_table = HashTableNew(1 << 10, task);
	if (Fs && Fs != task)
		task->hash_table->next = Fs->hash_table;
}
