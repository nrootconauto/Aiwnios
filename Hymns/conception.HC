//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"eggbeater happened grossly Africa "
"Lassen's contraltos shoreline defames "
"Wuhan Plato's car's inheritor's "
"parkways tendons pipit's given "
"triangulation's proselytized wiliness checkmated "
$ID,-2$
);
$ID,-2$
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.824;
  try {
    while (!ScanKey) {
      Play("4etG5FDDCDqFsG4G5eFEFqE4eBqA5eE",
	    "Puff\0\0\0of\0\0\0smoke\n\0"
	    "\0\0\0\0\0\0\0\0\0");
      Play("4etG5FDDCDqFsG4G5eFEFqE4eBqAeB",
	    "Thin\0\0\0of\0\0\0air\n\0"
	    "\0\0\0\0\0\0\0\0\0");
      Play("6EFqF5BB6C5A6FD",
	    "Con\0\0dens\0ate\n\0\0\0\0\0\0");
      Play("eEFqF5BeBB6qC5A6FD",
	    "Can't\0\0re\0late\n\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
