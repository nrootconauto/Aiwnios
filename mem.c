#pragma once
#include "aiwn.h"
#include <stdint.h>
#if defined(_WIN32) || defined(WIN32)
#include <sysinfoapi.h>
#include <memoryapi.h>
#else
#include <sys/mman.h>
#endif
//
// Dec 8, I volenterreed today
//
struct CTask;
struct CMemBlk;
struct CMemUnused;
static void MemPagTaskFree(CMemBlk* blk, CHeapCtrl* hc)
{
	QueRem(blk);
	hc->alloced_u8s -= blk->pags * MEM_PAG_SIZE;
	#if defined(_WIN32) || defined(WIN32)
	VirtualFree(blk,blk->pags*MEM_PAG_SIZE,MEM_RELEASE);
	#else
	static int64_t ps;
	int64_t b;
	if(!ps)
		ps=sysconf(_SC_PAGE_SIZE);
	b=blk->pags*MEM_PAG_SIZE;
	if(b%ps) b+=ps;
	b/=ps;
	b*=ps;
	munmap(blk, b);
	#endif
}
static CMemBlk* MemPagTaskAlloc(int64_t pags, CHeapCtrl* hc)
{
	if (!hc)
		hc = Fs->heap;
	#if defined(_WIN32) || defined(WIN32)
	static int64_t dwAllocationGranularity;
    if (!dwAllocationGranularity) {
      SYSTEM_INFO si;
      GetSystemInfo(&si);
      dwAllocationGranularity = si.dwAllocationGranularity;
    }
    int64_t b=(pags*MEM_PAG_SIZE)/dwAllocationGranularity*dwAllocationGranularity;
    if((pags*MEM_PAG_SIZE)%dwAllocationGranularity)
		b+=dwAllocationGranularity;
	CMemBlk *ret=VirtualAlloc(NULL,b,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
	if(!ret) return NULL;
	#else
	static int64_t ps;
	int64_t b;
	if(!ps) {
		ps=sysconf(_SC_PAGE_SIZE);
	}
	b=pags*MEM_PAG_SIZE;
	if(b%ps) b+=ps;
	b/=ps;
	b*=ps;
	CMemBlk* ret = mmap(NULL, b, PROT_EXEC | PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (!ret || ret == MAP_FAILED)
		return NULL;
	#endif
	int64_t threshold = MEM_HEAP_HASH_SIZE >> 4, cnt;
	CMemUnused **unm, *tmp, *tmp2;
	QueIns(&ret->base, hc->mem_blks.last);
	ret->pags = pags;
	// Move silly willies malloc_free_lst,in to the heap_hash
	do {
		cnt = 0;
		unm = &hc->malloc_free_lst;
		while (tmp = *unm) {
			if (tmp->sz < threshold) {
				*unm = tmp->next;
				tmp->next = hc->heap_hash[tmp->sz / 8];
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
void* __AIWNIOS_MAlloc(int64_t cnt, void* t)
{
	if(!cnt) return NULL;
	cnt+=16;
	if (!t)
		t = Fs->heap;
	CHeapCtrl* hc = t;
	int64_t pags;
	CMemUnused* ret;
	if (hc->hc_signature != 'H')
		hc = ((CTask*)hc)->heap;
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
		ret = hc->heap_hash[cnt / 8];
		hc->heap_hash[cnt / 8] = ret->next;
		goto almost_done;
	} else
		ret = hc->malloc_free_lst;
	if (!ret) {
	new_lunk:
		// Make a new lunk
		ret = MemPagTaskAlloc(
			(pags = ((cnt + 16 * MEM_PAG_SIZE - 1) >> MEM_PAG_BITS)) + 1, hc);
		ret = (char*)ret + sizeof(CMemBlk);
		ret->sz = (pags << MEM_PAG_BITS) - sizeof(CMemBlk);
		hc->malloc_free_lst = ret;
	}
	// Chip off
	//
	// We must make sure there is room for another CMemUnused
	//
	if ((int64_t)(ret->sz - sizeof(CMemUnused)) >= cnt) {
		hc->malloc_free_lst = (char*)ret + cnt;
		hc->malloc_free_lst->sz = ret->sz - cnt;
		ret->sz = cnt;
		goto almost_done;
	} else
		goto new_lunk;
big:
	ret = MemPagTaskAlloc(1 + ((cnt + sizeof(CMemBlk)) >> MEM_PAG_BITS), hc);
	ret = (char*)ret + sizeof(CMemBlk);
	ret->sz = cnt;
	goto almost_done;
almost_done:
	hc->used_u8s += cnt;
	Misc_LBtr(&hc->locked_flags, 1);
	ret->hc = hc;
	ret++;
	return ret;
}

void __AIWNIOS_Free(void* ptr)
{
	CMemUnused *un = ptr, *hash;
	CHeapCtrl* hc;
	int64_t cnt;
	if (!ptr)
		return;
	un--; // Area before ptr is CMemUnused*
	if (!un->hc)
		throw('BadFree');
	if (un->hc->hc_signature != 'H')
		throw('BadFree');
	if (un->sz < 0) // Aligned chunks are negative and point to start
		un = un->sz + (char*)un;
	hc = un->hc;
	while (Misc_LBts(&hc->locked_flags, 1))
		;
	hc->used_u8s -= un->sz;
	if (un->sz <= MEM_HEAP_HASH_SIZE) {
		hash = hc->heap_hash[un->sz / 8];
		un->next = hash;
		hc->heap_hash[un->sz / 8] = un;
		un->hc = NULL;
	} else {
		// CMemUnused
		// CMemBlk
		// page start
		MemPagTaskFree((char*)(un) - sizeof(CMemBlk), hc);
	}
fin:
	Misc_LBtr(&hc->locked_flags, 1);
}

int64_t MSize(void* ptr)
{
	CMemUnused* un = ptr;
	int64_t cnt;
	if (!ptr)
		return 0;
	un--;
	if (un->hc->hc_signature != 'H')
		throw('BadFree');
	if (un->sz < 0) // Aligned chunks are negative and point to start
		un = un->sz + (char*)un;
	return un->sz - sizeof(CMemUnused);
}

void* __AIWNIOS_CAlloc(int64_t cnt, void* t)
{
	return memset(__AIWNIOS_MAlloc(cnt, t), 0, cnt);
}

CHeapCtrl* HeapCtrlInit(CHeapCtrl* ct, CTask* task)
{
	if (!ct)
		ct = calloc(sizeof(CHeapCtrl), 1);
	if (!task)
		task = Fs;
	ct->hc_signature = 'H';
	ct->mem_task = task;
	ct->malloc_free_lst = NULL;
	QueInit(&ct->mem_blks);
	return ct;
}

void HeapCtrlDel(CHeapCtrl* ct)
{
	CMemBlk *next, *m;
	// TODO atomic
	while (Misc_Bt(&ct->locked_flags, 1))
		;
	for (m = ct->mem_blks.next; m != &ct->mem_blks; m = next) {
		next = m->base.next;
		#if defined(_WIN32) || defined(WIN32)
		VirtualFree(m,m->pags*MEM_PAG_SIZE,MEM_RELEASE);
		#else
		static int64_t ps;
		int64_t b;
		if(!ps)
			ps=sysconf(_SC_PAGE_SIZE);
		b=m->pags*MEM_PAG_SIZE;
		if(b%ps) b+=ps;
		b/=ps;
		b*=ps;
		munmap(m, b);
		#endif
	}
	free(ct);
}

char* __AIWNIOS_StrDup(char* str, void* t)
{
	if (!str)
		return NULL;
	int64_t cnt = strlen(str);
	char* ret = A_MALLOC(cnt + 1, t);
	memcpy(ret, str, cnt + 1);
	return ret;
}
#if defined(_WIN32) || defined(WIN32)
int64_t IsValidPtr(char *chk) {
  MEMORY_BASIC_INFORMATION mbi;
  memset(&mbi, 0, sizeof mbi);
  if (VirtualQuery(chk, &mbi, sizeof(mbi))) {
    // https://stackoverflow.com/questions/496034/most-efficient-replacement-for-isbadreadptr
    DWORD mask =
        (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
         PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
    int64_t b = !!(mbi.Protect & mask);
    return b;
  }
  return 0;
}
#else
static int64_t Hex2I64(char* s, char** end)
{
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
int64_t IsValidPtr(char *chk)
{
	  static int64_t ps;
	  int64_t mptr=chk;
	  if (!ps)
		ps = getpagesize();
	  mptr /= ps;
	  mptr *= ps;
	  // https://renatocunha.com/2015/12/msync-pointer-validity/
	  return -1 != msync(mptr, ps, MS_ASYNC);
	int64_t ok = 0, sz = 0x1000;
	char buffer[0x1000];
	char* ptr = buffer;
	char *start, *end;
	FILE* f = fopen("/proc/self/maps", "rb");
	while (-1 != getline(&ptr, &sz, f)) {
		start = Hex2I64(ptr, &ptr);
		if (*ptr++ != '-')
			break;
		end = Hex2I64(ptr, &ptr);
		if (start <= chk) {
			if (chk < end) {
				if (*ptr++ != ' ')
					break;
				while (*ptr++ != ' ')
					if (*ptr == 'w') {
						ok = 1;
						break;
					}
				break;
			}
		} else if (start > chk) // These appear to be sorted
			break;
		ptr = buffer;
		sz = 0x1000;
	}
	fclose(f);
	return ok;
}
#endif
