//5 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
    while (!ScanKey) {
      Play("4eG5DGDqG4etAGBqG5GCE",
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0It's \0ko\0sher.\n\0");
      Play("4eG5DGDqG4etAGBqG5GCE",
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0It's \0ko\0sher.\n\0");
      Play("5C4G5sGFGFDCDCetE4G5CGF4A5qDeFC");
      Play("5qC4G5sGFGFDCDCetE4G5CGF4A5qDeFC");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
