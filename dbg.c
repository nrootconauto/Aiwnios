#include "aiwn.h"
#include <signal.h>
// Look at your vendor's ucontext.h
#define __USE_GNU
#define DBG_MSG_RESUME   "%ld:%d,RESUME,%d\n"     //(task,pid,single_step)
#define DBG_MSG_START    "%ld:%d,START\n"         //(task,pid)
#define DBG_MSG_SET_GREG "%ld:%d,SGREG,%ld,%ld\n" //(task,pid,which_reg,value)
#define DBG_MSG_WATCH_TID                                                      \
  "0:0,WATCHTID,%d\n" //(tid,aiwnios is a Godsend,it will send you a message for
                      // the important TIDs(cores))
#define DBG_MSG_OK "OK"
#if defined(__linux__) || defined(__FreeBSD__)
  #include <poll.h>
  #include <sys/ptrace.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <ucontext.h>
  #include <unistd.h>
  #if defined(__FreeBSD__)
    #include <machine/reg.h>
  #endif
  #if defined(__linux__)
    #include <linux/elf.h> //Why do I need this for NT_PRSTATUS
    #include <sys/uio.h>
    #include <sys/user.h>
  #endif
#else
  #include <windows.h>
  #include <errhandlingapi.h>
#endif
static int debugger_pipe[2];
#if defined(__FreeBSD__)
typedef struct CFuckup {
  struct CQue  base;
  int64_t      signal;
  pid_t        pid;
  void        *task;
  struct reg   regs;
  struct fpreg fp;
} CFuckup;
  #define PTRACE_TRACEME   PT_TRACE_ME
  #define PTRACE_ATTACH    PT_ATTACH
  #define PTRACE_SETREGS   PT_SETREGS
  #define PTRACE_GETREGS   PT_GETREGS
  #define PTRACE_SETFPREGS PT_SETFPREGS
  #define PTRACE_GETFPREGS PT_GETFPREGS
  #define gettid           getpid
#endif
#if defined(__linux__)
  #include <asm/ptrace.h>
  #include <sys/user.h>
typedef struct CFuckup {
  struct CQue base;
  int64_t     signal;
  pid_t       pid;
  void       *task;
  #if defined(_M_ARM64) || defined(__aarch64__)
  struct user_pt_regs       regs;
  struct user_fpsimd_struct fp;
  #elif defined(__x86_64__)
  struct user_fpregs_struct fp;
  struct user_regs_struct   regs;
  #endif
} CFuckup;

#endif
CFuckup *GetFuckupByTask(CQue *head, void *t) {
  CFuckup *fu;
  for (fu = head->next; fu != head; fu = fu->base.next) {
    if (fu->task == t)
      return fu;
  }
  return NULL;
}
CFuckup *GetFuckupByPid(CQue *head, pid_t pid) {
  CFuckup *fu;
  for (fu = head->next; fu != head; fu = fu->base.next) {
    if (fu->pid == pid)
      return fu;
  }
  return NULL;
}

void DebuggerClientWatchThisTID() {
  int64_t tid = gettid();
  char    buf[256];
  sprintf(buf, DBG_MSG_WATCH_TID, tid);
  write(debugger_pipe[1], buf, 256);
}
static int64_t DebuggerWait(CQue *head, pid_t *got) {
  int      code, sig;
  pid_t    pid = waitpid(-1, &code, WNOHANG | WUNTRACED | WCONTINUED);
  CFuckup *fu;
  if (pid <= 0)
    return 0;
  if (WIFEXITED(code)) {
    close(debugger_pipe[0]);
    close(debugger_pipe[1]);
    ptrace(PT_DETACH, pid, 0, 0);
    exit(WEXITSTATUS(code));
    return 0;
  } else if (WIFSIGNALED(code) || WIFSTOPPED(code) || WIFCONTINUED(code)) {
    if (WIFSIGNALED(code))
      sig = WTERMSIG(code);
    else if (WIFSTOPPED(code))
      sig = WSTOPSIG(code);
    else if (WIFCONTINUED(code))
      sig = SIGCONT;
    else
      abort();
    if (fu = GetFuckupByPid(head, pid))
      fu->signal = sig;
    if (got)
      *got = pid;
    return sig;
  }
}
void DebuggerBegin() {
  pipe(debugger_pipe);
  int   code, sig;
  void *task;
  char  buf[4048], *ptr;
  pid_t tid   = fork();
  pid_t child = tid;
  if (!tid) {
    ptrace(PTRACE_TRACEME, tid, NULL, NULL);
    strcpy(buf, DBG_MSG_OK);
    write(debugger_pipe[1], buf, 256);
    return;
  } else {
    ptrace(PT_ATTACH, tid, NULL, NULL);
    wait(NULL);
    ptrace(PT_CONTINUE, tid, 1, NULL);
    while (read(debugger_pipe[0], buf, 256) > 0) {
      if (strcmp(buf, DBG_MSG_OK))
        exit(1);
      else
        break;
    }

#if defined(__linux__)
    struct iovec poop;
#endif
    struct pollfd poll_for;
    CQue          fuckups;
    CFuckup      *fu, *last;
    char         *ptr, *name;
    int64_t       which, value, sig;
    QueInit(&fuckups);
    while (1) {
      memset(&poll_for, 0, sizeof(struct pollfd));
      poll_for.fd     = debugger_pipe[0];
      poll_for.events = POLL_IN;
      if (poll(&poll_for, 1, 2)) {
        read(debugger_pipe[0], buf, 256);
        ptr  = buf;
        task = NULL;
        while (1) {
          if (*ptr == ':') {
            *ptr = 0;
            task = strtoul(buf, NULL, 10);
            ptr++;
            tid  = strtoul(ptr, &ptr, 10);
            name = ptr + 1;
          }
          if (*ptr == 0) {
            if (!strncmp(name, "SGREG", strlen("SGREG"))) {
				while (DebuggerWait(&fuckups, &tid)) {
                if (fu = GetFuckupByPid(&fuckups, tid)) {
                  if (fu->signal == SIGCONT) {
                    fu->signal = 0;
                    break;
                  }
                }
              }
              ptr = name + strlen("SGREG,");
              if (strtok(ptr, ",")) {
                which = strtoul(ptr, &ptr, 10);
                ptr++; // Skip ','
                value = strtoul(ptr, &ptr, 10);
                // SEE swapctxX86.s
#if defined(__FreeBSD__) && defined(__x86_64__)
                switch (which) {
                case 0:
                  fu->regs.r_rip = value;
                  break;
                case 1:
                  fu->regs.r_rsp = value;
                  break;
                case 2:
                  fu->regs.r_rbp = value;
                  break;
                  // DONT RELY ON CHANGING GPs
                }
#endif
#if defined(__linux__) && defined(__x86_64__)
                switch (which) {
                case 0:
                  fu->regs.rip = value;
                  break;
                case 1:
                  fu->regs.rsp = value;
                  break;
                case 2:
                  fu->regs.rbp = value;
                  break;
                  // DONT RELY ON CHANGING GPs
                }
#endif
              }
              ptrace(PT_CONTINUE, tid, 1, 0);
              break;
            } else if (!strncmp(name, "WATCHTID", strlen("WATCHTID"))) {
              ptr = name + strlen("WATCHTID");
              if (*ptr != ',')
                abort();
              ptr++;
              ptrace(PTRACE_ATTACH, strtoul(ptr, NULL, 10), NULL, NULL);
              break;
            } else if (!strncmp(name, "START", strlen("START"))) {
              while (DebuggerWait(&fuckups, &tid)) {
                if (fu = GetFuckupByPid(&fuckups, tid)) {
                  if (fu->signal == SIGCONT) {
                    fu->signal = 0;
                    fu->task   = task;
                    break;
                  }
                }
              }
              ptrace(PT_CONTINUE, tid, 1, 0);
              break;
            } else if (!strncmp(name, "RESUME", strlen("RESUME"))) {
              while (DebuggerWait(&fuckups, &tid)) {
                if (fu = GetFuckupByPid(&fuckups, tid)) {
                  if (fu->signal == SIGCONT) {
                    fu->signal = 0;
                    break;
                  }
                }
              }
              fu = GetFuckupByTask(&fuckups, task);
              if (!fu) {
                ptrace(PT_CONTINUE, tid, 1, 0);
                break;
              }
#if defined(__x86_64__)
              // IF you are blessed you are running on a platform that has these
              ptrace(PTRACE_SETREGS, tid, 0, &fu->regs);
              ptrace(PTRACE_SETFPREGS, tid, 0, &fu->fp);
#else
              poop.iov_len  = sizeof(fu->regs);
              poop.iov_base = &fu->regs;
              ptrace(PTRACE_SETREGSET, tid, NT_PRSTATUS, &poop);
              poop.iov_len  = sizeof(fu->fp);
              poop.iov_base = &fu->fp;
              ptrace(PTRACE_SETREGSET, tid, NT_PRFPREG, &poop);
#endif
              ptr = name + strlen("RESUME");
              if (*ptr == ',') {
                if (strtoul(ptr + 1, NULL, 10))
                #if defined (__FreeBSD__)
                  ptrace(PT_STEP, tid, 1, SIGTRAP);
                #else
                  ptrace(PTRACE_SINGLESTEP, tid, 0, 0);
                #endif
                else {
#if defined(__x86_64__)
                  ptrace(PT_CONTINUE, tid, 1, 0);
#else
                  ptrace(PT_CONTINUE, tid, 0, 0);
#endif
                  QueRem(fu);
                  free(fu);
                }
              } else {
                ptrace(PT_CONTINUE, tid, 1, 0);
                QueRem(fu);
                free(fu);
              }
            } else
              abort();
            break;
          }
          ptr++;
        }
      } else if (sig = DebuggerWait(&fuckups, &tid)) {
        switch (sig) {
        case SIGFPE:
        case SIGBUS:
        case SIGSEGV:
        case SIGTRAP:
          fu = calloc(sizeof(CFuckup), 1);
          QueIns(fu, &fuckups);
          fu->task = NULL;
          fu->pid  = tid;
#if defined(__x86_64__)
          // IF you are blessed you are running on a platform that has these
          ptrace(PTRACE_GETREGS, tid, 0, &fu->regs);
          ptrace(PTRACE_GETFPREGS, tid, 0, &fu->fp);
#elif defined(PTRACE_GETREGSET)
          poop.iov_len = sizeof(fu->regs);
          poop.iov_base = &fu->regs;
          ptrace(PTRACE_GETREGSET, tid, NT_PRSTATUS, &poop);
          poop.iov_len = sizeof(fu->fp);
          poop.iov_base = &fu->fp;
          ptrace(PTRACE_GETREGSET, tid, NT_PRFPREG, &poop);
#endif
        default:
          if (sig == SIGSTOP) // This is used for ptrace events
            ptrace(PT_CONTINUE, tid, 1, 0);
          else
            ptrace(PT_CONTINUE, tid, 1, sig);
        }
      }
    }
  }
}
static void UnblockSignals() {
#if defined(__linux__) || defined(__FreeBSD__)
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGILL);
  sigaddset(&set, SIGSEGV);
  sigaddset(&set, SIGBUS);
  sigaddset(&set, SIGTRAP);
  sigaddset(&set, SIGFPE);
  sigprocmask(SIG_UNBLOCK, &set, NULL);
#endif
}
#if defined(__linux__) || defined(__FreeBSD__)
static void SigHandler(int64_t sig, siginfo_t *info, ucontext_t *_ctx) {
  #if defined(__x86_64__)
    #if defined(__linux__)
  // See /usr/include/x86_64-linux-gnu/sys/ucontext.h
  enum {
    REG_R8 = 0,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,
    REG_R14,
    REG_R15,
    REG_RDI,
    REG_RSI,
    REG_RBP,
    REG_RBX,
    REG_RDX,
    REG_RAX,
    REG_RCX,
    REG_RSP,
    REG_RIP,
  };
  UnblockSignals();
  mcontext_t *ctx = &_ctx->uc_mcontext;
  int64_t     actx[32];
  actx[0] = ctx->gregs[REG_RIP];
  actx[1] = ctx->gregs[REG_RSP];
  actx[2] = ctx->gregs[REG_RBP];
  actx[3] = ctx->gregs[REG_R11];
  actx[4] = ctx->gregs[REG_R12];
  actx[5] = ctx->gregs[REG_R13];
  actx[6] = ctx->gregs[REG_R14];
  actx[7] = ctx->gregs[REG_R15];
  CHashExport *exp;
  if (exp = HashFind("AiwniosDbgCB", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
    FFI_CALL_TOS_2(exp->val, sig, actx);
  } else if (exp = HashFind("Exit", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
    ctx->gregs[0] = exp->val;
  } else
    abort();
  setcontext(_ctx);
    #elif defined(__FreeBSD__)
  UnblockSignals();
  mcontext_t *ctx = &_ctx->uc_mcontext;
  int64_t     actx[32];
  actx[0] = ctx->mc_rip;
  actx[1] = ctx->mc_rsp;
  actx[2] = ctx->mc_rbp;
  actx[3] = ctx->mc_r11;
  actx[4] = ctx->mc_r12;
  actx[5] = ctx->mc_r13;
  actx[6] = ctx->mc_r14;
  actx[7] = ctx->mc_r15;
  CHashExport *exp;
  if (exp = HashFind("AiwniosDbgCB", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
    FFI_CALL_TOS_2(exp->val, sig, actx);
  } else if (exp = HashFind("Exit", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
    ctx->mc_rip = exp->val;
  } else
    abort();
  setcontext(_ctx);
    #endif
  #elif defined(__aarch64__) || defined(_M_ARM64)
  mcontext_t               *ctx = &_ctx->uc_mcontext;
  CHashExport              *exp;
  int64_t                   is_single_step;
  // See swapctxAARCH64.s
  //  I have a secret,im only filling in saved registers as they are used
  //  for vairables in Aiwnios. I dont have plans on adding tmp registers
  //  in here anytime soon
  int64_t (*fp)(int64_t sig, int64_t * ctx), (*fp2)();
  int64_t actx[(30 - 18 + 1) + (15 - 8 + 1) + 1];
  int64_t i, i2, sz, fp_idx;
  UnblockSignals();
  for (i = 18; i <= 30; i++)
    if (i - 18 != 12) // the 12th one is the return register
    #if defined(__linux__)
      actx[i - 18] = ctx->regs[i];
    #elif defined(__FreeBSD__)
      actx[i - 18] = ctx->mc_gpregs.gp_x[i];
    #endif
    #if defined(__linux__)
  // We will look for FPSIMD_MAGIC(0x46508001)
  // From
  // https://github.com/torvalds/linux/blob/master/arch/arm64/include/uapi/asm/sigcontext.h
  for (i = 0; i < 4096;) {
    sz = ((uint32_t *)(&ctx->__reserved[i]))[1];
    if (((uint32_t *)(&ctx->__reserved[i]))[0] == 0x46508001) {
      i += 4 + 4 + 4 + 4;
      fp_idx = i;
      for (i2 = 8; i2 <= 15; i2++)
        actx[(30 - 18 + 1) + i2 - 8] =
            *(int64_t *)(&ctx->__reserved[fp_idx + 16 * i2]);
      break;
    } else if (!sz)
      break;
    else
      i += sz;
  }
    #elif defined(__FreeBSD__)
  for (i2 = 8; i2 <= 15; i2++)
    actx[(30 - 18 + 1) + i2 - 8] = *(int64_t *)(&ctx->mc_fpregs.fp_q[16 * i2]);
    #endif
    #if defined(__linux__)
  actx[21] = ctx->sp;
    #elif defined(__FreeBSD__)
  actx[21] = ctx->mc_gpregs.gp_sp;
    #endif
  // AiwniosDbgCB will return 1 for singlestep
  if (exp = HashFind("AiwniosDbgCB", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
    fp = exp->val;
    if (FFI_CALL_TOS_2(fp, sig, actx)) { // Returns 1 for single step
    #if defined(__x86_64__)
      #if defined(__FreeBSD__)
      ctx->mc_rip = actx[0];
      ctx->mc_rsp = actx[1];
      ctx->mc_rbp = actx[2];
      ctx->mc_r11 = actx[3];
      ctx->mc_r12 = actx[4];
      ctx->mc_r13 = actx[5];
      ctx->mc_r14 = actx[6];
      ctx->mc_r15 = actx[7];
      ctx->mc_eflags |= 1 << 8;
      #elif defined(__linux__)
      ctx->gregs[REG_RIP] = actx[0];
      ctx->gregs[REG_RSP] = actx[1];
      ctx->gregs[REG_RBP] = actx[2];
      ctx->gregs[REG_R11] = actx[3];
      ctx->gregs[REG_R12] = actx[4];
      ctx->gregs[REG_R13] = actx[5];
      ctx->gregs[REG_R14] = actx[6];
      ctx->gregs[REG_R15] = actx[7];
      #endif
    #endif
    }
  } else if (exp = HashFind("Exit", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
  call_exit:
    fp2 = exp->val;
    FFI_CALL_TOS_0(fp2);
  } else
    abort();
  #endif
}
#endif
#if defined(WIN32) || defined(_WIN32)
static void VectorHandler(struct _EXCEPTION_POINTERS *einfo) {
  CONTEXT *ctx = einfo->ContextRecord;
  int64_t  actx[23];
  actx[0]  = ctx->Rip;
  actx[1]  = ctx->Rsp;
  actx[2]  = ctx->Rbp;
  actx[3]  = ctx->R11;
  actx[4]  = ctx->R12;
  actx[5]  = ctx->R13;
  actx[6]  = ctx->R14;
  actx[7]  = ctx->R15;
  actx[20] = ctx->R10;
  actx[21] = ctx->Rdi;
  actx[22] = ctx->Rsi;
  CHashExport *exp;
  if (exp = HashFind("AiwniosDbgCB", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
    FFI_CALL_TOS_2(exp->val, 0, actx);
  } else if (exp = HashFind("Exit", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1))
    FFI_CALL_TOS_0(exp->val);
  else
    abort();
  ctx->Rsp = actx[1];
  ctx->Rbp = actx[2];
  ctx->R11 = actx[3];
  ctx->R12 = actx[4];
  ctx->R13 = actx[5];
  ctx->R14 = actx[6];
  ctx->R15 = actx[7];
  ctx->R10 = actx[20];
  ctx->Rdi = actx[21];
  ctx->Rsi = actx[22];
  ctx->Rip = actx[0];
  return EXCEPTION_CONTINUE_EXECUTION;
}
#endif
void InstallDbgSignalsForThread() {
#if defined(__linux__) || defined(__FreeBSD__)
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler   = SIG_IGN;
  sa.sa_flags     = SA_SIGINFO;
  sa.sa_sigaction = (void *)&SigHandler;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
  sigaction(SIGTRAP, &sa, NULL);
  sigaction(SIGILL, &sa, NULL);
  sigaction(SIGFPE, &sa, NULL);
#else
  AddVectoredExceptionHandler(1, VectorHandler);
#endif
}
// This happens when after we call DebuggerClientEnd
void DebuggerClientSetGreg(void *task, int64_t which, int64_t v) {
  char buf[256];
  sprintf(buf, DBG_MSG_SET_GREG, task, gettid(), which, v);
  write(debugger_pipe[1], buf, 256);
  raise(SIGCONT);
}
void DebuggerClientStart(void *task) {
  char buf[256], *ptr;
  sprintf(buf, DBG_MSG_START, task, gettid());
  write(debugger_pipe[1], buf, 256);
  raise(SIGCONT);
}
void DebuggerClientEnd(void *task, int64_t wants_singlestep) {
  char buf[256];
  sprintf(buf, DBG_MSG_RESUME, task, gettid(), wants_singlestep);
  write(debugger_pipe[1], buf, 256);
  raise(SIGCONT);
}
