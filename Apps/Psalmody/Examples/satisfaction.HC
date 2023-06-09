//5 has graphics
//Melody by Terry A. Davis




$SP,"<1>",BI=1$





$SP,"<2>",BI=2$



$SP,"<3>",BI=3$


F64 t0=Beat;

#define PHASE1	9.5
#define PHASE2	10.0
#define PHASE3	10.5
#define PHASE4	16.0
#define PHASE5	16.5

#define MAN_X	250
#define MAN_Y	30

U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0;
  U8 *tmps;
  if (dt<PHASE1)
    Sprite3(dc,MAN_X,MAN_Y,0,$IB,"<1>",BI=1$);
  else if (dt<PHASE2) {
    tmps=SpriteInterpolate((dt-PHASE1)/(PHASE2-PHASE1),$IB,"<1>",BI=1$,$IB,"<2>",BI=2$);
    Sprite3(dc,MAN_X,MAN_Y,0,tmps);
    Free(tmps);
  } else if (dt<PHASE3) {
    tmps=SpriteInterpolate((dt-PHASE2)/(PHASE3-PHASE2),$IB,"<2>",BI=2$,$IB,"<3>",BI=3$);
    Sprite3(dc,MAN_X,MAN_Y,0,tmps);
    Free(tmps);
  } else if (dt<PHASE4)
    Sprite3(dc,MAN_X,MAN_Y,0,$IB,"<3>",BI=3$);
  else if (dt<PHASE5) {
    tmps=SpriteInterpolate((dt-PHASE4)/(PHASE5-PHASE4),$IB,"<3>",BI=3$,$IB,"<1>",BI=1$);
    Sprite3(dc,MAN_X-100*(dt-PHASE4),MAN_Y-5*(dt-PHASE4),0,tmps);
    Free(tmps);
  } else
    Sprite3(dc,MAN_X-100*(PHASE5-PHASE4),MAN_Y-5*(PHASE5-PHASE4),0,$IB,"<1>",BI=1$);
}

U0 Song()
{
  CTask *task;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  MusicSettingsRst;
  music.tempo= 2.350;
  music.stacatto_factor= 0.900;
  try {
    task=PopUpViewPrint(
	  "$LK,"Ecclesiastes,2:24",A="BF:Ecclesiastes,2:24"$\n"
	  "$LK,"Ecclesiastes,3:13",A="BF:Ecclesiastes,3:13"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,TEXT_COLS-10,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    Fs->draw_it=&DrawIt;
    Fs->task_end_cb=&SndTaskEndCB;
    while (!ScanKey) {
      t0=Beat;
      Play("M5/45qE4A5eDDCC4qB5E4A5eDD",
	    "Up\0a\0bove\0He\0gen\0tly\0smiles\n\0as\0we\0meet\0and\0");
      Play("5CC4qB5F4B5eEEDDqCF",
	    "con\0quer\0trials.\n\0$$RED$$You\0can\0"
	    "do\0it\0lit\0tle\0one.\n\0With\0");
      Play("4B5eEEDDqC",
	    "out\0work\0there\0is\0no\0fun.\n$FG$\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
        �         
U      T      
T      X       
X       Z      
Z      ]      
W      \   ����
X   ����U   ����
X   ����U   ����
[   ����]   ����
]   ����[       
U   ����T   ����
T   ����R   ����        �         
U      O      
O      X       
X       [      
[      `      
W      U   ����
X   ����O   ����
R   ����M   ����
U   ����S   ����
S   ����Q      
O   ����O   ����
O   ����K              �         
U      K      
K      Q      
R      U      
U      `      
R      J   ����
Q      D   ����
F   ����=   ����
J   ����L   
   
L   
   F      
D   ����F      
F      >       