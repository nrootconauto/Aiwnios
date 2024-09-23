Cd(__DIR__);;

//if (IsCmdLineMode) Silent(ON);
#define SERVER_HTML "/HTML"
#define SERVER_META "/WWW_Meta"
#define SERVER_WWW "/WWW"
#define SERVER_PORT 8080
#define SERVER_RAW_PREFIX "/raw/"
#define SERVER_ISOC_PREFIX "/raw_dir/"
#define MAX_UPLOAD 10*1024*1024

U0 ServerYield() {
  I64 to=FramePtr("TIMEOUT");
  if(to) {
    if(to(F64)<=tS)
      Exit;
  }
  Yield;
}

class CURL {
  U8 *hostname;
  U8 *abs_path;
  U8 *query;
  I64 port;
};
class CHTTPRequest {
#define HTTP_UNDEF 0
#define HTTP_GET 1
#define HTTP_POST 2
  I64 method;
};
class CServer {
  CTask *task; 
  I64 port;
  I64 s_sock;
  U0 (*get)(CServer*,I64,CURL*,CHTTPRequest*);
  U0 (*post)(CServer*,I64,CURL*,CHTTPRequest*,CHashTable *);
  CQue connects;
};

U0 EnsurePathExists(U8 *path) {
  U8 *tmp=StrNew(path),*opath=DirCur,*sl;
  if(sl=StrLastOcc(tmp,"/")) {
    *sl=0;
    Cd(tmp,TRUE);
    Cd(opath);
  }
  Free(tmp),Free(opath);
}

//Im too lazy to make a new file
U8 *ChrootFile(U8 *fn,U8 *prefix=SERVER_WWW) {
  U8 *t1,*t2;
  if(fn[1]==':')
    fn=StrNew(fn+2);
  else
    fn=StrNew(fn);
  //Check for "." , ".." and "//"(root dir)
  t1=fn;
  while (t2=StrFirstOcc(t1,"/")) {
    if(!StrNCmp(t2,"/./",3)||!StrCmp(t2,"/.")) { //The regular StrCmp checks for end of string
      StrCpy(t2,t2+2);
    } else if(!StrNCmp(t2,"/../",4)||!StrCmp(t2,"/..")) {  //See above note
      t1=t2;
      if(t2-1<=fn) {
	StrCpy(fn,t2+3); //Include the last '/'
	t1=fn;
      } else {
       t2--; //Ignore current '/'
      while(t2>fn&&*t2!='/')
        t2--;
      StrCpy(t2,t1+3); //Include the last '/'
      t1=t2;
      }
    } else if(!StrNCmp(t2,"//",2)) {
      StrCpy(fn,t2+1);
      t1=fn;
    } else
      t1=t2+1;
  }
  t1=fn;
  if(*fn=='/') fn++;
  U8 *virt=MStrPrint("%s/%s",prefix,fn);
  U8 *abs=FileNameAbs(virt),*abs2=FileNameAbs(prefix);
  if(!StrNCmp(abs,abs2,StrLen(abs2))) {
    goto fin;
  }
  if(StrLastOcc(fn,"/")) {
    Free(virt);
    virt=MStrPrint("%s/%s",prefix,StrLastOcc(fn,"/")+1);
    goto fin;
  }
  Free(virt);
  virt=MStrPrint("%s/%s",prefix,fn);
fin:
  //t1 has the start of fn
  Free(abs),Free(abs2),Free(t1);
  return virt;
}

#include "http.HC";
#include "FileBMP";
#include "ToHtml.HC";
#include "gen_html.HC";
#if __CMD_LINE__
Cd("T:/");; //We are not serving from RootFS
CServer *srv=CreateServer(SERVER_PORT);
srv->get=&Get;
srv->post=&Post;
Serve(srv);
#endif 
