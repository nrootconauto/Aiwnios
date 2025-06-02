#include "aiwn_asm.h"
static int *b(char *ptr, int64_t *_b) {
  int64_t b = *_b;
  int64_t align = -((uint64_t)ptr % sizeof(int));
  ptr += align;
  b -= 8 * align;
  ptr = (int *)ptr + b / (8 * sizeof(int));
  if (_b)
    *_b = b % (8 * sizeof(int));
  return (int *)ptr;
}
int64_t Bt(void *_ptr, int64_t bit) {
  int *ptr = _ptr;
  ptr = b(ptr, &bit);
  int mask = 1 << bit;
  return !!(__atomic_load_n(ptr, __ATOMIC_SEQ_CST) & mask);
}
int64_t Btc(void *_ptr, int64_t bit) {
  int *ptr = _ptr;
  ptr = b(ptr, &bit);
  int mask = 1 << bit;
  int ret = !!(*ptr & mask);
  *ptr ^= mask;
  return ret;
}
int64_t Btr(void *_ptr, int64_t bit) {
  int *ptr = _ptr;
  ptr = b(ptr, &bit);
  int mask = 1 << bit;
  int ret = !!(*ptr & mask);
  *ptr &= ~mask;
  return ret;
}
int64_t Bts(void *_ptr, int64_t bit) {
  int *ptr = _ptr;
  ptr = b(ptr, &bit);
  int mask = 1 << bit;
  int ret = !!(*ptr & mask);
  *ptr |= mask;
  return ret;
}
int64_t LBt(void *_ptr, int64_t bit) {
  int *ptr = _ptr;
  ptr = b(ptr, &bit);
  int mask = 1 << bit;
  return !!(__atomic_load_n(ptr, __ATOMIC_SEQ_CST) & mask);
}

int64_t LBtc(void *_ptr, int64_t bit) {
  int *ptr = _ptr;
  ptr = b(ptr, &bit);
  int mask = 1 << bit;
  int ret = __atomic_fetch_xor(ptr, mask, __ATOMIC_SEQ_CST);
  return !!(ret & mask);
}
int64_t LBtr(void *_ptr, int64_t bit) {
  int *ptr = _ptr;

  ptr = b(ptr, &bit);
  int mask = 1 << bit;
  int ret = __atomic_fetch_and(ptr, ~mask, __ATOMIC_SEQ_CST);
  return !!(ret & mask);
}
int64_t LBts(void *_ptr, int64_t bit) {
  int *ptr = _ptr;
  ptr = b(ptr, &bit);
  int mask = 1 << bit;
  int ret = __atomic_fetch_or(ptr, mask, __ATOMIC_SEQ_CST);
  return !!(ret & mask);
}
