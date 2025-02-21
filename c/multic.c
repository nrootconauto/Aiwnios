#include "c/aiwn_asm.h"
#include "c/aiwn_except.h"
#include "c/aiwn_fs.h"
#include "c/aiwn_hash.h"
#include "c/aiwn_mem.h"
#include "c/aiwn_multic.h"
#include <SDL.h>
#include <inttypes.h>
#include <signal.h>
#include <stdlib.h>

#ifdef __x86_64__
#  include <errno.h>
#  include <string.h>
#  ifndef _WIN32
#    include <setjmp.h>
#    include <sys/syscall.h>
#    ifndef __OpenBSD__
#      include <ucontext.h>
#    else
#      define OPENBSD_POOP_TLS "Living in 1998"
#    endif
#    ifdef __FreeBSD__
#      include <machine/sysarch.h>
#    endif
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
#  ifdef __OpenBSD__
#    include <tib.h>
#    define FS_OFF   0xF0
#    define GS_OFF   0xF8
#    define ThreadFs (*(void *__seg_fs *)FS_OFF)
#    define ThreadGs (*(void *__seg_fs *)GS_OFF)
#  else
#    define FS_OFF   0xF0
#    define GS_OFF   0xF8
#    define ThreadFs (*(void *__seg_gs *)FS_OFF)
#    define ThreadGs (*(void *__seg_gs *)GS_OFF)
#  endif
#elif defined(__aarch64__)
static _Thread_local void *__fsgs[2];
#  define FS_OFF   0
#  define GS_OFF   8
#  define ThreadFs __fsgs[0]
#  define ThreadGs __fsgs[1]
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

#ifdef __aarch64__
void __bootstrap_tls(void) {
  asm("mov\tx28,%0" : : "r"(__fsgs));
}
#elif defined(__x86_64__) && !defined(_WIN64)
static int __setgs(void *gs);
static jmp_buf jmpb;

static void __sigillhndlr(int sig) {
  (void)sig;
  longjmp(jmpb, 1);
}

void __bootstrap_tls(void) {
#  if defined(OPENBSD_POOP_TLS) && defined(__OpenBSD__) 
  struct tib *old = TCB_TO_TIB(__get_tcb());
  struct tib *new =
      _dl_allocate_tib(TIB_EXTRA_ALIGN + 0x100); // Fs x Gs pointers end at 0xf8
  memcpy(new, old, sizeof(struct tib) + TIB_EXTRA_ALIGN);
  new->__tib_self = (void *)new;
  __set_tcb(TIB_TO_TCB(new));
#  else
  static bool init, fsgsbase;
  if (!init) {
    int ebx;
    asm("cpuid"          // Structured Extended Feature Flags Enumeration Leaf
        : "=b"(ebx)      //         (Initial EAX Value = 07H, ECX = 0)
        : "a"(7), "c"(0) //       EBX - Bit 00: FSGSBASE. Supports if 1.
        : "edx");
    fsgsbase = ebx & 1;
    init = true;
  }
  void *tls = calloc(1, 0x10) - 0xF0;
  if (fsgsbase) {
    struct sigaction sa = {.sa_handler = __sigillhndlr};
    sigaction(SIGILL, &sa, 0);
    if (!setjmp(jmpb))
      asm("wrgsbase\t%0" : : "r"(tls));
    else
      fsgsbase = false;
    sa.sa_handler = SIG_DFL;
    sigaction(SIGILL, &sa, 0);
    if (!fsgsbase)
      goto sysc;
  } else {
  sysc:
    if (!__setgs(tls)) // both syscall and sysarch return 0 on success
      return;
    fprintf(stderr, "\e[0;31mCRITICAL\e[0m: Unable to set %%gs: %s\n",
            strerror(errno));
    fflush(stderr);
    // do not ever change this to normal exit()
    // this is a harsh exit that ignores atexit
    _Exit(-1);
  }
#  endif
}

static int __setgs(void *gs) {
  int ret = -1;

#  ifdef __linux__
  asm("syscall"   //    (not defined in musl)
      : "=a"(ret) //         ARCH_SET_GS
      : "0"(SYS_arch_prctl), "D"(0x1001), "S"(gs)
      : "rcx", "r11", "memory");
#  elif defined(__FreeBSD__)
  ret = amd64_set_gsbase(gs);
#  endif
  return ret;
}
#else
void __bootstrap_tls(void) {
}
#endif

typedef struct {
  void *fp, *gs;
  int64_t num;
  void *profiler_int;
  CHashTable *parent_table;
  char name[16];
} CorePair;
#ifdef __linux__
#  include <linux/futex.h>
#elif defined(__FreeBSD__)
#  include <sys/types.h>
#  include <sys/umtx.h>
#elif defined(__OpenBSD__)
#  include <sys/futex.h>
#elif defined(__APPLE__)
#  define PRIVATE 1
#  include "c/ulock.h" //Not canoical
#  undef PRIVATE
#endif

#ifndef _WIN64
#  include <pthread.h>
#  include <sys/mman.h>
#  include <sys/syscall.h>
#  include <sys/time.h>
#  include <time.h>
#  include <unistd.h>
typedef struct {
  pthread_t pt;
  int wake_futex;
  void (*profiler_int)(void *fs);
  int64_t profiler_freq;
  struct itimerval profile_timer;
#  if defined(OPENBSD_POOP_TLS) && defined(__OpenBSD__)
  struct tib *otib;
  pid_t tid;
#  endif
#  ifdef __APPLE__
  pthread_cond_t wake_cond;
#  endif
} CCPU;

static void ProfRt(int sig, siginfo_t *info, void *__ctx);
static void InteruptRt(int sig, siginfo_t *info, void *__ctx);
static void Div0Rt(int sig);
static void ExitCoreRt(int sig);
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
  HANDLE thread, tid;
  CONTEXT ctx;
  int64_t sleep;
  void *profiler_int;
  int64_t profiler_freq, next_prof_int;
  uint8_t *alt_stack;
} CCPU;
static int64_t nproc;
#endif
static _Thread_local core_num = -1;
static CCPU cores[128];
CHashTable *glbl_table;

#ifdef __APPLE__
#  define pthread_setname_np(a, b) pthread_setname_np(b)
#endif
static void *threadrt(void *_pair) {
  CorePair *pair = _pair;
  core_num = pair->num;
  __bootstrap_tls();
#ifndef _WIN64
#  ifndef __OpenBSD__
  pthread_setname_np(pthread_self(), pair->name);
#  elif defined(OPENBSD_POOP_TLS) &&defined(__OpenBSD__)
  cores[core_num].otib = TCB_TO_TIB(__get_tcb());
  cores[core_num].tid = TCB_TO_TIB(__get_tcb())->tib_tid;
#  endif
  stack_t stk = {
#  ifndef __OpenBSD__
      .ss_sp = malloc(SIGSTKSZ),
#  else
      .ss_sp = mmap(0, SIGSTKSZ, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0),
#  endif
      .ss_size = SIGSTKSZ,
  };
  sigaltstack(&stk, 0);
  static struct Sig {
    int sig;
    struct sigaction sa;
  } sigs[] = {
      {SIGFPE, {.sa_handler = Div0Rt}},
      {SIGUSR1, {.sa_sigaction = InteruptRt, .sa_flags = SA_SIGINFO}},
      {SIGUSR2, {.sa_handler = ExitCoreRt}},
      {SIGPROF, {.sa_sigaction = ProfRt, .sa_flags = SA_ONSTACK | SA_SIGINFO}},
      {-1},
  };
  for (struct Sig *sg = sigs; sg->sig != -1; sg++)
    sigaction(sg->sig, &sg->sa, 0);
#endif
  InstallDbgSignalsForThread();
  DebuggerClientWatchThisTID();
  Fs = calloc(sizeof *Fs, 1);
  VFsThrdInit();
  TaskInit(Fs, NULL, 0);
  Fs->hash_table->next = pair->parent_table;
  SetHolyGs(pair->gs);
  void (*fp)();
  fp = pair->fp;
  free(pair);
  fp();
}
#ifndef _WIN32

#  ifdef __OpenBSD__
static _Atomic(pid_t) which_interupt;
#  endif

void InteruptCore(int64_t core) {
#  ifndef defined(OPENBSD_POOP_TLS)
  pthread_kill(cores[core].pt, SIGUSR1);
#  else
  // we changed the address of the tib so we'll have to pass it ourselves
  CCPU *c = cores + core;
  which_interupt = c->tid;
  thrkill(c->otib->tib_tid, SIGUSR1, c->otib);
#  endif
}
static void InteruptRt(int sig, siginfo_t *info, void *__ctx) {
  (void)sig, (void)info;
  ucontext_t *_ctx = __ctx;
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  pthread_sigmask(SIG_UNBLOCK, &set, NULL);
#  ifndef __OpenBSD__
  mcontext_t *ctx = &_ctx->uc_mcontext;
#  elif defined(OPENBSD_POOP_TLS) && defined(__OpenBSD__)
  if (TCB_TO_TIB(__get_tcb())->tib_tid != which_interupt) {
    fprintf(stderr, "Report to nroot, OpenBSD is acting poopy\n");
    abort();
  }
#  endif
  CHashExport *y = HashFind("InteruptRt", glbl_table, HTT_EXPORT_SYS_SYM, 1);
  void (*fp)();
  if (y) {
    fp = y->val;
#  if defined(__OpenBSD__) && defined(__x86_64__)
    FFI_CALL_TOS_2(fp, _ctx->sc_rip, _ctx->sc_rbp);
#  endif
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
#  elif (defined(_M_ARM64) || defined(__aarch64__)) && defined(__OpenBSD__)
    FFI_CALL_TOS_2(fp, NULL, NULL);
#  endif
  }
}

static void Div0Rt(int sig) {
  (void)sig;
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGFPE);
  pthread_sigmask(SIG_UNBLOCK, &set, 0);
  throw(*(uint64_t *)"Div0\0\0\0");
}

static void ExitCoreRt(int sig) {
  (void)sig;
  pthread_exit(0);
}

static void ProfRt(int sig, siginfo_t *info, void *__ctx) {
  (void)sig, (void)info;
  ucontext_t *_ctx = __ctx;
  int64_t c = core_num;
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGPROF);
  if (!pthread_equal(pthread_self(), cores[c].pt)) {
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    return;
  }
  if (cores[c].profiler_int) {
#  ifdef __x86_64__
#    ifdef __FreeBSD__
    FFI_CALL_TOS_CUSTOM_BP(_ctx->uc_mcontext.mc_rbp, cores[c].profiler_int,
                           _ctx->uc_mcontext.mc_rip);
#    elif defined(__linux__)
    FFI_CALL_TOS_CUSTOM_BP(_ctx->uc_mcontext.gregs[REG_RBP],
                           cores[c].profiler_int,
                           _ctx->uc_mcontext.gregs[REG_RIP]);
#    endif
#  endif
#  if defined(__riscv) || defined(__riscv__)
#    ifdef __linux__
    FFI_CALL_TOS_CUSTOM_BP(_ctx->uc_mcontext.__gregs[8], cores[c].profiler_int,
                           _ctx->uc_mcontext.__gregs[1]);
#    endif
#  endif
    cores[c].profile_timer.it_value.tv_usec = cores[c].profiler_freq;
    cores[c].profile_timer.it_interval.tv_usec = cores[c].profiler_freq;
  }
  pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void SpawnCore(void *fp, void *gs, int64_t core) {
  CHashTable *parent_table = 0;
  if (Fs)
    parent_table = Fs->hash_table;
  CorePair *ptr = calloc(1, sizeof *ptr);
  ptr->fp = fp, ptr->gs = gs, ptr->num = core, ptr->parent_table = parent_table;
  CCPU *c = &cores[core];
  pthread_create(&c->pt, 0, threadrt, ptr);
  snprintf(ptr->name, sizeof ptr->name, "Seth(Core%" PRIu64 ")", core);
#  if defined(__APPLE__)
  pthread_cond_init(&c->wake_cond, NULL);
#  endif
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
#  if defined(__OpenBSD__)
  futex(&cores[core_num].wake_futex, FUTEX_WAIT, 1, &ts, NULL);
#  elif defined(__linux__)
  syscall(SYS_futex, &cores[core_num].wake_futex, FUTEX_WAIT, 1, &ts, NULL, 0);
#  endif
#  if defined(__FreeBSD__)
  _umtx_op(&cores[core_num].wake_futex, UMTX_OP_WAIT, 1, NULL, &ts);
#  endif
#  if defined(__APPLE__)
  if (__ulock_wait) {
    __ulock_wait(UL_COMPARE_AND_WAIT_SHARED, &cores[core_num].wake_futex, 1,
                 ns);
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
#  if defined(__OpenBSD__)
    futex(&cores[core_num].wake_futex, FUTEX_WAKE, 1, NULL, NULL);
#  elif defined(__linux__)
    syscall(SYS_futex, &cores[core].wake_futex, 1, FUTEX_WAKE, NULL, NULL, 0);
#  elif defined(__FreeBSD__)
    _umtx_op(&cores[core].wake_futex, UMTX_OP_WAKE, 1, NULL, NULL);
#  endif
#  if defined(__APPLE__)
    if (__ulock_wake) {
      __ulock_wake(UL_COMPARE_AND_WAIT_SHARED | ULF_WAKE_ALL,
                   &cores[core].wake_futex, 1);
    } else {
      pthread_cond_signal(&cores[core].wake_cond);
    }
#  endif
  }
}
void __ShutdownCore(int core) {
#  ifndef OPENBSD_POOP_TLS
  pthread_kill(cores[core].pt, SIGUSR2);
#  else
  CCPU *c = cores + core;
  thrkill(c->otib->tib_tid, SIGUSR2, c->otib);
#  endif
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
    NtSetTimerResolution(ULONG, BOOLEAN, PULONG),
    NtCreateKeyedEvent(HANDLE, void const *, BOOLEAN, int64_t *),
    NtWaitForKeyedEvent(HANDLE, void const *, BOOLEAN, int64_t *),
    NtReleaseKeyedEvent(HANDLE, void const *, BOOLEAN, int64_t *);
uint64_t NtAlertThreadByThreadId(HANDLE);
__attribute__((constructor)) static void init(void) {
  NtSetTimerResolution(
      GetProcAddress(GetModuleHandle("ntdll.dll"), "wine_get_version") ? 10000
                                                                       : 5000,
      1, &(ULONG){0});
}
static int64_t inc = 1;
static int64_t pf_prof_active;
static MMRESULT pf_prof_timer;

static void initinc(void) {
  static bool init;
  if (init)
    return;
  TIMECAPS tc;
  timeGetDevCaps(&tc, sizeof tc);
  inc = tc.wPeriodMin;
  init = true;
}

int64_t GetTicksHP() {
  static int64_t init;
  static LARGE_INTEGER freq, start;
  if (!init) {
    initinc();
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    init = 1;
  }
  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);
  return (t.QuadPart - start.QuadPart) * 1e6 / freq.QuadPart;
}
void SpawnCore(void *fp, void *gs, int64_t core) {
  CHashTable *parent_table = 0;
  if (Fs)
    parent_table = Fs->hash_table;
  CorePair *ptr = calloc(1, sizeof *ptr);
  ptr->fp = fp, ptr->gs = gs, ptr->num = core, ptr->parent_table = parent_table;
  cores[core].thread = CreateThread(0, 0, (void *)threadrt, ptr, 0, 0);
  cores[core].alt_stack = VirtualAlloc(
      0, 65536, MEM_TOP_DOWN | MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  SetThreadPriority(cores[core].thread, THREAD_PRIORITY_HIGHEST);
  nproc++;
}
void MPSleepHP(int64_t us) {
  Misc_LBts(&cores[core_num].sleep, 0);
  LARGE_INTEGER delay = {.QuadPart = -us * 10};
  NtDelayExecution(TRUE, &delay);
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
  static bool init;
  static int64_t n;
  if (!init) {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    n = info.dwNumberOfProcessors;
    init = true;
  }
  return n;
}

static void nopf(uint64_t i) {
  (void)i;
}

// here's the deal, this is how it works:
// NtDelayExecution pauses the thread til
// the time expires or an APC is raise.
// (if the first arg is true) so we raise
// an APC that does nothing to wake it up
// http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/NT%20Objects/Thread/NtDelayExecution.html
// https://learn.microsoft.com/en-us/windows/win32/fileio/alertable-i-o
void MPAwake(int64_t c) {
  if (!Misc_LBtr(&cores[c].sleep, 0))
    return;
  QueueUserAPC(nopf, cores[c].thread, 0);
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
  uint64_t *rip;
  for (int i = 0; i < nproc; ++i) {
    if (!Misc_Bt(&pf_prof_active, i))
      continue;
    CCPU *c = cores + i;
    if (GetTicksHP() / 1e3 < c->next_prof_int || !c->profiler_int)
      continue;
    CONTEXT ctx = {.ContextFlags = CONTEXT_FULL};
    SuspendThread(c->thread);
    GetThreadContext(c->thread, &ctx);
    if (ctx.Rip > INT32_MAX)
      goto secular;
    c->ctx = ctx;
    ctx.Rsp = (uint64_t)c->alt_stack + 65536;
    *(uint64_t *)(ctx.Rsp -= 8) = ctx.Rip;
    ctx.Rip = WinProfTramp;
    SetThreadContext(c->thread, &ctx);
  secular:
    ResumeThread(c->thread);
    c->next_prof_int = c->profiler_freq + GetTicksHP() / 1e3;
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
