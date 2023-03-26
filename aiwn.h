#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <SDL2/SDL.h>
#define ERR 0x7fFFffFFffFFffFF
#define A_FREE __AIWNIOS_Free
#define A_MALLOC(sz,task) __AIWNIOS_MAlloc(sz,task)
#define A_CALLOC(sz,task) __AIWNIOS_CAlloc(sz,task)
#define A_STRDUP(str,task) __AIWNIOS_StrDup(str,task)
struct CHash;
struct CHashTable;
struct CHashFun;
struct CTask;
void *__AIWNIOS_CAlloc(int64_t cnt,void *t);
void *__AIWNIOS_MAlloc(int64_t cnt,void *t);
void __AIWNIOS_Free(void *ptr);
char *__AIWNIOS_StrDup(char *str,void *t);
#define ARM_ERR -1
#define ARM_ERR_INV_OFF -2
#define ARM_ERR_INV_REG -3

#define LSL 0
#define LSR 1
#define ASR 2
#define ROR 3
#define MSL 4
#define NONE 5

#define EXT_NONE 0xffff
#define UXTB_32 0b000
#define UXTH_32 0b001
#define EXT_LSL_32 0b010
#define UXTW_32 0b010
#define UXTX_32 0b011
#define SXTB_32 0b100
#define SXTH_32 0b101
#define SXTW_32 0b110
#define SXTX_32 0b111

#define UXTB_64 0b000
#define UXTH_64 0b001
#define EXT_LSL_64 0b010
#define UXTW_64 0b010
#define UXTX_64 0b011
#define SXTB_64 0b100
#define SXTH_64 0b101
#define SXTW_64 0b110
#define SXTX_64 0b111

#define ARM_EQ 0x0
#define ARM_NE 0x1
#define ARM_CS 0x2
#define ARM_HS 0x2
#define ARM_CC 0x3
#define ARM_LO 0x3
#define ARM_MI 0x4
#define ARM_PL 0x5
#define ARM_VS 0x6
#define ARM_VC 0x7
#define ARM_HI 0x8
#define ARM_LS 0x9
#define ARM_GE 0xa
#define ARM_LT 0xb
#define ARM_GT 0xc
#define ARM_LE 0xd
#define ARM_AL 0xe
#define ARM_NV 0xf


#define HTT_INVALID 0
#define HTT_DEFINE_STR 0x1
#define HTT_KEYWORD 0x2
#define HTT_CLASS 0x4
#define HTT_GLBL_VAR 0x8
#define HTT_FUN 0x10
#define HTF_IMPORT 0x20
#define HTF_EXTERN 0x40
#define HTT_IMPORT_SYS_SYM 0x80
#define HTT_EXPORT_SYS_SYM 0x100
#define HTT_IMPORT_SYS_SYM2 0x200 //See arm_loader.c
struct CCmpCtrl;
struct CRPN;
struct CHashClass;
typedef struct CRPN CRPN; 
//
// README
//
// This was written in the spirit of TempleOS,things may seem heptic but
// they are hepticaly written for a reason.
//
// When you see README's you should read them
//
// Dec 3,2022 I was listening to Donda by Kayne West
// Dec 4,2022 I got more minutes for my phone
// Dec 5,2022 Finaly got some tea
// Dec 14,2022 Walked in the snow to get tea,and im listening to Donda by Kanye
// West
struct CHashClass;
struct CCodeCtrl;
struct CHashFun;
struct CCodeMisc;
typedef struct CQue {
	struct CQue *last, *next;
} CQue;
#define HClF_LOCKED 1
#define MEM_HEAP_HASH_SIZE 1024
#define MEM_PAG_BITS (12)
#define MEM_PAG_SIZE (1 << MEM_PAG_BITS)
struct CHashTable;
#include <ucontext.h>
typedef struct CExceptPad {
	int64_t gp[30-18+1];
	double fp[15-8+1];
} CExceptPad;
typedef struct CExcept {
	CQue base;
	CExceptPad ctx;
} CExcept;
typedef struct CTask {
	int64_t task_signature;
	CQue* except; // CExcept
	int64_t except_ch;
	int64_t catch_except;
	struct CHashTable* hash_table;
	void* stack;
	ucontext_t ctx;
	ucontext_t exit_ctx;
	CExceptPad throw_pad;
	struct CHeapCtrl* heap;
} CTask;
typedef struct CHeapCtrl {
	int32_t hc_signature;
	int64_t locked_flags, alloced_u8s, used_u8s;
	struct CTask* mem_task;
	struct CMemUnused *malloc_free_lst, *heap_hash[MEM_HEAP_HASH_SIZE / 8+1];
	CQue mem_blks;
} CHeapCtrl;
typedef struct CMemBlk {
	CQue base;
	int64_t pags;
} CMemBlk;
typedef struct CMemUnused {
	// MUST BE FIRST MEMBER
	struct CMemUnused* next;
	CHeapCtrl* hc;
	int64_t sz, pad;
} CMemUnused;
typedef struct CHash {
	struct CHash* next;
	char* str;
	int32_t type, use_cnt;
} CHash;
typedef struct CHashTable {
	struct CHashTable* next;
	int64_t mask, locked_flags;
	CHash** body;
} CHashTable;
// This represents a peice of text being lexed(it could be a file macro, or
// such)
typedef struct CLexFile {
	//
	// If we include a file,we put the current file on hold
	// last will point to the "old" file we are in
	//
	struct CLexFile* last;
	char *filename, *text;
	int64_t ln, col, pos;
} CLexFile;
typedef struct CHashDefineStr {
	struct CHash base;
	char* src_link;
	char* name;
	char* data;
} CHashDefineStr;
typedef struct CHashKeyword {
	struct CHash base;
	int64_t tk;
} CHashKeyword;
#define STR_LEN 144
typedef struct CLexer {
	CLexFile* file;
	//
	// These are our lexer values,I put them in a union to save memory
	//
	int64_t str_len;
	union {
		int64_t integer;
		double flt;
		char string[STR_LEN*6];
	};
	//
	// Sometimes we are lexing and want to go back a charactor
	// LEXF_USE_LAST_CHAR allows us to simulate this.
	//
#define LEXF_USE_LAST_CHAR 1
#define LEXF_ERROR 2
#define LEXF_NO_EXPAND 4 // Don't expand macros
	int64_t flags, cur_char;
	int64_t cur_tok;
} CLexer;
typedef enum {
	TK_I64 = 0x100,
	TK_F64,
	TK_NAME,
	TK_STR,
	TK_CHR,
	TK_ERR,
	TK_INC_INC,
	TK_DEC_DEC,
	TK_AND_AND,
	TK_XOR_XOR,
	TK_OR_OR,
	TK_EQ_EQ,
	TK_NE,
	TK_LE,
	TK_GE,
	TK_LSH,
	TK_RSH,
	TK_ADD_EQ,
	TK_SUB_EQ,
	TK_MUL_EQ,
	TK_DIV_EQ,
	TK_LSH_EQ,
	TK_RSH_EQ,
	TK_AND_EQ,
	TK_OR_EQ,
	TK_XOR_EQ,
	TK_ARROW,
	TK_DOT_DOT_DOT,
	TK_MOD_EQ,
	TK_KW_UNION,
	TK_KW_CATCH,
	TK_KW_CLASS,
	TK_KW_TRY,
	TK_KW_IF,
	TK_KW_ELSE,
	TK_KW_FOR,
	TK_KW_WHILE,
	TK_KW_EXTERN,
	TK_KW__EXTERN,
	TK_KW_RETURN,
	TK_KW_SIZEOF,
	TK_KW__INTERN,
	TK_KW_DO,
	TK_KW_ASM,
	TK_KW_GOTO,
	TK_KW_BREAK,
	TK_KW_SWITCH,
	TK_KW_START,
	TK_KW_END,
	TK_KW_CASE,
	TK_KW_DEFUALT,
	TK_KW_PUBLIC,
	TK_KW_OFFSET,
	TK_KW_IMPORT,
	TK_KW__IMPORT,
	TK_KW_REG,
	TK_KW_NOREG,
	TK_KW_LASTCLASS,
	TK_KW_NOWARN,
	TK_KW_STATIC,
	TK_KW_LOCK,
	TK_KW_DEFINED,
	TK_KW_INTERRUPT,
	TK_KW_HASERRCODE,
	TK_KW_ARGPOP,
	TK_KW_NOARGPOP,
} LexTok;
typedef struct CArrayDim {
	struct CArrayDim* next;
	int64_t cnt, total_cnt;
} CArrayDim;
typedef struct CMemberLst {
	struct CMemberLst *next, *last;
	struct CHashClass* member_class;
	CArrayDim dim;
	struct CHashFun* fun_ptr;
	char* str;
#define MLF_STATIC 1
#define MLF_DFT_AVAILABLE 2
	int64_t off, reg, use_cnt, flags;
	union {
		int64_t dft_val;
		double dft_val_flt;
		//USed for MLF_STATIC,this is a heap allocated "static" variable
		char *static_bytes;
	};
} CMemberLst;
typedef struct CHashImport {
	CHash base;
	char** address;
} CHashImport;
#define STAR_CNT 5
typedef struct CHashClass {
	CHash base;
	char* src_link;
	char* import_name;
#define REG_ALLOC -1
#define REG_NONE -2
#define REG_MAYBE -3
	// TODO make a mechanism for assigning hardware regs

#define RT_U0 -1
#define RT_UNDEF 0
#define RT_I8i 1
#define RT_U8i 2
#define RT_I16i 3
#define RT_U16i 4
#define RT_I32i 5
#define RT_U32i 6
#define RT_I64i 7
#define RT_U64i 8
#define RT_PTR 9
#define RT_F64 10
#define RT_FUNC 11

#define CLSF_VARGS 1
#define CLSF_FUNPTR 2

	int64_t member_cnt, ptr_star_cnt, raw_type, flags, use_cnt, sz;
	struct CHashClass* base_class;
	CMemberLst* members_lst;
	struct CHashClass* fwd_class;
} CHashClass;

typedef struct CHashExport {
  CHash base;
  char *val;
} CHashExport;


typedef struct CHashFun {
	CHashClass base;
	char* import_name;
	CHashClass* return_class;
	int64_t argc;
	void* fun_ptr;
} CHashFun;
typedef struct CHashGlblVar {
	CHash base;
	char *src_link, *import_name;
	CHashClass* var_class;
	void* data_addr;
	CArrayDim dim;
	CHashFun* fun_ptr;
} CHashGlblVar;
typedef struct CCodeCtrl {
	struct CCodeCtrl* next;
	CQue* ir_code;
	CQue* code_misc;
	struct CCodeMisc* break_to;
	int64_t final_pass; //See OptPassFinal
  int64_t min_ln;
  char **dbg_info;
  int64_t statics_offset;
  CHeapCtrl *hc;
} CCodeCtrl;
typedef struct CCodeMisc {
	CQue base;
#define CMT_LABEL 1
#define CMT_JMP_TAB 2
#define CMT_STRING 3
#define CMT_FLOAT_CONST 4
#define CMT_INT_CONST 5 // ARM has constraints on literals
#define CMT_RELOC_U64 6
#define CMT_STATIC_DATA 7 //integer is the offset(in the statics data) str/str_len is the data
#define CMT_SHORT_ADDR 8
#define CMF_DEFINED 1 // Used with Labels to tell if already defined
	int32_t type, flags;
	// These are used for jump tables
	int64_t lo, hi;
	char* str;
	void* addr;
	union {
		struct CCodeMisc* dft_lab;
		int64_t str_len;
    void **patch_addr;
	};
	union {
		struct CCodeMisc** jmp_tab;
		double flt;
		int64_t integer;
	};
  int64_t aot_before_hint; //See __HC_SetAOTRelocBeforeRIP
} CCodeMisc;
typedef struct CCmpCtrl {
	CLexer* lex;
//
// See PrsSwitch,this will make break statements turn into IC_SUB_RET
//
#define CCF_IN_SUBSWITCH_START_BLOCK 0x1
#define CCF_STRINGS_ON_HEAP 0x2
#define CCF_AOT_COMPILE 0x4
	int64_t flags;
	CHashFun* cur_fun;
	CCodeCtrl* code_ctrl;
	int64_t backend_user_data0;
	int64_t backend_user_data1;
	int64_t backend_user_data2;
	int64_t backend_user_data3;
	int64_t backend_user_data4;
  //Used for returns
  int64_t epilog_offset;
  CHeapCtrl *hc;
} CCmpCtrl;
#define PRSF_CLASS 1
#define PRSF_UNION 2
#define PRSF_FUN_ARGS 4
#define PRSF_EXTERN 8
#define PRSF__EXTERN 0x10
#define PRSF_IMPORT 0x20
#define PRSF__IMPORT 0x40
#define PRSF_STATIC 0x80
enum {
	IC_GOTO,
  IC_GOTO_IF,
  IC_TO_I64,
  IC_TO_F64,
	IC_LABEL,
	IC_STATIC, //Just a pointer,integer is the pointer
	IC_LOCAL,
	IC_GLOBAL,
	IC_NOP,
	IC_PAREN,
	IC_NEG,
	IC_POS,
	IC_NAME,
	IC_STR, //->code_misc
	IC_CHR,
	IC_POW,
	IC_ADD,
	IC_EQ,
	IC_SUB,
	IC_DIV,
	IC_MUL,
	IC_DEREF,
	IC_AND,
	IC_ADDR_OF,
	IC_XOR,
	IC_MOD,
	IC_OR,
	IC_LT,
	IC_GT,
	IC_LNOT,
	IC_BNOT,
	IC_POST_INC, //->integer is amount
	IC_POST_DEC, //->integer is amount
	IC_PRE_INC, //->integer is amount
	IC_PRE_DEC, //->integer is amount
	IC_AND_AND,
	IC_OR_OR,
	IC_XOR_XOR,
	IC_EQ_EQ,
	IC_NE,
	IC_LE,
	IC_GE,
	IC_LSH,
	IC_RSH,
	IC_ADD_EQ,
	IC_SUB_EQ,
	IC_MUL_EQ,
	IC_DIV_EQ,
	IC_LSH_EQ,
	IC_RSH_EQ,
	IC_AND_EQ,
	IC_OR_EQ,
	IC_XOR_EQ,
	IC_ARROW,
	IC_DOT,
	IC_MOD_EQ,
	IC_I64,
	IC_F64,
	IC_ARRAY_ACC,
	IC_RET,
	IC_CALL,
	IC_COMMA,
	IC_UNBOUNDED_SWITCH, //->length is body count
	IC_BOUNDED_SWITCH, //->length is body count
	IC_SUB_RET,
	IC_SUB_PROLOG,
	IC_SUB_CALL, //->code_misc is the label to IC_SUB_PROLOG
	IC_TYPECAST, //->ir_dim/ir_fun point to the extra silly sauce type info
	//
	// Thes are used by Opt-pass
	//
	IC_BASE_PTR,
	IC_IREG,
	IC_FREG,
	IC_LOAD,
	IC_STORE,
	//
	// These are used by the HolyC side
	//
	__IC_VARGS, //This will create vargs for  __IC_CALL
  __IC_ARG, //This will write an argument into a destination
  __IC_SET_FRAME_SIZE,
  IC_RELOC,
  __IC_CALL, //Doesnt do CHashFun specific checks,good for HolyC side
  __IC_STATICS_SIZE, //Used from the HolyC part to allocate a chunk of static stuff
  __IC_STATIC_REF, //Offset from static area,see __IC_STATICS_SIZE 
  __IC_SET_STATIC_DATA, //CMT_STATIC_DATA will have the silly sauce
  IC_SHORT_ADDR, //Like a IC_RELOC,but the jump is an AARCH64 short jump
};
typedef struct CICArg {
	// Feel free to define more in backend
#define MD_NULL 0
#define MD_REG 1 // raw_type==RT_F64 for flt register
#define MD_FRAME 2 // ditto
#define MD_PTR 3 // ditto
#define MD_INDIR_REG 4 // ditto
#define MD_I64 5
#define MD_F64 6
#define MD_STATIC 7 //off points to start of function's code 
//
// Here's the deal. Arm let's you shift like this (ldr dst,[a,b,lsl 3])
//
// integer is the shift amt
//
#define __MD_ARM_SHIFT 8
	int32_t mode;
	int32_t raw_type;
	int32_t reg;
  //True on enter of things that want to set the flags
  //True/False if the result of the thing set the flags or not
  char set_flags;
	union {
		int64_t off;
		int64_t integer;
    int64_t reg2;
		double flt;
	};
} CICArg;
struct CRPN {
	CQue base;
#define ICF_SPILLED 1 // See PushSpilledTmp in XXX_backend.c
#define ICF_DEAD_CODE 2
#define ICF_TMP_NO_UNDO 4

	int32_t type, length, raw_type, flags,ic_line;
	CHashClass* ic_class;
	CHashFun* ic_fun;
	CArrayDim* ic_dim;
	union {
		double flt;
		int64_t integer;
		char* string;
		CMemberLst* local_mem;
		CHashGlblVar* global_var;
		CCodeMisc* code_misc;
		CCodeMisc* break_to;
	};
	CICArg arg, arg2, res;
};
extern char *Compile(struct CCmpCtrl *cctrl,int64_t *sz,char **dbg_info);
extern __thread struct CTask *Fs;
extern void AIWNIOS_throw(uint64_t code);
#define throw AIWNIOS_throw
void QueDel(CQue *f);
int64_t QueCnt(CQue *head);
void QueRem(CQue *a);
void QueIns(CQue *a,CQue *to);
void QueInit(CQue *i);

void HashTableDel(CHashTable *table);
int64_t HashRemDel(CHash *h,CHashTable *table,int64_t inst);
void HashAdd(CHash *h,CHashTable *table);
CHash **HashBucketFind(char *str,CHashTable *table);
CHash *HashSingleTableFind(char *str,CHashTable *table,int64_t type,int64_t inst);
CHash *HashFind(char *str,CHashTable *table,int64_t type,int64_t inst);
CHashTable *HashTableNew(int64_t sz,void *task);
void HashDel(CHash *h);
int64_t HashStr(char *str);


void PrsBindCSymbol(char *name,void *ptr);
void ICFree(CRPN *ic);
CHashClass *PrsClassNew();
char *PrsArray(CCmpCtrl *ccmp,CHashClass *base,CArrayDim *dim,char *write_to);
int64_t PrsFunArgs(CCmpCtrl *ccmp,CHashFun *fun);
int64_t PrsArrayDim(CCmpCtrl *ccmp,CArrayDim *to);
double PrsF64(CCmpCtrl *ccmp);
int64_t PrsI64(CCmpCtrl *ccmp);
int64_t _PrsStmt(CCmpCtrl *ccmp);
int64_t ParseWarn(CCmpCtrl *ctrl,char *fmt,...);
int64_t ParseExpr(CCmpCtrl *ccmp,int64_t flags);
int64_t AssignRawTypeToNode(CCmpCtrl *ccmp,CRPN *rpn);
void SysSymImportsResolve(char *sym,int64_t flags);
CRPN *ParserDumpIR(CRPN *rpn,int64_t indent);
CRPN *ICArgN(CRPN *rpn,int64_t n);
CRPN *ICFwd(CRPN *rpn);
CCmpCtrl *CmpCtrlNew(CLexer *lex);
CMemberLst *MemberFind(char *needle,CHashClass *cls);
CMemberLst *MemberFind(char *needle,CHashClass *cls);
CHashClass *PrsClass(CCmpCtrl *cctrl,int64_t flags);
CHashClass *PrsClass(CCmpCtrl *cctrl,int64_t _flags);
int64_t PrsDecl(CCmpCtrl *ccmp,CHashClass *base,CHashClass *add_to,int64_t *is_func_decl,int64_t flags,char *import_name);
int64_t PrsDecl(CCmpCtrl *ccmp,CHashClass *base,CHashClass *add_to,int64_t *is_func_decl,int64_t flags,char *import_name);
int64_t PrsTry(CCmpCtrl *cctrl);
int64_t PrsTry(CCmpCtrl *cctrl);
int64_t ParseErr(CCmpCtrl *ctrl,char *fmt,...);
int64_t ParseErr(CCmpCtrl *ctrl,char *fmt,...);
int64_t PrsReturn(CCmpCtrl *ccmp);
int64_t PrsReturn(CCmpCtrl *ccmp);
int64_t PrsGoto(CCmpCtrl *ccmp);
int64_t PrsGoto(CCmpCtrl *ccmp);
int64_t PrsLabel(CCmpCtrl *ccmp);
int64_t PrsScope(CCmpCtrl *ccmp);
int64_t PrsScope(CCmpCtrl *ccmp);
int64_t PrsSwitch(CCmpCtrl *ccmp);
int64_t PrsSwitch(CCmpCtrl *cctrl);
int64_t PrsSwitch(CCmpCtrl *cctrl);
int64_t PrsFor(CCmpCtrl *ccmp);
int64_t PrsFor(CCmpCtrl *ccmp);
int64_t PrsWhile(CCmpCtrl *ccmp);
int64_t PrsWhile(CCmpCtrl *ccmp);
int64_t PrsDo(CCmpCtrl *ccmp);
int64_t PrsDo(CCmpCtrl *ccmp);
int64_t PrsIf(CCmpCtrl *ccmp);
int64_t PrsIf(CCmpCtrl *ccmp);
int64_t PrsStmt(CCmpCtrl *ccmp);
int64_t PrsStmt(CCmpCtrl *ccmp);
int64_t PrsKw(CCmpCtrl *ccmp,int64_t);
int64_t PrsKw(CCmpCtrl *ccmp,int64_t kwt);
CCodeMisc *CodeMiscNew(CCmpCtrl *ccmp,int64_t type);
CHashClass *PrsType(CCmpCtrl *ccmp,CHashClass *base,char **name,CHashFun **fun,CArrayDim *dim);
CHashClass *PrsType(CCmpCtrl *ccmp,CHashClass *base,char **name,CHashFun **fun,CArrayDim *dim);
void PrsTests();

void TaskInit(CTask *task,void *addr,int64_t stk_sz);
struct CHeapCtrl *HeapCtrlInit(struct CHeapCtrl *ct,CTask *task);
void TaskExit();
extern __thread struct CTask *HolyFs;
extern __thread struct CTask *Fs;

char *OptPassFinal(CCmpCtrl *cctrl,int64_t *res_sz,char **dbg_info);

void AIWNIOS_ExitCatch();
CExceptPad *__throw(uint64_t code);
void AIWNIOS_throw(uint64_t code);
int64_t AIWNIOS_enter_try();
CExceptPad *__enter_try();

void LexTests();
void LexerDump(CLexer *lex);
CLexer *LexerNew(char *filename,char *text);
int64_t Lex(CLexer *lex);
int64_t LexAdvChr(CLexer *lex);
char *LexSrcLink(CLexer *lex,void *task);


char *__AIWNIOS_StrDup(char *str,void *t);
void HeapCtrlDel(CHeapCtrl *ct);
CHeapCtrl *HeapCtrlInit(CHeapCtrl *ct,CTask *task);
void *__AIWNIOS_CAlloc(int64_t cnt,void *t);
int64_t MSize(void *ptr);
void __AIWNIOS_Free(void *ptr);
void *__AIWNIOS_MAlloc(int64_t cnt,void *t);

int64_t LBtr(void *,int64_t);
int64_t LBts(void *,int64_t);
int64_t Bt(void *,int64_t);
int64_t Btc(void *,int64_t);
int64_t Bsr(int64_t v);
int64_t Bsf(int64_t v);
void *Caller(int64_t c);
uint64_t ToUpper(uint64_t ch);
char *WhichFun(char *fptr);
int64_t LBtc(char*,int64_t);

//
// These are used by optpass
//
#define AIWNIOS_IREG_START 19
#define AIWNIOS_IREG_CNT (28 - 19 + 1)
#define AIWNIOS_REG_FP ARM_REG_FP
#define AIWNIOS_REG_SP ARM_REG_SP
#define AIWNIOS_TMP_IREG_POOP 18 //Platform register,I use it as a poo poo
#define AIWNIOS_TMP_IREG_POOP2 17 //I use it as a second poo poo ALWAYS
#define AIWNIOS_TMP_IREG_START 8
#define AIWNIOS_TMP_IREG_CNT (16 - 8 + 1)
#define AIWNIOS_FREG_START 8
#define AIWNIOS_FREG_CNT (15 - 8 + 1)
#define AIWNIOS_TMP_FREG_START 16
#define AIWNIOS_TMP_FREG_CNT (31 - 16 + 1)


#define try                                              \
	{                                                    \
		if (AIWNIOS_enter_try()) {
#define catch(body)               \
	} else                          \
	{                             \
		body; \
    AIWNIOS_ExitCatch();                     \
	}                             \
	}
int64_t ARM_fmovF64I64(int64_t d,int64_t s);
int64_t ARM_fmovI64F64(int64_t d,int64_t s);
int64_t ARM_ldrRegRegF64(int64_t d,int64_t n,int64_t m);
int64_t ARM_strRegRegF64(int64_t d,int64_t n,int64_t m);
int64_t ARM_ldrPreImmF64(int64_t d,int64_t p,int64_t off);
int64_t ARM_strPreImmF64(int64_t d,int64_t p,int64_t off);
int64_t ARM_ldrPostImmF64(int64_t d,int64_t p,int64_t off);
int64_t ARM_strPostImmF64(int64_t d,int64_t p,int64_t off);
int64_t ARM_movnImmX(int64_t d,int64_t i16,int64_t sh);
int64_t ARM_movzImmX(int64_t d,int64_t i16,int64_t sh);
int64_t ARM_asrvRegX(int64_t d,int64_t n,int64_t m);
int64_t ARM_lsrvRegX(int64_t d,int64_t n,int64_t m);
int64_t ARM_lslvRegX(int64_t d,int64_t n,int64_t m);
int64_t ARM_sdivRegX(int64_t d,int64_t n,int64_t m);
int64_t ARM_fcmp(int64_t a,int64_t b);
int64_t ARM_fmovReg(int64_t d,int64_t s);
int64_t ARM_fnegReg(int64_t d,int64_t s);
int64_t ARM_strRegImmF64(int64_t r,int64_t n,uint64_t off);
int64_t ARM_ldrRegImmF64(int64_t r,int64_t n,uint64_t off);
int64_t ARM_ldpPostImmX(int64_t r,int64_t r2,int64_t b,int64_t off);
int64_t ARM_ldpPostImm(int64_t r,int64_t r2,int64_t b,int64_t off);
int64_t ARM_stpPostImmX(int64_t r,int64_t r2,int64_t b,int64_t off);
int64_t ARM_stpPostImm(int64_t r,int64_t r2,int64_t b,int64_t off);
int64_t ARM_ldpPreImmX(int64_t r,int64_t r2,int64_t b,int64_t off);
int64_t ARM_ldpPreImm(int64_t r,int64_t r2,int64_t b,int64_t off);
int64_t ARM_stpPreImmX(int64_t r,int64_t r2,int64_t b,int64_t off);
int64_t ARM_stpPreImm(int64_t r,int64_t r2,int64_t b,int64_t off);
int64_t ARM_ldrPreImmX(int64_t r,int64_t b,int64_t off);
int64_t ARM_strPreImmX(int64_t r,int64_t b,int64_t off);
int64_t ARM_ldrPostImmX(int64_t r,int64_t b,int64_t off);
int64_t ARM_strPostImmX(int64_t r,int64_t b,int64_t off);
int64_t ARM_ldrPreImm(int64_t r,int64_t b,int64_t off);
int64_t ARM_strPreImm(int64_t r,int64_t b,int64_t off);
int64_t ARM_ldrPostImm(int64_t r,int64_t b,int64_t off);
int64_t ARM_strPostImm(int64_t r,int64_t b,int64_t off);
int64_t ARM_ldrLabelF64(int64_t d,int64_t label);
int64_t ARM_ldrLabelX(int64_t d,int64_t label);
int64_t ARM_ldrLabel(int64_t d,int64_t label);
int64_t ARM_fcvtzs(int64_t d,int64_t n);
int64_t ARM_ucvtf(int64_t d,int64_t n);
int64_t ARM_scvtf(int64_t d,int64_t n);
int64_t ARM_fsubReg(int64_t d,int64_t n,int64_t m);
int64_t ARM_faddReg(int64_t d,int64_t n,int64_t m);
int64_t ARM_fdivReg(int64_t d,int64_t n,int64_t m);
int64_t ARM_fmulReg(int64_t d,int64_t n,int64_t m);
int64_t ARM_ldrRegRegX(int64_t r,int64_t a,int64_t b);
int64_t ARM_strRegRegX(int64_t r,int64_t a,int64_t b);
int64_t ARM_ldrRegImmX(int64_t r,int64_t n,int64_t off);
int64_t ARM_strRegImmX(int64_t r,int64_t n,int64_t off);
int64_t ARM_ldrRegImm(int64_t r,int64_t n,int64_t off);
int64_t ARM_ldrRegReg(int64_t r,int64_t a,int64_t b);
int64_t ARM_strRegReg(int64_t r,int64_t a,int64_t b);
int64_t ARM_strRegImm(int64_t r,int64_t n,int64_t off);
int64_t ARM_ldrhRegImm(int64_t r,int64_t n,int64_t off);
int64_t ARM_ldrhRegReg(int64_t r,int64_t a,int64_t b);
int64_t ARM_strhRegImm(int64_t r,int64_t n,int64_t off);
int64_t ARM_strhRegReg(int64_t r,int64_t a,int64_t b);
int64_t ARM_ldrbRegReg(int64_t r,int64_t a,int64_t b);
int64_t ARM_ldrbRegImm(int64_t r,int64_t n,int64_t off);
int64_t ARM_strbRegImm(int64_t r,int64_t n,int64_t off);
int64_t ARM_strbRegReg(int64_t r,int64_t a,int64_t b);
int64_t ARM_tbnz(int64_t rt,int64_t bit,int64_t label);
int64_t ARM_tbz(int64_t rt,int64_t bit,int64_t label);
int64_t ARM_cbnzX(int64_t r,int64_t label);
int64_t ARM_cbnz(int64_t r,int64_t label);
int64_t ARM_cbzX(int64_t r,int64_t label);
int64_t ARM_cbz(int64_t r,int64_t label);
int64_t ARM_bl(int64_t lab);
int64_t ARM_b(int64_t lab);
int64_t ARM_bcc(int64_t cond,int64_t lab);
int64_t ARM_retReg(int64_t r);
int64_t ARM_blr(int64_t r);
int64_t ARM_br(int64_t r);
int64_t ARM_eret();
int64_t ARM_ret();
int64_t ARM_lsrImmX(int64_t d,int64_t n,int64_t imm);
int64_t ARM_lsrImm(int64_t d,int64_t n,int64_t imm);
int64_t ARM_lslImmX(int64_t d,int64_t n,int64_t imm);
int64_t ARM_lslImm(int64_t d,int64_t n,int64_t imm);
int64_t ARM_asrImmX(int64_t d,int64_t n,int64_t imm);
int64_t ARM_asrImm(int64_t d,int64_t n,int64_t imm);
int64_t ARM_sxtwX(int64_t d,int64_t n);
int64_t ARM_sxtw(int64_t d,int64_t n);
int64_t ARM_sxthX(int64_t d,int64_t n);
int64_t ARM_sxth(int64_t d,int64_t n);
int64_t ARM_sxtbX(int64_t d,int64_t n);
int64_t ARM_sxtb(int64_t d,int64_t n);
int64_t ARM_uxtwX(int64_t d,int64_t n);
int64_t ARM_uxthX(int64_t d,int64_t n);
int64_t ARM_uxth(int64_t d,int64_t n);
int64_t ARM_uxtbX(int64_t d,int64_t n);
int64_t ARM_uxtb(int64_t d,int64_t n);
int64_t ARM_mulRegX(int64_t d,int64_t n,int64_t m);
int64_t ARM_csetX(int64_t d,int64_t cond);
int64_t ARM_cset(int64_t d,int64_t cond);
int64_t ARM_cselX(int64_t d,int64_t n,int64_t m,int64_t cond);
int64_t ARM_csel(int64_t d,int64_t n,int64_t m,int64_t cond);
int64_t ARM_cmpRegX(int64_t n,int64_t m);
int64_t ARM_negsRegX(int64_t d,int64_t m);
int64_t ARM_subRegX(int64_t d,int64_t n,int64_t m);
int64_t ARM_addsRegX(int64_t d,int64_t n,int64_t m);
int64_t ARM_addRegX(int64_t d,int64_t n,int64_t m);
int64_t ARM_movRegX(int64_t d,int64_t n);
int64_t ARM_movReg(int64_t d,int64_t n);
int64_t ARM_andsRegX(int64_t rd,int64_t rn,int64_t rm);
int64_t ARM_eorRegX(int64_t rd,int64_t rn,int64_t rm);
int64_t ARM_eorShiftX(int64_t rd,int64_t rn,int64_t rm,int64_t shmod,int64_t sh);
int64_t ARM_mvnRegX(int64_t rd,int64_t rm);
int64_t ARM_mvnReg(int64_t rd,int64_t rm);
int64_t ARM_mvnShiftX(int64_t rd,int64_t rm,int64_t shmod,int64_t sh);
int64_t ARM_mvnShift(int64_t rd,int64_t rm,int64_t shmod,int64_t sh);
int64_t ARM_orrRegX(int64_t rd,int64_t rn,int64_t rm);
int64_t ARM_andRegX(int64_t rd,int64_t rn,int64_t rm);
int64_t ARM_cmpImmX(int64_t n,int64_t imm);
int64_t ARM_subImmX(int64_t d,int64_t n,int64_t imm);
int64_t ARM_addImmX(int64_t d,int64_t n,int64_t imm);
int64_t ARM_adrX(int64_t d,int64_t pc_off);
int64_t ARM_strbRegRegShift(int64_t a,int64_t n,int64_t m);
int64_t ARM_ldrbRegRegShift(int64_t a,int64_t n,int64_t m);
int64_t ARM_strbRegRegShift(int64_t a,int64_t n,int64_t m);
int64_t ARM_ldrhRegRegShift(int64_t a,int64_t n,int64_t m);
int64_t ARM_strhRegRegShift(int64_t a,int64_t n,int64_t m);
int64_t ARM_ldrRegRegShift(int64_t a,int64_t n,int64_t m);
int64_t ARM_strRegRegShift(int64_t a,int64_t n,int64_t m);
int64_t ARM_ldrRegRegShiftX(int64_t a,int64_t n,int64_t m);
int64_t ARM_strRegRegShiftX(int64_t a,int64_t n,int64_t m);
int64_t ARM_ldpImmX(int64_t r1,int64_t r2,int64_t ra,int64_t off);
int64_t ARM_stpImmX(int64_t r1,int64_t r2,int64_t ra,int64_t off);
CCodeCtrl *CodeCtrlPush(CCmpCtrl* ccmp);
void CodeCtrlDel(CCodeCtrl* ctrl);
void CodeCtrlPop(CCmpCtrl* ccmp);
CRPN *__HC_ICAdd_Typecast(CCodeCtrl *cc,int64_t rt,int64_t ptr_cnt);
CRPN *__HC_ICAdd_SubCall(CCodeCtrl *cc,CCodeMisc *cm);
CRPN *__HC_ICAdd_SubProlog(CCodeCtrl *cc);
CRPN *__HC_ICAdd_SubRet(CCodeCtrl *cc);
CRPN *__HC_ICAdd_UnboundedSwitch(CCodeCtrl *cc,CCodeMisc *misc);
CRPN *__HC_ICAdd_PreInc(CCodeCtrl *cc,int64_t amt);
CRPN *__HC_ICAdd_Call(CCodeCtrl *cc,int64_t arity,int64_t rt,int64_t ptrs);
CRPN *__HC_ICAdd_F64(CCodeCtrl *cc,double f);
CRPN *__HC_ICAdd_I64(CCodeCtrl *cc,int64_t integer);
CRPN *__HC_ICAdd_PreDec(CCodeCtrl *cc,int64_t amt);
CRPN *__HC_ICAdd_PostDec(CCodeCtrl *cc,int64_t amt);
CRPN *__HC_ICAdd_PostInc(CCodeCtrl *cc,int64_t amt);
#define HC_IC_BINDINGH(name) \
  CRPN *__##name(CCodeCtrl *cc);
HC_IC_BINDINGH(HC_ICAdd_Pow)
HC_IC_BINDINGH(HC_ICAdd_Eq)
HC_IC_BINDINGH(HC_ICAdd_Div)
HC_IC_BINDINGH(HC_ICAdd_Sub)
HC_IC_BINDINGH(HC_ICAdd_Mul)
HC_IC_BINDINGH(HC_ICAdd_Add)
HC_IC_BINDINGH(HC_ICAdd_Comma)
HC_IC_BINDINGH(HC_ICAdd_Addr)
HC_IC_BINDINGH(HC_ICAdd_Xor)
HC_IC_BINDINGH(HC_ICAdd_Mod)
HC_IC_BINDINGH(HC_ICAdd_Or)
HC_IC_BINDINGH(HC_ICAdd_Lt)
HC_IC_BINDINGH(HC_ICAdd_Gt)
HC_IC_BINDINGH(HC_ICAdd_Le)
HC_IC_BINDINGH(HC_ICAdd_Ge)
HC_IC_BINDINGH(HC_ICAdd_LNot)
HC_IC_BINDINGH(HC_ICAdd_BNot)
HC_IC_BINDINGH(HC_ICAdd_AndAnd)
HC_IC_BINDINGH(HC_ICAdd_OrOr)
HC_IC_BINDINGH(HC_ICAdd_XorXor)
HC_IC_BINDINGH(HC_ICAdd_EqEq)
HC_IC_BINDINGH(HC_ICAdd_Ne)
HC_IC_BINDINGH(HC_ICAdd_Lsh)
HC_IC_BINDINGH(HC_ICAdd_Rsh)
HC_IC_BINDINGH(HC_ICAdd_AddEq)
HC_IC_BINDINGH(HC_ICAdd_SubEq)
HC_IC_BINDINGH(HC_ICAdd_MulEq)
HC_IC_BINDINGH(HC_ICAdd_DivEq)
HC_IC_BINDINGH(HC_ICAdd_LshEq)
HC_IC_BINDINGH(HC_ICAdd_RshEq)
HC_IC_BINDINGH(HC_ICAdd_AndEq)
HC_IC_BINDINGH(HC_ICAdd_OrEq)
HC_IC_BINDINGH(HC_ICAdd_XorEq)
HC_IC_BINDINGH(HC_ICAdd_ModEq)
HC_IC_BINDINGH(HC_ICAdd_Neg)
HC_IC_BINDINGH(HC_ICAdd_And)
HC_IC_BINDINGH(HC_ICAdd_EqEq)
HC_IC_BINDINGH(HC_ICAdd_Ret)
HC_IC_BINDINGH(HC_ICAdd_Add)
HC_IC_BINDINGH(HC_ICAdd_ToI64)
HC_IC_BINDINGH(HC_ICAdd_ToF64)
CRPN *__HC_ICAdd_FReg(CCodeCtrl *cc,int64_t r);
CRPN *__HC_ICAdd_IReg(CCodeCtrl *cc,int64_t r,int64_t rt,int64_t ptr_cnt);
CRPN *__HC_ICAdd_Frame(CCodeCtrl *cc,int64_t off,int64_t rt,int64_t ptr_cnt);
CCodeMisc *__HC_CodeMiscJmpTableNew(CCmpCtrl *ccmp,CCodeMisc *labels,int64_t lo,int64_t hi);
CCodeMisc *__HC_CodeMiscStrNew(CCmpCtrl *ccmp,char *str,int64_t sz);
CCodeMisc *__HC_CodeMiscLabelNew(CCmpCtrl *ccmp);
CCmpCtrl *__HC_CmpCtrlNew();
CCodeCtrl *__HC_CodeCtrlPush(CCmpCtrl *ccmp);
CCodeCtrl *__HC_CodeCtrlPop(CCmpCtrl *ccmp);
char *__HC_Compile(CCmpCtrl *ccmp,int64_t *sz,char **dbg_info);
CRPN *__HC_ICAdd_Goto(CCodeCtrl* cc,CCodeMisc *cm);
CRPN *__HC_ICAdd_GotoIf(CCodeCtrl* cc,CCodeMisc *cm);
CRPN *__HC_ICAdd_Str(CCodeCtrl* cc,CCodeMisc *cm);
CRPN *__HC_ICAdd_Label(CCodeCtrl *cc,CCodeMisc *misc);
CRPN *__HC_ICAdd_Arg(CCodeCtrl *cc,int64_t arg);
CRPN *__HC_ICAdd_SetFrameSize(CCodeCtrl *cc,int64_t arg);
CRPN *__HC_ICAdd_Reloc(CCmpCtrl *cmpc,CCodeCtrl* cc, int64_t *pat_addr,char *sym,int64_t rt,int64_t ptrs);
CCodeMisc* AddRelocMisc(CCmpCtrl* cctrl, char* name);
CRPN *__HC_ICAdd_Deref(CCodeCtrl* cc, int64_t rt, int64_t ptr_cnt);
void __HC_ICSetLine(CRPN *r,int64_t ln);
CRPN *__HC_ICAdd_Switch(CCodeCtrl *cc,CCodeMisc *misc,CCodeMisc *dft);
CRPN* __HC_ICAdd_Vargs(CCodeCtrl* cc, int64_t arity);
CRPN *__HC_ICAdd_StaticData(CCmpCtrl *cmp,CCodeCtrl* cc,int64_t at,char *d,int64_t len);
CRPN *__HC_ICAdd_StaticRef(CCodeCtrl* cc,int64_t off,int64_t rt,int64_t ptrs);
CRPN *__HC_ICAdd_SetStaticsSize(CCodeCtrl* cc,int64_t len);
char* Load(char* filename);
CRPN *__HC_ICAdd_ShortAddr(CCmpCtrl *,CCodeCtrl* cc,char *name,char **ptr);
//TODO remove
char *FileRead(char *fn,int64_t *sz);
void FileWrite(char *fn,char *data,int64_t sz);

void VFsThrdInit();
void VFsSetDrv(char d);
int VFsCd(char *to,int flags);
int64_t VFsDel(char *p);
char *__VFsFileNameAbs(char *name);
int64_t VFsUnixTime(char *name);
int64_t VFsFSize(char *name);
int64_t VFsFileWrite(char *name,char *data,int64_t len);
int64_t VFsFileRead(char *name,int64_t *len);
int VFsFileExists(char *path);
int VFsMountDrive(char let,char *path);
FILE *VFsFOpen(char *path,char *m);
int64_t VFsFClose(FILE *f);
int64_t VFsTrunc(char *fn,int64_t sz);
int64_t VFsFBlkRead(void *d,int64_t n, int64_t sz,FILE *f);
int64_t VFsFBlkWrite(void *d,int64_t n, int64_t sz,FILE *f);
int64_t VFsFSeek(int64_t off,FILE *f);
FILE *VFsFOpenW(char *f);
FILE *VFsFOpenR(char *f);
void VFsSetPwd(char *pwd);
int64_t VFsDirMk(char *f);
char **VFsDir(char *fn);
int64_t VFsIsDir(char *name);

void DrawWindowNew();
void UpdateScreen(char *px,int64_t w,int64_t h,int64_t wi);
void GrPaletteColorSet(int64_t i,uint64_t bgr48);

void LaunchSDL(void (*boot_ptr)(void *data),void *data);
void WaitForSDLQuit();
void SetKBCallback(void *fptr);
void SetMSCallback(void *fptr);

void ImportSymbolsToHolyC(void(*cb)(char *name,void *addr));
int64_t ARM_eorImmX(int64_t d,int64_t s,int64_t i);
int64_t ARM_orrImmX(int64_t d,int64_t s,int64_t i);
int64_t ARM_andImmX(int64_t d,int64_t s,int64_t i);
CCmpCtrl* CmpCtrlDel(CCmpCtrl *d);
void SndFreq(int64_t f);
void InitSound();
int64_t IsValidPtr(char *chk);
void InstallDbgSignalsForThread();
void *GetHolyGs();
void SetHolyGs(void *);
int64_t mp_cnt();
void SpawnCore(void(*fp)(),void *gs,int64_t core);
void MPSleepHP(int64_t ns);
void MPAwake(int64_t core);
extern int64_t user_ev_num;
int64_t Btr(void *,int64_t);
int64_t Bts(void *,int64_t);
int64_t ARM_andsImmX(int64_t d,int64_t n,int64_t imm);
void __HC_CmpCtrl_SetAOT(CCmpCtrl *cc);
int64_t ARM_udivX(int64_t d,int64_t n,int64_t m);
int64_t ARM_umullX(int64_t d,int64_t n,int64_t m);
void InteruptCore(int64_t core);
extern CHashTable *glbl_table;
//Sets how many bytes before function start a symbol starts at
//Symbol    <=====RIP-off
//some...code
//Fun Start <==== RIP
void __HC_SetAOTRelocBeforeRIP(CRPN *r,int64_t off);
