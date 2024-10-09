Bool LBEqu(U8 *ptr,I64 bit,Bool b) {
  if(b)
    return LBts(ptr,bit);
  return LBtr(ptr,bit);
}

U16 EndianU16(U16 d)
{//Swap big<-->little endian.
  I64 res=0;
  res.u8[1]=d.u8[0];
  res.u8[0]=d.u8[1];
  return res;
}

U32 EndianU32(U32 d)
{//Swap big<-->little endian.
  I64 res=0;
  res.u8[3]=d.u8[0];
  res.u8[2]=d.u8[1];
  res.u8[1]=d.u8[2];
  res.u8[0]=d.u8[3];
  return res;
}

I64 EndianI64(I64 d)
{//Swap big<-->little endian.
  U64 res;
  res.u8[7]=d.u8[0];
  res.u8[6]=d.u8[1];
  res.u8[5]=d.u8[2];
  res.u8[4]=d.u8[3];
  res.u8[3]=d.u8[4];
  res.u8[2]=d.u8[5];
  res.u8[1]=d.u8[6];
  res.u8[0]=d.u8[7];
  return res;
}

I64 BCnt(I64 d)
{//Count set bits in I64.
  I64 res=0,i;
  for (i=0;i<8;i++)
    res+=set_bits_table[d.u8[i]];
  return res;
}

F64 tS()
{//Time since boot in seconds as a float.
  return __GetTicksHP/1000000.;
}

Bool Blink(F64 Hz=2.5)
{//Return TRUE, then FALSE, then TRUE at given frequency.
  if (!Hz) return 0;
  return ToI64(cnts.jiffies*2*Hz/JIFFY_FREQ)&1;
}

U0 SleepUntil(I64 wake_jiffy)
{//Not for power-saving.  It is to make a program pause without hogging the CPU.
  Bool old_idle=LBts(&(Fs->task_flags),TASKf_IDLE);
  Fs->wake_jiffy=wake_jiffy;
  Yield;
  if(old_idle)
    LBts(&(Fs->task_flags),TASKf_IDLE);
  else
    LBtr(&(Fs->task_flags),TASKf_IDLE);
}

U0 Sleep(I64 mS)
{//Not for power-saving.  It is to make a program pause without hogging the CPU.
  Bool old_idle=LBts(&(Fs->task_flags),TASKf_IDLE);
  if (!mS)
    Yield;
  Fs->wake_jiffy=__GetTicks+mS;
  Yield;
  if(old_idle)
    LBts(&(Fs->task_flags),TASKf_IDLE);
  else
    LBtr(&(Fs->task_flags),TASKf_IDLE);
}

F64 Ona2Freq(I8 ona)
{//Ona to freq. Ona=60 is 440.0Hz.
  if (!ona)
    return 0;
  else
    return 440.0/32*Pow(2.0,ona/12.0);
}

I8 Freq2Ona(F64 freq)
{//Freq to Ona. 440.0Hz is Ona=60.
  if (freq>0)
    return ClampI64(12*Log2(32.0/440.0*freq),1,I8_MAX);
  else
    return 0;
}

U0 Snd(I8 ona=0)
{//Play ona, a piano key num. 0 means rest.
  SndFreq(Ona2Freq(ona));
}

Bool ScrnCast(Bool val=ON,Bool just_audio=FALSE,U8 *print_fmt="T:/Video/%X.GR")
{//WinMgr saves GR files to a dir.
  Bool old_val;
  scrncast.just_audio=just_audio;
  if (val) {
    if (!(old_val=LBtr(&scrncast.record,0))) {
      Free(scrncast.print_fmt);
      scrncast.print_fmt=StrNew(print_fmt,adam_task);
      scrncast.t0_now=Now;
      scrncast.snd_head.tS=scrncast.t0_tS=tS;
      scrncast.snd_head.ona=scrncast.ona;
      LBts(&scrncast.record,0);
    }
  } else
    old_val=LBtr(&scrncast.record,0);
  Snd;
  return old_val;
}

U0 SndRst()
{//Fix stuck sound.
  Snd;
}

U0 Beep(I8 ona=62,Bool busy=FALSE)
{//Make beep at given ona freq.
  Snd(ona);
  Sleep(500);
  Snd;
  Sleep(200);
}

Bool Mute(Bool val)
{//Turn-off sound.
  Bool res=FALSE;
  /* TODO RESTORE
  if (val) {
    Snd;
    res=LBts(&sys_semas[SEMA_MUTE],0);
  } else
    res=LBtr(&sys_semas[SEMA_MUTE],0);
  */
  return res;
}

Bool IsMute()
{//Return is-mute flag.
  //return Bt(&sys_semas[SEMA_MUTE],0);
  return 0; //TODO RESTORE
}

Bool Silent(Bool val=ON)
{//Turn-off StdOut console text. (Not sound.)
  return LBEqu(&(Fs->display_flags),DISPLAYf_SILENT,val);
}

Bool IsSilent()
{//Return StdOut turned-off?
  return Bt(&(Fs->display_flags),DISPLAYf_SILENT);
}

Bool SysDbg(Bool val)
{//Set SysDbg bit you can use while debugging.
  //return LBEqu(&sys_semas[SEMA_DEBUG],0,val);
  return 0; //TODO RESTORE
}

Bool IsSysDbg()
{//Return SysDbg bit.
  //return Bt(&sys_semas[SEMA_DEBUG],0);
  return 0; //TODO RESTORE
}

Bool Raw(Bool val)
{//Set to direct scrn, BLACK & WHITE, non-windowed output mode.
  return 0; //TODO RESTORE
  /*if (!val)
    VGAFlush;
  return !LBEqu(&(Fs->display_flags),DISPLAYf_NOT_RAW,!val);
  */
}

Bool IsRaw()
{//Are we in BLACK & WHITE raw scrn mode?
  return IsCmdLineMode;//return !Bt(&(Fs->display_flags),DISPLAYf_NOT_RAW); TODO RESTORE
}

Bool SingleUser(Bool val)
{//Set single-user mode.
  return 0;//LBEqu(&sys_semas[SEMA_SINGLE_USER],0,val); TODO RESTORE
}

Bool IsSingleUser()
{//Return single-user mode.
  return 0; //TODO RESTORE
}

Bool DbgMode(Bool val)
{//Set dbg-mode.
  return 0; //TODO RESTORE
}

Bool IsDbgMode()
{
  return 0; //TODO RESTORE
}

U0 ProgressBarsRst(U8 *path=NULL)
{//Reset all progress bars to zero.
  ProgressBarsRegTf(path);
  MemSet(sys_progresses,0,4*sizeof(CProgress));
}

