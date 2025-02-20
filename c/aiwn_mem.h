#pragma once
#include "aiwn_multic.h"
#include "aiwn_que.h"
#include <setjmp.h>
#include <stdint.h>
typedef struct CTask {
  void *pad;
  int64_t task_signature;
  struct CQue *except; // CExcept
  int64_t except_ch;
  int64_t catch_except;
  struct CHashTable *hash_table;
  void *stack;
  char ctx[2048];
  jmp_buf throw_pad;
  struct CHeapCtrl *heap, *code_heap;
} CTask;

struct CHashTable;
struct CMemUnused;
#define HClF_LOCKED        1
#define MEM_HEAP_HASH_SIZE 1024
#define MEM_PAG_BITS       (12)
#define MEM_PAG_SIZE       (1 << MEM_PAG_BITS)
typedef struct CHeapCtrlArena {
  struct CMemUnused *heap_hash[5];
  struct CMemUnused *used_next, *used_last;
  struct CMemUnused *malloc_free_lst;
} CHeapCtrlArena;
typedef struct CBuddyPair {
  int64_t offset_l, offset_r;
  struct CBuddyPair *left, *right;
  char occupied, partial;
} CBuddyPair;
typedef struct CHeapCtrl {
  void *pad; // Needed
  int32_t hc_signature;
  int32_t is_code_heap;
  int64_t locked_flags, alloced_u8s, used_u8s;
  struct CTask *mem_task;
  int64_t arena_lock;
  CHeapCtrlArena arenas[16];
  CQue mem_blks;
#if defined(__OpenBSD__)
  int code_shm_fd;
  CBuddyPair *buddies;
#endif
} CHeapCtrl;
typedef struct CMemBlk {
  CQue base;
  CQue base2;
  int64_t pags;
#if defined(__OpenBSD__)
  void *rx;
  void *rw;
  int64_t shm_pag;
#endif
} CMemBlk;
typedef struct __attribute__((packed)) CMemUnused {
  // MUST BE FIRST MEMBER
  struct CMemUnused *next;
  struct CMemUnused *last; // Used for used memory only
  CHeapCtrl *hc;
  void *caller1, *caller2; // Used for used memory only
  int64_t which_bucket;
  int64_t sz; // MUST BE LAST MEMBER FOR MAllocAligned
} CMemUnused;

#define A_FREE              __AIWNIOS_Free
#define A_MALLOC(sz, task)  __AIWNIOS_MAlloc(sz, task)
#define A_CALLOC(sz, task)  __AIWNIOS_CAlloc(sz, task)
#define A_STRDUP(str, task) __AIWNIOS_StrDup(str, task)
void *__AIWNIOS_CAlloc(int64_t cnt, void *t);
void *__AIWNIOS_MAlloc(int64_t cnt, void *t);
void __AIWNIOS_Free(void *ptr);
char *__AIWNIOS_StrDup(char *str, void *t);

char *__AIWNIOS_StrDup(char *str, void *t);
void HeapCtrlDel(CHeapCtrl *ct);
CHeapCtrl *HeapCtrlInit(CHeapCtrl *ct, CTask *task, int64_t code_heap);
void *__AIWNIOS_CAlloc(int64_t cnt, void *t);
int64_t MSize(void *ptr);
void __AIWNIOS_Free(void *ptr);
void *__AIWNIOS_MAlloc(int64_t cnt, void *t);

int SetWriteNP(int);
#ifndef __APPLE__
#  define SetWriteNP(n) (n)
#endif
extern int64_t bc_enable;
// Returns good region if good,else NULL and after is set how many bytes OOB
// Returns INVALID_PTR on error
extern void *BoundsCheck(void *ptr, int64_t *after);
int64_t IsValidPtr(char *chk);
void InitBoundsChecker();
extern int64_t WriteProtectMemCpy(char *dst, char *src, int64_t len);
void *MemGetWritePtr(void *ptr);
void *MemGetExecPtr(void *ptr);
