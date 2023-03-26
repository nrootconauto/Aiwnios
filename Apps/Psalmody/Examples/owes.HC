//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"stops sedge Lucile's garter's "
"resister agonized unseasonable crocodiles "
"bindery's celluloid's Martinique resourced "
"unsubscribe bracing Cathleen's liveliness's "
"heartlessness shirk Occidental Eurasian "
$ID,-2$
);
$ID,-2$
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("4qA5DeDEqFetE4BA5sC4B5C4B5eGFetE4A5D",
	    "The\0owl\0in\0the\0night...\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("4qA5DeDEqFetE4BA5sC4B5C4B5eGFetE4A5D",
	    "Its\0eyes\0o\0pen\0tight...\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("qDGetG4A5C4sB5E4B5E4ABAB5etDFEDC4B5qG",
	    "Ain't\0play\0in'.\n\0\0\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("DGetG4A5C4sB5E4B5E4ABAB5etDFEDC4B5qG",
	    "Just\0say\0in'.\n\0\0\0\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
