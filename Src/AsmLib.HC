U0 AOTStoreCodeU8(CCmpCtrl *cc,U8 b)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i=aotc->rip>>AOT_BIN_BLK_BITS;
  CAOTBinBlk *tmpbin=aotc->bin;
  while (i--)
    if (!(tmpbin=tmpbin->next)) {
      i=aotc->rip>>AOT_BIN_BLK_BITS+1;
      tmpbin=&aotc->bin;
      while (i--) {
	if (!tmpbin->next)
	  tmpbin->next=CAlloc(sizeof(CAOTBinBlk));
	tmpbin=tmpbin->next;
      }
      break;
    }
  tmpbin->body[aotc->rip++ & (AOT_BIN_BLK_SIZE-1)]=b;
  if (aotc->lst) {
    if (aotc->lst_col>=24) {
      '\n';
      aotc->lst_col=0;
    }
    "%02X ",b;
    aotc->lst_col+=3;
  }
  if (aotc->rip>aotc->num_bin_U8s)
    aotc->num_bin_U8s=aotc->rip;
}

U0 AOTStoreCodeU32(CCmpCtrl *cc,U32 d)
{
  CAOTCtrl *aotc=cc->aotc;
  I64 i=aotc->rip>>AOT_BIN_BLK_BITS,j=(aotc->rip+sizeof(U32))>>AOT_BIN_BLK_BITS;
  U32 *_d;
  CAOTBinBlk *tmpbin;
  if (i!=j)
    for (i=0;i<sizeof(U32);i++)
      AOTStoreCodeU8(cc,d.u8[i]);
  else {
    tmpbin=aotc->bin;
    while (i--)
      if (!(tmpbin=tmpbin->next)) {
	i=aotc->rip>>AOT_BIN_BLK_BITS+1;
	tmpbin=&aotc->bin;
	while (i--) {
	  if (!tmpbin->next)
	    tmpbin->next=CAlloc(sizeof(CAOTBinBlk));
	  tmpbin=tmpbin->next;
	}
	break;
      }
    _d=tmpbin->body+ aotc->rip&(AOT_BIN_BLK_SIZE-1);
    *_d=d;
    aotc->rip+=sizeof(U32);
    if (aotc->lst) {
      if (aotc->lst_col>=24) {
	'\n';
	aotc->lst_col=0;
      }
      "%08X ",d;
      aotc->lst_col+=9;
    }
    if (aotc->rip>aotc->num_bin_U8s)
      aotc->num_bin_U8s=aotc->rip;
  }
}

U0 AOTStoreCodeU64(CCmpCtrl *cc,U64 q)
{
  CAOTCtrl *aotc=cc->aotc;
  I64	i=aotc->rip>>AOT_BIN_BLK_BITS,
	j=(aotc->rip+sizeof(I64))>>AOT_BIN_BLK_BITS, *_q;
  CAOTBinBlk *tmpbin;
  if (i!=j)
    for (i=0;i<sizeof(I64);i++)
      AOTStoreCodeU8(cc,q.u8[i]);
  else {
    tmpbin=aotc->bin;
    while (i--)
      if (!(tmpbin=tmpbin->next)) {
	i=aotc->rip>>AOT_BIN_BLK_BITS+1;
	tmpbin=&aotc->bin;
	while (i--) {
	  if (!tmpbin->next)
	    tmpbin->next=CAlloc(sizeof(CAOTBinBlk));
	  tmpbin=tmpbin->next;
	}
	break;
      }
    _q=tmpbin->body+ aotc->rip&(AOT_BIN_BLK_SIZE-1);
    *_q=q;
    aotc->rip+=sizeof(I64);
    if (aotc->lst) {
      if (aotc->lst_col>=24) {
	'\n';
	aotc->lst_col=0;
      }
      "%016X ",q;
      aotc->lst_col+=17;
    }
    if (aotc->rip>aotc->num_bin_U8s)
      aotc->num_bin_U8s=aotc->rip;
  }
}

U0 AOTStoreCodeU8At(CCmpCtrl *cc,I64 rip,...)
{
  CAOTCtrl *aotc=cc->aotc;
  CAOTBinBlk *tmpbin;
  I64 i,j;
  for (j=0;j<argc;j++) {
    i=rip>>AOT_BIN_BLK_BITS+1;
    tmpbin=&aotc->bin;
    while (i--) {
      if (!tmpbin->next) {
	tmpbin->next=CAlloc(sizeof(CAOTBinBlk));
	tmpbin->next->next=NULL;
      }
      tmpbin=tmpbin->next;
    }
    tmpbin->body[rip++ & (AOT_BIN_BLK_SIZE-1)]=argv[j];
  }
  if (rip>aotc->num_bin_U8s)
    aotc->num_bin_U8s=rip;
}
U0 AOTOrBytesU8At(CCmpCtrl *cc,I64 rip,...)
{
  CAOTCtrl *aotc=cc->aotc;
  CAOTBinBlk *tmpbin;
  I64 i,j;
  for (j=0;j<argc;j++) {
    i=rip>>AOT_BIN_BLK_BITS+1;
    tmpbin=&aotc->bin;
    while (i--) {
      if (!tmpbin->next) {
	tmpbin->next=CAlloc(sizeof(CAOTBinBlk));
	tmpbin->next->next=NULL;
      }
      tmpbin=tmpbin->next;
    }
    tmpbin->body[rip++ & (AOT_BIN_BLK_SIZE-1)]|=argv[j];
  }
  if (rip>aotc->num_bin_U8s)
    aotc->num_bin_U8s=rip;
}

U0 AOTAndBytesU8At(CCmpCtrl *cc,I64 rip,...)
{
  CAOTCtrl *aotc=cc->aotc;
  CAOTBinBlk *tmpbin;
  I64 i,j;
  for (j=0;j<argc;j++) {
    i=rip>>AOT_BIN_BLK_BITS+1;
    tmpbin=&aotc->bin;
    while (i--) {
      if (!tmpbin->next) {
	tmpbin->next=CAlloc(sizeof(CAOTBinBlk));
	tmpbin->next->next=NULL;
      }
      tmpbin=tmpbin->next;
    }
    tmpbin->body[rip++ & (AOT_BIN_BLK_SIZE-1)]&=argv[j];
  }
  if (rip>aotc->num_bin_U8s)
    aotc->num_bin_U8s=rip;
}
