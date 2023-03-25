//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("4qBe.GsA5DCeDsGE4eB5qGe.DsGeE4G5G4sB5C",
	    "God\0spoke.\n\0\0\0\0\0\0\0\0\0\0"
	    "How\0\0'bout\0that!\n\0\0\0");
      Play("4qBe.GsA5DCeDsGE4eB5qGe.DsGeE4G5G4sB5C",
	    "God\0spoke.\n\0\0\0\0\0\0\0\0\0\0"
	    "Where's\0\0He\0at?\n\0\0\0");
      "$$GREEN$$God talking...\n$$FG$$";
      Play("5eF4BB5sD4A5ECeC4qBe.GsA5F4G5eFe.G4sB5eDD",
	    "\0\0\0\0\0\0\0\0\0\0\0"
	    "$$RED$$I'm\0\0right\0here.\n\0\0\0\0");
      Play("5F4BB5sD4A5ECeC4qBe.GsA5F4G5eFe.G4sB5eDD",
	    "\0\0\0\0\0\0\0\0\0\0\0"
	    "You\0\0can\0hear!$$FG$$\n\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
