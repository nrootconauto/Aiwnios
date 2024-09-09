#if __CMD_LINE__
#include "../run.HC";
#endif
#exe {Cd(__DIR__);};;
#include "AiwnTubeCommon";
//POST
extern Bool OnPost(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable *post_data);
class CVideoComment {
  U8 from[STR_LEN] format "$$DA-P,\"From:%s\"$$";
  U8 message[STR_LEN] format "$$DA-P,\"Message:%s\"$$" html_input_type "textbox";
};

Bool OnPost(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable *post_data) {
  U8 *v,*tmp_str,*props_file,*as_txt;
  I64 len,cnt=0,captchanum;
  U32 num;
  CDirEntry *ent,*root;
  CDoc *tmp_doc,*html,*comments;
  CHashGeneric *from=HashFind("from",post_data,HTT_DICT_WORD),
	       *message=HashFind("message",post_data,HTT_DICT_WORD),
	       *captcha=HashFind("captcha",post_data,HTT_DICT_WORD),
	       *session=HashFind("session",post_data,HTT_DICT_WORD);
  CConnection *con=Fs->user_data;
  if(!from||!message||!captcha||!session)
    return FALSE;
  if(v=GetQueryValue(url->query,"for")) {
    tmp_doc=DocNew;
    if(AiwniosTubeGetVideoProp(v,"/Props/DisableComments")) {
      DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Comments are disabled for this video\"$$$$FD$$\n",v);
      DocPrint(tmp_doc,"\n fucking ass poodles.\n");
      goto flush;
    }
    num=FramePtr(session->user_data1,adam_task);
    StrScan(captcha->user_data1,"%X",&captchanum);
    if(num!=captchanum) {
      DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Captcha incorrect\"$$$$FD$$\n");
      goto flush;
    }

    DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Commented on Video(\\\"%Q\\\")\"$$$$FD$$\n",v);
    DocPrint(tmp_doc,"\n 21 Savage Lit($$LK,\"Return to video\",HTML=\"/ViewVideo.HC?view=%Q\"$$)\n",v);
    tmp_str=ChrootFile(v,SERVER_META"/VideoComments");
    EnsurePathExists(tmp_str);
    StrUtil(tmp_str,SUF_REM_SPACES);
    if(FileFind(tmp_str)) {
      comments=DocRead(tmp_str);
    } else
      comments=DocNew(tmp_str);
    DocTop(comments);
    DocPrint(comments,"$$PURPLE$$$$TX,\"Comment from %Q\"$$$$FD$$\n",from->user_data1);
    DocPrint(comments,"$$ID,2$$\n");
    DocPrint(comments,"%Q\n",message->user_data1);
    DocPrint(comments,"$$ID,-2$$\n");
    DocWrite(comments);
    DocDel(comments);

flush:;
    FramePtrDel(session->user_data1,adam_task);
    WriteDocToStream(stream,tmp_doc);
    StrCpy(con->response_mime,"text/html");
    con->response_code=200;    
    return TRUE;
   }
//TODO error mesg
  return FALSE;
}
