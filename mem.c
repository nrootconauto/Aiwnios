#pragma once
#include "aiwn.h"
#include <stdint.h>
#if defined(_WIN32) || defined(WIN32)
  #include <memoryapi.h>
  #include <sysinfoapi.h>
#else
  #include <sys/mman.h>
  #include <unistd.h>
#endif
//
// Dec 8, I volenterreed today
// Jun 29,Im staying up all night
//
struct CTask;
struct CMemBlk;
struct CMemUnused;

// Bounds checker works like this.
// All allocations will be in first 32bits
// There will be a bitmap of "good" 8bytes(this is ok as most accesses are 8
// bytes wide) Beacuse the address space will be 2 GB,we can have a fixed length
// bitmap of 2GB/8/64 int64_t's and i can Misc_Bt it to check if it is valid
//
int64_t      bc_enable = 0;
static char *bc_good_bitmap;

void InitBoundsChecker() {
  int64_t want   = (1ll << 31) / 8;
  bc_good_bitmap = calloc(want, 1);
  bc_enable      = 1;
}
static void MemPagTaskFree(CMemBlk *blk, CHeapCtrl *hc) {
  QueRem(blk);
  hc->alloced_u8s -= blk->pags * MEM_PAG_SIZE;
#if defined(_WIN32) || defined(WIN32)
  VirtualFree(blk, 0, MEM_RELEASE);
#else
  static int64_t ps;
  int64_t        b;
  if (!ps)
    ps = sysconf(_SC_PAGESIZE);
  b = (blk->pags * MEM_PAG_SIZE + ps - 1) & ~(ps - 1);
  munmap(blk, b);
#endif
}

static int64_t Hex2I64(char *ptr, char **_res);
static void   *GetAvailRegion32(int64_t b);

static CMemBlk *MemPagTaskAlloc(int64_t pags, CHeapCtrl *hc) {
  if (!hc)
    hc = Fs->heap;
#if defined(_WIN32) || defined(WIN32)
  CMemBlk       *ret = NULL;
  static int64_t dwAllocationGranularity;
  if (!dwAllocationGranularity) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    dwAllocationGranularity = si.dwAllocationGranularity;
  }
  int64_t b = (pags * MEM_PAG_SIZE) / dwAllocationGranularity *
              dwAllocationGranularity,
          _try, addr;
  if ((pags * MEM_PAG_SIZE) % dwAllocationGranularity)
    b += dwAllocationGranularity;
  if (bc_enable) {
    ret = VirtualAlloc(GetAvailRegion32(b), b, MEM_COMMIT | MEM_RESERVE,
                       hc->is_code_heap ? PAGE_EXECUTE_READWRITE
                                        : PAGE_READWRITE);
  } else
    ret = VirtualAlloc(NULL, b, MEM_COMMIT | MEM_RESERVE,
                       hc->is_code_heap ? PAGE_EXECUTE_READWRITE
                                        : PAGE_READWRITE);
  if (!ret)
    return NULL;
#else
  #if defined(__x86_64__)
  int64_t        add_flags = bc_enable || hc->is_code_heap ? MAP_32BIT : 0;
  #else
  int64_t add_flags = 0;
  #endif
  static int64_t ps;
  int64_t        b;
  if (!ps)
    ps = sysconf(_SC_PAGESIZE);
  b = (pags * MEM_PAG_SIZE + ps - 1) & ~(ps - 1);
  CMemBlk *ret =
      mmap(NULL, b, (hc->is_code_heap ? PROT_EXEC : 0) | PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS | add_flags, -1, 0);
#endif
  int64_t      threshold = MEM_HEAP_HASH_SIZE >> 4, cnt;
  CMemUnused **unm, *tmp, *tmp2;
  QueIns(&ret->base, hc->mem_blks.last);

  ret->pags = pags;
  // Move silly willies malloc_free_lst,in to the heap_hash
  do {
    cnt = 0;
    unm = &hc->malloc_free_lst;
    while (tmp = *unm) {
      if (tmp->sz < threshold) {
        *unm                       = tmp->next;
        tmp->next                  = hc->heap_hash[tmp->sz / 8];
        hc->heap_hash[tmp->sz / 8] = tmp;
      } else {
        cnt++;
        unm = tmp;
      }
    }
    threshold <<= 1;
  } while (cnt > 8 && threshold <= MEM_HEAP_HASH_SIZE);
  hc->alloced_u8s += pags * MEM_PAG_SIZE;
  return ret;
}

void *GetFs() {
  return Fs;
}
void *__AIWNIOS_MAlloc(int64_t cnt, void *t) {
  if (!cnt)
    return NULL;
  if (!t)
    t = Fs->heap;
  int64_t orig = cnt;
  cnt += 16;
  CHeapCtrl  *hc = t;
  int64_t     pags;
  CMemUnused *ret;
  if (hc->hc_signature != 'H')
    hc = ((CTask *)hc)->heap;
  if (hc->hc_signature != 'H')
    throw('BadMAll');
  // Rounnd up to 8
  cnt += 7 + sizeof(CMemUnused);
  cnt &= (int8_t)0xf8;
  // HClF_LOCKED is 1
  while (Misc_LBts(&hc->locked_flags, 1))
    ;
  if (cnt > MEM_HEAP_HASH_SIZE)
    goto big;
  if (hc->heap_hash[cnt / 8]) {
    ret                    = hc->heap_hash[cnt / 8];
    hc->heap_hash[cnt / 8] = ret->next;
    goto almost_done;
  } else
    ret = hc->malloc_free_lst;
  if (!ret) {
  new_lunk:
    // Make a new lunk
    ret = MemPagTaskAlloc(
        (pags = ((cnt + 16 * MEM_PAG_SIZE - 1) >> MEM_PAG_BITS)) + 1, hc);
    ret                 = (char *)ret + sizeof(CMemBlk);
    ret->sz             = (pags << MEM_PAG_BITS) - sizeof(CMemBlk);
    hc->malloc_free_lst = ret;
  }
  // Chip off
  //
  // We must make sure there is room for another CMemUnused
  //
  if ((int64_t)(ret->sz - sizeof(CMemUnused)) >= cnt) {
    hc->malloc_free_lst     = (char *)ret + cnt;
    hc->malloc_free_lst->sz = ret->sz - cnt;
    ret->sz                 = cnt;
    goto almost_done;
  } else
    goto new_lunk;
big:
  ret     = MemPagTaskAlloc(1 + ((cnt + sizeof(CMemBlk)) >> MEM_PAG_BITS), hc);
  ret     = (char *)ret + sizeof(CMemBlk);
  ret->sz = cnt;
  goto almost_done;
almost_done:
  hc->used_u8s += cnt;
  Misc_LBtr(&hc->locked_flags, 1);
  ret->hc = hc;
  ret++;
  if (bc_enable) {
    assert((int64_t)ret < (1ll << 31));
    if (orig < 8)
      orig = 8;
    if (orig & 7)
      orig = 8 + orig & ~7ll;
    memset(&bc_good_bitmap[(int64_t)ret / 8], 7, orig / 8);
  }
  return ret;
}

void __AIWNIOS_Free(void *ptr) {
  CMemUnused *un = ptr, *hash;
  CHeapCtrl  *hc;
  int64_t     cnt;
  if (!ptr)
    return;
  un--; // Area before ptr is CMemUnused*
  if (!un->hc)
    throw('BadFree');
  if (un->hc->hc_signature != 'H')
    throw('BadFree');
  if (un->sz < 0) // Aligned chunks are negative and point to start
    un = un->sz + (char *)un;
  if (bc_enable) {
    cnt = MSize(ptr);
    assert((int64_t)ptr < (1ll << 31));
    memset(&bc_good_bitmap[(int64_t)ptr / 8], 0, cnt / 8);
  }
  hc = un->hc;
  while (Misc_LBts(&hc->locked_flags, 1))
    ;
  hc->used_u8s -= un->sz;
  if (un->sz <= MEM_HEAP_HASH_SIZE) {
    hash                      = hc->heap_hash[un->sz / 8];
    un->next                  = hash;
    hc->heap_hash[un->sz / 8] = un;
    un->hc                    = NULL;
  } else {
    // CMemUnused
    // CMemBlk
    // page start
    MemPagTaskFree((char *)(un) - sizeof(CMemBlk), hc);
  }
fin:
  Misc_LBtr(&hc->locked_flags, 1);
}

int64_t MSize(void *ptr) {
  CMemUnused *un = ptr;
  int64_t     cnt;
  if (!ptr)
    return 0;
  un--;
  if (un->hc->hc_signature != 'H')
    throw('BadFree');
  if (un->sz < 0) // Aligned chunks are negative and point to start
    un = un->sz + (char *)un;
  return un->sz - sizeof(CMemUnused);
}

void *__AIWNIOS_CAlloc(int64_t cnt, void *t) {
  return memset(__AIWNIOS_MAlloc(cnt, t), 0, cnt);
}

CHeapCtrl *HeapCtrlInit(CHeapCtrl *ct, CTask *task, int64_t is_code_heap) {
  if (!ct)
    ct = calloc(sizeof(CHeapCtrl), 1);
  if (!task)
    task = Fs;
  ct->is_code_heap    = is_code_heap;
  ct->hc_signature    = 'H';
  ct->mem_task        = task;
  ct->malloc_free_lst = NULL;
  QueInit(&ct->mem_blks);
  return ct;
}

void HeapCtrlDel(CHeapCtrl *ct) {
  CMemBlk *next, *m;
  // TODO atomic
  while (Misc_Bt(&ct->locked_flags, 1))
    ;
  for (m = ct->mem_blks.next; m != &ct->mem_blks; m = next) {
    next = m->base.next;
#if defined(_WIN32) || defined(WIN32)
    VirtualFree(m, 0, MEM_RELEASE);
#else
    static int64_t ps;
    int64_t        b;
    if (!ps)
      ps = sysconf(_SC_PAGESIZE);
    b = (m->pags * MEM_PAG_SIZE + ps - 1) & ~(ps - 1);
    munmap(m, b);
#endif
  }
  free(ct);
}

char *__AIWNIOS_StrDup(char *str, void *t) {
  if (!str)
    return NULL;
  int64_t cnt = strlen(str);
  char   *ret = A_MALLOC(cnt + 1, t);
  memcpy(ret, str, cnt + 1);
  return ret;
}
#if defined(_WIN32) || defined(WIN32)
int64_t IsValidPtr(char *chk) {
  MEMORY_BASIC_INFORMATION mbi;
  memset(&mbi, 0, sizeof mbi);
  if (VirtualQuery(chk, &mbi, sizeof mbi)) {
    // https://stackoverflow.com/questions/496034/most-efficient-replacement-for-isbadreadptr
    DWORD mask =
        (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
         PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
    int64_t b = !!(mbi.Protect & mask);
    return b;
  }
  return 0;
}

static void *GetAvailRegion32(int64_t len) {
  static int64_t dwAllocationGranularity;
  static void   *try_page       = 0x100000;
  void          *start_page     = try_page;
  int64_t        wrapped_around = 0, sz;
  if (!dwAllocationGranularity) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    dwAllocationGranularity = si.dwAllocationGranularity;
  }

  MEMORY_BASIC_INFORMATION mbi;
  while (!(!wrapped_around && start_page > try_page)) {
    memset(&mbi, 0, sizeof mbi);
    if (sz = VirtualQuery(try_page, &mbi, sizeof(mbi))) {
      if (mbi.State & MEM_FREE && mbi.RegionSize > len) {
        return try_page;
      }
    next:
      if (mbi.State & MEM_FREE)
        try_page += mbi.RegionSize;
      else
        try_page = (char *)mbi.BaseAddress + mbi.RegionSize;
      if (try_page > (void *)(1ll << 31)) {
        try_page       = 0x100000;
        wrapped_around = 1;
      }
    } else
      abort();
  }
  return NULL;
}
#else
static int64_t Hex2I64(char *s, char **end) {
  int64_t ret = 0;
  while (isxdigit(*s)) {
    ret <<= 4;
    if (isdigit(*s))
      ret += *s - '0';
    else
      ret += toupper(*s) - 'A' + 10;
    s++;
  }
  if (end)
    *end = s;
  return ret;
}
int64_t IsValidPtr(char *chk) {
  static int64_t ps;
  int64_t mptr = chk;
  if (!ps)
    ps = sysconf(_SC_PAGESIZE);
  mptr &= ~(ps - 1);
  // https://renatocunha.com/2015/12/msync-pointer-validity/
  return -1 != msync(mptr, ps, MS_ASYNC);
}

#endif
// Returns good region if good,else NULL and after is set how many bytes OOB
// Returns INVALID_PTR on error
void *BoundsCheck(void *ptr, int64_t *after) {
  if (!bc_enable)
    return INVALID_PTR;
  int64_t waste = 0;
  int64_t optr  = ptr;
  ptr           = ((int64_t)ptr) & ~7ll;
  if (after)
    *after = 0;
  // TOO big
  if ((int64_t)ptr > 1ll << 31)
    return ptr;
  while ((int64_t)ptr + waste >= 0) {
    if (bc_good_bitmap[(int64_t)(ptr + waste) / 8])
      break;
    waste -= 8;
  }
  if (!waste || (optr - (int64_t)(ptr + waste)) == 0)
    return ptr;
  if (after)
    *after = optr - (int64_t)(ptr + waste);
  return NULL;
}
