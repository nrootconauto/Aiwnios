//5 has graphics





$SP,"<1>",BI=1$

F64 t0=Beat;
#define CENTER_X	200

U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0;
  dc->flags|=DCF_SYMMETRY;
  DCSymmetrySet(dc,CENTER_X,0,CENTER_X,1);
  Sprite3(dc,CENTER_X-40,75+dt,0,$IB,"<1>",BI=1$);
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;

  MusicSettingsRst;
  music.tempo=2.5;
  try {
    while (!ScanKey) {
      t0=Beat;
      Play("5sFGFGetDCFqD4G5DGDetDF4B",
	    "God,\0\0\0\0please\0\0\0help\0me.\n\0\0\0\0\0\0\0");
      Play("5sFGFGetDCFqD4G5DGDetDF4B",
	    "God,\0\0\0\0please\0\0\0help\0me.\n\0\0\0\0\0\0\0");
      Play("4qA5C4A5DetF4GBsG5F4G5FetCDFsDFDF",
	    "Thy\0will\0be\0done.\n\0\0\0\0\0\0\0"
	    "\0\0\0\0\0\0\0\0");
      Play("4qA5C4A5DetF4GBsG5F4G5FetCDFsDFDF",
	    "Thy\0will\0be\0done.\n\0\0\0\0\0\0\0"
	    "\0\0\0\0\0\0\0\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
        2     ��������(   4   �������������������������������������������������������  ��������������������������������������  ������  ����������������������������  ���   ����������������������������  ����  ���������������������������  ����  ����������������������������  ���  ���  �����������������������  ���  ���  ����������������������  ���  ���  ��  �����������������  ��   ����  ��  �����������������  ��  ���  �  �����������������  ��  ���     ����������������  ��  ��   �����������������  ��  ��   �������   �������  ��  ��    ������  �������  �  ��   ������  ������   �  �    �������  ������  �  �    �������  �����  �      ��������  �����  �      ��������  �����  �   �  ���������   ���       ����������  ��  �  ����������  �     ����������     ������������   ������������   ������������   �������������   ��������������    ����������������   �����������������     �������������������    ���������������������    ����������������������    ����������������������    �����������������������   ������������������������   �������������������������  �������������������������  ��������������������������  ��������������������������  ��������������������������  ��������������������������  ��������������������������  ��������������������������  ��������������������������      �������������������������� ����     ��������������������������� �������     ��������������������������������������� ����������������������������������������������������������������������������������������������� 