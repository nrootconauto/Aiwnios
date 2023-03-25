//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("5sC#C#D#D#C#C#CCC#C#D#D#C#C#CCEEDDC#C#DDEEDDC#C#DD",
	    "Still\0\0\0\0there.\n\0\0\0\0"
	    "Still\0\0\0\0there.\n\0\0\0\0"
	    "Not\0\0\0\0fair.\n\0\0\0\0"
	    "Not\0\0\0\0fair.\n\0\0\0\0");
      Play("5C#C#D#D#C#C#CCC#C#D#D#C#C#CCEEDDC#C#DDEEDDC#C#DD",
	    "Un\0\0\0\0aware!\n\0\0\0\0"
	    "Un\0\0\0\0aware!\n\0\0\0\0"
	    "Every\0\0\0\0where!\n\0\0\0\0"
	    "Every\0\0\0\0where!\n\0\0\0\0");
      Play("4eB5DetFG4A5eFE4B5EetEFEqCeFED4A",
	    "$$RED$$Running\0\0\0a\0way\0\0\0\0\0\0"
	    "from\0the\0Lord\0won't\0\0work.\n\0\0");
      Play("4B5DetFG4A5eFE4B5EetEFEqCeFED4A",
	    "Running\0\0\0a\0way\0\0\0\0\0\0"
	    "from\0the\0Lord\0won't\0\0work.$$FG$$\n\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
