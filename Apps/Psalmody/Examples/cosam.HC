//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 3.966;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("4qAB5eCC4qABAAR",
	    "What\0makes\0You\0\0\0think\0You're\0God?\n\0\0");
      Play("4AB5eCC4qABAAR",
	    "What\0makes\0You\0\0\0think\0You're\0God?\n\0\0");
      Play("5DeCCCCqCDCCR",
	    "Do\0You\0\0think\0\0\0I'll\0app\0laud?\n\0\0");
      Play("4AB5eCC4qABAAR",
	    "What\0makes\0You\0\0\0think\0You're\0God?\n\0\0");
      Play("5GDGeG4AG5FqFeD4RqB",
	    "$$RED$$Be\0-e-\0cause\0\0\0\0I\0\0a\0\0m.\n\0");
      Play("5GDGeG4AG5FqFeD4RqB",
	    "Are\0\0you\0\0\0\0a\0\0fan?\n$$FG$$\0\0\0");
      "$$GREEN$$(If God talks, how can you be sure it's not Him?\n"
	    "You can't, so harden not your heart.)\n$$FG$$";
    }
  } catch
    PutExcept;
  Snd;
}

Song;
