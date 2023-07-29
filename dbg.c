#include "aiwn.h"
#include <signal.h>
// Look at your vendor's ucontext.h
#define __USE_GNU
#if defined(__linux__) || defined(__FreeBSD__)
  #include <ucontext.h>
#endif
static void UnblockSignals() {
#if defined(__linux__) || defined(__FreeBSD__)
  sigset_t set;
  sigemptyset(&set);
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
  actx[3] = ctx->gregs[REG_RBX];
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
  actx[3] = ctx->mc_rbx;
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
  mcontext_t  *ctx = &_ctx->uc_mcontext;
  CHashExport *exp;
  int64_t      is_single_step;
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
      ctx->mc_rbx = actx[3];
      ctx->mc_r12 = actx[4];
      ctx->mc_r13 = actx[5];
      ctx->mc_r14 = actx[6];
      ctx->mc_r15 = actx[7];
      ctx->mc_eflags |= 1 << 8;
      #elif defined(__linux__)
      ctx->gregs[REG_RIP] = actx[0];
      ctx->gregs[REG_RSP] = actx[1];
      ctx->gregs[REG_RBP] = actx[2];
      ctx->gregs[REG_RBX] = actx[3];
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
  goto call_exit;
  #endif
}
#endif
void InstallDbgSignalsForThread() {
#if defined(__linux__) || defined(__FreeBSD__)
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler   = SIG_IGN;
  sa.sa_flags     = SA_SIGINFO;
  sa.sa_sigaction = SigHandler;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
  sigaction(SIGTRAP, &sa, NULL);
  sigaction(SIGFPE, &sa, NULL);
#endif
}
