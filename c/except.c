#include "aiwn_asm.h"
#include "aiwn_except.h"
#include "aiwn_hash.h"
#include "aiwn_mem.h"
#include "aiwn_multic.h"
#include "aiwn_que.h"
#include <setjmp.h>
#include <string.h>
// See AIWNIOS_enter_try t in except_*.s
jmp_buf *__enter_try() {
  CExcept *ex = A_MALLOC(sizeof(CExcept), Fs);
  QueIns(ex, Fs->except->last);
  Fs->catch_except = 0;
  return &ex->ctx;
}
jmp_buf *__throw(uint64_t code) {
  Fs->except_ch = code;
  CExcept *ex;
  Fs->catch_except = 0;
  Fs->except_ch = code;
  QueRem(ex = Fs->except->last);
  memcpy(Fs->throw_pad, ex->ctx, sizeof(ex->ctx));
  A_FREE(ex);
  return &(Fs->throw_pad);
}
void AIWNIOS_ExitCatch() {
  if (Fs->catch_except) {
    Fs->catch_except = 0;
  } else
    throw(Fs->except_ch);
}

int64_t AIWNIOS_enter_try() {
  return !setjmp(*__enter_try());
}

void AIWNIOS_throw(uint64_t c) {
  static CHashExport *exp;
  if (!exp)
    exp = HashFind("throw", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1);
  FFI_CALL_TOS_1(exp->val, c);
  __builtin_unreachable();
}
