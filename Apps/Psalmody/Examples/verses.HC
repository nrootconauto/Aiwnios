//5 special
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
    task=PopUpViewPrint(
	  "$$WW,1$$This shows a random passage based on "
	  "the time at the moment this song was run."
	  "Talk to God as an offering, then run press $FG,2$<F7>$FG$ if you "
	  "want God to talk some more.The $LK+PU,"Holy Spirit",A="FI:::/Adam/God/HSNotes.DD"$ can "
	  "puppet you.\n");
    TaskWait(task);
    WinHorz(Fs->win_left,TEXT_COLS-10,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    "$$WW+H,1$$";
    DocRecalc(DocPut(task)); //Reformat word-wrap with new window size.
    while (!ScanKey) {
      FifoU8Flush(god.fifo);
      GodBitsIns(GOD_GOOD_BITS,KbdMsEvtTime>>GOD_BAD_BITS);
      GodBiblePassage(5);
      Play("4qAeAB5C4B5qC4etABB5qC4etAA5REE4G");
      Play("4qAeAB5C4B5qC4etABB5qC4etAA5REE4G");
      Play("5eDRRRqGReRRGGGGqE");
      Play("5eDRRRqGReRRGGGGqE");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
