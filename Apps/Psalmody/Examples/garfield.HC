//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("5eGsD4B5eFCFF4GGB5sEEeDsDCqE4etA5ED",
	    "I\0don't\0\0get\0that\0joke.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("5eGsD4B5eFCFF4GGB5sEEeDsDCqE4etA5ED",
	    "To\0much\0\0pot\0I\0smoke.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("5C4GB5sDEDEeF4B5etE4G5C4qGeAGqG5sGFGF",
	    "\0$$RED$$You\0\0can\0\0re\0\0cover!$$FG$$\n\0"
	    "\0\0\0\0I'm\0not\0sure.\n\0\0\0\0\0\0");
      Play("5etC4GB5sDEDEeF4B5etE4G5C4qGeAGqG5sGFGF",
	    "\0My\0\0cell\0\0smoth\0\0er.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
