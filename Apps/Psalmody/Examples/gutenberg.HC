//5 has graphics




$SP,"<1>",BI=1$

$SP,"<2>",BI=2$

F64 t0=Beat;

U0 DrawIt(CTask *,CDC *dc)
{
  F64 �,dt=Beat-t0;
  if (dt>12) dt=12;
  �=0.2*FullTri(dt,4);
  Sprite3ZB(dc,140+30*Sin(dt),40,0,$IB,"<1>",BI=1$,�);
  Sprite3(dc,170,15,0,$IB,"<2>",BI=2$);
}

U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  Fs->draw_it=&DrawIt;
  MusicSettingsRst;
  music.tempo= 2.500;
  music.stacatto_factor= 0.900;
  try {
    task=PopUpViewPrint("$LK,"Revelation,22:18",A="BF:Revelation,22:18"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,TEXT_COLS-10,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    "$$WW+H,1$$";
    while (!ScanKey) {
      t0=Beat;
      Play("4qB5RE4sG5C4G5CeCFqEeG4AsRGRG",
	    "The\0\0quill\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("4qB5RE4sG5C4G5CeCFqEeG4AsRGRG",
	    "is\0\0still\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
      Play("5qE4RetA5CGqF4G5etD4G5F4B5F4G5DRF",
	    "\0\0God's\0\0\0word\0for\0all\0\0"
	    "his\0\0hap\0\0py\0\0peo\0");
      Play("5qE4RetA5CGqF4G5etD4G5F4B5F4G5DRF",
	    "ple.\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
      "$$GREEN$$Gutenberg invented the printing"
	    " press and made Bibles.Until then,"
	    "I wonder how many man-hours for one word in the Bible!\n$$FG$$";
    }
  } catch
    PutExcept;
  Snd;
}

Song;
        2     ��������   "   �������  ��     ������   ��     ������   ��     ������   �     ����� �  �     �����  ��      ����  � �     ����  � ��     ���� �� ��     ��� �� � ��     ��� � � ���     �� � � ���     �� � � ���     �� � � ���     �  �  ����     �  � � ����     � � � ����     � � � ����     � �  �����      �   �����        � �����       � � �����      �   ������      �   ������     � �  ������     �    ������     �  ��������     �� ��������     �� ��������     �� ��������     �� ��������     �� ��������     �� ��������     �� ��������             b      ��������   
   �    �  �    �    �                                                          