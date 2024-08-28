#include "aiwn_asm.h"
#include "aiwn_hash.h"
#include "aiwn_mem.h"
#include "aiwn_multic.h"
#include <stddef.h>
#include <stdint.h>
int64_t Bsf(int64_t v) {
  return v ? __builtin_ctzll(v) : -1;
}

int64_t Bsr(int64_t v) {
  return v ? __builtin_popcountll(-2) - __builtin_clzll(v) : -1;
}

char *WhichFun(char *fptr) {
  int64_t idx, best_dist = 0x1000000, dist;
  CHashFun *fun, *best = NULL;
  CHashExport *exp;
  for (idx = 0; idx <= Fs->hash_table->mask; idx++) {
    for (fun = Fs->hash_table->body[idx]; fun; fun = fun->base.base.next) {
      if (fun->base.base.type & HTT_FUN) {
        if ((char *)fun->fun_ptr <= fptr) {
          dist = fptr - (char *)fun->fun_ptr;
          if (dist < best_dist) {
            best = fun;
            best_dist = dist;
          }
        }
      }
      exp = fun;
      if (fun->base.base.type & HTT_EXPORT_SYS_SYM) {
        if ((char *)exp->val <= fptr) {
          dist = fptr - (char *)exp->val;
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

int64_t DoNothing() {
  return 0;
}
