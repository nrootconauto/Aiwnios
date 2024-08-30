#if __CMD_LINE__
 #include "../run.HC";
#endif
//Example HolyCGI script
//GET
extern U0 OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req); 
//POST
extern U0 OnPost(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable*);

class CVideoUpload {
  U8 video_filename[32] format "$$DA-P,\"Video File:%s\"$$\n" html_input_type "file";
  U8 video_title[STR_LEN] format "$$DA-P,\"Video Tile:%s\"$$\n";
  U8 video_desc[STR_LEN] format "$$DA-P,\"Description:%s\"$$\n";
  Bool video_disable_comments format "$$CB,\"Disable Comments\"$$\n";
};

U8 *action="/VideoUpload.HC";
U0 OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  I64 len;
  U8 *as_txt;
  CDoc *tmp=DocNew,*html;
  CConnection *con=Fs->user_data;
  DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"AiwniosTube Video Upload:\"$$$$FD$$\n");
  DocPrint(tmp,"\n  Upload a video to AiwniosTube. No illegal content.\n");
  html=HtmlFormGen("CVideoUpload",action,"POST",tmp);
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
  CHashGeneric *disable;
  CDoc *tmp=DocNew,*html,*props;
  CDoc *page=DocNew;
  I64 len;
  U8 *as_txt,*tmp_str2;
  U8 *filename,*tnam,*tdesc,*ttitle;
  CConnection *con=Fs->user_data;
  if(file&&title&&desc) {


    if(file->user_data0<MAX_UPLOAD) {
      if(tnam=FileExtDot(file->user_data2)) {
	if(StrICmp(tnam,".mp4")) {
          DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"Video Needs to be an mp4:\"$$$$FD$$\n");
	  goto fin;
	}
      }

      ttitle=title->user_data1;
      tdesc=desc->user_data1;
      ttitle[title->user_data0]=0;
      tdesc[desc->user_data0]=0;

      tmp_str2=ChrootFile(ttitle,SERVER_WWW"/VideoProps");
      StrUtil(tmp_str2,SUF_REM_SPACES);
      EnsurePathExists(tmp_str2);
      as_txt=MStrPrint("%s.DD",tmp_str2);
      props=DocNew(as_txt);
      Free(tmp_str2);
      if(disable=HashFind("video_disable_comments",post_data,HTT_DICT_WORD)) {
        DocTreeWrite(props,"Props/DisableComments",FALSE,"%d;",1);
      }
      DocTreeWrite(props,"Props/Description",FALSE,"StrNew(\"%Q\");\n",tdesc);
      DocWrite(props);
      Free(as_txt);
      DocDel(props);

//Generate Page HTML(TODO make a HolyCGI script for this)
      DocPrint(page,"$$PURPLE$$$$TX+CX,\"AiwniosTube\"$$$$FD$$\n");
      DocPrint(page,"\n$$HC,\"<video controls width=640 height=480><source src='/Videos/%Q'></video>\"$$",file->user_data2);
      DocPrint(page,"\n$$TR-C,\"%Q\"$$$$ID,2$$\n",ttitle);
      DocPrint(page,"%Q\n$$ID,-2$$\n",tdesc);
      
      filename=file->user_data2;
      tnam=MStrPrint("/Videos/%s",filename);
      EnsurePathExists(tnam);
//Save vidoe
      filename=ChrootFile(tnam);
      Free(tnam);
      StrUtil(filename,SUF_REM_SPACES);
      FileWrite(filename,file->user_data1,file->user_data0);
      Free(filename);
//Save Video page
      tnam=MStrPrint("/Videos/%s.DD",ttitle);
      StrUtil(tnam,SUF_REM_SPACES);
      filename=ChrootFile(tnam);
      EnsurePathExists(tnam);
      as_txt=DocSave(page,&len);
      FileWrite(filename,as_txt,len);
      Free(as_txt);

      DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"Video Uploaded:\"$$$$FD$$\n");
      DocPrint(tmp,"\n Video \"%Q\" uploaded in $$LK,\"aiwnios.com/%Q\",HTML=\"/ViewVideo.HC?view=%Q\"$$\n",filename,tnam,tnam+StrLen("/Videos"));
      Free(filename);
      Free(tnam);
    } else {
      DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"Video Too Drake Massive:\"$$$$FD$$\n");
      DocPrint(tmp,"\n  Video was size %d,Limit is %d bytes.\n",file->user_data0,MAX_UPLOAD);
    }
  } else {
    DocPrint(tmp,"$$PURPLE$$$$TX+CX,\"Invalid Response:\"$$$$FD$$\n");
  }
fin:;
  StrCpy(con->response_mime,"text/html");
  con->response_code=200;

  html=Doc2Html(tmp);
  as_txt=DocSave(html,&len);
  WriteNBytes(stream,NULL,as_txt,len);
  Free(as_txt);
  DocDel(page);
  DocDel(html);
  DocDel(tmp);
}