//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.802;
  try {
    while (!ScanKey) {
      DocClear;
      Play("5qDsCCCC4eBB5CCqDsCCCC4eBB5CC",
	    "I'm\0\0\0\0\0on\0my\0way.\n\0"
	    "\0Got\0a\0\0\0\0bus\0y\0day.\n\0\0");
      Play("4sBBBBeAAsBAGAqAsBBBBeAAsBAGAqA",
	    "Woe,\0\0\0\0I\0for\0got\0\0to\0\0pray.\n\0"
	    "What's\0\0\0\0God\0\0have\0\0to\0\0say?\n\0");
      "$$GREEN$$God talking...\n$$FG$$";
      Play("4AeBG5GGqE4etG5F4B5eFGqFeEC",
	    "$$RED$$Oh,\0\0\0hi-\0a\0you!\n\0What\0ch\0u\0been\0"
	    "\0do\0ing?\n\0\0");
      Play("4qAeBG5GGqE4etG5F4B5eFGqFeEC",
	    "Oh,\0\0\0hi-\0a\0you!\n\0What\0ch\0u\0been\0\0"
	    "do\0ing?$$FG$$\n\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
