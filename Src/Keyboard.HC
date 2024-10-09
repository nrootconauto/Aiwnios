//http://www.rohitab.com/discuss/topic/39438-keyboard-driver/
U8 NORMAL_KEY_SCAN_DECODE_TABLE[0x5a]={
  0, CH_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', CH_BACKSPACE, '\t',  'q',
  'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd',
  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b',
  'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,	0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, '5', 0, '+', 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0
};

U8 SHIFT_KEY_SCAN_DECODE_TABLE[0x50]={
	0,CH_SHIFT_ESC,'!','@','#','\d','%','^','&','*','(',')','_','+',CH_BACKSPACE,'\t',
	'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S',
	'D','F','G','H','J','K','L',':','"','~',0,'|','Z','X','C','V',
	'B','N','M','<','>','?',0,'*',0,CH_SHIFT_SPACE,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0
};
U8 CTRL_KEY_SCAN_DECODE_TABLE[0x50]={
	0,CH_ESC,'1','2','3','4','5','6','7','8','9','0','-','=',CH_BACKSPACE,'\t',
	CH_CTRLQ,CH_CTRLW,CH_CTRLE,CH_CTRLR,CH_CTRLT,CH_CTRLY,CH_CTRLU,
		CH_CTRLI,CH_CTRLO,CH_CTRLP,'[',']','\n',0,CH_CTRLA,CH_CTRLS,
	CH_CTRLD,CH_CTRLF,CH_CTRLG,CH_CTRLH,CH_CTRLJ,CH_CTRLK,CH_CTRLL,
		';','\'','\`',0,'\\',CH_CTRLZ,CH_CTRLX,CH_CTRLC,CH_CTRLV,
	CH_CTRLB,CH_CTRLN,CH_CTRLM,',','.','/',0,'*',0,CH_SPACE,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0
};

I64 Char2ScanCode(I64 ch,I64 sc_flags=0)
{//ASCII val to scan code (Slow).
  I64 i;
  U8 *table;
  if (sc_flags) {
    table=NORMAL_KEY_SCAN_DECODE_TABLE;
    if (sc_flags & SCF_CTRL || ch<26)
      table=CTRL_KEY_SCAN_DECODE_TABLE;
    else if (sc_flags & SCF_SHIFT || 'A'<=ch<='Z') {
      if (!(sc_flags & SCF_CAPS))
	table=SHIFT_KEY_SCAN_DECODE_TABLE;
    } else {
      if (sc_flags & SCF_CAPS)
	table=SHIFT_KEY_SCAN_DECODE_TABLE;
    }
    for (i=0;i<0x50;i++)
      if (table[i]==ch)
	return i|sc_flags;
    return sc_flags;
  } else {
    table=NORMAL_KEY_SCAN_DECODE_TABLE;
    for (i=0;i<0x50;i++)
      if (table[i]==ch)
	return i;
    table=SHIFT_KEY_SCAN_DECODE_TABLE;
    for (i=0;i<0x50;i++)
      if (table[i]==ch)
	return i|SCF_SHIFT;
    table=CTRL_KEY_SCAN_DECODE_TABLE;
    for (i=0;i<0x50;i++)
      if (table[i]==ch)
	return i|SCF_CTRL;
    return 0;
  }
}

U8 ScanCode2Char(I64 sc)
{//Scan code to ASCII val.
  U8 *table=NORMAL_KEY_SCAN_DECODE_TABLE;
  if (sc&SCF_E0_PREFIX)
    return 0;
  if (sc&SCF_CTRL)
    table=CTRL_KEY_SCAN_DECODE_TABLE;
  else if (sc&SCF_SHIFT) {
    if (!(sc&SCF_CAPS))
      table=SHIFT_KEY_SCAN_DECODE_TABLE;
  } else {
    if (sc&SCF_CAPS)
      table=SHIFT_KEY_SCAN_DECODE_TABLE;
  }
  sc&=0x7F;
  if (sc>=0x50)
    return 0;
  else
    return table[sc];
}

U8 scan_code_map[0x100]={
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,SC_SHIFT,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,SC_ENTER,SC_CTRL,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0x35,0,0,SC_ALT,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,SC_HOME,
	SC_CURSOR_UP,SC_PAGE_UP,0,SC_CURSOR_LEFT,0,SC_CURSOR_RIGHT,0,SC_END,
  SC_CURSOR_DOWN,SC_PAGE_DOWN,SC_INS,SC_DELETE,0,0,0,0,
	0,0,0,0,SC_GUI,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

U8 num_lock_map[0x100]={
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,8,9,10,0,5,6,7,0,2,
  3,4,11,0x34,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,SC_ENTER,SC_CTRL,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0x35,0,0,SC_ALT,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,SC_HOME,
	SC_CURSOR_UP,SC_PAGE_UP,0,SC_CURSOR_LEFT,0,SC_CURSOR_RIGHT,0,SC_END,
  SC_CURSOR_DOWN,SC_PAGE_DOWN,SC_INS,SC_DELETE,0,0,0,0,
	0,0,0,0,SC_GUI,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

U8 *Char2KeyName(I64 ch,Bool include_ctrl=TRUE)
{//ASCII val to key name.
  I64 i;
  U8 buf[STR_LEN];
  if (ch<=CH_SPACE) {
    switch (ch) {
      case '\n':
	StrCpy(buf,"ENTER");
	break;
      case CH_BACKSPACE:
	StrCpy(buf,"BACKSPACE");
	break;
      case '\t':
	StrCpy(buf,"TAB");
	break;
      case CH_ESC:
	StrCpy(buf,"ESC");
	break;
      case CH_SHIFT_ESC:
	StrCpy(buf,"SHIFT_ESC");
	break;
      case 0: //nobound switch
      case 29:
      case 30:
	buf[0]=0;
	break;
      case CH_SHIFT_SPACE:
	StrCpy(buf,"SHIFT_SPACE");
	break;
      case CH_SPACE:
	StrCpy(buf,"SPACE");
	break;
      default:
	if (include_ctrl)
	  StrCpy(buf,"CTRL ");
	buf[i=StrLen(buf)]=ch-1+'a';
	buf[i+1]=0;
	break;
    }
  } else if (Bt(char_bmp_printable,ch)) {
    buf[0]=ch;
    buf[1]=0;
  } else
    buf[0]=0;
  return StrNew(buf);
}

U8 *ScanCode2KeyName(I64 sc)
{//Scan code to key name.
  I64 ch;
  U8 buf[STR_LEN],*st;
  buf[0]=0;
  if (sc&SCF_CTRL)
    CatPrint(buf,"CTRL ");
  if (sc&SCF_ALT)
    CatPrint(buf,"ALT ");
  if (sc&SCF_SHIFT)
    CatPrint(buf,"SHIFT ");
  if (sc&SCF_NO_SHIFT)
    CatPrint(buf,"      ");
  if (ch=ScanCode2Char(sc&255)) {
    st=Char2KeyName(ch,FALSE);
    StrCpy(buf+StrLen(buf),st);
    Free(st);
  } else {
    switch (sc&255) {
      case SC_BACKSPACE:CatPrint(buf,"BACK");	break;
      case SC_CAPS:	CatPrint(buf,"CAPS");	break;
      case SC_NUM:	CatPrint(buf,"NUM");	break;
      case SC_SCROLL:	CatPrint(buf,"SCROLL");	break;
      case SC_CURSOR_UP:CatPrint(buf,"UP");	break;
      case SC_CURSOR_DOWN:CatPrint(buf,"DOWN");	break;
      case SC_CURSOR_LEFT:CatPrint(buf,"LEFT");	break;
      case SC_CURSOR_RIGHT:CatPrint(buf,"RIGHT"); break;
      case SC_PAGE_UP:	CatPrint(buf,"PAGE_UP");  break;
      case SC_PAGE_DOWN:CatPrint(buf,"PAGE_DOWN");break;
      case SC_HOME:	CatPrint(buf,"HOME");	break;
      case SC_END:	CatPrint(buf,"END");	break;
      case SC_INS:	CatPrint(buf,"INS");	break;
      case SC_DELETE:	CatPrint(buf,"DELETE");	break;
      case SC_F1:	CatPrint(buf,"F1");	break;
      case SC_F2:	CatPrint(buf,"F2");	break;
      case SC_F3:	CatPrint(buf,"F3");	break;
      case SC_F4:	CatPrint(buf,"F4");	break;
      case SC_F5:	CatPrint(buf,"F5");	break;
      case SC_F6:	CatPrint(buf,"F6");	break;
      case SC_F7:	CatPrint(buf,"F7");	break;
      case SC_F8:	CatPrint(buf,"F8");	break;
      case SC_F9:	CatPrint(buf,"F9");	break;
      case SC_F10:	CatPrint(buf,"F10");	break;
      case SC_F11:	CatPrint(buf,"F11");	break;
      case SC_F12:	CatPrint(buf,"F12");	break;
      case SC_GUI:	CatPrint(buf,"WINDOWS");  break;
      case SC_PRTSCRN1:	CatPrint(buf,"PRTSCRN1"); break;
      case SC_PRTSCRN2:	CatPrint(buf,"PRTSCRN2"); break;
    }
  }
  return StrNew(buf);
}

U0 KbdBuildSC(U8 raw_byte,Bool in_irq,U8 *_last_raw_byte,I64 *_last_sc)
{
  I64 ch,sc_flags,sc,sc2,sc_raw,new_key_f;
  U0 (*cb)(I64);
  Bool set_LEDs=FALSE;
  if (raw_byte==0xE0) {
    *_last_sc&=~0x1FF;
    *_last_raw_byte=raw_byte;
    return;
  }
  sc=raw_byte;
  LBEqu(&sc,SCf_E0_PREFIX,*_last_raw_byte==0xE0);
  LBEqu(&sc,SCf_KEY_UP,raw_byte & 0x80);
  *_last_raw_byte=raw_byte;

  sc_flags=_last_sc->u32[0]&~0x1FF;
  sc_raw=sc;

  if (sc_flags & SCF_NUM) {
    if (sc2=num_lock_map[sc.u8[0]])
      sc.u8[0]=sc2;
  } else {
    if (sc2=scan_code_map[sc.u8[0]])
      sc.u8[0]=sc2;
  }

  new_key_f=SCF_NEW_KEY;
  if (sc&SCF_KEY_UP)
    switch (sc&~SCF_KEY_UP) {
      case SC_SHIFT:	sc_flags&=~SCF_SHIFT;	break;
      case SC_CTRL:	sc_flags&=~SCF_CTRL;	break;
      case SC_ALT:	sc_flags&=~SCF_ALT;	break;
      case SC_DELETE:	sc_flags&=~SCF_DELETE;	break;
      case SC_INS:	sc_flags&=~SCF_INS;	break;
      case SC_CAPS:	sc_flags^=SCF_CAPS;	set_LEDs=TRUE;	break;
      case SC_NUM:	sc_flags^=SCF_NUM;	set_LEDs=TRUE;	break;
      case SC_SCROLL:	sc_flags^=SCF_SCROLL;	set_LEDs=TRUE;	break;
    }
  else
    switch (sc) {
      case SC_SHIFT:
	if (LBts(&sc_flags,SCf_SHIFT)) new_key_f=0;
	break;
      case SC_CTRL:
	if (LBts(&sc_flags,SCf_CTRL)) new_key_f=0;
	break;
      case SC_ALT:
	if (LBts(&sc_flags,SCf_ALT)) new_key_f=0;
	break;
      case SC_DELETE:
	sc_flags|=SCF_DELETE;
	break;
      case SC_INS:
	sc_flags|=SCF_INS;
	break;
    }

  sc_flags|=new_key_f;
  sc=sc_flags|sc|(sc_flags|sc_raw)<<32;
  if (sc_flags & SCF_CTRL && sc_flags & SCF_ALT) {
    if (!(sc&SCF_KEY_UP)) {
      if (sc&255==SC_DELETE && !(sc_flags & SCF_SHIFT))
	;//CtrlAltDel(sc); TODO RESTORE
      else {
	if (sc&255==SC_ESC)
	  ch='t';
	else if (sc&255==SC_TAB)
	  ch='n';
	else
	  ch=ScanCode2Char(sc&255);
	if ('a'<=ch<='z') {
	  sc&=~(SCF_NEW_KEY|SCF_NEW_KEY<<32);
	  ch-='a';
	  kbd.last_down_scan_code=sc;
	  if (keydev.fp_ctrl_alt_cbs[ch] &&
		Bt(&keydev.ctrl_alt_in_irq_flags,ch)==in_irq &&
		(!(sc_flags & SCF_SHIFT)&&keydev.ctrl_alt_no_shift_descs[ch]) ||
		sc_flags & SCF_SHIFT && keydev.ctrl_alt_shift_descs[ch]) {
      cb=keydev.fp_ctrl_alt_cbs[ch];
	    (*cb)(sc);
    }
	}
      }
    }
  }
  *_last_sc=sc;
}

U0 KbdInit() {
  MemSet(&kbd,0,sizeof(CKbdStateGlbls));
  kbd.scan_code_fifo=FifoI64New(0x1000);
  kbd.scan_code_fifo2=FifoI64New(0x1000);
  kbd.irqs_working=TRUE; //???
}


I64 KbdMsgsQue()
{
  I64 arg1,arg2,msg_code=MSG_NULL;
  CTask *task_focus;
  if (task_focus=sys_focus_task) {
    while (FifoI64Rem(kbd.scan_code_fifo,&arg2)) {
      arg1=ScanCode2Char(arg2);
      if (arg2 & SCF_KEY_UP) {
	TaskMsg(task_focus,0,MSG_KEY_UP,arg1,arg2,0);
	msg_code=MSG_KEY_UP;
      } else {
	TaskMsg(task_focus,0,MSG_KEY_DOWN,arg1,arg2,0);
	msg_code=MSG_KEY_DOWN;
      }
    }
  }
  return msg_code;
}

I64 KbdMsEvtTime()
{//Timestamp of last key or mouse event.
  if (ms_hard.timestamp>kbd.timestamp)
    return ms_hard.timestamp;
  else
    return kbd.timestamp;
}

I64 kb_mtx=0;
U0 InteruptRt(I64 rip,U8 **rbp) {
  U8 **ptr;
  DbgPrint("Interupt At:%p",rip,rip);
  while(ChkOnStk(rbp)) {
    ptr=rbp+1;
    DbgPrint(",%p",*ptr);
    if(rbp>=*rbp)
      break;
    rbp=*rbp;
  }
  DbgPrint("\n");
  Yield;
}
U0 KBCb(I64 c,I64 s) {
  I64 initial_time,freeze_time=100,time_out;
  while(LBts(&kb_mtx,0))
    ; //TODO ATOMIC WAIT
  if(!(s&SCF_KEY_UP))
    kbd.cnt++;
  kbd.last_down_scan_code=s;
  FifoI64Ins(kbd.scan_code_fifo2,s);
  kbd.timestamp=cnts.jiffies;
  c=ScanCode2Char(s&0x7f);
  if(!(s&SCF_KEY_UP)&&s&SCF_ALT&&s&SCF_CTRL&&(c=='c'||c=='x')&&TaskValidate(sys_focus_task)) {
    if (__GetTicks>=cpu_structs[0].last_yield+500)
      InteruptCore(0);
  }
  LBtr(&kb_mtx,0);
}
SetKBCallback(&KBCb);
U0 CheckForKBData() {
  U0 (*cb)(I64);
  I64 s,c,time_out;
loop:
  if(!FifoI64Rem(kbd.scan_code_fifo2,&s)) return;
  FifoI64Ins(kbd.scan_code_fifo,s);
  kbd.scan_code=s;
  switch(s&0xff) {
    break; case 0x0 ... 0x7f:
    LBEqu(&kbd.down_bitmap,s&0x7f,!(s&SCF_KEY_UP));
    break; case SC_GUI:
    LBEqu(&kbd.down_bitmap,SC_GUI,!(s&SCF_KEY_UP));
  }
  c=ScanCode2Char(s&0x7f);
  if(keydev.fp_ctrl_alt_cbs&&!(s&SCF_KEY_UP)&&s&SCF_ALT&&s&SCF_CTRL&&'a'<=c<='z') {
    if(cb=keydev.fp_ctrl_alt_cbs[c-'a'])
      (*cb)(s);
  }
  if(!(s&SCF_KEY_UP)&&s&SCF_ALT&&s&SCF_CTRL&&(c=='c'||c=='x')&&TaskValidate(sys_focus_task)) {
    if(c=='c') {
       if(!Bt(&SYS_CTRL_ALT_FLAGS,CTRL_ALT_C)) {
         LBts(&SYS_CTRL_ALT_FLAGS,CTRL_ALT_C);
         goto await_yeild;
       }
    } else if(c=='x') {
      if(!Bt(&SYS_CTRL_ALT_FLAGS,CTRL_ALT_X)) {
        LBts(&SYS_CTRL_ALT_FLAGS,CTRL_ALT_X);
        await_yeild:
        time_out=cnts.jiffies;
        while(time_out-__GetTicks<ToI64(0.05*1000)) {
          /*
          if(IsFrozen(cpu_structs[0])) {
            InteruptCore(0);
            break;
          } else
            break;
            */
          __Sleep(2);
        }
      }
    }
  }
  goto loop;
}
