//5 has graphics
U0 DrawIt(CTask *task,CDC *dc)
{
  F64 x1,x2,y1,y2,x3,y3,radius,pedals,�,t=Beat,
	cx=335,cy=task->pix_height>>1;
  Bool first=TRUE;

  radius=cy*(1.0+Sin(2*�*t));
  pedals=8;

  dc->color=BLACK;
  for (�=0;�<2*�;�+=2*�/100) {
    P2R(&x2,&y2,radius*Abs(Sin(pedals/2*�))+5,�+t);
    if (!first)
      GrLine(dc,x1+cx,cy-y1,x2+cx,cy-y2);
    else {
      first=FALSE;
      x3=x2; y3=y2;
    }
    x1=x2; y1=y2;
  }
  GrLine(dc,x1+cx,cy-y1,x3+cx,cy-y3);

  dc->color=(2*t)%16;
  GrFloodFill(dc,cx,cy);
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  MusicSettingsRst;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;

  music.tempo=2.5;
  try {
    while (TRUE) {
      Play("4qBqG5sG4A5G4A5etC4BBqAqGsBBBB5eFE",
	    "I \0am \0feeling \0\0\0\0\0\0brave.\n\0");
      Play("4qBqG5sG4A5G4A5etC4BBqAqGsBBBB5eFE",
	    "I \0am \0feeling \0\0\0\0\0\0brave.\n\0");
      Play("5qCsDEDEq4G5qD4qBqA5qGq4G",
	    "Test \0me \0\0\0\0Lord. \0See \0if \0I'm \0wor\0thy.\n\0");
      Play("5qCsDEDEq4G5qD4qBqA5qGq4G",
	    "Test \0me \0\0\0\0Lord. \0See \0if \0I'm \0wor\0thy.\n\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
