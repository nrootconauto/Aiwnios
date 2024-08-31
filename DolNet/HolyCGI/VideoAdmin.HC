#if __CMD_LINE__
#include "../run.HC";
#endif
extern Bool OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req); 
extern Bool OnPost(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable *); 

class CVideoAdmin {
  U8 video_admin_pass[STR_LEN] format "$$DA-P,\"Admin Password for this Video:%s\"$$"   html_input_type "password";
  Bool video_disable_comments format "$$CB,\"Disable Comments\"$$\n";
  Bool video_delete format "$$CB,\"Delete Video\"$$\n";

};
U8 *action="/VideoAdmin.HC";

Bool OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  CDoc *tmp_doc,*html,*a_with_path;
  I64 as_txt;
  I64 len;
  U8 *v;
  CConnection *con=Fs->user_data;
  if(v=GetQueryValue(url->query,"video")) {
    tmp_doc=DocNew;
    a_with_path=MStrPrint("%s?video=%s",action,v);
    DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Admin Panel for \\\"%Q\\\"\"$$$$FD$$\n",v);
    DocPrint(tmp_doc,"\n  You will need to enter the Video's admin password to make changes.\n");
    DocPrint(tmp_doc,"\n  Aiwnios.com doenst track it's users so Video passwords are per video as aiwnios.com doesnt do accounts\n");
    html=HtmlFormGen("CVideoAdmin",a_with_path,"POST",tmp_doc);
    Free(a_with_path);
    as_txt=DocSave(html,&len);
    WriteNBytes(stream,NULL,as_txt,len);
    Free(as_txt);
    DocDel(html);
    DocDel(tmp_doc);
    StrCpy(con->response_mime,"text/html");
    con->response_code=200;
    return TRUE;
  }
  return FALSE;
}

Bool OnPost(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable *post_data) {
  Bool disable_comments;
  Bool delete;
  U8 *v,*props,*as_txt,*video_dd_path,*comments_path;
  U8 *tpass;
  I64 len;
  CDoc *tmp_doc,*html;
  CHashGeneric *pass=HashFind("video_admin_pass",post_data,HTT_DICT_WORD);
  CConnection *con=Fs->user_data;
  if(!pass) return FALSE;
  if(v=GetQueryValue(url->query,"video")) {
    tmp_doc=DocNew;

    tpass=pass->user_data1;
    tpass[pass->user_data0]=0;

    props=ChrootFile(v,SERVER_META"/VideoProps");
    if(!FileFind(props)) {
      DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Can't find vidoe \\\"%Q\\\"\"$$$$FD$$\n",v);
      Free(props);
      goto flush;
    }
    if(HashStr(tpass)!=DocTreeFExe(props,"Props/AdminHash")) {
      DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Invalid Password for \\\"%Q\\\"\"$$$$FD$$\n",v);
      DocPrint(tmp_doc,"\n  Go mess with your own videos.\n");
      Free(props);
      goto flush;
    }

    DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Adminstation Successful.\"$$$$FD$$\n");

 
    disable_comments=FALSE;
    if(HashFind("video_disable_comments",post_data,HTT_DICT_WORD))
      disable_comments=TRUE;
    delete=FALSE;
    if(HashFind("video_delete",post_data,HTT_DICT_WORD))
      delete=TRUE;

    video_dd_path=ChrootFile(v,SERVER_WWW"/Videos");
    comments_path=ChrootFile(v,SERVER_META"/VideoComments");

    if(disable_comments) {
      DocPrint(tmp_doc,"\n  Comments Disabled.\n");
      Del(comments_path);
    }
    DocTreeFWrite(props,"/Props/DisableComments","%d;\n",disable_comments);
    if(delete) {
      DocPrint(tmp_doc,"\n  Video Deleted.\n");
      Del(props);
      Del(video_dd_path);
      Del(comments_path);
    }
    Free(video_dd_path);
    Free(comments_path);
   Free(props);
    Free(v);
flush:
    html=Doc2Html(tmp_doc);
    as_txt=DocSave(html,&len);
    WriteNBytes(stream,NULL,as_txt,len);
    Free(as_txt);
    DocDel(html);
    DocDel(tmp_doc);
    StrCpy(con->response_mime,"text/html");
    con->response_code=200;
    
    return TRUE;
  }
  return FALSE;
}