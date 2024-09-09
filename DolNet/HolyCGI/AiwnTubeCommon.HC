I64 AiwniosTubeGetVideoProp(U8 *v,U8 *name) {
  v=ExtDft(v,"DD");
  U8 *at=ChrootFile(v,SERVER_META"/VideoProps");
  StrUtil(at,SUF_REM_SPACES);
  EnsurePathExists(at);
  I64 ret=0;
  if(FileFind(at)) {
    ret=DocTreeFExe(at,name);
  }
  Free(v);
  Free(at);
  return ret;
}
U0 AiwniosTubeSetVideoPropStr(U8 *v,U8 *name,U8 *fmt,...) {
  v=ExtDft(v,"DD");
  U8 *at=ChrootFile(v,SERVER_META"/VideoProps");
  StrUtil(at,SUF_REM_SPACES);
  EnsurePathExists(at);
  fmt=StrPrintJoin(NULL,fmt,argc,argv);
  DocTreeFWrite(at,name,"StrNew(\"%Q\");\n",fmt);
  Free(fmt);
  Free(v);
  Free(at);
}

U0 AiwniosTubeSetVideoPropI64(U8 *v,U8 *name,I64 val) {
  v=ExtDft(v,"DD");
  U8 *at=ChrootFile(v,SERVER_META"/VideoProps");
  StrUtil(at,SUF_REM_SPACES);
  EnsurePathExists(at);
  DocTreeFWrite(at,name,"%d;\n",val);
  Free(v);
  Free(at);
}

U0 WriteDocToStream(I64 stream,CDoc *d) {
  CDoc *html=Doc2Html(d);
  I64 len;
  U8 *data=DocSave(html,&len);
  WriteNBytes(stream,NULL,data,len);
  Free(data);
  DocDel(html);
}