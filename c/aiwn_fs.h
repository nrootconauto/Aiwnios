#pragma once
#include <stdbool.h>
#include <stdio.h>
char *FileRead(char *fn, int64_t *sz);

void VFsThrdInit();
void VFsSetDrv(char d);
int64_t VFsDel(char *p);
int64_t VFsUnixTime(char *name);
int64_t VFsFSize(char *name);
int64_t VFsFileWrite(char *name, char *data, int64_t len);
int64_t VFsFileRead(char *name, int64_t *len);
int VFsFileExists(char *path);
int VFsMountDrive(char let, char *path);
int VFsFOpen(char *path, bool b);
void VFsFClose(int fd);
int64_t VFsTrunc(char *fn, int64_t sz);
int64_t VFsFBlkRead(void *d, int64_t sz, int fd);
int64_t VFsFBlkWrite(void *d, int64_t sz, int fd);
int64_t VFsFSeek(int64_t off, int fd);
void VFsSetPwd(char *pwd);
int64_t VFsDirMk(char *f);
char **VFsDir(void);
int64_t VFsIsDir(char *name);

#include "generated.h" //AIWNIOS_INSTALL_DIRAIWNIOS_getcontext,
#if defined(_WIN32) || defined(WIN32)
#  define AIWNIOS_OSTREAM      stdout
#  define AIWNIOS_TEMPLATE_DIR "TODO"
#else
#  define AIWNIOS_OSTREAM      stderr
#  define AIWNIOS_TEMPLATE_DIR AIWNIOS_INSTALL_DIR "/share/aiwnios"
#endif
const char *ResolveBootDir(char *use, int make_new_dir, const char *t);
