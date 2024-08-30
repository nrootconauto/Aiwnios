#if __CMD_LINE__
#include "../run.HC";
#endif
//POST
extern Bool OnPost(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable *post_data); 
class CVideoComment {
  U8 from[STR_LEN] format "$$DA-P,\"From:%s\"$$";
  U8 message[STR_LEN] format "$$DA-P,\"Message:%s\"$$" html_input_type "textbox";
};

Bool OnPost(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req,CHashTable *post_data) {
  U8 *v,*tmp_str,*props_file;
  U8 *as_txt;
  I64 len;
  I64 cnt=0;
  CDirEntry *ent,*root;
  CDoc *tmp_doc,*html,*comments;
  CHashGeneric *from=HashFind("from",post_data,HTT_DICT_WORD),*message=HashFind("message",post_data,HTT_DICT_WORD);
  if(!from||!message)
    return FALSE;
  if(v=GetQueryValue(url->query,"for")) {
    tmp_doc=DocNew;
    props_file=ChrootFile(v,SERVER_WWW"/VideoProps");
    if(DocTreeFExe(props_file,"Props/DisableComments")) {
      DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Comments are disabled for this video\"$$$$FD$$\n",v);
      DocPrint(tmp_doc,"\n fucking ass poodles.\n");
      goto flush;
    }

    DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Commented on Video(\\\"%Q\\\")\"$$$$FD$$\n",v);
    DocPrint(tmp_doc,"\n 21 Savage Lit($$LK,\"Return to video\",HTML=\"/ViewVideo.HC?view=%Q\"$$)\n",v);
    tmp_str=ChrootFile(v,SERVER_WWW"/VideoComments");
    EnsurePathExists(tmp_str);
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
    html=Doc2Html(tmp_doc);
    as_txt=DocSave(html,&len);
    WriteNBytes(stream,NULL,as_txt,len);
    Free(as_txt);
    DocDel(html);
    return TRUE;
  }
//TODO error mesg
  return FALSE;
}