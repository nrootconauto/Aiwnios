//5 has graphics
U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  F64 dt=Beat%8.0,z,w,x1,y1,z1,x2,y2,z2;
  Bool first=TRUE;
  DCDepthBufAlloc(dc);
  dc->flags|=DCF_TRANSFORMATION;
  dc->x=30*dt+20.0*Sin(dt)+25;
  dc->y=10*dt-15;
  dc->z=GR_Z_ALL;
  Mat4x4RotX(dc->r,0.75*dt);
  Mat4x4RotY(dc->r,0.45*dt);
  Mat4x4RotZ(dc->r,0.50*dt);
  Mat4x4Scale(dc->r,0.5);
  for (i=-50;i<40;i+=2) {
    if (i&2)
      dc->color=LTGRAY;
    else
      dc->color=DKGRAY;
    z=10*Cos(i/10.0*�/8.0)*Cos(dt);
    w=Tri(100+i,200);
    x1=i-3;y1=0;z1=z;
    if (-30<=i<=40) {
      GrLine3(dc,i,-10*w,z,i-3,0,z);
      GrLine3(dc,i,+10*w,z,i-3,0,z);
    }
    if (!first) {
      dc->color=DKGRAY;
      GrLine3(dc,x1,y1,z1,x2,y2,z2);
    }
    x2=x1;y2=y1;z2=z1;
    first=FALSE;
  }
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=LTCYAN<<4+BLACK;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    "$$BG+H,LTCYAN$$";
    while (!ScanKey) {
      Play("5qE4AA5etF4G5GqDFetFED4eG5G",
	    "A\0dam...\0\0no\0\0\0long\0er\0sees\0"
	    "Your\0face.\n\0\0\0");
      Play("5qE4AA5etF4G5GqDFetFED4eG5G",
	    "Oh!\0\0\0What\0\0a\0wretch\0ed\0bar\0"
	    "ren\0waste!\n\0\0\0");
      Play("4qA5CF4eB5C4BGqG5ED",
	    "\0\0\0\0\0\0\0\0Ex\0iled.\n\0");
      Play("4A5CF4eB5C4BGqG5ED");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
