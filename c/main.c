#define SDL_MAIN_HANDLED 1
#define AIWN_BOOTSTRAP
#define AIWNIOS_TESTS
#include "argtable3.h"
#include "c/aiwn_asm.h"
#include "c/aiwn_except.h"
#include "c/aiwn_fs.h"
#include "c/aiwn_lexparser.h"
#include "c/aiwn_mem.h"
#include "c/aiwn_multic.h"
#include "c/aiwn_que.h"
#include "c/aiwn_snd.h"
#include "c/aiwn_sock.h"
#include "c/aiwn_tui.h"
#include "c/aiwn_windows.h"
#include "isocline.h"
#include <SDL.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#ifndef _WIN64
#  include <sys/resource.h>
#endif
void InputLoop(void *ul);
extern CHashTable *glbl_table;
extern int64_t user_ev_num;
#if defined(WIN32) || defined(_WIN32)
#  include <shlobj.h>
#include <bcrypt.h>
#else
#  include <pwd.h>
#  include <sys/types.h>
#endif
#if defined(__APPLE__)
#  include <libkern/OSCacheControl.h>
#endif
// clang-format off
#ifdef __FreeBSD__ 
#include <machine/sysarch.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dev/io/iodev.h>
#include <machine/iodev.h>
#endif
// clang-format on
#ifdef _WIN64
#  include <windows.h>
#  include <processthreadsapi.h>
#endif
int64_t sdl_window_grab_enable = 0;
struct arg_lit *arg_help, *arg_overwrite, *arg_new_boot_dir, *arg_asan_enable,
    *sixty_fps, *arg_cmd_line, *arg_cmd_line2 = NULL, *arg_fork, *arg_no_debug,
                               *arg_grab, *arg_fast_fail;
struct arg_file *arg_t_dir, *arg_bootstrap_bin, *arg_boot_files, *arg_pidfile;
static struct arg_end *_arg_end;
#ifdef AIWNIOS_TESTS
// Import PrintI first
static void PrintI(char *str, int64_t i) {
  printf("%s:%ld\n", str, i);
}
static void PrintF(char *str, double f) {
  printf("%s:%lf\n", str, f);
}
static void PrintPtr(char *str, void *f) {
  printf("%s:%p\n", str, f);
}
static int64_t STK_PrintI(int64_t *);
static int64_t STK_PrintF(double *);
static int64_t STK_PrintPtr(int64_t *stk) {
  PrintPtr((char *)(stk[0]), (void *)stk[1]);
}
static int64_t STK_DolDocDumpIR(int64_t *stk) {
  int64_t len = 0, ir_cnt, idx;
  CCmpCtrl *cctrl = (CCmpCtrl *)(stk[2]);
  CRPN *c, *head = cctrl->code_ctrl->ir_code, **array;
  ir_cnt = 0;
  for (c = head->base.next; c != head; c = ICFwd(c)) {
    ir_cnt++;
  }
  array = A_MALLOC(ir_cnt * sizeof(void *), NULL);
  idx = 0;
  // REVERSE polish notation
  for (c = head->base.next; c != head; c = ICFwd(c)) {
    array[ir_cnt - ++idx] = c;
  }
  for (idx = 0; idx != ir_cnt; idx++)
    len = DolDocDumpIR((char *)stk[0], len, array[idx]);
  A_FREE(array);
  return len;
}
static void ExitAiwnios(int64_t *);
static void PrsAddSymbol(char *name, void *ptr, int64_t arity) {
  PrsBindCSymbol(name, ptr, arity);
}
static void PrsAddSymbolNaked(char *name, void *ptr, int64_t arity) {
  PrsBindCSymbolNaked(name, ptr, arity);
}
static void FuzzTest1() {
  int64_t i, i2, o;
  char tf[BUFSIZ];
  strcpy(tf, "FUZZ1.HC");
  char buf[BUFSIZ + 64];
  FILE *f = fopen(tf, "w");
  fprintf(f, "extern U0 PrintI(U8i *,I64i);\n");
  // Do complicated expr to dirty some temp registers
  fprintf(
      f,
      "I64i Fa() {I64i a,b,c;a=50,b=2,c=1; return c+(10*b)+(a*(1+!!c))+b;}\n");
  fprintf(f, "I64i Fb() {return Fa-123+6;}\n");
  fprintf(f, "U0 Fuzz() {\n");
  fprintf(f, "    I64i ra,rb,na,nb,*pna,*pnb;\n");
  fprintf(f, "    pna=&na,pnb=&nb;\n");
  fprintf(f, "    ra=na=123;\n");
  fprintf(f, "    rb=nb=6;\n");
  char *operandsA[] = {
      "ra", "na", "*pna(I32i*)", "Fa", "123",
  };
  char *operandsB[] = {
      "rb", "nb", "*pnb(I32i*)", "Fb", "6",
  };
  char *bopers[] = {
      "+", "-",  "*",  "%", "/", "==", "!=", ">",
      "<", ">=", "<=", "&", "^", "|",  "<<", ">>",
  };
  char *assignOps[] = {
      "=", "+=", "-=", "*=", "%=", "/=", "<<=", ">>=", "&=", "|=", "^="};
  char *assignUOps[] = {"++", "--"};
  for (o = 0; o != sizeof(bopers) / sizeof(char *); o++) {
    for (i = 0; i != sizeof(operandsA) / sizeof(char *); i++)
      for (i2 = 0; i2 != sizeof(operandsB) / sizeof(char *); i2++) {
        fprintf(f, "PrintI(\"%s(123) %s %s(6) ==\", %s %s %s);\n", operandsA[i],
                bopers[o], operandsB[i2], operandsA[i], bopers[o],
                operandsB[i2]);
      }
  }
  for (o = 0; o != sizeof(assignOps) / sizeof(char *); o++) {
    // Last 2 of operandsA/B are immuatable,so dont change them
    for (i = 0; i != sizeof(operandsA) / sizeof(char *) - 2; i++)
      for (i2 = 0; i2 != sizeof(operandsB) / sizeof(char *); i2++) {
        fprintf(f, "%s = 123;\n", operandsA[i]);
        fprintf(f, "%s %s %s;\nPrintI(\"%s(123) %s %s(6) ==\", %s);\n",
                operandsA[i], assignOps[o], operandsB[i2], operandsA[i],
                assignOps[o], operandsB[i2], operandsA[i]);
      }
  }
  for (o = 0; o != sizeof(assignUOps) / sizeof(char *); o++) {
    // Last 2 of operandsA/B are immuatable,so dont change them
    for (i = 0; i != sizeof(operandsA) / sizeof(char *) - 2; i++) {
      fprintf(f, "%s = 123;\n", operandsA[i]);
      fprintf(f, "PrintI(\"(%s) %s\",(%s) %s);", operandsA[i], assignUOps[o],
              operandsA[i], assignUOps[o]);
      fprintf(f, "PrintI(\"%s\",%s);\n", operandsA[i], operandsA[i]);
      fprintf(f, "%s = 123;\n", operandsA[i]);
      fprintf(f, "PrintI(\"%s (%s)\",%s (%s));\n", assignUOps[o], operandsA[i],
              assignUOps[o], operandsA[i]);
      fprintf(f, "PrintI(\"%s\",%s);\n", operandsA[i], operandsA[i]);
    }
  }
  fprintf(f, "}\n");
  fclose(f);
  sprintf(buf, "#include \"%s\";\n", tf);
  CLexer *lex = LexerNew("None", buf);
  CCmpCtrl *ccmp = CmpCtrlNew(lex);
  CodeCtrlPush(ccmp);
  Lex(lex);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsAddSymbol("PrintI", &STK_PrintI, 2);
  ccmp->cur_fun = HashFind("Fuzz", Fs->hash_table, HTT_FUN, 1);
  int64_t (*poop5)() = ccmp->cur_fun->fun_ptr;
  SetWriteNP(0);
  FFI_CALL_TOS_0(poop5);
  SetWriteNP(1);
}
static void FuzzTest2() {
  int64_t i, i2, o;
  char tf[BUFSIZ];
  strcpy(tf, "FUZZ2.HC");
  char buf[BUFSIZ + 64];
  FILE *f = fopen(tf, "w");
  fprintf(f, "extern U0 PrintF(U8i *,F64);\n");
  // Do complicated expr to dirty some temp registers
  fprintf(f, "F64 Fa() {F64 a=50,b=2,c=1; return c+(10*b)+(a*(1+!!c))+b;}\n");
  fprintf(f, "F64 Fb() {return Fa-123. +6.;}\n");
  char *operandsA[] = {
      "ra", "na", "*pna", "Fa", "123.",
  };
  char *operandsB[] = {
      "rb", "nb", "*pnb", "Fb", "6.",
  };
  char *bopers[] = {"+", "-", "*", "%", "/", "==", "!=", ">", "<", ">=", "<="};
  char *assignOps[] = {"=", "+=", "-=", "*=", "%=", "/="};
  char *assignUOps[] = {"++", "--"};
  fprintf(f, "U0 Fuzz() {\n");
  fprintf(f, "    F64 ra,rb,na,nb,*pna,*pnb;\n");
  fprintf(f, "    pna=&na,pnb=&nb;\n");
  fprintf(f, "    ra=na=123.;\n");
  fprintf(f, "    rb=nb=6.;\n");
  for (o = 0; o != sizeof(bopers) / sizeof(*bopers); o++)
    for (i = 0; i != sizeof(operandsA) / sizeof(*operandsA); i++)
      for (i2 = 0; i2 != sizeof(operandsA) / sizeof(*operandsA); i2++) {
        fprintf(f, "PrintF(\"%s %s %s ==\", %s %s %s);\n", operandsA[i],
                bopers[o], operandsB[i2], operandsA[i], bopers[o],
                operandsB[i2]);
      }
  for (o = 0; o != sizeof(assignOps) / sizeof(*assignOps); o++)
    for (i = 0; i != sizeof(operandsA) / sizeof(*operandsA) - 2; i++)
      for (i2 = 0; i2 != sizeof(operandsB) / sizeof(*operandsB); i2++) {
        fprintf(f, "%s=123.;\n", operandsA[i]);
        fprintf(f, "PrintF(\"%s %s %s ==\", %s %s %s);\n", operandsA[i],
                assignOps[o], operandsB[i2], operandsA[i], assignOps[o],
                operandsB[i2]);
      }
  for (o = 0; o != sizeof(assignUOps) / sizeof(char *); o++) {
    // Last 2 of operandsA/B are immuatable,so dont change them
    for (i = 0; i != sizeof(operandsA) / sizeof(char *) - 2; i++) {
      fprintf(f, "%s=123;\n", operandsA[i]);
      fprintf(f, "PrintF(\"(%s)%s\",(%s)%s);", operandsA[i], assignUOps[o],
              operandsA[i], assignUOps[o]);
      fprintf(f, "PrintF(\"%s\",%s);\n", operandsA[i], operandsA[i]);
      fprintf(f, "%s=123;\n", operandsA[i]);
      fprintf(f, "PrintF(\"%s(%s)\",%s(%s));\n", assignUOps[o], operandsA[i],
              assignUOps[o], operandsA[i]);
      fprintf(f, "PrintF(\"%s\",%s);\n", operandsA[i], operandsA[i]);
    }
  }
  fprintf(f, "}\n");
  fclose(f);
  sprintf(buf, "#include \"%s\";\n", tf);
  CLexer *lex = LexerNew("None", buf);
  CCmpCtrl *ccmp = CmpCtrlNew(lex);
  CodeCtrlPush(ccmp);
  Lex(lex);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsAddSymbol("PrintF", &STK_PrintF, 2);
  ccmp->cur_fun = HashFind("Fuzz", Fs->hash_table, HTT_FUN, 1);
  int64_t (*poopf)() = ccmp->cur_fun->fun_ptr;
  FFI_CALL_TOS_0(poopf);
}
static void FuzzTest3() {
  int64_t i, i2, o;
  char tf[BUFSIZ];
  strcpy(tf, "FUZZ3.HC");
  char buf[BUFSIZ + 64];
  FILE *f = fopen(tf, "w");
  fprintf(f, "extern U0 PrintPtr(U8i *,I64i);\n");
  // Do complicated expr to dirty some temp registers
  fprintf(f, "I64i Fa() {I64i a=50,b=2,c=1; return "
             "(c+(10*b)+(a*(1+!!c))+b-23)/100*0x100;}\n");
  fprintf(f, "I64i Fb() {return Fa-0x100 +2;}\n");
  char *operandsA[] = {
      "ra", "na", "*pna", "Fa(I32i*)", "0x100(I32i*)",
  };
  char *operandsB[] = {
      "rb", "nb", "*pnb", "Fb", "2",
  };
  char *bopers[] = {"+", "-"};
  char *assignOps[] = {"=", "+=", "-="};
  char *assignUOps[] = {"++", "--"};
  fprintf(f, "U0 Fuzz() {\n");
  fprintf(f, "    I32i *ra,*na,**pna;\n");
  fprintf(f, "    I64i rb,nb,*pnb;\n");
  fprintf(f, "    pna=&na,pnb=&nb;\n");
  fprintf(f, "    ra=na=0x100;\n");
  fprintf(f, "    rb=nb=2;\n");
  for (o = 0; o != sizeof(bopers) / sizeof(*bopers); o++)
    for (i = 0; i != sizeof(operandsA) / sizeof(*operandsA); i++)
      for (i2 = 0; i2 != sizeof(operandsA) / sizeof(*operandsA); i2++) {
        fprintf(f, "PrintPtr(\"%s %s %s ==\", %s %s %s);\n", operandsA[i],
                bopers[o], operandsB[i2], operandsA[i], bopers[o],
                operandsB[i2]);
      }

  // Test subtraction between ptr and ptr
  for (i = 0; i != sizeof(operandsA) / sizeof(char *) - 2; i++) {
    fprintf(f, "PrintPtr(\"%s-0x10(I32i*)\",%s-0x10(I32i*));\n", operandsA[i],
            operandsA[i]);
  }
  for (o = 0; o != sizeof(assignOps) / sizeof(*assignOps); o++)
    for (i = 0; i != sizeof(operandsA) / sizeof(*operandsA) - 2; i++)
      for (i2 = 0; i2 != sizeof(operandsB) / sizeof(*operandsB); i2++) {
        fprintf(f, "%s=0x100;\n", operandsA[i]);
        fprintf(f, "PrintPtr(\"%s %s %s ==\", %s %s %s);\n", operandsA[i],
                assignOps[o], operandsB[i2], operandsA[i], assignOps[o],
                operandsB[i2]);
      }
  for (o = 0; o != sizeof(assignUOps) / sizeof(char *); o++) {
    // Last 2 of operandsA/B are immuatable,so dont change them
    for (i = 0; i != sizeof(operandsA) / sizeof(char *) - 2; i++) {
      fprintf(f, "%s=0x100;\n", operandsA[i]);
      fprintf(f, "PrintPtr(\"(%s)%s\",(%s)%s);", operandsA[i], assignUOps[o],
              operandsA[i], assignUOps[o]);
      fprintf(f, "PrintPtr(\"%s\",%s);\n", operandsA[i], operandsA[i]);
      fprintf(f, "%s=0x100;\n", operandsA[i]);
      fprintf(f, "PrintPtr(\"%s(%s)\",%s(%s));\n", assignUOps[o], operandsA[i],
              assignUOps[o], operandsA[i]);
      fprintf(f, "PrintPtr(\"%s\",%s);\n", operandsA[i], operandsA[i]);
    }
  }
  fprintf(f, "}\n");
  fclose(f);
  sprintf(buf, "#include \"%s\";\n", tf);
  CLexer *lex = LexerNew("None", buf);
  CCmpCtrl *ccmp = CmpCtrlNew(lex);
  CodeCtrlPush(ccmp);
  Lex(lex);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsStmt(ccmp);
  PrsAddSymbol("PrintPtr", &STK_PrintPtr, 2);
  ccmp->cur_fun = HashFind("Fuzz", Fs->hash_table, HTT_FUN, 1);
  int64_t (*poop_ptr)() = ccmp->cur_fun->fun_ptr;
  FFI_CALL_TOS_0(poop_ptr);
}
#endif
static double Pow10(double d) {
  return pow(10, d);
}

static void *MemSetU16(int16_t *dst, int16_t with, int64_t cnt) {
  while (--cnt >= 0) {
    dst[cnt] = with;
  }
  return dst;
}
static void *MemSetU32(int32_t *dst, int32_t with, int64_t cnt) {
  while (--cnt >= 0) {
    dst[cnt] = with;
  }
  return dst;
}
static void STK_AiwniosSetVolume(double *stk) {
  AiwniosSetVolume(*stk);
}
double STK_AiwniosGetVolume(double *stk) {
  return AiwniosGetVolume();
}
static void *MemSetU64(int64_t *dst, int64_t with, int64_t cnt) {
  while (--cnt >= 0) {
    dst[cnt] = with;
  }
  return dst;
}
static void PutS(char *s) {
  fprintf(stdout, "%s", s);
  fflush(stdout);
}
static uint64_t STK_60fps(uint64_t *stk) {
  return sixty_fps->count != 0;
}
extern int64_t GetTicksHP();
static int64_t __GetTicksHP() {
#if defined(_WIN32) || defined(WIN32)
  return GetTicksHP(); // From multic.c
#else
  struct timespec ts;
  static int64_t initial = 0;
  int64_t theTick = 0U;
  if (!initial) {
    clock_gettime(CLOCK_REALTIME, &ts);
    theTick = ts.tv_nsec / 1000;
    theTick += ts.tv_sec * 1000000U;
    initial = theTick;
    return 0;
  }
  clock_gettime(CLOCK_REALTIME, &ts);
  theTick = ts.tv_nsec / 1000;
  theTick += ts.tv_sec * 1000000U;
  return theTick - initial;
#endif
}
static int64_t __GetTicks() {
  return __GetTicksHP() / 1000;
}
#if defined(__x86_64__)
static int __iofd_warned;
#  if defined(__FreeBSD__) || defined(__linux__)
static int __iofd = -1, __iofd_errno = -1, __iofd_cur_port = -1;
static char const *__iofd_str;
static void __aiwn_out(uint64_t wut, uint64_t port, uint64_t sz) {
  if (-1 == __iofd) {
    if (__iofd_warned)
      return;
    fprintf(stderr, "Couldn't open %s: %s\n", __iofd_str,
            strerror(__iofd_errno));
    __iofd_warned = 1;
    return;
  }
#    ifdef __linux__
  if (port != __iofd_cur_port)
    lseek(__iofd, port, SEEK_SET);
  write(__iofd, &wut, sz);
#    elif defined(__FreeBSD__)
  ioctl(__iofd, IODEV_PIO,
        &(struct iodev_pio_req){
            .access = IODEV_PIO_WRITE,
            .port = port,
            .width = sz,
            .val = wut,
        });
#    endif
}
static uint64_t __aiwn_in(uint64_t port, uint64_t sz) {
  if (-1 == __iofd) {
    if (__iofd_warned)
      return -1ul;
    fprintf(stderr, "Couldn't open %s: %s\n", __iofd_str,
            strerror(__iofd_errno));
    __iofd_warned = 1;
    return -1ul;
  }
#    ifdef __linux__
  uint64_t res = 0;
  if (port != __iofd_cur_port)
    lseek(__iofd, port, SEEK_SET);
  read(__iofd, &res, sz);
  return res;
#    elif defined(__FreeBSD__)
  // IODEV_PIO_READ
  //     The operation is an "in" type.  A value will be read
  //     from the specified port (retrieved from the port member)
  //     and the result will be stored in the val member.
  //  --man 4 io
  struct iodev_pio_req req = {
      .access = IODEV_PIO_READ,
      .port = port,
      .width = sz,
  };
  ioctl(__iofd, IODEV_PIO, &req);
  return req.val;
#    endif
}
#  elif defined(_WIN32)
static void __aiwn_out(uint64_t ul1, uint64_t ul2, uint64_t ul3) {
  if (__iofd_warned)
    return;
  fprintf(stderr, "In/Out not supported on Windows\n");
  __iofd_warned = 1;
}
static uint64_t __aiwn_in(uint64_t ul1, uint64_t ul2) {
  if (__iofd_warned)
    return -1ull;
  fprintf(stderr, "In/Out not supported on Windows\n");
  __iofd_warned = 1;
  return -1ull;
}
#  endif
static void STK_OutU8(uint64_t *stk) {
  __aiwn_out(stk[1], stk[0], 1);
}
static void STK_OutU16(uint64_t *stk) {
  __aiwn_out(stk[1], stk[0], 2);
}
static void STK_OutU32(uint64_t *stk) {
  __aiwn_out(stk[1], stk[0], 4);
}
static uint64_t STK_InU8(uint64_t *stk) {
  return __aiwn_in(stk[0], 1);
}
static uint64_t STK_InU16(uint64_t *stk) {
  return __aiwn_in(stk[0], 2);
}
static uint64_t STK_InU32(uint64_t *stk) {
  return __aiwn_in(stk[0], 4);
}
#  ifdef _WIN32
#    define RepIn(n)                                                           \
      static void STK_RepInU##n(uint64_t *) __attribute__((alias("STK_"        \
                                                                 "InU32")))
#    define RepOut(n)                                                          \
      static void STK_RepOutU##n(uint64_t *) __attribute__((alias("STK_"       \
                                                                  "InU32")))
#  else
#    define RepIn(n)                                                           \
      static void STK_RepInU##n(uint64_t *stk) {                               \
        uint64_t port = stk[2], cnt = stk[1];                                  \
        uint##n##_t *buf = stk[0];                                             \
        for (uint64_t i = 0; i < cnt; i++)                                     \
          buf[i] = __aiwn_in(port, n / 8) & ((1ull << n) - 1);                 \
      }
#    define RepOut(n)                                                          \
      static void STK_RepOutU##n(uint64_t *stk) {                              \
        uint64_t port = stk[2], cnt = stk[1];                                  \
        uint##n##_t *buf = stk[0];                                             \
        for (uint64_t i = 0; i < cnt; i++)                                     \
          __aiwn_out(buf[i] & ((1ull << n) - 1), port, n / 8);                 \
      }
#  endif
RepIn(8);
RepIn(16);
RepIn(32);
RepOut(8);
RepOut(16);
RepOut(32);
#  undef RepIn
#  undef RepOut
#endif
#ifdef _WIN32
#  define Initrand(h) h = 0
#  define Getrand(h, b, i) BCRYPT_SUCCESS(BCryptGenRandom(h, b, i, BCRYPT_USE_SYSTEM_PREFERRED_RNG))
#else
#  define Initrand(h) h = open("/dev/urandom", O_RDONLY)
#  define Getrand(h, b, i) (i == read(h, b, i))
#endif
static int64_t STK_CSPRNG(int64_t *stk) {
  int64_t sz = stk[1];
  char *buf = (char *)stk[0];
  static int fd = -1;
  static char tmp[1024];
  static int64_t cur;
  if (fd == -1)
    Initrand(fd);
  if (!buf || sz < 0)
    return 0;
  else if (!sz)
    return 1;
  else if (sz <= sizeof tmp) {
    int64_t ret = 1;
    if (!cur || sizeof tmp < cur + sz)
      cur = 0, ret = Getrand(fd, tmp, sizeof tmp);
    memcpy(buf, tmp + cur, sz);
    cur += sz;
    return ret;
  } else
    return Getrand(fd, buf, sz);
}
static int64_t MemCmp(char *a, char *b, int64_t s) {
  return memcmp(a, b, s);
}
int64_t UnixNow() {
  return (int64_t)time(NULL);
}
static double Arg(double x, double y) {
  return atan2(y, x);
}
static char *AiwniosGetClipboard() {
  char *has, *ret;
  if (!SDL_HasClipboardText())
    return A_STRDUP("", NULL);
  has = SDL_GetClipboardText();
  ret = A_STRDUP(has, NULL);
  SDL_free(has);
  return ret;
}
static void AiwniosSetClipboard(char *c) {
  if (c)
    SDL_SetClipboardText(c);
}

static void TaskContextSetRIP(int64_t *ctx, void *p) {
#if defined(_M_ARM64) || defined(__aarch64__)
  ctx[12] = (int64_t)p;
#else
  ctx[0] = (int64_t)p;
#endif
}

static STK_TaskContextSetRIP(int64_t *stk) {
  TaskContextSetRIP((int64_t *)(stk[0]), (void *)stk[1]);
}

static int64_t STK_GenerateFFIForFun(void **stk) {
  TaskContextSetRIP(stk[0], (void *)stk[1]);
}

static int64_t STK_AIWNIOS_makecontext(void **stk) {
  return AIWNIOS_makecontext(stk[0], stk[1], stk[2]);
}

static int64_t STK___HC_SetAOTRelocBeforeRIP(void **stk) {
  __HC_SetAOTRelocBeforeRIP(stk[0], (int64_t)stk[1]);
}

static int64_t STK___HC_CodeMiscIsUsed(void **stk) {
  __HC_CodeMiscIsUsed((CCodeMisc *)stk[0]);
}

static int64_t STK_AiwniosSetClipboard(void **stk) {
  AiwniosSetClipboard((char *)stk[0]);
}

static int64_t STK_AiwniosGetClipboard(void **stk) {
  return (int64_t)AiwniosGetClipboard();
}

static int64_t STK_CmpCtrlDel(void **stk) {
  CmpCtrlDel((CCmpCtrl *)stk[0]);
}

_Static_assert(sizeof(double) == sizeof(uint64_t));
typedef union {
  double d;
  uint64_t i
} dbl2u64;

#define MATHFUNDEF(x)                                                          \
  static uint64_t STK_##x(double *stk) {                                       \
    return ((dbl2u64)x(stk[0])).i;                                             \
  }
#define MATHFUNDEF2(x)                                                         \
  static uint64_t STK_##x(double *stk) {                                       \
    return ((dbl2u64)x(stk[0], stk[1])).i;                                     \
  }

MATHFUNDEF(cos);
MATHFUNDEF(sin);
MATHFUNDEF(tan);
MATHFUNDEF(acos);
MATHFUNDEF(atan);
MATHFUNDEF(asin);

MATHFUNDEF2(pow);
MATHFUNDEF2(Arg);

static int64_t STK_Misc_Btc(int64_t *stk) {
  return Misc_Btc((void *)stk[0], stk[1]);
}

static int64_t STK_Misc_Btr(int64_t *stk) {
  return Misc_Btr((void *)stk[0], stk[1]);
}
static int64_t STK_Misc_LBtr(int64_t *stk) {
  return Misc_LBtr((void *)stk[0], stk[1]);
}
static int64_t STK_Misc_LBts(int64_t *stk) {
  return Misc_LBts((void *)stk[0], stk[1]);
}
static int64_t STK_Misc_Bt(int64_t *stk) {
  return Misc_Bt((void *)stk[0], stk[1]);
}
static int64_t STK_Misc_LBtc(int64_t *stk) {
  return Misc_LBtc((void *)stk[0], stk[1]);
}
static int64_t STK_Misc_Bts(int64_t *stk) {
  return Misc_Bts((void *)stk[0], stk[1]);
}

static int64_t STK_HeapCtrlInit(int64_t *stk) {
  return (int64_t)HeapCtrlInit((CHeapCtrl *)stk[0], (CTask *)stk[1], stk[2]);
}

static int64_t STK_HeapCtrlDel(int64_t *stk) {
  HeapCtrlDel((CHeapCtrl *)stk[0]);
}

static int64_t STK___AIWNIOS_MAlloc(int64_t *stk) {
  return (int64_t)__AIWNIOS_MAlloc(stk[0], (CTask *)stk[1]);
}
static int64_t STK___AIWNIOS_CAlloc(int64_t *stk) {
  return (int64_t)__AIWNIOS_CAlloc(stk[0], (CTask *)stk[1]);
}

static int64_t STK___HC_ICSetLock(int64_t *stk) {
  __HC_ICSetLock(stk[0]);
}

static int64_t STK___AIWNIOS_Free(int64_t *stk) {
  __AIWNIOS_Free((void *)stk[0]);
}

static int64_t STK_MSize(int64_t *stk) {
  return MSize((void *)stk[0]);
}

static int64_t STK___SleepHP(int64_t *stk) {
  MPSleepHP(stk[0]);
}

static int64_t STK___GetTicksHP(int64_t *stk) {
  return __GetTicksHP(stk[0]);
}

static int64_t STK___AIWNIOS_StrDup(int64_t *stk) {
  return (int64_t)__AIWNIOS_StrDup((char *)stk[0], (void *)stk[1]);
}

static void *STK_MemCpy(void **stk) {
  return memmove(stk[0], stk[1], (size_t)stk[2]);
}

static void *STK_MemSet(int64_t *stk) {
  return memset((void *)stk[0], stk[1], stk[2]);
}

static int64_t STK_MemSetU16(int64_t *stk) {
  return (int64_t)MemSetU16((void *)stk[0], stk[1], stk[2]);
}

static int64_t STK_MemSetU32(int64_t *stk) {
  return (int64_t)MemSetU32((void *)stk[0], stk[1], stk[2]);
}

static int64_t STK_MemSetU64(int64_t *stk) {
  return (int64_t)MemSetU64((void *)stk[0], stk[1], stk[2]);
}

static int64_t STK_MemSetI64(int64_t *stk) {
  return (int64_t)MemSetU64((void *)stk[0], stk[1], stk[2]);
}

static size_t STK_StrLen(char **stk) {
  return strlen(stk[0]);
}

static int64_t STK_StrCmp(char **stk) {
  // dont cast, sign extend
  return strcmp(stk[0], stk[1]);
}

static char *STK_StrCpy(char **stk) {
  return memmove(stk[0], stk[1], strlen(stk[1]) + 1);
}

static int64_t STK_StrNCmp(char **stk) {
  return __builtin_strncmp(stk[0], stk[1], (size_t)stk[2]);
}

static int64_t STK_StrICmp(char **stk) {
  return __builtin_strcasecmp(stk[0], stk[1]);
}

static int64_t STK_StrNICmp(char **stk) {
  return __builtin_strncasecmp(stk[0], stk[1], (size_t)stk[2]);
}

static char *STK_StrMatch(char **stk) {
  return __builtin_strstr(stk[1], stk[0]);
}

static char *STK_StrIMatch(char **stk) {
#ifndef _WIN64
  return strcasestr(stk[1], stk[0]);
#else
  // SDL forces UTF8,so roll our own
  char *heystack = stk[1], *needle = stk[0];
  int64_t i = 0, len = strlen(needle), len2 = strlen(heystack);
  if (len > len2)
    return 0;
  for (i = 0; i <= len2 - len; i++)
    if (!__builtin_strncasecmp(needle, heystack + i, len))
      return heystack + i;
  return 0;
#endif
}

MATHFUNDEF(log10);
MATHFUNDEF(log2);
MATHFUNDEF(Pow10);
MATHFUNDEF(sqrt);
MATHFUNDEF(exp);
MATHFUNDEF(round);
MATHFUNDEF(log);
MATHFUNDEF(floor);
MATHFUNDEF(ceil);

static int64_t STK_PrintI(int64_t *stk) {
  PrintI((char *)stk[0], stk[1]);
}

static int64_t STK_PrintF(double *stk) {
  PrintF(((char **)stk)[0], stk[1]);
}

static int64_t STK_memcmp(int64_t *stk) {
  return (int64_t)memcmp((void *)stk[0], (void *)stk[1], stk[2]);
}

static int64_t STK_Bt(int64_t *stk) {
  return Misc_Bt((void *)stk[0], stk[1]);
}

static int64_t STK_LBts(int64_t *stk) {
  return Misc_LBts((void *)stk[0], stk[1]);
}
static int64_t STK_LBtr(int64_t *stk) {
  return Misc_LBtr((void *)stk[0], stk[1]);
}
static int64_t STK_LBtc(int64_t *stk) {
  return Misc_LBtc((void *)stk[0], stk[1]);
}
static int64_t STK_Btr(int64_t *stk) {
  return Misc_Btr((void *)stk[0], stk[1]);
}
static int64_t STK_Bts(int64_t *stk) {
  return Misc_Bts((void *)stk[0], stk[1]);
}
static int64_t STK_Bsf(int64_t *stk) {
  return Bsf(stk[0]);
}
static int64_t STK_Bsr(int64_t *stk) {
  return Bsr(stk[0]);
}

static int64_t STK_DebuggerClientStart(int64_t *s) {
  DebuggerClientStart(*s, s[1]);
  return 0;
}

static int64_t STK_DebuggerClientEnd(int64_t *s) {
  DebuggerClientEnd(s[0], s[1]);
  return 0;
}

static int64_t STK_DbgPutS(int64_t *stk) {
  fprintf(stdout, "%s", (char *)stk[0]);
  fflush(stdout);
}
static int64_t STK_PutS(int64_t *stk) {
  fprintf(stdout, "%s", (char *)stk[0]);
  fflush(stdout);
}

static void STK_PutS2(int64_t *stk) {
  fprintf(stdout, "%s", (char *)stk[0]);
  fflush(stdout);
}

static int64_t STK_SetHolyFs(int64_t *stk) {
  SetHolyFs((void *)stk[0]);
}

static int64_t STK_GetHolyFs(int64_t *stk) {
  return (int64_t)GetHolyFs();
}

static int64_t STK_SpawnCore(int64_t *stk) {
  SpawnCore((void *)stk[0], (void *)stk[1], stk[2]);
}

static int64_t STK_MPSleepHP(int64_t *stk) {
  MPSleepHP(stk[0]);
}

static int64_t STK_MPAwake(int64_t *stk) {
  MPAwake(stk[0]);
}

static int64_t STK_mp_cnt(int64_t *stk) {
  return mp_cnt();
}

static int64_t STK_SetHolyGs(int64_t *stk) {
  SetHolyGs((void *)stk[0]);
}
static int64_t STK___GetTicks(int64_t *stk) {
  return __GetTicks();
}

static int64_t STK___Sleep(int64_t *stk) {
  MPSleepHP(stk[0] * 1e3);
}

static int64_t STK_ImportSymbolsToHolyC(int64_t *stk) {
  ImportSymbolsToHolyC((void *)stk[0]);
}
static int64_t STK_AIWNIOS_getcontext(int64_t *stk) {
  return AIWNIOS_getcontext((void *)stk[0]);
}

static int64_t STK_AIWNIOS_setcontext(int64_t *stk) {
  AIWNIOS_setcontext((void *)stk[0]);
}

static int64_t STK_IsValidPtr(int64_t *stk) {
  return IsValidPtr((char *)stk[0]);
}
static int64_t STK___HC_CmpCtrl_SetAOT(int64_t *stk) {
  __HC_CmpCtrl_SetAOT((CCmpCtrl *)stk[0]);
}

static int64_t STK__HC_ICAdd_RawBytes(int64_t *stk) {
  return (int64_t)__HC_ICAdd_RawBytes((CCmpCtrl *)stk[0], (char *)stk[1],
                                      stk[2]);
}

static int64_t STK___HC_ICAdd_GetVargsPtr(int64_t *stk) {
  return (int64_t)__HC_ICAdd_GetVargsPtr((CCmpCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Typecast(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Typecast((CCodeCtrl *)stk[0], stk[1], stk[2]);
}

static int64_t STK___HC_ICAdd_SubCall(int64_t *stk) {
  return (int64_t)__HC_ICAdd_SubCall((CCodeCtrl *)stk[0], (CCodeMisc *)stk[1]);
}

static int64_t STK___HC_ICAdd_Sqrt(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Sqrt((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Sqr(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Sqr((CCodeCtrl *)stk[0]);
}


static int64_t STK___HC_ICAdd_SubProlog(int64_t *stk) {
  return (int64_t)__HC_ICAdd_SubProlog((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_SubRet(int64_t *stk) {
  return (int64_t)__HC_ICAdd_SubRet((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Switch(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Switch((CCodeCtrl *)stk[0], (CCodeMisc *)stk[1],
                                    (CCodeMisc *)stk[2]);
}

static int64_t STK___HC_ICAdd_UnboundedSwitch(int64_t *stk) {
  return (int64_t)__HC_ICAdd_UnboundedSwitch((CCodeCtrl *)stk[0],
                                             (CCodeMisc *)stk[1]);
}

static int64_t STK___HC_ICAdd_PreInc(int64_t *stk) {
  return (int64_t)__HC_ICAdd_PreInc((CCodeCtrl *)stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_Call(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Call((CCodeCtrl *)stk[0], stk[1], stk[2], stk[3]);
}

static int64_t STK___HC_ICAdd_F64(int64_t *stk) {
  return (int64_t)__HC_ICAdd_F64((CCodeCtrl *)stk[0], ((double *)stk)[1]);
}

static int64_t STK___HC_ICAdd_I64(int64_t *stk) {
  return (int64_t)__HC_ICAdd_I64((CCodeCtrl *)stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_PreDec(int64_t *stk) {
  return (int64_t)__HC_ICAdd_PreDec((CCodeCtrl *)stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_PostInc(int64_t *stk) {
  return (int64_t)__HC_ICAdd_PostInc((CCodeCtrl *)stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_Max_I64(int64_t *stk) {
  return __HC_ICAdd_Max_I64(stk[0]);
}
static int64_t STK___HC_ICAdd_Min_I64(int64_t *stk) {
  return __HC_ICAdd_Min_I64(stk[0]);
}
static int64_t STK___HC_ICAdd_Max_U64(int64_t *stk) {
  return __HC_ICAdd_Max_U64(stk[0]);
}
static int64_t STK___HC_ICAdd_Min_U64(int64_t *stk) {
  return __HC_ICAdd_Min_U64(stk[0]);
}
static int64_t STK_SetCaptureMouse(int64_t *stk) {
  SetCaptureMouse(stk[0]);
  return 0;
}
static int64_t STK___HC_ICAdd_Max_F64(int64_t *stk) {
  return __HC_ICAdd_Max_F64(stk[0]);
}
static int64_t STK___HC_ICAdd_Min_F64(int64_t *stk) {
  return __HC_ICAdd_Min_F64(stk[0]);
}

static int64_t STK___HC_ICAdd_PostDec(int64_t *stk) {
  return (int64_t)__HC_ICAdd_PostDec((CCodeCtrl *)stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_Pow(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Pow((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Eq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Eq((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Div(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Div((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Sub(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Sub((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Mul(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Mul((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Add(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Add((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Deref(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Deref((CCodeCtrl *)stk[0], stk[1], stk[2]);
}

static int64_t STK___HC_ICAdd_Comma(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Comma((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Addr(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Addr((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Xor(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Xor((CCodeCtrl *)stk[0]);
}

static int64_t STK___HC_ICAdd_Mod(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Mod((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Or(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Or((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Lt(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Lt((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Gt(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Gt((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Ge(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Ge((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Le(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Le((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_LNot(int64_t *stk) {
  return (int64_t)__HC_ICAdd_LNot((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Vargs(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Vargs((CCodeCtrl *)stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_BNot(int64_t *stk) {
  return (int64_t)__HC_ICAdd_BNot((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_AndAnd(int64_t *stk) {
  return (int64_t)__HC_ICAdd_AndAnd((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_OrOr(int64_t *stk) {
  return (int64_t)__HC_ICAdd_OrOr((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_XorXor(int64_t *stk) {
  return (int64_t)__HC_ICAdd_XorXor((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Ne(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Ne((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Lsh(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Lsh((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Rsh(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Rsh((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_AddEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_AddEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Lock(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Lock((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Fs(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Fs((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_Gs(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Gs((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_SubEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_SubEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_MulEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_MulEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_DivEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_DivEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_LshEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_LshEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_RshEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_RshEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_AndEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_AndEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_OrEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_OrEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_XorEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_XorEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_ModEq(int64_t *stk) {
  return (int64_t)__HC_ICAdd_ModEq((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_ICAdd_IReg(int64_t *stk) {
  return (int64_t)__HC_ICAdd_IReg((CCodeCtrl *)stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK___HC_ICAdd_FReg(int64_t *stk) {
  return (int64_t)__HC_ICAdd_FReg((CCodeCtrl *)stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_Frame(int64_t *stk) {
  return (int64_t)__HC_ICAdd_Frame((CCodeCtrl *)stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK___HC_CodeMiscStrNew(int64_t *stk) {
  return (int64_t)__HC_CodeMiscStrNew((CCmpCtrl *)stk[0], (char *)stk[1],
                                      stk[2]);
}
static int64_t STK___HC_CodeMiscLabelNew(int64_t *stk) {
  return (int64_t)__HC_CodeMiscLabelNew((CCmpCtrl *)stk[0], (void **)stk[1]);
}
static int64_t STK___HC_CmpCtrlNew(int64_t *stk) {
  return (int64_t)__HC_CmpCtrlNew();
}
static int64_t STK___HC_CodeCtrlPush(int64_t *stk) {
  return (int64_t)__HC_CodeCtrlPush((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_CodeCtrlPop(int64_t *stk) {
  return __HC_CodeCtrlPop((CCodeCtrl *)stk[0]);
}
static int64_t STK___HC_Compile(int64_t *stk) {
  return (int64_t)__HC_Compile((CCmpCtrl *)stk[0], (int64_t *)stk[1],
                               (char **)stk[2], (char **)stk[3]);
}
static int64_t STK___HC_ICAdd_Label(int64_t *stk) {
  return __HC_ICAdd_Label(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_Goto(int64_t *stk) {
  return __HC_ICAdd_Goto(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_GotoIf(int64_t *stk) {
  return __HC_ICAdd_GotoIf(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_Str(int64_t *stk) {
  return __HC_ICAdd_Str(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_And(int64_t *stk) {
  return __HC_ICAdd_And(stk[0]);
}
static int64_t STK___HC_ICAdd_EqEq(int64_t *stk) {
  return __HC_ICAdd_EqEq(stk[0]);
}
static int64_t STK___HC_ICAdd_Neg(int64_t *stk) {
  return __HC_ICAdd_Neg(stk[0]);
}
static int64_t STK___HC_ICAdd_Ret(int64_t *stk) {
  return __HC_ICAdd_Ret(stk[0]);
}
static int64_t STK___HC_ICAdd_Arg(int64_t *stk) {
  return __HC_ICAdd_Arg(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_SetFrameSize(int64_t *stk) {
  return __HC_ICAdd_SetFrameSize(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_Reloc(int64_t *stk) {
  return __HC_ICAdd_Reloc(stk[0], stk[1], stk[2], stk[3], stk[4], stk[5]);
}
static int64_t STK___HC_ICAdd_RelocUnique(int64_t *stk) {
  return __HC_ICAdd_RelocUnqiue(stk[0], stk[1], stk[2], stk[3], stk[4], stk[5]);
}

static int64_t STK___HC_ICSetLine(int64_t *stk) {
  __HC_ICSetLine(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_StaticRef(int64_t *stk) {
  return __HC_ICAdd_StaticRef(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK___HC_ICAdd_StaticData(int64_t *stk) {
  return __HC_ICAdd_StaticData(stk[0], stk[1], stk[2], stk[3], stk[4]);
}
static int64_t STK___HC_ICAdd_SetStaticsSize(int64_t *stk) {
  return __HC_ICAdd_SetStaticsSize(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_ToI64(int64_t *stk) {
  return __HC_ICAdd_ToI64(stk[0]);
}
static int64_t STK___HC_ICAdd_BT(int64_t *stk) {
  return __HC_ICAdd_BT(stk[0]);
}
static int64_t STK___HC_ICAdd_BTS(int64_t *stk) {
  return __HC_ICAdd_BTS(stk[0]);
}
static int64_t STK___HC_ICAdd_BTR(int64_t *stk) {
  return __HC_ICAdd_BTR(stk[0]);
}
static int64_t STK___HC_ICAdd_BTC(int64_t *stk) {
  return __HC_ICAdd_BTC(stk[0]);
}

static int64_t STK___HC_ICAdd_LBTS(int64_t *stk) {
  return (int64_t)__HC_ICAdd_LBTS(stk[0]);
}
static int64_t STK___HC_ICAdd_LBTR(int64_t *stk) {
  return (int64_t)__HC_ICAdd_LBTR(stk[0]);
}
static int64_t STK___HC_ICAdd_LBTC(int64_t *stk) {
  return (int64_t)__HC_ICAdd_LBTC(stk[0]);
}

static int64_t STK___HC_ICAdd_ToF64(int64_t *stk) {
  return (int64_t)__HC_ICAdd_ToF64(stk[0]);
}
static int64_t STK___HC_ICAdd_ShortAddr(int64_t *stk) {
  return (int64_t)__HC_ICAdd_ShortAddr(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK_Misc_Caller(int64_t *stk) {
  return (int64_t)Misc_Caller(stk[0]);
}
static int64_t STK_VFsSetPwd(int64_t *stk) {
  VFsSetPwd(stk[0]);
}
static int64_t STK_VFsFileExists(int64_t *stk) {
  return VFsFileExists(stk[0]);
}
static int64_t STK_VFsIsDir(int64_t *stk) {
  return VFsIsDir(stk[0]);
}
static int64_t STK_VFsFileRead(int64_t *stk) {
  return (int64_t)VFsFileRead(stk[0], stk[1]);
}
static int64_t STK_VFsFileWrite(int64_t *stk) {
  return VFsFileWrite(stk[0], stk[1], stk[2]);
}
static int64_t STK_VFsDel(int64_t *stk) {
  return VFsDel(stk[0]);
}
static int64_t STK_VFsDirMk(int64_t *stk) {
  return VFsDirMk(stk[0]);
}
static int64_t STK_VFsBlkRead(int64_t *stk) {
  return VFsFBlkRead(stk[0], stk[1] * stk[2], stk[3]);
}
static int64_t STK_VFsBlkWrite(int64_t *stk) {
  return VFsFBlkWrite(stk[0], stk[1] * stk[2], stk[3]);
}
static int64_t STK_VFsFOpen(int64_t *stk) {
  return VFsFOpen(stk[0], stk[1]);
}
static void STK_VFsFClose(int64_t *stk) {
  VFsFClose(stk[0]);
}
static int64_t STK_VFsFSeek(int64_t *stk) {
  return VFsFSeek(stk[0], stk[1]);
}
static int64_t STK_VFsSetDrv(int64_t *stk) {
  VFsSetDrv(stk[0]);
}
static int64_t STK_VFsUnixTime(int64_t *stk) {
  return VFsUnixTime(stk[0]);
}
static int64_t STK_VFsTrunc(int64_t *stk) {
  return VFsTrunc(stk[0], stk[1]);
}
static int64_t STK_UpdateScreen(int64_t *stk) {
  UpdateScreen(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK_VFsFSize(int64_t *stk) {
  return VFsFSize(stk[0]);
}
static int64_t STK_UnixNow(int64_t *stk) {
  return UnixNow();
}
static int64_t STK_GrPaletteColorSet(int64_t *stk) {
  GrPaletteColorSet(stk[0], stk[1]);
}
static int64_t STK_DrawWindowNew(int64_t *stk) {
  DrawWindowNew();
}
static int64_t STK_SetKBCallback(int64_t *stk) {
  SetKBCallback(stk[0]);
}
static int64_t STK_SndFreq(int64_t *stk) {
  SndFreq(stk[0]);
}
static int64_t STK_SetMSCallback(int64_t *stk) {
  SetMSCallback(stk[0]);
}
static int64_t STK_InteruptCore(int64_t *stk) {
  InteruptCore(stk[0]);
}
static int64_t STK___HC_CodeMiscInterateThroughRefs(int64_t *stk) {
  __HC_CodeMiscInterateThroughRefs(stk[0], stk[1], stk[2]);
}
static int64_t STK___HC_CodeMiscJmpTableNew(int64_t *stk) {
  return (int64_t)__HC_CodeMiscJmpTableNew(stk[0], stk[1], stk[2], stk[3]);
}

static int64_t STK_BoundsCheck(int64_t *stk) {
  return BoundsCheck((void *)stk[0], (int64_t *)stk[1]);
}

static int64_t STK_NetPollForHangup(int64_t *stk) {
  return NetPollForHangup(stk[0], stk[1]);
}

static int64_t STK_NetIP4ByHost(char **stk) {
  return NetIP4ByHost(stk[0]);
}

static int64_t STK_NetPollForWrite(int64_t *stk) {
  return NetPollForWrite(stk[0], stk[1]);
}

static int64_t STK_NetPollForRead(int64_t *stk) {
  return NetPollForRead(stk[0], stk[1]);
}

static int64_t STK_NetWrite(int64_t *stk) {
  return NetWrite(stk[0], stk[1], stk[2]);
}

static int64_t STK_NetRead(int64_t *stk) {
  return NetRead(stk[0], stk[1], stk[2]);
}

static int64_t STK_NetClose(int64_t *stk) {
  NetClose(stk[0]);
}

static int64_t STK_NetAccept(int64_t *stk) {
  return NetAccept(stk[0], stk[1]);
}

static int64_t STK_NetListen(int64_t *stk) {
  NetListen(stk[0], stk[1]);
}

static int64_t STK_NetBindIn(int64_t *stk) {
  NetBindIn(stk[0], stk[1]);
}

static int64_t STK_DebuggerClientSetGreg(int64_t *stk) {
  DebuggerClientSetGreg(stk[0], stk[1], stk[2]);
  return 0;
}

static int64_t STK_NetSocketNew(int64_t *stk) {
  return NetSocketNew(stk[0]);
}
static int64_t STK_NetUDPSocketNew(int64_t *stk) {
  return NetUDPSocketNew(stk[0]);
}

static int64_t STK_NetAddrNew(int64_t *stk) {
  return NetAddrNew(stk[0], stk[1], stk[2]);
}

static int64_t STK_NetUDPAddrNew(int64_t *stk) {
  return NetUDPAddrNew(stk[0], stk[1], stk[2]);
}

static int64_t STK_NetAddrDel(int64_t *stk) {
  NetAddrDel(stk[0]);
}
static int64_t STK_NetConnect(int64_t *stk) {
  NetConnect(stk[0], stk[1]);
}
static int64_t STK_MPSetProfilerInt(int64_t *stk) {
  MPSetProfilerInt((void *)stk[0], stk[1], stk[2]);
}

static int64_t STK_NetUDPRecvFrom(int64_t *stk) {
  return NetUDPRecvFrom(stk[0], stk[1], stk[2], stk[3]);
}

static int64_t STK_NetUDPSendTo(int64_t *stk) {
  return NetUDPSendTo(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK_NetUDPAddrDel(int64_t *stk) {
  NetUDPAddrDel(stk[0]);
}

int64_t IsCmdLineMode() {
  return arg_bootstrap_bin->count != 0 || arg_cmd_line->count != 0;
}
int64_t IsCmdLineMode2() {
  // On windows im not supporting --tui yet
  if (arg_cmd_line2 == NULL)
    return 0;
  return arg_cmd_line2->count != 0;
}

int64_t STK_TermSize(int64_t *stk) {
  TermSize((int64_t *)(stk[0]), (int64_t *)(stk[1]));
  return 0;
}

static void _freestr(void *p) {
  free(*(void **)p);
}

static char *CmdLineGetStr(char **stk) {
  char __attribute__((cleanup(_freestr))) *s = ic_readline(*stk);
  return A_STRDUP(s ?: "", 0);
}

char **CmdLineBootFiles() {
  return arg_boot_files->filename;
}
int64_t CmdLineBootFileCnt() {
  return arg_boot_files->count;
}
static int64_t STK__HC_ICAdd_ToBool(void **stk) {
  return __HC_ICAdd_ToBool(stk[0]);
}
static int64_t STK_WriteProtectMemCpy(int64_t *stk) {
  char *ptr = (void *)stk[0];
  int64_t r = (int64_t)WriteProtectMemCpy(ptr, (char *)stk[1], stk[2]);
#if defined(__APPLE__)
  sys_icache_invalidate(ptr, stk[2]);
#else
  __builtin___clear_cache(ptr, stk[0] + stk[2]);
#endif
  return r;
}
static int64_t is_fast_fail = 0;
int64_t IsFastFail() {
  return is_fast_fail;
}
static char *AiwniosPackRamDiskPtr(int64_t **);
static char *AiwniosPackBootCommand(int64_t **);
static void BootAiwnios(char *bootstrap_text) {
  // Run a dummy expression to link the functions into the hash table
  CLexer *lex = LexerNew("None", !bootstrap_text ? "1+1;" : bootstrap_text);
  CCmpCtrl *ccmp = CmpCtrlNew(lex);
  void (*to_run)();
  int old;
  ccmp->flags |= CCF_STRINGS_ON_HEAP; // We free the code data,so dont put code
                                      // data with string data
  CodeCtrlPush(ccmp);
  Lex(lex);
  while (PrsStmt(ccmp)) {
    to_run = Compile(ccmp, NULL, NULL, NULL);
    old = SetWriteNP(1);
    FFI_CALL_TOS_0(to_run);
    SetWriteNP(old);
    A_FREE(to_run);
    CodeCtrlPop(ccmp);
    CodeCtrlPush(ccmp);
    // TODO make a better way of doing this
    PrsAddSymbol("DolDocDumpIR", STK_DolDocDumpIR, 3);
    PrsAddSymbol("ScreenUpdateInProgress", ScreenUpdateInProgress, 0);
    PrsAddSymbol("SetVolume", STK_AiwniosSetVolume, 1);
    PrsAddSymbol("GetVolume", STK_AiwniosGetVolume, 0);
    PrsAddSymbol("__HC_ICAdd_Min_F64", STK___HC_ICAdd_Min_F64, 1);
    PrsAddSymbol("__HC_ICAdd_Max_F64", STK___HC_ICAdd_Max_F64, 1);
    PrsAddSymbol("__HC_ICAdd_Min_I64", STK___HC_ICAdd_Min_I64, 1);
    PrsAddSymbol("__HC_ICAdd_Max_I64", STK___HC_ICAdd_Max_I64, 1);
    PrsAddSymbol("__HC_ICAdd_Min_U64", STK___HC_ICAdd_Min_U64, 1);
    PrsAddSymbol("__HC_ICAdd_Max_U64", STK___HC_ICAdd_Max_U64, 1);
    PrsAddSymbol("CmdLineBootFiles", CmdLineBootFiles, 0);
    PrsAddSymbol("CmdLineBootFileCnt", CmdLineBootFileCnt, 0);
    PrsAddSymbol("CmdLineGetStr", CmdLineGetStr, 1);
    PrsAddSymbol("MPSetProfilerInt", STK_MPSetProfilerInt, 3);
    PrsAddSymbol("BoundsCheck", STK_BoundsCheck, 2);
    PrsAddSymbol("TaskContextSetRIP", STK_TaskContextSetRIP, 2);
    PrsAddSymbol("MakeContext", STK_AIWNIOS_makecontext, 3);
    PrsAddSymbol("__HC_ICAdd_RawBytes", STK__HC_ICAdd_RawBytes, 3);
    PrsAddSymbol("__HC_SetAOTRelocBeforeRIP", STK___HC_SetAOTRelocBeforeRIP, 2);
    PrsAddSymbol("__HC_CodeMiscIsUsed", STK___HC_CodeMiscIsUsed, 1);
    PrsAddSymbol("AiwniosSetClipboard", STK_AiwniosSetClipboard, 1);
    PrsAddSymbol("AiwniosGetClipboard", STK_AiwniosGetClipboard, 0);
    PrsAddSymbol("__HC_CmpCtrlDel", STK_CmpCtrlDel, 1);
    PrsAddSymbol("Cos", STK_cos, 1);
    PrsAddSymbol("Sin", STK_sin, 1);
    PrsAddSymbol("Tan", STK_tan, 1);
    PrsAddSymbol("Arg", STK_Arg, 2);
    PrsAddSymbol("ACos", STK_acos, 1);
    PrsAddSymbol("IsFastFail", IsFastFail, 0);
    PrsAddSymbol("ASin", STK_asin, 1);
    PrsAddSymbol("ATan", STK_atan, 1);
    PrsAddSymbol("Exp", STK_exp, 1);
    PrsAddSymbol("Btc", STK_Misc_Btc, 2);
    PrsAddSymbol("HeapCtrlInit", STK_HeapCtrlInit, 3);
    PrsAddSymbol("HeapCtrlDel", STK_HeapCtrlDel, 1);
    PrsAddSymbol("__MAlloc", STK___AIWNIOS_MAlloc, 2);
    PrsAddSymbol("__CAlloc", STK___AIWNIOS_CAlloc, 2);
    PrsAddSymbol("Free", STK___AIWNIOS_Free, 1);
    PrsAddSymbol("__HC_ICSetLock", STK___HC_ICSetLock, 1);
    PrsAddSymbol("MSize", STK_MSize, 1);
    PrsAddSymbol("__SleepHP", STK___SleepHP, 1);
    PrsAddSymbol("__GetTicksHP", STK___GetTicksHP, 0);
    PrsAddSymbol("__StrNew", STK___AIWNIOS_StrDup, 2);
    PrsAddSymbol("AiwniosPackRamDisk", AiwniosPackRamDiskPtr, 1);
    PrsAddSymbol("AiwniosPackBootCommand", AiwniosPackBootCommand, 0);
#define X(a, b) PrsAddSymbol(#a, STK_##a, b)
    X(MemCpy, 3);
    X(MemSet, 3);
    X(MemSetU16, 3);
    X(MemSetU32, 3);
    X(MemSetU64, 3);
    X(MemSetI64, 3);
    X(StrLen, 1);
    X(StrMatch, 2);
    X(StrIMatch, 2);
    X(StrCmp, 2);
    X(StrNCmp, 3);
    X(StrCpy, 2);
    X(StrICmp, 2);
    X(StrNICmp, 3);
#if defined(__x86_64__) && !defined(__OpenBSD__)
    X(OutU8, 2);
    X(OutU16, 2);
    X(OutU32, 2);
    X(RepOutU8, 3);
    X(RepOutU16, 3);
    X(RepOutU32, 3);
    X(InU8, 1);
    X(InU16, 1);
    X(InU32, 1);
    X(RepInU8, 3);
    X(RepInU16, 3);
    X(RepInU32, 3);
#endif
#undef X
    PrsAddSymbol("Log10", STK_log10, 1);
    PrsAddSymbol("Log2", STK_log2, 1);
    PrsAddSymbol("Pow10", STK_Pow10, 1);
    PrsAddSymbol("Pow", STK_pow, 2);
    PrsAddSymbol("PrintI", STK_PrintI, 2);
    PrsAddSymbol("PrintF", STK_PrintF, 2);
    PrsAddSymbol("Round", STK_round, 1);
    PrsAddSymbol("Ln", STK_log, 1);
    PrsAddSymbol("Floor", STK_floor, 1);
    PrsAddSymbol("Ceil", STK_ceil, 1);
    PrsAddSymbol("Sqrt", STK_sqrt, 1);
    PrsAddSymbol("MemCmp", STK_memcmp, 3);
    PrsAddSymbol("Bt", STK_Misc_Bt, 2);
    PrsAddSymbol("LBtc", STK_Misc_LBtc, 2);
    PrsAddSymbol("LBts", STK_Misc_LBts, 2);
    PrsAddSymbol("LBtr", STK_Misc_LBtr, 2);
    PrsAddSymbol("Bts", STK_Misc_Bts, 2);
    PrsAddSymbol("Btr", STK_Misc_Btr, 2);
    PrsAddSymbol("Bsf", STK_Bsf, 1);
    PrsAddSymbol("Bsr", STK_Bsr, 1);
    PrsAddSymbol("DbgPutS", STK_PutS, 1);
    PrsAddSymbol("PutS", STK_PutS, 1);
    PrsAddSymbol("PutS2", STK_PutS2, 1);
    PrsAddSymbol("SetFs", STK_SetHolyFs, 1);
    PrsAddSymbol("Fs", GetHolyFs, 0);
    PrsAddSymbol("WriteProtectMemCpy", STK_WriteProtectMemCpy, 3);
    PrsAddSymbolNaked("GetRBP", &Misc_BP, 0);
    //__Fs is special
    //__Gs is special then so add the RESULT OF THE function
    PrsAddSymbolNaked("__Fs", NULL, 0);
    ((CHashExport *)HashFind("__Fs", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1))
        ->val = GetHolyFsPtr();
    PrsAddSymbolNaked("__Gs", NULL, 0);
    ((CHashExport *)HashFind("__Gs", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1))
        ->val = GetHolyGsPtr();
    PrsAddSymbol("DebuggerClientSetGreg", STK_DebuggerClientSetGreg, 3);
    PrsAddSymbol("DebuggerClientStart", STK_DebuggerClientStart, 2);
    PrsAddSymbol("DebuggerClientEnd", STK_DebuggerClientEnd, 2);
    PrsAddSymbol("SpawnCore", STK_SpawnCore, 3);
    PrsAddSymbol("MPSleepHP", STK_MPSleepHP, 1);
    PrsAddSymbol("MPAwake", STK_MPAwake, 1);
    PrsAddSymbol("mp_cnt", STK_mp_cnt, 0);
    PrsAddSymbol("Gs", GetHolyGs, 0); // Gs just calls Thread local storage on
                                      // linux(not mutations in saved registers)
    PrsAddSymbol("SetGs", STK_SetHolyGs, 1);
    PrsAddSymbol("__GetTicks", STK___GetTicks, 0);
    PrsAddSymbol("__Sleep", STK___Sleep, 1);
    PrsAddSymbol("ImportSymbolsToHolyC", STK_ImportSymbolsToHolyC, 1);
    // These dudes will expected to return to a location on the stack,SO DONT
    // MUDDY THE STACK WITH ABI "translations"
    PrsAddSymbolNaked("AIWNIOS_SetJmp", AIWNIOS_getcontext, 1);
    PrsAddSymbolNaked("AIWNIOS_LongJmp", AIWNIOS_setcontext, 1);
    PrsAddSymbolNaked("Call", TempleOS_Call, 1);
    PrsAddSymbolNaked("CallArgs", TempleOS_CallN, 3);
    PrsAddSymbolNaked("CallVaArgs", TempleOS_CallVaArgs,
                      5); // fptr,argc1,argv1,argc,argv but argpop so ignored
                          //(this is for parser.c checks)
    PrsAddSymbol("__HC_ICAdd_ToBool", STK__HC_ICAdd_ToBool, 1);
    PrsAddSymbol("__HC_ICAdd_GetVargsPtr", STK___HC_ICAdd_GetVargsPtr, 1);
    PrsAddSymbol("IsValidPtr", STK_IsValidPtr, 1);
    PrsAddSymbol("__HC_CmpCtrl_SetAOT", STK___HC_CmpCtrl_SetAOT, 1);
    PrsAddSymbol("__HC_ICAdd_Typecast", STK___HC_ICAdd_Typecast, 3);
    PrsAddSymbol("__HC_ICAdd_SubCall", STK___HC_ICAdd_SubCall, 2);
    PrsAddSymbol("__HC_ICAdd_SubProlog", STK___HC_ICAdd_SubProlog, 1);
    PrsAddSymbol("__HC_ICAdd_SubRet", STK___HC_ICAdd_SubRet, 1);
    PrsAddSymbol("__HC_ICAdd_BoundedSwitch", STK___HC_ICAdd_Switch, 3);
    PrsAddSymbol("__HC_ICAdd_UnboundedSwitch", STK___HC_ICAdd_UnboundedSwitch,
                 2);
    PrsAddSymbol("__HC_ICAdd_PreInc", STK___HC_ICAdd_PreInc, 2);
    PrsAddSymbol("__HC_ICAdd_Call", STK___HC_ICAdd_Call, 4);
    PrsAddSymbol("__HC_ICAdd_F64", STK___HC_ICAdd_F64, 2);
    PrsAddSymbol("__HC_ICAdd_I64", STK___HC_ICAdd_I64, 2);
    PrsAddSymbol("__HC_ICAdd_PreDec", STK___HC_ICAdd_PreDec, 2);
    PrsAddSymbol("__HC_ICAdd_PostDec", STK___HC_ICAdd_PostDec, 2);
    PrsAddSymbol("__HC_ICAdd_PostInc", STK___HC_ICAdd_PostInc, 2);
    PrsAddSymbol("__HC_ICAdd_Pow", STK___HC_ICAdd_Pow, 1);
    PrsAddSymbol("__HC_ICAdd_Eq", STK___HC_ICAdd_Eq, 1);
    PrsAddSymbol("__HC_ICAdd_Div", STK___HC_ICAdd_Div, 1);
    PrsAddSymbol("__HC_ICAdd_Sub", STK___HC_ICAdd_Sub, 1);
    PrsAddSymbol("__HC_ICAdd_Mul", STK___HC_ICAdd_Mul, 1);
    PrsAddSymbol("__HC_ICAdd_Add", STK___HC_ICAdd_Add, 1);
    PrsAddSymbol("__HC_ICAdd_Deref", STK___HC_ICAdd_Deref, 3);
    PrsAddSymbol("__HC_ICAdd_Comma", STK___HC_ICAdd_Comma, 1);
    PrsAddSymbol("__HC_ICAdd_Addr", STK___HC_ICAdd_Addr, 1);
    PrsAddSymbol("__HC_ICAdd_Xor", STK___HC_ICAdd_Xor, 1);
    PrsAddSymbol("__HC_ICAdd_Mod", STK___HC_ICAdd_Mod, 1);
    PrsAddSymbol("__HC_ICAdd_Or", STK___HC_ICAdd_Or, 1);
    PrsAddSymbol("__HC_ICAdd_Lt", STK___HC_ICAdd_Lt, 1);
    PrsAddSymbol("__HC_ICAdd_Gt", STK___HC_ICAdd_Gt, 1);
    PrsAddSymbol("__HC_ICAdd_Le", STK___HC_ICAdd_Le, 1);
    PrsAddSymbol("__HC_ICAdd_Ge", STK___HC_ICAdd_Ge, 1);
    PrsAddSymbol("__HC_ICAdd_LNot", STK___HC_ICAdd_LNot, 1);
    PrsAddSymbol("__HC_ICAdd_Vargs", STK___HC_ICAdd_Vargs, 2);
    PrsAddSymbol("__HC_ICAdd_BNot", STK___HC_ICAdd_BNot, 1);
    PrsAddSymbol("__HC_ICAdd_AndAnd", STK___HC_ICAdd_AndAnd, 1);
    PrsAddSymbol("__HC_ICAdd_OrOr", STK___HC_ICAdd_OrOr, 1);
    PrsAddSymbol("__HC_ICAdd_XorXor", STK___HC_ICAdd_XorXor, 1);
    PrsAddSymbol("__HC_ICAdd_Ne", STK___HC_ICAdd_Ne, 1);
    PrsAddSymbol("__HC_ICAdd_Lsh", STK___HC_ICAdd_Lsh, 1);
    PrsAddSymbol("__HC_ICAdd_Rsh", STK___HC_ICAdd_Rsh, 1);
    PrsAddSymbol("__HC_ICAdd_AddEq", STK___HC_ICAdd_AddEq, 1);
    PrsAddSymbol("__HC_ICAdd_SubEq", STK___HC_ICAdd_SubEq, 1);
    PrsAddSymbol("__HC_ICAdd_MulEq", STK___HC_ICAdd_MulEq, 1);
    PrsAddSymbol("__HC_ICAdd_DivEq", STK___HC_ICAdd_DivEq, 1);
    PrsAddSymbol("__HC_ICAdd_LshEq", STK___HC_ICAdd_LshEq, 1);
    PrsAddSymbol("__HC_ICAdd_RshEq", STK___HC_ICAdd_RshEq, 1);
    PrsAddSymbol("__HC_ICAdd_AndEq", STK___HC_ICAdd_AndEq, 1);
    PrsAddSymbol("__HC_ICAdd_OrEq", STK___HC_ICAdd_OrEq, 1);
    PrsAddSymbol("__HC_ICAdd_XorEq", STK___HC_ICAdd_XorEq, 1);
    PrsAddSymbol("__HC_ICAdd_ModEq", STK___HC_ICAdd_ModEq, 1);
    PrsAddSymbol("__HC_ICAdd_FReg", STK___HC_ICAdd_FReg, 2);
    PrsAddSymbol("__HC_ICAdd_IReg", STK___HC_ICAdd_IReg, 4);
    PrsAddSymbol("__HC_ICAdd_Frame", STK___HC_ICAdd_Frame, 4);
    PrsAddSymbol("__HC_CodeMiscStrNew", STK___HC_CodeMiscStrNew, 3);
    PrsAddSymbol("__HC_CodeMiscLabelNew", STK___HC_CodeMiscLabelNew, 2);
    PrsAddSymbol("__HC_CmpCtrlNew", STK___HC_CmpCtrlNew, 0);
    PrsAddSymbol("__HC_CodeCtrlPush", STK___HC_CodeCtrlPush, 1);
    PrsAddSymbol("__HC_CodeCtrlPop", STK___HC_CodeCtrlPop, 1);
    PrsAddSymbol("__HC_Compile", STK___HC_Compile, 4);
    PrsAddSymbol("__HC_CodeMiscStrNew", STK___HC_CodeMiscStrNew, 3);
    PrsAddSymbol("__HC_CodeMiscJmpTableNew", STK___HC_CodeMiscJmpTableNew, 4);
    PrsAddSymbol("__HC_ICAdd_Label", STK___HC_ICAdd_Label, 2);
    PrsAddSymbol("__HC_ICAdd_Goto", STK___HC_ICAdd_Goto, 2);
    PrsAddSymbol("__HC_ICAdd_GotoIf", STK___HC_ICAdd_GotoIf, 2);
    PrsAddSymbol("__HC_ICAdd_Str", STK___HC_ICAdd_Str, 2);
    PrsAddSymbol("__HC_ICAdd_And", STK___HC_ICAdd_And, 1);
    PrsAddSymbol("__HC_ICAdd_Lock", STK___HC_ICAdd_Lock, 1);
    PrsAddSymbol("__HC_ICAdd_Fs", STK___HC_ICAdd_Fs, 1);
    PrsAddSymbol("__HC_ICAdd_Gs", STK___HC_ICAdd_Gs, 1);
    PrsAddSymbol("__HC_ICAdd_EqEq", STK___HC_ICAdd_EqEq, 1);
    PrsAddSymbol("__HC_ICAdd_Neg", STK___HC_ICAdd_Neg, 1);
    PrsAddSymbol("__HC_ICAdd_Ret", STK___HC_ICAdd_Ret, 1);
    PrsAddSymbol("__HC_ICAdd_Arg", STK___HC_ICAdd_Arg, 2);
    PrsAddSymbol("__HC_ICAdd_SetFrameSize", STK___HC_ICAdd_SetFrameSize, 2);
    PrsAddSymbol("__HC_ICAdd_Reloc", STK___HC_ICAdd_Reloc, 6);
    PrsAddSymbol("__HC_ICAdd_RelocUnique", STK___HC_ICAdd_RelocUnique, 6);
    PrsAddSymbol("__HC_ICSetLine", STK___HC_ICSetLine, 2);
    PrsAddSymbol("__HC_ICAdd_StaticRef", STK___HC_ICAdd_StaticRef, 4);
    PrsAddSymbol("__HC_ICAdd_StaticData", STK___HC_ICAdd_StaticData, 5);
    PrsAddSymbol("__HC_ICAdd_SetStaticsSize", STK___HC_ICAdd_SetStaticsSize, 2);
    PrsAddSymbol("__HC_ICAdd_ToI64", STK___HC_ICAdd_ToI64, 1);
    PrsAddSymbol("__HC_ICAdd_ToF64", STK___HC_ICAdd_ToF64, 1);
    PrsAddSymbol("__HC_ICAdd_ShortAddr", STK___HC_ICAdd_ShortAddr, 4);
    PrsAddSymbol("__HC_CodeMiscInterateThroughRefs",
                 STK___HC_CodeMiscInterateThroughRefs, 3);
    PrsAddSymbol("__HC_ICAdd_BT", STK___HC_ICAdd_BT, 1);
    PrsAddSymbol("__HC_ICAdd_BTS", STK___HC_ICAdd_BTS, 1);
    PrsAddSymbol("__HC_ICAdd_BTR", STK___HC_ICAdd_BTR, 1);
    PrsAddSymbol("__HC_ICAdd_BTC", STK___HC_ICAdd_BTC, 1);
    PrsAddSymbol("__HC_ICAdd_LBTS", STK___HC_ICAdd_LBTS, 1);
    PrsAddSymbol("__HC_ICAdd_LBTR", STK___HC_ICAdd_LBTR, 1);
    PrsAddSymbol("__HC_ICAdd_LBTC", STK___HC_ICAdd_LBTC, 1);
    PrsAddSymbol("Caller", STK_Misc_Caller, 1);
    PrsAddSymbol("VFsSetPwd", STK_VFsSetPwd, 1);
    PrsAddSymbol("VFsExists", STK_VFsFileExists, 1);
    PrsAddSymbol("VFsIsDir", STK_VFsIsDir, 1);
    PrsAddSymbol("VFsFSize", STK_VFsFSize, 1);
    PrsAddSymbol("VFsFRead", STK_VFsFileRead, 2);
    PrsAddSymbol("VFsFWrite", STK_VFsFileWrite, 3);
    PrsAddSymbol("VFsDel", STK_VFsDel, 1);
    PrsAddSymbol("VFsDir", VFsDir, 0);
    PrsAddSymbol("VFsDirMk", STK_VFsDirMk, 1);
    PrsAddSymbol("VFsFBlkRead", STK_VFsBlkRead, 4);
    PrsAddSymbol("VFsFBlkWrite", STK_VFsBlkWrite, 4);
    PrsAddSymbol("VFsFOpen", STK_VFsFOpen, 2);
    PrsAddSymbol("VFsFClose", STK_VFsFClose, 1);
    PrsAddSymbol("VFsFSeek", STK_VFsFSeek, 2);
    PrsAddSymbol("VFsSetDrv", STK_VFsSetDrv, 1);
    PrsAddSymbol("FUnixTime", STK_VFsUnixTime, 1);
    PrsAddSymbol("FSize", STK_VFsFSize, 1);
    PrsAddSymbol("VFsFTrunc", STK_VFsTrunc, 2);
    PrsAddSymbol("UnixNow", STK_UnixNow, 0);
    PrsAddSymbol("__GrPaletteColorSet", STK_GrPaletteColorSet, 2);
    PrsAddSymbol("DrawWindowNew", STK_DrawWindowNew, 0);
    PrsAddSymbol("UpdateScreen", STK_UpdateScreen, 4);
    PrsAddSymbol("SetKBCallback", STK_SetKBCallback, 1);
    PrsAddSymbol("SndFreq", STK_SndFreq, 1);
    PrsAddSymbol("__HC_ICAdd_Sqrt", STK___HC_ICAdd_Sqrt, 1);
    PrsAddSymbol("__HC_ICAdd_Sqr", STK___HC_ICAdd_Sqr, 1);
    PrsAddSymbol("SetMSCallback", STK_SetMSCallback, 1);
    PrsAddSymbol("InteruptCore", STK_InteruptCore, 1);
    PrsAddSymbol("ExitAiwnios", ExitAiwnios, 1);
    PrsAddSymbol("NetSocketNew", STK_NetSocketNew, 1);
    PrsAddSymbol("NetUDPAddrNew", STK_NetUDPAddrNew, 3);
    PrsAddSymbol("NetUDPSocketNew", STK_NetUDPSocketNew, 1);
    PrsAddSymbol("NetUDPRecvFrom", STK_NetUDPRecvFrom, 4);
    PrsAddSymbol("NetUDPSendTo", STK_NetUDPSendTo, 4);
    PrsAddSymbol("NetUDPAddrDel", STK_NetUDPAddrDel, 1);
    PrsAddSymbol("NetIP4ByHost", STK_NetIP4ByHost, 1);
    PrsAddSymbol("NetBindIn", STK_NetBindIn, 2);
    PrsAddSymbol("NetListen", STK_NetListen, 2);
    PrsAddSymbol("NetAccept", STK_NetAccept, 2);
    PrsAddSymbol("NetClose", STK_NetClose, 1);
    PrsAddSymbol("NetRead", STK_NetRead, 3);
    PrsAddSymbol("NetWrite", STK_NetWrite, 3);
    PrsAddSymbol("NetPollForHangup", STK_NetPollForHangup, 2);
    PrsAddSymbol("NetPollForRead", STK_NetPollForRead, 2);
    PrsAddSymbol("NetPollForWrite", STK_NetPollForWrite, 2);
    PrsAddSymbol("NetAddrDel", STK_NetAddrDel, 1);
    PrsAddSymbol("NetAddrNew", STK_NetAddrNew, 3);
    PrsAddSymbol("NetConnect", STK_NetConnect, 2);
    PrsAddSymbol("_SixtyFPS", STK_60fps, 0);
    PrsAddSymbol("IsCmdLineMode", IsCmdLineMode, 0);
    PrsAddSymbol("AiwniosTUIEnable", AiwniosTUIEnable, 0);
    PrsAddSymbol("IsCmdLineMode2", IsCmdLineMode2, 0); // Sexy text mode
    PrsAddSymbol("TermSize", STK_TermSize, 2);
    PrsAddSymbol("AIWNIOS_SetCaptureMouse", STK_SetCaptureMouse, 1);
    PrsAddSymbol("CSPRNG", STK_CSPRNG, 2);
  }
  CmpCtrlDel(ccmp);
  LexerDel(lex);
}
static const char *t_drive;
static const char *exe_name = NULL;
static char *ramdisk_ptr = NULL;
static int64_t ramdisk_size = 0;
static char *boot_command = NULL;
// See Src/AiwniosPack
static char *AiwniosPackRamDiskPtr(int64_t **stk) {
  int64_t *sz = stk[0];
  if (sz)
    *sz = ramdisk_size;
  return ramdisk_ptr;
}
// See Src/AiwniosPack
static char *AiwniosPackBootCommand(int64_t **stk) {
  return boot_command;
}

typedef struct CAiwniosPack {
  // Non-absolute paths are relative to home directory
  int64_t ramdisk_size;
  int64_t hcrt_size;
  int64_t hcrt_offset;
  int64_t ramdisk_offset;
  char boot_command[144];
  char save_directory[144];
} CAiwniosPack;
static void Boot() {
  int64_t len, size, hcrt_size;
  char *fbuf, *hcrt;
  Fs = calloc(sizeof(CTask), 1);
  TaskInit(Fs, NULL, 0);
  char *host_abi;
  if (exe_name) {
    uint64_t offset;
    // See AiwniosPack/Embed.HC
    if (!(fbuf = FileRead(exe_name, &size))) {
      goto normal;
    }
    if (size < 16) {
      abort();
    }
    if (strncmp(fbuf + size - 16, "AiwnPack", 8)) {
      A_FREE(fbuf);
      goto normal;
    }
    offset = *(int64_t *)(fbuf + size - 8);
    if (!(offset > 0 && offset < size)) {
      A_FREE(fbuf);
      goto normal;
    }
    CAiwniosPack *cpack = fbuf + offset;
    hcrt = fbuf + cpack->hcrt_offset;
    hcrt_size = cpack->hcrt_size;
    ramdisk_ptr = fbuf + cpack->ramdisk_offset;
    ramdisk_size = cpack->ramdisk_size;
    boot_command = cpack->boot_command;
    printf("PACK_HCRT:%p,%d\n", cpack->hcrt_offset, cpack->hcrt_size);
    printf("PACK_RAMDISK:%p,%d\n", cpack->ramdisk_offset, cpack->ramdisk_size);
    printf("PACK_BOOTCOMD:%s\n", cpack->boot_command);
  } else {
  normal:;
    char bin[strlen("HCRT2.BIN") + strlen(t_drive) + 1 + 1];
    strcpy(bin, t_drive);
    strcat(bin, "/HCRT2.BIN");
    hcrt = FileRead(bin, &size);
    hcrt_size = size;
  }
  InstallDbgSignalsForThread();
  VFsMountDrive('T', t_drive);
  /*  FuzzTest1();
    FuzzTest2();
    FuzzTest3();*/
  if (arg_bootstrap_bin->count) {
#define BOOTSTRAP_FMT                                                          \
  "#define TARGET_%s 1\n"                                                      \
  "#define lastclass \"U8\"\n"                                                 \
  "#define public \n"                                                          \
  "#define IMPORT_AIWNIOS_SYMS 1\n"                                            \
  "#define TEXT_MODE 1\n"                                                      \
  "#define BOOTSTRAP 1\n"                                                      \
  "#define HOST_ABI '%s'\n"                                                    \
  "#include \"Src/FULL_PACKAGE.HC\";;\n"
#if defined(__aarch64__) || defined(_M_ARM64)
#  if defined(__APPLE__)
    host_abi = "Apple";
#  else
    host_abi = "SysV";
#  endif
    len = snprintf(NULL, 0, BOOTSTRAP_FMT, "AARCH64", host_abi);
    char buf[len + 1];
    sprintf(buf, BOOTSTRAP_FMT, "AARCH64", host_abi);
#elif defined(__x86_64__)
#  if defined(_WIN32) || defined(WIN32)
    host_abi = "Win";
#  elif defined(__OpenBSD__)
    host_abi = "OpenBSD";
#  else
    host_abi = "SysV";
#  endif
    len = snprintf(NULL, 0, BOOTSTRAP_FMT, "X86", host_abi);
    char buf[len + 1];
    sprintf(buf, BOOTSTRAP_FMT, "X86", host_abi);
#elif defined(__riscv) || defined(__riscv__)
    host_abi = "SysV";
    len = snprintf(NULL, 0, BOOTSTRAP_FMT, "RISCV", host_abi);
    char buf[len + 1];
    sprintf(buf, BOOTSTRAP_FMT, "RISCV", host_abi);
#else
#  error "Arch not supported"
#endif
    BootAiwnios(buf);
  } else
    BootAiwnios(NULL);
  glbl_table = Fs->hash_table;
  if (hcrt)
    Load(hcrt, hcrt_size);
}
static int64_t quit = 0, quit_code = 0;
static void AiwniosBye() {
  static int64_t done = 0;
  if (!done) {
    done = 1;
    DeinitVideo();
    DeinitSound();
    SDL_Quit();
  }
}
static void ExitAiwnios(int64_t *stk) {
  quit = 1, quit_code = stk[0];
  if (arg_cmd_line->count || arg_bootstrap_bin->count) {
    AiwniosBye();
    exit(stk[0]);
  } else {
    SDL_QuitEvent qev;
    qev.type = SDL_QUIT;
    qev.timestamp = SDL_GetTicks();
    SDL_PushEvent(&qev);
  }
}
static int64_t IsAiwniosPackApp() {
  char signature[9];
  signature[8] = 0;
  if (exe_name) {
    FILE *f = fopen(exe_name, "rb");
    fseek(f, -16, SEEK_END);
    fread(signature, 1, 8, f);
    fclose(f);
    if (!strcmp(signature, "AiwnPack"))
      return 1;
  }
  return 0;
}
int main(int argc, char **argv) {
  SDL_SetMainReady();
  exe_name = argv[0];
  setlocale(LC_ALL, "C");
  atexit(&AiwniosBye);
#ifndef _WIN64
  struct rlimit rl;
  getrlimit(RLIMIT_NOFILE, &rl);
  rl.rlim_cur = rl.rlim_max;
  setrlimit(RLIMIT_NOFILE, &rl);
#endif
  __bootstrap_tls();
  void *argtable[] = {
      arg_help = arg_lit0("h", "help", "Show the help message"),
      arg_overwrite =
          arg_lit0("o", "overwrite",
                   "Overwrite the T directory with the installed T template."),
      arg_t_dir = arg_file0("t", NULL, "Directory",
                            "Specify the boot drive(dft is current dir)."),
      arg_bootstrap_bin =
          arg_lit0("b", "bootstrap",
                   "Build a new binary with the \"slim\" compiler of aiwnios."),
      arg_asan_enable =
          arg_lit0("a", "address-sanitize", "Enable bounds checking."),
      arg_new_boot_dir = arg_lit0("n", "new-boot-dir",
                                  "Create a new boot directory(backs up old "
                                  "boot directory if present)."),
#if !defined(WIN32) && !defined(_WIN32)
      arg_fork =
          arg_lit0("f", "fork", "Fork to background (for FreeBSD daemons)"),
      arg_pidfile =
          arg_file0("p", "pidfile", "<path>", "PID file (for services)"),
#endif
      arg_grab = arg_lit0(
          "g", "grab-focus",
          "Grab the keyboard(Windows/Logo key will be handled by aiwnios)"),
      arg_no_debug = arg_lit0(
          "d", "user-debugger",
          "Faults will be handled by an external debugger(such as gdb)."),
      sixty_fps = arg_lit0("6", "60fps", "Run in 60 fps mode."),
      arg_cmd_line = arg_lit0("c", NULL, "Run in command line mode(dumb)."),
      arg_cmd_line2 = arg_lit0(
          NULL, "tui", "Run in text mode(command line mode on steriods)."),
      arg_boot_files =
          arg_filen(NULL, NULL, "Command Line Boot files", 0, 100000,
                    "Files to run on  boot in command line mode."),
      arg_fast_fail =
          arg_lit0("F", "fast-fail",
                   "Instantly fail on signals(like segmentation faults)."),
      _arg_end = arg_end(20),
  };
  int64_t errors, idx;
  errors = arg_parse(argc, argv, argtable);
  if (errors || arg_help->count) {
    if (errors)
      arg_print_errors(stdout, _arg_end, "aiwnios");
    printf("Usage: aiwnios\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    exit(1);
  }
  if (arg_fast_fail->count)
    is_fast_fail = 1;
  if (arg_grab->count)
    sdl_window_grab_enable = 1;
#if defined(__x86_64__) && (defined(__FreeBSD__) || defined(__linux__))
#  ifdef __linux__
  __iofd = open(__iofd_str = "/dev/port", O_RDWR);
#  elif defined(__FreeBSD__)
  __iofd = open(__iofd_str = "/dev/io", O_RDWR);
#  endif
  if (-1 == __iofd)
    __iofd_errno = errno;
#endif
#ifndef __APPLE__
  void DebuggerBegin(void);
  if (!arg_no_debug->count)
    DebuggerBegin();
#endif
#ifndef _WIN32
  if (arg_fork->count) {
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid > 0)
      return 0; // parent
    assert(setsid() >= 0);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    pid = fork();
    assert(pid >= 0);
    if (pid > 0)
      return 0; // parent(child of previous parent, we want the grandchild)
    if (arg_pidfile->count) {
      int fd = open(arg_pidfile->filename[0], O_RDWR | O_CREAT, 0666);
      if (-1 == fd)
        goto nowrite;
      ftruncate(fd, 0);
      char buf[0x20];
      int written = snprintf(buf, sizeof buf, "%jd", getpid());
      write(fd, buf, written);
      close(fd);
    }
  nowrite:
    umask(0);
  }
#endif
  t_drive = NULL;
  if (arg_asan_enable->count)
    InitBoundsChecker();
  if (arg_t_dir->count)
    t_drive = arg_t_dir->filename[0];
  else if (arg_bootstrap_bin->count)
    t_drive = "."; // Bootstrap in current directory
  if (IsAiwniosPackApp() && !arg_t_dir->count) {
    t_drive = ResolveBootDir(".", 0, NULL);
  } else {
#if !defined(WIN32) && !defined(_WIN32)
    struct passwd *pwd = getpwuid(getuid());
    const char *dft = "/.local/share/aiwnios/T";
    char *home = ".";
    if (pwd)
      home = pwd->pw_dir;
    char template_dir[strlen(dft) + strlen(home) + 1];
    strcpy(template_dir, home);
    strcat(template_dir, dft);
    if ((!arg_t_dir->count || arg_overwrite->count ||
         arg_new_boot_dir->count) &&
        !arg_bootstrap_bin->count)
      t_drive = ResolveBootDir(!t_drive ? template_dir : t_drive,
                               arg_new_boot_dir->count, AIWNIOS_TEMPLATE_DIR);
#else
    int64_t has_installed = 0;
    char installed_at[MAX_PATH];
    installed_at[0] = 0;
    long reg_size = 0;
    RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Aiwnios", "InstallAt",
                 RRF_RT_REG_MULTI_SZ, NULL, NULL, &reg_size);
    if (reg_size > 0) {
      char inst_dir[reg_size + 1];
      RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Aiwnios", "InstallAt",
                   RRF_RT_REG_MULTI_SZ, NULL, inst_dir, &reg_size);
      strcpy(installed_at, inst_dir);
      has_installed = 1;
    }
    char home_dir[MAX_PATH];
    strcpy(home_dir, "");
    SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, home_dir);
    // Dumb haCk
    // Windows doesnt know how to do lowerase C anymore. I dont know what I
    // fuked up
    sprintf(home_dir + strlen(home_dir), "\\.local\\share\\aiwnios\\T");
    if ((!arg_t_dir->count || arg_overwrite->count ||
         arg_new_boot_dir->count) &&
        !arg_bootstrap_bin->count) {
      t_drive = ResolveBootDir(!t_drive ? home_dir : t_drive,
                               arg_new_boot_dir->count, installed_at);
      // Dont use system wide directory we are installed in(the place we start
      // running aiwnios in when installed on windows)
      if (has_installed && t_drive) {
        char poo1[MAX_PATH];
        char poo2[MAX_PATH];
        GetFullPathNameA(installed_at, MAX_PATH, poo1, NULL);
        GetFullPathNameA(t_drive, MAX_PATH, poo2, NULL);
        if (!strcmp(poo1, poo2)) {
          // Same file
          t_drive = ResolveBootDir(home_dir, 1, installed_at);
        }
      }
    }
#endif
  }
  if (arg_new_boot_dir->count)
    exit(EXIT_SUCCESS);
  InitSound();
  if (!(arg_cmd_line->count || arg_bootstrap_bin->count)) {
    if (0 > SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
      char *p = SDL_GetError();
      if (!p)
        p = "???";
      int64_t l = snprintf(NULL, 0, "Failed to init SDL(%s)", p);
      char buf[l + 1];
      sprintf(buf, "Failed to init SDL(%s)", p);
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "AIWNIOS", buf, NULL);
      exit(EXIT_FAILURE);
    }
    user_ev_num = SDL_RegisterEvents(1);
    SpawnCore(&Boot, argv[0], 0);
    InputLoop(&quit);
  } else {
    Boot(argv[0]);
  }
  AiwniosBye(); // My RISCV(and presumably others) dont like atexit with
                // SDL_QUIT
  arg_freetable(argtable, sizeof(argtable) / sizeof(*argtable));
  return quit_code;
}
