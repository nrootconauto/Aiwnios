//5 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("5eDM6/8");
      Play("5qGeG5qAeB6qD5eBqA6eC");
      Play("5qBeGqGeBqAeFqDeD");
      Play("5qGeGqAeB6qD5eBqA6eC");
      Play("5qBeGGABq.AqGeD");
      Play("6sD5G6D5GeGF6qCeDC5B6D5GF");
      Play("5qD6sD5G6D5GeGF6qCeDC5B6D");
      Play("5GFD");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
