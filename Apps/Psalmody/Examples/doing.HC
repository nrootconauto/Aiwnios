//5 has graphics;




	   $SP,"<1>",BI=1$





F64 t0=Beat;
U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0;
  dc->flags|=DCF_TRANSFORMATION;
  DCDepthBufAlloc(dc);
  dc->x=320;
  dc->y=27;
  dc->z=GR_Z_ALL;
  Mat4x4RotX(dc->r,0.140*�);
  Mat4x4RotZ(dc->r,0.250*�*Sin(�*dt));
  Mat4x4RotY(dc->r,0.230*�);
  if (dt>16)
    Mat4x4RotY(dc->r,0.5*Sin(�/2*dt));
  Sprite3(dc,0,0,0,$IB,"<1>",BI=1$);
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
      "$$GREEN$$To God...$$FG$$\n";
      Play("5qG4BA5sEDEDqCG4B5C",
	    "Fa\0ther,\0\0what\0\0\0\0You\0been\0do\0ing?\n\0");
      Play("5G4BA5sEDEDqCG4B5C",
	    "Watch\0-it!\0\0Or,\0\0\0\0they'll\0be\0su\0ing.\n\0");
      Play("5eGDD4B5G4BqB5eE4A5EDFE4qG",
	    "They\0don't\0real\0ly\0un\0der\0stand\n\0"
	    "What\0\0You're\0\0up\0\0to.\n\0");
      Play("5eGDD4B5G4BqB5eE4A5EDFE4qG",
	    "High\0a\0bove\0our\0ways,\n\0\0\0If\0\0"
	    "they\0\0on\0ly\0knew.\n\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
        �        E   ��������   ����           ����   ��������   ��������   ��������   ��������   ����       ����      ����      ����                ��������    ��������    ����        ����        ��������    ��������    ��������        ����               ����       ����       ����       ����        ����    ��������   ��������    ��������   ����0           0        
   0        
   0     
   	   0        	   0     	      0     	      0     	      0            0            0           0           0           0           0           0           0           0           0           0           0           0           0           0           0           0           0           0           0           0            0            0           0            0           0           0           0           0        
   0           0     
   	   0     	      0     	      0     	      0           0           0           0           0  
         0           0           0           0           0           0           0           0     	      0     	      0           0           0           0           0           0     
      0           0           0           0           0           0            