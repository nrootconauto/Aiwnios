//5 has graphics

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"minute near appeareth rolled "
"imagination streaming flashes ambition "
"bewail goading suggest bad "
"abide rightly shelter workmaster "
"directing panted prices loosed "
$ID,-2$
);
$ID,-2$




	$SP,"<1>",BI=1$






F64 t0=Beat;
U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0;
  DCDepthBufAlloc(dc);
  dc->x=295;
  dc->y=50;
  dc->z=GR_Z_ALL;
  dc->flags|=DCF_TRANSFORMATION;
  if (dt<16) {
    Mat4x4RotZ(dc->r,(0.3*Sin(dt*�/2))+�/2);
  } else
    Mat4x4RotZ(dc->r,5*�/2);
  Mat4x4RotX(dc->r,-10*�/8);
  Mat4x4Scale(dc->r,0.6);
  Sprite3(dc,0,0,0,$IB,"<1>",BI=1$);
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.770;
  try {
    while (!ScanKey) {
      t0=Beat;
      Play("4eABAB5C4A5qDeFEqDeFEqA",
	    "$$BLACK$$Alph\0a\0male\0\0is\0se\0cure.\n\0"
	    "Be\0ta\0test?\n\0Is\0not\0blessed?\n\0");
      Play("4eABAB5C4A5qDeFEqDeFEqC",
	    "Alph\0a\0male\0\0is\0se\0cure.\n\0"
	    "Be\0ta\0test?\n\0Is\0not\0blessed?\n\0");
      Play("6E5eAB6qCGGC5A6D",
	    "The\0soft\0\0ware\0is\0not\0ru\0\0nning.\n\0");
      Play("E5eAB6qCGGC5A6D",
	    "The\0re\0\0lease\0is\0not\0co\0\0ming.\n\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
        V        "   X           L   ����    L                         ����        ����   ��������             ����      ����      ��������   ��������   ����          ��������������������       ��������   ��������   ��������������������������������   ����        ����P       ����           P                                                                                                                                                      	     	           
                 
                           
              	   
                                                                                                                                                                  