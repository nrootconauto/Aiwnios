//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 3.360;
  music.stacatto_factor= 0.846;
  try {
    while (!ScanKey) {
      Play("M5/4");
      Play("4qGGG5qtC4BAqGGG5qtC4etBBqtA",
	    "Hymn's\0not\0long.\n\0\0\0\0"
	    "Nor\0this\0song.\n\0\0\0\0\0");
      Play("5qEeCCqDqtC4BAM4/4",
	    "For\0\0\0give!\n\0\0\0\0");
      Play("5qD4eAG5DG4G5DFF4A5C4qGeA5E",
	    "That's\0\0\0all\0\0I\0\0say.\n\0\0\0\0\0\0\0");
      Play("5qD4eAG5DG4G5DFF4A5C4qGeA5E",
	    "This\0\0\0I\0\0\0\0pray.\n\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
