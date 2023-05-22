#pragma once
#include "aiwn.h"
#include <ctype.h>
uint64_t ToUpper(uint64_t ch)
{
	char arr[8];
	int64_t i;
	memcpy(arr, &ch, 8);
	for (i = 0; i != 8; i++)
		arr[i] = toupper(arr[i]);
	memcpy(&ch, arr, 8);
	return ch;
}
int64_t Bsf(int64_t v)
{
	#if defined(_WIN32) || defined(WIN32)
	return __builtin_ffsll(v) - 1;
	#else
	return __builtin_ffsl(v) - 1;
	#endif
}
int64_t Bsr(int64_t v)
{
	if (!v)
		return -1;
	#if defined(_WIN32) || defined(WIN32)
	return 63 - __builtin_clzll(v);
	#else
	return 63 - __builtin_clzl(v);
	#endif
}
char* WhichFun(char* fptr)
{
	int64_t idx, best_dist = 0x1000000, dist;
	CHashFun *fun, *best = NULL;
	CHashExport* exp;
	for (idx = 0; idx <= Fs->hash_table->mask; idx++) {
		for (fun = Fs->hash_table->body[idx]; fun; fun = fun->base.base.next) {
			if (fun->base.base.type & HTT_FUN) {
				if ((char*)fun->fun_ptr <= fptr) {
					dist = fptr - (char*)fun->fun_ptr;
					if (dist < best_dist) {
						best = fun;
						best_dist = dist;
					}
				}
			}
			exp = fun;
			if (fun->base.base.type & HTT_EXPORT_SYS_SYM) {
				if ((char*)exp->val <= fptr) {
					dist = fptr - (char*)exp->val;
					if (dist < best_dist) {
						best = exp;
						best_dist = dist;
					}
				}
			}
		}
	}
	if (best)
		return best->base.base.str;
	return NULL;
}
