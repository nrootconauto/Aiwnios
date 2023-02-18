#pragma once
#include "aiwn.h"
#include <setjmp.h>
#include <ucontext.h>
//See AIWNIOS_enter_try t in except_*.s
CExceptPad* __enter_try()
{
	CExcept* ex = A_MALLOC(sizeof(CExcept), Fs);
	QueIns(ex, Fs->except->last);
	Fs->catch_except = 0;
	return &ex->ctx;
}
extern int64_t AIWNIOS_enter_try();
extern void AIWNIOS_throw(uint64_t code);
CExceptPad* __throw(uint64_t code)
{
	Fs->except_ch = code;
	CExcept* ex;
	Fs->catch_except = 0;
	Fs->except_ch = code;
	QueRem(ex = Fs->except->last);
	Fs->throw_pad = ex->ctx;
	A_FREE(ex);
	return &(Fs->throw_pad);
}
void AIWNIOS_ExitCatch()
{
	if (Fs->catch_except) {
		Fs->catch_except = 0;
	} else
		throw(Fs->except_ch);
}
