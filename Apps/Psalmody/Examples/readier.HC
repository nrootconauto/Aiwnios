//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.646;
  music.stacatto_factor= 0.989;
  try {
    while (!ScanKey) {
      Play("4q.G5sCC4qAeBBqG5C4eAAqB");
      Play("5q.CsCCeCCCC4B5DDDDD4qB",
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0Go!\n\0");
      Play("4e.AsA5DC4eA5etDDD4eA5sEG4eB5FsFDFDeEsF4A5CGeC",
	    "\0\0\0\0\0\0\0\0\0\0\0\0"
	    "\0\0\0\0\0\0\0\0\0\0Go!\n\0");
      Play("4e.AsA5DC4eA5etDDD4eA5sEG4eB5FsFDFDeEsF4A5CGeC");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
