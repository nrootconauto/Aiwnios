#if __CMD_LINE__
#include "../run.HC";
#endif
//Example HolyCGI script
//GET
extern Bool OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req); 

class CVideoComment {
  U8 from[STR_LEN] format "$$DA-P,\"From:%s\"$$";
  U8 message[STR_LEN] format "$$DA-P,\"Message:%s\"$$" html_input_type "textbox";
};

Bool OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  U8 *v,*tmp_str,*tmp_str2;
  U8 *as_txt;
  I64 len,views;
  Bool comments_enable=TRUE;
  CDirEntry *ent,*root;
  CDoc *tmp_doc,*html,*comments,*props;
  if(v=GetQueryValue(url->query,"view")) {
    tmp_str=ChrootFile(v,SERVER_WWW"/Videos");
    if(FileFind(tmp_str)) {
//Predefined(maybe change this in the future)
      tmp_doc=DocRead(tmp_str);
//Load Comments
      tmp_str2=ChrootFile(v,SERVER_WWW"/VideoComments");
      EnsurePathExists(tmp_str2);
      if(FileFind(tmp_str2)) {
        comments=DocRead(tmp_str2);
      } else
	comments=NULL;
      Free(tmp_str2);


      tmp_str2=ChrootFile(v,SERVER_WWW"/VideoProps");
      EnsurePathExists(tmp_str2);
      if(FileFind(tmp_str2)) {
use_props:
	if(DocTreeFExe(tmp_str2,"Props/DisableComments"))
	  comments_enable=FALSE;
	DocTreeFWrite(tmp_str2,"Props/ViewCnt","%d;",views=DocTreeFExe(tmp_str2,"Props/ViewCnt")+1);
      } else {
	FileWrite(tmp_str2,"",0);
        goto use_props;
      }
      Free(tmp_str2);

      tmp_str2=MStrPrint("/VideoComment.HC?for=%Q",v);
      DocPrint(tmp_doc,"$$GREEN$$This video has $$RED$$%d$$GREEN$$ views$$FD$$\n",views);


      if(comments_enable) {
        DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Comments\"$$$$FD$$");
        html=HtmlFormGen("CVideoComment",tmp_str2,"POST",tmp_doc,comments);
      } else {
        DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Comments Disabled\"$$$$FD$$");
        html=Doc2Html(tmp_doc);
      }
      as_txt=DocSave(html,&len);
      WriteNBytes(stream,NULL,as_txt,len);
      Free(as_txt);


      DocDel(html);
      DocDel(tmp_doc);
    } else {
      tmp_doc=DocNew;
      DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"I coudln't find that video:\"$$$$FD$$\n");
      DocPrint(tmp_doc,"\n  Video \"%Q\" not found.\n",v);
      html=Doc2Html(tmp_doc);
      as_txt=DocSave(html,&len);
      WriteNBytes(stream,NULL,as_txt,len);
      DocDel(html);
      DocDel(tmp_doc);
    }
    Free(tmp_str);
    Free(v);
    return TRUE;
  } else {
    tmp_doc=DocNew;
    DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"AiwniosTube:\"$$$$FD$$\n");
    DocPrint(tmp_doc,"\n  Welcome to AiwniosTube,upload a video or explore.\n");
    DocPrint(tmp_doc,"\n  Click $$LK,\"Here to Upload a Video\",A=\"/VideoUpload.HC\"$$ .\n");
    DocPrint(tmp_doc,"\n  Click $$LK,\"Here to find a video\",A=\"/VideoSearch.HC\"$$\n");

    tmp_str=ChrootFile("/",SERVER_WWW"/Videos");
    v=MStrPrint("%s/*.DD",tmp_str);
    Free(tmp_str);
    root=FilesFind(v,FUF_FLATTEN_TREE|FUF_JUST_FILES|FUF_RECURSE);

    for(ent=root;ent;ent=ent->next) {
      v=MStrPrint("Videos/%Q",ent->name);
//TODO encode strange names
      if(tmp_str=StrMatch("Videos/",ent->full_name))
        DocTreeWrite(tmp_doc,tmp_str,FALSE,"$$LK,\"%Q\",HTML=\"/ViewVideo.HC?view=%Q\"$$\n",ent->name,tmp_str+StrLen("Videos/"));
      Free(v);
    }
    DirEntryDel(root);

    DocCollapse(FALSE,tmp_doc);

    html=Doc2Html(tmp_doc);
    as_txt=DocSave(html,&len);
    WriteNBytes(stream,NULL,as_txt,len);
    Free(as_txt);
    DocDel(html);

    DocDel(tmp_doc);    
    return TRUE;
  }
  return FALSE;
}
