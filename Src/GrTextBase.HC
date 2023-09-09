#help_index "TextBase Layer;Char/TextBase Layer"
#help_file "::/Doc/TextBase"

Bool TextChar(CTask *task,Bool allow_border=FALSE,I64 x,I64 y,I64 d) {
  I64 sx=task->scroll_x>>3+x,sy=task->scroll_y>>3+y;
  if(allow_border)
    goto __10;
  sx+=task->win_left;
  if(sx<0)
    goto __5;
  if(sx>task->win_right)
    goto __5;
  if(sx>=TEXT_COLS)
    goto __5;
  sy+=task->win_top;
  if(sy<task->win_top)
    goto __5;
  if(sy>task->win_bottom)
    goto __5;
  if(sy>=TEXT_ROWS)
    goto __5;
  goto __15;
__5: return FALSE;
__10:
  if(sx<-1)
    goto __5;
  sx+=task->win_left;
  if(sx<0)
    goto __5;
  if(sx>task->win_right+1)
    goto __5;
  if(sx>=TEXT_COLS)
    goto __5;
  sy+=task->win_top;
  if(sy>task->win_bottom+1)
    goto __5;
  if(sy<task->win_top-1)
    goto __5;
  if(sy>=TEXT_ROWS)
    goto __5;
  goto __15;
__15:
  if(!(0<=sy<TEXT_ROWS))
    return FALSE;
  gr.text_base[sy*TEXT_COLS+sx]=d;
  return TRUE;
}
Bool TextLenStr(CTask *task,I64 x,I64 y,I64 len,I64 attr,U8 *s) {
  U32 *ptr;
  x+=task->scroll_x>>3;
  y+=task->scroll_y>>3;
  if(x<0) {
    s+=-x;
    len-= -x;
    x=0;
  }
  x+=task->win_left;
  if(x<0) {
    s+=-x;
    len-= -x;
    x=0;
  }
  if(len<0) return FALSE;
  y+=task->win_top;
  if(x>task->win_right||y>task->win_bottom||y>=TEXT_ROWS) {
    return FALSE;
  }
  if(task->win_right>=TEXT_COLS)
    len=MinI64(len,TEXT_COLS-x);
  else
    len=MinI64(len,task->win_right-x+1);
  if(!len) return FALSE;
  if(!(0<=y<TEXT_ROWS))
    return FALSE;
  if(!(0<=x<TEXT_COLS))
    return FALSE;
  ptr=&gr.text_base[TEXT_COLS*y+x];
  while(--len>=0) {
    *ptr++=*s++|attr;
  }
  return TRUE;
}
Bool TextLenAttrStr(CTask *task,I64 x,I64 y,I64 len,U32 *attr_s) {
  U32 *ptr;
  x+=task->scroll_x>>3;
  y+=task->scroll_y>>3;
  if(x<0) {
    attr_s+=-x;
    len-= -x;
    x=0;
  }
  x+=task->win_left;
  if(x<0) {
    attr_s+=-x;
    len-= -x;
    x=0;
  }
  if(len<0) return FALSE;
  y+=task->win_top;
  if(x>task->win_right||y>task->win_bottom||y>=TEXT_ROWS||y<0) {
    return FALSE;
  }
  if(task->win_right>=TEXT_COLS)
    len=MinI64(len,TEXT_COLS-x);
  else
    len=MinI64(len,task->win_right-x+1);
  if(!len) return FALSE;
  if(!(0<=y<TEXT_ROWS))
    return FALSE;
  if(!(0<=x<TEXT_COLS))
    return FALSE;
  ptr=&gr.text_base[TEXT_COLS*y+x];
  while(--len>=0) {
    *ptr++=*attr_s++;
  }
  return TRUE;
}
Bool TextLenAttr(CTask *task,I64 x,I64 y,I64 len,I64 attr) {
  U32 *ptr;
  x+=task->scroll_x>>3;
  y+=task->scroll_y>>3;
  if(x<0) {
    len-= -x;
    x=0;
  }
  x+=task->win_left; 
  if(x<0) {
    len-= -x;
    x=0;
  }
  if(len<0) return FALSE;
  y+=task->win_top;
  if(x>task->win_right||y>task->win_bottom||y>=TEXT_ROWS||y<0) {
    return FALSE;
  }
  if(task->win_right>=TEXT_COLS)
    len=MinI64(len,TEXT_COLS-x);
  else
    len=MinI64(len,task->win_right-x+1);
  if(!len) return FALSE;
  if(!(0<=y<TEXT_ROWS))
    return FALSE;
  if(!(0<=x<TEXT_COLS))
    return FALSE;
  ptr=&gr.text_base[TEXT_COLS*y+x];
  while(--len>=0) {
    if(ptr(U8*)[0]) break;
    *ptr++=attr;
  }
  return len==-1;
}

public U0 TextPrint(CTask *task,I64 x,I64 y,I64 attr,U8 *fmt,...)
{//Plot chars. 30fps in Fs->draw_it() callback.
//You probably want $LK,"GrPrint",A="MN:GrPrint"$() or just $LK,"Print",A="MN:Print"$().
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  TextLenStr(task,x,y,StrLen(buf),attr<<8,buf);
  Free(buf);
}

public U0 TextBorder(CTask *task=NULL,
	I64 l,I64 r,I64 t,I64 b,I64 attr,Bool solid)
{//Plot border square. 30fps in Fs->draw_it() callback.
//Draws window borders or DolDoc text bttn borders.
  //Set task=sys_winmgr_task for no clipping.
  I64 i;
  if (!task) task=Fs;
  attr<<=8;
  TextChar(task,,l-1,t-1,text.border_chars[6+solid]+attr);
  TextChar(task,,r+1,t-1,text.border_chars[8+solid]+attr);
  TextChar(task,,l-1,b+1,text.border_chars[10+solid]+attr);
  TextChar(task,,r+1,b+1,text.border_chars[12+solid]+attr);
  for (i=l;i<=r;i++) {
    TextChar(task,,i,t-1,text.border_chars[2+solid]+attr);
    TextChar(task,,i,b+1,text.border_chars[2+solid]+attr);
  }
  for (i=t;i<=b;i++) {
    TextChar(task,,l-1,i,text.border_chars[4+solid]+attr);
    TextChar(task,,r+1,i,text.border_chars[4+solid]+attr);
  }
}

public U0 TextRect(I64 l,I64 r,I64 t,I64 b,I64 d)
{//Fill text rect. 30fps in Fs->draw_it() callback.
  I64 y,w;
  l+=Fs->scroll_x>>3;
  t+=Fs->scroll_y>>3;
  if (l>r || t>b) return;
  if (t<0) t=0;
  if (b>=TEXT_ROWS) b=TEXT_ROWS-1;
  if (l<0) l=0;
  if (r>=TEXT_COLS) r=TEXT_COLS-1;
  if (w=r-l+1)
    for (y=t;y<=b;y++)
      MemSetU32(gr.text_base(U8 *)+(y*TEXT_COLS+l)*sizeof(U32),d,w);
}

