#pragma once
#include <stdint.h>
extern _Thread_local struct CTask *Fs;
void InstallDbgSignalsForThread();
void *GetHolyGs();
void SetHolyGs(void *);
void *GetHolyFs();
void SetHolyFs(void *);
int64_t mp_cnt();
void SpawnCore(void (*fp)(), void *gs, int64_t core);
void MPSleepHP(int64_t ns);
void MPAwake(int64_t core);
