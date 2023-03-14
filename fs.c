#include "aiwn.h"
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
static int __FExists(char *path) {
    return access(path,F_OK)==0;
}
static int __FIsDir(char *path) {
    struct stat s;
    stat(path,&s);
    return (s.st_mode&S_IFMT)==S_IFDIR;
}

int64_t FileExists(char* name) { return access(name, F_OK) == 0; }
void FileWrite(char *fn,char *data,int64_t sz) {
  FILE *f=fopen(fn,"wb");
  if(!f) return ;
  fwrite(data,sz,1,f);
  fclose(f);
}
char *FileRead(char *fn,int64_t *sz) {
  int64_t s,e;
  FILE *f=fopen(fn,"rb");
  char *ret;
  if(!f) return NULL;
  fseek(f,0,SEEK_END);
  e=ftell(f);
  fseek(f,0,SEEK_SET);
  s=e-ftell(f);
  ret=A_MALLOC(s+1,NULL);
  fread(ret,1,s,f);
  ret[s]=0;
  fclose(f);
  if(sz) *sz=s;
  return ret;
}

//From 3Days(also by nroot)
__thread char thrd_pwd[1024];
__thread char thrd_drv;
void VFsThrdInit() {
	strcpy(thrd_pwd,"/");
	thrd_drv='T';
}
void VFsSetDrv(char d) {
	if(!isalpha(d)) return;
	thrd_drv=toupper(d);
}
int VFsCd(char *to,int make) {
	to=__VFsFileNameAbs(to);
	if(__FExists(to)&&__FIsDir(to)) {
		A_FREE(to);
		return 1;
	} else if(make) {
		mkdir(to,0700);
		A_FREE(to);
		return 1;
	}
	return 0;
}
static void DelDir(char *p) {
	DIR *d=opendir(p); 
	struct dirent *d2;
	char od[2048];
	while(d2=readdir(d)) {
		if(!strcmp(".",d2->d_name)||!strcmp("..",d2->d_name))
			continue;
		strcpy(od,p);
		strcat(od,"/");
		strcat(od,d2->d_name);
		if(__FIsDir(od)) {
			DelDir(od);
		} else {
			remove(od);
		}
	}
	closedir(d);
	rmdir(p);
}

int64_t VFsDel(char *p) {
	int r;
	p=__VFsFileNameAbs(p);
	if(!__FExists(p)) {
    A_FREE(p);
		return 0;
  }
	if(__FIsDir(p)) {
		DelDir(p);
	} else 
		remove(p);
	r=!__FExists(p);
	A_FREE(p);
	return r;
}

static char *mount_points['z'-'a'+1];
char *__VFsFileNameAbs(char *name) {
	char computed[1024];
	strcpy(computed,mount_points[toupper(thrd_drv)-'A']);
	computed[strlen(computed)+1]=0;
	computed[strlen(computed)]='/';
	strcat(computed,thrd_pwd);
	computed[strlen(computed)+1]=0;
	computed[strlen(computed)]='/';
	strcat(computed,name);
	return A_STRDUP(computed,NULL);
}
int64_t VFsUnixTime(char *name) {
	char *fn=__VFsFileNameAbs(name);
	struct stat s;
	stat(fn,&s);
	int64_t r=mktime(localtime(&s.st_ctime));
	A_FREE(fn);
	return r;
}
int64_t VFsFSize(char *name) {
	struct stat s;
	long cnt;
	DIR *d;
	char *fn=__VFsFileNameAbs(name);
	if(!__FExists(fn)) {
		A_FREE(fn);
		return -1;
	} else if(__FIsDir(fn)) {
		d=opendir(fn);
		cnt=0;
		while(readdir(d))
			cnt++;
		closedir(d);
		A_FREE(fn);
		return cnt;
	}
	stat(fn,&s);
	A_FREE(fn);
	return s.st_size;
}
int64_t VFsFileWrite(char *name,char *data,int64_t len) {
    FILE *f;
    name=__VFsFileNameAbs(name);
    if(name) {
        f=fopen(name,"wb");
        if(f) {
			fwrite(data,1,len,f);
			fclose(f);
		}
    }
    A_FREE(name);
    return !!name;
}
int64_t VFsIsDir(char *name) {
  int64_t ret;
  name=__VFsFileNameAbs(name);
  if(!name) return 0;
  ret=__FIsDir(name);
  A_FREE(name);
  return ret;
}
int64_t VFsFileRead(char *name,int64_t *len) {
    if(len) *len=0;
    FILE *f;
    int64_t s,e;
    void *data=NULL;
    name=__VFsFileNameAbs(name);
    if(!name) return NULL;
    if(__FExists(name))
    if(!__FIsDir(name)) {
        f=fopen(name,"rb");
        if(!f) goto end;
        s=ftell(f);
        fseek(f,0,SEEK_END);
        e=ftell(f);
        fseek(f,0,SEEK_SET);
        fread(data=A_MALLOC(e-s+1,NULL),1,e-s,f);
        fclose(f);
        if(len) *len=e-s;
        ((char*)data)[e-s]=0;
    }
    end:
    A_FREE(name);
    return data;
}
int VFsFileExists(char *path) {
	path=__VFsFileNameAbs(path);
	int e=__FExists(path);
  A_FREE(path);
	return e;
}

int VFsMountDrive(char let,char *path) {
	int idx=toupper(let)-'A';
	if(mount_points[idx])
	  A_FREE(mount_points[idx]);
	mount_points[idx]=A_MALLOC(strlen(path)+1,NULL);
	strcpy(mount_points[idx],path);
}
FILE *VFsFOpen(char *path,char *m) {
	path=__VFsFileNameAbs(path);
	FILE *f=fopen(path,m);
	A_FREE(path);
	return f;
}

int64_t VFsFClose(FILE *f) {
	fclose(f);
	return 0;
}
int64_t VFsFBlkRead(void *d,int64_t n, int64_t sz,FILE *f) {
	return 0!=fread(d,n,sz,f);
}
int64_t VFsFBlkWrite(void *d,int64_t n, int64_t sz,FILE *f) {
	return 0!=fwrite(d,n,sz,f);
}
int64_t VFsFSeek(int64_t off,FILE *f) {
	fseek(f,off,SEEK_SET);
	return 0;
}

int64_t VFsTrunc(char *fn,int64_t sz) {
	fn=__VFsFileNameAbs(fn);
	if(fn) {
		truncate(fn,sz);
    A_FREE(fn);
  }
  return 0;
}
void VFsSetPwd(char *pwd) {
	if(!pwd) pwd="/";
	strcpy(thrd_pwd,pwd);
}

FILE *VFsFOpenW(char *f) {
  return fopen(f,"wb");
}

FILE *VFsFOpenR(char *f) {
  return fopen(f,"rb");
}

int64_t VFsDirMk(char *f) {
  return VFsCd(f,1);
}

char **VFsDir(char *fn) {
	int64_t sz;
	char **ret;
  fn=__VFsFileNameAbs("");
  DIR *dir=opendir(fn);
  if(!dir) {
      A_FREE(fn);
      return NULL;
  }
  struct dirent *ent;
  sz=0;
  while(ent=readdir(dir))
    sz++;
  rewinddir(dir);
  ret=A_MALLOC((sz+1)*sizeof(char*),NULL);
  sz=0;
  while(ent=readdir(dir)) {
    //CDIR_FILENAME_LEN  is 38(includes nul terminator)
    if(strlen(ent->d_name)<=37)
      ret[sz++]=A_STRDUP(ent->d_name,NULL);
	}
  ret[sz]=0;
  A_FREE(fn);
  closedir(dir);
  return ret;
}
