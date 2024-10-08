#pragma once
#include <stdint.h>
extern _Thread_local struct CTask *Fs;
void InstallDbgSignalsForThread();

void *GetHolyFs();
void *GetHolyGs();
void *GetHolyFsPtr();
void *GetHolyGsPtr();
void SetHolyFs(void *);
void SetHolyGs(void *);

int64_t mp_cnt();
void SpawnCore(void *fp, void *gs, int64_t core);
void MPSleepHP(int64_t ns);
void MPAwake(int64_t core);
void InteruptCore(int64_t core);
// Uses TempleOS ABI
// f is delay in nano seconds
extern void MPSetProfilerInt(void *fp, int c, int64_t f);
void __ShutdownCore(int core);
void DebuggerClientStart(void *, void **);
void DebuggerClientSetGreg(void *, int64_t, int64_t);
void DebuggerClientEnd(void *, int64_t);
void DebuggerClientWatchThisTID();
void TaskInit(struct CTask *, void *, int64_t);

void __bootstrap_tls(void);
