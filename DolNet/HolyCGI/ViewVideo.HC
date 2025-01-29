#if __CMD_LINE__
#include "../run.HC";
#endif
#exe {Cd(__DIR__);};;
#include "AiwnTubeCommon";
//Example HolyCGI script
//GET
extern Bool OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req); 

class CVideoComment {
  U8 from[STR_LEN] format "$$DA-P,\"From:%s\"$$";
  U8 message[STR_LEN] format "$$DA-P,\"Message:%s\"$$" html_input_type "textbox";
};

Bool OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  U8 *v,*tmp_str,*tmp_str2,*as_txt,*props_file,*desc;
  I64 len,views;
  U32 n;
  Bool comments_enable=TRUE;
  CDirEntry *ent,*root;
  CDoc *tmp_doc,*html,*comments,*props;
  CConnection *con=Fs->user_data;
  if(v=GetQueryValue(url->query,"view")) {
    tmp_str=ChrootFile(v,SERVER_WWW"/Videos");
    if(FileFind(tmp_str)) {
//Predefined(maybe change this in the future)
      tmp_doc=DocRead(tmp_str);
//Load Comments
      tmp_str2=ChrootFile(v,SERVER_META"/VideoComments");
      EnsurePathExists(tmp_str2);
      StrUtil(tmp_str2,SUF_REM_SPACES);
      if(FileFind(tmp_str2)) {
        comments=DocRead(tmp_str2);
      } else
	comments=NULL;
      Free(tmp_str2);

      if(AiwniosTubeGetVideoProp(v,"Props/DisableComments"))
	comments_enable=FALSE;

      views=AiwniosTubeGetVideoProp(v,"Props/ViewCnt");
      AiwniosTubeSetVideoPropI64(v,"Props/ViewCnt",views+1);

      tmp_str2=MStrPrint("/VideoComment.HC?for=%Q",v);
      DocPrint(tmp_doc,"$$GREEN$$This video has $$RED$$%d$$GREEN$$ views$$FD$$\n",views+1);

      DocPrint(tmp_doc,"$$LK,\"Admin Panel\",HTML=\"/VideoAdmin.HC?video=%Q\"$$\n",v);

      if(comments_enable) {
        DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Comments\"$$$$FD$$");
        html=HtmlFormGen("CVideoComment",tmp_str2,,tmp_doc,comments,&n);
      } else {
        DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Comments Disabled\"$$$$FD$$");
        html=Doc2Html(tmp_doc);
      }

      as_txt=DocSave(html,&len);
      StrCpy(con->response_mime,"text/html");
      con->response_code=200;
      WriteNBytes(stream,NULL,as_txt,len);
      DocDel(html);
      DocDel(tmp_doc);
    } else {
      tmp_doc=DocNew;
      DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"I coudln't find that video:\"$$$$FD$$\n");
      DocPrint(tmp_doc,"\n  Video \"%Q\" not found.\n",v);

      StrCpy(con->response_mime,"text/html");
      con->response_code=200;
      WriteDocToStream(stream,tmp_doc);
      DocDel(tmp_doc);
    }
    Free(tmp_str);
    Free(v);
    return TRUE;
  } else {
    tmp_doc=DocNew;
    DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"AiwniosTube:\"$$$$FD$$\n");
    DocPrint(tmp_doc,"\n  Welcome to AiwniosTube,upload a video or explore.\n");
    DocPrint(tmp_doc,"\n  Click $$LK,\"Here to Upload a Video\",A=\"/VideoUpload.HC\"$$.\n"
                       "  Use slashes to categorize and group your videos,like Compiler/Lexer.\n");
    DocPrint(tmp_doc,"\n  Click $$LK,\"Here to find a video\",A=\"/VideoSearch.HC\"$$\n");

    tmp_str=ChrootFile("/",SERVER_WWW"/Videos");
    v=MStrPrint("%s/*.DD",tmp_str);
    Free(tmp_str);
    root=FilesFind(v,FUF_FLATTEN_TREE|FUF_JUST_FILES|FUF_RECURSE);
    Free(v);

    for(ent=root;ent;ent=ent->next) {
      v=MStrPrint("Videos/%Q",ent->name);


//TODO encode strange names
      desc=NULL;
      if(tmp_str=StrMatch("Videos/",ent->full_name)) {
	desc=AiwniosTubeGetVideoProp(tmp_str+StrLen("Videos/"),"Props/Description");
        DocTreeWrite(tmp_doc,tmp_str,FALSE,"$$LK,\"%Q\",HTML=\"/ViewVideo.HC?view=%Q\"$$\n%Q\n",ent->name,tmp_str+StrLen("Videos/"),desc);
      }
      Free(v),Free(desc);
    }
    DirEntryDel(root);
    DocCollapse(FALSE,tmp_doc);

    StrCpy(con->response_mime,"text/html");
    con->response_code=200;
    WriteDocToStream(stream,tmp_doc);
    DocDel(tmp_doc);    
    return TRUE;
  }
  return FALSE;
}
