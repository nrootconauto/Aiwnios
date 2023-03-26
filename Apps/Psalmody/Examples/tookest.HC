//5 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    task=PopUpViewPrint("$LK,"Luke,17:36",A="BF:Luke,17:36"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,TEXT_COLS-10,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("4qAB5eC4q.A5qCDeE4q.A",
	    "Pile\0of\0clo\0ths.\n\0Pile\0of\0clo\0ths.\n\0");
      Play("5qDeDq.C4qBABAR",
	    "Has\0the\0rap\0ture\0come\0and\0gone?\n\0\0");
      Play("5EeD4sG5D4eBG5qFCeCsC4G5etG4BG5qE",
	    "Dear\0God,\0\0\0am\0I\0here?\n\0"
	    "\0\0\0\0\0\0\0\0");
      Play("5EeG4sG5EeC4AqG5CeCsC4G5EE4AA5eEE",
	    "Dear\0God,\0\0\0am\0I\0here?\n\0"
	    "\0\0\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
