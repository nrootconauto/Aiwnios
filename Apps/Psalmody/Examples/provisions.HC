//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"Neanderthal Natalie epilog's accordance's "
"discontent feeds Raoul abduction's "
"epicureans securing chirp McDonnell's "
"Scipio Amur's charbroiling starves "
"bottle's SALT's Demeter's Bloom "
$ID,-2$
);
$ID,-2$
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("5qDetF4GG5qC4sGBGB5qFsGDGDqCF",
	    "Meet\0me...\0\0\0\0\0\0\0\0"
	    "in\0the\0\0\0\0air.\n\0\0");
      Play("DetF4GG5qC4sGBGB5qFsGDGDqCF",
	    "Do\0I...\0\0\0\0\0\0\0\0"
	    "trust\0or\0\0\0\0dare.\n\0\0");
      Play("4B5eG4GGGAAqG5etFDDeC4G5qC",
	    "\0\0\0\0\0\0\0\0Lov\0ing\0"
	    "\0\0care.\n\0\0");
      Play("4B5eG4GGGAAqG5etFDDeC4G5qC",
	    "\0\0\0\0\0\0\0\0Laid\0"
	    "\0\0\0\0bare.\n\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
