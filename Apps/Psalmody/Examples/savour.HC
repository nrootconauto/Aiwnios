//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("5qFCeE4A5E4A5qDeCE4qBA",
	    "This\0\0\0\0\0\0sand\0wich\0is\0bland.\n\0\0");
      Play("5eCCqDeDDqE4eA5ECECE4qA",
	    "God\0\0damn,\0my\0\0tastes\0\0\0\0"
	    "sure\0\0are\0grand.\n\0");
      Play("M6/8");
      Play("5FE4BGsA5GeCqE",
	    "Ho\0hum.\0\0\0\0How\0\0rude!\n\0");
      Play("5FE4BGsA5GeCqE",
	    "That\0bum\0\0\0\0likes\0his\0food.\n\0");
      Play("4GsA5GeDqGFE4B",
	    "\0\0Likes\0his\0food!\n\0\0\0\0");
      Play("4GsAA5eEqD4G5sD4B5D4BeGG");
      Play("4A5sED4qAeAsB5GE4B5E4BqG5sD4B5D4B");
      Play("4eGGA5sED4qAeAsB5GE4B5E4B");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
