//5 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    task=PopUpViewPrint("$LK,"Luke,13:25",A="BF:Luke,13:25"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,TEXT_COLS-10,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("5qGeEFqE4B5CG4qtB5DA",
	    "God,\0let\0me\0in!\n\0\0\0\0\0\0\0");
      Play("5qGeEFqE4B5CG4qtB5DF",
	    "God,\0did\0I\0sin?\n\0\0\0\0\0\0\0");
      "$$GREEN$$God talking...\n$$FG$$";
      Play("5qE4eG5FqDE4eB5EqCeCD4B5C",
	    "$$RED$$Who's\0\0\0\0there?\n\0\0\0\0\0\0\0\0");
      Play("4qBeAsBA5qE4eG5FqDE4eB5EqC",
	    "\0\0\0\0Who's\0\0\0\0where?$$FG$$\n\0\0\0\0");
      Play("5eCD4B5C4qBeAB");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
