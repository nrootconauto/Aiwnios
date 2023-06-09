//5 has graphics

F64 t0=Beat;

U0 MyCircle3(CDC *dc,I64 x,I64 y,I64 z,I64 r)
{
  F64 �,d�=1.0/r;
  for (�=0;�<2*�;�+=d�)
    GrPlot3(dc,x+r*Cos(�),y+r*Sin(�),z);
}

U0 MyCylinder(CDC *dc,I64 x,I64 y,I64 z1,I64 z2,I64 r)
{
  I64 z;
  for (z=z1;z<z2;z++)
    MyCircle3(dc,x,y,z,r);
}

U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0,r1=12,r2=10,w1=25,w2=20,w3=18;

  DCDepthBufAlloc(dc);
  dc->x=300;
  dc->y=35;
  dc->z=GR_Z_ALL;
  dc->flags|=DCF_TRANSFORMATION;
  Mat4x4RotX(dc->r,Sin(�*dt/4)/8);
  Mat4x4RotY(dc->r,�*dt/16);

  w1+=3*Sin(�*dt/3);

  dc->color=BROWN;
  MyCylinder(dc,0,0,-w3,w3,r2);

  dc->color=YELLOW;
  MyCylinder(dc,0,0,-w1-w2/2+0,-w1-w2/2+1,r1);
  dc->color=BLACK;
  MyCylinder(dc,0,0,-w1-w2/2+1,-w1-w2/2+2,r1);
  dc->color=LTGRAY;
  MyCylinder(dc,0,0,-w1-w2/2+2,-w1+w2/2-2,r1);
  dc->color=BLACK;
  MyCylinder(dc,0,0,-w1+w2/2-2,-w1+w2/2-1,r1);
  dc->color=YELLOW;
  MyCylinder(dc,0,0,-w1+w2/2-1,-w1+w2/2,r1);

  dc->color=BLACK;
  GrLine3(dc,r1,0,-w1-w2/2+1, r1,0,-w1-w2/2+w2-2);

  dc->color=YELLOW;
  MyCylinder(dc,0,0,w1-w2/2+0,w1-w2/2+1,r1);
  dc->color=BLACK;
  MyCylinder(dc,0,0,w1-w2/2+1,w1-w2/2+2,r1);
  dc->color=LTGRAY;
  MyCylinder(dc,0,0,w1-w2/2+2,w1+w2/2-2,r1);
  dc->color=BLACK;
  MyCylinder(dc,0,0,w1+w2/2-2,w1+w2/2-1,r1);
  dc->color=YELLOW;
  MyCylinder(dc,0,0,w1+w2/2-1,w1+w2/2,r1);

  dc->color=BLACK;
  GrLine3(dc,r1,0,w1-w2/2+1, r1,0,w1-w2/2+w2-2);
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.824;
  try {
    while (!ScanKey) {
      t0=Beat;
      Play("4eG5sE4AeA5DDG4qG5sFGeCG4B5sDE4eG5E4B",
	    "Was\0\0Ga\0li\0le\0o\0\0wrong?\n\0"
	    "\0\0\0\0\0\0\0\0\0\0");
      Play("4G5sE4AeA5DsDDeG4qG5sFGeCG4B5sDE4eG5E4B",
	    "His\0\0brain\0\0was\0might\0\0y\0strong.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0");
      Play("5etEDEAGFeEGqDsDG4eAsA5G4A5G4qA5eEC",
	    "Did\0you\0\0real\0ly\0want\0to\0\0win?\n\0"
	    "\0\0\0\0\0\0\0\0\0\0");
      Play("5etEDEAGFeEGqDsDG4eAsA5G4A5G4qA5eEC",
	    "We\0have\0\0us,\0a\0real\0Huck\0\0Finn.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
