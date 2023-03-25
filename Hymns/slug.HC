//5 has graphics

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"moppet's emulator's Farley chores "
"witness Dzungaria formed sicks "
"adeptness Sheila's shapely Fri "
"earnests clammy elementary charioteer's "
"adept kitten Gingrich's roughly "
$ID,-2$
);
$ID,-2$
F64 t0=Beat;

U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0;
  I64 i;
  for (i=200;i<300;i+=10) {
    dc->thick=8*Tri(dt,1)+1;
    dc->color=winmgr.updates&15;
    GrLine3(dc,20*Sin(2*dt)+i,12*Tri(dt*4,4)+30+30*Sin(dt/2+i),0,
	  20*Sin(2*dt)+i+10,12*Tri(dt*4,4)+30+30*Sin(dt/2+i+10),0);
  }
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.516;
  try {
    while (!ScanKey) {
	t0=Beat;
	Play("4qA5eEsCCeEE4qB5eDD4sAAAAqGG");
	Play("5EeDEqFeEFsEEqEsEEqEeRE",
		"\0\0\0\0\0\0\0\0\0Led.\n\0\0\0\0\0");
	Play("FqCeDE4qB5eDD4sAAAAqGG5E");
	Play("eDEqFeEFsEEqEsEEqEReDC",
		"\0\0\0\0\0\0\0\0Lead.\n\0\0\0\0\0\0");
	Play("etGECeGsEC4qAeBsGGeGBqGeB5sDD4qB");
	Play("B5eFDE4G5qF4eG5CqDCeDC",
		"\0\0\0\0\0\0Hap\0py\0trails.\n\0\0\0\0");
	Play("etGECeGsEC4qAeBsGGeGBqGeB5sDD4qB");
	Play("B5eFDE4G5qF4eG5CqDC",
		"\0\0\0\0\0\0No\0\0fails.\n\0\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
