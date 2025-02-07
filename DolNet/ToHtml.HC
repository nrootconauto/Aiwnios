// vi: set fenc:utf-8 :vi
#help_index "DolDoc/Conversion"

//See $LK,"::/Doc/Credits.DD"$.

I64 Cp437[0x100-0x7F]={
'.',//not rendered
'Ç','ü','é','â','ä','à','å','ç',
'ê','ë','è','ï','î','ì','Ä','Å',
'É','æ','Æ','ô','ö','ò','û','ù',
'ÿ','Ö','Ü','¢','£','¥','₧','ƒ',
'á','í','ó','ú','ñ','Ñ','ª','º',
'¿','⌐','¬','½','¼','¡','«','»',
'░','▒','▓','│','┤','╡','╢','╖',
'╕','╣','║','╗','╝','╜','╛','┐',
'└','┴','┬','├','─','┼','╞','╟',
'╚','╔','╩','╦','╠','═','╬','╧',
'╨','╤','╥','╙','╘','╒','╓','╫',
'╪','┘','┌','█','▄','▌','▐','▀',
'α','ß','Γ','π','Σ','σ','µ','τ',
'Φ','Θ','Ω','δ','∞','φ','∈','∩',
'≡','±','≥','≤','⌠','⌡','÷','≈',
'°','∙','·','√','ⁿ','²','￭','■'};

#define DOCET_UNDEF_COLOR	0x100000000
#define DOCET_LINK_UNDERLINE	0x200000000
U0 HtmlPutS(CDoc *doc,I64 u32_attr,I64 *_old_u32_attr,Bool underline_update,
	U8 *st,I64 *_col,U8 *style_bitmap=NULL)
{
  U8 ch,*ptr;
  I64 cnt;
  u32_attr&=0xFFFFFF00;
  U32 old_attr=*_old_u32_attr;
  if (u32_attr&DOCET_INVERT)
    u32_attr.u8[1]=(u32_attr.u8[1]&15)<<4|u32_attr.u8[1]>>4;
  if (underline_update &&
	!(u32_attr&DOCET_UNDERLINE) && *_old_u32_attr&DOCET_UNDERLINE) {
    if (!(*_old_u32_attr&DOCET_LINK_UNDERLINE))
      DocPrint(doc,"</u>");
    *_old_u32_attr&=~DOCET_LINK_UNDERLINE;
  }
  if (!(u32_attr&DOCET_BLINK) && *_old_u32_attr&DOCET_BLINK)
    DocPrint(doc,"</blink>");
  if (u32_attr&0xFF00!=*_old_u32_attr&0xFF00) {
    if (!(*_old_u32_attr&DOCET_UNDEF_COLOR))
      DocPrint(doc,"</span>");
    DocPrint(doc,"<span class=\"c%02X\">",u32_attr.u8[1]);
    if (style_bitmap)
      LBts(style_bitmap,u32_attr.u8[1]);
  }
  if (u32_attr&DOCET_BLINK && !(*_old_u32_attr&DOCET_BLINK))
    DocPrint(doc,"<blink>");
  if (underline_update) {
    if (u32_attr&DOCET_UNDERLINE && !(*_old_u32_attr&DOCET_UNDERLINE))
      DocPrint(doc,"<u>");
  } else //Keep same underline status.
    u32_attr=u32_attr&~DOCET_UNDERLINE | *_old_u32_attr&DOCET_UNDERLINE;

  *_old_u32_attr=u32_attr | *_old_u32_attr&DOCET_LINK_UNDERLINE;

  while (ch=*st++) {
    switch (ch) {
      case '\t':
	do {
	  DocPutKey(doc,CH_SPACE,0);
	  *_col=*_col+1;
	} while (*_col&7);
	break;

      start:
	case CH_SHIFT_SPACE: ptr=" ";	break;
	case '&':	ptr="&amp;";	break;
	case '<':	ptr="&lt;";	break;
	case '>':	ptr="&gt;";	break;
	case '"':	ptr="&quot;";	break;
      end:
	DocPrint(doc,ptr);
	*_col=*_col+1;
	break;

      default:
	if (CH_SPACE<=ch<0x7F || ch=='\n') {
	  //Added by nroot,if non white background,be sure to fill it up
	  //TextBase puts background color in bits 12-16
          /*if((u32_attr.u8[1]>>4)!=0xf)
	    if(ch=='\n'&&!(old_attr&DOCET_UNDEF_COLOR)) { 
	      //Dont underline the whitesapce
	      if(u32_attr&DOCET_UNDERLINE)
	        DocPrint(doc,"</u>");
	      //Here's the deal,I (nrootconauto) use table-cell as it works,I dont know why though
	      if(TEXT_COLS-*_col)
	        DocPrint(doc,"<span style=\"display:table-cell;width:%dch;height:1ch\"></span>",TEXT_COLS-*_col);
	      //restore
	      if(u32_attr&DOCET_UNDERLINE)
	        DocPrint(doc,"<u>");
            }*/
	  DocPutKey(doc,ch,0);
	} else
	  DocPrint(doc,"%c",Cp437[ch-0x7F]);
	*_col=*_col+1;
    }
  }
}

U8 *TOSLinkCvt2(U8 *filename,I64 line_num)
{// ::/  --> http://www.templeos.org/Wb/
//Make your own LinkCvt routine
  U8 *res=NULL,*st;
  if (filename) {
//    st=ExtChg(filename,"html");
    st=filename;
    if (st && StrLen(st)>3 && !StrNCmp(st+1,":/",2))
      res=MStrPrint("/%s#l%d",st+3,line_num);
//    Free(st);
  }
  return res;
}

U8 *URLBibleGateway(U8 *src)
{
  U8 buf[STR_LEN],*dst=buf;
  if (!MemCmp(src,"BF:",3))
    src+=3;
  while (*src) {
    if (*src==CH_SPACE || *src==',')
      *dst++='+';
    else
      *dst++=*src;
    src++;
  }
  *dst=0;
  if (StrOcc(buf,'-'))
    return MStrPrint(
	  "http://www.biblegateway.com/passage/?search=%s&version=NIV",buf);
  else
    return MStrPrint(
	  "http://www.biblegateway.com/verse/en/%s",buf);
}

U8 *TOSLinkCvt1(U8 *link_st)
{
  static CDoc *bible=NULL;
  static I64 locks=0;
  CDoc *anchor_doc;
  CDocEntry *cde;
  U8 *res=NULL,*filename,*needle,*ofilename;
  I64 i,num;
  if (link_st) {
    switch (i=EdLinkCvt(link_st,&filename,&needle,&num)) {
      case LK_FILE_LINE:
      case LK_PLAIN_LINE:
      case LK_FILE:
        res=TOSLinkCvt2(filename,num);
        break;
      case -1:
      case LK_DEF:
      case LK_HELP_INDEX:
      case LK_DOC:
      case LK_DOC_FIND:
      case LK_DOC_LINE:
      case LK_DOC_ANCHOR:
        break;
      case LK_FILE_ANCHOR:
//Nroot was here
      //Change into our WWW directory to get the full path then push 
        filename=ChrootFile(ofilename=filename);
        if(FileFind(filename)) {
	  anchor_doc=DocRead(filename);
	  if(DocAnchorFind(anchor_doc,needle)) {
	    res=TOSLinkCvt2(ofilename,anchor_doc->cur_entry->y);
	  }
	  DocDel(anchor_doc);
        }
	Free(ofilename);
        break;
      case LK_BIBLE_FIND:
        while (LBts(&locks,0))
	  Yield;
        if (!bible)
	  bible=Adam("DocRead(\"%s\");",filename);
        if (DocFind(bible,num,needle))
	  res=URLBibleGateway(link_st);
        LBtr(&locks,0);
        break;
      default:
        if (DocFileEd(i,filename,needle,&num,EDF_UNCOLLAPSE|EDF_BAIL))
	  res=TOSLinkCvt2(filename,num);
    }
    Free(filename);
    Free(needle);
  }
  return res;
}

U0 SpriteExtentsTransform(I64 *mat,U8 *elems,I64 *min_x=NULL,I64 *max_x=NULL,
				   I64 *min_y=NULL,I64 *max_y=NULL)
{//Ignores flood fills.
  CDC *dc=DCNew(GR_WIDTH,GR_HEIGHT);
  Bool found_pixel=FALSE;
  I64 x,y;
  Mat4x4Equ(dc->r,mat);
 DCDepthBufAlloc(dc);
  DCFill(dc);
  dc->flags|=DCF_TRANSFORMATION;
  dc->x=GR_WIDTH/2,dc->y=GR_HEIGHT/2;
  dc->z=GR_Z_ALL;
  Sprite3(dc,0,0,0,elems);
  dc->min_x=GR_WIDTH;
  dc->min_y=GR_HEIGHT;
  dc->max_x=0;
  dc->max_y=0;
  for(x=0;x!=GR_WIDTH;x++)
    for(y=0;y!=GR_HEIGHT;y++) {
      if(GrPeek0(dc,x,y)!=TRANSPARENT) {
	found_pixel=TRUE;
        dc->min_x=MinI64(dc->min_x,x);
        dc->min_y=MinI64(dc->min_y,y);
        dc->max_x=MaxI64(dc->max_x,x);
        dc->max_y=MaxI64(dc->max_y,y);
      } 
    }
  if(found_pixel) {
    if (min_x) *min_x=dc->min_x-GR_WIDTH/2;
    if (max_x) *max_x=dc->max_x-GR_WIDTH/2;
    if (min_y) *min_y=dc->min_y-GR_HEIGHT/2;
    if (max_y) *max_y=dc->max_y-GR_HEIGHT/2;
  } else  {
    if (min_x) *min_x=0;
    if (max_x) *max_x=0;
    if (min_y) *min_y=0;
    if (max_y) *max_y=0;
  }
  DCDel(dc);
}

//Added by Nroot
U0 ConvertSpriteToBmp(CDoc *html_doc,U8 *filename,CDocEntry *doc,I64 digits) {
  if(!filename) filename=SERVER_WWW"/TMP.DD";
  filename=FileNameAbs(filename);
  U8 *tmp2=FileNameAbs(SERVER_WWW);
  U8 *fp=MStrPrint("%s.%d.BMP",filename+StrLen(tmp2),doc->bin_data->num);
  U8 *to=ChrootFile(fp,SERVER_HTML);
  CDC *res;
  I64 min_x,max_x,min_y,max_y,mat[16];
  if(SpriteTypeMask(doc->bin_data->data)&(1<<SPT_MESH|1<<SPT_SHIFTABLE_MESH)) {
    Mat4x4IdentEqu(mat);
    Mat4x4RotZ(mat,.5*3.1);
    Mat4x4RotY(mat,.5*1.9);
    Mat4x4RotX(mat,.5);
    SpriteExtentsTransform(mat,doc->bin_data->data,&min_x,&max_x,&min_y,&max_y);
  } else
    SpriteExtents(doc->bin_data->data,&min_x,&max_x,&min_y,&max_y);
  res=DCNew(max_x-min_x+1,max_y-min_y+1);
  DCFill(res,TRANSPARENT);
  if(SpriteTypeMask(doc->bin_data->data)&(1<<SPT_MESH|1<<SPT_SHIFTABLE_MESH)) {
    DCDepthBufAlloc(res);
    res->flags|=DCF_TRANSFORMATION;
    res->x=-min_x;
    res->y=-min_y;
    res->z=GR_Z_ALL;
    Mat4x4Equ(res->r,mat);
    Sprite3(res,0,0,0,doc->bin_data->data);
  } else 
    Sprite3(res,-min_x,-min_y,0,doc->bin_data->data);
  EnsurePathExists(to);
  BMPWrite(to,res);
  //https://stackoverflow.com/a/6040258
  DocPrint(html_doc,"<div style=\"position:relative;width:0;height:0\"><div style=\"position:absolute;left:%drem\">",digits);
   DocPrint(html_doc,"<img src=\"%Q\" width=\"%d\" height=\"%d\">",fp,
         max_x-min_x+1,
         max_y-min_y+1,
         );
  DocPrint(html_doc,"</div></div>");
  DCDel(res);
  Free(fp),Free(to),Free(filename),Free(tmp2);
}

public CDoc *Doc2Html(CDoc *doc_in,U8 *html_header=NULL,U8 *body_header=NULL,
	U8 *body_footer=NULL,U8 *html_footer=NULL,Bool line_anchors=TRUE,
	U8 (*link_cvt)(U8 *link_st)=&TOSLinkCvt1,Bool line_nums=TRUE)
{//Cvt $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$doc to HTML file.
  CDocEntry *doc_e,*style,*doc_e2;
  I64 i,y,old_y=I64_MIN,col,old_u32_attr=DOCET_UNDEF_COLOR,old_attr,digits;
  U32 *hl,*src;
  U8 *st,st_2[2],*link_st,style_bitmap[256/8];
  MemSet(style_bitmap,0,sizeof style_bitmap);
  CBGR48 p[COLORS_NUM];
  GrPaletteGet(p);
  CDoc *doc_out=DocNew;
  Bool unlock_doc_in=DocLock(doc_in),no_bwd;
  U8 *filename=doc_in->filename.name;
  old_attr=doc_in->win_task->text_attr;
  doc_in->win_task->text_attr=DOC_ATTR_DFT_TEXT;

  for (i=0xF0;i<=0xFF;i++)
    LBts(style_bitmap,i);

  DocRecalc(doc_in,RECALCt_NORMAL|RECALCF_TO_HTML);

  digits=Log10(doc_in->head.last->y+1)+1;

  st_2[0]=0; st_2[1]=0;
  doc_out->flags|=DOCF_PLAIN_TEXT|DOCF_NO_CURSOR;

  if (!html_header) html_header=
	  "<!DOCTYPE HTML>\n"
	  "<html>\n"
	  "<head>\n"
	  "<meta http-equiv=\"Content-Type\" "
	  "content=\"text/html;charset=UTF-8\">\n";
  if (!body_header) body_header=
	  "<body>\n"
	  "<pre>\n"; //Nroot removed font style for better compatability(it looks better)
  if (!body_footer) body_footer=
	  "</pre></body>\n";
  if (!html_footer) html_footer=
	  "</html>\n";

  DocPutS(doc_out,html_header);

  DocPutS(doc_out,"<style type=\"text/css\">\n"
		  "html * {\n"
		  "\tfont-size: 10pt !important;\n"
		  "\tfont-family: monospace, \"Cascadia Mono\", \"IBM Plex Mono\", \"Roboto\" !important;\n"
		  "}\n"
		  "html {\n"
		  "    display: table;\n"
		  "    margin: auto;\n"
		  "}\n"
		  "\n"
		  "body {\n"
		  "    display: table-cell;\n"
		  "    vertical-align: middle;\n"
		  "}\n"
		  "\n");
  style=doc_out->cur_entry->last;
  DocPutS(doc_out,
        "</style>\n"
        "</head>\n");
  DocPutS(doc_out,body_header);

  doc_e=doc_in->head.next;
  col=doc_e->x;
  y=doc_e->y;
  while (doc_e!=doc_in) {
    if (!(doc_e->de_flags&DOCEF_SKIP)) {
      if (y!=old_y) {
        if (line_anchors)
	  DocPrint(doc_out,"<a name=\"l%d\"></a>",y+1);
        if (line_nums)
	  DocPrint(doc_out,"<a href=\"#l%d\">%0*d</a>",y+1,digits,y+1);
        old_y=y;
      }
      while (y<doc_e->y) {
        HtmlPutS(doc_out,0xF000//.u8[1]=F0(white),no other attribs needed so it's a constant
	      ,&old_u32_attr,TRUE,"\n",&col,style_bitmap);
        if (++y!=old_y) {
	  if (old_u32_attr&DOCET_UNDERLINE) DocPutS(doc_out,"</u>");
	  if (old_u32_attr&DOCET_BLINK) DocPutS(doc_out,"</blink>");
	  if (line_anchors) DocPrint(doc_out,"<a name=\"l%d\"></a>",y+1);
	  if (line_nums) DocPrint(doc_out,"<a href=\"#l%d\">%0*d</a>",y+1,digits,y+1);
	  if (old_u32_attr&DOCET_UNDERLINE) DocPutS(doc_out,"<u>");
	  if (old_u32_attr&DOCET_BLINK) DocPutS(doc_out,"<blink>");
	  old_y=y;
        }
        col=0;
      }

      no_bwd=TRUE;
      doc_e2=doc_e->next;
      while (doc_e2!=doc_in && doc_e2->y==doc_e->y) {
        if (doc_e2->x<doc_e->x) {
	  no_bwd=FALSE;
	  break;
        }
        doc_e2=doc_e2->next;
      }

      if (no_bwd)
        while (col<doc_e->x)
	  HtmlPutS(doc_out,doc_e->settings.final_u32_attr&~DOCET_UNDERLINE,
	        &old_u32_attr,TRUE,"",&col,style_bitmap);

      link_st=NULL;
      if (doc_e->de_flags&DOCEF_HTML_LINK)
        link_st=StrNew(doc_e->html_link);
      else if (st=DocEntryLink(doc_in,doc_e)) {
        link_st=link_cvt(st);
        Free(st);
      }

      if (link_st) {
        if (old_u32_attr&DOCET_UNDERLINE &&!(old_u32_attr&DOCET_LINK_UNDERLINE))
	  DocPrint(doc_out,"</u>");
        if (old_u32_attr&DOCET_BLINK)
	  DocPrint(doc_out,"</blink>");
        if (!(old_u32_attr&DOCET_UNDEF_COLOR))
	  DocPrint(doc_out,"</span>");
        old_u32_attr=DOCET_UNDEF_COLOR|DOCET_UNDERLINE|DOCET_LINK_UNDERLINE;
        DocPrint(doc_out,"<a href=\"%s\">",link_st);
      }

      switch (doc_e->type_u8) {
        case DOCT_TEXT:
	  if (doc_e->de_flags&DOCEF_HIGHLIGHT) {
	    if (doc_e->last==doc_in)
	      MemCpy(&doc_e->settings,&doc_in->settings_head,
		    sizeof(CDocSettings));
	    else
	      MemCpy(&doc_e->settings,&doc_e->last->settings,
		    sizeof(CDocSettings));
	    src=hl=DocHighlight(doc_e,doc_e->tag,StrLen(doc_e->tag),
		  doc_e->type&0xFF00);
	    while (*src) {
	      st_2[0]=*src&0xFF;
	      HtmlPutS(doc_out,*src++,&old_u32_attr,TRUE,
		    st_2,&col,style_bitmap);
	    }
	    Free(hl);
	  } else
	    HtmlPutS(doc_out,doc_e->settings.final_u32_attr,
		  &old_u32_attr,TRUE,doc_e->tag,&col,style_bitmap);
	  break;
        case DOCT_TAB:
	  HtmlPutS(doc_out,doc_e->settings.final_u32_attr,&old_u32_attr,TRUE,
	        "\t",&col,style_bitmap);
	  break;
        case DOCT_HTML_CODE:
	  if (old_u32_attr&DOCET_UNDERLINE &&
	        !(old_u32_attr&DOCET_LINK_UNDERLINE))
	    DocPrint(doc_out,"</u>");
	  if (old_u32_attr&DOCET_BLINK)
	    DocPrint(doc_out,"</blink>");
	  if (!(old_u32_attr&DOCET_UNDEF_COLOR))
	    DocPrint(doc_out,"</span>");
	  old_u32_attr=DOCET_UNDEF_COLOR;
	  DocPrint(doc_out,"%s",doc_e->tag);
	  break;
        case DOCT_SPRITE:
	  HtmlPutS(doc_out,doc_e->settings.final_u32_attr,
	        &old_u32_attr,TRUE,doc_e->tag,&col,style_bitmap);
	  ConvertSpriteToBmp(doc_out,filename,doc_e,digits);
	  break;
        default:
	  if (doc_e->de_flags&DOCEF_TAG)
	    HtmlPutS(doc_out,doc_e->settings.final_u32_attr,
		  &old_u32_attr,TRUE,doc_e->tag,&col,style_bitmap);
      }
      if (link_st) {
        if (old_u32_attr&DOCET_UNDERLINE &&!(old_u32_attr&DOCET_LINK_UNDERLINE))
	  DocPrint(doc_out,"</u>");
        if (old_u32_attr&DOCET_BLINK)
	  DocPrint(doc_out,"</blink>");
        if (!(old_u32_attr&DOCET_UNDEF_COLOR))
	  DocPrint(doc_out,"</span>");
        DocPrint(doc_out,"</a>");
        old_u32_attr=DOCET_UNDEF_COLOR|DOCET_UNDERLINE|DOCET_LINK_UNDERLINE;
        Free(link_st);
      }
    }
    doc_e=doc_e->next;
  }
  while (y++<doc_e->y) {
    HtmlPutS(doc_out,doc_e->settings.final_u32_attr,
	  &old_u32_attr,TRUE,"\n",&col,style_bitmap);
    col=0;
  }
  if (old_u32_attr&DOCET_UNDERLINE && !(old_u32_attr&DOCET_LINK_UNDERLINE))
    DocPrint(doc_out,"</u>");
  if (old_u32_attr&DOCET_BLINK)
    DocPrint(doc_out,"</blink>");
  if (!(old_u32_attr&DOCET_UNDEF_COLOR))
    DocPrint(doc_out,"</span>");
  old_u32_attr=DOCET_UNDEF_COLOR;
  DocPrint(doc_out,"%s",body_footer);
  DocPrint(doc_out,"%s",html_footer);

  doc_out->cur_entry=style->next;
  for (i=0;i<256;i++)
    if (Bt(style_bitmap,i))
      DocPrint(doc_out,
	    ".c%02X{color:#%02x%02x%02x;background:#%02x%02x%02x;}\n",
	    i,p[i&15].r>>8,p[i&15].g>>8,p[i&15].b>>8,
	    p[i/16].r>>8,p[i/16].g>>8,p[i/16].b>>8);
  doc_out->cur_entry=&doc_out->head;
  DocRecalc(doc_out);

  doc_in->win_task->text_attr=old_attr;

  if (unlock_doc_in)
    DocUnlock(doc_in);
  return doc_out;
}
#help_index "Cmd Line (Typically);DolDoc/Conversion;DolDoc/Cmd Line (Typically)"
public U0 ToHtml(U8 *_in_name,U8 *_out_name=NULL,U8 *html_header=NULL,
	U8 *body_header=NULL,U8 *body_footer=NULL,U8 *html_footer=NULL,
	I64 width=80,Bool line_anchors=TRUE,
	U8 (*link_cvt)(U8 *link_st)=&TOSLinkCvt1,Bool line_nums=TRUE)
{//Convert $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$file to HTML.
//Supply your own link_cvt routine.
  U8 *in_name,*out_name;
  CDoc *doc_in,*doc_out;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinHorz(0,width-1); //Sets doc width for word wrap.

  in_name=ExtDft(_in_name,"HC.Z");
  if (_out_name)
    out_name=ExtDft(_out_name,"html");
  else
    out_name=ExtChg(_in_name,"html");

  doc_in=DocRead(in_name,DOCF_NO_CURSOR);
  DocCollapse(FALSE,doc_in); //Added by nroot
  doc_out=Doc2Html(doc_in,html_header,body_header,body_footer,html_footer,
	line_anchors,link_cvt,line_nums);
  StrCpy(&doc_out->filename.name,out_name);

  SettingsPop;

  DocWrite(doc_out);
  DocDel(doc_in);
  DocDel(doc_out);
  Free(in_name);
  Free(out_name);
}
U0 HtmlRawPrint(CDoc *d,U8 *fmt,...) {
  fmt=StrPrintJoin(NULL,fmt,argc,argv);
  DocPrint(d,"$$HC,\"%Q\"$$",fmt);
  Free(fmt);
}
U0 Captcha(CDoc *doc,U32 *u) {
  Seed(cnts.jiffies);
  CDC *dc=DCNew(256,64),*sc=DCNew(256,64);
  U8 *p,*s;
  I64 x,y;
  *u=RandU32;
  DCFill(dc,TRANSPARENT);
  Mat4x4IdentEqu(sc->r);
  Mat4x4Scale(sc->r,1.5);
  sc->flags|=DCF_TRANSFORMATION;
  DCFill(sc,WHITE);
  dc->color=RED;
  for(y=0;y<64;y+=8)
    for(x=-(RandU16&7)*8;x<256;) {
      s=god.words[RandU16%god.num_words];
      GrPrint(dc,x,y,"%s ",s);
      x+=StrLen(s)*8+8;
    }
  GrBlot3(sc,0,0,0,dc);
 DCFill(dc,TRANSPARENT);
  Mat4x4IdentEqu(sc->r);
  Mat4x4Scale(sc->r,3);
  dc->color=BLUE;
  GrPrint(dc,0,0,s=MStrPrint("%x",*u));
  Mat4x4TranslationAdd(sc->r,sc->width/2-StrLen(s)*8*3/2.,sc->height/2-3*8/2.,0.);
  GrBlot3(sc,0,0,0,dc);
  DocSprite(doc,p=DC2Sprite(sc));
  Free(s),Free(p),DCDel(dc),DCDel(sc);
}
public CDoc*HtmlFormGen(U8 *cn,U8 *action,U8 *method="POST",
			  CDoc *header=NULL,CDoc *footer=NULL,
			  U32 *num=NULL) {
  CDoc *ret,*tmp;
  CHashClass *cls=HashFind(cn,Fs->hash_table,HTT_CLASS);
  CDocEntry *ent;
  CMemberLst *mlst,*override;
  U8 *fmt,*trimmed,*more=NULL,*s,*nam,*col;
  I64 rand;
  if(!cls) throw('HTML');
  tmp=DocNew;
  if(header)
    DocInsDoc(tmp,header);
  mlst=cls->member_lst_and_root;
  if(!StrICmp(method,"POST"))
    more="enctype=\"multipart/form-data\"";
  HtmlRawPrint(tmp,"<form action=\"%s\" method=\"%s\" %s>",action,method,more);
  while(mlst) {
    if(fmt=MemberMetaData("format",mlst)) {
      if(ent=DocPrint(tmp,"%s",fmt)) {
        switch(ent->type_u8) {
	  start:
	    if(ent->aux_str)
 	      trimmed=StrNew(ent->aux_str);
	    else
	      trimmed=MStrPrint("%s:",mlst->str);
	    if(col=StrFirstOcc(trimmed,":"))
	      *col=0;
	    override=MemberMetaData("html_input_type",mlst);
	  case DOCT_DATA:
	    if(!override)
	      override="text";
	    break;
	  case DOCT_LST:
	    DocEntryDel(tmp,ent);
	    break;
	  case DOCT_CHECK_BOX:
	    if(!override)
	      override="checkbox";
	    break;
	  end:
	    HtmlRawPrint(tmp,"<label for=\"%s\">%s:</label><br>",mlst->str,trimmed);
	    if(!StrCmp(override,"checkbox"))
	      HtmlRawPrint(tmp,"<input type=\"%s\" id=\"%s\" name=\"%s\" value=\"%Q\"><br>",override,mlst->str,mlst->str,mlst->str);
	    else if(StrCmp(override,"textarea"))
	      HtmlRawPrint(tmp,"<input type=\"%s\" id=\"%s\" name=\"%s\"><br>",override,mlst->str,mlst->str);
	    else
	      HtmlRawPrint(tmp,"<textarea type=\"%s\" id=\"%s\" name=\"%s\"></textarea><br>",override,mlst->str,mlst->str);
	    DocEntryDel(tmp,ent);
	    Free(trimmed);
	    break;
        }
      }
     }
    mlst=mlst->next;
  }
  if(num) {
    HtmlRawPrint(tmp,"<label for=\"captcha\">Type the hexadecimal digits only</label><br>\n");
    //HtmlRawPrint(tmp,"<br><br>");
    Captcha(tmp,num);
    HtmlRawPrint(tmp,"<br><br><br><br><br><input id=\"captcha\" type=\"text\" name=\"captcha\" placeholder=\"Captcha\">\n<br>\n");
    rand=RandI64;
    HtmlRawPrint(tmp,"<input type=\"hidden\" id=\"session\" name=\"session\" value=\"%x\">",rand);
    FramePtrAdd(nam=MStrPrint("%x",rand),*num,adam_task);
    Free(nam);
  }
  HtmlRawPrint(tmp,"<input type=\"submit\" value=\"Submit\"><br>");
  HtmlRawPrint(tmp,"</form>");
  if(footer)
    DocInsDoc(tmp,footer);
  ret=Doc2Html(tmp,NULL,NULL,NULL,NULL,FALSE,&TOSLinkCvt1,FALSE);
  DocDel(tmp);
  return ret;

}
