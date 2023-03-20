#include "aiwn.h"
#include <pthread.h>
#include <unistd.h>
typedef struct {
  void (*fp)(),*gs;
} CorePair;
static void threadrt(CorePair *pair) {
  SetHolyGs(pair->gs);
  void (*fp)();
  fp=pair->fp;
  free(pair);
  fp();
}
void SpawnCore(void (*fp)(),void *gs) {
  CorePair pair={fp,gs},*ptr=malloc(sizeof(CorePair));
  *ptr=pair;
  pthread_t dummy;
  pthread_create(&dummy,NULL,threadrt,ptr);
}

int64_t mp_cnt() {
  static int64_t ret=0;
  if(!ret)
    ret=sysconf(_SC_NPROCESSORS_CONF);
  return ret;
}
