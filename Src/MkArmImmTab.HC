//https://gist.github.com/dinfuehr/51a01ac58c0b23e4de9aac313ed6a06a
U8 *MakeArmImmTab() {
  CDoc *doc=DocNew("/Src/ArmImmTab.HC");
  I64 line=1;
  U8 *ret,*ptr;
  U64 hex,n,r,s;
  DocPrint(doc,"U64 arm_imm_table[5334*2]={\n");
  while(ret=DocLineRead("/Src/armimmtab2.txt",line)) {
    ptr=ret;
    hex=Str2I64(ptr,16,&ptr);
    ptr=StrFirstOcc(ptr,"01");
    n=Str2I64(ptr,2,&ptr);
    ptr=StrFirstOcc(ptr,"01");
    r=Str2I64(ptr,2,&ptr);
    ptr=StrFirstOcc(ptr,"01");
    s=Str2I64(ptr,2,&ptr);
    DocPrint(doc,"\t0x%X,0x%X,\n",hex,(n<<12|r<<6|s)<<10);
    "%d/5334\n",line++;
    Free(ret);
  }
  DocPrint(doc,"};\n");
  DocWrite(doc);
  DocDel(doc);
}
MakeArmImmTab;