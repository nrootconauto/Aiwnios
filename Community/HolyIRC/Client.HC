1#exe {Cd(__DIR__);};;
#ifdef __AIWNIOS__
#include "CrossNetShims/Aiwnios2.HC";
#else
#ifdef __EXODUS
#include "CrossNetShims/EXODUS.HC";
#else
#endif
#endif
/**
 * Type '/JOIN #anonchan' after the Client logs you in
 * Usage ClientNew(hn="chat.freenode.org",port=6667,user_name);
 * I don't know how to use IRC so you are on your own
 */
class CClient {
  I64 srv;
  U8 name[STR_LEN];
  U8 passwd[STR_LEN];
  U8 host[STR_LEN];
  U8 channel[STR_LEN];
  CDoc *doc;
  CDocEntry *prompt;
  CTask *iconify_task;
  CTask *win_task;
  CTask *bg_thread,*left_side,*right_side;
  Bool shown;
  CDoc *user_list_doc;
  CDoc *user_list_doc2;
  CDoc *channel_list_doc;
  CDoc *channel_list_doc2;
  Bool first_run,disconnected,is_op;
  U8 *send_filename;
  U8 *send_filename_full;
};


U0 ToggleClientShow(CClient* cl) {
  I64 i,b;
  CTask *win_task;
  CTask *t,*left_side,*right_side;
  cl->shown^=TRUE;
  for(i=0;i!=3;i++) {
    switch(i) {
       case 0:
       t=cl->win_task;
       break; 
       case 1:
       t=cl->left_side;
       break; 
       case 2:
       t=cl->right_side;
       break; 
    }
    if(TaskValidate(t)) {
      if(!cl->shown) {
        t->win_inhibit=1<<WIf_SELF_FOCUS;
	LBtr(&t->display_flags,DISPLAYf_SHOW);
      } else {
	LBts(&t->display_flags,DISPLAYf_SHOW);
        t->win_inhibit=WIG_USER_TASK_DFT;
        if(!i) WinFocus(t); //win_task
        else WinToTop(t);
      }
    }
  }
  WinZBufUpdate;
}
U0 IconifyTask(CClient *cl) {
  cl->iconify_task=Fs;
  I64 mnx,mny,mxx,mxy,w,h;
  I64 last=ms.lb;
  SpriteExtents($IB,"<1>",BI=1$,&mnx,&mxx,&mny,&mxy);
  w=mxx-mnx;
  h=mxy-mny;
  while (TRUE) {
    DCFill(gr.dc);
    if(Blink) Sprite3(gr.dc,GR_WIDTH,GR_HEIGHT-50,0,$IB,"<1>",BI=1$);
    Refresh;
    if(ms.lb&&!last){
      if(GR_WIDTH-w<=ms.pos.x<=GR_WIDTH) {
        if(GR_HEIGHT-50-h<=ms.pos.y<=GR_HEIGHT-50) {
	  ToggleClientShow(cl);
        }
      }
    }
    last=ms.lb;
  }
}


U0 FreeLeftExprs(CDoc *doc) {
  CDocEntry *ent=doc->head.next;
  while(ent!=doc) {
    if(ent->de_flags&(1<<DOCEf_LEFT_EXP))
      Free(ent->left_exp);
    ent=ent->next;
  }
}
Bool DCCSend(CClient *cl,U8 *who) {
  U8 *send_file_name=PopUpPickFile;
  U8 *buffer,*tmp;
  CDirEntry de;
  if(!send_file_name)
     return FALSE;
  if(FileFind(send_file_name,&de)) {
    if(de.attr&RS_ATTR_DIR) {
      RedSeaISO("/IRCTmp.ISO.C",de.full_name);
      send_file_name=StrNew("/IRCTmp.ISO.C");
      Free(de.full_name);
      FileFind(send_file_name,&de);
    }
    tmp=MStrPrint("PRIVMSG %s :\x01DCC SEND %Q 0 0 %d\x01\r\n",who,de.name,de.size);
    send(cl->srv,tmp,StrLen(tmp),0);
    cl->send_filename_full=StrNew(de.full_name,cl->win_task);
    cl->send_filename=StrNew(de.name,cl->win_task);
    Free(de.full_name);
    Free(send_file_name);
    return TRUE;
  }
  Free(send_file_name);
  return FALSE;
}
Bool DCCDownload(U8 *from,U8 *file,I64 host,I64 port,I64 fsz) {
  I64 ptr=0,ofsz=fsz,cnt;
  U8 *buf;
  U8 *tmp=MStrPrint("Do you want to download '%s' from '%s'\n",file,from);
  if(!PopUpNoYes(tmp,"Look in 'T:/Downloads' when done")) {
     Free(tmp);
     return FALSE;
  }
  Free(tmp);
  buf=CAlloc(fsz);
  if(!FileFind("T:/Downloads")) DirMk("T:/Downloads");
  U8 host_str=MStrPrint("%d.%d.%d.%d",host.u8[3],host.u8[2],host.u8[1],host.u8[0]);
  I64 sock=create_connection(host_str,port);
  if(sock>0) {
    while(fsz>0&&(cnt=recv(sock,buf+ptr,fsz,0))>0) {
      ptr+=cnt;
      fsz-=cnt;
      Sleep(10);
    }
    if(!fsz) {
      tmp=MStrPrint("T:/Downloads/%s",file);
      FileWrite(tmp,buf,ofsz);
      Free(buf),Free(tmp);
      close(sock);
      Free(host_str);
      return TRUE;
    } else 
     PopUpOk("File transfer failed");
    close(sock);
  }
  Free(host_str);
  Free(buf);
  return  FALSE;
}
U0 RightSideBarTask2(CClient *cl)
{
  CTask *parent=Fs->parent_task;
  while (TRUE) {
    WinHorz(cl->win_task->win_right+2,TEXT_COLS-1,parent);
    WinVert(cl->win_task->win_top,cl->win_task->win_bottom,parent);
    Refresh;
  }
}
U0 LeftSideBarTask2(CClient *cl)
{
  CTask *parent=Fs->parent_task;
  while (TRUE) {
    WinHorz(1,cl->win_task->win_left-2,parent);
    WinVert(cl->win_task->win_top,cl->win_task->win_bottom,parent);
    Refresh;
  }
}
U0 RightSideBarTask(CClient *cl) {
  CTask *p=cl->win_task;
  Spawn(&RightSideBarTask2,cl,,0,Fs);
  DocTermNew;
  if(cl->shown) {
    Fs->win_inhibit=WIG_USER_TASK_DFT;
    LBts(&(Fs->display_flags),DISPLAYf_SHOW);
  } else
    Fs->win_inhibit=1<<WIf_SELF_FOCUS;
  WinToTop;
  WinZBufUpdate;
again:
  DocClear(DocPut);
  DocInsDoc(DocPut,cl->user_list_doc2);
  DocCollapse(FALSE,DocPut);
  WinFocus(cl->win_task);
  I64 v=DocMenu(DocPut);
  if(!TaskValidate(p))
    Exit;
  if(v==DOCM_CANCEL)
    goto again;
  XTalk(cl->win_task,"%s\n",StrNew(v));
  goto again;
}
U0 LeftSideBarTask(CClient *cl) {
  CTask *p=cl->win_task;
  Spawn(&LeftSideBarTask2,cl,,0,Fs);
  DocTermNew;
  if(cl->shown) {
    Fs->win_inhibit=WIG_USER_TASK_DFT;
    LBts(&(Fs->display_flags),DISPLAYf_SHOW);
  } else
    Fs->win_inhibit=1<<WIf_SELF_FOCUS;
  WinToTop;
  WinZBufUpdate;
again:
  DocClear(DocPut);
  DocInsDoc(DocPut,cl->channel_list_doc2);
  DocCollapse(FALSE,DocPut);
  WinFocus(cl->win_task);
  I64 v=DocMenu(DocPut);
  if(!TaskValidate(p))
    Exit;
  if(v==DOCM_CANCEL)
    goto again;
  XTalk(cl->win_task,"%s\n",StrNew(v));
  goto again;
}

U0 RefreshSidebars(CClient *cl) {
  WinHorz(16,TEXT_COLS-16,cl->win_task);
  if(cl->right_side) Kill(cl->right_side,FALSE);
  if(cl->left_side) Kill(cl->left_side,FALSE);
  cl->right_side=Spawn(&RightSideBarTask,cl);
  cl->left_side=Spawn(&LeftSideBarTask,cl);
}
U0 ClientPrint(CClient *cl,U8 *fmt,...) {
  //StrPrintJoin takes argc and argc
  U8 *fmted=StrPrintJoin(NULL,fmt,argc,argv);
  CDocEntry *cur=cl->doc->cur_entry;
  DocLock(cl->doc);
  cl->doc->cur_entry=cl->prompt->last;
  DocPrint(cl->doc,"%s",fmted);
  cl->doc->cur_entry=cur;
  DocUnlock(cl->doc);
  Free(fmted);
}
U0 ServerWrite(CClient *cl,U8 *fmt,...) {
  U8 to_send;
  //StrPrintJoin takes argc and argc
  to_send=StrPrintJoin(NULL,fmt,argc,argv);
  sendString(cl->srv,to_send,StrLen(to_send)); 
  sendString(cl->srv,"\r\n",2); 
  Free(to_send);
}
U8 *Skip(U8 *str,U8 chr) {
  while(*str&&*str!=chr)
   str++;
  if(*str!=0)
    *str++=0;
  return str;
}
U0 ServerConnect(I64 s,CClient *cl) {
  if(cl->first_run) {
    cl->first_run=FALSE;
// This is optional
// PASS passwd
//
  if(cl->passwd[0])
    ServerWrite(cl,"PASS %s",cl->passwd);

//
// USER username hostname servername :realname
//
    ServerWrite(cl,"USER %s localhost %s :%s",
	  cl->name,
	  cl->host,
	  cl->name
	  );
//
// NICK namae
//
    ServerWrite(cl,"NICK %s",cl->name);
  }
}
U8 *DblDollars(U8 *s) {
  if(!s) return StrNew("");
  I64 more=StrOcc(s,'\d');
  U8 *ret=MAlloc(1+StrLen(s)+more),ch,*ptr=ret;
  while(ch=*s++) {
    if(ch!='\d')
      *ptr++=ch;
    else {
      *ptr++='\d';
      *ptr++='\d';
    }
  }
  *ptr++=0;
  return ret;
}
U0 ServerReadCb(I64 s,U8 *text,I64 len,U8 *user_data) {
  CClient *cl=user_data;
  U8 *cmd=text,*usr=NULL,*txt=NULL,*par=NULL,*tmp,*chan="",*tmp2,*tmp3;
  U8 dummy[STR_LEN],*dp=dummy;
  U8 topic[STR_LEN],*tp=topic;
  I64 users,flen,ptr,sock,cnt;
  Bool is_op=FALSE;
//
  // Server stuff has this format
  // ':',servername,'!',nick,' ',params \r\n
  // params = ' ',':',stuff,params*
  //
  if(cmd[0]=='@') { //A Tag
    cmd=Skip(cmd,' ');
    if(StrOcc(cmd,':'))
      cmd=StrFirstOcc(cmd,":");
  }
  if(cmd[0]==':') {
//Command ends at space,so we skip it 
    usr=cmd+1; //$BK,1$servername can also be a nickname$BK,0$
    cmd=Skip(cmd,' ');
//User comes after '!'
    Skip(usr,'!');    
  }
  par=Skip(cmd,' ');
  txt=Skip(par,':');
  if(!StrCmp("PONG",cmd)) {
    return;
  }
  if(!StrCmp("NOTICE",cmd)) {
    tmp=DblDollars(txt);
    ClientPrint(cl,"$$RED$$$$BK,1$$%Q$$BK,0$$$$FD$$\n",tmp);
    Free(tmp);
    return;
  }
  if(!StrCmp("PRIVMSG",cmd)) {
//DCCmessage
    if(txt[0]==1) {
      txt++;
      if(!StrNCmp("DCC SEND",txt,!StrLen("DCC SEND"))) {
        U8 *dcc_fn=Skip(txt+StrLen("DCC SEND"),' ');
        tmp=Skip(dcc_fn,' ');
        I64 addrI32=Str2I64(tmp);
        tmp=Skip(tmp,' ');
        I64 port=Str2I64(tmp);
        tmp=Skip(tmp,' ');
        I64 fsz=Str2I64(tmp);
//If we sent a (Reverse) DCC request,the server will return a valid place to write to
        if(cl->send_filename) {
	  if(!StrCmp(cl->send_filename,dcc_fn)) {
	    tmp2=MStrPrint("%d.%d.%d.%d",addrI32.u8[3],addrI32.u8[2],addrI32.u8[1],addrI32.u8[0]);
	    sock=create_connection(tmp2,port);
	    Free(tmp2); 
	    tmp2=FileRead(cl->send_filename_full,&flen);
	    if(sock>0) {
	      while(flen>0&&(cnt=send(sock,tmp2+ptr,flen,0))>0) {
	        ptr+=cnt;
	        flen-=cnt;
	        Sleep(10);
	      }
	      close(sock);
	    }
	    Free(cl->send_filename);
	    Free(cl->send_filename_full);
	    cl->send_filename=NULL;
	    cl->send_filename_full=NULL;
	    Free(tmp2);
	    return;
	  }
        } 
        tmp=MStrPrint("/DCCRECV \"%Q\" \"%Q\" %d %d %d;\n",usr,dcc_fn,addrI32,port,fsz);
        ClientPrint(cl,"<$$MA,\"Download %Q from %Q\",LM=\"%Q\"$$>\n",dcc_fn,usr,tmp);
        Free(tmp);
      }
      return;
    }
    chan=par;
    StrUtil(chan,SUF_REM_SPACES);
    usr=DblDollars(usr);
    chan=DblDollars(chan);
    txt=DblDollars(txt);
    if(StrLen(chan)) {
      ClientPrint(cl,"<$$MA,\"%Q\",LM=\"/join %Q\n\"$$/$$MA,\"%Q\",LM=\"/privmsg %Q\n\"$$> %s\n",chan,chan,usr,usr,txt);
    } else
      ClientPrint(cl,"<$$MA,\"%Q\",LM=\"/privmsg %Q\n\"$$> %s\n",usr,usr,txt);
    Free(usr);
    Free(chan);
    Free(txt);
    return;
  }
  if(!StrCmp("NICK",cmd)) {
    StrUtil(txt,SUF_REM_SPACES);
    if(txt==':') txt++;
    StrCpy(cl->name,txt);
    return;
  }
  if(!StrCmp("PING",cmd)) {
    ServerWrite(cl,"PONG %s",txt);
    return;
  }
  if(!StrCmp("321",cmd)) {
//I StrNew values into them
    FreeLeftExprs(cl->channel_list_doc2);
    DocClear(cl->channel_list_doc);
    DocClear(cl->channel_list_doc2);
    DocPrint(cl->channel_list_doc,"$$PURPLE$$$$TX+CX,\"Channel List\"$$$$FD$$\n");    
    return;
  }
//Nickname is use
  if(!StrCmp("433",cmd)) {
    CatPrint(cl->name,"_"); //Add a '_' to make a "free" nickname
    ServerWrite(cl,"NICK %s",cl->name);
    return;   
  }
  if(!StrCmp("322",cmd)) {
//Skip user
    par=Skip(par,' ');
    dummy[0]=0;
    par=tmp=DblDollars(par);
    txt=DblDollars(txt);
    while(*par&&!Bt(char_bmp_white_space,*par))
      CatPrint(dummy,"%c",*par++);
    DocPrint(cl->channel_list_doc,"$$TR-C,\"%Q\"$$\n$$ID,2$$",dummy);
    DocPrint(cl->channel_list_doc,"$$LTGREEN$$%Q$$FD$$\n",txt);
    DocPrint(cl->channel_list_doc,"$$MA,\"Connect\",LM=\"/join %Q\n\"$$\n",dummy);
    DocPrint(cl->channel_list_doc,"$$ID,-2$$");


    DocPrint(cl->channel_list_doc2,"$$TR-C,\"%Q\"$$\n$$ID,2$$",dummy);
    DocPrint(cl->channel_list_doc2,"$$LTGREEN$$%Q$$FD$$\n",txt);
    tmp3=MStrPrint("/JOIN %Q",dummy);
    DocPrint(cl->channel_list_doc2,"$$RED$$$$MU,\"Connect\",LE=%d$$$$FD$$\n",StrNew(tmp3,cl->win_task));
    Free(tmp3);
    DocPrint(cl->channel_list_doc2,"$$ID,-2$$");

    Free(txt);
    Free(tmp);
    return;
  }
  if(!StrCmp("323",cmd)) {
    cl->channel_list_doc->flags|=DOCF_NO_CURSOR;
    //ClientPrint(cl,"%s\n",tmp=DocSave(cl->channel_list_doc));
    Free(tmp);
    return;
  }
  if(!StrCmp("372",cmd)) {
    tmp=DblDollars(txt);
    ClientPrint(cl,"%s\n",tmp);
    Free(tmp);
    return;
  }
  if(!StrCmp("353",cmd)) {
//Users doc
    DocClear(cl->user_list_doc);
//I StrNew values into them
    FreeLeftExprs(cl->user_list_doc2);
    DocClear(cl->user_list_doc2);
    DocPrint(cl->user_list_doc,"$$PURPLE$$$$TX+CX,\"User List of %Q\"$$$$FD$$\n",par);
    tmp2=txt=DblDollars(txt);
    while(*txt&&Bt(char_bmp_white_space,*txt))
      txt++;
//First pass,check if we are a operator for channel
    while(*txt) {
      dummy[0]=0;
      while(*txt&&!Bt(char_bmp_white_space,*txt))
        CatPrint(dummy,"%c",*txt++);
      while(*txt&&Bt(char_bmp_white_space,*txt))
        txt++;
      tmp=dummy;
      is_op=FALSE;
      if(*tmp=='@') tmp++,is_op=TRUE;
      else if(*tmp=='+') tmp++;
      else if(*tmp=='%') tmp++,is_op=TRUE;
      if(!StrCmp(cl->name,tmp))
	cl->is_op=is_op;
    }
    //2nd pass,make macros
    txt=tmp2;
    while(*txt&&Bt(char_bmp_white_space,*txt))
      txt++;
    while(*txt) {
      dummy[0]=0;
      while(*txt&&!Bt(char_bmp_white_space,*txt))
        CatPrint(dummy,"%c",*txt++);
      while(*txt&&Bt(char_bmp_white_space,*txt))
        txt++;
      tmp=dummy;
      if(*tmp=='@') tmp++;
      else if(*tmp=='+') tmp++;
      else if(*tmp=='%') tmp++;
      DocTreeWrite(cl->user_list_doc,tmp,FALSE,"$$MA,\"Private Message\",LM=\"/PRIVMSG %Q\n\"$$\n",tmp);
      DocTreeAppend(cl->user_list_doc,tmp,FALSE,"$$MA,\"Send File To\",LM=\"/SEND %Q\n\"$$\n",tmp);
      tmp3=MStrPrint("/PRIVMSG %Q",tmp);
      DocTreeWrite(cl->user_list_doc2,tmp,FALSE,"$$RED$$$$MU,\"Private Message\",LE=%d$$\n$$FD$$",StrNew(tmp3,cl->win_task));
      Free(tmp3);
      tmp3=MStrPrint("/SEND %Q",tmp);
      DocTreeAppend(cl->user_list_doc2,tmp,FALSE,"$$RED$$$$MU,\"Send File To\",LE=%d$$\n$$FD$$",StrNew(tmp3,cl->win_task));
      Free(tmp3);
      if(cl->is_op&&cl->channel[0]=='#') {
        //Add operator constrols
        tmp3=MStrPrint("/MODE %Q -v %Q",cl->channel,tmp);
        DocTreeAppend(cl->user_list_doc2,tmp,FALSE,"$$RED$$$$MU,\"Mute\",LE=%d$$\n$$FD$$",StrNew(tmp3,cl->win_task));
        Free(tmp3);
        tmp3=MStrPrint("/MODE %Q +v %Q",cl->channel,tmp);
        DocTreeAppend(cl->user_list_doc2,tmp,FALSE,"$$RED$$$$MU,\"Unmute\",LE=%d$$\n$$FD$$",StrNew(tmp3,cl->win_task));
        Free(tmp3);
        tmp3=MStrPrint("/MODE %Q +o %Q",cl->channel,tmp);
        DocTreeAppend(cl->user_list_doc2,tmp,FALSE,"$$RED$$$$MU,\"Op\",LE=%d$$\n$$FD$$",StrNew(tmp3,cl->win_task));
        Free(tmp3);
        tmp3=MStrPrint("/MODE %Q -o %Q",cl->channel,tmp);
        DocTreeAppend(cl->user_list_doc2,tmp,FALSE,"$$RED$$$$MU,\"Remove Op\",LE=%d$$\n$$FD$$",StrNew(tmp3,cl->win_task));
        Free(tmp3);
      }
    }
    Free(tmp2);
    cl->user_list_doc->flags|=DOCF_NO_CURSOR;
    ClientPrint(cl,"%s\n",tmp=DocSave(cl->user_list_doc));
    Free(tmp);
    return;
  }
  usr=DblDollars(usr);
  cmd=DblDollars(cmd);
  txt=DblDollars(txt);
  ClientPrint(cl,"(%s)>>> %s (%s): %s\n",usr,cmd,par,txt);
  Free(txt);
  Free(cmd);
  Free(usr);
}
U0 ListDialog(CClient *cl) {
  ServerWrite(cl,"LIST");
}
U0 ServerLoop(CClient *cl) {
  U8 buffer[4048+1];
 I64 len;
  while(TRUE) {
    RefreshSidebars(cl);
    len=recvLine(cl->srv,buffer,4048,0);
    if(0>len) {
      cl->disconnected=TRUE;
      ClientPrint(cl,"Disconnected by server\n");
      Exit;
    }
    ServerReadCb(cl->srv,buffer,len,cl);
  }
}
U0 MsgChannel(CClient *cl,U8 *channel,U8 *msg) {
  if(!msg||!StrLen(msg)) return;
  ClientPrint(cl,"<%s> %s\n",channel,msg);
  ServerWrite(cl,"PRIVMSG %s :%s",channel,msg);
}
U0 ExitCb() {
  CClient *cl=FramePtr("client");
  Kill(cl->bg_thread,FALSE);
  Kill(cl->left_side,FALSE);
  Kill(cl->right_side,FALSE);
  close(cl->srv);
  DCFill;
  Exit;
}
CClient *ClientNew(U8 *server="irc.libera.chat",I64 port=6667,U8 *name="Terry",U8 *passwd="") {
  if(!name) name=PopUpGetStr("IRC username:");
  else name=StrNew(name);
  CCmpCtrl *lex;
  Bool cont=TRUE;
  U8 *s,*p;
  U8 who[STR_LEN],msg[STR_LEN];
  I64 i;
  WinMax;
  DocClear;
  CClient *cl=CAlloc(sizeof(CClient));
  FramePtrAdd("client",cl);
  Fs->task_end_cb=&ExitCb;
  cl->win_task=Fs;
  cl->shown=TRUE;
  cl->iconify_task=Spawn(&IconifyTask,cl,,,Fs);
  cl->channel_list_doc=DocNew;
  cl->user_list_doc=DocNew;
  cl->channel_list_doc2=DocNew;
  cl->user_list_doc2=DocNew;
  cl->srv=create_connection(server,port);
  StrCpy(cl->passwd,passwd);
  StrCpy(cl->name,name);
  StrCpy(cl->host,server);
  StrCpy(cl->channel,"<none>");
  cl->doc=DocPut; //The current document
  cl->first_run=TRUE;
  cl->bg_thread=Spawn(&ServerLoop,cl,,,Fs);
  DocPrint(cl->doc,"$$WW,1$$"); //Dummy element
  while(cont&&!cl->disconnected) {
    DocLock(cl->doc);
    DocBottom(cl->doc);
//Display prompt link
    if(cl->channel[0]=='#') {
      cl->prompt=DocPrint(cl->doc,"$$MA,\"(%Q)\",LM=\"/join %Q\n\"$$",cl->channel,cl->channel);
    } else
      cl->prompt=DocPrint(cl->doc,"$$MA,\"(%Q)\",LM=\"/privmsg %Q\n\"$$",cl->channel,cl->channel);
    DocPrint(cl->doc,">>>$$PT$$",cl->channel);
    DocUnlock(cl->doc);
    if(cl->first_run) {
      ListDialog(cl);
      ServerConnect(cl->srv,cl);
    }
    s=GetStr(,"",GSF_SHIFT_ESC_EXIT);
    if(s) {
      //Things without a '/' automagically go to the current channel
      if(s[0]!='/') {
        MsgChannel(cl,cl->channel,s);
      } else {
        if(!StrNICmp(s+1,"PASS",4)) {
	  StrUtil(s+4+1,SUF_REM_LEADING|SUF_REM_TRAILING);
	  ServerWrite(cl,"PASS %s",s+4+1);
	} else if(!StrNICmp(s+1,"NAMES",5)) {
	  ServerWrite(cl,"NAMES : %s",cl->channel);
	} else if(!StrNICmp(s+1,"JOIN",4)) {
	  //If we are in a channel,quit
	  if(*cl->channel) {
	    ServerWrite(cl,"PART %s",cl->channel);
	  }
	  //Join a channel
	  StrCpy(cl->channel,s+5);
	  StrUtil(cl->channel,SUF_REM_LEADING|SUF_REM_TRAILING);
//
// JOIN #channel
//
	  ServerWrite(cl,"JOIN %s",cl->channel);
	//
// TOPIC #channel
//
	  ServerWrite(cl,"TOPIC %s",cl->channel);
        }
        else if(!StrNICmp(s+1,"LIST",4)) {
//
// LIST
//
	  ServerWrite(cl,"LIST");
        }
        else if(!StrNICmp(s+1,"NICK",4)) {
	  ServerWrite(cl,s+1);  
        }
        else if(!StrNICmp(s+1,"WHO",3)) {
//
// NAMES #channel //What do you think it does
//
	  ServerWrite(cl,"NAMES %s",cl->channel);  
	}
        else if(!StrNICmp(s+1,"PART",4)) {
//
// PART #channel,leave a channel
//
	  StrCpy(who,s+5);
	  StrUtil(who,SUF_REM_LEADING|SUF_REM_TRAILING);
	  ServerWrite(cl,"PART %s",who);
	}
        else if(!StrNICmp(s+1,"PRIVMSG",7)) {
	  StrCpy(who,s+7+1);
	  StrUtil(who,SUF_REM_LEADING|SUF_REM_TRAILING);
          ClientPrint(cl,"$$LTGREEN$$Now secret chatting with \"%Q\",click $$MA,\"here(%Q)\",LM=\"/join %Q\n\"$$ to return to the main chat\n",who,cl->channel,cl->channel);
//Leave our other channel
	  ServerWrite(cl,"PART %s",cl->channel);
	  StrCpy(cl->channel,who);
	} else if(!StrNICmp(s+1,"MSG",3)) {
	  //Join a channel
	  i++;
	  StrCpy(who,s+4);
	  StrUtil(who,SUF_REM_LEADING|SUF_REM_TRAILING);
          if(!StrLen(who)) {
	    ClientPrint(cl,"WHODOIMESSAGE.\n");
	    goto next;
          }
          while(who[i]&&!Bt(char_bmp_white_space,who[i]))
            i++;
          who[i]=0;
          if(!who[i+1]) {
	    ClientPrint(cl,"IWANTAMESSAGETOSENDTO'%s'.\n",who);
	    goto next;
          }
          StrCpy(msg,who+i+1);
	  StrUtil(msg,SUF_REM_LEADING|SUF_REM_TRAILING);
	  MsgChannel(cl,who,msg);
        } else if(!StrNICmp(s+1,"PING",4)) {
	  ServerWrite(cl,"PING %s",cl->channel);
	} else if(!StrNICmp(s+1,"MODE",4)) {
	  ServerWrite(cl,"%s",s+1);
	} else if(!StrNICmp(s+1,"SEND",4)) {
	  StrCpy(who,s+5);
	  StrUtil(who,SUF_REM_LEADING|SUF_REM_TRAILING);
          if(!StrLen(who)&&cl->channel=='#') {
            ClientPrint(cl,"$$LTRED$$I need a nickname to send file to.\n");
	  } else if(StrLen(who)) {
	    DCCSend(cl,who);
          } else if(cl->channel[0]!='#'){
	    DCCSend(cl,cl->channel);
          }
	} else if(!StrNICmp(s+1,"DCCRECV",7)) {
          U8 *dfrom=NULL,*dname=NULL;
          lex=CmpCtrlNew(s+8,CCF_DONT_FREE_BUF);
	  Lex(lex);
	  if(lex->token!=TK_STR)
  	    goto downfail;
	  dfrom=StrNew(lex->cur_str);
	  Lex(lex);
	  if(lex->token!=TK_STR)
  	    goto downfail;
	  dname=StrNew(lex->cur_str);
	  Lex(lex);
	  if(lex->token!=TK_I64)
  	    goto downfail;
	  I64 dhost=lex->cur_i64;
	  Lex(lex);
	  if(lex->token!=TK_I64)
  	    goto downfail;
	  I64 dport=lex->cur_i64;
	  Lex(lex);
	  if(lex->token!=TK_I64)
  	    goto downfail;
          I64 dfsz=lex->cur_i64;
	  DCCDownload(dfrom,dname,dhost,dport,dfsz);
downfail:
	  Free(dfrom),Free(dname);
	  CmpCtrlDel(lex);
        } else if(!StrNICmp(s+1,"EXIT",4)) {
	  if(*cl->channel) {
	    ServerWrite(cl,"PART %s",cl->channel);
	  }
	  ServerWrite(cl,"QUIT");
	  ClientPrint(cl,"BYE BYE!!!\n");
	  cont=FALSE;
	} else  {
	  ClientPrint(cl,"/DCCRECV --- Download from a DCC socket.\n");
	  ClientPrint(cl,"/SEND --- SendFile.\n");
	  ClientPrint(cl,"/MODE --- Mode command.\n");
	  ClientPrint(cl,"/PRIVMSG --- Private message a user.\n");
	  ClientPrint(cl,"/PART --- Leave a channel.\n");
	  ClientPrint(cl,"/PASS --- Set a password.\n");
	  ClientPrint(cl,"/LIST --- What are the channels.\n");
	  ClientPrint(cl,"/WHO --- Who is on.\n");
	  ClientPrint(cl,"/HELP --- Display this message.\n");
	  ClientPrint(cl,"/JOIN <chan> --- What do you think it does.\n");
	  ClientPrint(cl,"/MSG <dude> <msg> --- Msg a dude.\n");
	  ClientPrint(cl,"/EXIT --- Leave this sexy client.\n");
	}
      }
    }
next:
    Free(s);
  }
  Free(name);
  Exit;
}

       





        $SP,"<1>",BI=1$;


CClient *cl=ClientNew("aiwnios.com",6667,NULL);
        c          ßÿÿÿáÿÿÿ             €   åÿÿÿðÿÿÿ÷ÿÿÿÿÿÿÿøÿÿÿëÿÿÿ
ðÿÿÿóÿÿÿîÿÿÿâÿÿÿ	ÒÿÿÿÝÿÿÿIRC  