U0 R404(I64 s,I64 *ptr,U8 *msg) {
  WriteLn(s,ptr,"<html>");
  WriteLn(s,ptr,"	<body>");
  WriteLn(s,ptr,"		<H1>File %s not found</H1>",msg);
  WriteLn(s,ptr,"	</body>");
  WriteLn(s,ptr,"</html>");
}

U8 *FileExtDot2(U8 *s) {
  U8 *tmp;
  I64 dot=1,l;
  if(s=FileExtDot(s)) {
    tmp=s;
    l=StrLen(tmp);
    if(l>=2)
      if(!StrICmp(tmp+l-2,".Z"))
        dot=2;
    while(StrOcc(tmp,'.')>dot) {
      tmp=StrFirstOcc(tmp+1,".");
    }
    return tmp;
  }
  return s;
}
U0 FileGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  CConnection *con;
  U8 *chroot;
  if(!url->abs_path) return;
  U8 *link=ChrootFile(url->abs_path,SERVER_HTML);
  if(!FileFind(link,,FUF_Z_OR_NOT_Z)) {
    Free(link);
    link=ChrootFile(url->abs_path,SERVER_WWW);
  }
  U8 *f,*mime,*dot=FileExtDot2(link),*et;
  I64 len;
  if(dot&&FileFind(link,,FUF_Z_OR_NOT_Z)){
    if(!StrICmp(dot,".GIF")||!StrICmp(dot,".GIF.Z")) mime="image/gif";
    else if(!StrICmp(dot,".JPEG")||!StrICmp(dot,".JPEG.Z")) mime="image/jpeg";
    else if(!StrICmp(dot,".JPG")||!StrICmp(dot,".JPG.Z")) mime="image/jpg";
    else if(!StrICmp(dot,".PNG")||!StrICmp(dot,".PNG.Z"))  mime="image/png";
    else if(!StrICmp(dot,".BMP")||!StrICmp(dot,".BMP.Z"))  mime="image/bmp";
    else if(!StrICmp(dot,".CSS")||!StrICmp(dot,".CSS.Z"))  mime="text/css";
    else if(!StrICmp(dot,".MP4")||!StrICmp(dot,".MP4.Z"))  mime="video/mp4";
    else if(!StrICmp(dot,".MIN.CSS")||!StrICmp(dot,".MIN.CSS.Z"))  mime="text/css";
    else mime="text/plain";
    con=Fs->user_data;
    et=HashFileToETag(link);
    if(con->etag_text&&!StrCmp(et,con->etag_text)) {
      Free(et);
      con->response_code=304;
      return;
    }
    con->provided_etag_by_server=TRUE;
    Free(con->etag_text);
    con->etag_text=et;
    f=FileRead(link,&len);
    StrCpy(con->response_mime,mime);
    con->response_code=200;
    WriteNBytes(stream,NULL,f,len);
    con->cache_age=24*60*60*3; //3 days
    Free(f);
  } else {
    R404(stream,&len,url->abs_path);
    con=Fs->user_data;
    StrCpy(con->response_mime,"text/html");
    con->response_code=404;
    R404(stream,NULL,url->abs_path);
  }
en:
  Free(link);
}
U0 DirGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  U8 *prefix="/";
  U8 *tmp,*last;
  Bool raw=FALSE;
  if(!url->abs_path) return;
  last=&url->abs_path[StrLen(url->abs_path)-1];
  if (*last=='/')
    *last=0;
  if(!StrNCmp(url->abs_path,SERVER_RAW_PREFIX,StrLen(SERVER_RAW_PREFIX))) {
    tmp=StrNew(url->abs_path+StrLen(SERVER_RAW_PREFIX));
    Free(url->abs_path);
    url->abs_path=tmp;
    prefix=SERVER_RAW_PREFIX;
    raw=TRUE;
  }
  U8 *path=ChrootFile(tmp=MStrPrint("%s/*",url->abs_path));
  Free(tmp);
  CDirEntry *root=FilesFind(path),*cur;
  I64 len;
  CDoc *tmpd=DocNew("/tmp_doc.DD"),*html;
  DocPrint(tmpd,"$$FG,PURPLE$$$$TX+CX,\"Pro Tip!!!\"$$$$FD$$$$WW,1$$\n\n");
  DocPrint(tmpd,"  You can download the whole directory in a TempleOS $$RED$$.ISO.C$$FD$$ format which you get the files out via $$HL,1$$MountFile(\"File.ISO.C\");$$HL,0$$ by clicking the typing in \n$$RED$$aiwnios.com/raw_dir/some_directory.ISO.C$$FD$$ or by clicking here $$LK,\"%Q.ISO.C\",A=\"FL:/raw_dir%Q.ISO.C\"$$\n\n",url->abs_path,url->abs_path);
  DocPrint(tmpd,"  You can download indivual files raw with $$RED$$aiwnios.com/raw/some_file$$FD$$\n\n");
  DocPrint(tmpd,"$$FG,PURPLE$$$$TX+CX,\"Directory listing of %Q:\"$$$$FD$$\n\n",url->abs_path);
  DocPrint(tmpd,"$$ID,2$$");
  for(cur=root;cur;cur=cur->next) {
    if(IsDir(cur->full_name)) {
      DocPrint(tmpd,"$$LK,\"%Q/\",A=\"FL:%Q%Q/%Q\"$$\n",cur->name,prefix,url->abs_path,cur->name);
    } else 
      DocPrint(tmpd,"$$LK,\"%Q\",A=\"FL:%Q%Q/%Q\"$$\n",cur->name,prefix,url->abs_path,cur->name);
  }
  DocPrint(tmpd,"$$ID,-2$$");
  html=Doc2Html(tmpd);
  tmp=DocSave(html,&len);
  WriteNBytes(stream,NULL,tmp,len);
  Free(tmp);
  DocDel(tmpd),DocDel(html);
  Free(path);
  DirEntryDel(root);
}
#define ISOC_CACHE "../ISOCs"
if(!FileFind(ISOC_CACHE)) DirMk(ISOC_CACHE);
U0 Dir_ISOC_Get(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  U8 *tmp,*fp,*cache=NULL,*data;
  CDirEntry dent,cache_ent;
  I64 flen;
  CConnection *con=Fs->user_data;  
  if(!url->abs_path) return;
  if(!StrNCmp(url->abs_path,SERVER_ISOC_PREFIX,StrLen(SERVER_ISOC_PREFIX))) {
    tmp=StrNew(url->abs_path+StrLen(SERVER_ISOC_PREFIX));
    Free(url->abs_path);
    url->abs_path=tmp;
    FileExtRem(url->abs_path); //Remove .ISO.C
  }
  fp=ChrootFile(url->abs_path);
  if(!FileFind(fp,&dent)) {
    R404(stream,NULL,url->abs_path);
    goto fin;
  }
  tmp=ChrootFile(url->abs_path,ISOC_CACHE);
  cache=ExtDft(tmp,"ISO.C");
  Free(tmp);
  if(!FileFind(cache,&cache_ent)) {
regen:
     EnsurePathExists(cache);
     data=FileNameAbs(cache);
     tmp=MStrPrint("RedSeaISO(\"%Q\",\"%Q\");;",data,fp);
     Free(data);
     JobResGet(TaskExe(Gs->seth_task,Fs,tmp,JOBf_WAKE_MASTER));
     Free(tmp);
  } else if(cache_ent.datetime<dent.datetime) {
    Free(cache_ent.full_name);
    goto regen;
  } else
    Free(cache_ent.full_name);
  StrCpy(con->response_mime,"application/x-binary");
  data=FileRead(cache,&flen);
  WriteNBytes(stream,NULL,data,flen);
  Free(data);
  Free(dent.full_name);
fin:
  Free(cache);
  Free(fp);
}
U0 DolDocGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  if(!url->abs_path) return;
  U8 *path=ChrootFile(url->abs_path),*html_name=MStrPrint("%s.html",url->abs_path),*tmp,*et;
  CConnection *con=Fs->user_data;
  CDirEntry dol_file,html_file;
  I64 i;
  tmp=ChrootFile(html_name,SERVER_HTML);
  Free(html_name);
  html_name=tmp;
  for(i=0;i!=2;i++) {
    if(i) 
      tmp=StrNew(path);
    else
      tmp=ToggleZorNotZ(path);
    if(tmp[StrLen(tmp)-1]=='/')
      tmp[StrLen(tmp)-1]=0;
    if(FileFind(tmp,&dol_file,0)) {
      if(IsDir(path))
        DirGet(srv,stream,url,req);
      else if(!FileFind(html_name,&html_file)) {
        EnsurePathExists(tmp);
        ToHtml(tmp,html_name);
      } else if(html_file.datetime<dol_file.datetime) {
        EnsurePathExists(html_name);
        ToHtml(tmp,html_name);
      } //Htm exists and is not newer than the doldoc file
      Free(tmp);
      break;
    }
    Free(tmp);
  }
  if(i==2) {
//Try like a normal file in HTML directory.
    FileGet(srv,stream,url,req); //Fails for us.
  }else {
    et=HashFileToETag(html_name);
    if(con->etag_text&&!StrCmp(et,con->etag_text)) {
      Free(et);
      con->response_code=304;
      return;
    }
    con->provided_etag_by_server=TRUE;
    Free(con->etag_text);
    con->etag_text=et;
    tmp=FileRead(html_name,&i);
    WriteNBytes(srv,NULL,tmp,i);
    Free(tmp);
  }
  Free(path);
  Free(html_name);
}


Bool HolyCGIGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  U8 *try_path=MStrPrint(__DIR__"/HolyCGI/%s",url->abs_path);
  U8 *ftxt,*pass;
  Bool ret=FALSE;
  if(FileFind(try_path)) {
    if(ftxt=FileRead(try_path)) {
      if(StrMatch("OnGet",ftxt)) {
        try {
	  ExePutS2(ftxt,try_path);
	  pass=MStrPrint("OnGet(%d,%d,%d,%d);;\n",srv,stream,url,req);
	  ExePutS2(pass,try_path);
	  ret=TRUE;
	  Free(pass);
        } catch {
	  PutExcept(TRUE);
        }
      }
      Free(ftxt);
    } 
  }
  Free(try_path);
  return ret;
}

Bool HolyCGIPost(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable *post) {
  U8 *try_path=MStrPrint(__DIR__"/HolyCGI/%s",url->abs_path);
  U8 *ftxt,*pass;
  Bool ret=FALSE;
  if(FileFind(try_path)) {
    if(ftxt=FileRead(try_path)) {
      if(StrMatch("OnPost",ftxt)) {
        try {
	  ExePutS2(ftxt,try_path);
	  pass=MStrPrint("OnPost(%d,%d,%d,%d,%d);;\n",srv,stream,url,req,post);
	  ExePutS2(pass,try_path);
	  Free(pass);
	  ret=TRUE;
        } catch {
	  PutExcept(TRUE);
        }
      }
      Free(ftxt);
    } 
  }
  Free(try_path);
  return ret;
}


U0 Get(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req)  {
  if(!url||!url->abs_path) return;
  U8 *dot=FileExtDot(url->abs_path),*np,*data;
  I64 flen;
  CConnection *con=Fs->user_data;
  if(HolyCGIGet(srv,stream,url,req))
    goto fin;
  CDirEntry dent;
  if(!StrNCmp(url->abs_path,SERVER_ISOC_PREFIX,StrLen(SERVER_RAW_PREFIX))) {
    Dir_ISOC_Get(srv,stream,url,req);
    goto fin;
  } else if(!StrNCmp(url->abs_path,SERVER_RAW_PREFIX,StrLen(SERVER_RAW_PREFIX))) {
    np=ChrootFile(url->abs_path+StrLen(SERVER_RAW_PREFIX));
    if(!FileFind(np,&dent)) {
      R404(stream,NULL,url->abs_path);
    } else if(IsDir(dent.full_name)) {
      DirGet(srv,stream,url,req);
      Free(dent.full_name);
    } else {
      StrCpy(con->response_mime,"application/x-binary");
      data=FileRead(np,&flen);
      WriteNBytes(stream,NULL,data,flen);
      Free(dent.full_name);
      Free(data);
    }
    Free(np);
    goto fin;
  }
  if(!StrCmp(url->abs_path,"/")&&FileFind("/WWW/index.DD")) {
    Free(url->abs_path);
    url->abs_path=StrNew("/index.DD");
  }
  if(!dot) {
    DolDocGet(srv,stream,url,req);
    goto fin;
  }        
  if(!StrICmp(dot,".DD")
        ||!StrICmp(dot,".DD.Z")
        ||!StrICmp(dot,".HC")
        ||!StrICmp(dot,".HC.Z")||!StrICmp(dot,".GMR")) {
    DolDocGet(srv,stream,url,req);
    goto fin;
  }
  FileGet(srv,stream,url,req);
fin:
  EndHttpRequest(stream);
}

U0 Post(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable *post)  {
  if(HolyCGIPost(srv,stream,url,req,post)) {
    EndHttpRequest(stream);
    return;
  }
//Fallback
  Get(srv,stream,url,req);
}
