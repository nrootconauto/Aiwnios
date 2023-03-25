//5 has words

U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("5eC4sAA5eC4sAA5eD4AB5RCqE4sBB5eEDFE",
	    "Bill\0\0\0Gates...\0\0\0\0\0"
	    "\0\0\0\0\0\0\0Life's\0not\0fair.\n\0");
      Play("5C4A5qCsDD4AAeB5RCDsCC4BB5eE4A5FsEE",
	    "Heavy\0\0weights...\0\0\0\0\0"
	    "\0\0\0\0\0\0\0\0\0Done\0my\0share!\n\0\0");
      Play("5G4G5G4G5qG4eBBB5FsD4G5eD4sBGBGeBA5sGF4eG",
	    "God's\0\0\0\0World?\0\0\0\0"
	    "Per\0fect\0\0ly\0\0\0\0\0\0\0just.\n\0\0\0");
      Play("5sG4G5G4G5qG4eBBB5FsD4G5eD4sBGBGeBA5sGF4eG",
	    "Thank's\0\0\0\0Bill,\0\0\0\0"
	    "but\0God\0\0we\0\0\0\0\0\0\0trust.\n\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
