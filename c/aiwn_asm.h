#pragma once
#include <stdint.h>
#ifdef _WIN32
extern void Misc_ForceYield();
#endif
extern int64_t Misc_Bt(void *ptr, int64_t);
extern int64_t Misc_Btc(void *ptr, int64_t);
extern int64_t Misc_Btr(void *ptr, int64_t);
extern int64_t Misc_Bts(void *ptr, int64_t);
extern int64_t Misc_LBt(void *ptr, int64_t);
extern int64_t Misc_LBtc(void *ptr, int64_t);
extern int64_t Misc_LBtr(void *ptr, int64_t);
extern int64_t Misc_LBts(void *ptr, int64_t);
extern void *Misc_BP();
extern int64_t Bsr(int64_t v);
extern int64_t Bsf(int64_t v);

void *Misc_Caller(int64_t c);
int64_t LBtc(char *, int64_t);
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

#if defined(_M_ARM64) || defined(__aarch64__)
#  define PAUSE asm("yield ");
#elif defined(__x86_64__)
#  define PAUSE asm("pause ");
#else
#  define PAUSE asm(".4byte 0x100000f");
#endif
int64_t Btr(void *, int64_t);
int64_t Bts(void *, int64_t);
