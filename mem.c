#pragma once
#include "aiwn.h"
#include <stdint.h>
#include <sys/mman.h>
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
	munmap(blk, blk->pags * MEM_PAG_SIZE);
}
static CMemBlk* MemPagTaskAlloc(int64_t pags, CHeapCtrl* hc)
{
	if (!hc)
		hc = Fs->heap;
	CMemBlk* ret = mmap(NULL, pags * MEM_PAG_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	int64_t threshold = MEM_HEAP_HASH_SIZE >> 4, cnt;
	CMemUnused **unm, *tmp, *tmp2;
	if (!ret || ret == MAP_FAILED)
		return NULL;
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

void* __AIWNIOS_MAlloc(int64_t cnt, void* t)
{
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
	// TODO atomic
	while (hc->locked_flags & HClF_LOCKED)
		;
	hc->locked_flags |= HClF_LOCKED;
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
			pags = ((cnt + 16 * MEM_PAG_SIZE - 1) >> MEM_PAG_BITS), hc);
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
	// TODO atomic
	hc->locked_flags &= ~HClF_LOCKED;
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
	// TODO atomic
	while (hc->locked_flags & HClF_LOCKED)
		;
	hc->locked_flags |= HClF_LOCKED;
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
	hc->locked_flags &= ~HClF_LOCKED;
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
	while (ct->locked_flags & HClF_LOCKED)
		;
	ct->locked_flags |= HClF_LOCKED;
	for (m = ct->mem_blks.next; m != &ct->mem_blks; m = next) {
		next = m->base.next;
		munmap(m, m->pags * MEM_PAG_SIZE);
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
