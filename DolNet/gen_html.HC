U0 R404(I64 s,I64 *ptr,U8 *msg) {
  WriteLn(s,ptr,"<html>");
  WriteLn(s,ptr,"	<body>");
  WriteLn(s,ptr,"		<H1>File %s not found</H1>",msg);
  WriteLn(s,ptr,"	</body>");
  WriteLn(s,ptr,"</html>");
}

U0 FileGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  CConnection *con;
  if(!url->abs_path) return;
  U8 *link=ChrootFile(url->abs_path,SERVER_HTML);
  U8 *f,*mime,*dot=FileExtDot(link),*et;
  I64 len;
  if(dot&&FileFind(link,,FUF_Z_OR_NOT_Z)){
    if(!StrICmp(dot,".GIF")||!StrICmp(dot,".GIF.Z")) mime="image/gif";
    else if(!StrICmp(dot,".JPEG")||!StrICmp(dot,".JPEG.Z")) mime="image/jpeg";
    else if(!StrICmp(dot,".JPG")||!StrICmp(dot,".JPG.Z")) mime="image/jpg";
    else if(!StrICmp(dot,".PNG")||!StrICmp(dot,".PNG.Z"))  mime="image/png";
    else if(!StrICmp(dot,".BMP")||!StrICmp(dot,".BMP.Z"))  mime="image/bmp";
    else if(!StrICmp(dot,".CSS")||!StrICmp(dot,".CSS.Z"))  mime="text/css";
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
  U8 *tmp;
  if(!url->abs_path) return;
  U8 *path=ChrootFile(tmp=MStrPrint("%s/*",url->abs_path));
  CDirEntry *root=FilesFind(path),*cur;
  I64 len;
  CDoc *tmpd=DocNew(blkdev.tmp_filename),*html;
  Free(tmp);
  DocPrint(tmpd,"$$FG,PURPLE$$$$TX+CX,\"Directory listing of %Q:\"$$$$FD$$\n\n",url->abs_path);
  DocPrint(tmpd,"$$ID,2$$");
  for(cur=root;cur;cur=cur->next) {
    if(IsDir(cur->full_name)) {
      DocPrint(tmpd,"$$LK,\"%Q/\",A=\"FL:/%Q/%Q\"$$\n",cur->name,url->abs_path,cur->name);
    } else 
      DocPrint(tmpd,"$$LK,\"%Q\",A=\"FL:/%Q/%Q\"$$\n",cur->name,url->abs_path,cur->name);
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
  if(i==2)
    R404(srv,NULL,"Not found!!!");
  else {
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

U0 Get(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req)  {
  if(!url||!url->abs_path) return;
  U8 *dot=FileExtDot(url->abs_path),*np;
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
	||!StrICmp(dot,".HC.Z")
	) {
    DolDocGet(srv,stream,url,req);
    goto fin;
  }
  FileGet(srv,stream,url,req);
fin:
  EndHttpRequest(stream);
}
