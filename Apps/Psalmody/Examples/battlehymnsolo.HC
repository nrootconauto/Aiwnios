//5 has graphics
U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BLACK<<4+WHITE;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.9;
  try {
    while (!ScanKey) {
      DocClear;
      "$$BG,BLACK$$$$WHITE$$\n\n\n\n\nWait for the end...";
      "\n\n\n\n\n%17c",CH_SPACE;
      Sprite($IB,"<1>",BI=1,BP="::/Demo/Graphics/SpritePut.HC.Z,1"$);
      Play("5e.GsGe.GsFe.EsG6e.CsDe.EsEe.EsDqCe.C5sB");
      '\n';
      Play("5e.AsAe.AsB6e.C5sB6e.C5sAe.GsAe.GsEqGe.GsG");
      '\n';
      Play("5e.GsGe.GsFe.EsG6e.CsDe.EsEe.EsDqCC");
      '\n';
      Play("6DDC5B6h.CqR");
      '\n';
      Play("5(Ge.G");
      '\n';
      Play("5sFe.EsG6e.CsDhEq.CeR");
      '\n';
      Play("5q(Ae.A");
      '\n';
      Play("5sB6e.C5sB6e.C5sAhGq.EeR");
      '\n';
      Play("5q(Ge.G");
      '\n';
      Play("5sFe.EsG6e.CsDhEqCC");
      '\n';
      Play("6DDC5B6h.CqR");
      '\n';
      Play("6etEEGqCsDCDCFCFC5qA6etCD5BB6D5G6FCF");
      '\n';
      Play("6EEGqCsDCDCFCFC5qA6etCD5BB6D5G6FCF");
      '\n';
    }
  } catch
    PutExcept;
  DocClear;
  SettingsPop;
}

Song;
