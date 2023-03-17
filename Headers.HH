extern U8 *GodSongStr();
extern U0 GodSong();
extern U8 *TimeStampCB(CDoc *,CDocEntry *,CTask *mem_task);
extern U8 *KbdMsTimeCB(CDoc *,CDocEntry *,CTask *mem_task);
extern I64 PopUpTimerOk(U8 *header=NULL,U8 *footer=NULL);
extern I64 GodPick(U8 *msg=NULL);
extern U0 GodBitsIns(I64 num_bits,I64 n);
extern U0 GodHexIns(U8 *st);
extern I64 GodBits(I64 num_bits,U8 *msg=NULL);
extern I64 GodInit(U8 *files_find_mask,U8 *fu_flags=NULL);
extern U8 *GodWordStr(I64 bits=17);
extern U0 GodWord(I64 bits=17);
extern U0 GodBiblePassage(I64 num_lines=20);
