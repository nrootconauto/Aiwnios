//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.714;
  try {
    while (!ScanKey) {
      Play("4sBBBB5E4B5D4B5eE4B5qD4eBB5EDE4B5qD",
	    "\0\0\0\0Had\0\0a\0\0pass\0ing\0"
	    "thought.\n\0\0\0Not\0a\0thought\0I\0ought.\n\0");
      Play("5sFEFEeFED4B5qDeEFE4BAB5qD",
	    "Can't\0\0\0\0clear\0my\0head\0of\0"
	    "thought.\n\0In\0this\0trap\0my\0mind\0is\0caught.\n\0");
      Play("5eDGqE4eBAAB5qF4eB5DqE4A");
      Play("5eDGqE4eBAAB5qF4eB5DqE4A");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
