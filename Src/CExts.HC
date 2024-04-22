extern U8 *Doc2PlainText(CDoc *doc,CDocEntry *doc_e);
extern U0 DocBinsValidate(CDoc *doc);
extern U0 DocDel(CDoc *doc);
extern U0 DocLoad(CDoc *doc,U8 *src2,I64 size);
extern CDoc *DocNew(U8 *filename=NULL,CTask *task=NULL);
extern CDocEntry *DocPrint(CDoc *doc=NULL,U8 *fmt,...);
extern CDocEntry *DocPutLine(CDoc *doc,CDocEntry *doc_e);
extern I64 DocSize(CDoc *doc);
extern Bool DocWrite(CDoc *doc,Bool prompt=FALSE);
extern U8 *CmdLinePmt();
extern I64 HashEntrySize2(CHashSrcSym *tmph);
extern Bool IsLexExpression2Bin(CCmpCtrl *cc,U8 **_machine_code);
extern I64 Lex(CCmpCtrl *cc);
extern I64 LexExpression(CCmpCtrl *cc);
extern I64 LexGetChar(CCmpCtrl *cc);
extern CCodeMisc *OptLabelFwd(CCodeMisc *lb);
extern CIntermediateCode *OptPass012(CCmpCtrl *cc);
extern U0 OptPass3(CCmpCtrl *cc,COptReg *reg_offsets);
extern U0 OptPass4(CCmpCtrl *cc,COptReg *reg_offsets,I64 *_type);
extern U0 OptPass5(CCmpCtrl *cc);
extern U0 OptPass6(CCmpCtrl *cc);
extern I64 OptPass789A(
	CCmpCtrl *cc,COptReg *reg_offsets,U8 *buf,CDbgInfo **_dbg);
extern CHashClass *PrsClass(
	CCmpCtrl *cc,I64 keyword,I64 fsp_flags,Bool is_extern);
extern CHashFun *PrsFunJoin(
	CCmpCtrl *cc,CHashClass *tmp_return,U8 *name,I64 fsp_flags);
extern I64 PrsKeyWord(CCmpCtrl *cc);
extern Bool PrsStmt(CCmpCtrl *cc,I64 try_cnt=0,
	CCodeMisc *lb_break=NULL,I64 cmp_flags=CMPF_PRS_SEMICOLON);
extern U0 PrsStreamBlk(CCmpCtrl *cc);
extern CHashClass *PrsType(CCmpCtrl *cc,CHashClass **_tmpc1,
	I64 *_mode,CMemberLst *tmpm,U8 **_ident,CHashFun **_fun_ptr,
	CHashExport **_tmpex,CArrayDim *tmpad,I64 fsp_flags);
extern I64 PrsUnaryModifier(CCmpCtrl *cc,CPrsStk *ps,
	CMemberLst **_local_var,CArrayDim **_tmpad,I64 *unary_post_prec);
extern I64 PrsUnaryTerm(
	CCmpCtrl *cc,CPrsStk *ps,CMemberLst **_local_var,CArrayDim **_tmpad,
	I64 *max_prec,I64 *unary_pre_prec,I64 *paren_prec);
extern U0 PrsVarInit2(CCmpCtrl *cc,U8 **_dst,CHashClass *tmpc,
	CArrayDim *tmpad,U8 *data_addr_rip,U8 **_base,Bool data_heap,I64 pass,CHash * to);
extern U0 FillInShortCalls(CCmpCtrl *cc);
extern CHashClass *AssignRawTypeToNode(CRPN *r);
