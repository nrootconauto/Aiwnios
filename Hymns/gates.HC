//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"mouth distances recommending terrors "
"dignities wondrously balm predicated "
"ungodliness inner resolution secondary "
"blasphemy appeared etext01 object "
"particular placed peacefully advantage "
$ID,-2$
);
$ID,-2$
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.538;
  try {
    task=PopUpViewPrint(
	"$LK,"Nehemiah,3:14",A="BF:Nehemiah,3:14"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,TEXT_COLS-10,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
	Play("M4/4");
	Play("5eDDC4BB5DqDhFeFEqF",
		"As\0signed\0by\0the\0task,\0to\0day.\n\0\0\0\0\0");
	Play("M7/8");
	Play("EhDeRM4/4");
	Play("DDC4BB5DqDhFeFEqF",
		"Schem\0ers\0in\0the\0well\0will\0pay.\n\0\0\0\0\0");
	Play("M7/8");
	Play("EsGAq.GeRM4/4");
	Play("6C5G4qAA5eD4BM5/4",
		"I\0\0got\0the\0shit\0gate.\n\0");
	Play("5qG4AA5eDq.GM4/4");
	Play("eBG4qAA5eD4BM5/4",
		"Don't\0\0you\0\0be\0late.\n\0");
	Play("5qG4AA5eDq.C");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
