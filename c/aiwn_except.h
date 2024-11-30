#pragma once
#include "aiwn_que.h"
#include <setjmp.h>
#include <stdint.h>
typedef struct CExceptPad {
  int64_t gp[30 - 18 + 1];
  double fp[15 - 8 + 1];
} CExceptPad;
typedef struct CExcept {
  CQue base;
  jmp_buf ctx;
} CExcept;

#define try                                                                    \
  {                                                                            \
    if (AIWNIOS_enter_try()) {
#define catch                                                                  \
  (body)                                                                       \
  }                                                                            \
  else {                                                                       \
    body;                                                                      \
    AIWNIOS_ExitCatch();                                                       \
  }                                                                            \
  }
void AIWNIOS_throw(uint64_t code);
#define throw AIWNIOS_throw

void AIWNIOS_ExitCatch();
jmp_buf *__throw(uint64_t code);
void AIWNIOS_throw(uint64_t code);
int64_t AIWNIOS_enter_try();
jmp_buf *__enter_try();
