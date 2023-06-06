#include "aiwn.h"
#include <SDL2/SDL.h>
typedef struct {
	void (*fp)(), *gs;
	int64_t num;
} CorePair;
#if defined(__linux__)
#include <linux/futex.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
typedef struct {
	pthread_t pt;
	int wake_futex;
} CCPU;
#elif defined(_WIN32) || defined(WIN32)
#include <processthreadsapi.h>
#include <synchapi.h>
#include <sysinfoapi.h>
#include <time.h>
#include <windows.h>
typedef struct {
	HANDLE thread;
	HANDLE event;
	HANDLE mtx;
	int64_t awake_at;
} CCPU;
#endif
static __thread core_num = 0;
static void threadrt(CorePair* pair)
{
	Fs = calloc(sizeof(CTask), 1);
	VFsThrdInit();
	TaskInit(Fs, NULL, 0);
	SetHolyGs(pair->gs);
	core_num = pair->num;
	void (*fp)();
	fp = pair->fp;
	free(pair);
	fp();
}
#if defined(__linux__)
static CCPU cores[128];
CHashTable* glbl_table;
void InteruptCore(int64_t core)
{
	pthread_kill(cores[core].pt, SIGUSR1);
}
static void InteruptRt(int ul)
{
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	CHashExport* y = HashFind("Yield", glbl_table, HTT_EXPORT_SYS_SYM, 1);
	void (*fp)();
	if (y) {
		fp = y->val;
		(*fp)();
	}
}
void SpawnCore(void (*fp)(), void* gs, int64_t core)
{
	char buf[144];
	CorePair pair = { fp, gs, core }, *ptr = malloc(sizeof(CorePair));
	*ptr = pair;
	pthread_create(&cores[core].pt, NULL, threadrt, ptr);
	signal(SIGUSR1, InteruptRt);
}
int64_t mp_cnt()
{
	static int64_t ret = 0;
	if (!ret)
		ret = sysconf(_SC_NPROCESSORS_ONLN);
	return ret;
}

void MPSleepHP(int64_t ns)
{
	struct timespec ts = { 0 };
	ts.tv_nsec += ns * 1000U;
	syscall(SYS_futex, &cores[core_num].wake_futex, 0, FUTEX_WAIT, &ts, NULL, 0);
	cores[core_num].wake_futex = 0;
}

void MPAwake(int64_t core)
{
	if (!Misc_LBts(&cores[core].wake_futex, 0))
		syscall(SYS_futex, &cores[core].wake_futex, 1, FUTEX_WAKE, NULL, NULL, 0);
}
#else
static CCPU cores[128];
CHashTable* glbl_table;
static int64_t ticks = 0;
static int64_t tick_inc = 1;
static void update_ticks(UINT tid, UINT msg, DWORD_PTR dw_user, void* ul,
	void* ul2)
{
	ticks += tick_inc;
	for (int64_t idx = 0; idx < mp_cnt(NULL); ++idx) {
		WaitForSingleObject(cores[idx].mtx, INFINITE);
		if (cores[idx].awake_at && ticks >= cores[idx].awake_at) {
			SetEvent(cores[idx].event);
			cores[idx].awake_at = 0;
		}
		ReleaseMutex(cores[idx].mtx);
	}
}
int64_t GetTicksHP()
{
	static int64_t init;
	TIMECAPS tc;
	if (!init) {
		init = 1;
		timeGetDevCaps(&tc, sizeof tc);
		tick_inc = tc.wPeriodMin;
		timeSetEvent(tick_inc, tick_inc, &update_ticks, NULL, TIME_PERIODIC);
	}
	return ticks;
}
void SpawnCore(void (*fp)(), void* gs, int64_t core)
{
	CorePair pair = { fp, gs, core }, *ptr = malloc(sizeof(CorePair));
	*ptr = pair;
	cores[core].mtx = CreateMutex(NULL, FALSE, NULL);
	cores[core].event = CreateEvent(NULL, 0, 0, NULL);
	cores[core].thread = CreateThread(NULL, 0, threadrt, ptr, 0, NULL);
	SetThreadPriority(cores[core].thread, THREAD_PRIORITY_HIGHEST);
}
void MPSleepHP(int64_t us)
{
	int64_t s, e;
	s = GetTicksHP();
	WaitForSingleObject(cores[core_num].mtx, INFINITE);
	cores[core_num].awake_at = s + us / 1000;
	ReleaseMutex(cores[core_num].mtx);
	WaitForSingleObject(cores[core_num].event, INFINITE);
}
void InteruptCore(int64_t core)
{
	puts("Poopies,this isnt implemented for windows yet");
	/*
	CONTEXT ctx;
	memset(&ctx, 0, sizeof ctx);
	ctx.ContextFlags = CONTEXT_FULL;
	SuspendThread(cores[core].thread);
	GetThreadContext(cores[core].thread, &ctx);
	ctx.Rsp -= 8;
	((int64_t *)ctx.Rsp)[0] = ctx.Rip;
	ctx.Rip = FUNC;
	SetThreadContext(cores[core].thread, &ctx);
	ResumeThread(cores[core].thread)
	*/
}
int64_t mp_cnt()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
}
void MPAwake(int64_t c)
{
	WaitForSingleObject(cores[c].mtx, INFINITE);
	cores[c].awake_at = 0;
	SetEvent(cores[c].event);
	ReleaseMutex(cores[c].mtx);
}
#endif
