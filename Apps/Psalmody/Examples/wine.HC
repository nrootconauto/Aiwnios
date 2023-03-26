//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"kingfisher's Loyola caters powwows "
"UK's proclivity's absolving unaccepted "
"snowfalls Noels charismatics Descartes "
"Mara's hoodlums Sue cartridge "
"Acosta Lancelot gaff's souping "
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
      Play("5sFFFFeCGqEetD4B5CsD4G5D4GqBB5D",
	    "Beast\0\0\0\0\0\0throws\n\0"
	    "\0\0\0\0\0\0\0\0a\0dart.\n\0");
      Play("sFFFFeCGqEetD4B5CsD4G5D4GqBB5D",
	    "Hands\0\0\0\0\0\0now\n\0"
	    "\0\0\0\0\0\0\0\0a\0part.\n\0");
      Play("A6sDCDC5eA6FqDEeDFsF5B6F5B6F5B6F5B",
	    "Shattered\n\0\0\0a\0\0jar.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("qA6sDCDC5eA6FqDEeDFsF5B6F5B6F5B6F5B",
	    "Shards\n\0\0\0a\0\0card.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
