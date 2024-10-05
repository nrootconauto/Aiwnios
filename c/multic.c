#include "aiwn_asm.h"
#include "aiwn_fs.h"
#include "aiwn_hash.h"
#include "aiwn_mem.h"
#include "aiwn_multic.h"
#include <SDL.h>
#include <inttypes.h>
#include <signal.h>
#include <stdlib.h>

#ifdef __x86_64__
#  include <errno.h>
#  include <string.h>
#  ifdef __linux__
#    include <sys/syscall.h>
#    include <ucontext.h>
#  elif defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  endif
#  ifndef __SEG_FS
#    ifdef __clang__
#      define __seg_fs __attribute__((address_space(257)))
#      define __seg_gs __attribute__((address_space(256)))
#    else
#      error Compiler too old (should never happen)
#    endif
#  endif
/****************************************************************
 * The Windows TIB has an ULONG[31] array at offset %gs:0x80
 * This array's size is 0x80, which gives us plenty of space
 * from [0x80,0x100)
 *
 * On Wine a structure called struct ntuser_thread_info that
 * takes up 0x40 is at the start of the reserved area, so we
 * need to look at the end. on the DEC Alpha the first ULONG
 * of the TIB area is ued by Wow64ExitThread which shouldn't
 * be a concern since UserReserved has already been occupied
 * by Wine. Problem being Wow64ExitThread uses UserReserved[0].
 * UserReserved starts from %gs:0xCF as far as I am aware.
 * and base/fs/mup/lock.c, base/fs/srv/lock.c use UserReserved
 * from index 0 to 2. Now we're only left with whatever is in
 * between the struct ntuser_thread_info and UserReserved, or
 * use UserReserved[3] and UserReserved[4]. The answer is obvious.
 * (Wow64ExitThread is in base/wow64/wow64/thread.c)
 * (source: https://github.com/tongzx/nt5src)
 *
 * Linux/FreeBSD use FS for TLS, but we will allocate space on
 * %gs for compatibility with windows ABI. (__bootstrap_tls())
 ***************************************************************/
#  define FS_OFF   0xF0
#  define GS_OFF   0xF8
#  define ThreadFs (*(void *__seg_gs *)FS_OFF)
#  define ThreadGs (*(void *__seg_gs *)GS_OFF)
#elif defined(__aarch64__)
register long aiwnios_tls_reg asm("x28");
#  define FS_OFF   0
#  define GS_OFF   8
#  define ThreadFs (*(void **)(aiwnios_tls_reg + FS_OFF))
#  define ThreadGs (*(void **)(aiwnios_tls_reg + GS_OFF))
#endif

#if defined(__riscv) || defined(__riscv__)
static _Thread_local void *ThreadFs;
static _Thread_local void *ThreadGs;

void *GetHolyGsPtr() {
  char *gs = &ThreadGs, *tls;
  asm volatile("mv\t%0,tp" : "=r"(tls));
  return gs - tls;
}
void *GetHolyFsPtr() {
  char *fs = &ThreadFs, *tls;
  asm volatile("mv\t%0,tp" : "=r"(tls));
  return fs - tls;
}
#elif defined(__x86_64__) || defined(__aarch64__)
void *GetHolyGsPtr() {
  return (void *)GS_OFF;
}
void *GetHolyFsPtr() {
  return (void *)FS_OFF;
}
#else

#  error This isn't an architecture that's supported yet. Check again later.

#endif

void SetHolyFs(void *new) {
  ThreadFs = new;
}
void SetHolyGs(void *new) {
  ThreadGs = new;
}
void *GetHolyFs() {
  return ThreadFs;
}
void *GetHolyGs() {
  return ThreadGs;
}

__attribute__((maybe_unused)) static void __sigillhndlr(int sig) {
  (void)sig;
  fprintf(stderr, "\e[0;31mCRITICAL\e[0m: "
                  "processor too old\n");
  fflush(stderr);
  // do not ever change this to normal exit()
  // this is a harsh exit that ignores atexit
  _Exit(-1);
}

void __bootstrap_tls(void) {
#ifdef __aarch64__
  aiwnios_tls_reg = (long)calloc(1, 0x10);
#elif defined(__x86_64__) && !defined(_WIN64)
  void *tls = calloc(1, 0x10) - 0xF0;
  int ret = -1;
#  ifdef __linux__
  asm volatile("syscall"   //    (not defined in musl)
               : "=a"(ret) //         ARCH_SET_GS
               : "0"(SYS_arch_prctl), "D"(0x1001), "S"(tls)
               : "rcx", "r11", "memory");
#  elif defined(__FreeBSD__)
  ret = amd64_set_gsbase(tls);
#  endif
  if (!ret)
    return;
  fprintf(stderr,
          "\e[0;31mCRITICAL\e[0m: "
          "Failed setting GS with error "
          "\"%s\"; retrying with wrgsbase "
          "(only available since ivy bridge)\n",
          strerror(errno));
  fflush(stderr);
  struct sigaction sa = {.sa_handler = __sigillhndlr};
  sigaction(SIGILL, &sa, 0);
  asm("wrgsbase\t%0" : : "r"(tls));
  sa.sa_handler = SIG_DFL;
  sigaction(SIGILL, &sa, 0);
#endif
}

typedef struct {
  void (*fp)(), *gs;
  int64_t num;
  void (*profiler_int)(void *fs);
  CHashTable *parent_table;
} CorePair;
#ifdef __linux__
#  include <linux/futex.h>
#elif defined(__FreeBSD__)
#  include <sys/types.h>
#  include <sys/umtx.h>
#elif defined(__APPLE__)
#  include <dlfcn.h>
#  define PRIVATE 1
#  include "c/ulock.h" //Not canoical
#  undef PRIVATE
#endif

#ifndef _WIN64
#  include <pthread.h>
#  include <sys/syscall.h>
#  include <sys/time.h>
#  include <unistd.h>
typedef struct {
  pthread_t pt;
  int wake_futex;
  void (*profiler_int)(void *fs);
  int64_t profiler_freq;
  struct itimerval profile_timer;
#  if defined(__APPLE__)
  pthread_cond_t wake_cond;
#  endif
} CCPU;
#else
#  include <windows.h>
#  include <winnt.h>
#  include <memoryapi.h>
#  include <processthreadsapi.h>
#  include <profileapi.h>
#  include <synchapi.h>
#  include <sysinfoapi.h>
#  include <time.h>
typedef struct {
  HANDLE thread;
  CONTEXT ctx;
  int64_t sleep;
  void (*profiler_int)(void *fs);
  int64_t profiler_freq, next_prof_int;
  uint8_t *alt_stack;
} CCPU;
static int64_t nproc;
#endif
static _Thread_local core_num = 0;

static void threadrt(CorePair *pair) {
  __bootstrap_tls();
#ifndef _WIN64
  stack_t stk = {
      .ss_sp = malloc(SIGSTKSZ),
      .ss_size = SIGSTKSZ,
  };
  sigaltstack(&stk, NULL);
#endif
  InstallDbgSignalsForThread();
  DebuggerClientWatchThisTID();
  Fs = calloc(sizeof(CTask), 1);
  VFsThrdInit();
  TaskInit(Fs, NULL, 0);
  Fs->hash_table->next = pair->parent_table;
  SetHolyGs(pair->gs);
  core_num = pair->num;
  void (*fp)();
  fp = pair->fp;
  free(pair);
  fp();
}
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
static CCPU cores[128];
CHashTable *glbl_table;

void InteruptCore(int64_t core) {
  pthread_kill(cores[core].pt, SIGUSR1);
}
static void InteruptRt(int ul, siginfo_t *info, ucontext_t *_ctx) {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  pthread_sigmask(SIG_UNBLOCK, &set, NULL);
  CHashExport *y = HashFind("InteruptRt", glbl_table, HTT_EXPORT_SYS_SYM, 1);
  mcontext_t *ctx = &_ctx->uc_mcontext;
  void (*fp)();
  if (y) {
    fp = y->val;
#  if defined(__FreeBSD__) && defined(__x86_64__)
    FFI_CALL_TOS_2(fp, ctx->mc_rip, ctx->mc_rbp);
#  elif defined(__linux__) && defined(__x86_64__)
    FFI_CALL_TOS_2(fp, ctx->gregs[REG_RIP], ctx->gregs[REG_RBP]);
#  endif
#  if (defined(_M_ARM64) || defined(__aarch64__)) && defined(__FreeBSD__)
    FFI_CALL_TOS_2(fp, NULL, ctx->mc_gpregs.gp_x[29]);
#  elif (defined(__riscv) || defined(__riscv__)) && defined(__linux__)
    FFI_CALL_TOS_2(fp, NULL, ctx->__gregs[8]);
#  elif (defined(_M_ARM64) || defined(__aarch64__)) && defined(__linux__)
    FFI_CALL_TOS_2(fp, NULL, ctx->regs[29]);
#  endif
  }
}
static void ExitCoreRt(int s) {
  pthread_exit(0);
}

static void ProfRt(int64_t sig, siginfo_t *info, ucontext_t *_ctx) {
  int64_t c = core_num;
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGPROF);
  if (!pthread_equal(pthread_self(), cores[c].pt)) {
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    return;
  }
  if (cores[c].profiler_int) {
#  if defined(__x86_64__)
#    if defined(__FreeBSD__)
    FFI_CALL_TOS_CUSTOM_BP(_ctx->uc_mcontext.mc_rbp, cores[c].profiler_int,
                           _ctx->uc_mcontext.mc_rip);
#    elif defined(__linux__)
    FFI_CALL_TOS_CUSTOM_BP(_ctx->uc_mcontext.gregs[REG_RBP],
                           cores[c].profiler_int,
                           _ctx->uc_mcontext.gregs[REG_RIP]);
#    endif
#  endif
#  if defined(__riscv) || defined(__riscv__)
#    if defined(__linux__)
    FFI_CALL_TOS_CUSTOM_BP(_ctx->uc_mcontext.__gregs[8], cores[c].profiler_int,
                           _ctx->uc_mcontext.__gregs[1]);
#    endif
#  endif
    cores[c].profile_timer.it_value.tv_usec = cores[c].profiler_freq;
    cores[c].profile_timer.it_interval.tv_usec = cores[c].profiler_freq;
  }
  pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void SpawnCore(void (*fp)(), void *gs, int64_t core) {
  struct sigaction sa;
  char buf[144];
  CHashTable *parent_table = NULL;
  if (Fs)
    parent_table = Fs->hash_table;
  CorePair pair = {fp, gs, core, NULL, parent_table},
           *ptr = malloc(sizeof(CorePair));
  *ptr = pair;
  pthread_create(&cores[core].pt, NULL, (void *)&threadrt, ptr);
  char nambuf[16];
  snprintf(nambuf, sizeof nambuf, "Seth(Core%" PRIu64 ")", core);
#  if defined(__APPLE__)
  pthread_setname_np(nambuf);
#  else
  pthread_setname_np(cores[core].pt, nambuf);
#  endif
#  if defined(__APPLE__)
  pthread_cond_init(&cores[core].wake_cond, NULL);
#  endif
  signal(SIGUSR1, InteruptRt);
  signal(SIGUSR2, ExitCoreRt);
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
  sa.sa_sigaction = (void *)&ProfRt;
  sigaction(SIGPROF, &sa, NULL);
}
int64_t mp_cnt() {
  static int64_t ret = 0;
  if (!ret)
    ret = sysconf(_SC_NPROCESSORS_ONLN);
  return ret;
}

#  include <errno.h>
void MPSleepHP(int64_t ns) {
  struct timespec ts = {0};
  ts.tv_nsec = (ns % 1000000) * 1000U;
  ts.tv_sec = ns / 1000000;
  Misc_LBts(&cores[core_num].wake_futex, 0);
#  if defined(__linux__)
  syscall(SYS_futex, &cores[core_num].wake_futex, FUTEX_WAIT, 1, &ts, NULL, 0);
#  endif
#  if defined(__FreeBSD__)
  _umtx_op(&cores[core_num].wake_futex, UMTX_OP_WAIT, 1, NULL, &ts);
#  endif
#  if defined(__APPLE__)
  static typeof(__ulock_wait) *ulWait = 0;
  static int init = 0;
  if (!init) {
    init = 1;
    ulWait = dlsym(RTLD_DEFAULT, "__ulock_wait");
  }
  if (ulWait != NULL) {
    (*ulWait)(UL_COMPARE_AND_WAIT_SHARED, &cores[core_num].wake_futex, 1, ns);
  } else {
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += (ns % 1000000) * 1000U;
    ts.tv_sec += ts.tv_nsec / 1000000000U;
    ts.tv_nsec %= 1000000000U;
    ts.tv_sec += ns / 1000000;
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mtx);
    pthread_cond_timedwait(&cores[core_num].wake_cond, &mtx, &ts);
  }
#  endif
  Misc_LBtr(&cores[core_num].wake_futex, 0);
}

void MPAwake(int64_t core) {
  if (Misc_Bt(&cores[core].wake_futex, 0)) {
#  if defined(__linux__)
    syscall(SYS_futex, &cores[core].wake_futex, 1, FUTEX_WAKE, NULL, NULL, 0);
#  elif defined(__FreeBSD__)
    _umtx_op(&cores[core].wake_futex, UMTX_OP_WAKE, 1, NULL, NULL);
#  endif
#  if defined(__APPLE__)
    static typeof(__ulock_wake) *ulWake = 0;
    static int init = 0;
    if (!init) {
      init = 1;
      ulWake = dlsym(RTLD_DEFAULT, "__ulock_wake");
    }
    if (ulWake) {
      ulWake(UL_COMPARE_AND_WAIT_SHARED | ULF_WAKE_ALL, &cores[core].wake_futex,
             1);
    } else {
      pthread_cond_signal(&cores[core].wake_cond);
    }
#  endif
  }
}
void __ShutdownCore(int core) {
  pthread_kill(cores[core].pt, SIGUSR2);
  pthread_join(cores[core].pt, NULL);
}

// Freq in microseconds
void MPSetProfilerInt(void *fp, int c, int64_t f) {
  if (!fp) {
    struct itimerval none;
    none.it_value.tv_sec = 0;
    none.it_value.tv_usec = 0;
    setitimer(ITIMER_PROF, &none, NULL);
  } else {
    cores[c].profiler_int = fp;
    cores[c].profiler_freq = f;
    cores[c].profile_timer.it_value.tv_sec = 0;
    cores[c].profile_timer.it_value.tv_usec = f;
    cores[c].profile_timer.it_interval.tv_sec = 0;
    cores[c].profile_timer.it_interval.tv_usec = f;
    setitimer(ITIMER_PROF, &cores[c].profile_timer, NULL);
  }
}
#else
extern void NtDelayExecution(BOOLEAN, LARGE_INTEGER *),
    NtSetTimerResolution(ULONG, BOOLEAN, PULONG);
__attribute__((constructor)) static void init(void) {
  NtSetTimerResolution(
      GetProcAddress(GetModuleHandle("ntdll.dll"), "wine_get_version") ? 10000
                                                                       : 5000,
      1, &(ULONG){0});
}
static CCPU cores[128];
CHashTable *glbl_table;
static int64_t ticks = 0;
static int64_t inc = 1;
static int64_t pf_prof_active;
static MMRESULT pf_prof_timer;

int64_t GetTicksHP() {
  static int64_t init;
  static LARGE_INTEGER freq, start;
  if (!init) {
    init = 1;
    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof tc);
    inc = tc.wPeriodMin;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
  }
  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);
  return (t.QuadPart - start.QuadPart) * 1e6 / freq.QuadPart;
}
void SpawnCore(void (*fp)(), void *gs, int64_t core) {
  CHashTable *parent_table = 0;
  if (Fs)
    parent_table = Fs->hash_table;
  CorePair pair = {fp, gs, core, NULL, parent_table},
           *ptr = malloc(sizeof(CorePair));
  *ptr = pair;
  cores[core].thread = CreateThread(0, 0, threadrt, ptr, 0, 0);
  cores[core].alt_stack = VirtualAlloc(
      0, 65536, MEM_TOP_DOWN | MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  SetThreadPriority(cores[core].thread, THREAD_PRIORITY_HIGHEST);
  nproc++;
}
void MPSleepHP(int64_t us) {
  Misc_LBts(&cores[core_num].sleep, 0);
  NtDelayExecution(1, &(LARGE_INTEGER){.QuadPart = -us * 10});
  Misc_LBtr(&cores[core_num].sleep, 0);
}
// Dont make this static,used for miscWIN.s
void ForceYield0() {
  CHashExport *y = HashFind("Yield", glbl_table, HTT_EXPORT_SYS_SYM, 1);
  FFI_CALL_TOS_0(y->val);
}

void InteruptCore(int64_t core) {
  CONTEXT ctx = {.ContextFlags = CONTEXT_FULL};
  SuspendThread(cores[core].thread);
  GetThreadContext(cores[core].thread, &ctx);
  *(uint64_t *)(ctx.Rsp -= 8) = ctx.Rip;
  ctx.Rip = &Misc_ForceYield; // THIS ONE SAVES ALL THE REGISTERS
  SetThreadContext(cores[core].thread, &ctx);
  ResumeThread(cores[core].thread);
}

int64_t mp_cnt() {
  static bool b;
  static SYSTEM_INFO info;
  if (!b)
    goto _init;
ret:
  return info.dwNumberOfProcessors;
_init:
  GetSystemInfo(&info);
  b = true;
  goto ret;
}

static void nopf(uint64_t ul) {
}
void MPAwake(int64_t c) {
  if (!Misc_LBtr(&cores[core_num].sleep, 0))
    return;
  QueueUserAPC(nopf, cores[core_num].thread, 0);
}
void __ShutdownCore(int core) {
  TerminateThread(cores[core].thread, 0);
}

static void WinProfTramp() {
  CCPU *c = cores + core_num;
  FFI_CALL_TOS_CUSTOM_BP(c->ctx.Rbp, c->profiler_int, c->ctx.Rip);
  RtlRestoreContext(&c->ctx, 0);
  __builtin_trap();
}

typedef unsigned _4b;
typedef uint64_t _8b;
static void WinProf(_4b _0, _4b _1, _8b _2, _8b _3, _8b _4) {
  (void)_0, (void)_1, (void)_2, (void)_3, (void)_4;
  for (int i = 0; i < nproc; ++i) {
    if (!Misc_Bt(&pf_prof_active, i))
      continue;
    CCPU *c = cores + i;
    if (ticks < c->next_prof_int || !c->profiler_int)
      continue;
    CONTEXT ctx = {.ContextFlags = CONTEXT_FULL};
    SuspendThread(c->thread);
    GetThreadContext(c->thread, &ctx);
    if (ctx.Rip > INT32_MAX)
      goto a;
    c->ctx = ctx;
    ctx.Rsp = (uint64_t)c->alt_stack + 65536;
    *(uint64_t *)(ctx.Rsp -= 8) = ctx.Rip;
    ctx.Rip = WinProfTramp;
    SetThreadContext(c->thread, &ctx);
  a:
    ResumeThread(c->thread);
    c->next_prof_int = c->profiler_freq + ticks;
  }
}

// here's the thing about using winmm:
// amongst the win32 APIs, the highest
// precision timer's a MSDOS subsystem
// from 1989 written in 16bit assembly
// for lowend boxes that need accurate
// timer precision for multimedia play
//
// microsoft now wants to deprecate it
// for the sake of it; winmm is barely
// high precision in modern times, but
// its ms precisions still the highest
// in windows and there arent any alts
// to it, NtDelayExecution() loops get
// close but it's dumb compared to the
// stuff POSIX has: sigaction(SIGPROF)
//
// thank god ms never removes anything

void MPSetProfilerInt(void *fp, int c, int64_t f) {
  static MMRESULT pf_prof_timer;
  CCPU *core = cores + c;
  if (fp) {
    core->profiler_freq = f / 1e3;
    core->profiler_int = fp;
    core->next_prof_int = 0;
    if (!pf_prof_active && !pf_prof_timer)
      pf_prof_timer = timeSetEvent(inc, inc, WinProf, 0, TIME_PERIODIC);
    Misc_LBts(&pf_prof_active, c);
  } else {
    Misc_LBtr(&pf_prof_active, c);
    if (!pf_prof_active && pf_prof_timer) {
      timeKillEvent(pf_prof_timer);
      pf_prof_timer = 0;
    }
  }
}
#endif
