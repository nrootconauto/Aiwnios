//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"Guy's uncommunicative Texan's artless "
"egg's dung breeding's humblings "
"oncology's wretchedness's forecastles decor's "
"referenced coauthor twists desegregates "
"sublimate quilt spermicide's Uruguayan "
$ID,-2$
);
$ID,-2$
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.549;
  try {
    while (!ScanKey) {
      Play("5e.DsCeE4B5D4BqB5eFFe.EFeFqD",
	    "Jade\0\0\0\0mon\0key\0\0laugh\0ing\0"
	    "\0at\0the\0beach.\n\0");
      Play("e.DsCeE4B5D4BqB5sFFFFe.EFeFqD",
	    "Wade\0\0dunk\0ey,\0\0\0\0$$RED$$\"Fer\0\0al\0"
	    "\0\0I\0can't\0teach.\"$$FG$$\n\0");
      Play("F4B5sFe.G4eA5C4qA5F4GeGG",
	    "I\0got\0mo\0jo!\n\0\0\0\0\0\0\0\0");
      Play("5qF4B5e.FsG4eA5C4qA5F4Ge.GsG",
	    "You\0an\0ass\0\0hole.\n\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
