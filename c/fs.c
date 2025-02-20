#include "aiwn_except.h"
#include "aiwn_fs.h"
#include "aiwn_mem.h"
#include <SDL_messagebox.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// clang-format off
#include <sys/types.h>
#include <sys/stat.h>
// clang-format on
#include <fcntl.h>
#include <unistd.h>
//__builtin_stpcpy was reducing down to stpcpy on my windows machine
static char *StPCpy(char *a, const char *b) {
  size_t i = strlen(b);
  return memcpy(a, b, i + 1) + i;
}
#ifndef _WIN64
#  ifndef O_BINARY
#    define O_BINARY 0
#  endif
#else
#  include <windows.h>
#  include <direct.h>
#  include <io.h>
#  include <libloaderapi.h>
#  include <processthreadsapi.h>
#  include <shlwapi.h>
#  include <synchapi.h>
#  define open(a...)  _open(a)
#  define close(a...) _close(a)
#  define write(a...) _write(a)
#  define mkdir(a, b) _mkdir(a)
#  define lseek(a...) _lseeki64(a)

static void MakePathSane(char *ptr) {
  char *ptr2 = ptr;
enter:
  while (*ptr) {
    if (*ptr == '/' && ptr[1] == '/') {
      *ptr2++ = *ptr++;
      while (*ptr == '/')
        ptr++;
      goto enter;
    }
    if (ptr != ptr2)
      *ptr2++ = *ptr;
    else
      ptr2++;
    if (*ptr)
      ptr++;
  }
  *ptr2 = 0;
}
#endif

static char mount_points['z' - 'a' + 1][0x200];
_Thread_local char thrd_pwd[0x200];
_Thread_local char thrd_drv;

static char *__VFsFileNameAbs(char *name) {
  char ret[0x200], *cur;
  cur = StPCpy(ret, mount_points[toupper(thrd_drv) - 'A']);
  cur = StPCpy(cur, thrd_pwd);
  if (name) {
    cur = StPCpy(cur, "/");
    cur = StPCpy(cur, name);
  }
  return strdup(ret);
}

static int __FExists(char *path) {
#ifdef _WIN64
  return PathFileExistsA(path);
#else
  return !access(path, F_OK);
#endif
}

static int __FIsDir(char *path) {
#ifdef _WIN64
  return PathIsDirectoryA(path);
#else
  struct stat s;
  stat(path, &s);
  return S_ISDIR(s.st_mode);
#endif
}

char *FileRead(char *fn, int64_t *sz) {
  int fd = open(fn, O_BINARY | O_RDONLY);
  if (-1 == fd) {
    if (sz)
      *sz = 0;
    return A_CALLOC(8, 0);
  }
  struct stat st;
  fstat(fd, &st);
  char *ret = A_MALLOC(st.st_size + 1, NULL);
  if (!ret) {
    if (sz)
      *sz = 0;
    return A_CALLOC(8, 0);
  }
  read(fd, ret, st.st_size);
  ret[st.st_size] = 0;
  close(fd);
  if (sz)
    *sz = st.st_size;
  return ret;
}

void VFsThrdInit() {
  strcpy(thrd_pwd, "/");
  thrd_drv = 'T';
}

void VFsSetDrv(char d) {
  if (!isalpha(d))
    return;
  thrd_drv = toupper(d);
}

static bool VFsCd(char *to, bool make) {
  to = __VFsFileNameAbs(to);
  if (__FExists(to) && __FIsDir(to))
    goto success;
  else if (make) {
    mkdir(to, 0700);
    goto success;
  }
  return false;
success:
  free(to);
  return true;
}

static void DelDir(char *p) {
  DIR *d = opendir(p);
  if (!d)
    return;
  struct dirent *d2;
  char od[0x200];
  while (d2 = readdir(d)) {
    if (!strcmp(".", d2->d_name) || !strcmp("..", d2->d_name))
      continue;
    char *p = StPCpy(od, p);
    *p++ = '/';
    strcpy(p, d2->d_name);
    if (__FIsDir(od)) {
      DelDir(od);
    } else {
      remove(od);
    }
  }
  closedir(d);
  rmdir(p);
}

int VFsFOpen(char *f, bool b) {
  char *path = __VFsFileNameAbs(f);
  int fd = open(path, O_BINARY | (b ? O_RDWR | O_CREAT : O_RDONLY), 0666);
  free(path);
  return fd;
}

void VFsFClose(int fd) {
  close(fd);
}

int64_t VFsDel(char *p) {
  int r;
  p = __VFsFileNameAbs(p);
  if (!__FExists(p)) {
    free(p);
    return 0;
  }
  if (__FIsDir(p)) {
    DelDir(p);
  } else
    remove(p);
  r = !__FExists(p);
  free(p);
  return r;
}

int64_t VFsUnixTime(char *name) {
  char *fn = __VFsFileNameAbs(name);
  struct stat s;
  stat(fn, &s);
  free(fn);
  return s.st_mtime;
}

#ifdef _WIN64

int64_t VFsFSize(char *name) {
  char *fn = __VFsFileNameAbs(name), *delim;
  int64_t s64;
  int32_t h32;
  if (!fn)
    return 0;
  if (!__FExists(fn)) {
    free(fn);
    return 0;
  }
  if (__FIsDir(fn)) {
    WIN32_FIND_DATAA data;
    HANDLE dh;
    char buffer[strlen(fn) + 4];
    strcpy(buffer, fn);
    strcat(buffer, "/*");
    MakePathSane(buffer);
    while (delim = strchr(buffer, '/'))
      *delim = '\\';
    s64 = 0;
    dh = FindFirstFileA(buffer, &data);
    do
      s64++;
    while (FindNextFileA(dh, &data));
    free(fn);
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
    if (dh != INVALID_HANDLE_VALUE)
      FindClose(dh);
    return s64;
  }
  HANDLE fh = CreateFileA(fn, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                          FILE_FLAG_BACKUP_SEMANTICS, NULL);
  s64 = GetFileSize(fh, &h32);
  s64 |= (int64_t)h32 << 32;
  free(fn);
  CloseHandle(fh);
  return s64;
}

char **VFsDir(void) {
  char *fn = __VFsFileNameAbs(""), **ret = NULL, *delim;
  if (!fn)
    return A_CALLOC(8, NULL); // Just an empty array
  if (!__FExists(fn) || !__FIsDir(fn)) {
    free(fn);
    return A_CALLOC(8, NULL); // Just an empty array
  }
  int64_t sz = VFsFSize("");
  if (sz) {
#  ifdef _WIN64
    //+1 for "."
    ret = A_CALLOC((sz + 1 + 1) * 8, NULL);
#  else
    ret = A_CALLOC((sz + 1) * 8, NULL);
#  endif
    WIN32_FIND_DATAA data;
    HANDLE dh;
    char buffer[strlen(fn) + 4];
    strcpy(buffer, fn);
    strcat(buffer, "/*");
    MakePathSane(buffer);
    while (delim = strchr(buffer, '/'))
      *delim = '\\';
    int64_t s64 = 0;
    dh = FindFirstFileA(buffer, &data);
    while (FindNextFileA(dh, &data)) {
      // CDIR_FILENAME_LEN  is 38(includes nul terminator)
      if (strlen(data.cFileName) <= 37)
        ret[s64++] = A_STRDUP(data.cFileName, NULL);
    }
#  ifdef _WIN64
    ret[s64++] = A_STRDUP(".", NULL);
#  endif
    free(fn);
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
    if (dh != INVALID_HANDLE_VALUE)
      FindClose(dh);
  }
  return ret;
}

#else

char **VFsDir(void) {
  int64_t sz;
  char **ret;
  char *fn = __VFsFileNameAbs(""), *end_fn = fn + strlen(fn);
  while (*--end_fn == '/')
    *end_fn = 0;
  DIR *dir = opendir(fn);
  if (!dir) {
    free(fn);
    return A_CALLOC(8, 0);
  }
  struct dirent *ent;
  sz = 0;
  while (ent = readdir(dir))
    sz++;
  rewinddir(dir);
  ret = A_MALLOC((sz + 1) * sizeof(char *), NULL);
  sz = 0;
  while (ent = readdir(dir)) {
    // CDIR_FILENAME_LEN  is 38(includes nul terminator)
    if (strlen(ent->d_name) <= 37)
      ret[sz++] = A_STRDUP(ent->d_name, NULL);
  }
  ret[sz] = 0;
  free(fn);
  closedir(dir);
  return ret;
}

int64_t VFsFSize(char *name) {
  struct stat s;
  long cnt;
  DIR *d;
  struct dirent *de;
  char *fn = __VFsFileNameAbs(name);
  if (!__FExists(fn)) {
    free(fn);
    return -1;
  } else if (__FIsDir(fn)) {
    d = opendir(fn);
    if (!d) {
      free(fn);
      return 0;
    }
    cnt = 0;
    while (de = readdir(d))
      if (strcmp(de->d_name, ".") && strcmp(de->d_name, ".."))
        cnt++;
    closedir(d);
    free(fn);
    return cnt;
  }
  stat(fn, &s);
  free(fn);
  return s.st_size;
}
#endif

int64_t VFsFileWrite(char *name, char *data, int64_t len) {
  if (!name)
    return 0;
  name = __VFsFileNameAbs(name);
  int fd = open(name, O_BINARY | O_CREAT | O_WRONLY, 0644), res;
  free(name);
  if (-1 != fd) {
    res = write(fd, data, len);
    close(fd);
  } else
    throw(*(uint64_t *)"OpenFile");
  return !!res;
}

int64_t VFsIsDir(char *name) {
  int64_t ret;
  name = __VFsFileNameAbs(name);
  if (!name)
    return 0;
  ret = __FIsDir(name);
  free(name);
  return ret;
}

int64_t VFsFileRead(char *name, int64_t *len) {
  if (!name) {
    if (len)
      *len = 0;
    return 0;
  }
  name = __VFsFileNameAbs(name);
  if (!__FExists(name) || __FIsDir(name)) {
    free(name);
    if (len)
      *len = 0;
    return 0;
  }
  return (int64_t)FileRead(name, len);
}

int VFsFileExists(char *path) {
  if (!path)
    return 0;
  path = __VFsFileNameAbs(path);
  int e = __FExists(path);
  free(path);
  return e;
}

int VFsMountDrive(char let, char *path) {
  int idx = toupper(let) - 'A';
  char *p = StPCpy(mount_points[idx], path);
  strcpy(p, "/");
}

int64_t VFsFBlkRead(void *d, int64_t sz, int f) {
  return sz == read(f, d, sz);
}

int64_t VFsFBlkWrite(void *d, int64_t sz, int f) {
  return sz == write(f, d, sz);
}

int64_t VFsFSeek(int64_t off, int f) {
  if (off == -1)
    return -1 != lseek(f, 0, SEEK_END);
  return -1 != lseek(f, off, SEEK_SET);
}

int64_t VFsTrunc(char *fn, int64_t sz) {
  fn = __VFsFileNameAbs(fn);
  if (!fn)
    return 0;
  int fail = truncate(fn, sz);
  free(fn);
  return 1;
}

void VFsSetPwd(char *pwd) {
  if (!pwd)
    pwd = "/";
  strcpy(thrd_pwd, pwd);
}

int64_t VFsDirMk(char *f) {
  return VFsCd(f, 1);
}

// Creates a virtual drive by a template
static void CopyDir(char *dst, char *src) {
#ifdef _WIN64
  char delim = '\\';
#else
  char delim = '/';
#endif
  if (!__FExists(dst))
    mkdir(dst, 0700);
  char buf[1024], sbuf[1024], *s, buffer[0x10000];
  int64_t root, sz, sroot, r;
  strcpy(buf, dst);
  buf[root = strlen(buf)] = delim;
  buf[++root] = 0;

  strcpy(sbuf, src);
  sbuf[sroot = strlen(sbuf)] = delim;
  sbuf[++sroot] = 0;

  DIR *d = opendir(src);
  if (!d)
    return;
  struct dirent *ent;
  while (ent = readdir(d)) {
    if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
      continue;
    buf[root] = 0;
    sbuf[sroot] = 0;
    strcat(buf, ent->d_name);
    strcat(sbuf, ent->d_name);
    if (__FIsDir(sbuf)) {
      CopyDir(buf, sbuf);
    } else {
      FILE *read = fopen(sbuf, "rb"), *write = fopen(buf, "wb");
      if (!read || !write) {
        if (read)
          fclose(read);
        if (write)
          fclose(write);
        continue;
      }
      while (r = fread(buffer, 1, sizeof(buffer), read)) {
        if (r < 0)
          break;
        fwrite(buffer, 1, r, write);
      }
      fclose(read);
      fclose(write);
    }
  }
  closedir(d);
}

static int __FIsNewer(char *fn, char *fn2) {
#ifndef _WIN64
  struct stat s, s2;
  stat(fn, &s), stat(fn2, &s2);
  int64_t r = mktime(localtime(&s.st_ctime)),
          r2 = mktime(localtime(&s2.st_ctime));
  return r > r2;
#else
  uint64_t s64, s64_2;
  FILETIME t;
  HANDLE fh = CreateFileA(fn, GENERIC_READ, FILE_SHARE_READ, NULL,
                          OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL),
         fh2 = CreateFileA(fn2, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
  GetFileTime(fh, NULL, NULL, &t);
  s64 = (uint64_t)t.dwLowDateTime | ((uint64_t)t.dwHighDateTime << 32);
  GetFileTime(fh2, NULL, NULL, &t);
  s64_2 = (uint64_t)t.dwLowDateTime | ((uint64_t)t.dwHighDateTime << 32);
  CloseHandle(fh), CloseHandle(fh2);
  return s64 > s64_2;
#endif
}

int64_t IsCmdLineMode();

#define DUMB_MESSAGE(FMT, ...)                                                 \
  do {                                                                         \
    int64_t l = snprintf(NULL, 0, FMT, __VA_ARGS__);                           \
    char buffer3[l];                                                           \
    snprintf(buffer3, l, FMT, __VA_ARGS__);                                    \
    if (!IsCmdLineMode()) {                                                    \
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Aiwnios", buffer3, \
                               NULL);                                          \
    } else {                                                                   \
      fprintf(AIWNIOS_OSTREAM, "%s", buffer3);                                 \
    }                                                                          \
  } while (0)
int CreateTemplateBootDrv(char *to, char *template) {
  char buffer[1024], drvl[16], buffer2[1024];
  if (!__FExists(template)) {
    DUMB_MESSAGE(
        "Template directory %s doesn't exist. You probably didnt install "
        "Aiwnios\n",
        template);
    return 0;
  }
  if (__FExists(to)) {
    int64_t _try;
    for (_try = 0; _try != 0x10000; _try++) {
      sprintf(buffer, "%s_BAKCUP.%ld", to, _try);
      if (!__FExists(buffer)) {
        DUMB_MESSAGE(
            "Newer Template drive found,backing up old drive to \"%s\".\n",
            buffer);
// Rename the old boot drive to something else
#ifdef _WIN64
        MoveFile(to, buffer);
#else
        rename(to, buffer);
#endif
        break;
      }
    }
  }
#ifdef _WIN64
  strcpy(buffer2, template);
  strcat(buffer2, "\\");
#else
  strcpy(buffer2, template);
#endif
  if (!__FExists(buffer2)) {
    fprintf(AIWNIOS_OSTREAM,
            "Use \"./aiwnios -t T\" to specify the T drive.\n");
    return 0;
  }
  if (!__FExists(to)) {
    char *next, *old;
    char delim;
    strcpy(buffer, to);
    old = buffer;
#ifdef _WIN64
    const char fdelim = '\\';
#else
    const char fdelim = '/';
#endif
    while (next = strchr(old, fdelim)) {
      delim = *next;
      *next = 0;
      mkdir(buffer, 0700);
      *next = delim;
      old = next + 1;
    }
    if (!__FExists(buffer)) {
      mkdir(buffer, 0700);
    }
    CopyDir(to, buffer2);
    return 1;
  }
  return 0;
}

const char *ResolveBootDir(char *use, int make_new_dir,
                           const char *template_dir) {
  if (!make_new_dir && __FExists("HCRT2.BIN"))
    return ".";
  if (!make_new_dir && __FExists("T/HCRT2.BIN"))
    return "T";
  if (__FExists(use) && !make_new_dir) {
    if (template_dir)
      if (__FIsNewer(template_dir, use))
        goto new;
    ;
    return strdup(use);
  }
  new :;
  // CreateTemplateBootDrv will return existing boot dir if missing
  if (!CreateTemplateBootDrv(use, template_dir)) {
  fail:
    fprintf(AIWNIOS_OSTREAM, "I don't know where the HCRT2.BIN is!!!\n");
    fprintf(AIWNIOS_OSTREAM, "Use \"aiwnios -b\" in the root of the source "
                             "directory to build a boot binary.\n");
    fprintf(AIWNIOS_OSTREAM, "Or Use \"aiwnios -n\" to make a new boot "
                             "drive(if installed on your system).\n");
    exit(-1);
  }
  return use;
}
