extern U0 CInit(Bool first);
extern I64 ExeFile(U8 *name,I64 ccf_flags=0);
extern I64 ExeFile2(U8 *name,I64 ccf_flags=0);
extern I64 ExePrint(U8 *fmt,...);
extern I64 ExePrint2(U8 *fmt,...);
extern I64 ExePutS(U8 *buf,U8 *filename=NULL,I64 ccf_flags=0,
	CLexHashTableContext *htc=NULL);
extern I64 ExePutS2(U8 *buf,U8 *filename=NULL,I64 ccf_flags=0);
extern I64 RunFile(U8 *name,I64 ccf_flags=0,...);
extern I64 RunFile2(U8 *name,I64 ccf_flags=0,...);
extern CCmpGlbls cmp;

extern Bool Echo(Bool val);
extern Bool GetOption(I64 num);
extern I64 PassTrace(I64 i=0b1001111101);
extern U0 StreamDir();
extern I64 StreamExePrint(U8 *fmt,...);
extern U0 StreamPrint(U8 *fmt,...);
extern Bool Trace(Bool val=ON);

extern U0 ClassMemberLstDel(CHashClass *c);
extern U0 CmpCtrlDel(CCmpCtrl *cc);
extern CCmpCtrl *CmpCtrlNew(U8 *buf=NULL,I64 flags=0,U8 *filename=NULL);
extern I64 CmpCtrlSize(CCmpCtrl *cc);
extern I64 IsLexExpression2Bin(
	CCmpCtrl *cc,U8 **machine_code); //FALSE=no err
extern I64 Lex(CCmpCtrl *cc);
extern U0 LexAttachDoc(CCmpCtrl *cc,CLexFile *tmpf=NULL,
	CDoc *doc=NULL,U8 *abs_filename=NULL,CDocEntry *doc_e=NULL,I64 col=0);
extern CD2I32 *LexD2I32(CCmpCtrl *cc,CD2I32 *p);
extern CD3I32 *LexD3I32(CCmpCtrl *cc,CD3I32 *p);
extern U0 LexExcept(CCmpCtrl *cc,U8 *str=NULL);
extern I64 LexExpression(CCmpCtrl *cc);
extern U8 *LexExpression2Bin(CCmpCtrl *cc,I64 *_type=NULL);
extern F64 LexExpressionF64(CCmpCtrl *cc);
extern I64 LexExpressionI64(CCmpCtrl *cc);
extern U8 *LexExtStr(CCmpCtrl *cc,I64 *_size=NULL,Bool lex_next=TRUE);
extern U8 *LexFirstRem(CCmpCtrl *cc,U8 *marker,I64 _len=NULL);
extern I64 LexGetChar(CCmpCtrl *cc);
extern U0 LexPopNoRestore(CCmpCtrl *cc);
extern U0 LexPopRestore(CCmpCtrl *cc);
extern U0 LexPush(CCmpCtrl *cc);
extern U0 LexPutLine(CCmpCtrl *cc,U8 *start);
extern U0 LexPutPos(CCmpCtrl *cc);
extern U0 LexPutToken(CCmpCtrl *cc);
extern U0 LexSkipEol(CCmpCtrl *cc);
extern U8 *LexStmt2Bin(
	CCmpCtrl *cc,I64 *_type,I64 cmp_flags=CMPF_PRS_SEMICOLON);
extern U0 LexWarn(CCmpCtrl *cc,U8 *str=NULL);
extern CMemberLst *MemberClassBaseFind(
	CHashClass *needle_class,CHashClass *haystack_class);
extern CMemberLst *MemberFind(U8 *needle_str,CHashClass *haystack_class);
extern U0 MemberLstDel(CMemberLst *tmpm);
extern I64 MemberMetaData(U8 *st,CMemberLst *ml);
extern CMemberLstMeta *MemberMetaFind(U8 *st,CMemberLst *ml);
extern CHashClass *OptClassFwd(CHashClass *tmpc);
extern I64 PrsKeyWord(CCmpCtrl *cc);

extern I64 Cmp(U8 *filename,
	U8 *map_name=NULL,U8 *out_name=NULL,U8 mapfile_drv_let=0);

extern U8 *U(U8 *rip,I64 cnt=20,I64 seg_size=64);
extern U0 Ui(U8 *buf,U8 **_rip,I64 seg_size=64,
	I64 *_jmp_dst=NULL,Bool just_ins=FALSE);
extern CUAsmGlbls uasm;

extern I64 HashEntrySize(CHashSrcSym *tmph);
extern I64 HashEntrySize2(CHashSrcSym *tmph);
extern I64 HashTableSize2(CHashTable *table);
