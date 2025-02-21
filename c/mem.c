#include "aiwn_asm.h"
#include "aiwn_except.h"
#include "aiwn_mem.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ERR         0x7fFFffFFffFFffFF
#define INVALID_PTR ERR
#ifdef _WIN64
#  define _WIN32_WINNT 0x0602 /* [2] (GetProcessMitigationPolicy) */
#  include <windows.h>
#  include <memoryapi.h>
#  include <sysinfoapi.h>
#else
#  include <fcntl.h>
#  include <sys/mman.h>
#  include <unistd.h>
#endif
//
// Dec 8, I volenterreed today
// Jun 29,Im staying up all night
//

// Bounds checker works like this.
// All allocations will be in first 32bits
// There will be a bitmap of "good" 8bytes(this is ok as most accesses are 8
// bytes wide) Beacuse the address space will be 2 GB,we can have a fixed length
// bitmap of 2GB/8/64 int64_t's and i can Misc_Bt it to check if it is valid
//
int64_t bc_enable = 0;
static char *bc_good_bitmap;
static int64_t NextPower2(uint64_t v) {
  // https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}
static void **new_nul = &new_nul;
#if defined(__OpenBSD__)
/* THESE ARE MOTHERFUCKIN SORTED BY ->rx
 * Keep it that way or the computer will get confused.
 * */
typedef struct CMemPair {
  int64_t len;
  int fd;
  char *rw;
  char *rx;
} CMemPair;
static CMemPair *mem_pairs = NULL;
static int64_t mem_pairs_lock, mem_pairs_len;
static _Thread_local int64_t mem_pair_last_idx;
static void AddMemPair(void *rw, void *rx, int64_t mlen, int fd) {
  int64_t ptr, len, ptr2;
  char *last = NULL;
  CMemPair *cur = mem_pairs, *copy;
  int64_t density;
  while (Misc_LBts(&mem_pairs_lock, 0))
    PAUSE;
again:;
  last = (1ull << 31);
  len = mem_pairs_len;
  for (ptr = len - 1; ptr >= 0; ptr--) {
    cur = mem_pairs + ptr;
    if (!cur->rw && !cur->rx) {
      cur->rx = rx;
      cur->rw = rw;
      cur->len = mlen;
      cur->fd = fd;
      goto end;
    }
  }
  // Resize with spacing
  cur = mem_pairs;
  density = 0;
  for (ptr = 0; ptr != len; ptr++) {
    if (cur->rx)
      density++;
    cur++;
  }
  mem_pairs_len = density + 32;
  cur = mem_pairs;

  mem_pairs = calloc(sizeof(CMemPair), mem_pairs_len);
  for (ptr2 = ptr = 0; ptr != len; ptr++) {
    CMemPair *old = &cur[ptr];
    if (old->rx) {
      mem_pairs[ptr2++] = *old;
    }
  }
  free(cur);
  goto again;
end:
  Misc_LBtr(&mem_pairs_lock, 0);
}
static CMemPair *GetMemPairForPtrRX(void *rx) {
  int64_t ptr = 0, len;
  while (Misc_LBts(&mem_pairs_lock, 0))
    PAUSE;
  len = mem_pairs_len;
  CMemPair *cur = mem_pairs;
  if (mem_pair_last_idx < len) {
    CMemPair *last = &mem_pairs[mem_pair_last_idx];
    if (last->rw) {
      if (last->rx <= rx) {
        if (last->rx + last->len > rx) {
          cur = last;
          goto end;
        }
      }
    }
  }
  for (cur = mem_pairs; ptr != len; ptr++, cur++) {
    if (!cur->rw)
      continue;
    if (cur->rx <= rx) {
      if (cur->rx + cur->len > rx) {
        mem_pair_last_idx = ptr;
        goto end;
      }
    }
  }
  cur = NULL;
end:
  Misc_LBtr(&mem_pairs_lock, 0);
  return cur;
}

static CMemPair *GetMemPairForPtrRW(void *rw) {
  int64_t ptr, len;
  while (Misc_LBts(&mem_pairs_lock, 0))
    PAUSE;
  len = mem_pairs_len;
  CMemPair *cur = mem_pairs;
  for (ptr = 0; ptr != len; ptr++, cur++) {
    if (!cur->rw)
      continue;
    if (cur->rw <= rw) {
      if (cur->rw + cur->len > rw) {
        goto end;
      }
    }
  }
  cur = NULL;
end:
  Misc_LBtr(&mem_pairs_lock, 0);
  return cur;
}

static void *OBSD_SexyGetLow32(int64_t len, int fd) {
  static int64_t ps = 0;
  static void *low;
  int attempt;
  if (!ps) {
    ps = sysconf(_SC_PAGE_SIZE);
    low = (void *)ps;
  }
  for (attempt = 0; attempt != 2; attempt++) {
    while (MAP_FAILED == mquery(low, len, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_FIXED, fd, 0)) {
      if (low > (void *)(1ull << 31)) {
        break;
      }
      low = (char *)low + ps;
    }
    if (low > (void *)(1ull << 31)) {
      low = (void *)ps;
      continue;
    }
    return low;
  }
  return NULL;
}
#  include <sys/stat.h>
static void OBSD_SexyBuddyDel0(CBuddyPair *bp) {
  if (!bp)
    return;
  OBSD_SexyBuddyDel0(bp->left);
  OBSD_SexyBuddyDel0(bp->right);
  free(bp);
}
static int64_t OBSD_SexyBuddyAlloc(int64_t use, int64_t want, int64_t order,
                                   CBuddyPair *root) {
  int64_t bits = 0, tmp = 0, tmp2;
  want = NextPower2(want);
  int64_t pags = 1ul << order;
  CBuddyPair *new;
  if (pags < want)
    return -1;
  if (root->occupied)
    return -1;
  if (pags == want) {
    if (root->left != NULL || root->right != NULL)
      return -1;
    root->occupied = 1;
    return use;
  }

  if (!root->left) {
  new_left:
    new = root->left = calloc(1, sizeof(CBuddyPair));
    new->offset_l = root->offset_l;
    new->offset_r = root->offset_l + (1ul << (order - 1));
    return OBSD_SexyBuddyAlloc(new->offset_l, want, order - 1, new);
  }
  if (root->left) {
    if (1) {
      tmp = OBSD_SexyBuddyAlloc(root->offset_l, want, order - 1, root->left);
      if (0 <= tmp) {
        return tmp;
      }
    }
  }
  if (!root->right) {
  new_right:
    new = root->right = calloc(1, sizeof(CBuddyPair));
    new->offset_l = root->offset_r;
    new->offset_r = root->offset_r + (1ul << (order - 1));
    return OBSD_SexyBuddyAlloc(new->offset_r, want, order - 1, new);
  }
  if (root->right) {
    if (1) {
      tmp = OBSD_SexyBuddyAlloc(root->offset_r, want, order - 1, root->right);
      if (0 <= tmp) {
        return tmp;
      }
    }
  }
  return -1;
};
static int OBSD_SexyBuddyFree(int64_t off, int64_t pags, int64_t order,
                              CBuddyPair *root) {
  pags = NextPower2(pags);
  if (pags == (1u << order)) {
    OBSD_SexyBuddyDel0(root);
    return 1;
  }
  if (off < root->offset_r && root->left)
    if (OBSD_SexyBuddyFree(off, pags, order - 1, root->left))
      root->left = NULL;
  if (root->right && off >= root->offset_r)
    if (OBSD_SexyBuddyFree(off, pags, order - 1, root->right))
      root->right = NULL;
  if (!root->left && !root->right) {
    OBSD_SexyBuddyDel0(root);
    return 1;
  }
  return 0;
};

static void OBSD_SexyDelShm(CHeapCtrl *hc, void *rx) {
  CMemPair *mp = GetMemPairForPtrRX(rx);
  CMemBlk *blk = rx;
  if (!mp)
    return; //???
  if (OBSD_SexyBuddyFree(blk->shm_pag, blk->pags, 31, hc->buddies)) {
    hc->buddies = calloc(1, sizeof(CBuddyPair));
    hc->buddies->offset_r = (1ul << 31) / 2;
  }
  while (Misc_LBts(&mem_pairs_lock, 0))
    PAUSE;
  mp->rw = NULL;
  mp->rx = NULL;
  mp->len = 0;
  Misc_LBtr(&mem_pairs_lock, 0);
}
// RETURNS pag in shm file
static int64_t OBSD_SexyShmAlloc(void **low32_rx, void **rw, int64_t len,
                                 CHeapCtrl *hc) {
  int fd = hc->code_shm_fd;
  int64_t foffset = 0;
  char the_name[0x100];
  int new = 0;
  if (fd == 0) {
    new = 1;
    strcpy(the_name, "/tmp/AiwnPhysXXXXXXX");
    fd = hc->code_shm_fd = shm_mkstemp(the_name);
  }
  void *the_addy, *p1, *p2;
  static int64_t ps = 0;
  if (!ps)
    ps = sysconf(_SC_PAGE_SIZE);
  if (len % ps)
    len = (len / ps + 1) * ps;
  if (fd < 0) {
    if (low32_rx)
      *low32_rx = NULL;
    if (low32_rx)
      *rw = NULL;
  } else {
    foffset = OBSD_SexyBuddyAlloc(0, len / ps, 31, hc->buddies) * ps;
    int64_t flen;
    struct stat sb;
    fstat(fd, &sb);
    flen = sb.st_size;
    flen = flen >= foffset + len ? flen : foffset + len;
    ftruncate(fd, flen);
    the_addy = OBSD_SexyGetLow32(len, fd);
    if (the_addy != NULL)
      p1 = mmap(the_addy, len, PROT_READ | PROT_EXEC, MAP_SHARED | MAP_FIXED,
                fd, foffset);
    else
      p1 = mmap(NULL, len, PROT_READ | PROT_EXEC, MAP_SHARED, fd, foffset);
    p2 = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, foffset);
    if (p1 == MAP_FAILED) {
      // Fuck,your on your own.
      p1 = mmap(NULL, len, PROT_READ | PROT_EXEC, MAP_SHARED, fd, foffset);
    }
    if (low32_rx)
      *low32_rx = p1;
    if (rw)
      *rw = p2;
    AddMemPair(p2, p1, len, fd);
    if (new)
      shm_unlink(the_name); // OpenBSD keeps them until all fd's are closed
    // We unlink the shm file then we close later ,got it
  }
  return foffset / ps;
}
#endif
#if defined(__APPLE__)
static CQue code_pages = {0};
_Thread_local int is_write_np = 123;
void InvalidateCache() {
  CMemBlk *blk;
  CQue *head, *cur, cnt;
  // Mac exclusive! CLEAR THE CODE PAGES
  if (code_pages.next) {
    head = &code_pages;
    for (cur = head->next; cur != head; cur = cur->next) {
      blk = (char *)cur - offsetof(CMemBlk, base2);
      __builtin___clear_cache(blk, (char *)blk + blk->pags * MEM_PAG_SIZE);
    }
  }
}

#  include <pthread.h>
int SetWriteNP(int st) {
  int old = is_write_np;
  is_write_np = st;
  if (st != old) {
    pthread_jit_write_protect_np(st);
    if (!old && st)
      InvalidateCache();
  }
  return old;
}
#endif

void InitBoundsChecker() {
  int64_t want = (1ull << 31) / 8;
  bc_good_bitmap = calloc(want, 1);
  bc_enable = 1;
}
static void MemPagTaskFree(CMemBlk *blk, CHeapCtrl *hc) {
#ifndef _WIN64
  static int64_t ps;
  int64_t b;
  if (!ps)
    ps = sysconf(_SC_PAGE_SIZE);
  b = (blk->pags * MEM_PAG_SIZE + ps - 1) & ~(ps - 1);

#endif
#if defined(__OpenBSD__)
  CMemBlk *rw, *rx;
  blk = blk->rw;
  hc->alloced_u8s -= blk->pags * MEM_PAG_SIZE;
  rw = blk;
  rx = blk->rx;
  QueRem(rw);
  OBSD_SexyDelShm(hc, rx);
  munmap(rw, b);
  if (rx && rx != rw)
    munmap(rx, b);
  return;
#endif
  QueRem(blk);
  hc->alloced_u8s -= blk->pags * MEM_PAG_SIZE;
#ifdef _WIN64
  VirtualFree(blk, 0, MEM_RELEASE);
#else
#  if defined(__APPLE__)
  if (blk->base2.next)
    QueRem(&blk->base2);
#  endif
  munmap(blk, b);
#endif
}

#ifndef _WIN32
static void *GetAvailRegion32(int64_t b);
#else
static void *NewVirtualChunk(uint64_t sz, bool low32, bool exec);
#endif
#ifdef _WIN64
typedef BOOL (*fp_GetProcessMitigationPolicy_t)(HANDLE,
                                                PROCESS_MITIGATION_POLICY,
                                                PVOID, size_t);
fp_GetProcessMitigationPolicy_t *DynamicFptr_GetProcessMitigationPolicy() {
  static int init = 0;
  static fp_GetProcessMitigationPolicy_t ret = NULL;
  if (!init) {
    HANDLE *k32_dll = GetModuleHandleA("kernel32.dll");
    ret = GetProcAddress(k32_dll, "GetProcessMitigationPolicy");
    init = 1;
  }
  return ret;
}
#endif
#if defined(__OpenBSD__)
static CMemBlk *MemPagTaskAlloc(int64_t pags, CHeapCtrl *hc) {
  if (!hc)
    hc = Fs->heap;
  int64_t b;
  static int64_t ps = 0;
  int64_t pag = -1;
  CMemBlk *ret;
  CMemBlk *rw = NULL, *rx = NULL;
  int fd = -1;
  if (!ps)
    ps = sysconf(_SC_PAGESIZE);
  b = (pags * MEM_PAG_SIZE + ps - 1) & ~(ps - 1);
  if (hc->is_code_heap) {
    pag = OBSD_SexyShmAlloc(&rx, &rw, b, hc);
    ret = rx;
  } else {
    ret = mmap(NULL, b, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    rw = rx = ret;
  }
  QueIns(&rw->base, hc->mem_blks.last);
  rw->pags = pags;
  rw->rw = rw;
  rw->rx = rx;
  rw->shm_pag = pag;
  hc->alloced_u8s += pags * MEM_PAG_SIZE;
  return ret;
}
#endif
#if !defined(__OpenBSD__)
static CMemBlk *MemPagTaskAlloc(int64_t pags, CHeapCtrl *hc) {
  if (!hc)
    hc = Fs->heap;
  void *at = NULL;
#  ifdef _WIN64
  static bool init;
  static uint64_t ag;
  if (!init) {
    SYSTEM_INFO si;
    HANDLE proc;
    GetSystemInfo(&si);
    ag = si.dwAllocationGranularity;
    proc = GetCurrentProcess();
    fp_GetProcessMitigationPolicy_t fp_GetProcessMitigationPolicy =
        DynamicFptr_GetProcessMitigationPolicy();
    if (fp_GetProcessMitigationPolicy) {
      PROCESS_MITIGATION_DYNAMIC_CODE_POLICY wxallowed;
      /* Disable ACG */
      fp_GetProcessMitigationPolicy(proc, ProcessDynamicCodePolicy, &wxallowed,
                                    sizeof wxallowed);
      wxallowed.ProhibitDynamicCode = 0;
      fp_GetProcessMitigationPolicy(proc, ProcessDynamicCodePolicy, &wxallowed,
                                    sizeof wxallowed);
    }
    init = true;
  }
  int64_t b = (pags * MEM_PAG_SIZE) / ag * ag, _try, addr;
  CMemBlk *ret = NULL;
  if ((pags * MEM_PAG_SIZE) % ag)
    b += ag;
  ret = NewVirtualChunk(b, bc_enable || hc->is_code_heap, hc->is_code_heap);
  if (!ret)
    return NULL;
#  else
#    ifdef __x86_64__
  int64_t add_flags = bc_enable || hc->is_code_heap ? MAP_32BIT : 0;
#    else
  int64_t add_flags = 0;
#    endif
  static int64_t ps;
  int64_t b;
  if (!ps)
    ps = sysconf(_SC_PAGESIZE);
  b = (pags * MEM_PAG_SIZE + ps - 1) & ~(ps - 1);
#    if defined(__riscv__) || defined(__riscv)
  if (hc->is_code_heap || bc_enable)
    at = GetAvailRegion32(b);
#    endif
#    if (defined(__aarch64__) || defined(_M_ARM64)) && !defined(__APPLE__)
  if (bc_enable)
    at = GetAvailRegion32(b);
#    endif
#    if defined(__APPLE__)
  CMemBlk *ret =
      mmap(at, b, (hc->is_code_heap ? PROT_EXEC : PROT_READ) | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS | add_flags |
               (hc->is_code_heap ? MAP_JIT | MAP_NOCACHE : 0),
           -1, 0);
#    else
  CMemBlk *ret =
      mmap(at, b, (hc->is_code_heap ? PROT_EXEC : 0) | PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS | add_flags, -1, 0);
#    endif
#    if defined(__x86_64__)
  if (ret == MAP_FAILED && (add_flags & MAP_32BIT))
    ret = mmap(GetAvailRegion32(b), b,
               (hc->is_code_heap ? PROT_EXEC : 0) | PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS | add_flags, -1, 0);

  // Just give it a 64bit address and hope for the best
  if (ret == MAP_FAILED && (add_flags & MAP_32BIT))
    ret = mmap(NULL, b,
               (hc->is_code_heap ? PROT_EXEC : 0) | PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS | (add_flags & ~MAP_32BIT), -1, 0);
#    endif
  if (ret == MAP_FAILED)
    return NULL;
#  endif
  int64_t threshold = MEM_HEAP_HASH_SIZE >> 4, cnt, something_happened;
  CMemUnused *unm, *tmp, *tmp2, *got;
  QueIns(&ret->base, hc->mem_blks.last);
  ret->pags = pags;
  hc->alloced_u8s += pags * MEM_PAG_SIZE;
#  if defined(__APPLE__)
  if (!code_pages.next) {
    QueInit(&code_pages);
  }
#  endif
  return ret;
}
#endif
static CHeapCtrlArena *PickArena(CHeapCtrl *hc) {
  int64_t a;
again:
  for (a = 0; a != 16; a++) {
    if (!Misc_LBts(&hc->arena_lock, a))
      return &hc->arenas[a];
  }
  PAUSE;
  goto again;
}
static int64_t WhichBucket(int64_t cnt) {
  switch (cnt) {
  default:
  case 64:
    return 0;
    break;
  case 128:
    return 1;
    break;
  case 256:
    return 2;
    break;
  case 512:
    return 3;
    break;
  case 1024:
    return 4;
  }
}
void *__AIWNIOS_MAlloc(int64_t cnt, void *t) {
  if (!cnt)
    return NULL;
  int old_wnp = SetWriteNP(0);
  if (!t)
    t = Fs->heap;
  int64_t orig = cnt;
  cnt += sizeof(CMemUnused);
  if (bc_enable)
    cnt += 16;
  CHeapCtrl *hc = t;
  int64_t pags;
  int64_t which_bucket = 0;
  CMemUnused *ret, *got;
  if (hc->hc_signature != 'H')
    hc = ((CTask *)hc)->heap;
  if (hc->hc_signature != 'H')
    throw('BadMAll');
  CHeapCtrlArena *arena = PickArena(hc);
  // During MemPagTaskAlloc,i splt up the malloc_free_lst into power of 2
  // things,so use power of two to use them
  if (cnt <= MEM_HEAP_HASH_SIZE)
    cnt = NextPower2(cnt);
#ifndef __x86_64__
  if (hc->is_code_heap)
    goto big;
#endif
  if (cnt > MEM_HEAP_HASH_SIZE)
    goto big;
  which_bucket = WhichBucket(cnt);
again:;
  ret = arena->heap_hash[which_bucket];
  if (ret != &new_nul) {
    arena->heap_hash[which_bucket] = ret->next;
    goto almost_done;
  } else {
    ret = arena->malloc_free_lst;
  }
  if (ret == &new_nul) {
  new_lunk:
    while (Misc_LBts(&hc->locked_flags, 1))
      PAUSE;
    ret = MemPagTaskAlloc(
        (pags = ((cnt + 16 * MEM_PAG_SIZE - 1) >> MEM_PAG_BITS)) + 1, hc);
#if defined(__OpenBSD__)
    ret = ((CMemBlk *)ret)->rw;
#endif
    if (!ret) {
      Misc_LBtr(&hc->locked_flags, 2);
      SetWriteNP(old_wnp);
      return NULL;
    }
    ret = (char *)ret + sizeof(CMemBlk);
    ret->sz = (pags << MEM_PAG_BITS) - sizeof(CMemBlk);
    arena->malloc_free_lst = ret;
    Misc_LBtr(&hc->locked_flags, 1);
  }
  // Chip off
  //
  // We must make sure there is room for another CMemUnused
  //
chip_off:;
  ret = arena->malloc_free_lst;
  if (ret->sz >= cnt) {
    CMemUnused *new = (char *)arena->malloc_free_lst + cnt;
    new->sz = ret->sz - cnt;
    ret->sz = cnt;
    arena->malloc_free_lst = new;
    goto almost_done;
  } else {
    goto new_lunk;
  }
big:
  while (Misc_LBts(&hc->locked_flags, 1))
    PAUSE;
  ret = MemPagTaskAlloc(1 + ((cnt + sizeof(CMemBlk)) >> MEM_PAG_BITS), hc);
#if defined(__OpenBSD__)
  ret = ((CMemBlk *)ret)->rw;
#endif
  Misc_LBtr(&hc->locked_flags, 1);
  if (!ret) {
    SetWriteNP(old_wnp);
    return NULL;
  }
  ret = (char *)ret + sizeof(CMemBlk);
  ret->sz = cnt;
  goto almost_done;
almost_done:
  QueIns(&ret->next, &arena->used_next);
  ret->which_bucket = arena - (CHeapCtrlArena *)&hc->arenas;
  hc->used_u8s += cnt;
  ret->hc = hc;
  ret++;
  Misc_LBtr(&hc->arena_lock, arena - (CHeapCtrlArena *)&hc->arenas);
  if (bc_enable) {
    assert((int64_t)ret < (1ll << 31));
    if (orig < 8)
      orig = 8;
    if (orig & 7)
      orig = 8 + orig & ~7ll;
    memset(&bc_good_bitmap[(int64_t)ret / 8], 7, orig / 8);
  }
  SetWriteNP(old_wnp);
#if defined(__OpenBSD__)
  if (hc->is_code_heap) {
    // Translate back to rx if code_heap
    while (Misc_LBts(&hc->locked_flags, 1))
      PAUSE;
    static int64_t ps = 0;
    if (!ps)
      ps = sysconf(_SC_PAGE_SIZE);

    CMemBlk *mm = hc->mem_blks.next;
    while (mm != &hc->mem_blks) {
      if ((char *)mm->rw <= (char *)ret)
        if ((char *)mm->rw + mm->pags * ps > (char *)ret) {
          int64_t off = (char *)ret - (char *)mm->rw;
          ret = (char *)mm->rx + off;
          break;
        }
      mm = mm->base.next;
    }
    Misc_LBtr(&hc->locked_flags, 1);
  }
#endif
  return ret;
}

void __AIWNIOS_Free(void *ptr) {
  CMemUnused *un = ptr, *hash;
  CHeapCtrl *hc;
  int64_t cnt, which_bucket, which_bucket2;
  if (!ptr)
    return;
  int oldwnp = SetWriteNP(0);
  un--;           // Area before ptr is CMemUnused*
  if (un->sz < 0) // Aligned chunks are negative and point to start
    un = un->sz + (char *)un;
  if (!un->hc)
    throw('BadFree');
  if (un->hc->hc_signature != 'H')
    throw('BadFree');
  if (bc_enable) {
    cnt = MSize(ptr);
    assert((int64_t)ptr < (1ll << 31));
    memset(&bc_good_bitmap[(int64_t)ptr / 8], 0, cnt / 8);
  }
  hc = un->hc;
  hc->used_u8s -= un->sz;
  which_bucket = un->which_bucket;
  #ifdef __OpenBSD__
  if (hc->is_code_heap) {
    // Translate back to rx if code_heap
    while (Misc_LBts(&hc->locked_flags, 1))
      PAUSE;
    static int64_t ps = 0;
    if (!ps)
      ps = sysconf(_SC_PAGE_SIZE);

    un = (CMemUnused *)MemGetWritePtr(un);
    Misc_LBtr(&hc->locked_flags, 1);
  }
#endif
  while (Misc_LBts(&hc->arena_lock, which_bucket))
    PAUSE;
  un->hc = NULL;
  QueRem(&un->next);
  if (un->sz <= MEM_HEAP_HASH_SIZE) {
    cnt = un->sz;
    which_bucket2 = WhichBucket(NextPower2(cnt));
    un->next = hc->arenas[which_bucket].heap_hash[which_bucket2];
    hc->arenas[which_bucket].heap_hash[which_bucket2] = un;
  } else {
    // CMemUnused
    // CMemBlk
    // page start
    while (Misc_LBts(&hc->locked_flags, 1))
      PAUSE;
    MemPagTaskFree((char *)(un) - sizeof(CMemBlk), hc);
    Misc_LBtr(&hc->locked_flags, 1);
  }
  Misc_LBtr(&hc->arena_lock, which_bucket);
fin:
  SetWriteNP(oldwnp);
}

int64_t MSize(void *ptr) {
  CMemUnused *un = ptr;
  int64_t cnt;
  if (!ptr)
    return 0;
  int oldwnp = SetWriteNP(0);
  un--;
  if (un->sz < 0) // Aligned chunks are negative and point to start
    un = un->sz + (char *)un;
  if (un->hc->hc_signature != 'H')
    throw('BadFree');
  cnt = un->sz - sizeof(CMemUnused);
  SetWriteNP(oldwnp);
  return cnt;
}

void *__AIWNIOS_CAlloc(int64_t cnt, void *t) {
  void *ret = __AIWNIOS_MAlloc(cnt, t);
  int oldwnp = SetWriteNP(0);
  memset(MemGetWritePtr(ret), 0, cnt);
  SetWriteNP(oldwnp);
  return ret;
}

CHeapCtrl *HeapCtrlInit(CHeapCtrl *ct, CTask *task, int64_t is_code_heap) {
  if (!ct)
    ct = calloc(sizeof(CHeapCtrl), 1);
  if (!task)
    task = Fs;
  ct->is_code_heap = is_code_heap;
  ct->hc_signature = 'H';
  ct->mem_task = task;
  QueInit(&ct->mem_blks);
  int64_t a, b;
  for (a = 0; a != 16; a++) {
    for (b = 0; b < 5; b++)
      ct->arenas[a].heap_hash[b] = &new_nul;
    ct->arenas[a].malloc_free_lst = &new_nul;
    QueInit(&ct->arenas[a].used_next);
  }
#ifdef __OpenBSD__
  ct->buddies = calloc(1, sizeof(CBuddyPair));
  ct->buddies->offset_r = (1ul << 31) / 2;
#endif
  return ct;
}

void HeapCtrlDel(CHeapCtrl *ct) {
  CMemBlk *next, *m;
  int old = SetWriteNP(0);
  while (Misc_Bt(&ct->locked_flags, 1))
    PAUSE;
  for (m = ct->mem_blks.next; m != &ct->mem_blks; m = next) {
    next = m->base.next;
#if defined(_WIN32) || defined(WIN32)
    VirtualFree(m, 0, MEM_RELEASE);
#else
    static int64_t ps;
    int64_t b;
    if (!ps)
      ps = sysconf(_SC_PAGE_SIZE);
    b = (m->pags * MEM_PAG_SIZE + ps - 1) & ~(ps - 1);
    if (m->base2.next)
      QueRem(&m->base2);
    munmap(m, b);
#endif
  }
#if defined(__OpenBSD__)
  if (ct->code_shm_fd)
    close(ct->code_shm_fd);
  OBSD_SexyBuddyDel0(ct->buddies);
#endif
  free(ct);
  SetWriteNP(old);
}

char *__AIWNIOS_StrDup(char *str, void *t) {
  if (!str)
    return NULL;
  int64_t cnt = strlen(str);
  char *ret = A_MALLOC(cnt + 1, t);
  memcpy(ret, str, cnt + 1);
  return ret;
}

#ifdef _WIN64

int64_t IsValidPtr(char *chk) {
  MEMORY_BASIC_INFORMATION mbi;
  if (!VirtualQuery(chk, &mbi, sizeof mbi))
    return 0;
  // https://stackoverflow.com/questions/496034/most-efficient-replacement-for-isbadreadptr
  DWORD mask =
      (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
       PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
  return !!(mbi.Protect & mask);
}

#  define MEM MEM_RESERVE | MEM_COMMIT
#  define VALLOC(addr, sz, memflags, pagflags)                                 \
    VirtualAlloc((void *)addr, sz, memflags, pagflags)
// requirement: popcnt(to) == 1
#  define ALIGNNUM(x, to) ((x + to - 1) & ~(to - 1))

static void *NewVirtualChunk(uint64_t sz, bool low32, bool exec) {
  static bool running, init, topdown;
  static uint64_t ag, cur = 0x10000, max = (1ull << 31) - 1;
  if (!init) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    ag = si.dwAllocationGranularity;
    HANDLE proc = GetCurrentProcess();
    PROCESS_MITIGATION_ASLR_POLICY aslr;
    fp_GetProcessMitigationPolicy_t fptr =
        DynamicFptr_GetProcessMitigationPolicy();
    /* If DEP is disabled, don't let RW pages pile on RWX pages to avoid OOM */
    if (fptr) {
      fptr(proc, ProcessASLRPolicy, &aslr, sizeof aslr);
      if (!aslr.EnableBottomUpRandomization)
        topdown = true;
    } else
      topdown = true;
    init = true;
  }
  void *ret;
  while (Misc_LBts(&running, 0))
    while (Misc_Bt(&running, 0))
      __builtin_ia32_pause();
  if (low32) {
    MEMORY_BASIC_INFORMATION mbi;
    uint64_t region = cur;
    while (region <= max && VirtualQuery(region, &mbi, sizeof mbi)) {
      region = (uint64_t)mbi.BaseAddress + mbi.RegionSize;
      uint64_t addr = ALIGNNUM((uint64_t)mbi.BaseAddress, ag);
      if ((mbi.State & MEM_FREE) && sz <= region - addr) {
        ret = VALLOC(addr, sz, MEM,
                     exec ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE);
        cur = (uint64_t)ret + sz;
        goto ret;
      }
    }
  }
  /* Fallback if we cant grab a 32bit address */
  ret = VALLOC(NULL, sz, MEM | (MEM_TOP_DOWN * topdown),
               exec ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE);
ret:
  Misc_LBtr(&running, 0);
  return ret;
}

#else

static void *Str2Ptr(char *ptr, char **end) {
  int64_t v = 0;
  char c;
  while (isxdigit(*ptr)) {
    v <<= 4;
    switch (c = toupper(*ptr)) {
    case '0' ... '9':
      v += c - '0';
      break;
    case 'A' ... 'F':
      v += c - 'A' + 10;
      break;
    }
    ptr++;
  }
  if (end)
    *end = ptr;
  return (void *)v;
}
#  ifdef __FreeBSD__
// clang-format off
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <libprocstat.h>
#include <kvm.h>
#include <sys/sysctl.h>
#include <sys/user.h>
// clang-format on
typedef struct CRange {
  uint64_t s;
  uint64_t e;
} CRange;
static int64_t RangeSort(CRange *a, CRange *b) {
  return a->s - b->s;
}
static void *GetAvailRegion32(int64_t len) {
  int64_t poop_ant;
  // https://forums.freebsd.org/threads/how-to-get-virtual-memory-size-of-current-process-in-c-c.87022/
  // Thank God
  struct procstat *ps = procstat_open_sysctl();
  void *ret;
  unsigned int proc_cnt, vm_cnt, i;
  // man kvm_getprocs
  struct kinfo_proc *kproc =
      procstat_getprocs(ps, KERN_PROC_PID, getpid(), &proc_cnt);
  struct kinfo_vmentry *vments = procstat_getvmmap(ps, kproc, &vm_cnt);
  CRange ranges[vm_cnt + 2];
  for (i = 0; i != vm_cnt; i++) {
    ranges[i].s = vments[i].kve_start;
    ranges[i].e = vments[i].kve_end;
  }
  // Start range
  ranges[vm_cnt].s = 0x10000;
  ranges[vm_cnt].e = 0x10000;
  ranges[vm_cnt + 1].s = 0x100000000ull / 2; // 32bits(31 forward,31 backwards)
  ranges[vm_cnt + 1].e = 0x100000000ull / 2;
  qsort(ranges, vm_cnt, sizeof(CRange), &RangeSort);
  for (poop_ant = 0; poop_ant != vm_cnt + 1; poop_ant++) {
    if (ranges[poop_ant + 1].s - ranges[poop_ant].e >= len) {
      ret = ranges[poop_ant].e;
      break;
    }
  }
  procstat_freeprocs(ps, kproc);
  procstat_freevmmap(ps, vments);
  procstat_close(ps);
  return ret;
}
#  elif defined(__linux__)
static void *GetAvailRegion32(int64_t len) {
  static int64_t ps;
  if (!ps) {
    ps = sysconf(_SC_PAGESIZE);
  }
  void *last_gap_end = 0x10000;
  void *start;
  void *retv = NULL, *ptr;
  int fd = open("/proc/self/maps", O_RDONLY);
  if (fd == -1)
    return NULL;
  char buf[BUFSIZ];
  ssize_t n;
  while ((n = read(fd, buf, BUFSIZ)) > 0) {
    ssize_t pos = 0;
    while (pos < n) {
      char *ptr = buf + pos;
      start = Str2Ptr(ptr, &ptr);
      if (last_gap_end && start - last_gap_end >= len) {
        retv = last_gap_end;
        goto ret;
      }
      ++ptr;
      last_gap_end = Str2Ptr(ptr, &ptr);
      while (pos < n && buf[pos] != '\n')
        ++pos;
      ++pos;
    }
  }
ret:
  close(fd);
  return ((int64_t)retv + ps - 1) & ~(ps - 1);
}

#  endif // Linux
int64_t IsValidPtr(char *chk) {
  static int64_t ps;
  int64_t mptr = chk;
  if (!ps)
    ps = sysconf(_SC_PAGESIZE);
  mptr &= ~(ps - 1);
  // https://renatocunha.com/2015/12/msync-pointer-validity/
  return -1 != msync(mptr, ps, MS_ASYNC);
}
#endif   // Posix

// Returns good region if good,else NULL and after is set how many bytes OOB
// Returns INVALID_PTR on error
void *BoundsCheck(void *ptr, int64_t *after) {
  if (!bc_enable)
    return INVALID_PTR;
  int64_t waste = 0;
  int64_t optr = ptr;
  ptr = ((int64_t)ptr) & ~7ll;
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
    *after = optr - (int64_t)(ptr + waste) - 8;
  return NULL;
}

int64_t WriteProtectMemCpy(char *dst, char *src, int64_t len) {
#if defined(__OpenBSD__)
  CMemPair *mp = GetMemPairForPtrRX(dst);
  if (mp) {
    return (int64_t *)memcpy(mp->rw + (dst - mp->rx), src, len);
  }
#elif defined(__APPLE__)
  int old = SetWriteNP(0);
  int64_t r = (int64_t)memcpy(dst, src, len);
  SetWriteNP(old);
  return r;
#else
  return (int64_t)memcpy(dst, src, len);
#endif
}

void *MemGetWritePtr(void *ptr) {
#if defined(__OpenBSD__)
  CMemPair *mp = GetMemPairForPtrRX(ptr);
  if (!mp)
    return ptr;
  size_t off = (char *)ptr - mp->rx;
  return mp->rw + off;
#endif
  return ptr;
}
void *MemGetExecPtr(void *ptr) {
#if defined(__OpenBSD__)
  CMemPair *mp = GetMemPairForPtrRW(ptr);
  if (!mp)
    return ptr;
  size_t off = (char *)ptr - mp->rw;
  return mp->rx + off;
#endif
  return ptr;
}
