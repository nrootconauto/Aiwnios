//0 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
	Play("4eG5DFG4BGsG5C4G5C4BBBB5qCDhC");
	Play("4eG5DFG4BGsG5C4G5C4BBBB5qCDhC");
	Play("qFFetECEqF4BetAB5G4qAetBB5D");
	Play("qFFetECEqF4BetAB5G4qAetBB5D");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
