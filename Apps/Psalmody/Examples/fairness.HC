//5 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
    task=PopUpViewPrint(
	  "$LK,"BF:Luke,7:42"$\n"
	  "\n"
	  "Prodigal Son\n"
	  "$LK,"BF:Luke,15:21"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,TEXT_COLS-10,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    "$$WW+H,1$$";
    while (!ScanKey) {
      Play("5qGGsE4A5E4AqA5sF4G5F4G5qCeEFqD",
	    "Life,\0life,\0isn't\0\0\0\0fair.\0"
	    "Isn't\0\0\0\0fair.\n\0\0\0\0");
      Play("5GGsE4A5E4AqA5sF4G5F4G5qCeEFqD",
	    "Waaa,\0waaa,\0isn't\0\0\0\0fair.\0"
	    "Isn't\0\0\0\0fair.\n\0\0\0\0");
      Play("5eFCqE4B5D4B5E4eG5EsFEFE",
	    "\0\0I\0\0ne\0ver\0sin.\n\0\0\0\0\0\0\0");
      Play("5eFCqE4B5D4B5E4eG5EsFEFE",
	    "\0\0I\0\0ne\0ver\0win.\n\0\0\0\0\0\0\0");
      "$$GREEN$$(A warning on presumption. 'Who loves more man --"
	    " forgiven big debt or little?'  See prodigal son.)$$FG$$\n";
    }
  } catch
    PutExcept;
  Snd;
}

Song;
