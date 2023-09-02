extern class CConnection;
class CUniqueToken {
  CServer *srv;
  CConnection *con;
};
class CConnection {
	U8 *buffer;
	U8 *buffer_ptr;
	I64 buffer_len;
	//These occupy .u8[0]
	#define CON_S_METHOD 1
	#define CON_S_FIELDS 2
	#define CON_S_BLANK_LINE 3
	#define CON_S_RAW_DATA 4
	//These are flags
	#define CONf_AWAIT_BOUNDARY 8
	#define CONf_URL_ENCODED 9
	#define CONf_DATA 10
	#define CONf_KEEP_ALIVE 11
	I64 state,method;
	I64 b_ptr;
	CServer *server;
	I64 stream;
	CHashTable *post_data;
	I64 field_ptr,field_length;
	U8 *fbuffer;
	I64 content_len;
	CURL *url;
	U8 *name;
	CHeapCtrl *hc;
	CTask *task;
	CUniqueToken *unique_token;
	I64 is_dead,response_length;
	U8 *session_cookie,*response_text;
	I64 response_code,cache_age;
	Bool etag_changed,provided_etag;
	U8 response_mime[STR_LEN];
	U8 boundary[STR_LEN];
	U8 field[STR_LEN];
};
I64 __EncodeQueryValue(U8 *s,U8 *to) {
  I64 i,l=StrLen(s),len2=0;
  U8 *xdigits="0123456789abcdef";
  for(i=0;i!=l;i++) {
     switch(s[i]) {
       case 'a'...'z':
       case 'A'...'Z':
       case '_':
       case '0'...'9':
       if(to) *to++=s[i];
       len2++;
       break;
       case ' ':
       if(to) *to++='+';
       len2++;
       break;
       default:
       if(to) *to++='%';
       if(to) *to++=xdigits[(s[i]>>4)&0xf];
       if(to) *to++=xdigits[s[i]&0xf];
       len2+=3;
     }
  }
  if(to) *to++=0;
  len2++;//NULL terminaoter
  return len2;
}
U8 *EncodeQueryValue(U8 *s) {
  I64 len=__EncodeQueryValue(s,NULL);
  U8 *ret=MAlloc(len);
  __EncodeQueryValue(s,ret);
  return ret;

}
U8 *GetQueryValue(U8 *q,U8 *f) {
	I64 len=StrLen(q),bptr=0;
	U8 *buf,three[3],name[STR_LEN];
	U8 *ptr=q,*en;
	loop:
	buf=name;
	while(*ptr&&!StrOcc("&=",*ptr)) {
		buf[bptr++]=*ptr++;
	}
	buf[bptr]=0;
	bptr=0;
	if(!StrCmp(f,buf)) {
		if(*ptr=='=') {
			en=StrFirstOcc(ptr,"&");
			if(!en) en=ptr+StrLen(ptr);
			buf=CAlloc(en-ptr+1);
			ptr++;
			while(*ptr&&*ptr!='&') {
				if(*ptr=='%') {
					MemCpy(three,++ptr,2);
					three[2]=0;
					buf[bptr++]=Str2I64(three,16);
					ptr+=StrLen(three);
				} else if(*ptr=='+') {
					buf[bptr++]=' ';
					ptr++;
				} else
					buf[bptr++]=*ptr++;
			}
			buf[bptr++]=0;
			return buf;
		} else 
			return StrNew("");
	} else if(*ptr) {
		if(StrFirstOcc(ptr,"&")) {
			ptr=1+StrFirstOcc(ptr,"&");
			goto loop;
		}
	}
	return NULL;
}
CURL *ParseURL(U8 *url) {
	CURL *ret=CAlloc(sizeof(CURL));
	U8 *r,*q,*p,chr;
	I64 i,l;
	if(!(url=StrFind("//",url)))
		goto fail;
	url+=2;
	r=StrFirstOcc(url,"/");
	q=StrFirstOcc(url,"?");
	p=StrFirstOcc(url,":");
	if(!p) p=StrLen(url)+url;
	if(!r) r=StrLen(url)+url;
	if(!q) q=StrLen(url)+url;
	if(p<r||p<q) {
		ret->hostname=MAlloc(l=p-url+1);
		ret->hostname[l-1]=0;
		MemCpy(ret->hostname,url,l-1);
		url=p+1;
		ret->port=Str2I64(url,10,&url);
	} else {
		ret->hostname=MAlloc(l=MinI64(p,r)-url+1);
		ret->hostname[l-1]=0;
		MemCpy(ret->hostname,url,l-1);
		url=MinI64(p,r)+1;
	}
	if(!*url) goto retn;
	if(r<q) {
		ret->abs_path=MAlloc(l=q-r-1+1);		
		ret->abs_path[l-1]=0;
		MemCpy(ret->abs_path,r+1,l-1);
	}
	if(*q) {
		ret->query=StrNew(q+1);
	} else
		ret->query=StrNew("");
	retn:
	return ret;
	fail:
	Free(ret);
	return NULL;
}

U0 DumpURL(CURL *url) {
	if(!url) return;
}
U8 *SkipWhitespace(U8 *s) {
	while(*s&&StrOcc(" \t\n",*s)) s++;
	return s;
}
U8 *Trim(U8 *buf) {
	if(StrFirstOcc(buf," \t\n\x0d"))
			*StrFirstOcc(buf," \t\n\x0d")=0;
	return SkipWhitespace(buf);
}

U0 WriteNBytes(I64 s,I64 *len=NULL,U8 *bytes,I64 n) {
  CConnection *con=Fs->user_data;
  U8 *new,**blob;
  if(len) {
    *len+=n;
    return;
  }
  if(blob=FramePtr("CACHE_BLOB")) {
    MemCpy(*blob,bytes,n);
    *blob+=n;
    FramePtrSet("CACHE_BLOB",blob);
  }
  if(Bt(&con->is_dead,0))
    Exit;
  if(!con->response_text) {
    con->response_text=MAlloc(n+1024);
  } else if(MSize(con->response_text)<con->response_length+n) {
    new=MAlloc(MSize(con->response_text)+n+1024);
    MemCpy(new,con->response_text,con->response_length);
    Free(con->response_text);
    con->response_text=new;
  }
  MemCpy(con->response_text+con->response_length,bytes,n);
  con->response_length+=n;
fin:;
}
U0 Write(I64 s,I64 *len=NULL,U8 *fmt,...) {
	U8 *str=StrPrintJoin(NULL,fmt,argc,argv),**blob;
	WriteNBytes(s,len,str,StrLen(str));
	Free(str);
}
U0 WriteLn(I64 s,I64 *len=NULL,U8 *fmt,...) {
	U8 *str=StrPrintJoin(NULL,fmt,argc,argv),**blob;
	WriteNBytes(s,len,str,StrLen(str));
	WriteNBytes(s,len,"\x0d\n",2);
	Free(str);

}
U0 R200(I64 s,I64 *ptr,U8 *msg) {
	U8 *ftxt=FileRead(msg),*ptr2,*next;
	WriteLn(s,ptr,"<html>");
	WriteLn(s,ptr,"	<body>");
	WriteLn(s,ptr,"		<H1>File %s found!!!</H1>",msg);
	WriteLn(s,ptr,"			<PRE>");
	for(ptr2=ftxt;*ptr2;) {
		if(StrFirstOcc(ptr2,"\n"))
			*StrFirstOcc(ptr2,"\n")=0,next=1+ptr2+StrLen(ptr2);
		else
			next=ptr2+StrLen(ptr2);
		WriteLn(s,ptr,"%s",ptr2);
		ptr2=next;
	}
	WriteLn(s,ptr,"			</PRE>");
	WriteLn(s,ptr,"	</body>");
	WriteLn(s,ptr,"</html>");
	Free(ftxt);
}
U0 R404(I64 s,I64 *ptr,U8 *msg) {
	WriteLn(s,ptr,"<html>");
	WriteLn(s,ptr,"	<body>");
	WriteLn(s,ptr,"		<H1>File %s not found</H1>",msg);
	WriteLn(s,ptr,"	</body>");
	WriteLn(s,ptr,"</html>");
}
U0 GenericGet(CServer *serv,I64 s,CURL *url,CHTTPRequest *req) {
	CDirEntry ent;
	I64 len=0;
	DumpURL(url);
	CConnection *con=Fs->user_data;
	StrCpy(con->response_mime,"text/html");
	if(FileFind(url->abs_path,&ent,FUF_JUST_FILES)) {
		con->response_code=200;
		R200(s,&len,url->abs_path);
		WriteLn(s,,"Content-Length: %d",len);
		R200(s,NULL,url->abs_path);
	} else {
		con->response_code=404;
		R404(s,NULL,url->abs_path);
	}
}

CServer *CreateServer(I64 port) {
	CServer *srv=CAlloc(sizeof(CServer));
	srv->port=port;
	QueInit(&srv->connects);
	return srv;
}
U0 SendRequest(I64 s) {
  U8 *tmp,*tmp2,*cookie;
  U8 *msg="200 OK";
  CConnection *con=Fs->user_data;
  if(con->provided_etag&&!con->etag_changed)
    con->response_code=304;
  switch(con->response_code) {
      break;case 200: msg="200 OK";
      break;case 404: msg="404 Not Found";
      break;case 304: msg="304 Not Modified";
//If we have a 304(not-modefied)
//We will ommit the body
      con->response_length=0;
  }
  tmp=MStrPrint(
        "HTTP/1.1 %s\x0d\n"
        "Content-Type: %s\x0d\n"
        "Content-Length: %d\x0d\n"
        ,msg,con->response_mime,con->response_length
        );
  NetWrite(s,tmp,StrLen(tmp));
  Free(tmp);
  NetWrite(s,"\x0d\n",2);
  NetWrite(s,con->response_text,con->response_length);
}

U0 EndHttpRequest(I64 s) {
  CConnection *con=Fs->user_data,old;
  if(Bt(&con->is_dead,0)) {
    return;
  }
//SendRequest may set the session cookie,we we do this after we send the request
  SendRequest(s);
  NetClose(s);
  LBts(&con->is_dead,0);
}

U0 CloseConnectionCB() {
  CConnection *con=Fs->user_data;
  con->task=NULL;
  LBts(&con->is_dead,0);
}
U0 ParseRequest(CConnection *con) {
  FramePtrAdd("CACHE_BLOB",NULL);
  FramePtrAdd("CONNECTION",con);
  FramePtrAdd("TIMEOUT",(tS+5)(I64));
  Fs->put_doc=Fs->parent_task->put_doc;
  Fs->user_data=con;
  Fs->task_end_cb=&CloseConnectionCB;
  con->task=Fs;
  I64 stream=con->stream;
  U8 *http2,*t1,buf[STR_LEN],buf2[STR_LEN],*t2,*ptr,*optr;
  CHashGeneric *hash;
  CHTTPRequest req;
  I64 len;
  MemSet(&req,0,sizeof(CHTTPRequest));
  req.method=con->method;
  CURL *url=con->url;
  switch(con->method) {
    case HTTP_GET:
      (*con->server->get)(con->server,stream,url,&req);
      break;
    case HTTP_POST:
      (*con->server->post)(con->server,stream,url,&req,con->post_data);
      NetWrite(stream,"\x0d\n",2);
      break;
  }
  NetClose(stream);
  LBts(&con->is_dead,0);
  Exit;
}

U8 *LookForLine(U8 *ptr,I64 len) {
  U8 *cap=ptr+len;
  while(ptr<cap) {
    if(*ptr++ =='\n') {return ptr;}
  }
  return NULL;
}
extern Bool SReadLine(I64 s,U8 *data,I64 len,CUniqueToken *ut);
U0 DoNothing(I64) {
  while(TRUE) {Sleep(1000000);};
}
U0 SReadData(I64 s,U8 *data,I64 len,CUniqueToken *ut) {
  CConnection *con;
  U8 *tmp, *ptr;
  I64 len2;
  Bool en=FALSE;
  if(ut->con) {
    con=ut->con;
    goto found;
  }
  con=CAlloc(sizeof(CConnection));
  con->hc=Spawn(&DoNothing,,,,Fs);
  con->buffer=MAlloc(0xB00,con->hc);
  con->buffer_ptr=con->buffer;
  con->buffer_len=0;
  con->unique_token=ut;
  con->server=ut->srv;
  con->stream=s;
  con->state.u8[0]=CON_S_METHOD;
  con->name=StrNew("",con->hc);
  con->post_data=HashTableNew(0x50,con->hc);
  ut->con=con;
found:
  if(MSize(con->buffer)<=1+(con->buffer_ptr-con->buffer)+len+con->buffer_len) {
    tmp=MAlloc((con->buffer_ptr-con->buffer)+1+2*MSize(con->buffer)+len,con->hc);
    MemCpy(tmp,con->buffer,MSize(con->buffer));
    Free(con->buffer);
    con->buffer_ptr=tmp+(con->buffer_ptr-con->buffer);
    con->buffer=tmp;
  }
  MemCpy(con->buffer_ptr,data,len);
  con->buffer_len+=len;
  con->buffer_ptr[con->buffer_len]=0;
loop:
  if(ptr=LookForLine(con->buffer_ptr,con->buffer_len)) {
     len2=ptr-con->buffer_ptr;
     tmp=MAlloc(len2+1,con->hc);
     MemCpy(tmp,con->buffer_ptr,len2);
     tmp[len2]=0;
     en=SReadLine(s,tmp,len2,ut);
     Free(tmp);
     con->buffer_len-=len2;
     con->buffer_ptr+=len2;
     if(!en)
       goto loop;
  } else {
    len=con->buffer_len;
    en=SReadLine(s,con->buffer_ptr,len,ut);
	con->buffer_len-=len;
    con->buffer_ptr+=len;
  }
}
Bool SReadLine(I64 s,U8 *data,I64 len,CUniqueToken *ut) {
  static I64 spawn_at=0;
  CConnection *first,*con,*con2;
  CHashGeneric *hash;
  U8 *buf,*buf2;
  U8 *fb,*http2,*t1,*t2,*odata;
  con=ut->con;
  buf=MAlloc(len+1,con->hc),buf2=MAlloc(len+1,con->hc);
  if(len==2) con->state.u8[0]=CON_S_BLANK_LINE;
  if(con->state.u8[0]==CON_S_METHOD&&!StrNICmp(data,"GET",3)) {
    con->method.u8[0]=HTTP_GET;
    t1=MStrPrint("http://potato.com/%s",Trim(SkipWhitespace(data+3)));
    con->url=ParseURL(t1);
    Free(t1);
    con->state.u8[0]=CON_S_FIELDS;
  } else if(con->state.u8[0]==CON_S_METHOD&&!StrNICmp(data,"POST",4)) {
    con->method.u8[0]=HTTP_POST;
    t1=MStrPrint("http://potato.com/%s",Trim(SkipWhitespace(data+4)));
    con->url=ParseURL(t1);
    Free(t1);
    con->state.u8[0]=CON_S_FIELDS;
  } else if(con->state.u8[0]==CON_S_METHOD) {
    con->method.u8[0]=HTTP_UNDEF;
  } else if(!StrNICmp(data,"Content-Length",StrLen("Content-Length"))) {
    data=SkipWhitespace(StrFirstOcc(data,":")+1);
    con->fbuffer=MAlloc(Str2I64(data),con->hc);
    con->content_len=Str2I64(data);
  }
  if(con->state.u8[0]==CON_S_BLANK_LINE&&len>2&&Bt(&con->state,CONf_URL_ENCODED)) {
    odata=data;
qloop:
    t2=StrFirstOcc(data,"=");
    if(!t2) t2=data+StrLen(data);
    MemCpy(buf,data,t2-data);
    buf[t2-data]=0;
    data=t2;
    t1=GetQueryValue(odata,buf);
    hash=CAlloc(sizeof(CHashGeneric)+StrLen(t1)+1,con->hc);
    hash->str=StrNew(buf,con->hc);
    hash->user_data0=StrLen(t1);
    hash->user_data1=hash+1;
    StrCpy(hash->user_data1,t1);
    hash->type=HTT_DICT_WORD;
    HashAdd(hash,con->post_data);
    Free(t1);
    if(*data&&StrFirstOcc(data,"&")) {
      data=StrFirstOcc(data,"&")+1;
      goto qloop;
    }
    Btr(&con->state,CONf_URL_ENCODED);
    con->field_ptr=StrLen(odata);
    goto process;
  } else if(con->method==HTTP_POST&&con->state.u8[0]!=CON_S_BLANK_LINE&&StrOcc(data,':')) {
    t1=buf,t2=buf2;
    StrScan(data,"%s:%s",&t1,&t2);
    if(!StrICmp(Trim(buf),"Content-Type")) {
      if(StrIMatch("multipart/form-data",buf2)) {
        t2=con->boundary;
        if(!StrIMatch("boundary=",buf2)) goto malformed;
        StrScan(StrFirstOcc(buf2,";")+1,"%sboundary=%s\x0d\n",&t1,&t2);
        LBts(&con->state,CONf_AWAIT_BOUNDARY);
      } else if(StrIMatch("urlencoded",buf2)) {
        LBts(&con->state,CONf_URL_ENCODED);
      }
    }
  }
  if(!Bt(&con->state,CONf_DATA)&&!StrNICmp(data,"If-None-Match",StrLen("If-None-Match"))) {
    con->provided_etag=TRUE;
    data=SkipWhitespace(StrFirstOcc(data,":")+1);
etag_loop:
    t1=buf;
    if(StrOcc(data,'\"')!=2) goto malformed;
    data=StrScan(data,"\"%s\"",&t1);
    if(*data==',') {
      data=SkipWhitespace(data+1);
      goto etag_loop;
    }
  } else if(!Bt(&con->state,CONf_DATA)&&!StrNICmp(data,"Connection",StrLen("Connection"))) {
    data=SkipWhitespace(StrFirstOcc(data,":")+1);
    if(StrIMatch("keep-alive",data)) {
      Bts(&con->state,CONf_KEEP_ALIVE);
    } else
      Btr(&con->state,CONf_KEEP_ALIVE);
  } else if(!Bt(&con->state,CONf_DATA)&&!StrNICmp(data,"Cookie",StrLen("Cookie"))) {
    data=SkipWhitespace(StrFirstOcc(data,":")+1);
    t1=buf,t2=buf2;
    if(*data) {
cloop:;
      odata=data;
      StrCpy(buf,"");
      StrCpy(buf2,"");
      if(!StrFirstOcc(data,"="))
        goto malformed;
      t1=buf,t2=buf2;
      if(StrOcc(data,';'))
        data=StrScan(data,"%s=%s;",&t1,&t2);
      else
        data=StrScan(data,"%s=%s",&t1,&t2);
      if(data)
      if(*data==' ') {
        data++;
	goto cloop;
      }
    }
  }else if(Bt(&con->state,CONf_AWAIT_BOUNDARY)) {
    t1=buf;
    if(!StrNCmp(data,"--",2)&&!StrCmp(Trim(data+2),con->boundary)) {
      if(con->field[0]) {
        hash=CAlloc(1+con->field_ptr+1+StrLen(con->name)+sizeof(CHashGeneric),con->hc);
        hash->str=StrNew(con->field,con->hc);
        if(con->field_length==-1)
	  hash->user_data0=con->field_ptr-2;
        else
	  hash->user_data0=con->field_length;
        hash->user_data1=hash+1;
        hash->user_data2=hash->user_data1+con->field_ptr+1;
        StrCpy(hash->user_data2,con->name);
        MemCpy(hash->user_data1,con->fbuffer,con->field_ptr);
        hash->type=HTT_DICT_WORD;
        HashAdd(hash,con->post_data);
      }
      con->field[0]=0;
      con->field_ptr=0;
      con->field_length=-1;
      Btr(&con->state,CONf_DATA);
    } else if(!StrNCmp(data,"--",2) && !StrNCmp(data+2,con->boundary,StrLen(con->boundary))
	  &&!StrCmp(Trim(data+2+StrLen(con->boundary)),"--")
    ) {
      if(con->field[0]) {
        hash=CAlloc(1+con->field_ptr+1+StrLen(con->name)+sizeof(CHashGeneric),con->hc);
        hash->str=StrNew(con->field,con->hc);
        if(con->field_length==-1)
	  hash->user_data0=con->field_ptr-2;
        else
	  hash->user_data0=con->field_length;
        hash->user_data1=hash+1;
        hash->user_data2=hash->user_data1+con->field_ptr+1;
        StrCpy(hash->user_data2,con->name);
        MemCpy(hash->user_data1,con->fbuffer,con->field_ptr);
        hash->type=HTT_DICT_WORD;
        HashAdd(hash,con->post_data);
      }
      //We hit the boundary: safe to transmit data
      con->field_ptr=con->content_len;
      goto process;
    } else if(!Bt(&con->state,CONf_DATA)&&!StrNICmp(data,"Content-Length",StrLen("Content-Length"))) { 
      con->field_length=Str2I64(SkipWhitespace(StrFirstOcc(data,":")+1));
    } else if(!Bt(&con->state,CONf_DATA)&&!StrNICmp(data,"Content-Disposition",StrLen("Content-Disposition"))) {
      data=SkipWhitespace(StrFirstOcc(data,":")+1);
      if(!StrNICmp(data,"form-data",StrLen("form-data"))) {
loop:
        if(data) {
	  data=SkipWhitespace(data);
	  if(!StrNICmp(data,"name",StrLen("name"))) {
	    t2=con->field;
	    if(!StrFirstOcc(data,"\"")) goto malformed;
	    if(StrOcc(data,'\"')<2) goto malformed;
	    data=StrScan(StrFirstOcc(data,"\""),"\"%s\"",&t2);
	  } else if(!StrNICmp(data,"filename",StrLen("filename"))) {
	    if(con->name) Free(con->name);
	    con->name=MAlloc(len+1,con->hc);
	    t2=con->name;
	    if(!StrFirstOcc(data,"\"")) goto malformed;
	    if(StrOcc(data,'\"')<2) goto malformed;
	    data=StrScan(StrFirstOcc(data,"\""),"\"%s\"",&t2);
	  }
	  if(data) data=StrFirstOcc(data,";");
	  if(data) {
	    data++;
	    goto loop;
	  }
	  Free(buf),Free(buf2);
	  return FALSE;
        }
      } 
    } else if(!Bt(&con->state,CONf_DATA)&&len==2) { //+2 for \x0d\n
      Bts(&con->state,CONf_DATA);
    } else if(Bt(&con->state,CONf_DATA)) {
      if(!con->fbuffer) {
	con->fbuffer=MAlloc(len+1,con->hc);
      } else if(MSize(con->fbuffer)<=1+len+con->field_ptr) {
        t1=MAlloc(2*con->field_ptr+len+1,con->hc);
	MemCpy(t1,con->fbuffer,con->field_ptr);
	Free(con->fbuffer);
	con->fbuffer=t1;
      }
      MemCpy(con->fbuffer+con->field_ptr,data,len);
      con->field_ptr+=len;
      goto en;
    }	
  }
en:
//2 is \x0d\n
  if(len==2&&!Bt(&con->state,CONf_AWAIT_BOUNDARY)&&!(Bt(&con->state,CONf_URL_ENCODED)&&con->content_len))
    goto process;
  Free(buf),Free(buf2);
  return FALSE;
malformed:
  Free(buf),Free(buf2);
  NetClose(s);
  LBts(&con->is_dead,0);
  return TRUE;
process:
  Free(buf),Free(buf2);
//If we have content,make sure we recieved all the data
  if((con->content_len&&con->content_len==con->field_ptr)||!con->content_len){
    Spawn(&ParseRequest,con,"Request",,Fs);
  }
  return FALSE;
}
U0 CloseCallback(I64 s,CUniqueToken *ut) {
//Killing the thread will fre the connection too
  CConnection *con=ut->con;
  if(!con) return;
  if(con->task)
    Kill(con->task,TRUE); //Wait for kill
  con->task=NULL;
  Free(con->session_cookie);
  if(con->url) {
    Free(con->url->query);
    Free(con->url->abs_path);
    Free(con->url->hostname);
    Free(con->url);
    Free(con->fbuffer);
  }
  if(con->post_data)
    HashTableDel(con->post_data);
  Kill(con->hc);
  Free(con);
  return;
}
#define BACKLOG 128
CUniqueToken unique_tokens[BACKLOG];
I64 fds[BACKLOG];
U0 ServeExit() {
  CServer *server=Fs->user_data(CServer *);
  I64 idx;
  for(idx=0;idx!=BACKLOG;idx++) {
    if(fds[idx]!=-1)
      CloseCallback(fds[idx],&unique_tokens[idx]);
  }
  NetClose(server->s_sock);
  Exit;
}
U0 ServeTask(CServer *srv) {
  U8 buf[0x10000];
  I64 fd_cnt=0,idx;
  I64 sleep_time=10,len;
  MemSetU64(fds,-1,BACKLOG);
  Fs->user_data=srv;
  Fs->task_end_cb=&ServeExit;
  srv->task=Fs;
  srv->s_sock=NetSocketNew;
  CNetAddr *addr=NetAddrNew("127.0.0.1",srv->port);
  NetBindIn(srv->s_sock,addr);
  NetAddrDel(addr);
  NetListen(srv->s_sock,BACKLOG-1);
  while (TRUE) {
    if(-1==NetPollForRead(1,&srv->s_sock)) {
      for(idx=0;idx!=BACKLOG;idx++) {
        if(fds[idx]!=-1&&unique_tokens[idx].con) {
	  if(Bt(&unique_tokens[idx].con->is_dead,0)) {
	    CloseCallback(fds[idx],&unique_tokens[idx]);
	    fds[idx]=-1;
	    goto nxt;
	  }
        }
        if(fds[idx]!=-1&&-1!=NetPollForRead(1,&fds[idx])) {
	  len=NetRead(fds[idx],buf,0x10000);
	  buf[len]=0;
	  SReadData(fds[idx],buf,len,&unique_tokens[idx]);
        }
nxt:;
      }
      for(idx=0;idx!=BACKLOG;idx++) {
        if(fds[idx]!=-1) {
	  if(-1!=NetPollForHangup(1,&fds[idx])) {
	    CloseCallback(fds[idx],&unique_tokens[idx]);
	    fds[idx]=-1;
	  }
        }
      }
      Sleep(10);
    } else {
      for(idx=0;idx!=BACKLOG;idx++) {
        if(fds[idx]==-1) {
	  unique_tokens[idx].srv=srv;
	  unique_tokens[idx].con=NULL;
	  fds[idx]=NetAccept(srv->s_sock,NULL);
	  break;
        }
      }
    }
  }
}
U0 Serve(CServer *srv) {
  I64 core=-1;
  CTask *srv_task;
  "Press a button to quit serving\n";
  srv_task=Spawn(&ServeTask,srv,"Server",core);
  PressAKey;
  Kill(srv_task);
}

