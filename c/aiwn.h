#pragma once
#include "generated.h" //See CMakeLists.txt
#include <SDL.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
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
int64_t IsValidPtr(char *chk);
void InteruptCore(int64_t core);
// Sets how many bytes before function start a symbol starts at
// Symbol    <=====RIP-off
// some...code
// Fun Start <==== RIP

void CmpCtrlCacheArgTrees(CCmpCtrl *cctrl);

extern void Misc_ForceYield();

int64_t DoNothing();
