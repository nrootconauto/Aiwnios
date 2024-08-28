#pragma once
#include "aiwn_que.h"
#include <setjmp.h>
#include <stdint.h>

typedef struct CArrayDim {
  struct CArrayDim *next;
  int64_t cnt, total_cnt;
} CArrayDim;
typedef struct CMemberLst {
  struct CMemberLst *next, *last;
  struct CHashClass *member_class;
  CArrayDim dim;
  struct CHashFun *fun_ptr;
  char *str;
#define MLF_STATIC        1
#define MLF_DFT_AVAILABLE 2
  int64_t off, reg, use_cnt, flags;
  union {
    int64_t dft_val;
    double dft_val_flt;
    // USed for MLF_STATIC,this is a heap allocated "static" variable
    char *static_bytes;
  };
} CMemberLst;

#define HTT_INVALID         0
#define HTT_DEFINE_STR      0x1
#define HTT_KEYWORD         0x2
#define HTT_CLASS           0x4
#define HTT_GLBL_VAR        0x8
#define HTT_FUN             0x10
#define HTF_IMPORT          0x20
#define HTF_EXTERN          0x40
#define HTT_IMPORT_SYS_SYM  0x80
#define HTT_EXPORT_SYS_SYM  0x100
#define HTT_IMPORT_SYS_SYM2 0x200 // See arm_loader.c

typedef struct CHash {
  struct CHash *next;
  char *str;
  int32_t type, use_cnt;
} CHash;
typedef struct CHashTable {
  struct CHashTable *next;
  int64_t mask, locked_flags;
  CHash **body;
} CHashTable;
typedef struct CHashDefineStr {
  struct CHash base;
  char *src_link;
  char *name;
  char *data;
} CHashDefineStr;
typedef struct CHashKeyword {
  struct CHash base;
  int64_t tk;
} CHashKeyword;
typedef struct CHashImport {
  CHash base;
  char **address;
} CHashImport;
#define STAR_CNT 5
typedef struct CHashClass {
  CHash base;
  char *src_link;
  char *import_name;
#define REG_ALLOC -1
#define REG_NONE  -2
#define REG_MAYBE -3
  // TODO make a mechanism for assigning hardware regs

#define RT_U0    -1
#define RT_UNDEF 0
#define RT_I8i   1
#define RT_U8i   2
#define RT_I16i  3
#define RT_U16i  4
#define RT_I32i  5
#define RT_U32i  6
#define RT_I64i  7
#define RT_U64i  8
#define RT_PTR   9
#define RT_F64   10
#define RT_FUNC  11

#define CLSF_VARGS  1
#define CLSF_FUNPTR 2

  int64_t member_cnt, ptr_star_cnt, raw_type, flags, use_cnt, sz;
  struct CHashClass *base_class;
  CMemberLst *members_lst;
  struct CHashClass *fwd_class;
} CHashClass;

typedef struct CHashExport {
  CHash base;
  char *val;
} CHashExport;

typedef struct CHashFun {
  CHashClass base;
  char *import_name;
  CHashClass *return_class;
  int64_t argc;
  void *fun_ptr;
} CHashFun;
typedef struct CHashGlblVar {
  CHash base;
  char *src_link, *import_name;
  CHashClass *var_class;
  void *data_addr;
  CArrayDim dim;
  CHashFun *fun_ptr;
} CHashGlblVar;

void HashTableDel(CHashTable *table);
int64_t HashRemDel(CHash *h, CHashTable *table, int64_t inst);
void HashAdd(CHash *h, CHashTable *table);
CHash **HashBucketFind(char *str, CHashTable *table);
CHash *HashSingleTableFind(char *str, CHashTable *table, int64_t type,
                           int64_t inst);
CHash *HashFind(char *str, CHashTable *table, int64_t type, int64_t inst);
CHashTable *HashTableNew(int64_t sz, void *task);
void HashDel(CHash *h);
int64_t HashStr(char *str);
