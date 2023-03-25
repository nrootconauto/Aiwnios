//TODO largley incomplete

U0 ToFileLine(U8 *_fl_file_line,U8 **_filename,I64 *_linenum)
{//"FI:D:/Dir/File.HC,123" to "D:/Dir/File.HC" and 123.
  U8 *st,*fl_file_line=StrNew(_fl_file_line);
  I64 linenum;
  StrFirstRem(fl_file_line,":");
  st=StrNew(fl_file_line);
  StrLastRem(fl_file_line,",",st);
  linenum=Str2I64(st);
  Free(st);
  *_filename=fl_file_line;
  *_linenum=linenum;
}
