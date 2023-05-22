#include "aiwn.h"
static void UnblockSignals()
{
#if defined(__linux__)
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGSEGV);
	sigaddset(&set, SIGBUS);
	sigaddset(&set, SIGTRAP);
	sigaddset(&set, SIGFPE);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
#endif
}
#if defined (__linux__)
static void SigHandler(int64_t sig, siginfo_t* info, ucontext_t* _ctx)
{
	#if defined(__x86_64__)
	#elif defined(_ARM64_)
	mcontext_t* ctx = &_ctx->uc_mcontext;
	CHashExport* exp;
	int64_t is_single_step;
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
			actx[i - 18] = ctx->regs[i];
	// We will look for FPSIMD_MAGIC(0x46508001)
	// From https://github.com/torvalds/linux/blob/master/arch/arm64/include/uapi/asm/sigcontext.h
	for (i = 0; i < 4096;) {
		sz = ((uint32_t*)(&ctx->__reserved[i]))[1];
		if (((uint32_t*)(&ctx->__reserved[i]))[0] == 0x46508001) {
			i += 4 + 4 + 4 + 4;
			fp_idx = i;
			for (i2 = 8; i2 <= 15; i2++)
				actx[(30 - 18 + 1) + i2 - 8] = *(int64_t*)(&ctx->__reserved[fp_idx + 16 * i2]);
			break;
		} else if (!sz)
			break;
		else
			i += sz;
	}
	actx[21] = ctx->sp;
	// AiwniosDbgCB will return 1 for singlestep
	if (exp = HashFind("AiwniosDbgCB", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
		fp = exp->val;
		actx[12] = ctx->pc;
		is_single_step = fp(sig, actx);
		ctx->pc = actx[12];
		for (i = 18; i <= 30; i++)
			if (i - 18 != 12) // the 12th one is the return register
				ctx->regs[i] = actx[i - 18];
		// for(i2=8;i2<=15;i2++)
		//   *(int64_t*)(&ctx->__reserved[fp_idx+16*i2])=actx[(30-18+1)+i2-8];
		if (is_single_step) {
			ctx->pstate |= (1 << 21);
			puts("ss");
		} else
			ctx->pstate &= ~(1 << 21);
	} else if (exp = HashFind("Exit", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
		fp2 = exp->val;
		ctx->pc = fp2;
	} else
		abort();
	__builtin___clear_cache(ctx->pc, 1024);
	setcontext(_ctx);
	#endif
}
#endif
void InstallDbgSignalsForThread()
{
	#if defined(__linux__)
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = SigHandler;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
	sigaction(SIGTRAP, &sa, NULL);
	sigaction(SIGFPE, &sa, NULL);
	#endif
}
