class CAiwniosPack {
//Non-absolute paths are relative to home directory
  U64 ramdisk_size;
  U64 hcrt_size;
  U64 hcrt_offset;
  U64 ramdisk_offset;
  U8 boot_command[STR_LEN];
  U8 save_directory[STR_LEN];
  U0 body;
};

I64 IsoSize(U8 *dir) {
  I64 s=0;
  U8 *dumb=MStrPrint("/Tmp%x.ISO.C",tS);
  CDirEntry *de;
  RedSeaISO(dumb,dir);
  if(de=FilesFind(dumb)) {
    s=de->size;
    DirEntryDel(de);
    Del(dumb);
  }
  Free(dumb);
  return s;
}
U8 *RamDiskImage(U8 *dir,I64 *len=NULL) {
  dir=FileNameAbs(dir);
  I64 s=IsoSize(dir),extra;
  U8 *dumb;
  CDrv *dv=DrvMakeFreeSlot(DrvNextFreeLet('A'));
  CBlkDev *bd=BlkDevNextFreeSlot(dv->drv_let,BDT_RAM);
  bd->max_blk=CeilI64(s,BLK_SIZE)/BLK_SIZE+256;
  BlkDevAdd(bd,,TRUE,TRUE);
  MemSet(bd->RAM_dsk,0,bd->max_blk*BLK_SIZE);
  Fmt(dv->drv_let,TRUE,FALSE,FSt_REDSEA);
  dumb=MStrPrint("%C:/",dv->drv_let);
  CopyTree(dir,dumb);
  Free(dumb);
  dumb=MAllocIdent(bd->RAM_dsk);
  if(len) *len=bd->max_blk*BLK_SIZE;
  Cd("~"); //Dont unmount before Cd to home
  Unmount(dv->drv_let);
  Free(dir);
  return dumb;
}

CDrv *MountRamDisk(U8 *rd,I64 len) {
  CDrv *dv=DrvMakeFreeSlot(DrvNextFreeLet('B'));
  CBlkDev *bd=BlkDevNextFreeSlot(dv->drv_let,BDT_RAM);
  bd->max_blk=CeilI64(len,BLK_SIZE)/BLK_SIZE;
  BlkDevAdd(bd,,TRUE,TRUE);
  MemSet(bd->RAM_dsk,0,bd->max_blk*BLK_SIZE);
  Fmt(dv->drv_let,TRUE,FALSE,FSt_REDSEA);
  MemCpy(bd->RAM_dsk,rd,len);
  DskCacheInvalidate(dv);
  RedSeaInit(dv);
  return dv;
}
U0 AiwniosPack(U8 *result_name,U8 *boot_command,U8 *root,U8 *hcrt_bin="/HCRT2.BIN") {
  if(!boot_command) boot_command="";
  if(!FileFind("/aiwnios")&&!FileFind("/aiwnios.exe")) {
    "I need aiwnios(or aiwnios.exe) to be present\n";
    throw('Pack');
  }
  CDirEntry *de;
  I64 len,aiwnios_len,total_len,hcrt_len;
  U8 *ram_disk=RamDiskImage(root,&len),*aiwnios,*hcrt,*final;
  U8 *ptr,*hcrt_ptr,*ramdisk_ptr;
  de=FilesFind("/aiwnios");
  if(de) {
pass:;
    aiwnios=FileRead(de->full_name,&aiwnios_len);
    hcrt=FileRead(hcrt_bin,&hcrt_len);
    total_len=hcrt_len+aiwnios_len+len+sizeof(CAiwniosPack)+16;
    final=CAlloc(total_len);
    MemCpy(final,aiwnios,aiwnios_len);
    ptr=final+aiwnios_len;
    hcrt_ptr=ptr+sizeof(CAiwniosPack);
    ramdisk_ptr=hcrt_ptr+hcrt_len;
    StrCpy(ptr(CAiwniosPack*)->boot_command,boot_command);
    ptr(CAiwniosPack*)->ramdisk_size=len;
    ptr(CAiwniosPack*)->hcrt_size=hcrt_len;
    ptr(CAiwniosPack*)->hcrt_offset=hcrt_ptr-final;
    ptr(CAiwniosPack*)->ramdisk_offset=ramdisk_ptr-final;
    MemCpy(hcrt_ptr,hcrt,hcrt_len);
    MemCpy(ramdisk_ptr,ram_disk,len);
    Free(hcrt);
    Free(aiwnios);
    DirEntryDel(de);
//Last U64 is the offset to CAiwniosPack
    (final+hcrt_len+aiwnios_len+len+sizeof(CAiwniosPack))(U64*)[1]=aiwnios_len;
    (final+hcrt_len+aiwnios_len+len+sizeof(CAiwniosPack))(U64*)[0]='AiwnPack';
    FileWrite(result_name,final,total_len);
  } else {
    if(de=FilesFind("/aiwnios.exe"))
       goto pass;
  }
  Free(ram_disk); 
}
U0 AiwniosPackLoad() {
  I64 l;
  U8 *dumb,*dumb2;
//Dont Free
  U8 *ram_disk=AiwniosPackRamDisk(&l);
  if(!ram_disk)
    return;
  dumb=MStrPrint("%C:/",
	blkdev.boot_drv_let=MountRamDisk(ram_disk,l)->drv_let);
  dumb2=MStrPrint("Cd(\"%Q\");",dumb);
  TaskExe(Gs->seth_task,Fs,dumb2,0);
  blkdev.home_dir=AStrNew(dumb);
  Cd(dumb);
  Free(dumb);
  User(AiwniosPackBootCommand);
}
/*
I64 len;
U8 *rd=RamDiskImage("/Doc",&len);
CDrv *d=MountRamDisk(rd,len);
*/
