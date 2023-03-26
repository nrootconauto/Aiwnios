//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"supplications Berlin's caressed contemporary's "
"Prudence alternatives chapels Bacchanalia's "
"economics's countersink's samovar autopsied "
"pile's psalmist's chucked lapels "
"long's anchorages protestation's smokiness "
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
      Play("5eCCDDqEeCsDDqCE4BG",
	    "One\0\0round\0\0left...\n\0\0\0\0\0\0\0\0");
      Play("5eCsCCeDDqEeCsDDqCE4BG",
	    "One\0\0\0be\0\0reft...\n\0\0\0\0\0\0\0\0");
      Play("5eDGsEFEFe.E4sA5eE4sBG5eDE4G5GFG4qG",
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	    "Feel\0luck\0y.\n\0\0");
      Play("5eDGsEFEFe.E4sA5eE4sBG5eDE4G5GFG4qG",
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	    "Squeek\0duck\0y.\n\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
