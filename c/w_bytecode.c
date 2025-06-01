#include "aiwn_bytecode.h"
#include "aiwn_hash.h"
#include "aiwn_lexparser.h"
#include <math.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {
  ABC_NOP,
  ABC_INTERN,
  ABC_GOTO,
  ABC_GOTO_IF,
  ABC_TO_I64,
  ABC_TO_F64,
  ABC_LABEL,
  ABC_STATIC,
  ABC_READ_PTRO, // offset
  ABC_READ_PTR,
  ABC_NEG,
  ABC_POS,
  ABC_STR,
  ABC_PCREL,
  ABC_IMM_I64,
  ABC_IMM_F64,
  ABC_POW,
  ABC_ADD,
  ABC_WRITE_PTR,
  ABC_WRITE_PTRO, // offset
  ABC_DIV,
  ABC_MUL,
  ABC_AND,
  ABC_SUB,
  ABC_FRAME_ADDR,
  ABC_XOR,
  ABC_MOD,
  ABC_OR,
  ABC_LT,
  ABC_GT,
  ABC_LNOT,
  ABC_BNOT,
  ABC_AND_AND,
  ABC_XOR_XOR,
  ABC_OR_OR,
  ABC_NE,
  ABC_EQ_EQ,
  ABC_LE,
  ABC_GE,
  ABC_LSH,
  ABC_RSH,
  ABC_RET,
  ABC_CALL,
  ABC_COMMA,
  ABC_SWITCH,
  ABC_SUB_PROLOG,
  ABC_SUB_RET,
  ABC_SUB_CALL,
  ABC_TYPECAST,
  ABC_RELOC,
  ABC_SET_FRAME_SIZE,
  ABC_DUP,
  ABC_END_EXPR,
  ABC_DISCARD_STK, // next u32 is amount to discard
  ABC_PUSH,
  ABC_POP,
  ABC_TO_BOOL,
  ABC_SKIP_ON_PASS, // next u32 is amt of u32's to skip
  ABC_SKIP_ON_FAIL, // next u32 is amt of u32's to skip
  ABC_GS,
  ABC_FS,
  ABC_PRE_INC,  // next u64 is amount to add
  ABC_POST_INC, // next u64 is amount to add
  ABC_DISCARD,  // next u32 is amount to discard
  ABC_MISC_START,
  ABC_CNT,      // MUST BE THE LAST ITEM
};

#define AiwnBCWrite(_dst, _src, rt)                                            \
  ({                                                                           \
    char *dst = _dst;                                                          \
    char *src = _src;                                                          \
    switch (rt) {                                                              \
    case RT_I8i:                                                               \
    case RT_U8i:                                                               \
      *dst = *src;                                                             \
      break;                                                                   \
    case RT_I16i:                                                              \
    case RT_U16i:                                                              \
      *(uint16_t *)dst = *(uint16_t *)src;                                     \
      break;                                                                   \
    case RT_I32i:                                                              \
    case RT_U32i:                                                              \
      *(uint32_t *)dst = *(uint32_t *)src;                                     \
      break;                                                                   \
    case RT_I64i:                                                              \
    case RT_U64i:                                                              \
    case RT_F64:                                                               \
    case RT_PTR:                                                               \
    case RT_FUNC:                                                              \
      *(uint64_t *)dst = *(uint64_t *)src;                                     \
      break;                                                                   \
    default:                                                                   \
    fprintf(stdout,"%d\n",rt); \
      abort();                                                                 \
      break;                                                                   \
    }                                                                          \
    0;                                                                         \
  })

#define AiwnBCReadI64(_src, rt)                                                \
  ({                                                                           \
    char *src = (char *)(_src);                                                \
    int64_t r;                                                                 \
    switch (rt) {                                                              \
      break;                                                                   \
    case RT_I8i:                                                               \
      r = (int64_t)*(int8_t *)src;                                             \
      break;                                                                   \
    case RT_U8i:                                                               \
      r = (int64_t)*(uint8_t *)src;                                            \
      break;                                                                   \
    case RT_I16i:                                                              \
      r = (int64_t)*(int16_t *)src;                                            \
      break;                                                                   \
    case RT_U16i:                                                              \
      r = (int64_t)*(uint16_t *)src;                                           \
      break;                                                                   \
    case RT_I32i:                                                              \
      r = (int64_t)*(int32_t *)src;                                            \
      break;                                                                   \
    case RT_U32i:                                                              \
      r = (int64_t)*(uint32_t *)src;                                           \
      break;                                                                   \
    case RT_I64i:                                                              \
    case RT_U64i:                                                              \
      r = (int64_t)*(int64_t *)src;                                            \
      break;                                                                   \
    case RT_F64:                                                               \
      r = (int64_t)*(double *)src;                                             \
      break;                                                                   \
    case RT_PTR:                                                               \
    case RT_FUNC:                                                              \
      r = (int64_t)*(int64_t *)src;                                            \
      break;                                                                   \
    }                                                                          \
    r;                                                                         \
  })
#define AiwnBCReadF64(src, rt)                                                 \
  ({                                                                           \
    double r;                                                                  \
    switch (rt) {                                                              \
    case RT_I8i:                                                               \
      r = (double)*(int8_t *)src;                                              \
      break;                                                                   \
    case RT_U8i:                                                               \
      r = (double)*(uint8_t *)src;                                             \
      break;                                                                   \
    case RT_I16i:                                                              \
      r = (double)*(int16_t *)src;                                             \
      break;                                                                   \
    case RT_U16i:                                                              \
      r = (double)*(uint16_t *)src;                                            \
      break;                                                                   \
    case RT_I32i:                                                              \
      r = (double)*(int32_t *)src;                                             \
      break;                                                                   \
    case RT_U32i:                                                              \
      r = (double)*(uint32_t *)src;                                            \
      break;                                                                   \
    case RT_I64i:                                                              \
      r = (double)*(int64_t *)src;                                             \
      break;                                                                   \
    case RT_U64i:                                                              \
      r = (double)*(uint64_t *)src;                                            \
      break;                                                                   \
    case RT_F64:                                                               \
      r = *(double *)src;                                                      \
      break;                                                                   \
    case RT_PTR:                                                               \
    case RT_FUNC:                                                              \
      r = (double)*(uint64_t *)src;                                            \
    }                                                                          \
    r;                                                                         \
  })
#define puts(...)
#define printf(...)

#ifndef __EMSCRIPTEN__
static _Thread_local ABCState *cur_bcstate;
static _Thread_local ABCState cur_dbgstate;
static _Thread_local jmp_buf except_to;
#else
static ABCState *cur_bcstate;
static ABCState cur_dbgstate;
static jmp_buf except_to;
#endif
typedef char *ABC_PTR;
static int64_t AiwnBCAddCode(ABC_PTR ptr, uint32_t v, int64_t len) {
  if (ptr) {
    *(uint32_t *)(ptr + len) = v;
  }
  return len += 4;
}

static int64_t ForceType(char *ptr, CRPN *a, int64_t to, int64_t len) {
  if ((a->raw_type == RT_F64) != (to == RT_F64)) {
    if (to == RT_F64 && a->type != IC_F64)
      len = AiwnBCAddCode(ptr, ABC_TO_F64, len);
    else if (to != RT_F64 && a->type != IC_I64)
      len = AiwnBCAddCode(ptr, ABC_TO_I64, len);
  }
  return len;
}

int64_t AiwnRunBC(ABCState *state) {
  cur_bcstate = state;
  uint32_t *bc = (void *)state->ip;
  if (!bc)
    return 0;
  int64_t i64, ai, bi, old_fp, old_addr, ret;
  uint64_t bu, au;
  int64_t retval = 0;
  double f64, af, bf;
  ABCFrame *abcf = state->fun_frame, *new;
  int64_t (*fun_ptr)(int64_t *stk);
  int64_t dft;
  int64_t *table;
  printf("IP:%p(%s),FP:%p,SP:%p\n", bc, WhichFun(bc), state->fp, state->_sp);
  //   printf("BTTM:%p,	%p,PR:(%p)\n", abcf->istk[abcf->sp - 1],
  //        abcf->istk[abcf->sp], abcf->sp);
  printf("%d,%d,%d\n", (*bc>>8)&0xff,*bc & 0xff,*bc);
  switch (*bc++ & 0xffff) {
  default:
    printf("FUCK:%d\n", bc[-1] & 0xff);
    break;
    //Pray that your optimizer is good
#define CASE_BALLER(cs, code)                                                  \
  break;                                                                       \
  case cs:                                                                     \
  case (cs | (-1 << 8)) & 0xffff: {                                 \
    const int64_t t = RT_I64i;                                                 \
    const int64_t is_f64=0,is_u64=0; \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_I8i << 8): {                                                   \
    const int64_t t = RT_I8i;               	                                   \
    const int64_t is_f64=0,is_u64=0; \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_F64 << 8): {                                                   \
    const int64_t t = RT_F64;                                                  \
    const int64_t is_f64=1,is_u64=0; \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_U8i << 8): {                                                   \
    const int64_t t = RT_U8i;                                                  \
    const int64_t is_f64=0,is_u64=0; \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_I16i << 8): {                                                  \
    const int64_t t = RT_I16i;                                                 \
    const int64_t is_f64=0,is_u64=0; \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_U16i << 8): {                                                  \
    const int64_t t = RT_U16i;                                                 \
    const int64_t is_f64=0,is_u64=0; \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_I32i << 8): {                                                  \
    const int64_t is_f64=0,is_u64=0; \
    const int64_t t = RT_I32i;                                                 \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_U32i << 8): {                                                  \
    const int64_t is_f64=0,is_u64=0; \
    const int64_t t = RT_U32i;                                                 \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_I64i << 8): {                                                  \
    const int64_t is_f64=0,is_u64=0; \
    const int64_t t = RT_I64i;                                                 \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_U64i << 8): {                                                  \
    const int64_t is_f64=0,is_u64=1; \
    const int64_t t = RT_U64i;                                                 \
    code;                                                                      \
  } break;                                                                     \
  case cs | (RT_PTR << 8):                                                     \
  case cs | (RT_FUNC << 8): {                                                  \
    const int64_t t = RT_I64i;                                                 \
    const int64_t is_f64=0,is_u64=0; \
    code;                                                                      \
  } break;


    CASE_BALLER(ABC_PRE_INC, {
      bi = abcf->istk[abcf->sp];
      if (is_f64) {
        af = *(double *)bc;
        bc += 2;
        af += AiwnBCReadF64(bi, RT_F64);
        AiwnBCWrite((void *)bi, &af, t);
        abcf->fstk[abcf->sp] = af;
      } else {
        ai = *(int64_t *)bc;
        bc += 2;
        ai += AiwnBCReadI64(bi, t);
        AiwnBCWrite((void *)bi, &ai, t);
        abcf->istk[abcf->sp] = ai;
      }
    })
    CASE_BALLER(ABC_POST_INC, {
      puts("POST_ICN");
      bi = abcf->istk[abcf->sp];

      if (is_f64) {
        af = *(double *)bc;
        abcf->fstk[abcf->sp] = AiwnBCReadF64(bi, t);
        bc += 2;
        af += AiwnBCReadF64(bi, t);
        AiwnBCWrite((void *)bi, &af, t);
        abcf->fstk[abcf->sp] = af;
      } else {
        ai = *(int64_t *)bc;
        abcf->istk[abcf->sp] = AiwnBCReadI64(bi, t);
        bc += 2;
        ai += AiwnBCReadI64(bi, t);
        AiwnBCWrite((void *)bi, &ai, t);
      }
    })
    break;

    CASE_BALLER(ABC_NOP, {
      puts("NOP");
      break;
    })
    CASE_BALLER(ABC_INTERN, {
      puts("INTERN");
      fun_ptr = *(void **)bc;
      bc += 2;
      state->ip = (int64_t)bc;
      ret = (*fun_ptr)((int64_t *)((char *)state->_sp + 16 + sizeof(ABCFrame)));
      abcf = state->fun_frame;
      abcf->istk[++abcf->sp] = ret;
      printf("aSS:%p\n", ret);
      goto fin;
    })
    CASE_BALLER(ABC_DUP, {
      puts("DUO");
      ai = abcf->istk[abcf->sp];
      abcf->istk[++abcf->sp] = ai;
    });
    CASE_BALLER(ABC_PUSH, {
      puts("push");
      state->_sp -= 8;
      if (is_f64) {
        af = abcf->fstk[abcf->sp--];
        AiwnBCWrite((void *)state->_sp, (void *)&af, RT_F64);
      } else {
        ai = abcf->istk[abcf->sp--];
        printf("%p\n", ai);
        AiwnBCWrite((void *)state->_sp, (void *)&ai, RT_I64i);
      }
    })
    CASE_BALLER(ABC_POP, {
      puts("POP");
      if (is_f64) {
        af = AiwnBCReadF64((void *)state->_sp, RT_F64);
        abcf->fstk[++abcf->sp] = af;
      } else {
        ai = AiwnBCReadI64((void *)state->_sp, RT_I64i);
        abcf->istk[++abcf->sp] = ai;
      }
      state->_sp += 8;
    });
    break;
    CASE_BALLER(ABC_SET_FRAME_SIZE, {
      puts("SET_FRAME_SIZE");
      state->_sp -= *bc++;
    })
    CASE_BALLER(ABC_SUB_RET, {
      AiwnBCWrite(&state->ip, (void *)state->_sp, RT_I64i);
      state->_sp += 8;
      goto fin;
    })
    CASE_BALLER(ABC_SUB_CALL, {
      old_addr = (int64_t)(bc - 1);

      bc += 2;
      state->_sp -= 8;
      AiwnBCWrite((void *)state->_sp, &bc, RT_I64i);
      state->ip = ((uint64_t *)bc)[-1] + old_addr;
      goto fin;
    })
    CASE_BALLER(ABC_CALL, {
      puts("CALL");
      if (abcf->istk[abcf->sp] != INVALID_PTR) {
        old_fp = state->fp;
        state->fp = state->_sp -= 16 + sizeof(ABCFrame);
        new = state->fp + 16;
        new->next = abcf;
        new->sp = 0;
        printf("TO::%p,ME:%p\n", abcf->istk[abcf->sp], bc);
        AiwnBCWrite((void *)(state->fp + 0), (void *)&old_fp, RT_I64i);
        AiwnBCWrite((void *)(state->fp + 8), (void *)&bc, RT_I64i);
        state->ip = abcf->istk[abcf->sp--];
        printf("FROM::%p\n", state->fun_frame);
        state->fun_frame = new;
        printf("INTO:%p\n", new);
        goto fin;

      } else {
        abcf->istk[abcf->sp] = 0;
        break;
      }
      break;
    })
    CASE_BALLER(ABC_RET, {
      puts("RET");
      ret = abcf->istk[abcf->sp];
      printf("FP:%p\n", state->fp);
      printf("RETAT:%p\n", state->fun_frame);
      old_fp = AiwnBCReadI64((void *)(state->fp), RT_I64i);
      old_addr = AiwnBCReadI64((void *)(state->fp + 8), RT_I64i);
      printf("RETTO:%p\n", old_addr);
      state->_sp = state->fp + 16 + sizeof(ABCFrame);
      printf("ASS:%p\n", state->fun_frame);
      state->fp = old_fp;
      state->ip = old_addr;
      printf("N1I:%p\n", state->fun_frame);
      if (state->fp) {
        abcf = state->fun_frame = (ABCFrame *)(state->fp + 16);
        abcf->istk[++abcf->sp] = ret;
      printf("NI2:%p\n", state->fun_frame);
      } else
        state->fun_frame = NULL;
      printf("NI:%p\n", state->fun_frame);
      retval = ret;
      goto fin;
    })
    CASE_BALLER(ABC_GOTO, {
      old_addr = (int64_t)(bc - 1);
      puts("GOTO");
      i64 = *(uint64_t *)bc;
      bc += 2;
      state->ip = i64 + old_addr;
      goto fin;
    })
    CASE_BALLER(ABC_END_EXPR, {
      puts("END_EXPR");

      abcf->sp = 0;
    })
    CASE_BALLER(ABC_GOTO_IF, {
      puts("ABC_GOTO_IF");
      old_addr = (int64_t)(bc - 1);
      i64 = *(uint64_t *)bc;
      bc += 2;
      if (is_f64) {
        if (abcf->fstk[abcf->sp--] != 0.) {
          state->ip = i64 + old_addr;
          goto fin;
        }
      } else {
        if (abcf->istk[abcf->sp--]) {
          state->ip = i64 + old_addr;
          goto fin;
        }
      }
    })
    CASE_BALLER(ABC_TO_I64, {
      puts("ABC_2I64");
      f64 = abcf->fstk[abcf->sp];
      abcf->istk[abcf->sp] = f64;
      break;
    })
  case ABC_TYPECAST:
    // Things are unioned
    break;
    CASE_BALLER(ABC_TO_F64, {
      puts("ABC_2F64");
      i64 = abcf->istk[abcf->sp];
      abcf->fstk[abcf->sp] = i64;
      break;
    })
    CASE_BALLER(ABC_TO_BOOL, {
      puts("ABC_2Bool");
      if (is_f64) {
        f64 = abcf->fstk[abcf->sp];
        abcf->fstk[abcf->sp] = !!f64;
      } else {
        i64 = abcf->istk[abcf->sp];
        abcf->istk[abcf->sp] = !!i64;
      }
    })
    break;
    CASE_BALLER(ABC_FS, {
      puts("ABC_FS");
      abcf->istk[++abcf->sp] = (int64_t)GetHolyFs();
    })
    CASE_BALLER(ABC_GS, {
      puts("ABC_GS");
      abcf->istk[++abcf->sp] = (int64_t)GetHolyGs();
    })
    CASE_BALLER(ABC_READ_PTR, {
      puts("READ_PTR");

      i64 = 0;
      if (is_f64)
        abcf->fstk[abcf->sp] =
            AiwnBCReadF64((void *)(abcf->istk[abcf->sp] + i64), t);
      else
        abcf->istk[abcf->sp] =
            AiwnBCReadI64((void *)(abcf->istk[abcf->sp] + i64), t);
      break;
    })
    CASE_BALLER(ABC_READ_PTRO, {
      i64 = *(int32_t *)bc;
      bc += 1;

      if (is_f64)
        abcf->fstk[abcf->sp] =
            AiwnBCReadF64((void *)(abcf->istk[abcf->sp] + i64), t);
      else
        abcf->istk[abcf->sp] =
            AiwnBCReadI64((void *)(abcf->istk[abcf->sp] + i64), t);
      break;
    })
    CASE_BALLER(ABC_NEG, {
      puts("NEG");
      if (is_f64) {
        abcf->fstk[abcf->sp] = -abcf->fstk[abcf->sp];
      } else {
        abcf->istk[abcf->sp] = -abcf->istk[abcf->sp];
      }
    })
    CASE_BALLER(ABC_POS, {
      puts("POS");
      if (is_f64) {
        abcf->fstk[abcf->sp] = abcf->fstk[abcf->sp];
      } else {
        abcf->istk[abcf->sp] = abcf->istk[abcf->sp];
      }
    })
    CASE_BALLER(ABC_IMM_F64, {
      puts("FG64");
      f64 = *(double *)bc;
      bc += 2;
      abcf->fstk[++abcf->sp] = f64;
    })
    CASE_BALLER(ABC_FRAME_ADDR, {
      puts("FRAME_ADDR");
      abcf->istk[++abcf->sp] = state->fp;
    });
    CASE_BALLER(ABC_PCREL, {
      i64 = *(int64_t *)bc;
      bc += 2;
      abcf->istk[++abcf->sp] = i64 + (int64_t)(bc - 3);
    });
    CASE_BALLER(ABC_IMM_I64, {
      puts("I64");
      i64 = *(int64_t *)bc;
      bc += 2;
      abcf->istk[++abcf->sp] = i64;
    });
    CASE_BALLER(ABC_SWITCH, {
      old_addr = (int64_t)(bc - 1);
      i64 = *bc;
      ++bc;
      table = *(int64_t *)bc + old_addr;
      bc += 2;
      dft = *(int64_t *)bc + old_addr;
      bc += 2;
      ai = abcf->istk[abcf->sp--];
      if (ai < 0) {
        state->ip = dft;
      } else if (ai >= i64) {
        state->ip = dft;
      } else {
        state->ip = table[ai];
      }
      goto fin;
    })
    CASE_BALLER(ABC_ADD, {
#define ABC_OP(op)                                                             \
  if (is_f64) {                                                                \
    bf = abcf->fstk[abcf->sp--];                                               \
    af = abcf->fstk[abcf->sp];                                                 \
    af op bf;                                                                  \
    abcf->fstk[abcf->sp] = af;                                                 \
  } else if (is_u64) {                                                         \
    bu = abcf->ustk[abcf->sp--];                                               \
    au = abcf->ustk[abcf->sp];                                                 \
    abcf->ustk[abcf->sp] = au op bu;                                           \
  } else {                                                                     \
    bi = abcf->ustk[abcf->sp--];                                               \
    ai = abcf->istk[abcf->sp];                                                 \
    abcf->istk[abcf->sp] = ai op bi;                                           \
  }
      ABC_OP(+=);
    });
    CASE_BALLER(ABC_WRITE_PTR, {
      i64 = 0;
      i64 += abcf->istk[abcf->sp-1];
      au=abcf->ustk[abcf->sp];
      AiwnBCWrite((void *)i64, (void *)&au, t);
      abcf->istk[--abcf->sp]=au;
    })
    CASE_BALLER(ABC_WRITE_PTRO, {
      i64 = *(int32_t *)bc;
      bc++;
      i64+=abcf->ustk[abcf->sp-1];
      au= abcf->istk[abcf->sp];
      AiwnBCWrite((void *)i64, (void *)&au, t);
      abcf->istk[--abcf->sp]=au;
    })
    CASE_BALLER(ABC_DIV, { ABC_OP(/=); });
    CASE_BALLER(ABC_MUL, { ABC_OP(*=); });
    CASE_BALLER(ABC_AND, {
      puts("AND");
      bi = abcf->istk[abcf->sp--];
      abcf->istk[abcf->sp] &= bi;
    });
    CASE_BALLER(ABC_SKIP_ON_FAIL, {
      puts("SKOF");
      if (is_f64) {
        bf = abcf->fstk[abcf->sp];
        i64 = *bc++;
        if (!bf) {
          state->ip = (uint64_t)(bc - 2) + i64;
          goto fin;
        }
        abcf->istk[abcf->sp] = 1;
      } else {
        bi = abcf->istk[abcf->sp];
        i64 = *bc++;
        if (!bi) {
          state->ip = (uint64_t)(bc - 2) + i64;
          goto fin;
        }
        abcf->istk[abcf->sp] = 1;
      }
    });
    CASE_BALLER(ABC_DISCARD, {
      puts("DISCARD");
      i64 = *bc++;
      abcf->sp -= i64;
    });
    CASE_BALLER(ABC_DISCARD_STK, {
      puts("DISARD_STK");

      i64 = *bc++;
      state->_sp += i64;
    });
    CASE_BALLER(ABC_SKIP_ON_PASS, {
      puts("SKOP");
      if (is_f64) {
        bf = abcf->fstk[abcf->sp];
        i64 = *bc++;
        if (bf) {
          state->ip = (uint64_t)(bc - 2) + i64;
          goto fin;
        }
        abcf->istk[abcf->sp] = 0;
      } else {
        bi = abcf->istk[abcf->sp];
        i64 = *bc++;
        if (bi) {
          state->ip = (uint64_t)(bc - 2) + i64;
          goto fin;
        }
        abcf->istk[abcf->sp] = 0;
      }
    });
    CASE_BALLER(ABC_SUB, { ABC_OP(-=); });
    CASE_BALLER(ABC_XOR, {
      puts("XOIR");
      bi = abcf->istk[abcf->sp--];
      abcf->istk[abcf->sp] ^= bi;
    });
    CASE_BALLER(ABC_MOD, {
      puts("MOD");
      if (is_f64) {
        bf = abcf->fstk[abcf->sp--];
        af = abcf->fstk[abcf->sp];
        abcf->fstk[abcf->sp] = fmod(af, bf);
      } else if (is_u64) {
        bu = abcf->ustk[abcf->sp--];
        au = abcf->ustk[abcf->sp];
        abcf->ustk[abcf->sp] = au %= bu;
      } else {
        bi = abcf->ustk[abcf->sp--];
        ai = abcf->ustk[abcf->sp];
        abcf->istk[abcf->sp] = ai %= bi;
      }
    });
    CASE_BALLER(ABC_OR, {
      puts("OR");

      bi = abcf->istk[abcf->sp--];
      abcf->istk[abcf->sp] |= bi;
    });
    CASE_BALLER(ABC_LT, {
#define ABC_OP2(op)                                                            \
  if (is_f64) {                                                                \
    bf = abcf->fstk[abcf->sp--];                                               \
    abcf->istk[abcf->sp] = abcf->fstk[abcf->sp] op bf;                         \
  } else if (is_u64) {                                                         \
    bu = abcf->ustk[abcf->sp--];                                               \
    abcf->ustk[abcf->sp] = abcf->ustk[abcf->sp] op bu;                         \
  } else {                                                                     \
    bi = abcf->istk[abcf->sp--];                                               \
    abcf->istk[abcf->sp] = abcf->istk[abcf->sp] op bi;                         \
  }
      ABC_OP2(<);
    });
    CASE_BALLER(ABC_GT, { ABC_OP2(>); });
    CASE_BALLER(ABC_LE, { ABC_OP2(<=); });
    CASE_BALLER(ABC_GE, { ABC_OP2(>=); });
    CASE_BALLER(ABC_LNOT, {
#define ABC_UNOP(op)                                                           \
  if (is_f64) {                                                                \
    bf = abcf->fstk[abcf->sp];                                                 \
    abcf->fstk[abcf->sp] = op bf;                                              \
  } else {                                                                     \
    bi = abcf->istk[abcf->sp];                                                 \
    abcf->istk[abcf->sp] = op bi;                                              \
  }
      ABC_UNOP(!);
    });
    CASE_BALLER(ABC_BNOT, {
      puts("BNOT");
      bi = abcf->istk[abcf->sp];
      abcf->istk[abcf->sp] = ~bi;
    });
    CASE_BALLER(ABC_XOR_XOR, {
      puts("XXOR");
      if (is_f64) {
        bf = abcf->fstk[abcf->sp--];
        af = abcf->fstk[abcf->sp];
        abcf->fstk[abcf->sp] = !!af ^ !!bf;
      } else {
        bi = abcf->istk[abcf->sp--];
        ai = abcf->istk[abcf->sp];
        abcf->istk[abcf->sp] = !!ai ^ !!bi;
      }
    });
    CASE_BALLER(ABC_NE, { ABC_OP2(!=); });
    CASE_BALLER(ABC_EQ_EQ, { ABC_OP2(==); });
    CASE_BALLER(ABC_LSH, {
      puts("LSH");
      bi = bu = abcf->istk[abcf->sp--];
      ai = au = abcf->istk[abcf->sp];
      if (is_u64) {
        abcf->ustk[abcf->sp] = ai <<= bi;
      } else
        abcf->istk[abcf->sp] = au <<= bu;
    });
    CASE_BALLER(ABC_RSH, {
      puts("RSH");
      bi = bu = abcf->istk[abcf->sp--];
      ai = au = abcf->istk[abcf->sp];
      if (is_u64) {
        abcf->ustk[abcf->sp] = au >>= bu;
      } else
        abcf->istk[abcf->sp] = ai >>= bi;
    })
  }
  state->ip = (int64_t)bc;
fin:;
  return retval;
}
static char *abc_mem = NULL;
static int64_t abc_mem_sz = 0;
void AiwnBCAddMem(int64_t bytes) {
  abc_mem_sz += bytes;
  if (abc_mem == NULL) {
    abc_mem = calloc(1, abc_mem_sz);
  } else
    abc_mem = realloc(abc_mem, abc_mem_sz);
}
static CCodeMiscRef *CodeMiscAddRefBC(CCodeMisc *misc, int32_t *addr) {
  CCodeMiscRef *ref;
  *(ref = calloc(sizeof(CCodeMiscRef), 1)) = (CCodeMiscRef){
      .add_to = addr,
      .next = misc->refs,
  };
  misc->refs = ref;
  return ref;
}
static CRPN *DerefOffset(CRPN *rpn, int64_t *off) {
  int64_t o = 0;
  CRPN *a, *b;
again:;
  if (rpn->type == IC_ADD) {
    a = ICArgN(rpn, 0);
    b = ICArgN(rpn, 1);
    if (a->type == IC_I64) {
      o += a->integer;
      rpn = b;
      goto again;
    }
    if (b->type == IC_I64) {
      o += b->integer;
      rpn = a;
      goto again;
    }
  } else if (rpn->type == IC_SUB) {
    a = ICArgN(rpn, 0);
    b = ICArgN(rpn, 1);
    if (a->type == IC_I64) {
      o -= a->integer;
      rpn = b;
      goto again;
    }
  }
  if (off)
    *off = o;
  return rpn;
}
static int64_t CompileToBC0(CCmpCtrl *cc, ABC_PTR ptr, CRPN *rpn, int64_t len) {
  CCodeMiscRef *cmr;
  CRPN *arg0 = ICArgN(rpn, 0), *arg1 = ICArgN(rpn, 1);
  CRPN *head;
  int64_t t, a, enter_addr, ienter_addr, off;
  switch (rpn->type) {
  case IC_GOTO:
    len = AiwnBCAddCode(ptr, ABC_GOTO, len);
    if (ptr) {
      cmr = CodeMiscAddRefBC(rpn->code_misc, (void *)(ptr + len));
      cmr->offset = 4;
      cmr->is_rel = 1;
    }
    break;
  case IC_GOTO_IF:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = AiwnBCAddCode(ptr, ABC_GOTO_IF | (arg0->raw_type << 8), len);
    if (ptr) {
      cmr = CodeMiscAddRefBC(rpn->code_misc, (void *)(ptr + len));
      cmr->offset = 4;
      cmr->is_rel = 1;
    }
    len = AiwnBCAddCode(ptr, 0, len);
    len = AiwnBCAddCode(ptr, 0, len);
    break;
  case IC_TO_I64:
    len = CompileToBC0(cc, ptr, arg0, len);
    if (arg0->raw_type != RT_F64)
      break;
    len = AiwnBCAddCode(ptr, ABC_TO_I64, len);
    break;
  case IC_TO_F64:
    len = CompileToBC0(cc, ptr, arg0, len);
    if (arg0->raw_type == RT_F64)
      break;
    len = AiwnBCAddCode(ptr, ABC_TO_F64, len);
    break;
  case IC_LABEL:
    if (ptr)
      rpn->code_misc->addr = ptr + len;
    break;
  case IC_STATIC:
    len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);
    len = AiwnBCAddCode(ptr, rpn->integer, len);
    len = AiwnBCAddCode(ptr, rpn->integer >> 32, len);
    if (ptr) {
      cmr = CodeMiscAddRefBC(rpn->code_misc, (void *)(ptr + len - 8));
      cmr->offset = rpn->integer;
    }
    len = AiwnBCAddCode(ptr, ABC_READ_PTR | (rpn->raw_type << 8), len);
    break;
  case IC_NOP:
    break;
  case IC_NEG:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_NEG | (rpn->raw_type << 8), len);
    break;
  case IC_POS:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_POS | (rpn->raw_type << 8), len);
    break;
  case IC_STR:
    len = AiwnBCAddCode(ptr, ABC_PCREL, len);
    len = AiwnBCAddCode(ptr, 0, len);
    len = AiwnBCAddCode(ptr, 0, len);
    if (ptr) {
      cmr = CodeMiscAddRefBC(rpn->code_misc, (void *)(ptr + len - 8));
      cmr->is_rel = 1;
      cmr->offset = 4;
    }
    break;
  case IC_CHR:
    len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);
    len = AiwnBCAddCode(ptr, rpn->integer, len);
    len = AiwnBCAddCode(ptr, rpn->integer >> 32ull, len);
    break;
  case IC_POW:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_POW | (rpn->raw_type << 8), len);
    break;
  case IC_ADD:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_ADD | (rpn->raw_type << 8), len);
    break;
  case IC_EQ:
  t=arg1->raw_type;
      if (arg1->type == IC_TYPECAST) {
      arg1 = ICArgN(arg1, 0);
    }
  #define ASN_DO  \
      len = CompileToBC0(cc, ptr, arg0, len); \
    len = ForceType(ptr, arg0, t, len);

    if (arg1->type == IC_STATIC) {
      len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);
      len = AiwnBCAddCode(ptr, arg1->integer, len);
      len = AiwnBCAddCode(ptr, arg1->integer >> 32, len);
      ASN_DO;
      len = AiwnBCAddCode(ptr, ABC_WRITE_PTRO | (t << 8), len);
      len = AiwnBCAddCode(ptr, arg1->integer, len);
    } else if (arg1->type == IC_BASE_PTR) {
      len = AiwnBCAddCode(ptr, ABC_FRAME_ADDR, len);
      ASN_DO;
      len = AiwnBCAddCode(ptr, ABC_WRITE_PTRO | (t << 8), len);
      len = AiwnBCAddCode(ptr, -arg1->integer, len);
    } else if (arg1->type == IC_DEREF) {
      arg1 = arg1->base.next;
      arg1 = DerefOffset(arg1, &off);
      len = CompileToBC0(cc, ptr, arg1, len);
      len = ForceType(ptr, arg1, RT_PTR, len);
      ASN_DO;
      len = AiwnBCAddCode(ptr, ABC_WRITE_PTRO | (t << 8), len);
      len = AiwnBCAddCode(ptr, off, len);
    } else if (arg1->type == IC_GLOBAL) {
#define GLOB_PTR(rpn)                                                          \
  if (rpn->global_var->base.type & HTT_GLBL_VAR) {                             \
    enter_addr = (uint64_t)rpn->global_var->data_addr;                         \
    ienter_addr = (uint64_t)&rpn->global_var->data_addr;                       \
  } else if (rpn->global_var->base.type & HTT_FUN) {                           \
    enter_addr = (uint64_t)((CHashFun *)rpn->global_var)->fun_ptr;             \
    ienter_addr = (uint64_t)&((CHashFun *)rpn->global_var)->fun_ptr;           \
  } else                                                                       \
    abort();                                                                   \
  len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);                                  \
  len = AiwnBCAddCode(ptr, ienter_addr, len);                                  \
  len = AiwnBCAddCode(ptr, ienter_addr >> 32ul, len);                          \
  len = AiwnBCAddCode(ptr, ABC_READ_PTR | (RT_I64i << 8), len);

      GLOB_PTR((arg1));

ASN_DO;
      len = AiwnBCAddCode(ptr, ABC_WRITE_PTR | (t << 8), len);
    } else
      abort();
    break;
  case IC_SUB:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_SUB | (rpn->raw_type << 8), len);
    break;
  case IC_DIV:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_DIV | (rpn->raw_type << 8), len);
    break;
  case IC_MUL:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_MUL | (rpn->raw_type << 8), len);
    break;
  case IC_DEREF:
    arg0 = DerefOffset(arg0, &off);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, RT_I64i, len);
    len = AiwnBCAddCode(ptr, ABC_READ_PTRO | (rpn->raw_type << 8), len);
    len = AiwnBCAddCode(ptr, off, len);
    break;
  case IC_AND:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_AND | (rpn->raw_type << 8), len);
    break;
  case IC_ADDR_OF:
#define GET_PTR                                                                \
  off = 0;                                                                     \
  if (arg0->type == IC_TYPECAST) {                                             \
    t = arg0->raw_type;                                                        \
    arg0 = ICArgN(arg0, 0);                                                    \
  } else                                                                       \
    t = arg0->raw_type;                                                        \
  if (arg0->type == IC_STATIC) {                                               \
    len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);                                \
    len = AiwnBCAddCode(ptr, arg0->integer, len);                              \
    len = AiwnBCAddCode(ptr, arg0->integer >> 32, len);                        \
  } else if (arg0->type == IC_BASE_PTR) {                                      \
    len = AiwnBCAddCode(ptr, ABC_FRAME_ADDR, len);                             \
    len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);                                \
    len = AiwnBCAddCode(ptr, -arg0->integer, len);                             \
    len = AiwnBCAddCode(ptr, -arg0->integer >> 32, len);                       \
    len = AiwnBCAddCode(ptr, ABC_ADD | (RT_I64i << 8), len);                   \
  } else if (arg0->type == IC_DEREF) {                                         \
    arg0 = arg0->base.next;                                                    \
    len = CompileToBC0(cc, ptr, arg0, len);                                    \
    len = ForceType(ptr, arg0, RT_PTR, len);                                   \
  } else if (arg0->type == IC_GLOBAL) {                                        \
    GLOB_PTR((arg0));                                                          \
  } else                                                                       \
    abort();
    GET_PTR;
    break;
  case IC_XOR:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_XOR | (rpn->raw_type << 8), len);
    break;
  case IC_GLOBAL:
    GLOB_PTR((rpn));
    if (rpn->global_var->base.type & HTT_FUN)
      break;
    len = AiwnBCAddCode(ptr, ABC_READ_PTR | (rpn->raw_type << 8), len);
    break;
  case IC_MOD:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_MOD | (rpn->raw_type << 8), len);
    break;
  case IC_OR:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_OR | (rpn->raw_type << 8), len);
    break;
  case IC_LT:
    goto cmp_style;
  case IC_GT:
    goto cmp_style;
  case IC_LNOT:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = AiwnBCAddCode(ptr, ABC_LNOT | (arg0->raw_type << 8), len);
    break;
  case IC_BNOT:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_BNOT | (rpn->raw_type << 8), len);
    break;
  case IC_AND_AND: {
    int64_t skipo;
    len = CompileToBC0(cc, ptr, arg1, len);
    skipo = len;
    len = AiwnBCAddCode(ptr, ABC_SKIP_ON_FAIL | (arg1->raw_type << 8), len);
    len = AiwnBCAddCode(ptr, 0, len);
    len = AiwnBCAddCode(ptr, ABC_DISCARD, len);
    len = AiwnBCAddCode(ptr, 1, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = AiwnBCAddCode(ptr, ABC_TO_BOOL | (arg0->raw_type << 8), len);
    if (ptr)
      *(uint32_t *)(&ptr[skipo + 4]) = len - skipo;
  } break;
  case IC_OR_OR: {
    int64_t skipo;
    len = CompileToBC0(cc, ptr, arg1, len);
    skipo = len;
    len = AiwnBCAddCode(ptr, ABC_SKIP_ON_PASS | (arg1->raw_type << 8), len);
    len = AiwnBCAddCode(ptr, 0, len);
    len = AiwnBCAddCode(ptr, ABC_DISCARD, len);
    len = AiwnBCAddCode(ptr, 1, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = AiwnBCAddCode(ptr, ABC_TO_BOOL | (arg0->raw_type << 8), len);
    if (ptr)
      *(uint32_t *)(&ptr[skipo + 4]) = len - skipo;
  } break;
  case IC_EQ_EQ:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_EQ_EQ | (rpn->raw_type << 8), len);
    break;
  case IC_NE:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_NE | (rpn->raw_type << 8), len);
    break;
  case IC_LE:
  cmp_style: {
    CRPN *next;
    int64_t range[IM_STK_SIZE];
    CRPN *range_args[IM_STK_SIZE];
    int64_t skipos[IM_STK_SIZE];
    int64_t cnt = 0, i = 0;
    for (next = rpn; 1; next = ICFwd((CRPN *)next->base.next) // TODO explain
    ) {
      switch (next->type) {
      case IC_LT:
      case IC_GT:
      case IC_LE:
      case IC_GE:
        goto cmp_pass;
      }
      range_args[cnt] = next; // See below
      break;
    cmp_pass:
      //
      // <
      //    right rpn.next
      //    left  ICFwd(rpn.next)
      //
      range_args[cnt] = (CRPN *)next->base.next;
      range[cnt] = next->type;
      cnt++;
    }
    t = RT_I64i;
    for (i = 0; i <= cnt; ++i) {
      if (range_args[i]->raw_type == RT_F64)
        t = RT_F64;
      else if (range_args[i]->raw_type == RT_U64i && t != RT_F64)
        t = RT_U64i;
    }
    arg1 = range_args[cnt];
    len = CompileToBC0(cc, ptr, arg1, len);
    if (t == RT_F64 && arg1->raw_type != RT_F64)
      len = AiwnBCAddCode(ptr, ABC_TO_F64 | (t << 8), len);
    i = cnt;
    while (--cnt >= 0) {
      arg0 = range_args[cnt];
      len = CompileToBC0(cc, ptr, arg0, len);
      if (t == RT_F64 && arg0->raw_type != RT_F64)
        len = AiwnBCAddCode(ptr, ABC_TO_F64 | (t << 8), len);
      len = AiwnBCAddCode(ptr, ABC_DUP | (t << 8), len);
      len = AiwnBCAddCode(ptr, ABC_PUSH, len);
      switch (range[cnt]) {
        break;
      case IC_LT:
        len = AiwnBCAddCode(ptr, ABC_LT | (t << 8), len);
        break;
      case IC_LE:
        len = AiwnBCAddCode(ptr, ABC_LE | (t << 8), len);
        break;
      case IC_GE:
        len = AiwnBCAddCode(ptr, ABC_GE | (t << 8), len);
        break;
      case IC_GT:
        len = AiwnBCAddCode(ptr, ABC_GT | (t << 8), len);
      }
      if (cnt) {
        skipos[cnt] = len;
        len = AiwnBCAddCode(ptr, ABC_SKIP_ON_FAIL | (RT_I64i << 8), len);
        len = AiwnBCAddCode(ptr, 0, len);
        len = AiwnBCAddCode(ptr, ABC_POP | (t << 8), len);
      }
    }

    if (ptr)
      for (cnt = i; --cnt >= 0;) {
        if (cnt)
          *(uint32_t *)(ptr + skipos[cnt] + 4) = len - skipos[cnt];
      }
    len = AiwnBCAddCode(ptr, ABC_DISCARD_STK, len);
    len = AiwnBCAddCode(ptr, 8, len);
  } break;
  case IC_GE:
    goto cmp_style;
  case IC_LSH:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_LSH | (rpn->raw_type << 8), len);
    break;
  case IC_RSH:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = ForceType(ptr, arg1, rpn->raw_type, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = ForceType(ptr, arg0, rpn->raw_type, len);
    len = AiwnBCAddCode(ptr, ABC_RSH | (rpn->raw_type << 8), len);
    break;
  case IC_ADD_EQ:
#define ASSIGN_OP(xxx)                                                         \
      t = arg1->raw_type; \
      if(arg1->type==IC_TYPECAST) \
		arg1=arg1->base.next; \
  if (arg1->type == IC_STATIC) {                                               \
    len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);                                \
    len = AiwnBCAddCode(ptr, 0, len);                                          \
    len = AiwnBCAddCode(ptr, 0, len);                                          \
    if (ptr) {                                                                 \
      cmr = CodeMiscAddRefBC(cc->statics_label, ptr + len - 8);                \
      cmr->offset = arg1->integer;                                             \
    }                 len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);                                \
    len = AiwnBCAddCode(ptr, arg1->integer, len);                              \
    len = AiwnBCAddCode(ptr, arg1->integer >> 32, len);                        \
    len = AiwnBCAddCode(ptr, ABC_READ_PTRO | (t << 8), len);                   \
    len = AiwnBCAddCode(ptr, -arg1->integer, len);                             \
    ASN_DO; \
    len = AiwnBCAddCode(ptr, xxx | (t << 8), len);                             \
    len = AiwnBCAddCode(ptr, ABC_WRITE_PTRO | (t << 8), len);                  \
    len = AiwnBCAddCode(ptr, -arg1->integer, len);                             \
  } else if (arg1->type == IC_BASE_PTR) {                                      \
    len = AiwnBCAddCode(ptr, ABC_FRAME_ADDR, len);                             \
    len = AiwnBCAddCode(ptr, ABC_FRAME_ADDR, len);                             \
    len = AiwnBCAddCode(ptr, ABC_READ_PTRO | (t << 8), len);                   \
    len = AiwnBCAddCode(ptr, -arg1->integer, len);                             \
    ASN_DO; \
    len = AiwnBCAddCode(ptr, xxx | (t << 8), len);                             \
    len = AiwnBCAddCode(ptr, ABC_WRITE_PTRO | (t << 8 ), len);             \
    len = AiwnBCAddCode(ptr, -arg1->integer, len);                             \
  } else if (arg1->type == IC_DEREF) {                                         \
    arg1 = DerefOffset(arg1->base.next, &off);                                 \
    len = CompileToBC0(cc, ptr, arg1, len);                                    \
    len = ForceType(ptr, arg1, RT_PTR, len);                                   \
    len = AiwnBCAddCode(ptr, ABC_DUP, len);                                    \
    len = AiwnBCAddCode(ptr, ABC_READ_PTRO | (t << 8), len);                   \
    len = AiwnBCAddCode(ptr, off, len);                                        \
    ASN_DO; \
    len = AiwnBCAddCode(ptr, xxx | (t << 8), len);                             \
    len = AiwnBCAddCode(ptr, ABC_WRITE_PTRO | (t << 8), len);                  \
    len = AiwnBCAddCode(ptr, off, len);                                        \
  } else if (arg1->type == IC_GLOBAL) {                                        \
    GLOB_PTR(arg1);                                                            \
		len = AiwnBCAddCode(ptr, ABC_DUP, len);                                    \
    len = AiwnBCAddCode(ptr, ABC_READ_PTR | (t << 8), len);                    \
    ASN_DO; \
    len = AiwnBCAddCode(ptr, xxx | (t << 8), len);                             \
    len = AiwnBCAddCode(ptr, ABC_WRITE_PTR | (t << 8), len);                   \
                                                                               \
  } else                                                                       \
    abort();
    ASSIGN_OP(ABC_ADD);
    break;
  case IC_SUB_EQ:
    ASSIGN_OP(ABC_SUB);
    break;
  case IC_MUL_EQ:
    ASSIGN_OP(ABC_MUL);
    break;
  case IC_DIV_EQ:
    ASSIGN_OP(ABC_DIV);
    break;
  case IC_LSH_EQ:
    ASSIGN_OP(ABC_LSH);
    break;
  case IC_RSH_EQ:
    ASSIGN_OP(ABC_RSH);
    break;
  case IC_MOD_EQ:
    ASSIGN_OP(ABC_MOD);
    break;
  case IC_AND_EQ:
    ASSIGN_OP(ABC_AND);
    break;
  case IC_OR_EQ:
    ASSIGN_OP(ABC_OR);
    break;
  case IC_XOR_EQ:
    ASSIGN_OP(ABC_XOR);
    break;
  case IC_I64:
    len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);
    len = AiwnBCAddCode(ptr, rpn->integer, len);
    len = AiwnBCAddCode(ptr, rpn->integer >> 32, len);
    break;
  case IC_F64:
    len = AiwnBCAddCode(ptr, ABC_IMM_F64, len);
    len = AiwnBCAddCode(ptr, rpn->integer, len); // unioned
    len = AiwnBCAddCode(ptr, rpn->integer >> 32, len);
    break;
  case IC_RET:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = AiwnBCAddCode(ptr, ABC_RET | (arg0->raw_type << 8), len);
    break;
  case __IC_CALL:
  case IC_CALL:
    t = 0;
    for (a = 0; a < rpn->length; ++a) {
      arg1 = ICArgN(rpn, a);
      if (arg1->type == __IC_VARGS)
        t = arg1->length;
      else
        ++t;
      len = CompileToBC0(cc, ptr, arg1, len);
      if (arg1->type != __IC_VARGS)
        len = AiwnBCAddCode(
            ptr,
            ABC_PUSH | ((arg1->raw_type == RT_F64 ? RT_F64 : RT_I64i) << 8),
            len);
    }
    arg0 = ICArgN(rpn, rpn->length);
    len = CompileToBC0(cc, ptr, arg0, len);
    printf("CALL:%d\n", rpn->raw_type);
    len = AiwnBCAddCode(ptr, ABC_CALL | (rpn->raw_type << 8), len);
    len = AiwnBCAddCode(ptr, ABC_DISCARD_STK, len);
    len = AiwnBCAddCode(ptr, t * 8, len);
    break;
  case IC_COMMA:
    len = CompileToBC0(cc, ptr, arg1, len);
    len = CompileToBC0(cc, ptr, arg0, len);
    len = AiwnBCAddCode(ptr, ABC_COMMA, len);
    break;
  case IC_UNBOUNDED_SWITCH:
  case IC_BOUNDED_SWITCH:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);
    len = AiwnBCAddCode(ptr, -rpn->code_misc->lo, len);
    len = AiwnBCAddCode(ptr, -rpn->code_misc->lo >> 32, len);
    len = AiwnBCAddCode(ptr, ABC_ADD | (RT_I64i << 8), len);
    len = AiwnBCAddCode(ptr, ABC_SWITCH, len);
    len =
        AiwnBCAddCode(ptr, (rpn->code_misc->hi - rpn->code_misc->lo + 1), len);
    len = AiwnBCAddCode(ptr, 0, len);
    len = AiwnBCAddCode(ptr, 0, len);
    if (ptr) {
      cmr = CodeMiscAddRefBC(rpn->code_misc, (void *)(ptr + len - 8));
      cmr->is_rel = 1;
      cmr->offset = 4 + 4;
    }
    len = AiwnBCAddCode(ptr, 0, len);
    len = AiwnBCAddCode(ptr, 0, len);
    if (ptr) {
      cmr = CodeMiscAddRefBC(rpn->code_misc->dft_lab, (void *)(ptr + len - 8));
      cmr->is_rel = 1;
      cmr->offset = 4 + 4 + 8;
    }
    break;
    break;
  case IC_SUB_RET:
    len = AiwnBCAddCode(ptr, ABC_SUB_RET, len);
    break;
  case IC_SUB_PROLOG:
    len = AiwnBCAddCode(ptr, ABC_SUB_PROLOG, len);
    break;
  case IC_SUB_CALL:
    len = AiwnBCAddCode(ptr, ABC_SUB_CALL, len);
    len = AiwnBCAddCode(ptr, 0, len);
    len = AiwnBCAddCode(ptr, 0, len);
    if (ptr) {
      cmr = CodeMiscAddRefBC(rpn->code_misc, (void *)(ptr + len - 8));
      cmr->offset = 4;
      cmr->is_rel = 1;
    }
    break;
  case IC_TYPECAST:
    t = rpn->raw_type == RT_F64 ? RT_F64 : RT_I64i;
    len = CompileToBC0(cc, ptr, arg0, len);
    if (arg0->raw_type == t)
      ; // do nothing
    else
      // the things are unioned,no need to typecasst
      ; // len = AiwnBCAddCode(ptr, ABC_TYPECAST | (t << 8), len);
    break;
  case IC_BASE_PTR:
    len = AiwnBCAddCode(ptr, ABC_FRAME_ADDR, len);
    len = AiwnBCAddCode(ptr, ABC_READ_PTRO | (rpn->raw_type << 8), len);
    len = AiwnBCAddCode(ptr, -rpn->integer, len);
    break;
  case __IC_VARGS:
    for (a = 0; a < rpn->length; ++a) {
      arg1 = ICArgN(rpn, a);
      len = CompileToBC0(cc, ptr, arg1, len);
      len = AiwnBCAddCode(ptr, ABC_PUSH | (arg1->raw_type << 8), len);
    }
    break;
  case __IC_ARG:
    len = AiwnBCAddCode(ptr, ABC_FRAME_ADDR, len);
    len = AiwnBCAddCode(ptr, ABC_FRAME_ADDR, len);
    len = AiwnBCAddCode(ptr, ABC_READ_PTRO | (RT_I64i << 8), len);
    len = AiwnBCAddCode(ptr, sizeof(ABCFrame) + 16 + 8 * rpn->integer, len);
    len = AiwnBCAddCode(ptr, ABC_WRITE_PTRO | (arg0->raw_type << 8), len);
    len = AiwnBCAddCode(ptr, -arg0->integer, len);

    break;
  case IC_RELOC:
    len = AiwnBCAddCode(ptr, ABC_PCREL, len);
    len = AiwnBCAddCode(ptr, 0, len);
    len = AiwnBCAddCode(ptr, 0, len);
    if (ptr) {
      cmr = CodeMiscAddRefBC(rpn->code_misc, (void *)(ptr + len - 8));
      cmr->is_rel = 1;
      cmr->offset = 4;
    }
    len = AiwnBCAddCode(ptr, ABC_READ_PTR | (RT_I64i << 8), len);
    break;
  case __IC_SET_FRAME_SIZE:
    len = AiwnBCAddCode(ptr, ABC_SET_FRAME_SIZE, len);
    len = AiwnBCAddCode(ptr, rpn->integer, len);
    break;
  case __IC_STATIC_REF:
    len = AiwnBCAddCode(ptr, ABC_PCREL, len);
    len = AiwnBCAddCode(ptr, 0, len);
    len = AiwnBCAddCode(ptr, 0, len);
    if (cc->statics_label) {
      cmr = CodeMiscAddRefBC(cc->statics_label, (void *)(ptr + len - 8));
      cmr->offset = rpn->integer + 4;
      cmr->is_rel = 1;
    }
    break;
  case __IC_SET_STATIC_DATA:
    break;
  case __IC_STATICS_SIZE:
    break;
  case IC_GET_VARGS_PTR:
    head = (CRPN *)cc->code_ctrl->ir_code;
    t = 0;
    for (arg0 = (CRPN *)head->base.next; arg0 != (CRPN *)head;
         arg0 = (CRPN *)arg0->base.next) {
      if (arg0->type == __IC_ARG)
        ++t;
    }
    arg0 = ICArgN(rpn, 0);
    len = AiwnBCAddCode(ptr, ABC_FRAME_ADDR, len);
    len = AiwnBCAddCode(ptr, ABC_FRAME_ADDR, len);
    len = AiwnBCAddCode(ptr, ABC_IMM_I64, len);
    len = AiwnBCAddCode(ptr, (16 + sizeof(ABCFrame) + 8 * t), len);
    len = AiwnBCAddCode(ptr, (16 + sizeof(ABCFrame) + 8 * t) >> 32, len);
    len = AiwnBCAddCode(ptr, ABC_ADD | (RT_I64i << 8), len);
    len = AiwnBCAddCode(ptr, ABC_WRITE_PTRO | (RT_I64i << 8), len);
    len = AiwnBCAddCode(ptr, -arg0->integer, len);
    break;
  case IC_TO_BOOL:
    len = CompileToBC0(cc, ptr, arg0, len);
    len = AiwnBCAddCode(ptr, ABC_TO_BOOL | (arg0->raw_type << 8), len);
    break;
  case IC_FS:
    len = AiwnBCAddCode(ptr, ABC_FS, len);
    break;
  case IC_GS:
    len = AiwnBCAddCode(ptr, ABC_GS, len);
    break;
  case IC_POST_DEC:;
    GET_PTR;
    len = AiwnBCAddCode(ptr, ABC_POST_INC | (t << 8), len);
    len = AiwnBCAddCode(ptr, -rpn->integer, len);
    len = AiwnBCAddCode(ptr, -rpn->integer >> 32, len);
    break;
  case IC_POST_INC:
    GET_PTR;
    len = AiwnBCAddCode(ptr, ABC_POST_INC | (t << 8), len);
    len = AiwnBCAddCode(ptr, rpn->integer, len);
    len = AiwnBCAddCode(ptr, rpn->integer >> 32, len);
    break;
  case IC_PRE_DEC:
    GET_PTR;
    len = AiwnBCAddCode(ptr, ABC_PRE_INC | (t << 8), len);
    len = AiwnBCAddCode(ptr, -rpn->integer, len);
    len = AiwnBCAddCode(ptr, -rpn->integer >> 32, len);
    break;
  case IC_PRE_INC:
    GET_PTR;
    len = AiwnBCAddCode(ptr, ABC_PRE_INC | (t << 8), len);
    len = AiwnBCAddCode(ptr, rpn->integer, len);
    len = AiwnBCAddCode(ptr, rpn->integer >> 32, len);
  }
  return len;
}
char *OptPassFinalBC(CCmpCtrl *cctrl, int64_t *res_sz, char **dbg_info,
                     CHeapCtrl *heap) {
  int64_t len = 0, stats = 0, insts = 0, i;
  ABC_PTR bin = NULL;
  CHashImport *import;
  CCodeMisc *cm, *to;
  CCodeMiscRef *cmr;
  CRPN *head = (CRPN *)cctrl->code_ctrl->ir_code, *cur;
  cctrl->statics_label = CodeMiscNew(cctrl, CMT_LABEL);
  for (cur = (CRPN *)head->base.next; cur != head; cur = ICFwd(cur))
    insts++;
  CRPN *forwards[insts];
  for (i = 1, cur = (CRPN *)head->base.next; cur != head;
       cur = ICFwd(cur), ++i) {
    forwards[insts - i] = cur;
  }
again:;
  len = 0;
  CMemberLst *lst;
  if (cctrl->cur_fun) {
    lst = cctrl->cur_fun->base.members_lst;
    while (lst) {
      lst = lst->next;
    }
    lst = cctrl->cur_fun->base.members_lst;

    for (i = 0; i != cctrl->cur_fun->argc; i++) {
      len = AiwnBCAddCode(bin, ABC_FRAME_ADDR, len);
      len = AiwnBCAddCode(bin, ABC_FRAME_ADDR, len);
      len = AiwnBCAddCode(
          bin, ABC_READ_PTRO | (lst->member_class->raw_type << 8), len);
      len = AiwnBCAddCode(bin, sizeof(ABCFrame) + 16 + 8 * i, len);
      len = AiwnBCAddCode(
          bin, ABC_WRITE_PTRO | (lst->member_class->raw_type << 8), len);
      len = AiwnBCAddCode(bin, -lst->off, len);
      if ((cctrl->cur_fun->base.flags & CLSF_VARGS) &&
          !strcmp("argv", lst->str)) {
        len = AiwnBCAddCode(bin, ABC_FRAME_ADDR, len);
        len = AiwnBCAddCode(bin, ABC_FRAME_ADDR, len);
        len = AiwnBCAddCode(bin, ABC_IMM_I64, len);
        len = AiwnBCAddCode(bin, sizeof(ABCFrame) + 16 + 8 * i, len);
        len = AiwnBCAddCode(bin, (sizeof(ABCFrame) + 16 + 8 * i) >> 32ul, len);
        len = AiwnBCAddCode(bin, ABC_ADD | (RT_I64i << 8), len);
        len = AiwnBCAddCode(bin, ABC_WRITE_PTRO | (RT_I64i << 8), len);
        len = AiwnBCAddCode(bin, -lst->off, len);
      }
      lst = lst->next;
    }
    len = AiwnBCAddCode(bin, ABC_SET_FRAME_SIZE, len);
    len = AiwnBCAddCode(bin, cctrl->cur_fun->base.sz + 16, len);
  }
  for (cur = (CRPN *)head->base.next; cur != head;
       cur = (CRPN *)cur->base.next) {
    if (cur->type == __IC_SET_FRAME_SIZE) {
      len = CompileToBC0(cctrl, bin, cur, len);
    } else if (cur->type == __IC_STATICS_SIZE)
      stats = cur->integer;
  }
  for (i = 0; i != insts; ++i) {
    cur = forwards[i];
    if (cur->type != __IC_SET_FRAME_SIZE) {
      len = CompileToBC0(cctrl, bin, cur, len);
      len = AiwnBCAddCode(bin, ABC_END_EXPR, len);
    }
  }
  len = AiwnBCAddCode(bin, ABC_IMM_I64,len);
  len = AiwnBCAddCode(bin, 0,len);
  len = AiwnBCAddCode(bin, 0,len);
  len = AiwnBCAddCode(bin, ABC_RET, len);
  len = AiwnBCAddCode(bin, ABC_MISC_START, len);
  for (cm = (CCodeMisc *)cctrl->code_ctrl->code_misc->next;
       cm != (CCodeMisc *)cctrl->code_ctrl->code_misc;
       cm = (CCodeMisc *)cm->base.next) {
    switch (cm->type) {
      break;
    case CMT_STATIC_DATA:
      if (bin) {
        memcpy(cctrl->statics_label->addr + cm->integer, cm->str, cm->str_len);
      }
      break;
    case CMT_RELOC_U64:
      cm->addr = bin + len;
      len += 8;
      if (bin) {
        *(import = A_CALLOC(sizeof(CHashImport), NULL)) = (CHashImport){
            .base =
                {
                    .type = HTT_IMPORT_SYS_SYM,
                    .str = A_STRDUP(cm->str, NULL),
                },
            .address = cm->addr,
        };
        HashAdd(import, Fs->hash_table);
      }
      goto fill_in_refs;
      break;
    case CMT_LABEL:
    fill_in_refs:
      if (cm->patch_addr)
        *cm->patch_addr = cm->addr;
      while ((cmr = cm->refs)) {
        if (cmr->is_rel) {
          *(int64_t **)cmr->add_to =
              (cm->addr + cmr->offset) - (int64_t)cmr->add_to;
        } else
          *(int64_t **)cmr->add_to = cm->addr + cmr->offset;
        cm->refs = cmr->next;
        free(cmr);
      }
      cm->refs = NULL;
      break;
    case CMT_JMP_TAB:
      to = 0;
      cm->addr = bin + len;
      for (i = 0; i != cm->hi - cm->lo + 1; ++i) {
        if (bin) {
          to = cm->jmp_tab[i];
          if (!to)
            to = cm->dft_lab;
          ((uint64_t *)(bin + len))[i] = (uint64_t)to->addr;
        }
      }
      len += 8 * i;
      goto fill_in_refs;
      break;
    case CMT_STRING:
      if (cctrl->flags & CCF_STRINGS_ON_HEAP) {
        cm->addr = A_CALLOC(cm->str_len + 1, NULL);
      } else {
		  len+=4;
		  if(bin) *(int32_t*)(bin+len)=cm->str_len;
        cm->addr = bin + len;
	  }
      if (bin) {
        memcpy((char *)cm->addr, cm->str, cm->str_len);
        ((char *)cm->addr)[cm->str_len] = 0;
      }
      len += cm->str_len + 1;
      goto fill_in_refs;
    }
  }

  len += stats;
  if (bin == NULL) {
    bin = A_CALLOC(len, NULL);
    cctrl->statics_label->addr = bin + len - stats;
    goto again;
  }
  // Reset code miscs
  for (cm = (CCodeMisc *)cctrl->code_ctrl->code_misc->next;
       cm != (CCodeMisc *)cctrl->code_ctrl->code_misc;
       cm = (CCodeMisc *)cm->base.next) {
    cm->addr = NULL;
  }
  if (res_sz)
    *res_sz = len;
  return bin;
}
#ifdef USE_BYTECODE
char *OptPassFinal(CCmpCtrl *cctrl, int64_t *res_sz, char **dbg_info,
                   CHeapCtrl *heap) {
  return OptPassFinalBC(cctrl, res_sz, dbg_info, heap);
}
#endif
ABCState *ABCStateNew(void *bc_addr, void *stk_ptr, int64_t argc,
                      int64_t *argv) {
  ABCState *state = calloc(1, sizeof(ABCState));
  stk_ptr -= sizeof(ABCFrame);
  ABCFrame *dummy = (void *)stk_ptr;

  int64_t *args = stk_ptr;
  args -= argc;
  while (--argc >= 0) {
    args[argc] = argv[argc];
  }
  state->ip = (int64_t)bc_addr;
  state->_sp = (int64_t)args;
  state->fp = state->_sp -= 16 + sizeof(ABCFrame);
  state->fun_frame = state->fp + 16;
  memset(dummy, 0, sizeof(ABCFrame));
  memset(state->fun_frame, 0, sizeof(ABCFrame));
  state->fun_frame->next = dummy;
  *(int64_t *)state->fp = NULL;
  ((int64_t *)state->fp)[1] = NULL;
  return state;
}

void ABCStateDel(ABCState *st) {
  ABCFrame *f = st->fun_frame, *next;
  while (f) {
    next = f->next;
    f = next;
  }
  free(st);
}

int64_t ABCRun_Done(ABCState *st, int64_t *retval) {
  if (!st->ip) {
    return 1;
  }
  if (retval)
    *retval = AiwnRunBC(st);
  else
    AiwnRunBC(st);
  return st->ip == 0;
}
int64_t ABCRun(ABCState *st) {
  int64_t ret = 0;
  while (!ABCRun_Done(st, &ret))
    ;
  return ret;
}
void AiwnBCDel(char *bc) {
  free(bc);
}
int64_t AiwnBCCall(void *bc) {
  ABCState *state = cur_bcstate;
  int64_t ret, old_fp = state->fp, old_ip = state->ip;
  state->fp = state->_sp -= 16;
  *(int64_t *)state->fp = old_fp;
  ((int64_t *)state->fp)[1] = 0;
  state->ip = (int64_t)bc;

  while (!ABCRun_Done(state, &ret))
    ;
  state->ip = old_ip;
  return ret;
}

int64_t AiwnBCCallArgs(void *bc, int64_t argc, int64_t *argv) {
  ABCState *state = cur_bcstate;
  int64_t ret, old_fp = state->fp, old_ip = state->ip, old_sp = state->_sp;
  state->_sp -= 8 * argc;
  memcpy((void *)state->_sp, argv, 8 * argc);
  state->fp = state->_sp -= 16;
  *(int64_t *)state->fp = old_fp;
  ((int64_t *)state->fp)[1] = 0;
  state->ip = (int64_t)bc;

  while (!ABCRun_Done(state, &ret))
    ;
  state->ip = old_ip;
  state->_sp = old_sp;
  return ret;
}

static int64_t BCGenerateFFICall0(uint32_t *ptr, void *fcall) {
  int64_t len = 0;
  len = AiwnBCAddCode(ptr, ABC_INTERN, len);
  len = AiwnBCAddCode(ptr, (uint64_t)fcall, len);
  len = AiwnBCAddCode(ptr, (uint64_t)fcall >> 32ull, len);
  len = AiwnBCAddCode(ptr, ABC_RET, len);
  return len;
}
uint32_t *BCGenerateFFICall(void *fcall) {
  int64_t len = BCGenerateFFICall0(NULL, fcall);
  uint32_t *ptr = calloc(1, len);
  BCGenerateFFICall0(ptr, fcall);
  return ptr;
}

// to_stk as its a "naked" function
uint64_t AiwnBCContextGet(ABCState **to_stk) {
  ABCState *to = to_stk[0];
  ABCState *state = cur_bcstate;
  int64_t old_fp = *(int64_t *)state->fp;
  int64_t old_ip = ((int64_t *)state->fp)[1];
  ABCFrame *fr = (ABCFrame *)(old_fp + 16);
  to->_sp = (int64_t)state->fp + 16 + sizeof(ABCFrame);
  to->fp = old_fp;
  to->ip = old_ip;
  to->fun_frame = fr;
  return 0;
}
// to_stk as its a "naked" function
uint64_t AiwnBCContextSet(ABCState **to_stk) {
  ABCState *to = to_stk[0];
  ABCState *state = cur_bcstate;
  ABCFrame *cur = state->fun_frame;
  ABCFrame *target = to->fun_frame, *next;
  memmove(state, to, sizeof(ABCState));
  return 1;
}
// to_stk as its a "naked" function
int64_t AiwnBCMakeContext(int64_t *to_stk) {
  ABCState *s = ABCStateNew((void *)to_stk[1], (void *)to_stk[2], 0, NULL);
  memcpy((void *)to_stk[0], s, sizeof(ABCState));
  free(s);
}
int64_t AiwnBC_FP(int64_t *) {
  return cur_bcstate->fp;
}

#if defined(USE_BYTECODE)
int64_t FFI_CALL_TOS_0_FEW_INSTS(void *fptr, int64_t iterations) {
  static int64_t active = 0;
  static char *stk = NULL;
  int64_t ret;
  static ABCState *state;
  if (!active) {
    active = 1;
    size_t stk_sz = 0x20000;
    stk = calloc(1, stk_sz);
    stk += stk_sz;
    state = ABCStateNew(fptr, stk, 0, NULL);
    setjmp(except_to);
    while (1) {
      AiwnRunBC(state);
      if (--iterations < 0 || !state->ip)
        break;
    }
  } else {
    fptr = (void *)state->ip;
    fprintf(stdout, "%s\n", WhichFun(fptr));
    setjmp(except_to);
    while (1) {
      AiwnRunBC(state);
      if (--iterations < 0)
        break;
    }
  }
  if (!state->ip) {
    free(state);
    state = NULL;
    active = 0;
    free(stk);
  }
  return active;
}
int64_t FFI_CALL_TOS_0(void *fptr) {
  size_t stk_sz = 0x20000;
  int64_t ret;
  char *stk = calloc(1, stk_sz);
  stk += stk_sz;
  ABCState *state = ABCStateNew(fptr, stk, 0, NULL);
  setjmp(except_to);
  while (!ABCRun_Done(state, &ret))
    ;

  stk -= stk_sz;
  free(stk);

  return ret;
}
int64_t FFI_CALL_TOS_1(void *fptr, int64_t a) {
  size_t stk_sz = 0x20000;
  int64_t ret;
  char *stk = calloc(1, stk_sz);
  stk += stk_sz;
  ABCState *state = ABCStateNew(fptr, stk, 1, &a);
  setjmp(except_to);
  while (!ABCRun_Done(state, &ret))
    ;

  stk -= stk_sz;
  free(stk);

  return ret;
}
int64_t FFI_CALL_TOS_2(void *fptr, int64_t a, int64_t b) {
  int64_t args[2] = {a, b};
  size_t stk_sz = 0x20000;
  int64_t ret;
  char *stk = calloc(1, stk_sz);
  stk += stk_sz;
  ABCState *state = ABCStateNew(fptr, stk, 2, &args);
  setjmp(except_to);
  while (!ABCRun_Done(state, &ret))
    ;

  stk -= stk_sz;
  free(stk);

  return ret;
}
int64_t FFI_CALL_TOS_3(void *fptr, int64_t a, int64_t b, int64_t c) {
  int64_t args[3] = {a, b, c};
  size_t stk_sz = 0x20000;
  int64_t ret;
  char *stk = calloc(1, stk_sz);
  stk += stk_sz;
  ABCState *state = ABCStateNew(fptr, stk, 3, &args);
  setjmp(except_to);
  while (!ABCRun_Done(state, &ret))
    ;

  stk -= stk_sz;
  free(stk);

  return ret;
}
int64_t FFI_CALL_TOS_4(void *fptr, int64_t a, int64_t b, int64_t c, int64_t d) {
  int64_t args[4] = {a, b, c, d};
  size_t stk_sz = 0x20000;
  int64_t ret;
  char *stk = calloc(1, stk_sz);
  stk += stk_sz;
  ABCState *state = ABCStateNew(fptr, stk, 4, &args);
  setjmp(except_to);
  while (!ABCRun_Done(state, &ret))
    ;

  stk -= stk_sz;
  free(stk);
  return ret;
}
// TODO
int64_t FFI_CALL_TOS_CUSTOM_BP(uint64_t bp, void *fp, uint64_t ip) {
  return FFI_CALL_TOS_0(bp); // whoops
}

int64_t AiwnBCTaskContextSetRIP(int64_t *stk) {
  ABCState *st = (ABCState *)stk[0];
  st->ip = stk[1];
  return 0;
}

int64_t AiwnBCTaskContextGetRBP(int64_t *stk) {
  ABCState *st = (ABCState *)stk[0];
  return st->fp;
}
int64_t AiwnBCTaskContextGetRIP(int64_t *stk) {
  ABCState *st = (ABCState *)stk[0];
  return st->ip;
}

void *AiwnBCDbgCurContext() {
  return &cur_dbgstate;
}
void AiwnBCDbgFault(int sig) {
  CHashExport *exp;
  ABCFrame *ff;
  int64_t *prolog, *args;
  exp = HashFind("AiwniosDbgCB", Fs->hash_table, HTT_EXPORT_SYS_SYM, 1);
  memcpy(&cur_dbgstate, cur_bcstate, sizeof(ABCState));
  if (!exp)
    exit(1);

  cur_bcstate->_sp -= 16 + sizeof(ABCFrame) - 16;
  prolog = (int64_t *)cur_bcstate->_sp;
  prolog[0] = cur_bcstate->fp;
  prolog[1] = cur_bcstate->ip;
  ff = (void *)&prolog[2];
  args = (void *)(ff + 1);
  args[0] = sig;
  args[1] = &cur_dbgstate;
  cur_bcstate->fun_frame = ff;
  ff->sp = 0;
  cur_bcstate->ip = exp->val;
  cur_bcstate->fp = (int64_t)prolog;
  longjmp(except_to, 1);
}
#endif

#ifdef dasdsaff__EMSCRIPTEN__
static char *ToCType() {
  switch (rt) {
  case RT_I8i:
    return "int8_t";
  case RT_U8i:
    return "uint8_t";
  case RT_I16i:
    return "int16_t";
  case RT_U16i:
    return "uint16_t";
  case RT_I32i:
    return "int32_t";
  case RT_U32i:
    return "uint32_t";
  case RT_I64i:
     return "int64_t";
  case RT_U64i:
     return "uint64_t";
  case RT_F64:
    return "double";
  case RT_PTR:
  case RT_FUNC:
    return "int64_t";
    break;
  }
	abort();
}
int64_t ToC(FILE *f,char *fun,int64_t rt,int32_t *start,int32_t *end,int64_t s) {
	int64_t i=0,i64;
	int stk=0;
	fprintf(f,"int64_t STK_%s(int64_t *args) {\n",fun);
	for(i=0;i!=IM_STK_SIZE;++i) {
	  fprintf(f," int64_t vari%d;\n",i);
      fprintf(f," uint64_t varu%d;\n",i);
      fprintf(f," double varf%d;\n",i);
    }
	fprintf(f,"int64_t (*fptr)(int64_t *);\n")
	while(start!=end) {
		int64_t t=*start>>8;
		switch(*start++) {
    break; case ABC_NOP:
  break; case ABC_INTERN:
  //TODO
  break; case ABC_GOTO:
        i64 = *(uint64_t *)start;
    start += 2;
    fprintf(f,"goto L%p;\n",(char*)(start-1)+i64);
  break; case ABC_GOTO_IF:
        i64 = *(uint64_t *)start;
    start += 2;
    if(t==RT_F64)
    fprintf(f,"if(var%d.f)goto L%p;\n",stk,(char*)(start-1)+i64);
    else
    fprintf(f,"if(var%d.i)goto L%p;\n",stk,(char*)(start-1)+i64);
    stk=0;
  break; case ABC_TO_I64:
  fprintf(f,"var%d.i=var%d.f;\n",stk,stk);
  break; case ABC_TO_F64:
  fprintf(f,"var%d.f=var%d.i;\n",stk,stk);
  break; case ABC_LABEL:
  fprintf(f,"L%p:\n",start-1);
  break; case ABC_STATIC:
  break; case ABC_READ_PTRO: // offset
        i64 = *(uint64_t *)start;
    start += 2;
    read_style:;
    if(t==RT_F64)
    fprintf(f,"var%d.f=*(%s*)(var%d.i+%lld);\n",++stk,ToCType(t),i64);
  else
  fprintf(f,"var%d.i=*(%s*)(var%d.i+%lld);\n",++stk,ToCType(t),i64);
  break; case ABC_READ_PTR:
  i64=0;
  goto read_style
  break; case ABC_NEG:
  if(t==RT_F64)
    fprintf(f,"stk.d[%d]=-stk.d[%d]",stk);
  else
    fprintf(f,"stk.d[%d]=-var%d.f",stk);

  break; case ABC_POS:
  //DO NOTHING
  break; case ABC_PCREL:
  i64 = *(int64_t *)start;
  start += 2;

  break; case ABC_IMM_I64:
  break; case ABC_IMM_F64:
  break; case ABC_POW:
  break; case ABC_ADD:
  break; case ABC_WRITE_PTR:
  break; case ABC_WRITE_PTRO: // offset
  break; case ABC_DIV:
  break; case ABC_MUL:
  break; case ABC_AND:
  break; case ABC_SUB:
  break; case ABC_FRAME_ADDR:
  break; case ABC_XOR:
  break; case ABC_MOD:
  break; case ABC_OR:
  break; case ABC_LT:
  break; case ABC_GT:
  break; case ABC_LNOT:
  break; case ABC_BNOT:
  break; case ABC_AND_AND:
  break; case ABC_XOR_XOR:
  break; case ABC_OR_OR:
  break; case ABC_NE:
  break; case ABC_EQ_EQ:
  break; case ABC_LE:
  break; case ABC_GE:
  break; case ABC_LSH:
  break; case ABC_RSH:
  break; case ABC_RET:
  break; case ABC_CALL:
  break; case ABC_COMMA:
  break; case ABC_SWITCH:
  break; case ABC_SUB_PROLOG:
  break; case ABC_SUB_RET:
  break; case ABC_SUB_CALL:
  break; case ABC_TYPECAST:
  break; case ABC_RELOC:
  break; case ABC_SET_FRAME_SIZE:
    fprintf(f,"\t char frame[%i];\n",*start++);
  break; case ABC_DUP:
  break; case ABC_END_EXPR:
  break; case ABC_DISCARD_STK: // next u32 is amount to discard
  break; case ABC_PUSH:
  break; case ABC_POP:
  break; case ABC_TO_BOOL:
  break; case ABC_SKIP_ON_PASS: // next u32 is amt of u32's to skip
  break; case ABC_SKIP_ON_FAIL: // next u32 is amt of u32's to skip
  break; case ABC_GS:
  break; case ABC_FS:
  break; case ABC_PRE_INC:  // next u64 is amount to add
  break; case ABC_POST_INC: // next u64 is amount to add
  break; case ABC_DISCARD:  // next u32 is amount to discard
  break; case ABC_CNT:      // MUST BE THE LAST ITEM
}
	}
	fprintf(f,"}\n");
}
#endif
