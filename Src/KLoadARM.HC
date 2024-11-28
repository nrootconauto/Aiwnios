U0 LoadOneImport(U8 **_src,U8 *module_base,I64 ld_flags)
{
  U8 *src=*_src,*ptr2,*st_ptr;
  I64 i,etype,off,width,low12,write;
  CHashExport *tmpex=NULL;
  CHashImport *tmpiss;
  Bool first=TRUE;

  while (etype=*src++) {
    i=*src(U32 *)++;
    off=0;
    width=4;
    switch (etype) {
      case IET_REL_I0 ... IET_REL_RISCV:
        off=*src(U32*)++;
        width=8;
    }
    st_ptr=src;
    src+=StrLen(st_ptr)+1;
    if (*st_ptr) {
      if (!first) {
        *_src=st_ptr-1-width;
        return;
      } else {
        first=FALSE;
        if (!(tmpex=HashFind(st_ptr,
	      Fs->hash_table,HTG_ALL-HTT_IMPORT_SYS_SYM))) {
	  if (!(ld_flags & LDF_SILENT))
	    "Unresolved Reference:%s\n",st_ptr;
	  tmpiss=CAlloc(sizeof(CHashImport));
	  tmpiss->str=StrNew(st_ptr);
	  tmpiss->type=HTT_IMPORT_SYS_SYM;
	  tmpiss->module_header_entry=st_ptr-1-width;
	  tmpiss->module_base=module_base;
	  HashAdd(tmpiss,Fs->hash_table);
        }
      }
    }
    if (tmpex) {
      ptr2=module_base+i;
      if (tmpex->type & HTT_FUN)
        i=tmpex(CHashFun *)->exe_addr;
      else if (tmpex->type & HTT_GLBL_VAR)
        i=tmpex(CHashGlblVar *)->data_addr;
      else
        i=tmpex->val;
      i+=off;
      switch (etype) {
        case IET_REL_I8:
	  i=i - ptr2 - 1;
	  WriteProtectMemCpy(ptr2,&i,1);
	  break;
        case IET_IMM_U8:
	  WriteProtectMemCpy(ptr2,&i,1);
	  break;
        case IET_REL_I16:
	  i=i - ptr2 - 4;
	  WriteProtectMemCpy(ptr2,&i,2);
	  break;
        case IET_IMM_U16:
	  WriteProtectMemCpy(ptr2,&i,2);
	  break;
        case IET_REL_I32:
	  i=i - ptr2 - 4;
	  WriteProtectMemCpy(ptr2,&i,4);
	  break;
        case IET_IMM_U32:
	  WriteProtectMemCpy(ptr2,&i,4);
	  break;
        case IET_REL_I64:
	  i=i - ptr2 - 8;
	  WriteProtectMemCpy(ptr2,&i,8);
	  break;
        case IET_IMM_I64:
	  WriteProtectMemCpy(ptr2,&i,8);
	  break;
	case IET_REL_RISCV:
	  i=i-ToI64(ptr2);
	  low12 = i - (i & ~((1 << 12) - 1));
	  write=ptr2(U64*)[0];
	  switch(write>>12) {
	    default:
	    case 1:
	    case 2:
	      if (-(1<<11)<=low12<=1<<11-1) { /*Chekc for bit 12 being set*/
	        write.u32[0] |= (i >> 12) << 12;
	        write.u32[1] |= low12 << 20;
	      } else {
	        write.u32[0] |= ((i >> 12) + 1) << 12;
	        write.u32[1] |= low12 << 20;
	      }
          WriteProtectMemCpy(ptr2,&write,8);
	      break;
      }
    }
    }
  }
  *_src=src-1;
}

U0 SysSymImportsResolve(U8 *st_ptr,I64 ld_flags)
{
  CHashImport *tmpiss;
  U8 *ptr;
  while (tmpiss=HashSingleTableFind(st_ptr,
	Fs->hash_table,HTT_IMPORT_SYS_SYM)) {
    ptr=tmpiss->module_header_entry;
    LoadOneImport(&ptr,tmpiss->module_base,ld_flags);
    tmpiss->type=HTT_INVALID;
  }
}

U0 LoadPass1(U8 *src,U8 *module_base,I64 ld_flags)
{
  U8 *ptr2,*ptr3,*st_ptr;
  I64 i,j,cnt,etype;
  CHashExport *tmpex=NULL;
  while (etype=*src++) {
    i=*src(U32 *)++;
    st_ptr=src;
    src+=StrLen(st_ptr)+1;
    switch (etype) {
      case IET_REL32_EXPORT:
      case IET_IMM32_EXPORT:
      case IET_REL64_EXPORT:
      case IET_IMM64_EXPORT:
	tmpex=CAlloc(sizeof(CHashExport));
	tmpex->str=StrNew(st_ptr);
	tmpex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	if (etype==IET_IMM32_EXPORT||etype==IET_IMM64_EXPORT)
	  tmpex->val=i;
	else
	  tmpex->val=i+module_base;
	HashAdd(tmpex,Fs->hash_table);
	SysSymImportsResolve(st_ptr,ld_flags);
	break;
      case IET_REL_I0...IET_IMM_I64:
	src=st_ptr-5;
	LoadOneImport(&src,module_base,ld_flags);
	break;
      case IET_ABS_ADDR:
	if (ld_flags & LDF_NO_ABSS)
	  src+=i*sizeof(U64);
	else {
	  cnt=i;
	  for (j=0;j<cnt;j++) {
	    ptr2=module_base+*src(U64 *)++;
	    *ptr2(U64 *)+=module_base;
	  }
	}
	break;

      start:
	case IET_CODE_HEAP:
	  ptr3=MAlloc(*src(I32 *)++,Fs->code_heap);
	  break;
	case IET_ZEROED_CODE_HEAP:
	  ptr3=CAlloc(*src(I32 *)++,Fs->code_heap);
	  break;
      end:
	if (*st_ptr) {
	  tmpex=CAlloc(sizeof(CHashExport));
	  tmpex->str=StrNew(st_ptr);
	  tmpex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	  tmpex->val=ptr3;
	  HashAdd(tmpex,Fs->hash_table);
	}
	cnt=i;
	for (j=0;j<cnt;j++) {
	  ptr2=module_base+*src(U32 *)++;
	  *ptr2(I32 *)+=ptr3;
	}
	break;

      start:
	case IET_DATA_HEAP:
	  ptr3=MAlloc(*src(I64 *)++);
	  break;
	case IET_ZEROED_DATA_HEAP:
	  ptr3=CAlloc(*src(I64 *)++);
	  break;
      end:
	if (*st_ptr) {
	  tmpex=CAlloc(sizeof(CHashExport));
	  tmpex->str=StrNew(st_ptr);
	  tmpex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	  tmpex->val=ptr3;
	  HashAdd(tmpex,Fs->hash_table);
	}
	cnt=i;
	for (j=0;j<cnt;j++) {
	  ptr2=module_base+*src(U32 *)++;
	  *ptr2(I64 *)+=ptr3;
	}
	break;
    }
  }
}

U0 LoadPass2(U8 *src,U8 *module_base,I64)
{
  U8 *st_ptr;
  I64 i,etype;
  U0 (*code)();
  while (etype=*src++) {
    i=*src(U32 *)++;
    switch(etype) {
      case IET_REL_I0 ... IET_REL_RISCV:
        src+=4; //Nroot,in Aiwnios AOT,i do [binoff,offset]
    }
    st_ptr=src;
    src+=StrLen(st_ptr)+1;
    switch (etype) {
      case IET_MAIN:
        code=i+module_base;
	(*code)();
        break;
      case IET_ABS_ADDR:
        src+=sizeof(U64)*i;
        break;
      case IET_CODE_HEAP:
      case IET_ZEROED_CODE_HEAP:
        src+=4+sizeof(U32)*i;
        break;
      case IET_DATA_HEAP:
      case IET_ZEROED_DATA_HEAP:
        src+=8+sizeof(U32)*i;
        break;
    }
  }
}

CBinFile *Load(U8 *filename,I64 ld_flags=0,CBinFile *bfh_addr=INVALID_PTR)
{//Load a .BIN file module into memory.
//bfh_addr==INVALID_PTR means don't care what load addr.
  U8 *fbuf,*module_base,*absname,*as_code;
  I64 size,module_align,misalignment;
  CBinFile *bfh;

  fbuf=ExtDft(filename,"BIN.Z");
  if (!(bfh=FileRead(fbuf,&size))) {
    Free(fbuf);
    return NULL;
  }

  as_code=MAllocIdent(bfh,Fs->code_heap);
  Free(bfh);
  bfh=as_code;

  //See $$LK,"Patch Table Generation",A="FF:::/Compiler/CMain.HC,IET_ABS_ADDR"$$
  module_align=1<<bfh->module_align_bits;
  if (!module_align || bfh->bin_signature!=BIN_SIGNATURE_VAL) {
    Free(bfh);
    Free(fbuf);
    throw('BinModul');
  }

  if (bfh_addr==INVALID_PTR) {
    if (bfh->org==INVALID_PTR) {
      misalignment=module_align-sizeof(CBinFile);
      if (misalignment<0)
	misalignment&=module_align-1;
      if (Fs->code_heap!=Fs->data_heap) {
	if (module_align<16)
	  module_align=16;
	bfh_addr=MAllocAligned(size,module_align,Fs->code_heap,misalignment);
      } else if (module_align>8)
	bfh_addr=MAllocAligned(size,module_align,,misalignment);
      else {//Less than 2Gig system memory
	bfh_addr=bfh;
	goto lo_skip; //File is already in code heap area, don't copy.
      }
    } else
      bfh_addr=bfh->org;
  }
  MemCpy(bfh_addr,bfh,size);
  Free(bfh);

  lo_skip:
  module_base=bfh_addr(U8 *)+sizeof(CBinFile);

  absname=FileNameAbs(fbuf);
  Free(fbuf);
  fbuf=StrNew(absname);
  FileExtRem(fbuf);
  if (fbuf[1]==':' && StrLen(fbuf)>2)
    HashGenericAdd(fbuf+2,HTT_MODULE|HTF_PUBLIC,bfh_addr);
  LoadPass1(bfh_addr(U8 *)+bfh_addr->patch_table_offset,module_base,ld_flags);
  if (!(ld_flags&LDF_JUST_LOAD))
    LoadPass2(bfh_addr(U8 *)+bfh_addr->patch_table_offset,module_base,ld_flags);
  Free(absname);
  Free(fbuf);
  return bfh_addr;
}
