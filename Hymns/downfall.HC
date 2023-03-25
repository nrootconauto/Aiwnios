//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
    while (TRUE) {
      Play("et4GG5Gq4G5qCq4G5sF4A5F4AqG5etGD4AqA",
	    "Don't\0\0need\0\0help.\0\0Got \0\0it\0\0all\0"
	    "figured\0\0\0out.\n\0");
      Play("et4GG5Gq4G5qCq4G5sF4A5F4AqG5etGD4AqA",
	    "Don't\0\0need\0\0help.\0\0Got \0\0it\0\0all\0"
	    "figured\0\0\0out.\n\0");
      Play("5etCCCq4G5qDeDFetCF4GqB5eGF4sA5G4A5G");
      Play("5etCCCq4G5qDeDFetCF4GqB5eGF4sA5G4A5G");
    }
  } catch
    PutExcept;
}

Song;
