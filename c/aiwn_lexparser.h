#pragma once
#define ERR         0x7fFFffFFffFFffFF
#define INVALID_PTR ERR
typedef struct CRPN CRPN;
#include "aiwn_hash.h"
#include <stdint.h>
struct CTask;
// This represents a peice of text being lexed(it could be a file macro, or
// such)
typedef struct CLexFile {
  //
  // If we include a file,we put the current file on hold
  // last will point to the "old" file we are in
  //
  struct CLexFile *last;
  char *filename, *text, *dir;
  int64_t ln, col, pos, is_file;
} CLexFile;
#define STR_LEN 144
typedef struct CLexer {
  CLexFile *file;
  //
  // These are our lexer values,I put them in a union to save memory
  //
  int64_t str_len;
  union {
    int64_t integer;
    double flt;
    char string[STR_LEN * 6];
  };
  //
  // Sometimes we are lexing and want to go back a charactor
  // LEXF_USE_LAST_CHAR allows us to simulate this.
  //
#define LEXF_USE_LAST_CHAR 1
#define LEXF_ERROR         2
#define LEXF_NO_EXPAND     4 // Don't expand macros
#define LEXF_UNTIL_NEWLINE 8
  int64_t flags, cur_char;
  int64_t cur_tok;
  struct CHeapCtrl *hc;
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
typedef struct CCodeCtrl {
  struct CCodeCtrl *next;
  CQue *ir_code;
  CQue *code_misc;
  struct CCodeMisc *break_to;
  int64_t final_pass; // See OptPassFinal
  int64_t min_ln;
  char **dbg_info;
  int64_t statics_offset;
  struct CHeapCtrl *hc;
} CCodeCtrl;
typedef struct CCodeMiscRef {
  struct CCodeMiscRef *next;
  CRPN *from_ic;
  int32_t *add_to, offset;
  // For arm
  int32_t (*patch_cond_br)(int64_t, int64_t);
  int32_t (*patch_uncond_br)(int64_t);
  int64_t user_data1;
  int64_t user_data2;
  int8_t is_abs64;
  // Used for RISC-V
  int8_t is_jal;     // Not JALR
  int8_t is_4_bytes; // Default is 8 bytes
} CCodeMiscRef;
typedef struct CCodeMisc {
  CQue base;
#define CMT_LABEL       1
#define CMT_JMP_TAB     2
#define CMT_STRING      3
#define CMT_FLOAT_CONST 4
#define CMT_INT_CONST   5 // ARM has constraints on literals
#define CMT_RELOC_U64   6
#define CMT_STATIC_DATA                                                        \
  7 // integer is the offset(in the statics data) str/str_len is the data
#define CMT_SHORT_ADDR 8
#define CMF_DEFINED    1 // Used with Labels to tell if already defined
#define CMF_JMP_TABLE_TAINTED                                                  \
  2 // Used with jump tables to tell if we tained the floating point registers
  int32_t type, flags;
  // These are used for jump tables
  int64_t lo, hi;
  char *str;
  void *addr;
  union {
    struct CCodeMisc *dft_lab;
    int64_t str_len;
  };
  void **patch_addr;
  union {
    struct CCodeMisc **jmp_tab;
    double flt;
    int64_t integer;
  };
  int32_t aot_before_hint; // See __HC_SetAOTRelocBeforeRIP
  int32_t use_cnt;
  int32_t code_off; // Used for riscv_backend.c for choosing big/small jumps
  // The bit is set if the floating point register is alive at this inst
  int64_t freg_alive_bmp;
  CCodeMiscRef *refs;
} CCodeMisc;
typedef struct CCmpCtrl {
  CLexer *lex;
//
// See PrsSwitch,this will make break statements turn into IC_SUB_RET
//
#define CCF_IN_SUBSWITCH_START_BLOCK 0x1
#define CCF_STRINGS_ON_HEAP          0x2
#define CCF_AOT_COMPILE              0x4
#define CCF_ICMOV_NO_USE_RAX         0x8
  int64_t flags;
  CHashFun *cur_fun;
  CCodeCtrl *code_ctrl;
  int64_t backend_user_data0;
  int64_t backend_user_data1;
  int64_t backend_user_data2;
  int64_t backend_user_data3;
  int64_t backend_user_data4;
  int64_t backend_user_data5;
  int64_t backend_user_data6;
  int64_t backend_user_data7;
  int64_t backend_user_data8;
  int64_t backend_user_data9;
  int64_t backend_user_data10;
  int64_t backend_user_data11;
  int64_t prolog_stk_sz;
  int64_t used_iregs_bmp;
  int64_t used_fregs_bmp;
  int found_used_iregs;
  int found_used_fregs;
  // Used for returns
  CCodeMisc *epilog_label, *statics_label;
  // In SysV,the fregs are not saved,so i will make a mini function to save them
  CCodeMisc *fregs_save_label, *fregs_restore_label;
  struct CHeapCtrl *hc, *final_hc;
  int64_t is_lock_expr; // lock EXPRESSION;
  // private for AARCH64 for use with IC_LOCK
  // I will use the ldxsr/stxr instructions in a loop
  int64_t aarch64_atomic_loop_start;
  CRPN *cur_rpn;
} CCmpCtrl;
#define PRSF_CLASS    1
#define PRSF_UNION    2
#define PRSF_FUN_ARGS 4
#define PRSF_EXTERN   8
#define PRSF__EXTERN  0x10
#define PRSF_IMPORT   0x20
#define PRSF__IMPORT  0x40
#define PRSF_STATIC   0x80
enum {
  IC_GOTO,
  IC_SQR,
  IC_GOTO_IF,
  IC_TO_I64,
  IC_TO_F64,
  IC_LABEL,
  IC_STATIC, // Just a pointer,integer is the pointer
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
  IC_PRE_INC,  //->integer is amount
  IC_PRE_DEC,  //->integer is amount
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
  IC_BOUNDED_SWITCH,   //->length is body count
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
  __IC_VARGS, // This will create vargs for  __IC_CALL
  __IC_ARG,   // This will write an argument into a destination
  __IC_SET_FRAME_SIZE,
  IC_RELOC,
  __IC_CALL,         // Doesnt do CHashFun specific checks,good for HolyC side
  __IC_STATICS_SIZE, // Used from the HolyC part to allocate a chunk of static
                     // stuff
  __IC_STATIC_REF,   // Offset from static area,see __IC_STATICS_SIZE
  __IC_SET_STATIC_DATA, // CMT_STATIC_DATA will have the silly sauce
  IC_SHORT_ADDR,        // Like a IC_RELOC,but the jump is an AARCH64 short jump
                        // These are "optional"
  IC_BT,
  IC_BTC,
  IC_BTS,
  IC_BTR,
  IC_LBTC,
  IC_LBTS,
  IC_LBTR,
  IC_MAX_I64,
  IC_MIN_I64,
  IC_MAX_U64,
  IC_MIN_U64,
  IC_MAX_F64,
  IC_MIN_F64,
  IC_SIGN_I64,
  IC_SQR_I64,
  IC_SQR_U64,
  IC_ABS,
  IC_SIN,
  IC_COS,
  IC_TAN,
  IC_ATAN,
  IC_RAW_BYTES,
  IC_GET_VARGS_PTR, // Doing this sets the function as a VARGS function
  IC_LOCK, // Lock means "lock EXPRESSION;",it operates on expressions as a
           // whole
  IC_FS,
  IC_GS,
  IC_TO_BOOL,
  IC_SQRT,
  IC_CNT, // MUST BE THE LAST ITEM
};
typedef struct CICArg {
  // Feel free to define more in backend
#define MD_NULL      -1
#define MD_REG       1 // raw_type==RT_F64 for flt register
#define MD_FRAME     2 // ditto
#define MD_PTR       3 // ditto
#define MD_INDIR_REG 4 // ditto
#define MD_I64       5
#define MD_F64       6
#define MD_STATIC    7 // off points to start of function's code
//
// Here's the deal. Arm let's you shift like this (ldr dst,[a,b,lsl 3])
//
// integer is the shift amt
//
#define __MD_ARM_SHIFT 8
//
// X86_64 SIB
//
#define __MD_X86_64_SIB 9
// Like X86_64 but uses LEA
#define __MD_X86_64_LEA_SIB 10
#define MD_CODE_MISC_PTR    11
#define __MD_ARM_SHIFT_ADD  12 // Like lea
#define __MD_ARM_SHIFT_SUB  13 // Like lea
  int32_t mode;
  int32_t raw_type;
  int8_t reg, reg2, fallback_reg;
  // True on enter of things that want to set the flags
  // True/False if the result of the thing set the flags or not
  char want_use_flags; // This is reset by __OptPassFinal
  char set_flags;
  // keep the value in a temp location,good for removing reundant stores
  char keep_in_tmp;
  char keep_in_reg0; // RAX on X86_64
  union {
    CCodeMisc *code_misc;
    int64_t integer;
    int64_t off;
    double flt;
  };
  int8_t __SIB_scale, pop_n_tmp_regs;
  char is_tmp;
  CRPN *__sib_base_rpn, *__sib_idx_rpn;
} CICArg;
enum {
  ICF_SPILLED = 1, // See PushSpilledTmp in XXX_backend.c
  ICF_DEAD_CODE = 2,
  ICF_TMP_NO_UNDO = 4,
  ICF_PRECOMPUTED =
      8,        // Doesnt re-compile a node,useful for putting in "dummy" values
  ICF_SIB = 16, // Has 2 registers (base and idnex)
  ICF_INDIR_REG = 32,    // Has 1 registers (idnex)
  ICF_STUFF_IN_REG = 64, // Will stuff the result into a register(.stuff_in_reg)
                         // once result is computed
  ICF_LOCK_EXPR = 128,   // Used with lock {}
  ICF_IS_BOOL = 256,
  ICF_NO_JUMP = 512, // Used for eliminating jumps to next instruction
  ICF_SPILLS_TMP_REGS = 1024,
  ICF_DOESNT_SPILL_TMP_REGS = 2048,
};
struct CRPN {
  CQue base;

  int16_t type, length, raw_type, flags, ic_line;
  CHashClass *ic_class;
  CHashFun *ic_fun;
  CArrayDim *ic_dim;
  union {
    double flt;
    char *raw_bytes;
    int64_t integer;
    char *string;
    CMemberLst *local_mem;
    CHashGlblVar *global_var;
    CCodeMisc *code_misc;
    CCodeMisc *break_to;
  };
  CCodeMisc *code_misc2, *code_misc3, *code_misc4;
  CICArg res, tmp_res;
  CRPN *tree1, *tree2, *ic_fwd;
  // Use with Misc_Bt,includes temporaries
  uint32_t changes_iregs, changes_fregs;
  uint32_t changes_iregs2, changes_fregs2;
  // Will be stored into this reg if ICF_STUFF_IN_REG is set
  void *start_ptr, *end_ptr;
  char stuff_in_reg;
};

extern char *Compile(struct CCmpCtrl *cctrl, int64_t *sz, char **dbg_info,
                     struct CHeapCtrl *);
void ICFree(CRPN *ic);
CHashClass *PrsClassNew();
char *PrsArray(CCmpCtrl *ccmp, CHashClass *base, CArrayDim *dim,
               char *write_to);
int64_t PrsFunArgs(CCmpCtrl *ccmp, CHashFun *fun);
int64_t PrsArrayDim(CCmpCtrl *ccmp, CArrayDim *to);
double PrsF64(CCmpCtrl *ccmp);
int64_t PrsI64(CCmpCtrl *ccmp);
int64_t _PrsStmt(CCmpCtrl *ccmp);
int64_t ParseWarn(CCmpCtrl *ctrl, char *fmt, ...);
int64_t ParseExpr(CCmpCtrl *ccmp, int64_t flags);
int64_t AssignRawTypeToNode(CCmpCtrl *ccmp, CRPN *rpn);
void SysSymImportsResolve(char *sym, int64_t flags);
CRPN *ParserDumpIR(CRPN *rpn, int64_t indent);
CRPN *ICArgN(CRPN *rpn, int64_t n);
CRPN *ICFwd(CRPN *rpn);
CCmpCtrl *CmpCtrlNew(CLexer *lex);
CMemberLst *MemberFind(char *needle, CHashClass *cls);
CMemberLst *MemberFind(char *needle, CHashClass *cls);
CHashClass *PrsClass(CCmpCtrl *cctrl, int64_t flags);
CHashClass *PrsClass(CCmpCtrl *cctrl, int64_t _flags);
int64_t PrsDecl(CCmpCtrl *ccmp, CHashClass *base, CHashClass *add_to,
                int64_t *is_func_decl, int64_t flags, char *import_name);
int64_t PrsDecl(CCmpCtrl *ccmp, CHashClass *base, CHashClass *add_to,
                int64_t *is_func_decl, int64_t flags, char *import_name);
int64_t PrsTry(CCmpCtrl *cctrl);
int64_t PrsTry(CCmpCtrl *cctrl);
int64_t ParseErr(CCmpCtrl *ctrl, char *fmt, ...);
int64_t ParseErr(CCmpCtrl *ctrl, char *fmt, ...);
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
int64_t PrsKw(CCmpCtrl *ccmp, int64_t);
int64_t PrsKw(CCmpCtrl *ccmp, int64_t kwt);
CCodeMisc *CodeMiscNew(CCmpCtrl *ccmp, int64_t type);
CHashClass *PrsType(CCmpCtrl *ccmp, CHashClass *base, char **name,
                    CHashFun **fun, CArrayDim *dim);
CHashClass *PrsType(CCmpCtrl *ccmp, CHashClass *base, char **name,
                    CHashFun **fun, CArrayDim *dim);
void PrsTests();

void TaskInit(struct CTask *task, void *addr, int64_t stk_sz);
struct CHeapCtrl *HeapCtrlInit(struct CHeapCtrl *ct, struct CTask *task,
                               int64_t is_code_heap);
void TaskExit();
extern _Thread_local struct CTask *HolyFs;
extern _Thread_local struct CTask *Fs;

char *OptPassFinal(CCmpCtrl *cctrl, int64_t *res_sz, char **dbg_info,
                   struct CHeapCtrl *heap);

void LexTests();
void LexerDump(CLexer *lex);
CLexer *LexerNew(char *filename, char *text);
void LexerDel(CLexer *lex);
int64_t Lex(CLexer *lex);
int64_t LexAdvChr(CLexer *lex);
char *LexSrcLink(CLexer *lex, void *task);

uint64_t ToUpper(uint64_t ch);
char *WhichFun(char *fptr);

//
// These are used by optpass
//
#if defined(__x86_64__)
enum {
  RAX = 0,
  RCX = 1,
  RDX = 2,
  RBX = 3,
  RSP = 4,
  RBP = 5,
  RSI = 6,
  RDI = 7,
  R8 = 8,
  R9 = 9,
  R10 = 10,
  R11 = 11,
  R12 = 12,
  R13 = 13,
  R14 = 14,
  R15 = 15,
  RIP = 16
};
#  define AIWNIOS_IREG_START 10
#  define AIWNIOS_IREG_CNT                                                     \
    (15 - AIWNIOS_IREG_START + 1 - 1 +                                         \
     2) //-1 for R13 as it is wierd,+2 for RSI/RDI
#  define AIWNIOS_REG_FP         RBP
#  define AIWNIOS_REG_SP         RSP
#  define AIWNIOS_TMP_IREG_POOP  R8
#  define AIWNIOS_TMP_IREG_POOP2 RCX
#  define AIWNIOS_TMP_IREG_START 0
#  define AIWNIOS_TMP_IREG_CNT   2
#  define AIWNIOS_FREG_START     6
#  define AIWNIOS_TMP_FREG_START 3
#  define AIWNIOS_TMP_FREG_CNT   (5 - 3 + 1)
#  define AIWNIOS_FREG_CNT       (15 - 6 + 1)
#elif (defined(__OpenBSD__) || defined(__linux__) || defined(__FreeBSD__)) &&  \
    (defined(_M_ARM64) || defined(__aarch64__))
#  define AIWNIOS_IREG_START     19
#  define AIWNIOS_IREG_CNT       (27 - 19 + 1)
#  define AIWNIOS_REG_FP         ARM_REG_FP
#  define AIWNIOS_REG_SP         ARM_REG_SP
#  define AIWNIOS_TMP_IREG_POOP  18 // Platform register,I use it as a poo poo
#  define AIWNIOS_TMP_IREG_POOP2 17 // I use it as a second poo poo ALWAYS
#  define AIWNIOS_TMP_IREG_START 8
#  define AIWNIOS_TMP_IREG_CNT   (16 - 8 + 1)
#  define AIWNIOS_FREG_START     8
#  define AIWNIOS_FREG_CNT       (15 - 8 + 1)
#  define AIWNIOS_TMP_FREG_START 16
#  define AIWNIOS_TMP_FREG_CNT   (31 - 16 + 1)
#elif defined(__APPLE__) && (defined(_M_ARM64) || defined(__aarch64__))
#  define AIWNIOS_IREG_START     19
#  define AIWNIOS_IREG_CNT       (27 - 19 + 1)
#  define AIWNIOS_REG_FP         ARM_REG_FP
#  define AIWNIOS_REG_SP         ARM_REG_SP
#  define AIWNIOS_TMP_IREG_POOP  16
#  define AIWNIOS_TMP_IREG_POOP2 17 // I use it as a second poo poo ALWAYS
#  define AIWNIOS_TMP_IREG_START 8
#  define AIWNIOS_TMP_IREG_CNT   (15 - 8 + 1)
#  define AIWNIOS_FREG_START     8
#  define AIWNIOS_FREG_CNT       (15 - 8 + 1)
#  define AIWNIOS_TMP_FREG_START 16
#  define AIWNIOS_TMP_FREG_CNT   (31 - 16 + 1)
#endif
#if defined(__riscv__) || defined(__riscv)
#  define AIWNIOS_IREG_START   0
#  define AIWNIOS_FREG_START   0
#  define AIWNIOS_TMP_IREG_CNT 3
#  define AIWNIOS_TMP_FREG_CNT 3
#  define AIWNIOS_IREG_CNT     (27 - 18 + 1 + 1)
#  define AIWNIOS_FREG_CNT     (27 - 18 + 1 + 2)
#endif

int64_t X86PushReg(char *to, int64_t reg);
int64_t X86MovRegReg(char *to, int64_t a, int64_t b);
int64_t X86AndImm(char *to, int64_t a, int64_t b);
int64_t X86SubImm32(char *to, int64_t a, int64_t b);
int64_t X86MovImm(char *to, int64_t a, int64_t off);
int64_t X86LeaSIB(char *to, int64_t a, int64_t s, int64_t i, int64_t b,
                  int64_t off);
int64_t X86CallReg(char *to, int64_t reg);
int64_t X86AddImm32(char *to, int64_t a, int64_t b);
int64_t X86PopReg(char *to, int64_t reg);
int64_t X86Ret(char *to, int64_t ul);
int64_t X86Leave(char *to, int64_t ul);
int64_t X86JmpReg(char *to, int64_t r);

CCodeCtrl *CodeCtrlPush(CCmpCtrl *ccmp);
void CodeCtrlDel(CCodeCtrl *ctrl);
void CodeCtrlPop(CCmpCtrl *ccmp);
CRPN *__HC_ICAdd_Typecast(CCodeCtrl *cc, int64_t rt, int64_t ptr_cnt);
CRPN *__HC_ICAdd_SubCall(CCodeCtrl *cc, CCodeMisc *cm);
CRPN *__HC_ICAdd_SubProlog(CCodeCtrl *cc);
CRPN *__HC_ICAdd_SubRet(CCodeCtrl *cc);
CRPN *__HC_ICAdd_UnboundedSwitch(CCodeCtrl *cc, CCodeMisc *misc);
CRPN *__HC_ICAdd_PreInc(CCodeCtrl *cc, int64_t amt);
CRPN *__HC_ICAdd_Call(CCodeCtrl *cc, int64_t arity, int64_t rt, int64_t ptrs);
CRPN *__HC_ICAdd_F64(CCodeCtrl *cc, double f);
CRPN *__HC_ICAdd_I64(CCodeCtrl *cc, int64_t integer);
CRPN *__HC_ICAdd_PreDec(CCodeCtrl *cc, int64_t amt);
CRPN *__HC_ICAdd_PostDec(CCodeCtrl *cc, int64_t amt);
CRPN *__HC_ICAdd_PostInc(CCodeCtrl *cc, int64_t amt);
CRPN *__HC_ICAdd_RawBytes(CCodeCtrl *cc, char *bytes, int64_t cnt);
void __HC_ICSetLock(CRPN *l);
CRPN *__HC_ICAdd_RelocUnqiue(CCmpCtrl *, CCodeCtrl *, int64_t *, char *,
                             int64_t, int64_t);

#define HC_IC_BINDINGH(name) CRPN *__##name(CCodeCtrl *cc);
HC_IC_BINDINGH(HC_ICAdd_GetVaArgsPtr)
HC_IC_BINDINGH(HC_ICAdd_Pow)
HC_IC_BINDINGH(HC_ICAdd_Eq)
HC_IC_BINDINGH(HC_ICAdd_Div)
HC_IC_BINDINGH(HC_ICAdd_Sqrt)
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
HC_IC_BINDINGH(HC_ICAdd_Sqr)
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
HC_IC_BINDINGH(HC_ICAdd_Lock)
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
HC_IC_BINDINGH(HC_ICAdd_BT)
HC_IC_BINDINGH(HC_ICAdd_BTC)
HC_IC_BINDINGH(HC_ICAdd_BTS)
HC_IC_BINDINGH(HC_ICAdd_BTR)
HC_IC_BINDINGH(HC_ICAdd_LBTC)
HC_IC_BINDINGH(HC_ICAdd_LBTS)
HC_IC_BINDINGH(HC_ICAdd_LBTR)
HC_IC_BINDINGH(HC_ICAdd_Max_I64)
HC_IC_BINDINGH(HC_ICAdd_Max_U64)
HC_IC_BINDINGH(HC_ICAdd_Min_I64)
HC_IC_BINDINGH(HC_ICAdd_Min_U64)
HC_IC_BINDINGH(HC_ICAdd_Min_F64)
HC_IC_BINDINGH(HC_ICAdd_Max_F64)
HC_IC_BINDINGH(HC_ICAdd_SIGN_I64)
HC_IC_BINDINGH(HC_ICAdd_SQR_I64)
HC_IC_BINDINGH(HC_ICAdd_SQR_U64)
HC_IC_BINDINGH(HC_ICAdd_SQR)
HC_IC_BINDINGH(HC_ICAdd_ABS)
HC_IC_BINDINGH(HC_ICAdd_SQRT)
HC_IC_BINDINGH(HC_ICAdd_SIN)
HC_IC_BINDINGH(HC_ICAdd_COS)
HC_IC_BINDINGH(HC_ICAdd_TAN)
HC_IC_BINDINGH(HC_ICAdd_ATAN)
HC_IC_BINDINGH(HC_ICAdd_Fs)
HC_IC_BINDINGH(HC_ICAdd_Gs)
HC_IC_BINDINGH(HC_ICAdd_ToBool)

CRPN *__HC_ICAdd_FReg(CCodeCtrl *cc, int64_t r);
CRPN *__HC_ICAdd_IReg(CCodeCtrl *cc, int64_t r, int64_t rt, int64_t ptr_cnt);
CRPN *__HC_ICAdd_Frame(CCodeCtrl *cc, int64_t off, int64_t rt, int64_t ptr_cnt);
CCodeMisc *__HC_CodeMiscJmpTableNew(CCmpCtrl *ccmp, CCodeMisc *labels,
                                    void **table_address, int64_t hi);
CCodeMisc *__HC_CodeMiscStrNew(CCmpCtrl *ccmp, char *str, int64_t sz);
CCodeMisc *__HC_CodeMiscLabelNew(CCmpCtrl *ccmp, void **);
CCmpCtrl *__HC_CmpCtrlNew();
CCodeCtrl *__HC_CodeCtrlPush(CCmpCtrl *ccmp);
CCodeCtrl *__HC_CodeCtrlPop(CCmpCtrl *ccmp);
char *__HC_Compile(CCmpCtrl *ccmp, int64_t *sz, char **dbg_info,
                   struct CHeapCtrl *heap);
CRPN *__HC_ICAdd_Goto(CCodeCtrl *cc, CCodeMisc *cm);
CRPN *__HC_ICAdd_GotoIf(CCodeCtrl *cc, CCodeMisc *cm);
CRPN *__HC_ICAdd_Str(CCodeCtrl *cc, CCodeMisc *cm);
CRPN *__HC_ICAdd_Label(CCodeCtrl *cc, CCodeMisc *misc);
CRPN *__HC_ICAdd_Arg(CCodeCtrl *cc, int64_t arg);
CRPN *__HC_ICAdd_SetFrameSize(CCodeCtrl *cc, int64_t arg);
CRPN *__HC_ICAdd_Reloc(CCmpCtrl *cmpc, CCodeCtrl *cc, int64_t *pat_addr,
                       char *sym, int64_t rt, int64_t ptrs);
int64_t IsCmdLineMode();
int64_t IsCmdLineMode2();
CCodeMisc *AddRelocMisc(CCmpCtrl *cctrl, char *name);
CRPN *__HC_ICAdd_Deref(CCodeCtrl *cc, int64_t rt, int64_t ptr_cnt);
void __HC_ICSetLine(CRPN *r, int64_t ln);
CRPN *__HC_ICAdd_Switch(CCodeCtrl *cc, CCodeMisc *misc, CCodeMisc *dft);
CRPN *__HC_ICAdd_Vargs(CCodeCtrl *cc, int64_t arity);
CRPN *__HC_ICAdd_StaticData(CCmpCtrl *cmp, CCodeCtrl *cc, int64_t at, char *d,
                            int64_t len);
CRPN *__HC_ICAdd_StaticRef(CCodeCtrl *cc, int64_t off, int64_t rt,
                           int64_t ptrs);
CRPN *__HC_ICAdd_SetStaticsSize(CCodeCtrl *cc, int64_t len);
char *Load(char *fbuf, int64_t size);
CRPN *__HC_ICAdd_ShortAddr(CCmpCtrl *, CCodeCtrl *cc, char *name,
                           CCodeMisc *ptr);

void ImportSymbolsToHolyC(void (*cb)(char *name, void *addr));
CCmpCtrl *CmpCtrlDel(CCmpCtrl *d);
;

void __HC_SetAOTRelocBeforeRIP(CRPN *r, int64_t off);
int64_t __HC_CodeMiscIsUsed(CCodeMisc *cm);
extern CCodeMiscRef *CodeMiscAddRef(CCodeMisc *misc, int32_t *addr);
extern void __HC_CodeMiscInterateThroughRefs(
    CCodeMisc *cm, void (*fptr)(void *addr, void *user_data), void *user_data);

extern void *GenFFIBinding(void *fptr, int64_t arity);
extern void *GenFFIBindingNaked(void *fptr, int64_t arity);
extern void PrsBindCSymbolNaked(char *name, void *ptr, int64_t arity);
extern void PrsBindCSymbol(char *name, void *ptr, int64_t arity);
void __HC_CmpCtrl_SetAOT(CCmpCtrl *cc);

CRPN *__HC_ICAdd_GetVargsPtr(CCodeCtrl *cc);

void CacheRPNArgs(CCmpCtrl *cctrl);
extern int64_t DolDocDumpIR(char *to, int64_t, CRPN *);
