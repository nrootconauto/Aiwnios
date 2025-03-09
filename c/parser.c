#include <stddef.h>
#include <stdlib.h>
#if defined(__APPLE__)
#  include <libkern/OSCacheControl.h>
#endif
extern void DoNothing();
#include "aiwn_asm.h"
#include "aiwn_except.h"
#include "aiwn_fs.h"
#include "aiwn_hash.h"
#include "aiwn_lexparser.h"
#include "aiwn_mem.h"
#include "aiwn_multic.h"
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
CCodeCtrl *CodeCtrlPush(CCmpCtrl *ccmp) {
  CCodeCtrl *cctrl;
  *(cctrl = A_CALLOC(sizeof(CCodeCtrl), ccmp->hc)) = (CCodeCtrl){
      .hc = ccmp->hc,
      .next = ccmp->code_ctrl,
      .ir_code = A_CALLOC(sizeof(CRPN), cctrl->hc),
      .code_misc = A_CALLOC(sizeof(CQue), cctrl->hc),
  };
  ((CRPN *)cctrl->ir_code)->type = IC_NOP;
  QueInit(cctrl->ir_code);
  QueInit(cctrl->code_misc);
  ccmp->code_ctrl = cctrl;
  return cctrl;
}

CCodeCtrl *CodeCtrlPopNoFree(CCmpCtrl *cc) {
  CCodeCtrl *ctrl = cc->code_ctrl;
  cc->code_ctrl = ctrl->next;
  return ctrl;
}

void CodeCtrlAppend(CCmpCtrl *ccmp, CCodeCtrl *ct) {
  CQue *head = ccmp->code_ctrl->ir_code, *next = head->next;
  if (ct->ir_code->next != ct->ir_code->last) {
    head->next = ct->ir_code->next;
    next->last = ct->ir_code->last;
    ct->ir_code->last->next = next;
    ct->ir_code->next->last = head;
  }

  head = ccmp->code_ctrl->code_misc;
  next = head->next;
  if (ct->code_misc->next != ct->code_misc->last) {
    head->next = ct->code_misc->next;
    next->last = ct->code_misc->last;
    ct->code_misc->last->next = next;
    ct->code_misc->next->last = head;
  }
  // Transfer "ownership"  of the code-miscs and CRPN's
  QueInit(ct->code_misc);
  QueInit(ct->ir_code);
}
static void ArrayDimDel(CArrayDim *dim) {
  if (dim->next)
    ArrayDimDel(dim->next);
  A_FREE(dim);
}
static void HashFunDel(CHashFun *fun);
CHashClass *PrsType(CCmpCtrl *ccmp, CHashClass *base, char **name,
                    CHashFun **fun, CArrayDim *dim);
static void MemberLstDel(CMemberLst *lst) {
  A_FREE(lst->str);
  if (lst->next)
    MemberLstDel(lst->next);
  if (lst->fun_ptr)
    HashFunDel(lst->fun_ptr);
  // Dont free ->dim as it is not MAlloc'ed(it is a member)
  if (lst->dim.next)
    ArrayDimDel(lst->dim.next);
  A_FREE(lst);
}

static void HashFunDel(CHashFun *fun) {
  A_FREE(fun->fun_ptr);
  if (fun->base.members_lst)
    MemberLstDel(fun->base.members_lst);
  A_FREE(fun->import_name);
  A_FREE(fun->base.base.str);
  A_FREE(fun);
}
void CodeCtrlDel(CCodeCtrl *ctrl) {
  CCodeMisc *misc;
  for (misc = ctrl->code_misc->next; misc != ctrl->code_misc;
       misc = misc->base.next) {
    switch (misc->type) {
      break;
    case CMT_STRING:
      A_FREE(misc->str);
      break;
    case CMT_RELOC_U64:
    case CMT_LABEL:
      A_FREE(misc->str);
      break;
    case CMT_JMP_TAB:
      A_FREE(misc->jmp_tab);
      break;
    case CMT_STATIC_DATA:
      A_FREE(misc->str);
    }
  }
  QueDel(ctrl->ir_code);
  QueDel(ctrl->code_misc);
  A_FREE(ctrl);
}

CCodeMisc *CodeMiscNew(CCmpCtrl *ccmp, int64_t type) {
  CCodeMisc *misc = A_CALLOC(sizeof(CCodeMisc), ccmp->hc);
  misc->type = type;
  QueIns(misc, ccmp->code_ctrl->code_misc->last);
  return misc;
}

void CodeCtrlPop(CCmpCtrl *ccmp) {
  CCodeCtrl *next = ccmp->code_ctrl->next, *c = ccmp->code_ctrl;
  CodeCtrlDel(c);
  ccmp->code_ctrl = next;
}

int64_t PrsKw(CCmpCtrl *ccmp, int64_t);
int64_t PrsStmt(CCmpCtrl *ccmp);
int64_t PrsIf(CCmpCtrl *ccmp);     // YES
int64_t PrsDo(CCmpCtrl *ccmp);     // YES
int64_t PrsWhile(CCmpCtrl *ccmp);  // YES
int64_t PrsFor(CCmpCtrl *ccmp);    // YES
int64_t PrsSwitch(CCmpCtrl *ccmp); // YES
int64_t PrsScope(CCmpCtrl *ccmp);
int64_t PrsLabel(CCmpCtrl *ccmp);
int64_t PrsGoto(CCmpCtrl *ccmp);
int64_t PrsReturn(CCmpCtrl *ccmp);
int64_t ParseErr(CCmpCtrl *ctrl, char *fmt, ...);
int64_t PrsTry(CCmpCtrl *cctrl); // YES
int64_t PrsDecl(CCmpCtrl *ccmp, CHashClass *base, CHashClass *add_to,
                int64_t *is_func_decl, int64_t flags, char *import_name);
int64_t PrsSwitch(CCmpCtrl *cctrl);
CHashClass *PrsClass(CCmpCtrl *cctrl, int64_t flags);
CMemberLst *MemberFind(char *needle, CHashClass *cls);
CCmpCtrl *CmpCtrlDel(CCmpCtrl *d) {
  HeapCtrlDel(d->hc);
  A_FREE(d);
}
CCmpCtrl *CmpCtrlNew(CLexer *lex) {
  int64_t idx, idx2;
  CHashClass *cls;
  CCmpCtrl *ccmp;
  *(ccmp = A_MALLOC(sizeof(CCmpCtrl), NULL)) = (CCmpCtrl){
      .lex = lex, .hc = HeapCtrlInit(NULL, Fs, 0), .final_hc = Fs->code_heap};
  struct {
    char *name;
    int64_t rt;
    int64_t sz;
  } static raw_types[] = {
      {"U0", RT_U0, 0},     {"U8i", RT_U8i, 1},   {"I8i", RT_I8i, 1},
      {"U16i", RT_U16i, 2}, {"I16i", RT_I16i, 2}, {"U32i", RT_U32i, 4},
      {"I32i", RT_I32i, 4}, {"U64i", RT_U64i, 8}, {"I64i", RT_I64i, 8},
      {"F64", RT_F64, 8},
  };
  for (idx = 0; idx != sizeof(raw_types) / sizeof(*raw_types); idx++) {
    if (!HashFind(raw_types[idx].name, Fs->hash_table, HTT_CLASS, 1)) {
      *(cls = A_CALLOC(sizeof(CHashClass) * STAR_CNT, NULL)) = (CHashClass){
          .base =
              {
                  .type = HTT_CLASS,
              },
          .sz = raw_types[idx].sz,
      };
      for (idx2 = 0; idx2 != STAR_CNT; idx2++) {
        cls[idx2].base.str = A_STRDUP(raw_types[idx].name, NULL);
        if (!idx2) {
          cls[idx2].raw_type = raw_types[idx].rt;
          cls[idx2].sz = raw_types[idx].sz;
        } else {
          cls[idx2].raw_type = RT_PTR;
          cls[idx2].sz = sizeof(void *);
        }
        cls[idx2].ptr_star_cnt = idx2;
        cls[idx2].use_cnt++;
      }
      HashAdd(cls, Fs->hash_table);
      *(cls = A_CALLOC(sizeof(CHashClass) * STAR_CNT, NULL)) = (CHashClass){
          .base =
              {
                  .type = HTT_CLASS,
              },
          .sz = raw_types[idx].sz,
      };
      for (idx2 = 0; idx2 != STAR_CNT; idx2++) {
        cls[idx2].base.str = A_STRDUP(raw_types[idx].name, NULL);
        if (!idx2) {
          cls[idx2].raw_type = raw_types[idx].rt;
          cls[idx2].sz = raw_types[idx].sz;
        } else {
          cls[idx2].raw_type = RT_PTR;
          cls[idx2].sz = sizeof(void *);
        }
        cls[idx2].ptr_star_cnt = idx2;
        cls[idx2].use_cnt++;
      }
      HashAdd(cls, Fs->hash_table);
    }
  }
  return ccmp;
}
CRPN *ICFwd(CRPN *rpn) {
  CRPN *orig_rpn = rpn;
  int64_t idx;
  if (rpn->ic_fwd)
    return rpn->ic_fwd;
  switch (rpn->type) {
    break;
  case IC_GS:
  case IC_FS:
    goto unop;
  case IC_LOCK:
  case IC_RAW_BYTES:
  case __IC_STATICS_SIZE:
  case __IC_SET_STATIC_DATA:
  case __IC_STATIC_REF:
    return rpn->base.next;
    break;
  case IC_SHORT_ADDR:
  case IC_RELOC:
    return rpn->base.next;
    break;
  case IC_GET_VARGS_PTR:
  case IC_TO_F64:
  case IC_TO_I64:
  case IC_TO_BOOL:
    goto unop;
    break;
  case IC_GOTO_IF:
    goto unop;
    break;
  case IC_STATIC:
    return rpn->base.next;
    break;
  case IC_TYPECAST:
    goto unop;
    break;
  case IC_COMMA:
  case IC_MAX_F64:
  case IC_MAX_U64:
  case IC_MAX_I64:
  case IC_MIN_F64:
  case IC_MIN_U64:
  case IC_MIN_I64:
    goto binop;
    break;
  case __IC_VARGS:
    rpn = rpn->base.next;
    // Args
    for (idx = 0; idx != orig_rpn->length; idx++)
      rpn = ICFwd(rpn);
    return rpn;
    break;
  case __IC_CALL:
  case IC_CALL:
    rpn = rpn->base.next;
    // Args
    for (idx = 0; idx != orig_rpn->length; idx++)
      rpn = ICFwd(rpn);
    // Function
    return rpn = ICFwd(rpn);
    break;
  case IC_BASE_PTR:
    return rpn->base.next;
    break;
  case IC_IREG:
    return rpn->base.next;
    break;
  case IC_FREG:
    return rpn->base.next;
    break;
  case IC_LOAD:
    goto binop;
    break;
  case IC_STORE:
    goto binop;
    break;
  case IC_RET:
    return ICFwd(rpn->base.next);
    break;
  case IC_LABEL:
    return rpn->base.next;
    break;
  case IC_GOTO:
    return rpn->base.next;
    break;
  case IC_GLOBAL:
    return rpn->base.next;
    break;
  case IC_LOCAL:
    return rpn->base.next;
    break;
  case IC_NOP:
    return rpn->base.next;
    break;
  case IC_BOUNDED_SWITCH:
    goto swit;
    break;
  case IC_UNBOUNDED_SWITCH:
  swit:
    return ICFwd(rpn->base.next);
    break;
  case IC_SUB_CALL:
    return rpn->base.next;
    break;
  case IC_SUB_PROLOG:
    return rpn->base.next;
    break;
  case IC_SUB_RET:
    return rpn->base.next;
    break;
  case IC_PAREN:
    abort();
    break;
  case IC_NEG:
  case IC_SQRT:
  unop:
    return ICFwd(rpn->base.next);
    break;
  case IC_SQR:
  case IC_POS:
    goto unop;
    break;
  case IC_NAME:
    return rpn->base.next;
    break;
  case IC_STR:
    return rpn->base.next;
    break;
  case IC_CHR:
    return rpn->base.next;
    break;
  case IC_POW:
  binop:
    return ICFwd(ICFwd(rpn->base.next));
    break;
  case IC_ADD:
    goto binop;
    break;
  case IC_BT:
  case IC_BTR:
  case IC_BTS:
  case IC_BTC:
  case IC_LBTR:
  case IC_LBTS:
  case IC_LBTC:
  case IC_EQ:
    goto binop;
    break;
  case IC_SUB:
    goto binop;
    break;
  case IC_DIV:
    goto binop;
    break;
  case IC_MUL:
    goto binop;
    break;
  case __IC_ARG:
  case IC_DEREF:
    goto unop;
    break;
  case IC_AND:
    goto binop;
    break;
  case IC_ADDR_OF:
    goto unop;
    break;
  case IC_XOR:
    goto binop;
    break;
  case IC_MOD:
    goto binop;
    break;
  case IC_OR:
    goto binop;
    break;
  case IC_LT:
    goto binop;
    break;
  case IC_GT:
    goto binop;
    break;
  case IC_LNOT:
    goto unop;
    break;
  case IC_BNOT:
    goto unop;
    break;
  case IC_POST_INC:
    goto unop;
    break;
  case IC_POST_DEC:
    goto unop;
    break;
  case IC_PRE_INC:
    goto unop;
    break;
  case IC_PRE_DEC:
    goto unop;
    break;
  case IC_AND_AND:
    goto binop;
    break;
  case IC_OR_OR:
    goto binop;
    break;
  case IC_XOR_XOR:
    goto binop;
    break;
  case IC_EQ_EQ:
    goto binop;
    break;
  case IC_NE:
    goto binop;
    break;
  case IC_LE:
    goto binop;
    break;
  case IC_GE:
    goto binop;
    break;
  case IC_LSH:
    goto binop;
    break;
  case IC_RSH:
    goto binop;
    break;
  case IC_ADD_EQ:
    goto binop;
    break;
  case IC_SUB_EQ:
    goto binop;
    break;
  case IC_MUL_EQ:
    goto binop;
    break;
  case IC_DIV_EQ:
    goto binop;
    break;
  case IC_LSH_EQ:
    goto binop;
    break;
  case IC_RSH_EQ:
    goto binop;
    break;
  case IC_AND_EQ:
    goto binop;
    break;
  case IC_OR_EQ:
    goto binop;
    break;
  case IC_XOR_EQ:
    goto binop;
    break;
  case IC_ARROW:
    goto unop;
    break;
  case IC_DOT:
    goto unop;
    break;
  case IC_MOD_EQ:
    goto binop;
    break;
  case IC_I64:
    return rpn->base.next;
    break;
  case __IC_SET_FRAME_SIZE:
  case IC_F64:
    return rpn->base.next;
    break;
  case IC_ARRAY_ACC:
    goto binop;
  }
  abort();
}

static char *PrsString(CCmpCtrl *ccmp, int64_t *sz) {
  char *ret = NULL, *tmp;
  int64_t len = 0;
  while (ccmp->lex->cur_tok == TK_STR) {
    len += ccmp->lex->str_len;
    tmp = A_CALLOC(len + 1, NULL);
    if (ret)
      memcpy(tmp, ret, len - ccmp->lex->str_len);
    A_FREE(ret);
    ret = tmp;
    memcpy(ret + (len - ccmp->lex->str_len), ccmp->lex->string,
           ccmp->lex->str_len);
    Lex(ccmp->lex);
    ret[len] = 0;
  }
  if (sz)
    *sz = len + 1;
  return ret;
}

CRPN *ICArgN(CRPN *rpn, int64_t n) {
  if (n == 1 && rpn->tree1)
    return rpn->tree1;
  if (n == 2 && rpn->tree2)
    return rpn->tree2;
  rpn = rpn->base.next;
  while (--n >= 0)
    rpn = ICFwd(rpn);
  return rpn;
}
static void DumpRPNDim(CArrayDim *dim) {
  if (dim->next)
    DumpRPNDim(dim->next);
  printf("[%ld]", dim->cnt);
}
static void DumpRPNType(CRPN *rpn) {
  int64_t stars;
  if (rpn->ic_class) {
    stars = rpn->ic_class->ptr_star_cnt;
    if (rpn->ic_class->raw_type != RT_FUNC)
      printf("%s", rpn->ic_class[-stars].base.str);
    while (--stars >= 0)
      printf("*");
  }
  if (rpn->ic_dim)
    DumpRPNDim(rpn->ic_dim);
  switch (rpn->raw_type) {
    break;
  case RT_UNDEF:
    break;
  case RT_F64:
    printf("(RT_F64)");
    break;
  case RT_FUNC:
    printf("(RT_FUNC)");
    break;
  case RT_U0:
    printf("(RT_U0)");
    break;
  case RT_I8i:
    printf("(RT_I8i)");
    break;
  case RT_I16i:
    printf("(RT_I16i)");
    break;
  case RT_I32i:
    printf("(RT_I32i)");
    break;
  case RT_I64i:
    printf("(RT_I64i)");
    break;
  case RT_U8i:
    printf("(RT_U8i)");
    break;
  case RT_U16i:
    printf("(RT_U16i)");
    break;
  case RT_U32i:
    printf("(RT_U32i)");
    break;
  case RT_U64i:
    printf("(RT_U64i)");
    break;
  case RT_PTR:
    printf("(RT_PTR)");
  }
}

CRPN *ParserDumpIR(CRPN *rpn, int64_t indent) {
  int64_t idx;
  CRPN *orig_rpn = rpn;
#define INDENT                                                                 \
  for (idx = 0; idx != indent; idx++)                                          \
    printf("  ");
  INDENT;
  switch (rpn->type) {
  case IC_MAX_F64:
    printf("MAX_F64:\n");
    goto binop;
  case IC_MAX_I64:
    printf("MAX_I64:\n");
    goto binop;
  case IC_MAX_U64:
    printf("MAX_U64:\n");
    goto binop;
  case IC_MIN_F64:
    printf("MIN_F64:\n");
    goto binop;
  case IC_MIN_I64:
    printf("MIN_I64:\n");
    goto binop;
  case IC_MIN_U64:
    printf("MIN_U64:\n");
    goto binop;
  case IC_LOCK:
    printf("IC_LOCK:\n");
    ParserDumpIR(rpn->base.next, indent + 1);
    goto ret;
  case IC_RAW_BYTES:
    printf("RAW_BYTES:%ld\n", rpn->length);
    goto ret;
  case __IC_STATICS_SIZE:
    printf("STATICS_SZ:%ld\n", rpn->integer);
    goto ret;
    break;
  case __IC_SET_STATIC_DATA:
    printf("SET_STATIC_DATA:%ld(%ld)\n", rpn->code_misc->integer,
           rpn->code_misc->str_len);
    goto ret;
    break;
  case __IC_STATIC_REF:
    printf("SET_STATIC_REF:(%ld)", rpn->integer);
    DumpRPNType(rpn);
    printf("\n");
    goto ret;
  case IC_SHORT_ADDR:
    printf("SHORT_ADDR:%s\n", rpn->code_misc->str);
    return ICFwd(rpn);
  case IC_RELOC:
    printf("RELOC:%s\n", rpn->code_misc->str);
    return ICFwd(rpn);
  case IC_TO_BOOL:
    printf("TO_BOOL\n");
    goto unop;
  case IC_GET_VARGS_PTR:
    printf("GET_VARGS_PTR:\n");
    return ParserDumpIR(rpn->base.next, indent + 1);
  case __IC_VARGS:
    printf("VARGS:%d\n", rpn->length);
    rpn = rpn->base.next;
    for (idx = 0; idx != orig_rpn->length; idx++) {
      ParserDumpIR(rpn, indent + 1);
      rpn = ICFwd(rpn);
    }
    return rpn;
  case IC_GOTO_IF:
    printf("GOTO_IF:%p\n", rpn->code_misc);
    goto unop;
    break;
  case __IC_SET_FRAME_SIZE:
    printf("SET_FRAME_SIZE: %p\n", rpn->integer);
    return rpn->base.next;
    break;
  case __IC_ARG:
    printf("ARG %p\n", rpn->integer);
    goto unop;
    break;
  case IC_STATIC:
    printf("STATIC %p ", rpn->integer);
    DumpRPNType(rpn);
    printf("\n");
    goto ret;
  case IC_TYPECAST:
    printf("TYPECAST");
    DumpRPNType(rpn);
    printf("\n");
    ParserDumpIR(rpn->base.next, indent + 1);
    goto ret;
    break;
  case IC_SUB_RET:
    printf("IC_SUB_RET\n");
    goto ret;
    break;
  case IC_SUB_PROLOG:
    printf("IC_SUB_PROLOG\n");
    goto ret;
    break;
  case IC_SUB_CALL:
    printf("IC_SUB_CALL\n");
    goto ret;
    break;
  case IC_UNBOUNDED_SWITCH:
    printf("SWITCH[]");
  swit:
    ParserDumpIR(ICArgN(rpn, 0), indent + 1);
    goto ret;
    break;
  case IC_TO_I64:
    printf("TOI64");
    goto unop;
  case IC_TO_F64:
    printf("TOF64");
    goto unop;
    break;
  case IC_BOUNDED_SWITCH:
    printf("SWITCH()\n");
    goto swit;
    break;
  case IC_GS:
    printf("GS:%p\n", rpn->integer);
    goto unop;
  case IC_FS:
    printf("FS:%p\n", rpn->integer);
    goto unop;
  case __IC_CALL:
  case IC_CALL:
    rpn = rpn->base.next;
    printf("CALL");
    DumpRPNType(orig_rpn);
    printf("\n");
    for (idx = 0; idx != orig_rpn->length; idx++) {
      ParserDumpIR(rpn, indent + 1);
      rpn = ICFwd(rpn);
    }
    ParserDumpIR(rpn, indent + 1);
    goto ret;
    break;
  case IC_COMMA:
    printf(",");
    DumpRPNType(orig_rpn);
    goto binop;
    break;
  case IC_BT:
    printf("BT");
    goto binop;
  case IC_BTR:
    printf("BTR");
    goto binop;
  case IC_BTS:
    printf("BTS");
    goto binop;
  case IC_BTC:
    printf("BTC");
    goto binop;
  case IC_LBTR:
    printf("LBTR");
    goto binop;
  case IC_LBTS:
    printf("LBTS");
    goto binop;
  case IC_LBTC:
    printf("LBTC");
    goto binop;
  case IC_BASE_PTR:
    printf("FRAME(%ld)", rpn->integer);
    DumpRPNType(orig_rpn);
    printf("\n");
    goto ret;
    break;
  case IC_IREG:
    printf("IREG(%ld)", rpn->integer);
    DumpRPNType(orig_rpn);
    printf("\n");
    goto ret;
    break;
  case IC_FREG:
    printf("FREG(%ld)", rpn->integer);
    DumpRPNType(orig_rpn);
    printf("\n");
    goto ret;
    break;
  case IC_LOAD:
    printf("IC_LOAD\n");
    goto binop;
    break;
  case IC_STORE:
    printf("IC_STORE\n");
    goto binop;
    break;
  case IC_RET:
    printf("RETURN\n");
    ParserDumpIR(rpn->base.next, indent + 1);
    goto ret;
    break;
  case IC_GOTO:
    if (rpn->code_misc)
      printf("GOTO:%s\n", rpn->code_misc->str);
    else
      printf("GOTO??\n");
    goto ret;
    break;
  case IC_LABEL:
    printf("LABEL:%p\n", rpn->code_misc);
    goto ret;
    break;
  case IC_GLOBAL:
    printf("GLOBAL:%s", rpn->global_var->base.str);
    DumpRPNType(rpn);
    printf("\n");
    goto ret;
    break;
  case IC_LOCAL:
    printf("LOCAL:%s", rpn->local_mem->str);
    DumpRPNType(rpn);
    printf("\n");
    goto ret;
    break;
  case IC_NOP:
    printf("NOP\n");
    goto ret;
    break;
  case IC_NAME:
    printf("NAME:%s\n", rpn->string);
    goto ret;
    break;
  case IC_POW:
    printf("`");
    goto binop;
    break;
  case IC_ADD:
    printf("+");
    goto binop;
    break;
  case IC_EQ:
    printf("=");
    goto binop;
    break;
  case IC_SUB:
    printf("-");
    goto binop;
    break;
  case IC_DIV:
    printf("/");
    goto binop;
    break;
  case IC_MUL:
    printf("*");
    goto binop;
    break;
  case IC_DEREF:
    printf("DEREF");
    goto unop;
    break;
  case IC_AND:
    printf("&");
    goto binop;
    break;
  case IC_ADDR_OF:
    printf("ADDR_OF");
    goto unop;
    break;
  case IC_XOR:
    printf("^");
    goto binop;
    break;
  case IC_MOD:
    printf("%%");
    goto binop;
    break;
  case IC_OR:
    printf("|");
    goto binop;
    break;
  case IC_LT:
    printf("<");
    goto binop;
    break;
  case IC_GT:
    printf(">");
    goto binop;
    break;
  case IC_LNOT:
    printf("!");
    goto unop;
    break;
  case IC_BNOT:
    printf("~");
    goto unop;
    break;
  case IC_POST_INC:
    printf("x++");
    goto unop;
    break;
  case IC_POST_DEC:
    printf("x--");
    goto unop;
    break;
  case IC_PRE_INC:
    printf("++x");
    goto unop;
    break;
  case IC_PRE_DEC:
    printf("--x");
    goto unop;
    break;
  case IC_AND_AND:
    printf("&&");
    goto binop;
    break;
  case IC_OR_OR:
    printf("||");
    goto binop;
    break;
  case IC_XOR_XOR:
    printf("^^");
    goto binop;
    break;
  case IC_EQ_EQ:
    printf("==");
    goto binop;
    break;
  case IC_NE:
    printf("!=");
    goto binop;
    break;
  case IC_LE:
    printf("<=");
    goto binop;
    break;
  case IC_GE:
    printf(">=");
    goto binop;
    break;
  case IC_LSH:
    printf("<<");
    goto binop;
    break;
  case IC_RSH:
    printf(">>");
    goto binop;
    break;
  case IC_ADD_EQ:
    printf("+=");
    goto binop;
    break;
  case IC_SUB_EQ:
    printf("-=");
    goto binop;
    break;
  case IC_MUL_EQ:
    printf("*=");
    goto binop;
    break;
  case IC_DIV_EQ:
    printf("/=");
    goto binop;
    break;
  case IC_LSH_EQ:
    printf("<<=");
    goto binop;
    break;
  case IC_RSH_EQ:
    printf(">>=");
    goto binop;
    break;
  case IC_AND_EQ:
    printf("&=");
    goto binop;
    break;
  case IC_OR_EQ:
    printf("|=");
    goto binop;
    break;
  case IC_XOR_EQ:
    printf("^=");
    goto binop;
    break;
  case IC_DOT:
    printf(".%s", rpn->local_mem->str);
  member:
    printf(" ");
    DumpRPNType(rpn);
    ParserDumpIR(rpn->base.next, indent + 1);
    goto ret;
  case IC_ARROW:
    printf("->%s", rpn->local_mem->str);
    goto member;
  case IC_MOD_EQ:
    printf("%=");
    goto binop;
  case IC_I64:
    printf("I64:%ld\n", rpn->integer);
    goto ret;
  case IC_STR:
    printf("STR:%s\n", rpn->code_misc->str);
    goto ret;
  case IC_CHR:
    printf("CHR:%s\n", &rpn->integer);
    goto ret;
  case IC_F64:
    printf("F64:%lf\n", rpn->flt);
    goto ret;
  case IC_ARRAY_ACC:
    printf("[]");
  binop:
    printf(" ");
    DumpRPNType(rpn);
    printf("\n");
    rpn = ParserDumpIR(rpn->base.next, indent + 1);
    rpn = ParserDumpIR(rpn, indent + 1);
    goto ret;
    break;
  case IC_SQRT:
    printf("SQRT");
    goto unop;
    break;
  case IC_NEG:
    printf("-");
    goto unop;
  case IC_POS:
    printf("+");
  unop:
    DumpRPNType(rpn);
    printf("\n");
    ParserDumpIR(rpn->base.next, indent + 1);
    goto ret;
  default:
    printf("%ld\n", rpn->type);
    abort();
  }
  //???
  __builtin_unreachable();
ret:
  return ICFwd(orig_rpn);
}
static int64_t AddBytes(char *to, int64_t len, char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  if (to)
    len += vsnprintf(to + len, 1 << 16, fmt, va);
  else
    len += vsnprintf(NULL, 0, fmt, va);
  va_end(va);
  return len;
}
static int64_t DolDocDumpDim(char *to, int64_t len, CArrayDim *dim) {
  if (dim->next)
    len = DolDocDumpDim(to, len, dim->next);
  len = AddBytes(to, len, "[%ld]", dim->cnt);
  return len;
}

static int64_t DolDocDumpType(char *to, int64_t len, CRPN *rpn) {
  int64_t stars;
  len = AddBytes(to, len, "$LTBLUE$");
  if (rpn->ic_class) {
    stars = rpn->ic_class->ptr_star_cnt;
    if (rpn->ic_class->raw_type != RT_FUNC)
      len = AddBytes(to, len, "%s", rpn->ic_class[-stars].base.str);
    while (--stars >= 0)
      len = AddBytes(to, len, "*");
  }
  if (rpn->ic_dim)
    len = DolDocDumpDim(len, to, rpn->ic_dim);
  switch (rpn->raw_type) {
    break;
  case RT_UNDEF:
    break;
  case RT_F64:
    len = AddBytes(to, len, "(RT_F64)");
    break;
  case RT_FUNC:
    len = AddBytes(to, len, "(RT_FUNC)");
    break;
  case RT_U0:
    len = AddBytes(to, len, "(RT_U0)");
    break;
  case RT_I8i:
    len = AddBytes(to, len, "(RT_I8i)");
    break;
  case RT_I16i:
    len = AddBytes(to, len, "(RT_I16i)");
    break;
  case RT_I32i:
    len = AddBytes(to, len, "(RT_I32i)");
    break;
  case RT_I64i:
    len = AddBytes(to, len, "(RT_I64i)");
    break;
  case RT_U8i:
    len = AddBytes(to, len, "(RT_U8i)");
    break;
  case RT_U16i:
    len = AddBytes(to, len, "(RT_U16i)");
    break;
  case RT_U32i:
    len = AddBytes(to, len, "(RT_U32i)");
    break;
  case RT_U64i:
    len = AddBytes(to, len, "(RT_U64i)");
    break;
  case RT_PTR:
    len = AddBytes(to, len, "(RT_PTR)");
  }
  len = AddBytes(to, len, "$FD$\n");
  return len;
}
static int64_t DolDocUnMacro(char *to, int64_t len, CRPN *rpn) {
  if (rpn->start_ptr < rpn->end_ptr) {
    len =
        AddBytes(to, len, "$MA,\"Un\",LM=\"Un(0x%llx,0x%llx);\\n\"$\n",
                 rpn->start_ptr, (char *)rpn->end_ptr - (char *)rpn->start_ptr);
  }
  return len;
}
int64_t DolDocDumpIR(char *to, int64_t len, CRPN *rpn) {
  int64_t idx;
  CRPN *orig_rpn = rpn;
  switch (rpn->type) {
  case IC_MAX_F64:
    len = AddBytes(to, len, "$PURPLE$MAX_F64\"$\n");
    goto binop;
  case IC_MAX_I64:
    len = AddBytes(to, len, "$PURPLE$MAX_I64:$FD$\n");
    goto binop;
  case IC_MAX_U64:
    len = AddBytes(to, len, "$PURPLE$MAX_U64:$FD$\n");
    goto binop;
  case IC_MIN_F64:
    len = AddBytes(to, len, "$PURPLE$MIN_F64:$FD$\n");
    goto binop;
  case IC_MIN_I64:
    len = AddBytes(to, len, "$PURPLE$MIN_I64:$FD$\n");
    goto binop;
  case IC_MIN_U64:
    len = AddBytes(to, len, "$PURPLE$MIN_I64:$FD$\n");
    goto binop;
  case IC_LOCK:
    len = AddBytes(to, len, "$PURPLE$LOCK:$FD$\t");
    goto unop;
  case IC_RAW_BYTES:
    len = AddBytes(to, len, "$PURPLE$RAW_BYTES:(%p)$FD$\n", rpn->length);
    len = DolDocUnMacro(to, len, rpn);
  bytes:
    len = AddBytes(to, len, "$ID,2$\n");
    for (idx = 0; idx != rpn->length; idx++) {
      len = AddBytes(to, len, "%02X,", rpn->raw_bytes[idx]);
    }
    len = AddBytes(to, len, "$ID,-2$");
    goto ret;
  case __IC_STATICS_SIZE:
    len = AddBytes(to, len, "$PURPLE$STATICS_SIZE(%p)$FD$\n", rpn->integer);
    goto ret;
    break;
  case __IC_SET_STATIC_DATA:
    len = AddBytes(to, len, "$PURPLE$SET_STATIC_DATA(%p) at %p:$FD$\n",
                   rpn->code_misc->str_len, rpn->code_misc->integer);
    goto bytes;
    break;
  case __IC_STATIC_REF:
    len = AddBytes(to, len, "$PURPLE$STATIC_REF:$FD$\n");
    goto ret;
  case IC_SHORT_ADDR:
    len =
        AddBytes(to, len, "$PURPLE$SHORT_ADDR(%s):$FD$\n", rpn->code_misc->str);
    len = DolDocUnMacro(to, len, rpn);
    return len;
  case IC_RELOC:
    len = AddBytes(to, len, "$PURPLE$RELOC(%s):$FD$\n", rpn->code_misc->str);
    len = DolDocUnMacro(to, len, rpn);
    return len;
  case IC_TO_BOOL:
    len = AddBytes(to, len, "$PURPLE$TO_BOOL$FD$\n");
    goto unop;
  case IC_GET_VARGS_PTR:
    len = AddBytes(to, len, "$PURPLE$GET_VARGS_PTR$FD$\n");
    return len;
  case __IC_VARGS:
    len = AddBytes(to, len, "$PURPLE$VARGS$FD$\n");
    len = DolDocUnMacro(to, len, rpn);
    len = AddBytes(to, len, "$ID,2$");
    rpn = rpn->base.next;
    for (idx = 0; idx != orig_rpn->length; idx++) {
      len = DolDocDumpIR(to, len, rpn);
      rpn = ICFwd(rpn);
    }
    len = AddBytes(to, len, "$ID,-2$");
    return len;
  case IC_GOTO_IF:
    len = AddBytes(to, len, "$PURPLE$GOTO_IF:(%p)$FD$\n", rpn->code_misc);
    goto unop;
    break;
  case __IC_SET_FRAME_SIZE:
    len = AddBytes(to, len, "$PURPLE$FRAME_SIZE(%p)$FD$\n", rpn->integer);
    return len;
    break;
  case __IC_ARG:
    len = AddBytes(to, len, "$PURPLE$ARG(%p)$FD$\n", rpn->integer);
    len = DolDocDumpType(to, len, rpn);
    return len;
    break;
  case IC_STATIC:
    len = AddBytes(to, len, "$PURPLE$STATIC(%p)$FD$\n", rpn->integer);
    len = DolDocDumpType(to, len, rpn);
    return len;
  case IC_TYPECAST:
    len = AddBytes(to, len, "$PURPLE$TYPECAST$FD$\n", rpn->integer);
    len = DolDocDumpType(to, len, rpn);
    len = DolDocUnMacro(to, len, rpn);
    len = AddBytes(to, len, "$ID,2$");
    len = DolDocDumpIR(to, len, rpn->base.next);
    len = AddBytes(to, len, "$ID,-2$");
    goto ret;
    break;
  case IC_SUB_RET:
    len = AddBytes(to, len, "$PURPLE$SUB_RET$FD$\n", rpn->integer);
    len = DolDocUnMacro(to, len, rpn);
    goto ret;
    break;
  case IC_SUB_PROLOG:
    len = AddBytes(to, len, "$PURPLE$SUB_PROLOG$FD$\n", rpn->integer);
    len = DolDocUnMacro(to, len, rpn);
    goto ret;
    break;
  case IC_SUB_CALL:
    len = AddBytes(to, len, "$PURPLE$SUB_CALL$FD$\n", rpn->integer);
    len = DolDocUnMacro(to, len, rpn);
    goto ret;
    break;
  case IC_UNBOUNDED_SWITCH:
    len = AddBytes(to, len, "$PURPLE$Switch[]$FD$\n", rpn->integer);
  swit:
    len = DolDocUnMacro(to, len, rpn);
    len = AddBytes(to, len, "$ID,2$");
    len = DolDocDumpIR(to, len, rpn->base.next);
    len = AddBytes(to, len, "$ID,-2$");
    goto ret;
    break;
  case IC_TO_I64:
    len = AddBytes(to, len, "$PURPLE$TO_I64$FD$\n");
    goto unop;
  case IC_TO_F64:
    len = AddBytes(to, len, "$PURPLE$TO_F64$FD$\n");
    goto unop;
    break;
  case IC_BOUNDED_SWITCH:
    len = AddBytes(to, len, "$PURPLE$SWITCH$FD$\n");
    goto swit;
    break;
  case IC_GS:
    len = AddBytes(to, len, "$PURPLE$GS:$FD$\n");
    goto unop;
  case IC_FS:
    len = AddBytes(to, len, "$PURPLE$FS:$FD$\n");
    goto unop;
  case __IC_CALL:
  case IC_CALL:
    len = AddBytes(to, len, "$PURPLE$CALL:$FD$\n");
    len = DolDocUnMacro(to, len, rpn);
    len = DolDocDumpType(to, len, rpn);
    len = AddBytes(to, len, "$ID,2$");
    rpn = rpn->base.next;
    for (idx = 0; idx != orig_rpn->length; idx++) {
      len = DolDocDumpIR(to, len, rpn);
      rpn = ICFwd(rpn);
    }
    len = DolDocDumpIR(to, len, rpn);
    len = AddBytes(to, len, "$ID,-2$");
    goto ret;
    break;
  case IC_COMMA:
    len = AddBytes(to, len, "$PURPLE$,$FD$\n");
    goto binop;
    break;
  case IC_BT:
    len = AddBytes(to, len, "$PURPLE$BT$FD$\n");
    goto binop;
  case IC_BTR:
    len = AddBytes(to, len, "$PURPLE$BTR$FD$\n");
    goto binop;
  case IC_BTS:
    len = AddBytes(to, len, "$PURPLE$BTS$FD$\n");
    goto binop;
  case IC_BTC:
    len = AddBytes(to, len, "$PURPLE$BTC$FD$\n");
    goto binop;
  case IC_LBTR:
    len = AddBytes(to, len, "$PURPLE$LBTR$FD$\n");
    goto binop;
  case IC_LBTS:
    len = AddBytes(to, len, "$PURPLE$LBTS$FD$\n");
    goto binop;
  case IC_LBTC:
    len = AddBytes(to, len, "$PURPLE$LBTC$FD$\n");
    goto binop;
  case IC_BASE_PTR:
    len = AddBytes(to, len, "$PURPLE$FRAME(%p)$FD$\n", rpn->integer);
    len = DolDocDumpType(to, len, rpn);
    goto ret;
    break;
  case IC_IREG:
    len = AddBytes(to, len, "$PURPLE$IREG(%p)$FD$\n", rpn->integer);
    len = DolDocDumpType(to, len, rpn);
    goto ret;
    break;
  case IC_FREG:
    len = AddBytes(to, len, "$PURPLE$FREG(%p)$FD$\n", rpn->integer);
    len = DolDocDumpType(to, len, rpn);
    goto ret;
    break;
  case IC_RET:
    len = AddBytes(to, len, "$PURPLE$RETURN$FD$\n");
    goto unop;
    break;
  case IC_GOTO:
    len = AddBytes(to, len, "$PURPLE$GOTO(%p):$FD$\n", rpn->code_misc);
    len = DolDocUnMacro(to, len, rpn);
    goto ret;
    break;
  case IC_LABEL:
    len = AddBytes(to, len, "$PURPLE$LABEL(%p):$FD$\n", rpn->code_misc);
    goto ret;
    break;
  case IC_GLOBAL:
    len = AddBytes(to, len, "$PURPLE$GLOBAL(%p):$FD$\n",
                   rpn->global_var->base.str);
    len = DolDocUnMacro(to, len, rpn);
    goto ret;
    break;
  case IC_LOCAL:
    len = AddBytes(to, len, "$PURPLE$LOCAL(%p):$FD$\n", rpn->local_mem->str);
    len = DolDocUnMacro(to, len, rpn);
    goto ret;
    break;
  case IC_NOP:
    goto ret;
    break;
  case IC_NAME:
    goto ret;
    break;
  case IC_POW:
    len = AddBytes(to, len, "$PURPLE$`$FD$\n");
    goto binop;
    break;
  case IC_ADD:
    len = AddBytes(to, len, "$PURPLE$+$FD$\n");
    goto binop;
    break;
  case IC_EQ:
    len = AddBytes(to, len, "$PURPLE$=$FD$\n");
    goto binop;
    break;
  case IC_SUB:
    len = AddBytes(to, len, "$PURPLE$-$FD$\n");
    goto binop;
    break;
  case IC_DIV:
    len = AddBytes(to, len, "$PURPLE$/$FD$\n");
    goto binop;
    break;
  case IC_MUL:
    len = AddBytes(to, len, "$PURPLE$*$FD$\n");
    goto binop;
    break;
  case IC_DEREF:
    len = AddBytes(to, len, "$PURPLE$*ptr$FD$\n");
    goto unop;
    break;
  case IC_AND:
    len = AddBytes(to, len, "$PURPLE$&$FD$\n");
    goto binop;
    break;
  case IC_ADDR_OF:
    len = AddBytes(to, len, "$PURPLE$&addr$FD$\n");
    goto unop;
    break;
  case IC_XOR:
    len = AddBytes(to, len, "$PURPLE$%$FD$\n");
    goto binop;
    break;
  case IC_MOD:
    len = AddBytes(to, len, "$PURPLE$%%$FD$\n");
    goto binop;
    break;
  case IC_OR:
    len = AddBytes(to, len, "$PURPLE$|$FD$\n");
    goto binop;
    break;
  case IC_LT:
    len = AddBytes(to, len, "$PURPLE$<$FD$\n");
    goto binop;
    break;
  case IC_GT:
    len = AddBytes(to, len, "$PURPLE$>$FD$\n");
    goto binop;
    break;
  case IC_LNOT:
    len = AddBytes(to, len, "$PURPLE$!$FD$\n");
    goto unop;
    break;
  case IC_BNOT:
    len = AddBytes(to, len, "$PURPLE$~$FD$\n");
    goto unop;
    break;
  case IC_POST_INC:
    len = AddBytes(to, len, "$PURPLE$x++$FD$\n");
    goto unop;
    break;
  case IC_POST_DEC:
    len = AddBytes(to, len, "$PURPLE$x--$FD$\n");
    goto unop;
    break;
  case IC_PRE_INC:
    len = AddBytes(to, len, "$PURPLE$++x$FD$\n");
    goto unop;
    break;
  case IC_PRE_DEC:
    len = AddBytes(to, len, "$PURPLE$--x$FD$\n");
    goto unop;
    break;
  case IC_AND_AND:
    len = AddBytes(to, len, "$PURPLE$&&$FD$\n");
    goto binop;
    break;
  case IC_OR_OR:
    len = AddBytes(to, len, "$PURPLE$||$FD$\n");
    goto binop;
    break;
  case IC_XOR_XOR:
    len = AddBytes(to, len, "$PURPLE$^^$FD$\n");
    goto binop;
    break;
  case IC_EQ_EQ:
    len = AddBytes(to, len, "$PURPLE$==$FD$\n");
    goto binop;
    break;
  case IC_NE:
    len = AddBytes(to, len, "$PURPLE$!=$FD$\n");
    goto binop;
    break;
  case IC_LE:
    len = AddBytes(to, len, "$PURPLE$<=$FD$\n");
    goto binop;
    break;
  case IC_GE:
    len = AddBytes(to, len, "$PURPLE$>=$FD$\n");
    goto binop;
    break;
  case IC_LSH:
    len = AddBytes(to, len, "$PURPLE$<<$FD$\n");
    goto binop;
    break;
  case IC_RSH:
    len = AddBytes(to, len, "$PURPLE$>>$FD$\n");
    goto binop;
    break;
  case IC_ADD_EQ:
    len = AddBytes(to, len, "$PURPLE$+=$FD$\n");
    goto binop;
    break;
  case IC_SUB_EQ:
    len = AddBytes(to, len, "$PURPLE$-=$FD$\n");
    goto binop;
    break;
  case IC_MUL_EQ:
    len = AddBytes(to, len, "$PURPLE$*=$FD$\n");
    goto binop;
    break;
  case IC_DIV_EQ:
    len = AddBytes(to, len, "$PURPLE$/=$FD$\n");
    goto binop;
    break;
  case IC_LSH_EQ:
    len = AddBytes(to, len, "$PURPLE$<<=$FD$\n");
    goto binop;
    break;
  case IC_RSH_EQ:
    len = AddBytes(to, len, "$PURPLE$>>=$FD$\n");
    goto binop;
    break;
  case IC_AND_EQ:
    len = AddBytes(to, len, "$PURPLE$&=$FD$\n");
    goto binop;
    break;
  case IC_OR_EQ:
    len = AddBytes(to, len, "$PURPLE$|=$FD$\n");
    goto binop;
    break;
  case IC_XOR_EQ:
    len = AddBytes(to, len, "$PURPLE$^=$FD$\n");
    goto binop;
    break;
  case IC_DOT:
    goto ret;
  case IC_ARROW:
    goto ret;
  case IC_MOD_EQ:
    len = AddBytes(to, len, "$PURPLE$%%=$FD$\n");
    goto binop;
  case IC_I64:
    len = AddBytes(to, len, "$PURPLE$INT($LK,\"0x%llx\",A=\"AD:%lld\"$)$FD$\n",
                   rpn->integer, rpn->integer);
    goto ret;
  case IC_STR:
    len = AddBytes(to, len, "$PURPLE$STRING(%p)$FD$\n", rpn->code_misc->str);
    goto bytes;
  case IC_CHR:
    len = AddBytes(to, len, "$PURPLE$CHAR(%02x)$FD$\n", rpn->integer);
    goto ret;
  case IC_F64:
    len = AddBytes(to, len, "$PURPLE$FLOAT(%lf)$FD$\n", rpn->flt);
    goto ret;
  case IC_ARRAY_ACC:
  binop:
    len = DolDocUnMacro(to, len, rpn);
    len = DolDocDumpType(to, len, rpn);
    len = AddBytes(to, len, "$ID,2$");
    rpn = rpn->base.next;
    for (idx = 0; idx != 2; idx++) {
      len = DolDocDumpIR(to, len, rpn);
      rpn = ICFwd(rpn);
    }
    len = AddBytes(to, len, "$ID,-2$");
    goto ret;
  case IC_SQRT:
    len = AddBytes(to, len, "$PURPLE$SQRT$FD$\n");
    goto unop;
    break;
  case IC_SQR:
    len = AddBytes(to, len, "$PURPLE$SQR$FD$\n");
    goto unop;
    break;
  case IC_NEG:
    len = AddBytes(to, len, "$PURPLE$-a$FD$\n");
    goto unop;
  case IC_POS:
    len = AddBytes(to, len, "$PURPLE$+a$FD$\n");
  unop:
    len = DolDocUnMacro(to, len, rpn);
    len = DolDocDumpType(to, len, rpn);
    len = AddBytes(to, len, "$ID,2$");
    rpn = rpn->base.next;
    len = DolDocDumpIR(to, len, rpn);
    len = AddBytes(to, len, "$ID,-2$");
    goto ret;
  default:
    printf("%ld\n", rpn->type);
    abort();
  }
  //???
  __builtin_unreachable();
ret:
  return len;
}

static int64_t IsRightAssoc(int64_t ic) {
  switch (ic) {
  case IC_PRE_DEC:
  case IC_PRE_INC:
  case IC_ADDR_OF:
  case IC_DEREF:
  case IC_BNOT:
  case IC_LNOT:
  case IC_POS:
  case IC_NEG:
  case IC_SQRT:
  case IC_EQ:
  case IC_ADD_EQ:
  case IC_SUB_EQ:
  case IC_MUL_EQ:
  case IC_DIV_EQ:
  case IC_LSH_EQ:
  case IC_RSH_EQ:
  case IC_AND_EQ:
  case IC_OR_EQ:
  case IC_XOR_EQ:
  case IC_MOD_EQ:
    return 1;
  }
  return 0;
}

CMemberLst *MemberFind(char *needle, CHashClass *cls) {
  CMemberLst *lst;
  for (lst = cls->members_lst; lst; lst = lst->next)
    if (lst->str)
      if (!strcmp(needle, lst->str))
        return lst;
  if (cls->base_class)
    return MemberFind(needle, cls->base_class);
  if (cls->fwd_class)
    return MemberFind(needle, cls->fwd_class);
  return NULL;
}
//
// This does not mirror TempleOS's exactly,what I do
// just fill in the ->address with the needed ptr
//
void SysSymImportsResolve(char *sym, int64_t flags) {
  CHashImport *imp;
  CHash *thing = HashFind(sym, Fs->hash_table, HTT_FUN | HTT_GLBL_VAR, 1);
  int written = 0;
  void *with;
  if (thing->type & HTT_FUN) {
    with = ((CHashFun *)thing)->fun_ptr;
  } else if (thing->type & HTT_GLBL_VAR) {
    with = ((CHashGlblVar *)thing)->data_addr;
  } else
    throw(*(int64_t *)"Resolve");
  if (!with)
    return;
  int old = SetWriteNP(0);
  while (imp =
             HashSingleTableFind(sym, Fs->hash_table, HTT_IMPORT_SYS_SYM, 1)) {
    SetWriteNP(0);
    *imp->address = with; // TODO make TempleOS like
    SetWriteNP(1);
#if defined(__APPLE__)
    sys_icache_invalidate(imp->address, 8);
#else
    __builtin___clear_cache(imp->address, imp->address + 8);
#endif
    imp->base.type = HTT_INVALID;
    written = 1;
  }
}
static int64_t SetIncAmt(CCmpCtrl *ccmp, CRPN *target) {
  CRPN *next;
  int64_t a = 1;
  AssignRawTypeToNode(ccmp, next = target->base.next);
  if (next->ic_dim || next->ic_class->ptr_star_cnt) {
    if (next->ic_dim) {
      a *= next->ic_dim->total_cnt;
      a *= next->ic_class->sz;
    } else
      a = next->ic_class[-1].sz;
  }
  target->integer = a;
}
// https://en.wikipedia.org/wiki/Shunting_yard_algorithm
#define PEF_NO_COMMA 1
int64_t ParseExpr(CCmpCtrl *ccmp, int64_t flags) {
#define POST_XXX_CHECK                                                         \
  {                                                                            \
    CRPN *ic;                                                                  \
    {                                                                          \
      switch (ccmp->lex->cur_tok) {                                            \
      case TK_INC_INC:                                                         \
        (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_POST_INC;               \
        break;                                                                 \
      case TK_DEC_DEC:                                                         \
        (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_POST_DEC;               \
      }                                                                        \
      if (ccmp->lex->cur_tok == TK_INC_INC ||                                  \
          ccmp->lex->cur_tok == TK_DEC_DEC) {                                  \
        QueIns(ic, ccmp->code_ctrl->ir_code);                                  \
        AssignRawTypeToNode(ccmp, ic);                                         \
        SetIncAmt(ccmp, ic);                                                   \
        Lex(ccmp->lex);                                                        \
      }                                                                        \
    }                                                                          \
  }
  int64_t tok, prec, type, binop_before = 1, arg, str_len;
  int64_t ic_stk[0x100];
  int64_t prec_stk[0x100];
  int64_t stk_ptr = 0, idx;
  CRPN *ic, *ic2, *orig = ccmp->code_ctrl->ir_code->next;
  char *string;
  CMemberLst *local;
  CHashGlblVar *global_var;
  CCodeMisc *misc;
  CHashClass *tc_class;
  CHashFun *tc_fun;
  CArrayDim dummy_dim;
next:
  while (tok = ccmp->lex->cur_tok) {
    switch (tok) {
    case '\n':
      goto fin;
      break;
    case TK_KW_OFFSET:
    offo:
      if (ccmp->lex->cur_tok != '(')
        ParseErr(ccmp, "Expected a '('.");
      Lex(ccmp->lex);
      if (ccmp->lex->cur_tok != TK_NAME)
        ParseErr(ccmp, "Expected a 'typename'.");
      tc_class = HashFind(ccmp->lex->string, Fs->hash_table, HTT_CLASS, 1);
      if (!tc_class)
        ParseErr(ccmp, "Expected a 'typename'.");
      Lex(ccmp->lex);
      if (ccmp->lex->cur_tok != '.')
        ParseErr(ccmp, "Expected a '.'.");
      Lex(ccmp->lex);
      if (ccmp->lex->cur_tok != TK_NAME)
        ParseErr(ccmp, "Expected a 'member'.");
      if (!MemberFind(ccmp->lex->string, tc_class))
        ParseErr(ccmp, "Class '%s' is missing member '%s'.", tc_class->base.str,
                 ccmp->lex->string);
      Lex(ccmp->lex);
      if (ccmp->lex->cur_tok != ')')
        ParseErr(ccmp, "Expected a ')'.");
      Lex(ccmp->lex);
      *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_I64,
          .integer = MemberFind(ccmp->lex->string, tc_class)->off,
      };
      QueIns(ic, ccmp->code_ctrl->ir_code);
      binop_before = 0;
      goto next;
      break;
    case TK_KW_SIZEOF:
      Lex(ccmp->lex);
    szo:
      if (ccmp->lex->cur_tok == '(') {
        Lex(ccmp->lex);
        if (ccmp->lex->cur_tok != TK_NAME)
          ParseErr(ccmp, "Expected a typename.");
        if (!(tc_class =
                  HashFind(ccmp->lex->string, Fs->hash_table, HTT_CLASS, 1)))
          ParseErr(ccmp, "Expected a typename.");
        Lex(ccmp->lex);
        tc_class = PrsType(ccmp, tc_class, NULL, NULL, &dummy_dim);
        arg = tc_class->sz * dummy_dim.total_cnt;
        if (dummy_dim.next)
          ArrayDimDel(dummy_dim.next);
        (ic = A_CALLOC(sizeof(CRPN), NULL))->type = type = IC_I64;
        QueInit(&ic->base);
        ic->integer = arg;
        QueIns(ic, ccmp->code_ctrl->ir_code);
        if (ccmp->lex->cur_tok != ')')
          ParseErr(ccmp, "Expected a ')'.");
        Lex(ccmp->lex);
        binop_before = 0;
        goto next;
      } else if (ccmp->lex->cur_tok == TK_NAME) {
        if (!(tc_class =
                  HashFind(ccmp->lex->string, Fs->hash_table, HTT_CLASS, 1)))
          ParseErr(ccmp, "Expected a typename.");
        arg = tc_class->sz;
        Lex(ccmp->lex);
        (ic = A_CALLOC(sizeof(CRPN), NULL))->type = type = IC_I64;
        QueInit(&ic->base);
        ic->integer = arg;
        QueIns(ic, ccmp->code_ctrl->ir_code);
        binop_before = 0;
        goto next;
      } else
        ParseErr(ccmp, "Expected a typename.");
      break;
    case '`':
      prec = 1;
      type = IC_POW;
      binop_before = 1;
      Lex(ccmp->lex);
      break;
    case '+':
      Lex(ccmp->lex);
      if (binop_before) {
        // Unary
        binop_before = 0;
        prec = 0;
        type = IC_POS;
      } else {
        prec = 6;
        binop_before = 1;
        type = IC_ADD;
      }
      break;
    case '=':
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_EQ;
      break;
    case '-':
      Lex(ccmp->lex);
      if (binop_before) {
        // Unary
        binop_before = 1; // Like a binop as it consumes the right item
        prec = 0;
        type = IC_NEG;
      } else {
        prec = 6;
        binop_before = 1;
        type = IC_SUB;
      }
      break;
    case '/':
      Lex(ccmp->lex);
      prec = 2;
      type = IC_DIV;
      binop_before = 1;
      break;
    case '*':
      Lex(ccmp->lex);
      if (binop_before) {
        // Unary
        binop_before = 1; // Consumes like a binop
        prec = 0;
        type = IC_DEREF;
      } else {
        prec = 2;
        type = IC_MUL;
        binop_before = 1;
      }
      break;
    case '&':
      Lex(ccmp->lex);
      if (binop_before) {
        // Unary
        binop_before = 1; // Consumes next thing like a binop
        prec = 0;
        type = IC_ADDR_OF;
      } else {
        prec = 3;
        type = IC_AND;
        binop_before = 1;
      }
      break;
    case '^':
      Lex(ccmp->lex);
      prec = 4;
      type = IC_XOR;
      binop_before = 1;
      break;
    case '%':
      Lex(ccmp->lex);
      prec = 2;
      type = IC_MOD;
      binop_before = 1;
      break;
    case '|':
      Lex(ccmp->lex);
      prec = 5;
      type = IC_OR;
      binop_before = 1;
      break;
    case '<':
      Lex(ccmp->lex);
      prec = 7;
      type = IC_LT;
      binop_before = 1;
      break;
    case '>':
      Lex(ccmp->lex);
      prec = 7;
      type = IC_GT;
      binop_before = 1;
      break;
    case '~':
      Lex(ccmp->lex);
      prec = 1;
      type = IC_BNOT;
      binop_before = 1; // Acts like a binop as it consumes the right side
      break;
    case '!':
      Lex(ccmp->lex);
      prec = 2;
      type = IC_LNOT;
      binop_before = 1; // Acts like a binop as it consumes the right side
      break;
    case '(':
      Lex(ccmp->lex);
      // If we are not being consumed by a binop,we are a function call
      // or a typecast,so if we have a type,it is a typecast,else a function
      // call
      if (!binop_before) {
        if (ccmp->lex->cur_tok == TK_NAME) {
          if (tc_class =
                  HashFind(ccmp->lex->string, Fs->hash_table, HTT_CLASS, 1)) {
            //
            // If our previous item is an uncalled function (we assumed we call
            // functions when a '(' comes after), we will now call it. IT makes
            // no  sense to typecast a function
            //
            // I64 Foo() {}
            // If we have Foo(F64),treat it as Foo()(F64)
            ic2 = ccmp->code_ctrl->ir_code->next;
            if (ic2->type == IC_GLOBAL) {
              if (ic2->global_var->base.type & HTT_FUN) {
                (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_CALL;
                QueIns(ic, ccmp->code_ctrl->ir_code);
                AssignRawTypeToNode(ccmp, ic);
              }
            }
            //
            // See ghost ahead
            //
            ic2 = ccmp->code_ctrl->ir_code->next;

            *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
                .type = IC_TYPECAST,
                .ic_dim = A_CALLOC(sizeof(CArrayDim), NULL),
            };
            *ic->ic_dim = (CArrayDim){
                .total_cnt = INT_MIN, // If still negative,we didint get a dim
                .cnt = 1,
            };
            Lex(ccmp->lex);
            ic->ic_class =
                PrsType(ccmp, tc_class, NULL, &ic->ic_fun, ic->ic_dim);
            if (!ic->ic_dim->next) {
              A_FREE(ic->ic_dim);
              ic->ic_dim = NULL;
            }
            binop_before = 0;
            QueIns(ic, ccmp->code_ctrl->ir_code);
            if (ccmp->lex->cur_tok != ')')
              ParseErr(ccmp, "Expected a ')'.");
            Lex(ccmp->lex);
            POST_XXX_CHECK;
            //  ________       ______
            // /        \     | READ \
            // | (*)_(*)|     |  ME  |
            // |  \___/ | <===_______/
            // |        |
            // \/\/\/\/\/
            // if we have function,make an implicit function call and typecast
            // the result
            if (ic2->type == IC_GLOBAL) {
              // If IC_ADDR_OF comes before,dont do an implicit call
              if (ic_stk[stk_ptr - 1] == IC_ADDR_OF) {
                // Do nothing
                if (ic2->global_var->base.type & HTF_EXTERN)
                  ParseErr(ccmp, "Can't address of extern symbol");
              } else if (ic2->global_var->base.type & HTT_FUN) {
                (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_CALL;
                QueIns(ic, ic2->base.last);
              }
            }
            goto next;
          } else
            goto is_func_call;
        } else {
          // Is func
        is_func_call:
          (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_CALL;
          for (arg = 0;;) {
            ic2 = ccmp->code_ctrl->ir_code->next;
            ParseExpr(ccmp, PEF_NO_COMMA);
            if (ic2 != ccmp->code_ctrl->ir_code->next)
              arg++;
            if (ccmp->lex->cur_tok == ')') {
              Lex(ccmp->lex);
              break;
            }
            if (ic2 == ccmp->code_ctrl->ir_code->next) {
              // Found no argument so make a IC_NOP
              (ic2 = A_CALLOC(sizeof(CRPN), NULL))->type = IC_NOP;
              QueIns(ic2, ccmp->code_ctrl->ir_code);
              arg++;
            }
            if (ccmp->lex->cur_tok == ',') {
              Lex(ccmp->lex);
            } else
              ParseErr(ccmp, "Expected a ')'.");
          }
          ic->length = arg;
          QueIns(ic, ccmp->code_ctrl->ir_code);
          AssignRawTypeToNode(ccmp, ic);
          goto next;
        }
      }
      prec = -1;
      type = IC_PAREN;
      binop_before = 1; // We consume the next thing like a binop
      break;
    case ')':
      binop_before = 0;
      for (idx = 0; idx != stk_ptr; idx++) {
        if (ic_stk[idx] == IC_PAREN) {
          Lex(ccmp->lex);
          while (ic_stk[--stk_ptr] != IC_PAREN) {
            (ic = A_CALLOC(sizeof(CRPN), NULL))->type = ic_stk[stk_ptr];
            QueIns(ic, ccmp->code_ctrl->ir_code);
            AssignRawTypeToNode(ccmp, ic);
            if (ic->type == IC_PRE_DEC || ic->type == IC_PRE_INC)
              SetIncAmt(ccmp, ic);
          }
          POST_XXX_CHECK;
          goto next;
        }
      }
      goto fin;
      break;
    case '[':
      prec = -1;
      type = IC_ARRAY_ACC;
      Lex(ccmp->lex);
      binop_before = 1; // Acts like a binop as it consumes the right side
      break;
    case ']':
      binop_before = 0;
      for (idx = 0; idx != stk_ptr; idx++) {
        if (ic_stk[idx] == IC_ARRAY_ACC) {
          Lex(ccmp->lex);
          while (ic_stk[--stk_ptr] != IC_ARRAY_ACC) {
            (ic = A_CALLOC(sizeof(CRPN), NULL))->type = ic_stk[stk_ptr];
            QueIns(ic, ccmp->code_ctrl->ir_code);
            AssignRawTypeToNode(ccmp, ic);
            if (ic->type == IC_PRE_DEC || ic->type == IC_PRE_INC)
              SetIncAmt(ccmp, ic);
          }
          (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_ARRAY_ACC;
          QueIns(ic, ccmp->code_ctrl->ir_code);
          AssignRawTypeToNode(ccmp, ic);
          POST_XXX_CHECK;
          goto next;
        }
      }
      goto fin;
      break;
    case TK_I64:
      binop_before = 0;
      prec = -2;
      (ic = A_CALLOC(sizeof(CRPN), NULL))->type = type = IC_I64;
      QueInit(&ic->base);
      ic->integer = ccmp->lex->integer;
      QueIns(ic, ccmp->code_ctrl->ir_code);
      Lex(ccmp->lex);
      goto next;
      break;
    case TK_F64:
      binop_before = 0;
      prec = -2;
      (ic = A_CALLOC(sizeof(CRPN), NULL))->type = type = IC_F64;
      QueInit(&ic->base);
      ic->flt = ccmp->lex->flt;
      QueIns(ic, ccmp->code_ctrl->ir_code);
      Lex(ccmp->lex);
      goto next;
      break;
    case TK_NAME:
      if (PrsKw(ccmp, TK_KW_SIZEOF))
        goto szo;
      if (PrsKw(ccmp, TK_KW_OFFSET))
        goto offo;
      binop_before = 0;
      ic = A_CALLOC(sizeof(CRPN), NULL);
      QueInit(&ic->base);
      if (ccmp->cur_fun) {
        if (local = MemberFind(ccmp->lex->string, ccmp->cur_fun)) {
          ic->type = IC_LOCAL;
          ic->local_mem = local;
          goto name_pass;
        }
      }
      if (global_var =
              HashFind(ccmp->lex->string, Fs->hash_table, HTT_GLBL_VAR, 1)) {
        ic->type = IC_GLOBAL;
        ic->global_var = global_var;
        goto name_pass;
      } else if (global_var =
                     HashFind(ccmp->lex->string, Fs->hash_table, HTT_FUN, 1)) {
        if (((CRPN *)ccmp->code_ctrl->ir_code->next)->type == IC_ADDR_OF) {
          ic->type = IC_GLOBAL;
          ic->global_var = global_var;
          goto name_pass;
        } else {
          Lex(ccmp->lex);
          ic->type = IC_GLOBAL;
          ic->global_var = global_var;
          if (ccmp->lex->cur_tok != '(') {
            // If IC_ADDR_OF comes before,dont do an implicit call
            if (stk_ptr && ic_stk[stk_ptr - 1] == IC_ADDR_OF) {
              // Do nothing
              if (ic->global_var->base.type & HTF_EXTERN)
                ParseWarn(ccmp, "Can't address of extern symbol");
            } else if (ic->global_var->base.type & HTT_FUN) {
              QueIns(ic, ccmp->code_ctrl->ir_code);
              ic2 = ic;
              (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_CALL;
              QueIns(ic, ic2->base.last);
              goto next;
            }
          }
          QueIns(ic, ccmp->code_ctrl->ir_code);
          goto next;
        }
      } else {
        A_FREE(ic);
        ParseErr(ccmp, "Unknown symbol '%s'.", ccmp->lex->string);
        return 0;
      }
    name_pass:
      Lex(ccmp->lex);
      QueIns(ic, ccmp->code_ctrl->ir_code);
      POST_XXX_CHECK;
      goto next;
      break;
    case TK_STR:
      binop_before = 0;
      string = PrsString(ccmp, &str_len);
      ic = A_CALLOC(sizeof(CRPN), NULL);
      QueInit(&ic->base);
      ic->type = IC_STR;
      for (misc = ccmp->code_ctrl->code_misc->next;
           ccmp->code_ctrl->code_misc != misc; misc = misc->base.next) {
        if (misc->type == CMT_STRING)
          if (misc->str_len == ccmp->lex->str_len)
            if (0 == memcmp(misc->str, string, str_len)) {
              ic->code_misc = misc;
              goto found_str;
            }
      }
      misc = CodeMiscNew(ccmp, CMT_STRING);
      misc->str_len = str_len;
      misc->str = string;
      ic->code_misc = misc;
    found_str:
      QueIns(ic, ccmp->code_ctrl->ir_code);
      goto next;
      break;
    case TK_CHR:
      binop_before = 0;
      ic = A_CALLOC(sizeof(CRPN), NULL);
      QueInit(&ic->base);
      ic->type = IC_CHR;
      ic->integer = ccmp->lex->integer;
      QueIns(ic, ccmp->code_ctrl->ir_code);
      Lex(ccmp->lex);
      goto next;
      break;
    case TK_ERR: // TODO
      return 0;
      break;
    case TK_INC_INC:
      Lex(ccmp->lex);
      binop_before = 1; // Consumes next arg like a binop
      type = IC_PRE_INC;
      prec = 0;
      break;
    case TK_DEC_DEC:
      Lex(ccmp->lex);
      binop_before = 1; // Consumes next arg like a binop
      type = IC_PRE_DEC;
      prec = 0;
      break;
    case TK_AND_AND:
      Lex(ccmp->lex);
      prec = 9;
      binop_before = 1;
      type = IC_AND_AND;
      break;
    case TK_XOR_XOR:
      Lex(ccmp->lex);
      prec = 10;
      binop_before = 1;
      type = IC_XOR_XOR;
      break;
    case TK_OR_OR:
      Lex(ccmp->lex);
      prec = 11;
      binop_before = 1;
      type = IC_OR_OR;
      break;
    case TK_EQ_EQ:
      Lex(ccmp->lex);
      prec = 8;
      binop_before = 1;
      type = IC_EQ_EQ;
      break;
    case TK_NE:
      Lex(ccmp->lex);
      prec = 8;
      binop_before = 1;
      type = IC_NE;
      break;
    case TK_LE:
      Lex(ccmp->lex);
      prec = 7;
      binop_before = 1;
      type = IC_LE;
      break;
    case TK_GE:
      Lex(ccmp->lex);
      prec = 7;
      binop_before = 1;
      type = IC_GE;
      break;
    case TK_LSH:
      Lex(ccmp->lex);
      prec = 1;
      binop_before = 1;
      type = IC_LSH;
      break;
    case TK_RSH:
      Lex(ccmp->lex);
      prec = 1;
      binop_before = 1;
      type = IC_RSH;
      break;
    case TK_ADD_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_ADD_EQ;
      break;
    case TK_SUB_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_SUB_EQ;
      break;
    case TK_MUL_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_MUL_EQ;
      break;
    case TK_DIV_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_DIV_EQ;
      break;
    case TK_LSH_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_LSH_EQ;
      break;
    case TK_RSH_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_RSH_EQ;
      break;
    case TK_AND_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_AND_EQ;
      break;
    case TK_OR_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_OR_EQ;
      break;
    case TK_XOR_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_XOR_EQ;
      break;
    case ',':
      if (flags & PEF_NO_COMMA)
        goto fin;
      Lex(ccmp->lex);
      prec = 13;
      binop_before = 1;
      type = IC_COMMA;
      break;
    case TK_ARROW:
      Lex(ccmp->lex);
      AssignRawTypeToNode(ccmp, ic2 = ccmp->code_ctrl->ir_code->next);
      if (ccmp->lex->cur_tok != TK_NAME)
        ParseErr(ccmp, "Expected a name.");
      if (!ic2->ic_class->ptr_star_cnt)
        ParseErr(ccmp, "Can't get member of non-pointer/array.");
      if (!MemberFind(ccmp->lex->string, ic2->ic_class - 1))
        ParseErr(ccmp, "Class '%s' is missing member '%s'\n",
                 ic2->ic_class[-1].base.str, ccmp->lex->string);
      *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_ARROW,
          .local_mem = MemberFind(ccmp->lex->string, ic2->ic_class - 1),
      };
      QueIns(ic, ccmp->code_ctrl->ir_code);
      ic->ic_fun = ic->local_mem->fun_ptr;
      Lex(ccmp->lex);
      binop_before = 0;
      POST_XXX_CHECK;
      goto next;
      break;
    case '.':
      Lex(ccmp->lex);
      AssignRawTypeToNode(ccmp, ic2 = ccmp->code_ctrl->ir_code->next);
      if (ccmp->lex->cur_tok != TK_NAME)
        ParseErr(ccmp, "Expected a name.");
      if (!MemberFind(ccmp->lex->string, ic2->ic_class))
        ParseErr(ccmp, "Class '%s' is missing member '%s'\n",
                 ic2->ic_class->base.str, ccmp->lex->string);
      *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_DOT,
          .local_mem = MemberFind(ccmp->lex->string, ic2->ic_class),
      };
      QueIns(ic, ccmp->code_ctrl->ir_code);
      Lex(ccmp->lex);
      binop_before = 0;
      POST_XXX_CHECK;
      goto next;
      break;
    case TK_MOD_EQ:
      Lex(ccmp->lex);
      prec = 12;
      binop_before = 1;
      type = IC_MOD_EQ;
      break;
    default:
      goto fin;
    }
    if (type == IC_PAREN || type == IC_ARRAY_ACC) {
      prec_stk[stk_ptr] = prec;
      ic_stk[stk_ptr++] = type;
    } else {
      for (; stk_ptr;) {
        if (ic_stk[stk_ptr - 1] == IC_PAREN ||
            ic_stk[stk_ptr - 1] == IC_ARRAY_ACC)
          goto fail;
        if (prec_stk[stk_ptr - 1] < prec)
          goto pass;
        if (prec_stk[stk_ptr - 1] <= prec && !IsRightAssoc(ic_stk[stk_ptr - 1]))
          goto pass;
        goto fail;
      pass:
        (ic = A_CALLOC(sizeof(CRPN), NULL))->type = ic_stk[stk_ptr - 1];
        QueIns(ic, ccmp->code_ctrl->ir_code);
        AssignRawTypeToNode(ccmp, ic);
        if (ic->type == IC_PRE_DEC || ic->type == IC_PRE_INC)
          SetIncAmt(ccmp, ic);
        stk_ptr--;
      }
    fail:
      prec_stk[stk_ptr] = prec;
      ic_stk[stk_ptr++] = type;
    }
  }
fin:
  while (stk_ptr) {
    ic = A_CALLOC(sizeof(CRPN), NULL);
    ic->type = ic_stk[stk_ptr - 1];
    QueIns(ic, ccmp->code_ctrl->ir_code);
    AssignRawTypeToNode(ccmp, ic);
    if (ic->type == IC_PRE_DEC || ic->type == IC_PRE_INC)
      SetIncAmt(ccmp, ic);
    stk_ptr--;
  }
  return orig != ccmp->code_ctrl->ir_code->next;
}

int64_t ParseWarn(CCmpCtrl *ctrl, char *fmt, ...) {
  va_list lst;
  va_start(lst, fmt);
  char buf[STR_LEN];
  vsprintf(buf, fmt, lst);
  va_end(lst);
  fprintf(AIWNIOS_OSTREAM, "WARN:%s:%d:%d %s\n", ctrl->lex->file->filename,
          ctrl->lex->file->ln + 1, ctrl->lex->file->col + 1, buf);
}

int64_t ParseErr(CCmpCtrl *ctrl, char *fmt, ...) {
  va_list lst;
  va_start(lst, fmt);
  char buf[STR_LEN];
  vsprintf(buf, fmt, lst);
  va_end(lst);
  fprintf(AIWNIOS_OSTREAM, "ERR:%s:%d:%d %s\n", ctrl->lex->file->filename,
          ctrl->lex->file->ln + 1, ctrl->lex->file->col + 1, buf);
  throw(*(int64_t *)"Prs\0\0\0\0\0");
}

int64_t PrsKw(CCmpCtrl *ccmp, int64_t kwt) {
  CHashKeyword *kw;
  if (ccmp->lex->cur_tok == TK_NAME)
    if (kw = HashFind(ccmp->lex->string, Fs->hash_table, HTT_KEYWORD, 1))
      if (kw->tk == kwt) {
        Lex(ccmp->lex);
        return 1;
      }
  return 0;
}
static int64_t PeekKw(CCmpCtrl *ccmp, int64_t kwt) {
  CHashKeyword *kw;
  if (ccmp->lex->cur_tok == TK_NAME)
    if (kw = HashFind(ccmp->lex->string, Fs->hash_table, HTT_KEYWORD, 1))
      if (kw->tk == kwt) {
        return 1;
      }
  return 0;
}

int64_t PrsGoto(CCmpCtrl *ccmp) {
  CRPN *rpn;
  CCodeMisc *misc;
  if (PrsKw(ccmp, TK_KW_GOTO)) {
    if (ccmp->lex->cur_tok != TK_NAME) {
      ParseErr(ccmp, "Expected a name.");
      return 0;
    }
    rpn = A_CALLOC(sizeof(CRPN), NULL);
    rpn->type = IC_GOTO;
    for (misc = ccmp->code_ctrl->code_misc->next;
         misc != ccmp->code_ctrl->code_misc; misc = misc->base.next) {
      if (misc->type == CMT_LABEL)
        if (misc->str)
          if (!strcmp(misc->str, ccmp->lex->string)) {
            rpn->code_misc = misc;
            goto found;
          }
    }
    rpn->code_misc = CodeMiscNew(ccmp, CMT_LABEL);
    rpn->code_misc->str = A_CALLOC(ccmp->lex->str_len + 1, NULL);
    memcpy(rpn->code_misc->str, ccmp->lex->string, ccmp->lex->str_len + 1);
  found:
    QueIns(rpn, ccmp->code_ctrl->ir_code);
    Lex(ccmp->lex);
    if (ccmp->lex->cur_tok != ';') {
      ParseErr(ccmp, "Expected a ';'.");
      return 0;
    }
    Lex(ccmp->lex);
    return 1;
  }
  return 0;
}
int64_t _PrsStmt(CCmpCtrl *ccmp) {
  CHashClass *cls, *cls2;
  CRPN *rpn;
  CCodeMisc *misc;
  int64_t is_func = 0, flags = 0;
  int64_t found_label = 0, assign_cnt = 0;
  char *import_name = NULL, *string;
  int64_t str_len, argc;
label_loop:
  // Check for label if not a function,global or local
  if (ccmp->lex->cur_tok == TK_NAME) {
    if (ccmp->cur_fun)
      if (MemberFind(ccmp->lex->string, ccmp->cur_fun))
        goto not_label;
    if (HashFind(
            ccmp->lex->string, Fs->hash_table,
            HTT_GLBL_VAR | HTT_GLBL_VAR | HTT_CLASS | HTT_KEYWORD | HTT_FUN, 1))
      goto not_label;
    rpn = A_CALLOC(sizeof(CRPN), NULL);
    rpn->type = IC_LABEL;
    found_label = 1;
    for (misc = ccmp->code_ctrl->code_misc->next;
         misc != ccmp->code_ctrl->code_misc; misc = misc->base.next) {
      if (misc->type == CMT_LABEL && misc->str)
        if (!strcmp(misc->str, ccmp->lex->string)) {
          rpn->code_misc = misc;
          goto label_fin;
        }
    }
    rpn->code_misc = CodeMiscNew(ccmp, CMT_LABEL);
    rpn->code_misc->str = A_STRDUP(ccmp->lex->string, NULL);
  label_fin:
    if (rpn->code_misc->flags & CMF_DEFINED)
      ParseErr(ccmp, "Redefinition of label '%s'.", rpn->code_misc->str);
    rpn->code_misc->flags |= CMF_DEFINED;
    Lex(ccmp->lex);
    if (ccmp->lex->cur_tok != ':') {
      ParseErr(ccmp, "Expected a ':'.");
      return 0;
    }
    Lex(ccmp->lex);
  }
not_label:
  if (found_label) {
    QueIns(rpn, ccmp->code_ctrl->ir_code);
    if (!PrsStmt(ccmp)) {
      ParseErr(ccmp, "Expected a statement for a label.");
    }
    return 1;
  }
  if (PrsKw(ccmp, TK_KW_BREAK)) {
    if (!ccmp->code_ctrl->break_to) {
      ParseErr(ccmp, "Unexpected break.");
      return 0;
    }
    if (ccmp->lex->cur_tok != ';') {
      ParseErr(ccmp, "Expected a ';'.");
      return 0;
    }
    Lex(ccmp->lex);
    if (ccmp->flags & CCF_IN_SUBSWITCH_START_BLOCK) {
      rpn = A_CALLOC(sizeof(CRPN), NULL);
      rpn->type = IC_SUB_RET;
      QueIns(rpn, ccmp->code_ctrl->ir_code);
    } else {
      rpn = A_CALLOC(sizeof(CRPN), NULL);
      rpn->type = IC_GOTO;
      rpn->code_misc = ccmp->code_ctrl->break_to;
      QueIns(rpn, ccmp->code_ctrl->ir_code);
    }
    return 1;
  }
  if (PrsTry(ccmp))
    return 1;
  if (PrsGoto(ccmp))
    return 1;
  if (PrsReturn(ccmp))
    return 1;
  if (PrsScope(ccmp))
    return 1;
  if (PrsIf(ccmp))
    return 1;
  if (PrsFor(ccmp))
    return 1;
  if (PrsSwitch(ccmp))
    return 1;
  if (PrsWhile(ccmp))
    return 1;
  if (PrsDo(ccmp))
    return 1;
  if (PrsKw(ccmp, TK_KW_EXTERN)) {
    flags = PRSF_EXTERN;
  prs_global:
    if (ccmp->lex->cur_tok != TK_NAME)
      ParseErr(ccmp, "Expected a type name.");
    if (cls = HashFind(ccmp->lex->string, Fs->hash_table, HTT_CLASS, 1)) {
      Lex(ccmp->lex);
    } else if (!(cls = PrsClass(ccmp, flags)))
      ParseErr(ccmp, "Expected a type name.");
    else { // cls was declared if we reach here
      // Don't whine about class declaration without variable declared also
      if (ccmp->lex->cur_tok == ';') {
        Lex(ccmp->lex);
        return 1;
      }
    }
  prs_global_loop:
    PrsDecl(ccmp, cls, NULL, &is_func, flags, import_name);
    if (is_func) {
      return 1;
    } else if (ccmp->lex->cur_tok == ',') {
      Lex(ccmp->lex);
      goto prs_global_loop;
    }
    if (ccmp->lex->cur_tok != ';' && !is_func) {
      ParseErr(ccmp, "Expected ';'");
      return 0;
    }
    if (import_name)
      A_FREE(import_name);
  } else if (PrsKw(ccmp, TK_KW__EXTERN)) {
    flags = PRSF__EXTERN;
    if (ccmp->lex->cur_tok != TK_NAME)
      ParseErr(ccmp, "Expected a extern name.");
    import_name = A_STRDUP(ccmp->lex->string, NULL);
    goto prs_global;
  } else if (PrsKw(ccmp, TK_KW_IMPORT)) {
    flags = PRSF_IMPORT;
    goto prs_global;
  } else if (PrsKw(ccmp, TK_KW__IMPORT)) {
    flags = PRSF__IMPORT;
    if (ccmp->lex->cur_tok != TK_NAME)
      ParseErr(ccmp, "Expected a import name.");
    import_name = A_STRDUP(ccmp->lex->string, NULL);
    goto prs_global;
  }
  if (PrsKw(ccmp, TK_KW_STATIC)) {
    flags |= PRSF_STATIC;
  }
  if (ccmp->lex->cur_tok == TK_NAME) {
    if (cls = PrsClass(ccmp, flags))
      goto mloop;
    if (cls = HashFind(ccmp->lex->string, Fs->hash_table, HTT_CLASS, 1)) {
      Lex(ccmp->lex);
      // Here's the deal,TempleOS lets you put a fallback type a class/union
      // Time to rock like kanYe West
      if (PeekKw(ccmp, TK_KW_CLASS)) {
      ye:
        cls2 = PrsClass(ccmp, 0);
        if (!cls2)
          ParseErr(ccmp, "Expected a class.");
        cls2->fwd_class = cls;
        cls2->raw_type = cls->raw_type;
        cls = cls2;
      } else if (PeekKw(ccmp, TK_KW_UNION)) {
        goto ye;
      }
    mloop:
      if (ccmp->cur_fun) {
        // IF we added a RPN,we assigned(?)
        rpn = ccmp->code_ctrl->ir_code->next;
        PrsDecl(ccmp, cls, ccmp->cur_fun, NULL, flags, NULL);
        if (ccmp->code_ctrl->ir_code->next != rpn)
          assign_cnt++;
      } else
        goto prs_global_loop;
      if (is_func)
        return 1;
      if (ccmp->lex->cur_tok == ',') {
        Lex(ccmp->lex);
        goto mloop;
      }
      if (ccmp->lex->cur_tok != ';' && !is_func) {
        ParseErr(ccmp, "Expected ';'");
        return 0;
      }
      Lex(ccmp->lex);
      return 1;
    }
  }
  if (ccmp->lex->cur_tok == TK_STR) {
    string = PrsString(ccmp, &str_len);
    *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_GLOBAL,
        .global_var = HashFind("Print", Fs->hash_table, HTT_FUN, 1),
    };
    if (!rpn->global_var)
      ParseErr(ccmp, "'Print' function doesnt exist yet.");
    QueIns(rpn, ccmp->code_ctrl->ir_code);
    (rpn = A_CALLOC(sizeof(CRPN), NULL))->type = IC_STR;
    for (misc = ccmp->code_ctrl->code_misc->next;
         ccmp->code_ctrl->code_misc != misc; misc = misc->base.next) {
      if (misc->type == CMT_STRING)
        if (misc->str_len == ccmp->lex->str_len)
          if (0 == memcmp(misc->str, string, str_len)) {
            goto found_str;
          }
    }
    misc = CodeMiscNew(ccmp, CMT_STRING);
    misc->str_len = str_len;
    misc->str = string;
  found_str:
    rpn->code_misc = misc;
    argc = 1;
    QueIns(rpn, ccmp->code_ctrl->ir_code);
    for (;;) {
      switch (ccmp->lex->cur_tok) {
        break;
      case ',':
        Lex(ccmp->lex);
        if (!ParseExpr(ccmp, PEF_NO_COMMA)) {
          ParseErr(ccmp, "Expected an expression,");
        }
        argc++;
        break;
      case ';':
        Lex(ccmp->lex);
        goto fin_str;
        break;
      default:
        ParseErr(ccmp, "Expected an argument.");
      }
    }
  fin_str:
    *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_CALL,
        .length = argc,
    };
    QueIns(rpn, ccmp->code_ctrl->ir_code);
    return 1;
  }
  if (ccmp->lex->cur_tok == ';') {
    Lex(ccmp->lex);
    (rpn = A_CALLOC(sizeof(CRPN), NULL))->type = IC_NOP;
    QueIns(rpn, ccmp->code_ctrl->ir_code);
    return 1;
  }
  if (ParseExpr(ccmp, 0)) {
    if (ccmp->lex->cur_tok != ';')
      ParseErr(ccmp, "Expected a ';'.");
    Lex(ccmp->lex);
    return 1;
  }
  return 0;
}

int64_t PrsStmt(CCmpCtrl *ccmp) {
  CRPN *old = ccmp->code_ctrl->ir_code->next, *cur, *new;
  char ret = _PrsStmt(ccmp);
  if (old == ccmp->code_ctrl->ir_code->next) {
    (old = A_CALLOC(sizeof(CRPN), 0))->type = IC_NOP;
    QueIns(old, ccmp->code_ctrl->ir_code);
  } else if (ICFwd(cur = ccmp->code_ctrl->ir_code->next) != old) {
  }
  return ret;
}
int64_t PrsScope(CCmpCtrl *ccmp) { // YES
  CRPN *ic, *old, *cur;
  int64_t old_cnt;
  if (ccmp->lex->cur_tok == '{') {
    old = ccmp->code_ctrl->ir_code->next;
    Lex(ccmp->lex);
    while (ccmp->lex->cur_tok != '}') {
      if (!PrsStmt(ccmp)) {
        ParseErr(ccmp, "Expected an expression.");
        return 0;
      }
    }
    Lex(ccmp->lex);
    return 1;
  }
  return 0;
}

int64_t PrsI64(CCmpCtrl *ccmp) {
  int64_t res;
  int64_t (*bin)();
  double (*binf)();
  CRPN *ir_code;
  CHashFun *fun = ccmp->cur_fun;
  ccmp->cur_fun = NULL;
  CodeCtrlPush(ccmp);
  ParseExpr(ccmp, PEF_NO_COMMA);
  ir_code = A_CALLOC(sizeof(CRPN), NULL);
  ir_code->type = IC_RET;
  QueIns(ir_code, ccmp->code_ctrl->ir_code);
  bin = Compile(ccmp, NULL, NULL, NULL);
  binf = (void *)bin;
  int old = SetWriteNP(1);
  if (AssignRawTypeToNode(ccmp, ir_code->base.next) != RT_F64)
    res = (*bin)();
  else
    res = (*binf)();
  SetWriteNP(old);
  CodeCtrlPop(ccmp);
  ccmp->cur_fun = fun; // Restore
  return res;
}

double PrsF64(CCmpCtrl *ccmp) {
  double res;
  int64_t (*bin)();
  double (*binf)();
  CRPN *ir_code;
  CHashFun *fun = ccmp->cur_fun;
  ccmp->cur_fun = NULL;
  CodeCtrlPush(ccmp);
  ParseExpr(ccmp, PEF_NO_COMMA);
  ir_code = A_CALLOC(sizeof(CRPN), NULL);
  ir_code->type = IC_RET;
  QueIns(ir_code, ccmp->code_ctrl->ir_code);
  bin = Compile(ccmp, NULL, NULL, NULL);
  binf = (void *)bin;
  int old = SetWriteNP(1);
  if (AssignRawTypeToNode(ccmp, ir_code->base.next) != RT_F64)
    res = (*bin)();
  else
    res = (*binf)();
  SetWriteNP(old);
  CodeCtrlPop(ccmp);
  ccmp->cur_fun = fun;
  return res;
}

int64_t PrsArrayDim(CCmpCtrl *ccmp, CArrayDim *to) {
  int64_t dim;
  // CDimArray.next!=NULL if array
  to->total_cnt = 1;
  to->next = NULL;
  if (ccmp->lex->cur_tok == '[') {
    Lex(ccmp->lex);
    dim = PrsI64(ccmp);
    if (ccmp->lex->cur_tok != ']') {
      ParseErr(ccmp, "Expected a ']'.");
      return 0;
    } else
      Lex(ccmp->lex);
    to->next = A_CALLOC(sizeof(CArrayDim), NULL);
    to->next->total_cnt = 1;
    to->next->cnt = dim;
    to->total_cnt *= dim;
    if (PrsArrayDim(ccmp, to->next))
      to->total_cnt *= to->next->total_cnt;
    return 1;
  }
  return 0;
}

// Be sure to assign members
static void MemberAdd(CCmpCtrl *ccmp, CMemberLst *lst, CHashClass *cls) {
  CMemberLst **ptr = &cls->members_lst;
  CMemberLst *last = cls->members_lst;
  while (*ptr) {
    last = *ptr;
    ptr = &ptr[0]->next;
  }
  *ptr = lst;
  lst->last = last;
  cls->member_cnt++;
}
int64_t PrsFunArgs(CCmpCtrl *ccmp, CHashFun *fun) {
  CMemberLst *bungis;
  CHashClass *cls;
  int64_t idx = 0;
  if (ccmp->lex->cur_tok != '(') {
    ParseErr(ccmp, "Expected a '('.");
    return 0;
  } else
    Lex(ccmp->lex);
  idx = 0;
  while (ccmp->lex->cur_tok != ')') {
    if (ccmp->lex->cur_tok == TK_NAME) {
      if (!(cls = HashFind(ccmp->lex->string, Fs->hash_table, HTT_CLASS, 1))) {
        ParseErr(ccmp, "Expected a type name.");
        return 0;
      }
      Lex(ccmp->lex);
      PrsDecl(ccmp, cls, fun, NULL, PRSF_FUN_ARGS, NULL);
      fun->argc++;
      if (ccmp->lex->cur_tok == ',') {
        Lex(ccmp->lex);
        continue;
      } else if (ccmp->lex->cur_tok == ')') {
        break;
      } else
        goto func_err;
    } else if (ccmp->lex->cur_tok == TK_DOT_DOT_DOT) {
      fun->base.flags |= CLSF_VARGS;
      bungis = A_CALLOC(sizeof(CMemberLst), NULL);
      bungis->dim.total_cnt =
          1; // MUST HAVE A DIM OF 1 TO NOT BE ZERO(Nroot was here)
      bungis->reg = REG_MAYBE;
      bungis->member_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
      assert(bungis->member_class);
      bungis->str = A_STRDUP("argc", NULL);
      MemberAdd(ccmp, bungis, fun);
      bungis = A_CALLOC(sizeof(CMemberLst), NULL);
      bungis->reg = REG_MAYBE;
      bungis->member_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
      assert(bungis->member_class);
      bungis->member_class++;
      bungis->str = A_STRDUP("argv", NULL);
      MemberAdd(ccmp, bungis, fun);
      bungis->dim.total_cnt =
          1; // MUST HAVE A DIM OF 1 TO NOT BE ZERO(Nroot was here)
      Lex(ccmp->lex);
      fun->argc += 2;
      if (ccmp->lex->cur_tok == ')') {
        break;
      }
      goto func_err;
    } else {
    func_err:
      ParseErr(ccmp, "Expected ')'.");
      return 0;
    }
  }
  Lex(ccmp->lex);
  return 1;
}
CHashClass *PrsType(CCmpCtrl *ccmp, CHashClass *base, char **name,
                    CHashFun **fun, CArrayDim *dim) {
  int64_t star_cnt = 0, star_cnt2 = 0;
  int64_t is_func_ptr = 0;
  if (name)
    *name = NULL;
  if (fun)
    *fun = NULL;
  if (dim) {
    memset(dim, 0, sizeof(CArrayDim));
    dim->total_cnt = 1;
  }
  while (ccmp->lex->cur_tok == '*')
    Lex(ccmp->lex), star_cnt++;
  if (star_cnt >= STAR_CNT)
    ParseErr(ccmp, "Too many pointer stars!");
  is_func_ptr = ccmp->lex->cur_tok == '(';
  if (is_func_ptr) {
    Lex(ccmp->lex);
    star_cnt2 = 0;
    while (ccmp->lex->cur_tok == '*')
      Lex(ccmp->lex), star_cnt2++;
    if (star_cnt >= STAR_CNT)
      ParseErr(ccmp, "Too many pointer stars!");
  }
  if (ccmp->lex->cur_tok == TK_NAME) {
    if (name)
      *name = A_STRDUP(ccmp->lex->string, NULL);
    Lex(ccmp->lex);
  }
  PrsArrayDim(ccmp, dim);
  if (is_func_ptr) {
    if (ccmp->lex->cur_tok != ')') {
      ParseErr(ccmp, "Expected a ')'!");
    } else
      Lex(ccmp->lex);
    assert(fun);
    fun[0] = A_CALLOC(sizeof(CHashFun), NULL);
    fun[0]->base.flags |= CLSF_FUNPTR;
    fun[0]->return_class = base + star_cnt;
    fun[0]->base.raw_type = RT_FUNC;
    fun[0]->base.sz = sizeof(void *);
    fun[0]->base.ptr_star_cnt = star_cnt2;
    PrsFunArgs(ccmp, *fun);
    return *fun;
  }
  return base + star_cnt;
}
//
// Array declarations within functions are not permited in TempleOS,so we just
// write into the
//  array's data directy
//
char *PrsArray(CCmpCtrl *ccmp, CHashClass *base, CArrayDim *dim,
               char *write_to) {
  CMemberLst *cur_mem;
  int64_t i, cap, ptr_width = base->sz;
  char *string;
  if (dim)
    if (dim->next)
      ptr_width *= dim->next->total_cnt;
  if (ccmp->lex->cur_tok != '{') {
    if (dim) {
      if (base->raw_type == RT_U8i && ccmp->lex->cur_tok == TK_STR) {
        cap = dim->cnt;
        string = PrsString(ccmp, &i);
        if (i + 1 > cap) {
          ParseWarn(ccmp, "String excedes array dim.");
          memcpy(write_to, string, cap);
        } else {
          memcpy(write_to, string, i + 1);
        }
        i = cap;
        A_FREE(string);
        return write_to + i;
      }
      ParseErr(ccmp, "Expected a value for array.");
    }
    if (base->raw_type == RT_F64)
      *(double *)write_to = PrsF64(ccmp);
    else {
      i = PrsI64(ccmp);
      switch (ptr_width) {
        break;
      case 1:
        *(int8_t *)write_to = i;
        break;
      case 2:
        *(int16_t *)write_to = i;
        break;
      case 4:
        *(int32_t *)write_to = i;
        break;
      case 8:
        *(int64_t *)write_to = i;
      }
    }
    return write_to + ptr_width;
  } else
    Lex(ccmp->lex);
  if (dim) {
    cap = dim->cnt;
    for (i = 0; i != cap; i++) {
      write_to = PrsArray(ccmp, base, dim->next, write_to);
      if (ccmp->lex->cur_tok != ',') {
        if (i + 1 == cap) {
          // I64 arr[3]={1,2,3,};
          // All is good,not expecting other item
        } else
          ParseErr(ccmp, "Expected another array element.");
      } else
        Lex(ccmp->lex);
    }
  } else {
    for (cur_mem = base->members_lst; cur_mem; cur_mem = cur_mem->next) {
      PrsArray(ccmp, cur_mem->member_class, cur_mem->dim.next,
               write_to + cur_mem->off);
      if (ccmp->lex->cur_tok != ',') {
        if (!cur_mem->next) {
          // class {I64 a,b,c;}={1,2,3,};
          // All is good,not expecting other item
        } else
          ParseErr(ccmp, "Expected another member element.");
      } else
        Lex(ccmp->lex);
    }
    write_to += base->sz;
  }
  if (ccmp->lex->cur_tok != '}')
    ParseErr(ccmp, "Expected a '}'.");
  Lex(ccmp->lex);
  return write_to;
}
int64_t PrsDecl(CCmpCtrl *ccmp, CHashClass *base, CHashClass *add_to,
                int64_t *is_func_decl, int64_t flags, char *import_name) {
  if (is_func_decl)
    *is_func_decl = 0;
  int64_t reg = REG_MAYBE, is_fun = 0, used = 0, tmpi;
  CArrayDim dim;
  CHashClass *cls;
  CMemberLst *lst, *bungis;
  CHashFun *fun, *extern_fun;
  CHashGlblVar *glbl_var, *import_var, *extern_glbl;
  CRPN *rpn;
  char *name;
  double tmpf;
  if (!(flags & PRSF_CLASS)) {
    if (PrsKw(ccmp, TK_KW_NOREG))
      reg = REG_NONE;
    else if (PrsKw(ccmp, TK_KW_REG)) {
      reg = REG_ALLOC;
      if (ccmp->lex->cur_tok == TK_I64) {
        reg = ccmp->lex->integer;
        Lex(ccmp->lex);
      }
    }
  }
  cls = PrsType(ccmp, base, &name, &fun, &dim);
  lst = A_CALLOC(sizeof(CMemberLst), NULL);
  lst->reg = reg;
  if (name)
    lst->str = name;
  lst->member_class = cls;
  lst->dim = dim;
  lst->member_class->use_cnt++;
  lst->fun_ptr = fun;
  if (ccmp->lex->cur_tok == '(') {
    if (ccmp->cur_fun) {
      ParseErr(ccmp, "No nested functions!!!");
      return 0;
    }
    *(fun = ccmp->cur_fun = A_CALLOC(sizeof(CHashFun), NULL)) =
        (CHashFun){.base =
                       {
                           .base =
                               {
                                   .str = A_STRDUP(name, NULL),
                                   .type = HTT_FUN,
                               },
                           .raw_type = RT_FUNC,
                       },
                   .fun_ptr = &DoNothing,
                   .return_class = cls};
    HashAdd(fun, Fs->hash_table);
    if (flags & (PRSF_EXTERN | PRSF__EXTERN)) {
      fun->base.base.type |= HTF_EXTERN;
    } else if (flags & (PRSF_IMPORT | PRSF__IMPORT)) {
      fun->base.base.type |= HTF_IMPORT;
    }
  found_fun:
    if (flags & (PRSF__EXTERN | PRSF__IMPORT)) {
      fun->import_name = A_STRDUP(import_name, NULL);
    }
    is_fun = 1;
    PrsFunArgs(ccmp, fun);
  }
  if (is_fun) {
    if (fun->base.base.type & (HTF_EXTERN | HTF_IMPORT)) {
      if (ccmp->lex->cur_tok == '{') {
        ParseErr(ccmp, "Function body on a extern/import.");
        return 0;
      }
      if (is_func_decl)
        *is_func_decl = 1;
      ccmp->cur_fun = NULL;
      goto ret;
    } else {
      // TempleOS doesn't allow C style forward declarations and will fill a
      // function Will an "empty" body,AIWN will force function bodies during
      // the Bootstraping phase
      if (ccmp->lex->cur_tok != '{') {
        ParseErr(ccmp, "Expected a function body.");
        return 0;
      }
    }
    CodeCtrlPush(ccmp);
    PrsScope(ccmp);
    //
    // Here's the deal. The function will stay "extern" so if it call's
    // itself a CMT_RELOC_U64 will be made and the function address will be
    // filled in later
    //
    ccmp->cur_fun->base.base.type |= HTF_EXTERN;
    ccmp->cur_fun->fun_ptr = Compile(ccmp, NULL, NULL, NULL);
    ccmp->cur_fun->base.base.type &= ~HTF_EXTERN;
    if (extern_fun = HashFind(name, Fs->hash_table, HTT_FUN,
                              2)) { // Pick 2nd EXTERN(?) function
      extern_fun->fun_ptr = ccmp->cur_fun->fun_ptr;
    }
    if (ccmp->cur_fun->base.base.str)
      SysSymImportsResolve(ccmp->cur_fun->base.base.str, 0);
    CodeCtrlPop(ccmp);
    ccmp->cur_fun = NULL;
    if (is_func_decl)
      *is_func_decl = 1;
    goto ret;
  }
  if (add_to) {
    if (flags & PRSF_STATIC)
      lst->flags |= MLF_STATIC;
    MemberAdd(ccmp, lst, add_to);
    used = 1;
  }
  if (ccmp->lex->cur_tok == '=') {
    //
    // Here's the Donald Trump deal. I will handle global assigns later
    //
    if ((flags & PRSF_FUN_ARGS) || (flags & PRSF_STATIC)) {
      Lex(ccmp->lex);
      lst->flags |= MLF_DFT_AVAILABLE;
      if (!lst->dim.next) {
        switch (lst->member_class->raw_type) {
          break;
        case RT_U8i:
        case RT_I8i:
        case RT_U16i:
        case RT_I16i:
        case RT_U32i:
        case RT_I32i:
        case RT_U64i:
        case RT_I64i:
        case RT_PTR:
        case RT_FUNC: // func ptr
          tmpi = lst->dft_val = PrsI64(ccmp);
          if (flags & PRSF_STATIC) {
            lst->static_bytes = A_CALLOC(8, NULL);
            *(int64_t *)lst->static_bytes = tmpi;
          }
          break;
        case RT_F64:
          tmpf = lst->dft_val_flt = PrsF64(ccmp);
          if (flags & PRSF_STATIC) {
            lst->static_bytes = A_CALLOC(lst->member_class->sz, NULL);
            *(double *)lst->static_bytes = tmpf;
          }
          break;
        default:
          if (flags & PRSF_STATIC)
            goto static_array;
        }
      } else {
      static_array: // Also applies to classes
        lst->static_bytes =
            A_CALLOC(lst->dim.total_cnt * lst->member_class->sz, NULL);
        PrsArray(ccmp, lst->member_class, lst->dim.next, lst->static_bytes);
      }
    } else if (add_to) { // TODO is a local???
      Lex(ccmp->lex);
      *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_LOCAL,
          .local_mem = lst,
      };
      QueIns(rpn, ccmp->code_ctrl->ir_code);
      ParseExpr(ccmp, PEF_NO_COMMA);
      (rpn = A_CALLOC(sizeof(CRPN), NULL))->type = IC_EQ;
      QueIns(rpn, ccmp->code_ctrl->ir_code);
    }
  }
  if (!add_to && lst->str) {
    // Is a global?
    glbl_var = A_CALLOC(sizeof(CHashGlblVar), NULL);
    glbl_var->base.str = lst->str; // we "steal" this from lst;
    lst->str = NULL;
    glbl_var->base.type = HTT_GLBL_VAR;
    glbl_var->var_class = lst->member_class;
    glbl_var->dim = lst->dim;
    lst->dim.next = NULL; // We steal this
    glbl_var->fun_ptr = lst->fun_ptr;
    lst->fun_ptr = NULL; // We steal this
    // lst will be Free'd at ret
    if (!(flags & PRSF_EXTERN))
      glbl_var->data_addr =
          A_CALLOC(glbl_var->var_class->sz * glbl_var->dim.total_cnt, NULL);
    if (flags & PRSF_IMPORT) {
      glbl_var->base.type |= HTF_IMPORT;
      goto set_import_name;
    } else if (flags & PRSF__IMPORT) {
      glbl_var->base.type |= HTF_IMPORT;
    set_import_name:
      // Ensure the import exists
      if (import_var = HashFind(import_name, Fs->hash_table,
                                HTT_FUN | HTT_GLBL_VAR, 1)) {
      } else {
        ParseErr(ccmp, "Can't import;missing symbol '%s'.", import_name);
        return 0;
      }
      //
      if (glbl_var->base.type & HTT_GLBL_VAR) {
        glbl_var->import_name = A_STRDUP(import_name, NULL);
      } else if (glbl_var->base.type & HTT_FUN) {
        ((CHashFun *)glbl_var)->import_name = A_STRDUP(import_name, NULL);
      }
    } else if (flags & PRSF_EXTERN) {
      glbl_var->base.type |= HTF_EXTERN;
    } else if (flags & PRSF__EXTERN) {
      glbl_var->base.type |= HTF_EXTERN;
      if (glbl_var->base.type & HTT_GLBL_VAR) {
        glbl_var->import_name = A_STRDUP(import_name, NULL);
      } else if (glbl_var->base.type & HTT_FUN) {
        ((CHashFun *)glbl_var)->import_name = A_STRDUP(import_name, NULL);
      }
    }
    HashAdd(glbl_var, Fs->hash_table);
  found_glbl:
    if (!(flags & PRSF_EXTERN)) {
      if (extern_glbl = HashFind(glbl_var->base.str, Fs->hash_table,
                                 HTT_GLBL_VAR, 2)) { // Pick 2nd EXTERN(?) Var
        extern_glbl->data_addr = glbl_var->data_addr;
      }
    }
    if (!(flags & PRSF_EXTERN))
      SysSymImportsResolve(glbl_var->base.str, 0);
    if (ccmp->lex->cur_tok == '=') {
      Lex(ccmp->lex);
      if (ccmp->lex->cur_tok == '{') {
        PrsArray(ccmp, glbl_var->var_class, glbl_var->dim.next,
                 glbl_var->data_addr);
      } else {
        *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
            .type = IC_GLOBAL,
            .global_var = glbl_var,
        };
        QueIns(rpn, ccmp->code_ctrl->ir_code);
        ParseExpr(ccmp, PEF_NO_COMMA);
        (rpn = A_CALLOC(sizeof(CRPN), NULL))->type = IC_EQ;
        QueIns(rpn, ccmp->code_ctrl->ir_code);
      }
    }
    goto ret;
  }
ret:
  if (!used)
    MemberLstDel(lst);
  return 1;
}

int64_t PrsWhile(CCmpCtrl *ccmp) { // YES
  CRPN *ic;
  CCodeMisc *old_break_to, *break_to, *enter_label;
  if (PrsKw(ccmp, TK_KW_WHILE)) {
    old_break_to = ccmp->code_ctrl->break_to; // Restored
    break_to = ccmp->code_ctrl->break_to = CodeMiscNew(ccmp, CMT_LABEL);
    enter_label = CodeMiscNew(ccmp, CMT_LABEL);
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_LABEL,
        .code_misc = enter_label,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    if (ccmp->lex->cur_tok != '(')
      ParseErr(ccmp, "Expected a '('.");
    Lex(ccmp->lex);
    if (!ParseExpr(ccmp, 0))
      ParseErr(ccmp, "Expected expression.");
    (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_LNOT;
    QueIns(ic, ccmp->code_ctrl->ir_code);
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_GOTO_IF,
        .code_misc = break_to,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    if (ccmp->lex->cur_tok != ')')
      ParseErr(ccmp, "Expected a ')'.");
    Lex(ccmp->lex);
    if (!PrsStmt(ccmp)) {
      ParseErr(ccmp, "Expected expression.");
    }
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_GOTO,
        .code_misc = enter_label,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_LABEL,
        .code_misc = break_to,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    ccmp->code_ctrl->break_to = old_break_to;
    return 1;
  }
  return 0;
}

int64_t PrsDo(CCmpCtrl *ccmp) {
  CRPN *ic;
  CCodeMisc *old_break_to, *enter_label;
  if (PrsKw(ccmp, TK_KW_DO)) {
    enter_label = CodeMiscNew(ccmp, CMT_LABEL);
    old_break_to = ccmp->code_ctrl->break_to; // Restored
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_LABEL,
        .code_misc = enter_label,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    ccmp->code_ctrl->break_to = CodeMiscNew(ccmp, CMT_LABEL);
    PrsStmt(ccmp);
    if (!PrsKw(ccmp, TK_KW_WHILE))
      ParseErr(ccmp, "Expected a 'while'.");
    if (ccmp->lex->cur_tok != '(')
      ParseErr(ccmp, "Expected a '('.");
    Lex(ccmp->lex);
    if (!ParseExpr(ccmp, 0))
      ParseErr(ccmp, "Expected expression.");
    if (ccmp->lex->cur_tok != ')')
      ParseErr(ccmp, "Expected a ')'.");
    Lex(ccmp->lex);
    if (ccmp->lex->cur_tok != ';')
      ParseErr(ccmp, "Expected a ';'.");
    Lex(ccmp->lex);
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_GOTO_IF,
        .code_misc = enter_label,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_LABEL,
        .code_misc = ccmp->code_ctrl->break_to,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    ccmp->code_ctrl->break_to = old_break_to;
    return 1;
  }
  return 0;
}

int64_t PrsFor(CCmpCtrl *ccmp) { // YES
  CRPN *ic, *nop;
  CCodeMisc *break_to, *old_break_to, *enter_label, *exit_label;
  CCodeCtrl *cctrl = NULL;
  int64_t idx;
  if (PrsKw(ccmp, TK_KW_FOR)) {
    old_break_to = ccmp->code_ctrl->break_to; //.Restored
    break_to = CodeMiscNew(ccmp, CMT_LABEL);
    ccmp->code_ctrl->break_to = break_to;
    if (ccmp->lex->cur_tok != '(')
      ParseErr(ccmp, "Expected a '('.");
    Lex(ccmp->lex);
    if (!ParseExpr(ccmp, 0)) {
      // Do nothing
    }
    if (ccmp->lex->cur_tok != ';')
      ParseErr(ccmp, "Expected a ';'.");
    Lex(ccmp->lex);
    enter_label = CodeMiscNew(ccmp, CMT_LABEL);
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_LABEL,
        .code_misc = enter_label,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    if (!ParseExpr(ccmp, 0)) {
      ParseErr(ccmp, "Expected a condition.");
    }
    (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_LNOT;
    QueIns(ic, ccmp->code_ctrl->ir_code);
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_GOTO_IF,
        .code_misc = break_to,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    if (ccmp->lex->cur_tok != ';')
      ParseErr(ccmp, "Expected a ';'.");
    Lex(ccmp->lex);
    if (ccmp->lex->cur_tok != ')') {
      CodeCtrlPush(ccmp);
      ParseExpr(ccmp, 0);
      cctrl = CodeCtrlPopNoFree(ccmp);
    }
    if (ccmp->lex->cur_tok != ')')
      ParseErr(ccmp, "Expected a ')'.");
    Lex(ccmp->lex);
    PrsStmt(ccmp);
    if (cctrl) {
      CodeCtrlAppend(ccmp, cctrl);
      CodeCtrlDel(cctrl);
    }
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_GOTO,
        .code_misc = enter_label,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_LABEL,
        .code_misc = break_to,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    ccmp->code_ctrl->break_to = old_break_to;
    return 1;
  }
  return 0;
}

int64_t PrsIf(CCmpCtrl *ccmp) {
  CRPN *ic;
  CCodeMisc *misc, *misc2;
  if (PrsKw(ccmp, TK_KW_IF)) {
    if (ccmp->lex->cur_tok != '(')
      ParseErr(ccmp, "Expected a '('.");
    Lex(ccmp->lex);
    ParseExpr(ccmp, 0);
    (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_LNOT;
    QueIns(ic, ccmp->code_ctrl->ir_code);
    (ic = A_CALLOC(sizeof(CRPN), NULL))->type = IC_GOTO_IF;
    QueIns(ic, ccmp->code_ctrl->ir_code);
    misc = CodeMiscNew(ccmp, CMT_LABEL);
    ic->code_misc = misc;
    if (ccmp->lex->cur_tok != ')')
      ParseErr(ccmp, "Expected a ')'.");
    Lex(ccmp->lex);
    PrsStmt(ccmp);
    if (PrsKw(ccmp, TK_KW_ELSE)) {
      misc2 = CodeMiscNew(ccmp, CMT_LABEL);
      *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_GOTO,
          .code_misc = misc2,
      };
      QueIns(ic, ccmp->code_ctrl->ir_code);
      *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_LABEL,
          .code_misc = misc,
      };
      QueIns(ic, ccmp->code_ctrl->ir_code);
      PrsStmt(ccmp);
      *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_LABEL,
          .code_misc = misc2,
      };
      QueIns(ic, ccmp->code_ctrl->ir_code);
    } else {
      *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_LABEL,
          .code_misc = misc,
      };
      QueIns(ic, ccmp->code_ctrl->ir_code);
    }
    return 1;
  }
  return 0;
}

#define BETWEEN(a, min, max) ((a) >= min) && ((a) <= max)
int64_t AssignRawTypeToNode(CCmpCtrl *ccmp, CRPN *rpn) {
  int64_t a, b, arg;
  CMemberLst *mlst;
  CHashFun *fun;
  char *name;
  CRPN *orig_rpn = rpn;
  if (rpn->raw_type)
    return rpn->raw_type;
  switch (rpn->type) {
  case IC_SQRT:
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 0));
    rpn->ic_class = HashFind("F64", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = RT_F64;
    break;
  case IC_MAX_I64:
  case IC_MIN_I64:
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 0));
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 1));
    rpn->ic_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = RT_I64i;
    break;
  case IC_MIN_U64:
  case IC_MAX_U64:
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 0));
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 1));
    rpn->ic_class = HashFind("U64i", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = RT_U64i;
    break;
  case IC_MIN_F64:
  case IC_MAX_F64:
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 0));
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 1));
    rpn->ic_class = HashFind("F64", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = RT_F64;
    break;
  case IC_FS:
  case IC_GS:
    rpn->ic_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = RT_I64i;
    break;
  case IC_LOCK: // Lock means "lock *ptr++;",it operates on expressions as a
                // whole
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 0));
    return rpn->raw_type = RT_U0; // dummy
    break;
  case IC_BT:
  case IC_BTR:
  case IC_BTS:
  case IC_BTC:
  case IC_LBTR:
  case IC_LBTS:
  case IC_LBTC:
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 1));
    AssignRawTypeToNode(ccmp, ICArgN(rpn, 0));
    rpn->ic_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = RT_I64i;
    break;
  case IC_SHORT_ADDR:
  case IC_RELOC:
    rpn->ic_class = HashFind("U8i", Fs->hash_table, HTT_CLASS, 1);
    rpn->ic_class++;
    return rpn->raw_type = RT_PTR;
    break;
  case __IC_VARGS:
    rpn->ic_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = RT_I64i;
    break;
  case IC_TO_BOOL:
    AssignRawTypeToNode(ccmp, rpn->base.next);
  case IC_TO_I64:
    rpn->ic_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = RT_I64i;
    break;
  case IC_TO_F64:
    rpn->ic_class = HashFind("F64", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = RT_F64;
    break;
  case IC_TYPECAST:
    return rpn->raw_type = rpn->ic_class->raw_type;
    break;
  case __IC_CALL:
    rpn = orig_rpn->base.next;
    for (arg = 0; arg != orig_rpn->length + 1; arg++) {
      if (rpn->type != IC_NOP)
        AssignRawTypeToNode(ccmp, rpn);
      rpn = ICFwd(rpn);
    }
    return rpn->raw_type = rpn->ic_class;
  case IC_CALL:
    rpn = orig_rpn->base.next;
    for (arg = 0; arg != orig_rpn->length; arg++) {
      if (rpn->type != IC_NOP)
        AssignRawTypeToNode(ccmp, rpn);
      rpn = ICFwd(rpn);
    }
    if (RT_PTR == AssignRawTypeToNode(ccmp, rpn)) {
      if (rpn->ic_class->ptr_star_cnt > 1)
        ParseErr(ccmp, "Can't call this pointer."); // TODO add type name
    }
    fun = rpn->ic_fun;
  got_fun:
    if (!fun)
      ParseErr(ccmp, "Invalid type to call."); // TODO add type name
    if (arg > fun->argc && !(fun->base.flags & CLSF_VARGS)) {
      ParseErr(ccmp, "Excess arguments for function");
    }
    mlst = fun->base.members_lst;
    rpn = orig_rpn->base.next;
    for (arg = 0; arg != orig_rpn->length; arg++) {
      if (arg >= fun->argc)
        break;
      rpn = ICArgN(orig_rpn, orig_rpn->length - arg - 1);
      if (rpn->type == IC_NOP)
        if (!(mlst->flags & MLF_DFT_AVAILABLE)) {
          ParseErr(ccmp, "No default value for argument %d.", arg);
        }
      mlst = mlst->next;
    }
    for (; arg < fun->argc; arg++) {
      if (fun->base.flags & CLSF_VARGS && arg >= fun->argc - 2)
        break;
      if (!(mlst->flags & MLF_DFT_AVAILABLE)) {
        ParseErr(ccmp, "No default value for argument %d.", arg);
      }
      mlst = mlst->next;
    }
    orig_rpn->ic_class = fun->return_class;
    orig_rpn->ic_dim = NULL;
    orig_rpn->raw_type = orig_rpn->ic_class->raw_type;
    break;
  case IC_COMMA:
    AssignRawTypeToNode(ccmp, rpn->base.next);
    AssignRawTypeToNode(ccmp, ICFwd(rpn->base.next));
    rpn->ic_class = ((CRPN *)rpn->base.next)->ic_class;
    rpn->ic_dim = ((CRPN *)rpn->base.next)->ic_dim;
    rpn->ic_fun = ((CRPN *)rpn->base.next)->ic_fun;
    return rpn->raw_type = ((CRPN *)rpn->base.next)->raw_type;
    break;
  case IC_GLOBAL:
    if (rpn->global_var->base.type & HTT_GLBL_VAR) {
      rpn->ic_class = rpn->global_var->var_class;
      rpn->ic_dim = rpn->global_var->dim.next;
      rpn->ic_fun = rpn->global_var->fun_ptr;
      return rpn->raw_type = rpn->ic_class->raw_type;
    } else if (rpn->global_var->base.type & HTT_FUN) {
      fun = rpn->global_var;
      rpn->ic_class = &fun->base;
      rpn->ic_dim = NULL;
      rpn->ic_fun = fun;
      return RT_FUNC;
    }
    abort();
    break;
  case IC_LOCAL:
    rpn->ic_class = rpn->local_mem->member_class;
    rpn->ic_dim = rpn->local_mem->dim.next;
    rpn->ic_fun = rpn->local_mem->fun_ptr;
    return rpn->raw_type = rpn->ic_class->raw_type;
    break;
  case IC_NEG:
  unop:
    AssignRawTypeToNode(ccmp, rpn->base.next);
    if (BETWEEN(((CRPN *)rpn->base.next)->raw_type, RT_I8i, RT_U32i)) {
      // Promote to 64bit
      rpn->raw_type = RT_I64i;
    } else {
      rpn->raw_type = ((CRPN *)rpn->base.next)->raw_type;
      rpn->ic_class = ((CRPN *)rpn->base.next)->ic_class;
      rpn->ic_fun = ((CRPN *)rpn->base.next)->ic_fun;
      rpn->ic_dim = NULL;
    }
    goto ret;
    break;
  case IC_POS:
    goto unop;
    break;
  case IC_NAME:
    abort();
    break;
  case IC_STR:
    rpn->raw_type = RT_PTR;
    goto ret;
    break;
  case IC_CHR:
    rpn->raw_type = RT_U64i;
    goto ret;
    break;
  case IC_POW:
    rpn->raw_type = RT_F64;
    goto ret;
    break;
  case IC_ADD:
    a = AssignRawTypeToNode(ccmp, ICFwd(rpn->base.next));
    b = AssignRawTypeToNode(ccmp, rpn->base.next);
    if (a == RT_PTR || b == RT_PTR) {
      if (a == RT_PTR) {
        rpn->raw_type = RT_PTR;
        rpn->ic_class = ICFwd(rpn->base.next)->ic_class;
        rpn->ic_dim = ICFwd(rpn->base.next)->ic_dim;
        rpn->ic_fun = ICFwd(rpn->base.next)->ic_fun;
      } else {
        rpn->raw_type = RT_PTR;
        rpn->ic_class = ((CRPN *)rpn->base.next)->ic_class;
        rpn->ic_fun = ((CRPN *)rpn->base.next)->ic_fun;
        rpn->ic_dim = ((CRPN *)rpn->base.next)->ic_dim;
      }
      return rpn->raw_type;
    } else if (BETWEEN(a, RT_I8i, RT_U32i) && BETWEEN(a, RT_I8i, RT_U32i)) {
      // Promote to I64i
      a = RT_I64i;
    }
    rpn->raw_type = (a > b) ? a : b;
    goto ret;
    break;
  case IC_EQ:
    goto abinop;
    break;
  case IC_SUB:
    a = AssignRawTypeToNode(ccmp, ICFwd(rpn->base.next));
    b = AssignRawTypeToNode(ccmp, rpn->base.next);
    // PTR-idx
    if (a == RT_PTR) {
      rpn->raw_type = RT_PTR;
      rpn->ic_class = ICFwd(rpn->base.next)->ic_class;
      rpn->ic_dim = ICFwd(rpn->base.next)->ic_dim;
      rpn->ic_fun = ICFwd(rpn->base.next)->ic_fun;
      return rpn->raw_type;
    } else if (BETWEEN(a, RT_I8i, RT_U32i) && BETWEEN(a, RT_I8i, RT_U32i)) {
      // Promote to I64i
      a = RT_I64i;
    }
    rpn->raw_type = (a > b) ? a : b;
    goto ret;
    break;
  case IC_DIV:
  binop:
    a = AssignRawTypeToNode(ccmp, ICFwd(rpn->base.next));
    b = AssignRawTypeToNode(ccmp, rpn->base.next);
    if (BETWEEN(a, RT_I8i, RT_U32i) && BETWEEN(b, RT_I8i, RT_U32i)) {
      // Promote to I64i
      a = RT_I64i;
    }
    rpn->raw_type = (a > b) ? a : b;
    goto ret;
    break;
  case IC_MUL:
    goto binop;
    break;
  case IC_DEREF:
    AssignRawTypeToNode(ccmp, rpn->base.next);
    if (!((CRPN *)rpn->base.next)->ic_class->ptr_star_cnt) {
      ParseErr(ccmp, "Can't derefernce a non-pointer/array.");
      return 0;
    }
    rpn->ic_dim = ((CRPN *)rpn->base.next)->ic_dim;
    rpn->ic_fun = ((CRPN *)rpn->base.next)->ic_fun;
    if (!rpn->ic_dim)
      goto no_dim;
    if (!rpn->ic_dim->next) {
    no_dim:
      rpn->ic_class = ((CRPN *)rpn->base.next)->ic_class - 1;
    } else {
      rpn->ic_dim = rpn->ic_dim->next;
    }
    rpn->raw_type = rpn->ic_class->raw_type;
    return rpn->raw_type;
    break;
  case IC_AND:
    goto binop;
    break;
  case IC_SQR:
    AssignRawTypeToNode(ccmp, rpn->base.next);
    rpn->ic_class = HashFind("F64", Fs->hash_table, HTT_CLASS, 1);
    return rpn->raw_type = rpn->ic_class->raw_type;
    break;
  case IC_DOT:
    AssignRawTypeToNode(ccmp, rpn->base.next);
    if (((CRPN *)rpn->base.next)->ic_class->ptr_star_cnt ||
        ((CRPN *)rpn->base.next)->ic_dim) {
      ParseErr(ccmp, "Must not be a pointer to use a '.' operator.");
      return 0;
    }
    rpn->ic_class = rpn->local_mem->member_class;
    rpn->ic_dim = rpn->local_mem->dim.next;
    rpn->ic_fun = rpn->local_mem->fun_ptr;
    return rpn->raw_type = rpn->ic_class->raw_type;
    break;
  case IC_ADDR_OF:
    AssignRawTypeToNode(ccmp, rpn->base.next);
    if (((CRPN *)rpn->base.next)->ic_class->ptr_star_cnt >= STAR_CNT) {
      ParseErr(ccmp, "Too many pointer stars.");
      return 0;
    }
    if (((CRPN *)rpn->base.next)->ic_dim) {
      rpn->ic_class = ((CRPN *)rpn->base.next)->ic_class;
      rpn->ic_dim = ((CRPN *)rpn->base.next)->ic_dim;
    } else {
      rpn->ic_class = ((CRPN *)rpn->base.next)->ic_class;
      if (rpn->ic_class->raw_type == RT_FUNC)
        rpn->ic_class = HashFind("U8i", Fs->hash_table, HTT_CLASS, 1);
      rpn->ic_class++;
    }
    rpn->ic_fun = ((CRPN *)rpn->base.next)->ic_fun;
    return rpn->raw_type = RT_PTR;
    break;
  case IC_XOR:
    goto binop;
    break;
  case IC_MOD:
    goto binop;
    break;
  case IC_OR:
    goto binop;
    break;
  case IC_LT:
    rpn->raw_type = RT_I64i;
    goto ret;
    break;
  case IC_GT:
    rpn->raw_type = RT_I64i;
    goto ret;
    break;
  case IC_LNOT:
    rpn->raw_type = RT_I64i;
    goto ret;
    break;
  case IC_BNOT:
    goto unop;
    break;
  case IC_POST_INC:
    goto unop;
    break;
  case IC_POST_DEC:
    goto unop;
    break;
  case IC_PRE_INC:
    goto unop;
    break;
  case IC_PRE_DEC:
    goto unop;
    break;
  case IC_AND_AND:
  cmp:
    rpn->raw_type = RT_I64i;
    goto ret;
    break;
  case IC_OR_OR:
    goto cmp;
    break;
  case IC_XOR_XOR:
    goto cmp;
    break;
  case IC_EQ_EQ:
    goto cmp;
    break;
  case IC_NE:
    goto cmp;
    break;
  case IC_LE:
    goto cmp;
    break;
  case IC_GE:
    goto cmp;
    break;
  case IC_LSH:
    goto binop;
    break;
  case IC_RSH:
    goto binop;
    break;
  case IC_ADD_EQ:
  abinop:
    AssignRawTypeToNode(ccmp, rpn->base.next);
    AssignRawTypeToNode(ccmp, ICFwd(rpn->base.next));
    rpn->ic_class = ICFwd(rpn->base.next)->ic_class;
    rpn->ic_fun = ICFwd(rpn->base.next)->ic_fun;
    rpn->ic_dim = ICFwd(rpn->base.next)->ic_dim;
    rpn->raw_type = rpn->ic_class->raw_type;
    return rpn->raw_type;
    break;
  case IC_SUB_EQ:
    goto abinop;
    break;
  case IC_MUL_EQ:
    goto abinop;
    break;
  case IC_DIV_EQ:
    goto abinop;
    break;
  case IC_LSH_EQ:
    goto abinop;
    break;
  case IC_RSH_EQ:
    goto abinop;
    break;
  case IC_AND_EQ:
    goto abinop;
    break;
  case IC_OR_EQ:
    goto abinop;
    break;
  case IC_XOR_EQ:
    goto abinop;
    break;
  case IC_ARROW:
    AssignRawTypeToNode(ccmp, rpn->base.next);
    rpn->ic_class = rpn->local_mem->member_class;
    rpn->ic_fun = rpn->local_mem->fun_ptr;
    rpn->ic_dim = rpn->local_mem->dim.next;
    rpn->raw_type = rpn->ic_class->raw_type;
    return rpn->raw_type;
    break;
  case IC_MOD_EQ:
    goto abinop;
    break;
  case IC_I64:
    rpn->raw_type = RT_I64i;
    goto ret;
    break;
  case IC_F64:
    rpn->raw_type = RT_F64;
    goto ret;
    break;
  case IC_ARRAY_ACC:
    b = AssignRawTypeToNode(ccmp, rpn->base.next);
    a = AssignRawTypeToNode(ccmp, ICFwd(rpn->base.next));
    if (ICFwd(rpn->base.next)->ic_class->ptr_star_cnt == 0 &&
        !ICFwd(rpn->base.next)->ic_dim) {
      ParseErr(ccmp, "Expected a pointer or array type for array access.");
      return 0;
    }
    if (ICFwd(rpn->base.next)->ic_dim) {
      rpn->ic_class = ICFwd(rpn->base.next)->ic_class;
      rpn->ic_dim = ICFwd(rpn->base.next)->ic_dim->next;
    } else
      rpn->ic_class = ICFwd(rpn->base.next)->ic_class - 1;
    rpn->ic_fun = ICFwd(rpn->base.next)->ic_fun;
    return rpn->raw_type = rpn->ic_class->raw_type;
  }
ret:
  switch (rpn->raw_type) {
    break;
  case RT_U0:
    name = "U0";
    break;
  case RT_I8i:
    name = "I8i";
    break;
  case RT_U8i:
    name = "U8i";
    break;
  case RT_I16i:
    name = "I16i";
    break;
  case RT_U16i:
    name = "U16i";
    break;
  case RT_I32i:
    name = "I32i";
    break;
  case RT_U32i:
    name = "U32i";
    break;
  case RT_I64i:
    name = "I64i";
    break;
  case RT_U64i:
    name = "U64i";
    break;
  case RT_F64:
    name = "F64";
    break;
  case RT_PTR:
    if (rpn->ic_class)
      return rpn->raw_type;
    else
      name = "U8i";
    break;
  default:
    return 0;
  }
  rpn->ic_class = HashFind(name, Fs->hash_table, HTT_CLASS, 1);
  if (rpn->raw_type == RT_PTR)
    rpn->ic_class++;
  return rpn->raw_type;
}

CHashClass *PrsClassNew() {
  int64_t idx2 = 0;
  CHashClass *cls = A_CALLOC((STAR_CNT + 1) * sizeof(CHashClass), NULL);
  for (idx2 = 0; idx2 != STAR_CNT; idx2++) {
    cls[idx2].base.str = NULL;
    cls[idx2].raw_type = RT_PTR;
    cls[idx2].ptr_star_cnt = idx2;
    if (idx2)
      cls[idx2].sz = 8;
    cls[idx2].use_cnt++;
  }
  return cls;
}

static void PrsMembers(CCmpCtrl *cctrl, CHashClass *bungis, int64_t flags,
                       int64_t *off) {
  int64_t is_func, union_end;
  CMemberLst *last_m, *base_class;
  union_end = *off;
  if (cctrl->lex->cur_tok == '{') {
    Lex(cctrl->lex);
    while (cctrl->lex->cur_tok != '}') {
      if (cctrl->lex->cur_tok == ';') {
        Lex(cctrl->lex);
      } else if (PrsKw(cctrl, TK_KW_UNION)) {
        if (cctrl->lex->cur_tok == '{') {
          PrsMembers(cctrl, bungis, flags | PRSF_UNION, off);
        }
      } else if (cctrl->lex->cur_tok == TK_NAME) {
        base_class = HashFind(cctrl->lex->string, Fs->hash_table, HTT_CLASS, 1);
        if (!base_class)
          goto exp_member;
        //
        // Here's the deal,I store the last member in last_m,once I find the
        // members I assign their offsets
        //
        for (last_m = bungis->members_lst; last_m; last_m = last_m->next)
          if (!last_m->next)
            break;
        do {
          Lex(cctrl->lex);
          PrsDecl(cctrl, base_class, bungis, &is_func, PRSF_CLASS, NULL);
        next_meta:
          if (cctrl->lex->cur_tok == TK_NAME) {
            // Nroot will parse the meta data but do nothing with it
            switch (Lex(cctrl->lex)) {
            case TK_STR:
              while (Lex(cctrl->lex) == TK_STR)
                ;
              if (cctrl->lex->cur_tok == ',')
                break;
              else
                goto next_meta;
            case TK_I64:
            case TK_CHR:

              Lex(cctrl->lex);
              if (cctrl->lex->cur_tok == ',')
                break;
              else
                goto next_meta;
              break;
            default:
              ParseErr(cctrl, "Unexpected meta-data!!!");
            }
          }
        } while (cctrl->lex->cur_tok == ',');
        if (last_m)
          last_m = last_m->next;
        else
          last_m = bungis->members_lst;
        for (; last_m; last_m = last_m->next) {
          last_m->off = *off;
          if (!(flags & PRSF_UNION)) {
            *off += last_m->member_class->sz * last_m->dim.total_cnt;
          } else {
            if (union_end <
                *off + last_m->member_class->sz * last_m->dim.total_cnt) {
              union_end =
                  *off + last_m->member_class->sz * last_m->dim.total_cnt;
            }
          }
        }
        if (is_func)
          goto exp_member;
      } else {
      exp_member:
        ParseErr(cctrl, "Expected a member.");
        return;
      }
    }
    Lex(cctrl->lex);
  }
  if (flags & PRSF_UNION)
    *off = union_end;
}

CHashClass *PrsClass(CCmpCtrl *cctrl, int64_t _flags) {
  CHashClass *bungis, *base_class;
  CMemberLst *last_m;
  int64_t flags = PRSF_CLASS;
  int64_t is_func, off = 0, sz, add = 0;
  if (!PrsKw(cctrl, TK_KW_CLASS)) {
    if (PrsKw(cctrl, TK_KW_UNION))
      flags |= PRSF_UNION;
    else
      return NULL;
  }
  if (cctrl->lex->cur_tok == TK_NAME) {
    if (bungis = HashSingleTableFind(cctrl->lex->string, Fs->hash_table,
                                     HTT_CLASS, 1)) {
      if (bungis->base.type & HTF_EXTERN) {
        // We can fill in the extern class
      } else {
        bungis = NULL;
      }
    }
    if (!bungis) {
      bungis = PrsClassNew();
      add = 1;
    }
    if (!bungis->base.str)
      bungis->base.str = A_STRDUP(cctrl->lex->string, NULL);
    bungis->base.type = HTT_CLASS;
    if (_flags & PRSF_EXTERN)
      bungis->base.type |= HTF_EXTERN;
    bungis->src_link = LexSrcLink(cctrl->lex, NULL);
    Lex(cctrl->lex);
  } else {
    ParseErr(cctrl, "Expected a class name.");
  }
  if (cctrl->lex->cur_tok == ':') {
    Lex(cctrl->lex);
    if (cctrl->lex->cur_tok != TK_NAME) {
    want_inher:
      ParseErr(cctrl, "Expected a class to inherit from.");
      return NULL;
    } else if (!(base_class = HashFind(cctrl->lex->string, Fs->hash_table,
                                       HTT_CLASS, 1)))
      goto want_inher;
    bungis->base_class = base_class;
    off = bungis->sz += base_class->sz;
    Lex(cctrl->lex);
  }
  if (bungis && add)
    if (bungis->base.str &&
        !bungis->base.next) // If we dont have  a next item in the chain,I would
                            // suppose we arent in a hashtable
      HashAdd(bungis, Fs->hash_table);
  PrsMembers(cctrl, bungis, flags, &off); // This consumes '{'
  // Compute size
  for (last_m = bungis->members_lst; last_m; last_m = last_m->next) {
    if (bungis->sz < (off = last_m->off +
                            last_m->member_class->sz * last_m->dim.total_cnt)) {
      bungis->sz = off;
    }
  }
  return bungis;
}

void ICFree(CRPN *ic) {
  QueRem(ic);
  A_FREE(ic);
}

int64_t PrsReturn(CCmpCtrl *ccmp) { // YES
  if (!PrsKw(ccmp, TK_KW_RETURN))
    return 0;
  CRPN *ret = A_CALLOC(sizeof(CRPN), 0), *ic;
  ret->type = IC_RET;
  if (!ccmp->cur_fun)
    ParseErr(ccmp, "Unexpected return(not in function).");
  if (ccmp->lex->cur_tok == ';') {
    if (ccmp->cur_fun->return_class->raw_type != RT_U0)
      ParseWarn(ccmp, "Empty return in non-U0 function.");
    *(ic = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_I64,
        .integer = 0,
    };
    QueIns(ic, ccmp->code_ctrl->ir_code);
    QueIns(ret, ccmp->code_ctrl->ir_code);
    Lex(ccmp->lex);
    return 1;
  }
  ParseExpr(ccmp, 0);
  // Check if they are not the same
  if ((ccmp->cur_fun->return_class->raw_type == RT_F64) ^
      (AssignRawTypeToNode(ccmp, ccmp->code_ctrl->ir_code->next) == RT_F64)) {
    (ic = A_CALLOC(sizeof(CRPN), NULL))->type =
        (ccmp->cur_fun->return_class->raw_type == RT_F64) ? IC_TO_F64
                                                          : IC_TO_I64;
    QueIns(ic, ccmp->code_ctrl->ir_code);
  }
  QueIns(ret, ccmp->code_ctrl->ir_code);
  if (ccmp->lex->cur_tok != ';')
    ParseErr(ccmp, "Expected a ';'");
  Lex(ccmp->lex);
  return 1;
}
typedef struct CSubSwitch {
  CQue base;
  CCodeMisc *lb_start, *lb_exit;
  CCodeMisc *prev_break_to;
} CSubSwitch;
typedef struct CSwitchCase {
  struct CSwitchCase *next;
  int64_t val;
  CCodeMisc *label;
} CSwitchCase;
int64_t PrsSwitch(CCmpCtrl *cctrl) {
  if (!PrsKw(cctrl, TK_KW_SWITCH))
    return 0;
  int64_t old_flags = cctrl->flags;
  CRPN *tmpir, *label, *switch_ir;
  CSwitchCase *header = NULL, *tmps;
  CSubSwitch subs, *tmpss;
  CCodeMisc *lb_dft = CodeMiscNew(cctrl, CMT_LABEL),
            *lb_exit = CodeMiscNew(cctrl, CMT_LABEL), *old_break_to, *jmp_tab;
  int64_t k_low = INT_MAX, k_hi = INT_MIN, last, lo, hi, idx;
  old_break_to = cctrl->code_ctrl->break_to;
  cctrl->code_ctrl->break_to = lb_exit;
  //
  // When we encounter the start: block,we enter a secret function call
  // So when we `break` or we implictly break by hitting the first sub-switch
  // case,we return
  //
  // in_start_code is set when he enter a a subswitch,and is set until we hit
  // the first case while in_start_code,break's will trigger a return(so will
  // hitting the first case)
  //
  //
  int64_t in_start_code = 0;
  last = k_low;
  QueInit(&subs);
  if (cctrl->lex->cur_tok == '[') {
    Lex(cctrl->lex);
    //
    // Eat your wheaties because you are playing with the power of
    // unbounded switches
    //
    if (!ParseExpr(cctrl, 0)) {
      ParseErr(cctrl, "Expected an expression.");
      return 0;
    }
    (switch_ir = A_CALLOC(sizeof(CRPN), NULL))->type = IC_UNBOUNDED_SWITCH;
    QueIns(switch_ir, cctrl->code_ctrl->ir_code);
    if (cctrl->lex->cur_tok != ']') {
      ParseErr(cctrl, "Expected a ']'.");
      return 0;
    } else
      Lex(cctrl->lex);
  } else if (cctrl->lex->cur_tok == '(') {
    Lex(cctrl->lex);
    if (!ParseExpr(cctrl, 0)) {
      ParseErr(cctrl, "Expected an expression.");
      return 0;
    }
    (switch_ir = A_CALLOC(sizeof(CRPN), NULL))->type = IC_BOUNDED_SWITCH;
    QueIns(switch_ir, cctrl->code_ctrl->ir_code);
    // IR will be in reverse polish notation
    // start,(cond),end,BOUNDED_SWITCH
    if (cctrl->lex->cur_tok != ')') {
      ParseErr(cctrl, "Expected a ')'.");
      return 0;
    } else
      Lex(cctrl->lex);
  }
  if (cctrl->lex->cur_tok != '{') {
    ParseErr(cctrl, "Expected a '{'.");
    return 0;
  } else
    Lex(cctrl->lex);
  while (cctrl->lex->cur_tok != '}') {
    if (PrsKw(cctrl, TK_KW_CASE)) {
      //
      // Subswitchs IC_SUB_CALL the CSubSwitch.lb_start label,
      // If are IN the start block,we IC_SUB_RET to return back to
      // the case
      //
      // start:
      // (IC_SUB_PROLOG) <===== ((CSubSwitch*)subs.base.last)->code_misc;
      //    START-BLOCK
      // (IC_SUB_RET)
      // case 0xb00bie5: //My holy ears were burning,ok
      // (IC_SUB_CALL) ->code_misc=((CSubSwitch*)subs.base.last)->code_misc
      //
      if (cctrl->flags & CCF_IN_SUBSWITCH_START_BLOCK) {
        (tmpir = A_CALLOC(sizeof(CRPN), NULL))->type = IC_SUB_RET;
        QueIns(tmpir, cctrl->code_ctrl->ir_code);
        switch_ir->length++;
        cctrl->flags &= ~CCF_IN_SUBSWITCH_START_BLOCK;
      }

      // Make a label
      *(label = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_LABEL,
          .code_misc = CodeMiscNew(cctrl, CMT_LABEL),
      };
      QueIns(label, cctrl->code_ctrl->ir_code);
      // There is a subswitch if the last item in the CQue doesnt point to
      // itself
      if (&subs != (tmpss = subs.base.last)) {
        *(tmpir = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
            .type = IC_SUB_CALL,
            .code_misc = tmpss->lb_start,
        };
        QueIns(tmpir, cctrl->code_ctrl->ir_code);
      }
      tmps = A_CALLOC(sizeof(CSwitchCase), NULL);
#define INS_TMPSS                                                              \
  {                                                                            \
    tmps->next = header;                                                       \
    header = tmps;                                                             \
  }
      tmps->label = label->code_misc;
      if (cctrl->lex->cur_tok == ':') {
        if (k_low == INT_MIN)
          tmps->val = k_low = 0;
        else
          tmps->val = last++;
      } else {
        lo = last = tmps->val = PrsI64(cctrl);
        if (lo < k_low)
          k_low = lo;
        if (lo > k_hi)
          k_hi = lo;
        if (cctrl->lex->cur_tok == TK_DOT_DOT_DOT) {
          Lex(cctrl->lex);
          hi = PrsI64(cctrl);
          if (hi < k_low)
            k_low = hi;
          if (hi > k_hi)
            k_hi = hi;
          INS_TMPSS;
          for (lo = lo + 1; lo <= hi; lo++) {
            tmps = A_CALLOC(sizeof(CSwitchCase), NULL);
            last = tmps->val = lo;
            tmps->label = label->code_misc;
            INS_TMPSS;
          }
          last++;
        } else {
          hi = lo;
          INS_TMPSS;
        }
        if (cctrl->lex->cur_tok == ':') {
          Lex(cctrl->lex);
        } else {
          ParseErr(cctrl, "Expected a ':'.");
          return 0;
        }
      }
      goto add_stmt;
    } else if (PrsKw(cctrl, TK_KW_DEFUALT)) {
      if (lb_dft->flags & CMF_DEFINED) {
        ParseErr(cctrl, "Repeat default in switch!");
      }
      if (cctrl->lex->cur_tok != ':') {
        ParseErr(cctrl, "Expected a ':'.");
        return 0;
      }
      lb_dft->flags |= CMF_DEFINED;
      Lex(cctrl->lex);
      // We provide a IC_NOP as label's consume a statemnet
      (tmpir = A_CALLOC(sizeof(CRPN), NULL))->type = IC_NOP;
      QueIns(tmpir, cctrl->code_ctrl->ir_code);
      // Make a start label(IC_LABEL consumed the next ir)
      *(label = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_LABEL,
          .code_misc = lb_dft, // See me
      };
      QueIns(label, cctrl->code_ctrl->ir_code);
      goto add_stmt;
    } else if (PrsKw(cctrl, TK_KW_START)) {
      if (cctrl->lex->cur_tok != ':') {
        ParseErr(cctrl, "Expected a ':'.");
        return 0;
      } else
        Lex(cctrl->lex);
      cctrl->flags |= CCF_IN_SUBSWITCH_START_BLOCK;
      *(tmpss = A_CALLOC(sizeof(CSubSwitch), NULL)) = (CSubSwitch){
          .prev_break_to = cctrl->code_ctrl->break_to,
          .lb_exit = CodeMiscNew(cctrl, CMT_LABEL),
      };
      QueIns(tmpss, subs.base.last);
      // Make a start label(IC_LABEL consumed the next ir)
      (label = A_CALLOC(sizeof(CRPN), NULL))->type = IC_LABEL;
      tmpss->lb_start = label->code_misc = CodeMiscNew(cctrl, CMT_LABEL);
      QueIns(label, cctrl->code_ctrl->ir_code);
      // set cctrl->code_ctrl->break_to to the exit of the subswitch
      cctrl->code_ctrl->break_to = tmpss->lb_exit;
      (tmpir = A_CALLOC(sizeof(CRPN), NULL))->type = IC_SUB_PROLOG;
      QueIns(tmpir, cctrl->code_ctrl->ir_code);
      // look here
      in_start_code = 1;
    add_stmt:
      switch_ir->length++;
      continue;
    } else if (PrsKw(cctrl, TK_KW_END)) {
      if (subs.base.last == &subs) {
        ParseErr(cctrl, "Unexpected 'end' statement.");
        return 0;
      }
      QueRem(tmpss = subs.base.last);
      // We provide a IC_NOP as label's consume a statemnet
      (tmpir = A_CALLOC(sizeof(CRPN), NULL))->type = IC_NOP;
      QueIns(tmpir, cctrl->code_ctrl->ir_code);
      // Make a end label
      *(label = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
          .type = IC_LABEL,
          .code_misc = tmpss->lb_exit,
      };
      QueIns(label, cctrl->code_ctrl->ir_code);
      cctrl->code_ctrl->break_to = tmpss->prev_break_to;
      A_FREE(tmpss);
      if (cctrl->lex->cur_tok != ':') {
        ParseErr(cctrl, "Expected a ':'.");
        return 0;
      } else
        Lex(cctrl->lex);
      goto add_stmt;
    } else if (PrsStmt(cctrl)) {
      goto add_stmt;
    } else {
      ParseErr(cctrl, "Expected a statement.");
      return 0;
    }
  }
ret:
  if (!(lb_dft->flags & CMF_DEFINED)) {
    // Make a end label
    *(label = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
        .type = IC_LABEL,
        .code_misc = lb_dft = CodeMiscNew(cctrl, CMT_LABEL),
    };
    QueIns(label, cctrl->code_ctrl->ir_code);
    ++switch_ir->length;
  }
  // Define lb_exit
  // Make a end label
  *(label = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_LABEL,
      .code_misc = lb_exit,
  };
  QueIns(label, cctrl->code_ctrl->ir_code);
  ++switch_ir->length;

  Lex(cctrl->lex);
  cctrl->code_ctrl->break_to = old_break_to;
  if (header) {
  gen_tab:
    jmp_tab = CodeMiscNew(cctrl, CMT_JMP_TAB);
    jmp_tab->jmp_tab = A_CALLOC((k_hi - k_low + 1) * sizeof(CCodeMisc *), NULL);
    while (header) {
      tmps = header->next;
      jmp_tab->jmp_tab[header->val - k_low] = header->label;
      A_FREE(header);
      header = tmps;
    }
    jmp_tab->dft_lab = lb_dft;
    jmp_tab->lo = k_low;
    jmp_tab->hi = k_hi;
    // Fill in the NULL's with default
    for (idx = k_low; idx <= k_hi; idx++) {
      if (!jmp_tab->jmp_tab[idx - k_low])
        jmp_tab->jmp_tab[idx - k_low] = lb_dft;
    }
    switch_ir->code_misc = jmp_tab;
  }
  cctrl->flags = old_flags;
  return 1;
}
static void __PrsBindCSymbol(char *name, void *ptr, int64_t naked,
                             int64_t arity) {
  CHashFun *fun;
  CHashGlblVar *glbl;
  CHashExport *exp;
  if (fun = glbl = HashFind(name, Fs->hash_table, HTT_FUN | HTT_GLBL_VAR, 1)) {
    if (glbl->base.type & HTT_GLBL_VAR) {
      glbl->base.type &= ~HTF_EXTERN;
      glbl->data_addr = ptr;
    } else if (glbl->base.type & HTT_FUN) {
      if (fun->argc != arity) {
        puts(name);
        abort();
      }
      if (!fun->fun_ptr || fun->fun_ptr == &DoNothing) {
        fun->base.base.type &= ~HTF_EXTERN;
        if (naked)
          fun->fun_ptr = GenFFIBindingNaked(ptr, arity);
        else
          fun->fun_ptr = GenFFIBinding(ptr, arity);
      }
    }
    SysSymImportsResolve(name, 0);
  }
  if (!HashFind(name, Fs->hash_table, HTT_EXPORT_SYS_SYM, 1)) {
    // Here's the deal,for Load(in arm_loader.c),we can use HTT_EXPORT_SYS_SYM
    *(exp = A_CALLOC(sizeof(CHashExport), NULL)) = (CHashExport){
        .base =
            {
                .str = A_STRDUP(name, NULL),
                .type = HTT_EXPORT_SYS_SYM,
            },
        .val =
            naked ? GenFFIBindingNaked(ptr, arity) : GenFFIBinding(ptr, arity),
    };
    HashAdd(exp, Fs->hash_table);
  }
}

void PrsBindCSymbol(char *name, void *ptr, int64_t arity) {
  __PrsBindCSymbol(name, ptr, 0, arity);
}

void PrsBindCSymbolNaked(char *name, void *ptr, int64_t arity) {
  __PrsBindCSymbol(name, ptr, 1, arity);
}

int64_t PrsTry(CCmpCtrl *cctrl) {
  CRPN *rpn;
  CCodeMisc *catch_misc = CodeMiscNew(cctrl, CMT_LABEL),
            *exit_misc = CodeMiscNew(cctrl, CMT_LABEL);
  if (!PrsKw(cctrl, TK_KW_TRY))
    return 0;
  // AIWNIOS_SetJmp(SysTry())
  *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_GLOBAL,
      .global_var = HashFind("AIWNIOS_SetJmp", Fs->hash_table, HTT_FUN, 1),
  };
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_GLOBAL,
      .global_var = HashFind("SysTry", Fs->hash_table, HTT_FUN, 1),
  };
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  (rpn = A_CALLOC(sizeof(CRPN), NULL))->type = IC_CALL;
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_CALL,
      .length = 1,
  };
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_GOTO_IF,
      .code_misc = catch_misc,
  };
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  // See below note
  PrsStmt(cctrl);
  // We will call SysUntry if we succesful got through the try block
  *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_GLOBAL,
      .global_var = HashFind("SysUntry", Fs->hash_table, HTT_FUN, 1),
  };
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  (rpn = A_CALLOC(sizeof(CRPN), NULL))->type = IC_CALL;
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_GOTO,
      .code_misc = exit_misc,
  };
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  //
  if (!PrsKw(cctrl, TK_KW_CATCH))
    ParseErr(cctrl, "Expected 'catch'.");
  // Catch label is here
  *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_LABEL,
      .code_misc = catch_misc,
  };
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  // Our catch block
  PrsStmt(cctrl);
  // Call EndCatch
  *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_GLOBAL,
      .global_var = HashFind("EndCatch", Fs->hash_table, HTT_FUN, 1),
  };
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  (rpn = A_CALLOC(sizeof(CRPN), NULL))->type = IC_CALL;
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  // Exit label is here
  *(rpn = A_CALLOC(sizeof(CRPN), NULL)) = (CRPN){
      .type = IC_LABEL,
      .code_misc = exit_misc,
  };
  QueIns(rpn, cctrl->code_ctrl->ir_code);
  return 1;
}
#ifdef AIWNIOS_TESTS
void PrsTests() {
  // TODO write test to validate
  CLexer *lex = LexerNew("None", "I64i Foo(I64i a) {return a+10;}\n"
                                 "{\n"
                                 "  I64i a;\n"
                                 "  do {\n"
                                 "	 a--;\n"
                                 "    break;\n"
                                 "  } while(a);\n"
                                 "  while(0<=a<3) {\n"
                                 "    a+=1;\n"
                                 "    break;\n"
                                 "  }\n"
                                 ""
                                 "  for(a=0;a!=10;a++) {\n"
                                 "    if(a) {\n"
                                 "      break;\n"
                                 "    }\n"
                                 "    if(a) {\n"
                                 "      break;\n"
                                 "    } else {"
                                 "      goto next;\n"
                                 "    }\n"
                                 "    next:;\n"
                                 "  }\n"
                                 "  a*(a+1)+a*(a+2);\n"
                                 "  switch(a) {\n"
                                 "  case 0: Foo(0);\n"
                                 "  case 1: Foo(1);\n"
                                 "  case 2: Foo(2);\n"
                                 "  default: break;"
                                 "  }\n"
                                 "  switch(a) {\n"
                                 "  case 0: Foo(0);\n"
                                 "  case 1: Foo(1);\n"
                                 "  case 2: Foo(2);\n"
                                 "  }\n"
                                 "  switch(a) {\n"
                                 "  start:\n"
                                 "  Foo(-1);\n"
                                 "  if(a) break;\n"
                                 "  case 0: Foo(0);break;\n"
                                 "  case 1: Foo(1);break;\n"
                                 "  end:\n"
                                 "  case 2: Foo(2);break;\n"
                                 "  }\n"
                                 "}\n"
                                 "class CCls {\n"
                                 "	I64i a,*b,c[20][30];"
                                 "  union {I32i o1;U16i o2;};"
                                 "  U8i fin;"
                                 "};\n"
                                 "class CUn {\n"
                                 "	U8i u8[8];\n"
                                 "	I8i i8[8];\n"
                                 "	U16i u16[4];\n"
                                 "	I16i i16[4];\n"
                                 "	U16i u32[2];\n"
                                 "	I16i i32[2];\n"
                                 "};\n");
  CCmpCtrl *ccmp = CmpCtrlNew(lex);
  CodeCtrlPush(ccmp);
  Lex(ccmp->lex);
  PrsStmt(ccmp); // Function
  PrsStmt(ccmp); // Scope with loads of stuff
  PrsStmt(ccmp); // class
  PrsStmt(ccmp); // union
  CHashClass *cls = HashFind("CCls", Fs->hash_table, HTT_CLASS, 1);
  CMemberLst *mlst;
  assert(cls);
  assert(cls->member_cnt == 6);
  mlst = cls->members_lst;
  assert(mlst->off == 0);
  assert(mlst->member_class == HashFind("I64i", Fs->hash_table, HTT_CLASS, 1));
  mlst = mlst->next;
  assert(mlst->off == 8);
  assert(mlst->member_class - 1 ==
         HashFind("I64i", Fs->hash_table, HTT_CLASS, 1));
  mlst = mlst->next;
  assert(mlst->off == 16);
  assert(mlst->member_class == HashFind("I64i", Fs->hash_table, HTT_CLASS, 1));
  assert(mlst->dim.total_cnt == 20 * 30);
  assert(mlst->dim.next->total_cnt == 30);
  mlst = mlst->next;
  assert(mlst->off == 16 + 20 * 30 * 8);
  mlst = mlst->next;
  assert(mlst->off == 16 + 20 * 30 * 8);
  mlst = mlst->next;
  assert(mlst->off == 16 + 20 * 30 * 8 + 4);
  assert(cls->sz == 16 + 20 * 30 * 8 + 4 + 1);
}
#endif
//
// Nroot here,im about to make bindings for the IR stuff
//
#define HC_IC_BINDING(name, op)                                                \
  CRPN *__##name(CCodeCtrl *cc) {                                              \
    CRPN *rpn;                                                                 \
    (rpn = A_CALLOC(sizeof(CRPN), cc->hc))->type = op;                         \
    QueIns(rpn, cc->ir_code);                                                  \
    return rpn;                                                                \
  }
HC_IC_BINDING(HC_ICAdd_Sqr, IC_SQR);
HC_IC_BINDING(HC_ICAdd_Lock, IC_LOCK);
HC_IC_BINDING(HC_ICAdd_Pow, IC_POW);
HC_IC_BINDING(HC_ICAdd_Eq, IC_EQ);
HC_IC_BINDING(HC_ICAdd_Div, IC_DIV);
HC_IC_BINDING(HC_ICAdd_Sqrt, IC_SQRT);
HC_IC_BINDING(HC_ICAdd_Sub, IC_SUB);
HC_IC_BINDING(HC_ICAdd_Mul, IC_MUL);
HC_IC_BINDING(HC_ICAdd_Add, IC_ADD);
HC_IC_BINDING(HC_ICAdd_Comma, IC_COMMA);
HC_IC_BINDING(HC_ICAdd_Addr, IC_ADDR_OF);
HC_IC_BINDING(HC_ICAdd_Xor, IC_XOR);
HC_IC_BINDING(HC_ICAdd_Mod, IC_MOD);
HC_IC_BINDING(HC_ICAdd_Or, IC_OR);
HC_IC_BINDING(HC_ICAdd_Lt, IC_LT);
HC_IC_BINDING(HC_ICAdd_Gt, IC_GT);
HC_IC_BINDING(HC_ICAdd_Le, IC_LE);
HC_IC_BINDING(HC_ICAdd_Ge, IC_GE);
HC_IC_BINDING(HC_ICAdd_LNot, IC_LNOT);
HC_IC_BINDING(HC_ICAdd_BNot, IC_BNOT);
HC_IC_BINDING(HC_ICAdd_AndAnd, IC_AND_AND);
HC_IC_BINDING(HC_ICAdd_OrOr, IC_OR_OR);
HC_IC_BINDING(HC_ICAdd_XorXor, IC_XOR_XOR);
HC_IC_BINDING(HC_ICAdd_EqEq, IC_EQ_EQ);
HC_IC_BINDING(HC_ICAdd_Ne, IC_NE);
HC_IC_BINDING(HC_ICAdd_Lsh, IC_LSH);
HC_IC_BINDING(HC_ICAdd_Rsh, IC_RSH);
HC_IC_BINDING(HC_ICAdd_Neg, IC_NEG);
HC_IC_BINDING(HC_ICAdd_And, IC_AND);
HC_IC_BINDING(HC_ICAdd_Ret, IC_RET);

CRPN *__HC_ICAdd_SetFrameSize(CCodeCtrl *cc, int64_t arg) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = __IC_SET_FRAME_SIZE,
      .integer = arg,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_Arg(CCodeCtrl *cc, int64_t arg) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = __IC_ARG,
      .integer = arg,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_PostInc(CCodeCtrl *cc, int64_t amt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_POST_INC,
      .integer = amt,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_PostDec(CCodeCtrl *cc, int64_t amt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_POST_DEC,
      .integer = amt,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_PreInc(CCodeCtrl *cc, int64_t amt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_PRE_INC,
      .integer = amt,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_PreDec(CCodeCtrl *cc, int64_t amt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_PRE_DEC,
      .integer = amt,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
HC_IC_BINDING(HC_ICAdd_AddEq, IC_ADD_EQ);
HC_IC_BINDING(HC_ICAdd_SubEq, IC_SUB_EQ);
HC_IC_BINDING(HC_ICAdd_MulEq, IC_MUL_EQ);
HC_IC_BINDING(HC_ICAdd_DivEq, IC_DIV_EQ);
HC_IC_BINDING(HC_ICAdd_LshEq, IC_LSH_EQ);
HC_IC_BINDING(HC_ICAdd_RshEq, IC_RSH_EQ);
HC_IC_BINDING(HC_ICAdd_AndEq, IC_AND_EQ);
HC_IC_BINDING(HC_ICAdd_OrEq, IC_OR_EQ);
HC_IC_BINDING(HC_ICAdd_XorEq, IC_XOR_EQ);
HC_IC_BINDING(HC_ICAdd_ModEq, IC_MOD_EQ);
HC_IC_BINDING(HC_ICAdd_ToI64, IC_TO_I64);
HC_IC_BINDING(HC_ICAdd_ToF64, IC_TO_F64);

HC_IC_BINDING(HC_ICAdd_Min_I64, IC_MIN_I64);
HC_IC_BINDING(HC_ICAdd_Max_I64, IC_MAX_I64);
HC_IC_BINDING(HC_ICAdd_Min_U64, IC_MIN_U64);
HC_IC_BINDING(HC_ICAdd_Max_U64, IC_MAX_U64);
HC_IC_BINDING(HC_ICAdd_Min_F64, IC_MIN_F64);
HC_IC_BINDING(HC_ICAdd_Max_F64, IC_MAX_F64);

CRPN *__HC_ICAdd_I64(CCodeCtrl *cc, int64_t integer) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_I64,
      .integer = integer,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_F64(CCodeCtrl *cc, double f) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_F64,
      .flt = f,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_Switch(CCodeCtrl *cc, CCodeMisc *misc, CCodeMisc *dft) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_BOUNDED_SWITCH,
      .code_misc = misc,
  };
  misc->dft_lab = dft;
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_UnboundedSwitch(CCodeCtrl *cc, CCodeMisc *misc) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_UNBOUNDED_SWITCH,
      .code_misc = misc,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_SubRet(CCodeCtrl *cc) {
  CRPN *rpn;
  (rpn = A_CALLOC(sizeof(CRPN), cc->hc))->type = IC_SUB_RET;
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_SubProlog(CCodeCtrl *cc) {
  CRPN *rpn;
  (rpn = A_CALLOC(sizeof(CRPN), cc->hc))->type = IC_SUB_PROLOG;
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_SubCall(CCodeCtrl *cc, CCodeMisc *cm) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_SUB_CALL,
      .code_misc = cm,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
static CHashClass *rt2cls(int64_t rt, int64_t ptr_cnt) {
  CHashClass *ic_class;
  switch (rt) {
  case 3:
    ic_class = HashFind("U0", Fs->hash_table, HTT_CLASS, 1);
    break;
  case 5:
    ic_class = HashFind("U8i", Fs->hash_table, HTT_CLASS, 1);
    break;
  case 4:
    ic_class = HashFind("I8i", Fs->hash_table, HTT_CLASS, 1);
    break;
  case 7:
    ic_class = HashFind("U16i", Fs->hash_table, HTT_CLASS, 1);
    break;
  case 6:
    ic_class = HashFind("I16i", Fs->hash_table, HTT_CLASS, 1);
    break;
  case 9:
    ic_class = HashFind("U32i", Fs->hash_table, HTT_CLASS, 1);
    break;
  case 8:
    ic_class = HashFind("I32i", Fs->hash_table, HTT_CLASS, 1);
    break;
  case 11:
    ic_class = HashFind("U64i", Fs->hash_table, HTT_CLASS, 1);
    break;
  case 10:
    ic_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
    break;
  case 14:
    ic_class = HashFind("F64", Fs->hash_table, HTT_CLASS, 1);
    break;
  default:
    ic_class = HashFind("I64i", Fs->hash_table, HTT_CLASS, 1);
  }
  return ic_class + ptr_cnt;
}
CRPN *__HC_ICAdd_ShortAddr(CCmpCtrl *acc, CCodeCtrl *cc, char *name,
                           CCodeMisc *ptr) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_SHORT_ADDR,
      .code_misc = ptr,
  };
  ptr->type = CMT_SHORT_ADDR;
  rpn->ic_class = HashFind("U8i", Fs->hash_table, HTT_CLASS, 1);
  rpn->ic_class++;
  rpn->raw_type = RT_PTR;
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_Deref(CCodeCtrl *cc, int64_t rt, int64_t ptr_cnt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_DEREF,
      .ic_class = rt2cls(rt, ptr_cnt),
  };
  rpn->raw_type = rpn->ic_class->raw_type, QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_Call(CCodeCtrl *cc, int64_t arity, int64_t rt,
                      int64_t ptr_cnt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = __IC_CALL,
      .length = arity,
      .ic_class = rt2cls(rt, ptr_cnt),
  };
  rpn->raw_type = rpn->ic_class->raw_type, QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_FReg(CCodeCtrl *cc, int64_t r) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_FREG,
      .integer = r,
      .ic_class = HashFind("F64", Fs->hash_table, HTT_CLASS, 1),
      .raw_type = RT_F64,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_IReg(CCodeCtrl *cc, int64_t r, int64_t rt, int64_t ptr_cnt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_IREG,
      .integer = r,
      .ic_class = rt2cls(rt, ptr_cnt),
  };
  if (ptr_cnt)
    rt = RT_PTR;
  rpn->raw_type = rpn->ic_class->raw_type;
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_Frame(CCodeCtrl *cc, int64_t off, int64_t rt,
                       int64_t ptr_cnt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_BASE_PTR,
      .integer = off,
      .ic_class = rt2cls(rt, ptr_cnt),
  };
  if (ptr_cnt)
    rt = RT_PTR;
  rpn->raw_type = rpn->ic_class->raw_type;
  QueIns(rpn, cc->ir_code);
  return rpn;
}
CRPN *__HC_ICAdd_Typecast(CCodeCtrl *cc, int64_t rt, int64_t ptr_cnt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_TYPECAST,
      .ic_class = rt2cls(rt, ptr_cnt),
  };
  if (ptr_cnt)
    rt = RT_PTR;
  rpn->raw_type = rpn->ic_class->raw_type;
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CCmpCtrl *__HC_CmpCtrlNew() {
  return CmpCtrlNew(NULL);
}
CCodeCtrl *__HC_CodeCtrlPush(CCmpCtrl *ccmp) {
  return CodeCtrlPush(ccmp);
}
CCodeCtrl *__HC_CodeCtrlPop(CCmpCtrl *ccmp) {
  CodeCtrlPop(ccmp);
}
char *__HC_Compile(CCmpCtrl *ccmp, int64_t *sz, char **dbg_info, CHeapCtrl *h) {
  return Compile(ccmp, sz, dbg_info, h);
}
CCodeMisc *__HC_CodeMiscLabelNew(CCmpCtrl *ccmp, void **patch_addr) {
  CCodeMisc *misc;
  (misc = CodeMiscNew(ccmp, CMT_LABEL))->patch_addr = patch_addr;
  return misc;
}
CCodeMisc *__HC_CodeMiscStrNew(CCmpCtrl *ccmp, char *str, int64_t sz) {
  CCodeMisc *misc;
  (misc = CodeMiscNew(ccmp, CMT_STRING))->str = A_CALLOC(sz + 1, NULL);
  memcpy(misc->str, str, sz);
  misc->str_len = sz;
  return misc;
}
CCodeMisc *__HC_CodeMiscJmpTableNew(CCmpCtrl *ccmp, CCodeMisc *labels,
                                    void **table_address, int64_t hi) {
  CCodeMisc *misc = CodeMiscNew(ccmp, CMT_JMP_TAB);
  misc->jmp_tab = A_CALLOC((hi - 0) * sizeof(CCodeMisc *), NULL);
  memcpy(misc->jmp_tab, labels, (hi - 0) * sizeof(CCodeMisc *));
  misc->hi = hi - 1;
  misc->lo = 0;
  misc->patch_addr = table_address;
  return misc;
}

CRPN *__HC_ICAdd_Label(CCodeCtrl *cc, CCodeMisc *misc) {
  CRPN *rpn = A_CALLOC(sizeof(CRPN), cc->hc);
  rpn->type = IC_NOP;
  // Label must consume something
  QueIns(rpn, cc->ir_code);
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_LABEL,
      .code_misc = misc,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_Goto(CCodeCtrl *cc, CCodeMisc *cm) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_GOTO,
      .code_misc = cm,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_GotoIf(CCodeCtrl *cc, CCodeMisc *cm) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_GOTO_IF,
      .code_misc = cm,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_RawBytes(CCodeCtrl *cc, char *bytes, int64_t cnt) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_RAW_BYTES,
      .length = cnt,
      .raw_bytes = A_CALLOC(cnt, cc->hc),
  };
  memcpy(rpn->raw_bytes, bytes, cnt);
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_Vargs(CCodeCtrl *cc, int64_t arity) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = __IC_VARGS,
      .length = arity,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_Str(CCodeCtrl *cc, CCodeMisc *cm) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_STR,
      .code_misc = cm,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CCodeMisc *AddRelocMisc(CCmpCtrl *cctrl, char *name) {
  CCodeMisc *reloc, *head = cctrl->code_ctrl->code_misc;
  for (reloc = head->base.next; reloc != head; reloc = reloc->base.next) {
    if (reloc->type == CMT_RELOC_U64) {
      if (!strcmp(reloc->str, name))
        return reloc;
    }
  }
  reloc = CodeMiscNew(cctrl, CMT_RELOC_U64);
  reloc->str = A_STRDUP(name, cctrl->hc);
  return reloc;
}
void __HC_ICSetLine(CRPN *r, int64_t ln) {
  r->ic_line = ln;
}

CRPN *__HC_ICAdd_Reloc(CCmpCtrl *cmpc, CCodeCtrl *cc, int64_t *pat_addr,
                       char *sym, int64_t rt, int64_t ptrs) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_RELOC,
      .ic_class = rt2cls(rt, ptrs),
      .code_misc = AddRelocMisc(cmpc, sym),
  };
  rpn->code_misc->patch_addr = pat_addr;
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_RelocUnqiue(CCmpCtrl *cmpc, CCodeCtrl *cc, int64_t *pat_addr,
                             char *sym, int64_t rt, int64_t ptrs) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_RELOC,
      .ic_class = rt2cls(rt, ptrs),
      .code_misc = CodeMiscNew(cmpc, CMT_RELOC_U64),
  };
  rpn->code_misc->patch_addr = pat_addr;
  rpn->code_misc->str = A_STRDUP(sym, cc->hc);
  QueIns(rpn, cc->ir_code);
  return rpn;
}

// Sets how many bytes before function start a symbol starts at
// Symbol    <=====RIP-off
// some...code
// Fun Start <==== RIP
void __HC_SetAOTRelocBeforeRIP(CRPN *r, int64_t off) {
  r->code_misc->aot_before_hint = off;
}

int64_t __HC_CodeMiscIsUsed(CCodeMisc *cm) {
  return cm->use_cnt != 0;
}

CRPN *__HC_ICAdd_StaticData(CCmpCtrl *cmp, CCodeCtrl *cc, int64_t at, char *d,
                            int64_t len) {
  CCodeMisc *misc = CodeMiscNew(cmp, CMT_STATIC_DATA);
  misc->integer = at;
  misc->str_len = len;
  memcpy(misc->str = A_CALLOC(len, cmp->hc), d, len);
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = __IC_SET_STATIC_DATA,
      .code_misc = misc,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_SetStaticsSize(CCodeCtrl *cc, int64_t len) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = __IC_STATICS_SIZE,
      .integer = len,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}

CRPN *__HC_ICAdd_StaticRef(CCodeCtrl *cc, int64_t off, int64_t rt,
                           int64_t ptrs) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = __IC_STATIC_REF,
      .integer = off,
      .ic_class = rt2cls(rt, ptrs),
  };
  rpn->raw_type = rpn->ic_class->raw_type;
  QueIns(rpn, cc->ir_code);
  return rpn;
}

void __HC_CmpCtrl_SetAOT(CCmpCtrl *cc) {
  cc->flags |= CCF_AOT_COMPILE;
}
HC_IC_BINDING(HC_ICAdd_ToBool, IC_TO_BOOL);
HC_IC_BINDING(HC_ICAdd_BT, IC_BT);
HC_IC_BINDING(HC_ICAdd_BTC, IC_BTC);
HC_IC_BINDING(HC_ICAdd_BTS, IC_BTS);
HC_IC_BINDING(HC_ICAdd_BTR, IC_BTR);
HC_IC_BINDING(HC_ICAdd_LBTC, IC_LBTC);
HC_IC_BINDING(HC_ICAdd_LBTS, IC_LBTS);
HC_IC_BINDING(HC_ICAdd_LBTR, IC_LBTR);

HC_IC_BINDING(HC_ICAdd_Fs, IC_FS);
HC_IC_BINDING(HC_ICAdd_Gs, IC_GS);

CCodeMiscRef *CodeMiscAddRef(CCodeMisc *misc, int32_t *addr) {
  CCodeMiscRef *ref;
  *(ref = A_CALLOC(sizeof(CCodeMiscRef), NULL)) = (CCodeMiscRef){
      .add_to = addr,
      .next = misc->refs,
  };
  misc->refs = ref;
  return ref;
}
void __HC_ICSetLock(CRPN *l) {
  l->flags |= ICF_LOCK_EXPR;
}

CRPN *__HC_ICAdd_GetVargsPtr(CCodeCtrl *cc) {
  CRPN *rpn;
  *(rpn = A_CALLOC(sizeof(CRPN), cc->hc)) = (CRPN){
      .type = IC_GET_VARGS_PTR,
      .ic_class = NULL,
      .raw_type = RT_I64i,
  };
  QueIns(rpn, cc->ir_code);
  return rpn;
}

void __HC_CodeMiscInterateThroughRefs(CCodeMisc *cm,
                                      void (*fptr)(void *addr, void *user_data),
                                      void *user_data) {
  CCodeMiscRef *refs = cm->refs;
  int old = SetWriteNP(1);
  while (refs) {
    FFI_CALL_TOS_2(fptr, MemGetExecPtr(refs->add_to), user_data);
    refs = refs->next;
  }
  SetWriteNP(old);
}

void CacheRPNArgs(CCmpCtrl *cctrl) {
  CRPN *head = cctrl->code_ctrl->ir_code, *rpn;
  for (rpn = head->base.next; rpn != head; rpn = rpn->base.next) {
    rpn->tree1 = ICArgN(rpn, 1);
    rpn->tree2 = ICArgN(rpn, 2);
  }
}
