//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"bookmaker in addressees anointment "
"complaint dictations snippiest puller "
"pot's Kampala's bleached stealth's "
"anticked carpel's Uranus's Olive's "
"countersinks sheikh stripes fidgety "
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
      Play("6etEEE5qB6etDDqtD5qB6etCCqtC5qBhB",
	    "Go\0to\0the\0well.\n\0This\0is\0a\0hell.\n\0"
	    "Shad\0ow\0of\0bell.\n\0\0");
      Play("6etEEE5qB6etDDqtD5qB6etCCqtC5qBhB",
	    "I\0know\0it\0well.\n\0I\0can\0do,\0fell.\n\0"
	    "Al\0read\0y\0tell.\n\0\0");
      Play("etAB6CqD5etB6CCqDetDC5BqAhA",
	    "On\0the\0way\0up\n\0to\0fill\0my\0cup\n\0"
	    "and\0go\0ing\0down.\n\0\0");
      Play("etAB6CqD5etB6CCqDetDC5BqAhA",
	    "Here\0comes\0a\0pup\n\0I\0give\0a\0hup\n\0"
	    "and\0get\0a\0crown.\n\0\0");
      Play("qA6etC5A6EsGCGC5etB6C5GA6D5AqA6GetD5A6D",
	    "Who's\0going\0\0to\0sing?\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("5qA6etC5A6EsGCGC5etB6C5GA6D5AqA6GetD5A6D",
	    "Who's\0going\0\0to\0swing?\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
