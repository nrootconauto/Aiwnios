#pragma once
#include <stdint.h>
typedef struct CQue {
  struct CQue *last, *next;
} CQue;
void QueDel(CQue *f);
int64_t QueCnt(CQue *head);
void QueRem(CQue *a);
void QueIns(CQue *a, CQue *to);
void QueInit(CQue *i);
