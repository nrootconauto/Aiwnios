//5 has graphics


	$SP,"<1>",BI=1$



	$SP,"<2>",BI=2$


	$SP,"<3>",BI=3$

F64 t0=Beat;

U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0,tt=Saw(dt,8);
  U8 *tmps;
  if (dt<8.0)
    tmps=SpriteInterpolate(tt,$IB,"<1>",BI=1$,$IB,"<2>",BI=2$);
  else if (dt<16.0)
    tmps=SpriteInterpolate(tt,$IB,"<2>",BI=2$,$IB,"<3>",BI=3$);
  else
    tmps=SpriteInterpolate(0,$IB,"<3>",BI=3$,$IB,"<3>",BI=3$);
  Sprite3(dc,220,33,0,tmps);
  Free(tmps);
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      t0=Beat;
	Play("5eE4B5DC4qBG5eC4BqA5eC4BqA",
		"Do\0not\0whine\0at\0age.\n\0\0\0\0\0\0\0\0");
	Play("5eE4B5DC4qBG5eC4BqA5eC4BqA",
		"Do\0not\0whine\0at\0age.\n\0\0\0\0\0\0\0\0");
	Play("5eD4sG5E4eA5sGDqDsDE4eB5e.F4sB5F4G5F4G5qEG");
	Play("5eD4sG5E4eA5sGDqDsDE4eB5e.F4sB5F4G5F4G5qEG");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
        �          ��������    ����                                               
                                        	   ����       ����       ����       ����    ��������                   �          ��������    ��������                        
   	       
          
                                        	   ����       ����       ����       ����        ����                   �          ��������    ����                                                                      ����                 ����    
   ����       ����       ����    ��������            