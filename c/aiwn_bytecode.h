#pragma once
#include <stdint.h>
#include "aiwn_lexparser.h"
#include "aiwn_mem.h"
#define IM_STK_SIZE 0x80
typedef struct ABCFrame {
  union {
    double fstk[IM_STK_SIZE];
    int64_t istk[IM_STK_SIZE];
    uint64_t ustk[IM_STK_SIZE];
  };
  struct ABCFrame *next;
  int64_t sp;
} ABCFrame;
typedef struct ABCState {
  ABCFrame *fun_frame;
  int64_t ip, fp, frame_sz,_sp;
} ABCState;
char *OptPassFinalBC(CCmpCtrl *cctrl, int64_t *res_sz, char **dbg_info,
                   CHeapCtrl *heap);
void AiwnRunBC(ABCState *state);
ABCState *ABCStateNew(void *bc_addr,void *stk_fptr,int64_t argc,int64_t *argv);
void ABCStateDel(ABCState *st);
int64_t ABCRun_Done(ABCState *st,int64_t *retval);
void AiwnBCDel(char *bc);
int64_t ABCRun(ABCState *st);
uint32_t *BCGenerateFFICall(void *fcall);
uint64_t AiwnBCContextSet(ABCState **to_stk);
uint64_t AiwnBCContextGet(ABCState **to_stk);
int64_t AiwnBCCallArgs(void *bc,int64_t argc,int64_t *argv);
int64_t AiwnBCCall(void *bc);
int64_t AiwnBC_FP();
