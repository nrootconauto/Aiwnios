//5 has words

$TR,"Reward1"$
$ID,2$CallExtStr("JukeReward",""
$TR,"Reward2"$
$ID,2$
"Yokohama Dumbledore's misbehaves syncs "
"confederated reimpose sheet Brandie "
"Bloomfield Bowen percentages semimonthlies "
"surrender's complainant advancing pirouette's "
"Boulez's Talley's Lombard wire "
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
      Play("5e.C4sG5qEe.F4sB5eCCsFG4eBAAetAR5EeEsC4B",
	    "c\0\0out<<\0\"Tribe\0of\0Is\0rael\"<<endl;\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0");
      Play("5e.C4sG5qEe.F4sB5eCCsFG4eBAAetAR5EeEsC4B",
	    "Don't\0\0pout!\0Imbibe\0\0\0oatmeal.\n\0"
	    "\0\0\0\0\0\0\0\0\0\0\0");
      Play("5eGE4qG5sCDCDe.EsGqCsDCDC4e.AsR5eGE",
	    "\0\0\0\0\0\0\0\0\0Flush\0buf\0\0fer!\n\0"
	    "\0\0\0\0\0");
      Play("GE4qG5sCDCDe.EsGqCsDCDC4e.AsR5eGE",
	    "\0\0\0\0\0\0\0\0\0Won't\0surf\0\0er.\n\0"
	    "\0\0\0\0\0");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
