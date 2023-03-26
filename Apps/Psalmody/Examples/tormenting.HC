//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("5eC4BB5C4B5CCEE4A5GG4AA5sE4AA5E",
	    "I'm\0\0\0in\0\0trou\0ble.\n\0\0\0"
	    "\0\0\0\0\0\0\0\0\0");
      Play("5eC4BB5C4B5CCEE4A5GG4AA5sE4AA5E",
	    "Bursted\0\0\0my\0\0bub\0ble.\n\0\0"
	    "\0\0\0\0\0\0\0\0\0\0");
      "$$GREEN$$God talking...\n$$FG$$";
      Play("4qG5eDDsCECE4qGG5eF4A5etEEEsC4AA5C",
	    "\0$$RED$$Yep,\0\0you're\0\0\0\0screwed.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0");
      Play("4qG5eDDsCECE4qGG5eF4A5etEEEsC4AA5C",
	    "\0Re\0\0pent,\0\0\0\0dude.$$FG$$\n\0\0\0"
	    "\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
