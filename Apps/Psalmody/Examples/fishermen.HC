//5 has graphics
$SP,"<1>",BI=1$




U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,w=task->pix_width,h=task->pix_height;
  dc->color=BROWN;
  for (i=60*Saw(Beat,20);i<w;i+=20)
    GrLine(dc,i,h,w,h-i/4);
  dc->color=BLACK;
  GrLine(dc,0,h-1,w,h-1);
  GrLine(dc,w-1,0,w-1,h);
}

U0 Song()
{
  CTask *task;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=LTCYAN<<4+BLACK;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
    task=PopUpViewPrint(
	  "$LK,"BF:Matthew,4:19"$\n"
	  "$LK,"BF:Luke,5:4"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,TEXT_COLS-10,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      "$$CL$$$$BG,LTCYAN$$";
      Play("5qEsGEGEeF4GB5CqCeDE4qB5sERER",
	    "$$FG$$Lord,\0I\0\0am\0\0feel\0ing\0"
	    "just\0a\0bit\0un\0in\0spired.\n\0With\0\0\0\0");
      Play("5qEsGEGEeF4GB5CqCeDE4qB5sERER",
	    "net\0or\0\0with\0\0reel\0ing\0\0"
	    "I\0keep\0get\0ting\0mired.\n\0\0\0\0\0");
      Play("5qD4etRA5FqDGetFC4GsB5C4B5CqDF",
	    "$$RED$$Throw\0\0out\0your\0net.\n\0\0\0"
	    "\0\0\0\0\0\0\0\0");
      Sprite($IB,"<1>",BI=1$);
      Play("5D4etRA5FqDGetFC4GsB5C4B5CqDF",
	    "Make\0\0you\0a\0bet.\n\0\0\0\0\0\0"
	    "\0\0\0\0\0");
      '\n\n\n\n\n';
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
             
            
         "   
   "   -      
-      ;      
;      F      
I      F   #   
F   #   P      
L      T      
T      ]      
]      `      
`      R      
R      B      
B      7   	   
7   	   6      
6      %      
%            
            
U      X      
V      Y      
   N      N      N      P      S      T      W      S      P      P      J      J      J      L      P      P      F      E      C      C      C      F      F      F      G      H      E      E      @      A      A      A      C      D      A      >      =      =      <      <      9      6      8      9      :      :      :      :      7      6      4      4      6      6      6      3      2      1      -      -      .      0      -      +      )      )      *      *      %      %      %      Y      U      R      T      W      Y      Z      ]      [       