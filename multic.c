#include "aiwn.h"
#include <SDL2/SDL.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
typedef struct {
  void (*fp)(),*gs;
  int64_t num;
} CorePair;
typedef struct {
  int64_t is_sleeping;
  pthread_t pt;
  int wake_futex;
} CCPU;
static CCPU cores[128];
static __thread core_num=0;
static void threadrt(CorePair *pair) {
  SetHolyGs(pair->gs);
  core_num=pair->num;
  void (*fp)();
  fp=pair->fp;
  free(pair);
  fp();
}
void SpawnCore(void (*fp)(),void *gs,int64_t core) {
  char buf[144];
  CorePair pair={fp,gs,core},*ptr=malloc(sizeof(CorePair));
  *ptr=pair;
  cores[core].is_sleeping=0;
  sprintf(buf,"Core:%d",core);
  SDL_CreateThread(&threadrt,buf,ptr);
}
int64_t mp_cnt() {
  static int64_t ret=0;
  if(!ret)
    ret=sysconf(_SC_NPROCESSORS_CONF);
  return ret;
}

void MPSleepHP(int64_t ns) {
  struct timespec ts={0};
  ts.tv_nsec+=ns*1000U;
  syscall(SYS_futex,&cores[core_num].wake_futex,0,FUTEX_WAIT,&ts,NULL,0);
  cores[core_num].wake_futex=0;
}

void MPAwake(int64_t core) {
  if(!LBts(&cores[core].wake_futex,0))
    syscall(SYS_futex,&cores[core].wake_futex,1,FUTEX_WAKE,NULL,NULL,0);
  
}
