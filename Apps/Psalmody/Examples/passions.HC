//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("5sDCDC4qA5DetDGFeDAqtEetF4eA5sC4A5e.GsE",
	    "Ad\0am\0\0Eve?\n\0\0\0\0\0\0\0\0\0\0"
	    "\0\0\0\0\0");
      Play("5DCDC4qA5DetDGFeDAqtEetF4eA5sC4A5e.GsE",
	    "No\0\0re\0prieve.\n\0\0\0\0\0\0\0\0\0"
	    "\0\0\0\0\0\0");
      Play("5e.C4sGe.B5sC4eA5sFC4FG5eCqE4eB5D4B5sC4B5eEE",
	    "Mon\0\0key\0\0\0\0\0want\0some\0fruit?\n\0"
	    "\0\0\0\0\0\0\0\0");
      Play("5e.C4sGe.B5sC4eA5sFC4FG5eCqE4eB5D4B5sC4B5eEE",
	    "Thou\0\0shalt\0\0\0\0\0wear\0a\0suit.\n\0"
	    "\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
