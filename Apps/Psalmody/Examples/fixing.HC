//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"jellyfish's Gentry's motorist's gleaming "
"Pekinese OSHA's slump's arcs "
"tee's Duracell's reinforcing Motorola "
"armament's hazed Therese bonuses "
"skater editable disregarded obfuscates "
$ID,-2$
);
$ID,-2$
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.758;
  try {
    while (!ScanKey) {
      Play("5sDD4eB5C4sAAeBGAF5sCCeDsFFeEsFFEEeDE",
	    "Been\0\0real\0busy.\n\0\0\0"
	    "Feel\0real\0dizzy.\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("sDD4eB5C4sAAeBGAF5sCCeDsFFeEsFFEEeDE",
	    "Quan\0\0tum\0spinning?\n\0\0\0"
	    "Find\0ing,\0binning.\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("sG4B5G4BetG5EGqFED4GeBA5DG",
	    "Fixed\0\0point\0\0\0iter\0a\0tion?\n\0"
	    "\0\0\0\0\0\0\0");
      Play("sG4B5G4BetG5EGqFED4GeBA5DG",
	    "Re\0lax\0ing\0\0\0va\0ca\0tion.\n\0"
	    "\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
