U0 SysGlblsInit()
{
  I64 i,j;

  sys_focus_task=Fs;
  QueInit(&sys_macro_head);

  /*blkdev.dft_iso_filename	=AStrNew(DFT_ISO_FILENAME); TODO  RESTORE
  blkdev.dft_iso_c_filename	=AStrNew(DFT_ISO_C_FILENAME);
  blkdev.dvd_boot_is_good=TRUE;
  */
  blkdev.tmp_filename=StrNew("~/Tmp.DD.Z",adam_task);
  
  rev_bits_table=CAlloc(256);
  set_bits_table=CAlloc(256);
  for (i=0;i<256;i++)
    for (j=0;j<8;j++) {
      if (Bt(&i,7-j))	LBts(rev_bits_table+i,j);
      if (Bt(&i,j))	set_bits_table[i]++;
    }

  //fp_getstr2=&SysGetStr2; TODO RESTORE
  KeyDevInit;

  pow10_I64=CAlloc(sizeof(F64)*(308+308+1));
  for (i=-308;i<309;i++)
    pow10_I64[i+309]=Pow10(i);

  QueInit(&scrncast.snd_head);
  scrncast.t0_now=Now;
  scrncast.t0_tS=tS;
  scrncast.ona=scrncast.snd_head.ona=0;

  ProgressBarsRst;

  QueInit(&dev.pci_head);
  
  dbg.fun_seg_cache=CAlloc(FUN_SEG_CACHE_SIZE*sizeof(CFunSegCache));
}

U0 SysGrInit()
{
  text.font=sys_font_std;
  text.aux_font=sys_font_cyrillic;
  text.cols=GR_WIDTH/FONT_WIDTH;
  text.rows=GR_HEIGHT/FONT_HEIGHT;
  text.raw_scrn_image=CAlloc(GR_WIDTH*GR_HEIGHT/8);
  text.border_chars[2] (I64)=0x0908070605040302;
  text.border_chars[10](U32)=0x0D0C0B0A;
}

U0 KMain()
{//Continued from $LK,"KStart64.HC",A="FF:::/Kernel/KStart64.HC,I32 &KMain"$
  MemSet(sys_semas,0,8*SEMA_SEMAS_NUM);
  CBlkDev *bd;
  //BlkPoolsInit;TODO RESTORE
  SysGlblsInit;
  //UncachedAliasAlloc; TODO RESTORE
  //LoadKernel; TODO wut???
  SysGrInit;
  StrCpy(Fs->task_name,"Adam Task CPU00");
  StrCpy(Fs->task_title,Fs->task_name);
  Fs->title_src=TTS_TASK_NAME;
  Fs->win_right=text.cols-2;
  Fs->win_top++;
  Fs->win_bottom=text.rows; //TODO REMOVE
  TaskDerivedValsUpdate;


  SysDefinesLoad;
  //Core0Init; TODORESTORE?
  //IntInit1; TODO RESTORE

  //Before this point use $LK,"Snd",A="MN:Snd"$() and $LK,"Busy",A="MN:Busy"$()
  //to debug.  After this point, use $LK,"RawPrint",A="MN:RawPrint"$()
  //LBts(&sys_run_level,RLf_RAW); TODO RESTORE
  //"TempleOS V%5.3f\t%D %T\n\n", TODO RESTORE
	//sys_os_version,sys_compile_time,sys_compile_time;

  BlkDevsInitAll;
  //"DskChg(':');\n"; TODO RESTORE
  DskChg(':');
  
  /*DrvRep; TODO RESTORE
  Spawn(&MsHardDrvrInstall);*/
  
  MsInit2;
  MsInit;
  KbdInit;
  /*"Loading Compiler\n"; TODO RESTORE
  Cd("/Compiler");
  Load("Compiler",LDF_SILENT);
  LBts(&sys_run_level,RLf_COMPILER);
  */

  HomeSet("/");
  Cd("/");
  /* TODO RESTORE
  try ExeFile("StartOS"); //Continues $LK,"/StartOS.HC",A="FL:/StartOS.HC,1"$
  catch {
    Raw(ON);
    Silent(OFF);
    GetOutOfDollar;
    PutExcept;
    Dbg;
  }*/

  //SrvTaskCont; TODO RESTORE //Never to return
}
KMain;
