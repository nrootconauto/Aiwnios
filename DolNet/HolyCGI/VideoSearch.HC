#if __CMD_LINE__
#include "../run.HC";
#endif
//GET
extern Bool OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req); 

class CVideoSearch {
  U8 look_for[STR_LEN] format "$$DA-P,\"Look for text:%s\"$$";
};

Bool OnGet(CServer *srv,I64 stream,CURL *url,CHTTPRequest *req) {
  U8 *v,*tmp_str;
  U8 *as_txt;
  U8 *props_file,*desc;
  I64 len;
  I64 cnt=0;
  CDirEntry *ent,*root;
  CDoc *tmp_doc,*html;
  if(v=GetQueryValue(url->query,"look_for")) {
    tmp_doc=DocNew;
    DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"Search results for \\\"%Q\\\":\"$$$$FD$$\n",v);
    tmp_str=ChrootFile("/",SERVER_WWW"/Videos");
    StrUtil(tmp_str,SUF_REM_SPACES);
    as_txt=MStrPrint("%s*.DD",tmp_str);
    Free(tmp_str);
    root=FilesFind(as_txt,FUF_FLATTEN_TREE|FUF_JUST_FILES|FUF_RECURSE);
    Free(as_txt);
    for(ent=root;ent;ent=ent->next) {
      as_txt=FileRead(ent->full_name);
      if(StrIMatch(v,as_txt)) {
        if(tmp_str=StrMatch("Videos/",ent->full_name)) {
	  props_file=ChrootFile(tmp_str+StrLen("Videos/"),SERVER_META"/VideoProps");
          StrUtil(props_file,SUF_REM_SPACES);
	  desc=NULL;
	  if(FileFind(props_file))
	    desc=DocTreeFExe(props_file,"/Props/Description");
	  Free(props_file);
	  cnt++;
	  DocTreeWrite(tmp_doc,tmp_str,FALSE,"$$LK,\"%Q\",HTML=\"/ViewVideo.HC?view=%Q\"$$\n%Q\n",ent->name,tmp_str+StrLen("Videos/"),desc);
	  Free(desc);
	}
      }
      Free(as_txt);
    }
    if(!cnt) {
      DocPrint(tmp_doc,"\n  Sorry,nothing was found 21.\n");
    }
    DirEntryDel(root);
    Free(v);


    DocCollapse(FALSE,tmp_doc);
    html=Doc2Html(tmp_doc);
    goto flush;
  } else {
    tmp_doc=DocNew;
    DocPrint(tmp_doc,"$$PURPLE$$$$TX+CX,\"AiwniosTube:\"$$$$FD$$\n");
    html=HtmlFormGen("CVideoSearch","VideoSearch.HC","GET",tmp_doc);

flush:;
    CConnection *con=Fs->user_data;
    as_txt=DocSave(html,&len);
    WriteNBytes(stream,NULL,as_txt,len);
    Free(as_txt);
    DocDel(html);

    DocDel(tmp_doc);    
    return TRUE;
  }
  return FALSE;
}