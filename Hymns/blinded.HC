//5 has words
F64 t0=Beat;

U0 DrawIt(CTask *task,CDC *dc)
{
  static I64 i=0;
  F64 dt=Beat-t0;
  if (dt>16.0 && i++&1) {
    dc->color=WHITE;
    GrRect(dc,0,0,task->pix_width,task->pix_height);
  }
}

U0 Song()
{
  CTask *task;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  try {
    task=PopUpViewPrint("$LK,"BF:Acts,9:3"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,TEXT_COLS-10,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      t0=Beat;
      Play("5qE4eA5C4AG5qCEC4G5E",
	    "Now... \0\0\0\0\0I \0see \0the \0light.\n\0\0");
      Play("5E4eA5C4AG5qCEC4G5E",
	    "Now... \0\0\0\0\0I \0see \0the \0light.\n\0\0");
      Play("5GCG4B5EeDFC4A5qD",
	    "It's \0\0quite \0\0bright.\n\0\0\0\0\0\0");
      Play("5GCG4B5EeDFC4A5qD",
	    "It's \0\0quite \0\0bright.\n\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
