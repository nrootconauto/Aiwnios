//5 has graphics

$SP,"<1>",BI=1$








#define LH_X 260
#define LH_Y 20

F64 t0=Beat;

U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0,y;
  Sprite3(dc,LH_X,LH_Y,0,$IB,"<1>",BI=1$);
  if (dt>24) {
    dc->color=YELLOW;
    for (y=-30;y<30;y+=5)
      GrLine(dc,LH_X,LH_Y,LH_X+80*(dt-28),LH_Y+y);
  }
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;

  MusicSettingsRst;
  music.tempo= 2.500;
  music.stacatto_factor= 0.900;
  try {
    while (!ScanKey) {
      t0=Beat;
      Play("4etA5CDsEEEEetGG4GqGetGBAqBetRB5GC4A5G",
	    "I\0walk\0\0bo-\0o-\0ld\0ly\0\0\0\0\0\0"
	    "though\0the\0fog.\n\0\0\0\0\0\0\0");
      Play("4A5CDsEEEEetGG4GqGetGBAqBetRB5GC4A5G",
	    "I\0got\0\0lit\0\0tle\0\0choice\0\0\0\0"
	    "but\0to\0trust\0God.\n\0\0\0\0\0\0\0");
      Play("4qB5etCCCqCCeCCCDqEetE4B5G",
	    "\0Don't\0\0kno-\0ow\0\0\0\0what\0"
	    "to\0do.\n\0\0\0\0");
      Play("4qB5etCCCqCCeCCCDqEetE4B5G",
	    "\0Where\0I\0go-\0o,\0\0\0\0I've\0"
	    "no\0clue.\n\0\0\0\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
        �      
������������-   
��������   ����
   ����   ,   
   ,   ����,   
����&         
����         
����	         
����         
         ����
   ������������
������������   
   +      7   
����.   ����9   
����1   ����=   
   0      ;   
����5   ����>   
   2      =   
   4      >      
����         
����'         
����         
����.      .   
   1   k   4   
����0   ����1   
   -   k   /   
   -   
   (   
����(   ����,   
����,   ����-    