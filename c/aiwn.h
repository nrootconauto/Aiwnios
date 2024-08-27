#pragma once
#include "generated.h" //See CMakeLists.txt
#include <SDL.h>
#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct CHash;
struct CHashTable;
struct CHashFun;
struct CTask;

// See -g or --grab-focus
extern int64_t sdl_window_grab_enable;



struct CCmpCtrl;
struct CRPN;
struct CHashClass;
struct CHashClass;
struct CCodeCtrl;
struct CHashFun;
struct CCodeMisc;

struct CHashTable;
void SndFreq(int64_t f);
void InitSound();
int64_t IsValidPtr(char *chk);
int64_t Btr(void *, int64_t);
int64_t Bts(void *, int64_t);
void __HC_CmpCtrl_SetAOT(CCmpCtrl *cc);
void InteruptCore(int64_t core);
// Sets how many bytes before function start a symbol starts at
// Symbol    <=====RIP-off
// some...code
// Fun Start <==== RIP

extern void *GenFFIBinding(void *fptr, int64_t arity);
extern void *GenFFIBindingNaked(void *fptr, int64_t arity);
extern void PrsBindCSymbolNaked(char *name, void *ptr, int64_t arity);
void CmpCtrlCacheArgTrees(CCmpCtrl *cctrl);
const char *ResolveBootDir(char *use, int make_new_dir,const char *AIWNIOS_getcontext,_dir);

// Uses TempleOS ABI
i
extern int64_t bc_enable;
// Returns good region if good,else NULL and after is set how many bytes OOB
// Returns INVALID_PTR on error
extern void *BoundsCheck(void *ptr, int64_t *after);
// f is delay in nano seconds
extern void MPSetProfilerInt(void *fp, int c, int64_t f);
extern void *GetHolyFs();


extern void Misc_ForceYield();


void *GetHolyGs();
void *GetHolyFs();
void *GetHolyGsPtr();
void *GetHolyFsPtr();
void SetHolyFs(void *);
void SetHolyGs(void *);



void AiwniosSetVolume(double v);
double AiwniosGetVolume();


int64_t DoNothing();
