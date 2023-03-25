//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
	Play("4eA5CD4A5DE4A5DC4BqAAeA5C",
		"I\0am\0on\0a\0jour\0ney.\n\0\0\0\0\0\0\0\0\0");
	Play("D4A5DE4A5DC4BqAA5eGGGF",
		"\0\0\0\0\0\0\0\0\0\0Pack\0your\0bags\n\0\0");
	Play("GEDEC4BAAAB5CCG4BA5G");
	Play("GC4B5GF4BB5DD",
		"\0\0\0\0\0\0\0The\0end.\n\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
