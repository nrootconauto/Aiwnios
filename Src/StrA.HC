U32
  char_bmp_alpha[16]=
  {0x0000000,0x00000000,0x87FFFFFF,0x07FFFFFE,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0,0,0,0,0,0,0,0},

  char_bmp_alpha_numeric[16]=
  {0x0000000,0x03FF0000,0x87FFFFFF,0x07FFFFFE,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0,0,0,0,0,0,0,0},

  char_bmp_alpha_numeric_no_at[16]=
  {0x0000000,0x03FF0000,0x87FFFFFE,0x07FFFFFE,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0,0,0,0,0,0,0,0},

  char_bmp_word[16]=
  {0x0000000,0x03FF0080,0x87FFFFFE,0x07FFFFFE,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0,0,0,0,0,0,0,0},

  char_bmp_filename[16]=
  {0x0000000,0x03FF73FB,0xEFFFFFFF,0x6FFFFFFF,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0,0,0,0,0,0,0,0},

  char_bmp_dec_numeric[16]=
  {0x0000000,0x03FF0000,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

  char_bmp_hex_numeric[16]=
  {0x0000000,0x03FF0000,0x7E,0x7E,0,0,0,0,0,0,0,0,0,0,0,0},

  char_bmp_white_space[16]=
  {0x80002600,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

  char_bmp_non_eol_white_space[16]=
  {0x80000200,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

  char_bmp_zero_cr_nl_cursor[16]=
  {0x00002421,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

  char_bmp_zero_tab_cr_nl_cursor[16]=
  {0x00002621,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

  char_bmp_zero_tab_cr_nl_cursor_dollar[16]=
  {0x00002621,0x10,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

  char_bmp_macro[16]=
  {0x80002600,0xFFFFFFDF,0xFFFFFFFF,0x7FFFFFFF,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0,0,0,0,0,0,0,0},

  char_bmp_printable[16]=
  {0x80002600,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0,0,0,0,0,0,0,0},

  char_bmp_displayable[16]=
  {0x80000000,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0,0,0,0,0,0,0,0},

  char_bmp_safe_dollar[16]=
  {0x80000000,0xFFFFFFEF,0xFFFFFFFF,0x7FFFFFFF,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0,0,0,0,0,0,0,0},//same but no dollar sign

  char_bmp_non_eol[16]=
  {0xFFFFDBFE,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
  0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

U8 *StrPrintHex(U8 *dst,I64 num,I64 width)
{
  U8 *res=dst+width;
  dst=res;
  while (width--) {
    *--dst="0123456789ABCDEF"(U8 *)[num&15];
    num>>=4;
  }
  return res;
}

U0 PutHex(I64 num,I64 width)
{
  U8 buf[17];
  if (width>16) width=16;
  *StrPrintHex(buf,num,width)=0;
  "%s",buf;
}
U0 PutHexU64(U64 u) {
  PutHex(u,16);
}
U0 PutHexU32(U64 u) {
  PutHex(u,8);
}
U0 PutHexU16(U64 u) {
  PutHex(u,4);
}
U0 PutHexU18(U64 u) {
  PutHex(u,2);
}

U64 ToUpper(U64 ch) {
  if('a'<=ch<='z') return ch-'a'+'A';
  return ch;
}

U8 *LstSub(I64 sub, U8 *lst)
{//Point to lst entry.
//Not efficient.  Use an array of U8 ptrs for efficiency.
  if (!lst) return NULL;
  while (*lst && sub>0) {
    while (*lst)	//Advance to end of cur entry.
      lst++;
    lst++;		//Skip trailing zero.
    if (*lst=='@')	//Check for '@' alias lst entry.
      lst++;
    else
      sub--;
  }
  if (sub||!*lst)
    return NULL;
  else
    return lst;
}

I64 LstMatch(U8 *needle, U8 *haystack_lst,I64 flags=0)
{//-2 if Ambiguous
// -1 if not found
  // Not efficient. Use hash tables for efficiency.
  I64 n,sub=0,res=-1;
  U8 *ptr;
  Bool exact_match=FALSE;
  if (!haystack_lst) return -1;
  n=StrLen(needle);
  while (*haystack_lst) {
    if (*haystack_lst=='@') {	//Check for '@' alias haystack_lst entry
      sub--;
      haystack_lst++;
    }
    ptr=needle;
    if (flags & LMF_IGNORE_CASE)
      while (*ptr && ToUpper(*ptr)==ToUpper(*haystack_lst)) {
	ptr++;
	haystack_lst++;
      }
    else
      while (*ptr && *ptr==*haystack_lst) {
	ptr++;
	haystack_lst++;
      }
    if (!*ptr) {		//Did we reach end of needle?
      if (!*haystack_lst)	//Did we reach end of haystack_lst?
	return sub;		//Found Exact match
      else {
	if (res!=-1) {
	  if (!exact_match)
	    res=-2;		//Ambiguous unless later exact match.
	} else {
	  if (!(flags & LMF_EXACT))
	    res=sub;
	}
      }
    }
    while (*haystack_lst)	//Advance to end of cur entry.
      haystack_lst++;
    haystack_lst++;		//Skip trailing zero
    sub++;
  }
  return res;
}

I64 StrOcc(U8 *src, I64 ch)
{//Count occurrences of a char.
  I64 i=0;
  if (!src) return 0;
  while (*src)
    if (*src++==ch)
      i++;
  return i;
}

I64 Spaces2Tabs(U8 *dst,U8 *src)
{//Src buf with spaces to dst buf without.
  U8 *src2;
  I64 chged=0,space_cnt,space_cnt2,col=0;
  if (*src)
    while (TRUE) {
      src2=src;
      while (*src2==CH_SPACE)
	src2++;
      space_cnt=src2-src;
      while (col+space_cnt>=8) {
	space_cnt2=8-col;
	if (space_cnt2==1)
	  *dst++=CH_SPACE;
	else {
	  *dst++='\t';
	  chged+=space_cnt2-1;
	}
	space_cnt-=space_cnt2;
	col=0;
      }
      if (*src2=='\t') {
	if (space_cnt==1 && col==7)
	  *dst++=CH_SPACE;
	else
	  chged+=space_cnt;
	*dst++='\t';
	col=0;
      } else {
	while (space_cnt--) {
	  *dst++=CH_SPACE;
	  if (++col==8)
	    col=0;
	}
	if (*src2) {
	  *dst++=*src2;
	  if (++col==8)
	    col=0;
	} else
	  break;
      }
      src=++src2;
    }
  *dst=0;
  return chged;
}

U8 *StrUtil(U8 *_src,I64 flags)
{//Modifies in place. See $LK,"flags",A="MN:SUF_REM_SPACES"$ for all the options.
  U8 *src=_src,*dst=_src;
  I64 ch;
  if (flags & SUF_REM_LEADING)
    while (Bt(char_bmp_white_space,*src))
      src++;
  while (ch=*src++)
    if (Bt(char_bmp_white_space,ch)) {
      if (!(flags & SUF_REM_SPACES)) {
	if (flags & SUF_SINGLE_SPACE) {
	  *dst++ = CH_SPACE;
	  while ((ch=*src++) && Bt(char_bmp_white_space,ch));
	  src--;
	} else
	  *dst++ = ch;
      }
    } else if (!(flags & SUF_REM_CTRL_CHARS) || ch>=CH_SHIFT_SPACE)
      *dst++=ch;
  *dst=0;
  
  
  if (flags & SUF_REM_TRAILING)
    while (dst!=_src && (!*dst || Bt(char_bmp_white_space,*dst)))
      *dst-- =0;
  if (flags & SUF_TO_UPPER)
    for (dst=_src;*dst;dst++) {
      ch=*dst;
      if ('a'<=ch<='z')
	*dst=ch-0x20;
    }
  if (flags & SUF_TO_LOWER)
    for (dst=_src;*dst;dst++) {
      ch=*dst;
      if ('A'<=ch<='Z')
	*dst=ch+0x20;
    }
  if (flags & SUF_SAFE_DOLLAR)
    for (dst=_src;*dst;dst++) {
      ch=*dst;
      if (!Bt(char_bmp_safe_dollar,*dst))
	*dst='.';
    }
  if (flags & SUF_S2T)
    Spaces2Tabs(_src,_src);
  return _src;
}
U8 *StrFirstOcc(U8 *src,U8 *marker)
{//Point to 1st occurrence of marker set in str.
  I64 ch;
  while ((ch=*src++) && !StrOcc(marker,ch));
  if (ch)
    return src-1;
  else
    return NULL;
}

U8 *StrFirstRem(U8 *src,U8 *marker,U8 *dst=NULL)
{//Remove first str segment and place in dst buf or NULL.
  I64 ch;
  U8 *ptr=src,*res=dst;
  if (dst) {
    while ((ch=*ptr++) && !StrOcc(marker,ch))
      *dst++=ch;
    *dst=0;
  } else
    while ((ch=*ptr++) && !StrOcc(marker,ch));
  if (ch)
    StrCpy(src,ptr);
  else
    *src=0;
  return res;
}

U8 *StrLastOcc(U8 *src,U8 *marker)
{//Point to last occurrence of market set in str.
  I64 ch;
  U8 *res=NULL;
  while (ch=*src++)
    if (StrOcc(marker,ch))
      res=src-1;
  return res;
}

U8 *StrLastRem(U8 *src,U8 *marker,U8 *dst=NULL)
{//Remove last str segment and place in dst buf or NULL.
  U8 *ptr;
  if (ptr=StrLastOcc(src,marker)) {
    if (dst)
      StrCpy(dst,ptr+1);
    *ptr=0;
  } else {
    if (dst)
      StrCpy(dst,src);
    *src=0;
  }
  return dst;
}

U8 *StrFind(U8 *needle,U8 *haystack_str,I64 flags=0)
{//Find needle_str in haystack_str with options.
  Bool cont;
  U8 *saved_haystack_str=haystack_str;
  I64 plen=StrLen(needle);
  do {
    cont=FALSE;
    if (flags & SFF_IGNORE_CASE)
      haystack_str=StrIMatch(needle,haystack_str);
    else
      haystack_str=StrMatch(needle,haystack_str);
    if (haystack_str && flags & SFF_WHOLE_LABELS_BEFORE &&
	  haystack_str!=saved_haystack_str &&
	  Bt(char_bmp_alpha_numeric,*(haystack_str-1))) {
      haystack_str++;
      if (*haystack_str)
	cont=TRUE;
      else
	haystack_str=NULL;
    }
    if (haystack_str && flags & SFF_WHOLE_LABELS_AFTER &&
	  Bt(char_bmp_alpha_numeric,*(haystack_str+plen))) {
      haystack_str++;
      if (*haystack_str)
	cont=TRUE;
      else
	haystack_str=NULL;
    }
  } while (cont);
  return haystack_str;
}

Bool WildMatch(U8 *test_str,U8 *wild_str)
{//Wildcard match with '*' and '?'.
  I64 ch1,ch2;
  U8 *fall_back_src=NULL,*fall_back_wild=NULL;
  while (TRUE) {
    if (!(ch1=*test_str++)) {
      if (*wild_str && *wild_str!='*')
	return FALSE;
      else
	return TRUE;
    } else {
      if (!(ch2=*wild_str++))
	return FALSE;
      else {
	if (ch2=='*') {
	  fall_back_wild=wild_str-1;
	  fall_back_src=test_str;
	  if (!(ch2=*wild_str++))
	    return TRUE;
	  while (ch2!=ch1)
	    if (!(ch1=*test_str++))
	      return FALSE;
	} else
	  if (ch2!='?' && ch1!=ch2) {
	    if (fall_back_wild) {
	      wild_str=fall_back_wild;
	      test_str=fall_back_src;
	      fall_back_wild=NULL;
	      fall_back_src=NULL;
	    } else
	      return FALSE;
	  }
      }
    }
  }
}

