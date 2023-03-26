//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"dykes Sprint bequest calcine "
"glaucoma bo's'n buttressed cheesecake "
"triumphing attend eucalyptus's Novocain's "
"incongruously Ionic's Lestrade Yakut's "
"Pennsylvanian's insentient Mithridates's mottle "
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
      Play("5etDDDeCDECqD4eB5DFEDCqD",
	    "Road\0to\0Hell?\0\0\0Side\0walk's\0full.\n\0"
	    "\0\0\0\0\0\0\0");
      Play("etDDDeCDECqD4eB5DFEDCqD",
	    "Get\0a\0Pell?\0\0\0Grav\0ity's\0pull.\n\0"
	    "\0\0\0\0\0\0\0");
      Play("sF4G5eE4sB5G4eA5C4A5sEGeEetECF4eAAqGA",
	    "Mean\0\0time?\0\0\0\0Fail\0ure.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0");
      Play("5sF4G5eE4sB5G4eA5C4A5sEGeEetECF4eAAqGA",
	    "Thine\0\0dime?\0\0\0\0Mail\0er.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
