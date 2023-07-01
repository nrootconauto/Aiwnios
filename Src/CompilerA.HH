//Intermediate compiler codes
	
class CIntermediateStruct
{
  U8 arg_cnt,res_cnt,type;
  Bool fpop,not_const,pad[3];
  U8 *name;
};


class COptReg
{
  I64	offset;
  CMemberLst *m;
};

extern CHashClass *PrsClassNew();
//Added by Nroot
extern U0 RPNDel(CRPN *);
extern U0 RPNAdd(CCmpCtrl *cc,CRPN *rpn,Bool ad_to_prs_stk=TRUE);
extern CRPN *RPNAddNew(CCmpCtrl *cc,I64 type,I64 data=0,CHashClass *cls=NULL,Bool add_to_prs_stk=TRUE);
#define PEF_END_EXPR 0x1 //Same as TRUE
#define PEF_PAREN 0x2
#define PEF_ARRAY 0x4
#define PEF_ADDR_OF 0x8
#define PEF_FUN_BEFORE 0x10
#define PEF_NO_COMMA 0x20
extern Bool PrsExpression(CCmpCtrl *cc,U8 **ul=NULL,I64 flags=0,U8 *prs_stk=NULL);
extern U8 *COCCompile(CCmpCtrl *cc,I64 *_code_size,CDbgInfo **_dbg,I64 *_type);
extern CCodeMisc *CodeMiscHashNew(CCmpCtrl *cc,CHash* h);
