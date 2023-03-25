//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("4qBA5eEE4qAeA5ED4BqG5F",
	    "$$RED$$You\0need\0help!\n\0\0\0\0"
	    "Po\0-o\0-o\0r\0dear.\n\0");
      Play("4BA5eEE4qAeA5ED4BqG5F",
	    "You\0need\0help!\n\0\0\0\0"
	    "Po\0-o\0-o\0r\0dear.\n$$FG$$\0");
      Play("6eFGqDetFEDeDE5etA6DEC5G6GqD5G",
	    "Pat\0ron\0ize?\0No,\0\0thank\0you!\n\0\0"
	    "Too\0\0man\0y\0strings\0a\0ttach\0ed.\n\0");
      Play("6eFGqDetFEDeDE5etA6DEC5G6GqD5G",
	    "Pat\0ron\0ize?\0No,\0\0thank\0you!\n\0\0"
	    "Too\0\0man\0y\0strings\0a\0ttach\0ed.\n\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
