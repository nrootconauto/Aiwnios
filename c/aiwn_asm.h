#pragma once
#include <stdint.h>
#ifdef _WIN32
extern void Misc_ForceYield();
#endif
#ifndef __x86_64__
extern int64_t Bt(void *ptr, int64_t);
extern int64_t Btc(void *ptr, int64_t);
extern int64_t Btr(void *ptr, int64_t);
extern int64_t Bts(void *ptr, int64_t);
extern int64_t LBt(void *ptr, int64_t);
extern int64_t LBtc(void *ptr, int64_t);
extern int64_t LBtr(void *ptr, int64_t);
extern int64_t LBts(void *ptr, int64_t);
#else
#  define __BITOPR(inst, mem, idx)                                             \
    ({                                                                         \
      _Bool __ret = 0;                                                         \
      asm(#inst "\t%[Idx],%[Mem]"                                              \
          : "=@ccc"(__ret)                                                     \
          : [Idx] "r"(idx), [Mem] "m"(*(char const(*)[(idx) / 8 + 1])(mem))    \
          : "cc");                                                             \
      __ret;                                                                   \
    })
#  define __BITOPW(inst, mem, idx)                                             \
    ({                                                                         \
      _Bool __ret = 0;                                                         \
      asm(#inst "\t%[Idx],%[Mem]"                                              \
          : "=@ccc"(__ret), [Mem] "+m"(*(char(*)[(idx) / 8 + 1])(mem))   \
          : [Idx] "r"(idx)                                                     \
          : "cc");                                                             \
      __ret;                                                                   \
    })
#  define Bt(x, y)   __BITOPR(bt, (x), (y))
#  define LBt(x, y)  __BITOPR(lock bt, (x), (y))
#  define Btc(x, y)  __BITOPW(btc, (x), (y))
#  define Btr(x, y)  __BITOPW(btr, (x), (y))
#  define Bts(x, y)  __BITOPW(bts, (x), (y))
#  define LBtc(x, y) __BITOPW(lock btc, (x), (y))
#  define LBtr(x, y) __BITOPW(lock btr, (x), (y))
#  define LBts(x, y) __BITOPW(lock bts, (x), (y))
#endif

#define Bsr(x)                                                                 \
  ({                                                                           \
    int64_t __x = (int64_t)(x);                                                     \
    __x ? __builtin_popcountll(-2) - __builtin_clzll(__x) : -1;                \
  })
#define Bsf(x)                                                                 \
  ({                                                                           \
    int64_t __x = (int64_t)(x);                                                     \
    __x ? __builtin_ctzll(__x) : -1;                                           \
  })

extern void *Misc_BP();
void *Misc_Caller(int64_t c);
extern void AIWNIOS_setcontext(void *);
extern int64_t AIWNIOS_getcontext(void *);
extern int64_t AIWNIOS_makecontext(void *, void *, void *);
extern int64_t FFI_CALL_TOS_0(void *fptr);
extern int64_t FFI_CALL_TOS_1(void *fptr, int64_t);
extern int64_t FFI_CALL_TOS_2(void *fptr, int64_t, int64_t);
extern int64_t FFI_CALL_TOS_3(void *fptr, int64_t, int64_t, int64_t);
extern int64_t FFI_CALL_TOS_4(void *fptr, int64_t, int64_t, int64_t, int64_t);
extern int64_t FFI_CALL_TOS_CUSTOM_BP(uint64_t bp, void *fp, uint64_t ip);
int64_t TempleOS_CallN(void (*fptr)(), int64_t argc, int64_t *argv);
int64_t TempleOS_Call(void (*fptr)(void));
int64_t TempleOS_CallVaArgs(void (*fptr)(void), int64_t argc, int64_t *argv);

#if defined(__EMSCRIPTEN__)
int64_t FFI_CALL_TOS_0_FEW_INSTS(void *fptr,int64_t iterations);
#  define PAUSE ;	
#elif defined(_M_ARM64) || defined(__aarch64__)
#  define PAUSE asm("yield ");
#elif defined(__x86_64__)
#  define PAUSE asm("pause ");
#else
#  define PAUSE asm(".4byte 0x100000f");
#endif
