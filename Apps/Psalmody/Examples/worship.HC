//5 has graphics





	$SP,"<1>",BI=1$








	$SP,"<2>",BI=2$




F64 t0=Beat;
U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0;
  U8 *tmps;
  DCDepthBufAlloc(dc);
  dc->x=280;
  dc->y=30;
  dc->z=GR_Z_ALL;
  dc->flags|=DCF_TRANSFORMATION;
  if (dt<16) {
    Mat4x4RotZ(dc->r,(dt+5)*�/2);
    Mat4x4RotY(dc->r,FullTri(dt,8)*�/8);
  } else
    Mat4x4RotZ(dc->r,5*�/2);
  Mat4x4RotX(dc->r,-11*�/8);
  Mat4x4Scale(dc->r,0.75);
  if (dt<16)
    Sprite3(dc,0,0,0,$IB,"<1>",BI=1$);
  else if (16<dt<17 || 23<dt<25 || 31<dt) {
    tmps=SpriteInterpolate(Tri(dt-16,2.0),$IB,"<1>",BI=1$,$IB,"<2>",BI=2$);
    Sprite3(dc,0,0,0,tmps);
    Free(tmps);
  } else
    Sprite3(dc,0,0,0,$IB,"<2>",BI=2$);
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
      Play("5qEDeCCetE4AA5qEetCDC4A5GFqG",
	    "I\0don't\0know\0\0\0\0\0\0\0\0"
	    "what\0\0to\0\0pray.\n\0");
      Play("5EDeCCetE4AA5qEetCDC4A5GFqG");
      Play("5eGEetFD4A5qFsGFGFGFGF4etB5GFqGeDE",
	    "It's\0a\0blah\0\0\0day.\n\0\0\0\0"
	    "\0\0\0\0\0\0\0\0\0\0\0");
      Play("5GEetFD4A5qFsGFGFGFGF4etB5GFqGeDE");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
        �         
       2       ����                       ����           
       ����
           ����                                                                    	            	           	           	                    �         
       2   ��������                       ��������       
       ����
           ����                                                                    	            	           	           	             