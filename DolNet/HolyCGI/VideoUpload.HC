#if __CMD_LINE__
 #include "../run.HC";
#endif
#exe {Cd(__DIR__);};;
#include "AiwnTubeCommon";
//Example HolyCGI script
//GET
extern U0 OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req); 
//POST
extern U0 OnPost(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable*);

class CVideoUpload {
  U8 video_filename[32] format "$$DA-P,A=\"Video File:%s\"$$\n" html_input_type "file";
  U8 video_title[STR_LEN] format "$$DA-P,A=\"Video Tile:%s\"$$\n";
  U8 video_desc[STR_LEN] format "$$DA-P,A=\"Description:%s\"$$\n";
  U8 video_admin_password[STR_LEN] format "$$DA-P,A=\"Video Admin Panel Password:%s\"$$.\n"  html_input_type "password";
  Bool video_disable_comments format "$$CB,\"Disable Comments\"$$\n";
};

U8 *action="/VideoUpload.HC";
U0 OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  I64 len;
  U32 n;
  U8 *as_txt,*s;
  CDoc *tmp=DocNew,*html;
  CConnection *con=Fs->user_data;
  if(StrIMatch(".i2p",url->hostname)||StrIMatch(".onion",url->hostname)) {
    s="451: Video uploads not allowed from I2P/Tor.\n";
    WriteNBytes(stream,0,s,StrLen(s));
    DocDel(tmp);
    StrCpy(con->response_mime,"text/html");
    con->response_code=451;
    return;
  }
  DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"AiwniosTube Video Upload:\"$$$$FD$$\n");
  DocPrint(tmp,"\n  Upload a video to AiwniosTube. No illegal content.\n");
  DocPrint(tmp,"\n  Each video has it's own video admin panel. No accounts are used here.\n");
  html=HtmlFormGen("CVideoUpload",action,,tmp,,&n);
  as_txt=DocSave(html,&len);
  WriteNBytes(stream,NULL,as_txt,len);
  Free(as_txt);
  DocDel(html);
  DocDel(tmp);
  StrCpy(con->response_mime,"text/html");
  con->response_code=200;
}

U0 OnPost(CServer *srv,I64 stream,CURL *url,I64,CHashTable *post_data) {
  CHashGeneric *file=HashFind("video_filename",post_data,HTT_DICT_WORD);
  CHashGeneric *title=HashFind("video_title",post_data,HTT_DICT_WORD);
  CHashGeneric *desc=HashFind("video_desc",post_data,HTT_DICT_WORD);
  CHashGeneric *passwd=HashFind("video_admin_password",post_data,HTT_DICT_WORD);
  CHashGeneric *session=HashFind("session",post_data,HTT_DICT_WORD);
  CHashGeneric *captcha=HashFind("captcha",post_data,HTT_DICT_WORD);
  CHashGeneric *disable;
  CDoc *tmp=DocNew,*html,*props;
  CDoc *page=DocNew;
  I64 len,num,captchanum;
  U8 *as_txt,*tmp_str2,*tpasswd;
  U8 *filename,*tnam,*tdesc,*ttitle;
  CConnection *con=Fs->user_data;
  if(!file||!title||!desc||!passwd||!session||!captcha)
    return FALSE;

  num=FramePtr(session->user_data1,adam_task);
  StrScan(captcha->user_data1,"%X",&captchanum);
  if(num!=captchanum) {
    DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"Captcha incorrect\"$$$$FD$$\n");
    goto fin;
  }

  if(file->user_data0>MAX_UPLOAD) {
    DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"Video Too Drake Massive:\"$$$$FD$$\n");
    DocPrint(tmp,"\n  Video was size %d,Limit is %d bytes.\n",file->user_data0,MAX_UPLOAD);
    goto fin;
  }

  if(tnam=StrLastOcc(file->user_data2,".")) {
    if(StrICmp(tnam,".mp4")) {
      DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"Video Needs to be an mp4:\"$$$$FD$$\n");
      goto fin;
    }
  }


  ttitle=title->user_data1;
  tmp_str2=AiwniosTubeGetVideoProp(ttitle,"Props/VideoPath");
  if(tmp_str2) {
    Free(tmp_str2);
exists:;
    DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"Video \\\"%Q\\\" already exists:\"$$$$FD$$\n",ttitle);
    goto fin;    
  }
  Free(tmp_str2);



  tdesc=desc->user_data1;
  if(passwd)  {
    tpasswd=passwd->user_data1;
    tpasswd[passwd->user_data0]=0;
  } else
    tpasswd=NULL;
  ttitle[title->user_data0]=0;
  tdesc[desc->user_data0]=0;

  if(disable=HashFind("video_disable_comments",post_data,HTT_DICT_WORD)) {
    AiwniosTubeSetVideoPropI64(ttitle,"Props/DisableComments",1);
  }
  if(tpasswd)
    AiwniosTubeSetVideoPropI64(ttitle,"Props/AdminHash",HashStr(tpasswd));

  AiwniosTubeSetVideoPropStr(ttitle,"Props/Description",tdesc);
//Generate Page HTML(TODO make a HolyCGI script for this)
  tmp_str2=StrLastOcc(file->user_data2,".");
  tnam=MStrPrint("%s%s",ttitle,tmp_str2);
  DocPrint(page,"$$PURPLE$$$$TX+CX,\"AiwniosTube\"$$$$FD$$\n");
  DocPrint(page,"\n$$HC,\"<video controls width=640 height=480><source src='/Videos/%Q'></video>\"$$",tnam);
  DocPrint(page,"\n$$TR-C,\"%Q\"$$$$ID,2$$\n",ttitle);
  DocPrint(page,"%Q\n$$ID,-2$$\n",tdesc);
  //Save vidoe
  filename=ChrootFile(tnam,SERVER_WWW"/Videos");
  Free(tnam);
  StrUtil(filename,SUF_REM_SPACES);
  if(FileFind(filename))
    goto exists;
  
  FileWrite(filename,file->user_data1,file->user_data0);
  AiwniosTubeSetVideoPropStr(ttitle,"Props/VideoPath","%s",filename);
  Free(filename);
//Save Video page
  tnam=MStrPrint("%s.DD",ttitle);
  StrUtil(tnam,SUF_REM_SPACES);
  filename=ChrootFile(tnam,SERVER_WWW"/Videos");
  EnsurePathExists(tnam);
  as_txt=DocSave(page,&len);
  FileWrite(filename,as_txt,len);
  Free(as_txt);
  Free(filename);

  DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"Video Uploaded:\"$$$$FD$$\n");
  DocPrint(tmp,"\n Video \"%Q\" uploaded in $$LK,\"aiwnios.com/%Q\",HTML=\"/ViewVideo.HC?view=%Q\"$$\n",filename,tnam,tnam);
  Free(tnam);
fin:;
  FramePtrDel(session->user_data1,adam_task);
  StrCpy(con->response_mime,"text/html");
  con->response_code=200;
  WriteDocToStream(stream,tmp);
  DocDel(tmp);
}
