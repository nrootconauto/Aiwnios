#pragma once
#include "aiwn.h"
#include <stdint.h>
#define INTERFACE 0
void QueInit(CQue *i) {
  i->next = i, i->last = i;
}

void QueIns(CQue *a, CQue *to) {
  a->next       = to->next;
  a->last       = to;
  a->last->next = a;
  a->next->last = a;
}

void QueRem(CQue *a) {
  CQue *next = a->next, *last = a->last;
  next->last = last;
  last->next = next;
  QueInit(a);
}

int64_t QueCnt(CQue *head) {
  int64_t r = 0;
  CQue *q;
  for (q = head->next; q != head; q = q->next)
    r++;
  return r;
}

void QueDel(CQue *f) {
  CQue *next = f->next, *next2;
  while (next != f) {
    next2 = next->next;
    A_FREE(next);
    next = next2;
  }
  A_FREE(f);
}
