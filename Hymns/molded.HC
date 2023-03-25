//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"bestowed compendiously unlearned babe "
"credence substances home awaited "
"opportunity emerging abated ask "
"central entangling believes mourn "
"unintelligible exquisitely departed cloyedness "
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
	Play("5eDCGGD4G5qEeCC4qG5G4eAB",
		"Pro\0me\0th\0ean\0\0\0mit.\n\0\0\0\0\0\0\0");
	Play("5DCGGD4G5qEeCC4qG5G4eAB",
		"Ed\0en\0wood\0take\0\0\0hit?\n\0\0\0\0\0\0\0");
	Play("qB5E4eA5CsFDFDqFFEetDFF",
		"\0\0\0\0Woo\0\0sy\0\0bout.\n\0\0\0\0\0\0");
	Play("4qB5E4eA5CsFDFDqFFEetDFF",
		"\0\0\0\0Pull\0\0it\0\0out?\n\0\0\0\0\0\0");
	Play("eDCGGD4G5qEeCC4qG5G4eAB",
		"Gold\0en\0\0\0fea\0\0ther.\n\0\0\0\0\0\0\0");
	Play("5DCGGD4G5qEeCC4qG5G4eAB",
		"Goo\0sey\0\0\0te\0\0ther.\n\0\0\0\0\0\0\0");
	Play("qB5E4eA5CsFDFDqFFEetDFF",
		"\0\0\0\0Ther\0\0mal\0\0hang\0glide?\n\0\0\0\0\0");
	Play("4qB5E4eA5CsFDFDqFFEetDFF",
		"\0\0\0\0Too\0\0much\0\0self\0pride.\n\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
