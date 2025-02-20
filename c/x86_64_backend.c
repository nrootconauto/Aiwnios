// X86_64 assembly generator
#include "aiwn_arm.h"
#include "aiwn_asm.h"
#include "aiwn_except.h"
#include "aiwn_hash.h"
#include "aiwn_lexparser.h"
#include "aiwn_mem.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern void DoNothing();
// backend_user_data5 is the "fail" codemisc for IC_LT/GT etc
// backend_user_data6 is the "pass" codemisc for IC_LT/GT etc
// backend_user_data7 is the number of Fregs
// backend_user_data9 is set if we were called from __SexyAssignOp
//
// I called CrunkLord420's stuff Voluptous - April 30,2023
// I found a duck statue while voluentering - May 3,2023
// I got bored and played with a color chooser May 7,2023
// I am going to get off my ass and document this file May 14,2023
//
// I am putting in the pooping computed goto's May 27,2023
//

// Mark Regioster
static int64_t MIR(CCmpCtrl *cc, int64_t r) {
  CRPN *rpn = cc->cur_rpn;
  if (!rpn)
    return r;
  rpn->changes_iregs |= 1u << r;
  return r;
}
static int64_t MFR(CCmpCtrl *cc, int64_t r) {
  CRPN *rpn = cc->cur_rpn;
  if (!rpn)
    return r;
  rpn->changes_fregs |= 1u << r;
  return r;
}
static int64_t IsCompoundCompare(CRPN *r);
static int64_t __SexyAssignOp(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                              int64_t code_off);
static int64_t IsSavedFReg(int64_t r) {
  if (r >= AIWNIOS_FREG_START)
    if (r - AIWNIOS_FREG_START < AIWNIOS_FREG_CNT)
      return 1;
  return 0;
}
static int64_t IsSavedIReg(int64_t r);
static int64_t ModeIsDerefToSIB(CRPN *m) {
  CRPN *next = m->base.next;
  if (m->type == IC_DEREF) {
    if (m->res.mode == __MD_X86_64_SIB || m->res.mode == MD_INDIR_REG)
      return 1;
    // Temporary registers may get destroyed
    if (next->res.mode == MD_REG && IsSavedIReg(next->res.reg))
      return 1;
  } else if (m->type == IC_BASE_PTR ||
             (m->res.mode == MD_FRAME &&
              m->res.keep_in_tmp ==
                  0 /* Dont allow to keep in tmp,we want the frame	*/
              ))
    return 1;
  return 0;
}

static int64_t RawTypeIs64(int64_t r) {
  switch (r) {
  case RT_U64i:
  case RT_I64i:
  case RT_F64:
  case RT_PTR:
    return 1;
  }
  return 0;
}
//
// Assembly section
//

extern int64_t ICMov(CCmpCtrl *cctrl, CICArg *dst, CICArg *src, char *bin,
                     int64_t code_off);
extern int64_t __OptPassFinal(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                              int64_t code_off);

#define X86_64_BASE_REG  5
#define X86_64_STACK_REG 4

#define AIWNIOS_ADD_CODE(func, ...)                                            \
  {                                                                            \
    if (bin)                                                                   \
      code_off += func(bin + code_off, __VA_ARGS__);                           \
    else                                                                       \
      code_off += func(NULL, __VA_ARGS__);                                     \
  }

static uint8_t ErectREX(int64_t big, int64_t reg, int64_t index, int64_t base) {
  if (index == -1)
    index = 0;
  if (base == -1)
    base = 0;
  return 0b01000000 | (big << 3) | (!!(reg & 0b1000) << 2) |
         (!!(index & 0b1000) << 1) | !!(base & 0b1000);
}
#define ADD_U8(v) to[len++] = (v);
#define ADD_U16(v)                                                             \
  {                                                                            \
    *(int16_t *)(to + len) = (v);                                              \
    len += 2;                                                                  \
  }
#define ADD_U32(v)                                                             \
  {                                                                            \
    *(int32_t *)(to + len) = (v);                                              \
    len += 4;                                                                  \
  }
#define ADD_U64(v)                                                             \
  {                                                                            \
    *(int64_t *)(to + len) = (v);                                              \
    len += 8;                                                                  \
  }

#define SIB_BEGIN(_64, r, s, i, b, o)                                          \
  do {                                                                         \
    int64_t R = (r), S = (s), I = (i), B = (b), O = (o);                       \
    switch (S) {                                                               \
    default:                                                                   \
      S = -1;                                                                  \
      break;                                                                   \
    case 1:                                                                    \
      S = 0;                                                                   \
      break;                                                                   \
    case 2:                                                                    \
      S = 1;                                                                   \
      break;                                                                   \
    case 4:                                                                    \
      S = 2;                                                                   \
      break;                                                                   \
    case 8:                                                                    \
      S = 3;                                                                   \
    }                                                                          \
    if (B == RIP) {                                                            \
      if (_64 || (I & 0b1000) || (B & 0b1000) || (R & 0b1000))                 \
        ADD_U8(ErectREX(_64, R, 0, 0));                                        \
    } else if (I != -1 || B == R12) {                                          \
      if (S == -1)                                                             \
        I = RSP;                                                               \
      if (_64 || (I & 0b1000) || (B & 0b1000) || (R & 0b1000))                 \
        ADD_U8(ErectREX(_64, R, I, B));                                        \
    } else {                                                                   \
      if (_64 || (I & 0b1000) || (B & 0b1000) || (R & 0b1000))                 \
        ADD_U8(ErectREX(_64, R, I, B));                                        \
    }
#define SIB_END()                                                              \
  if (I == -1 && B == -1) {                                                    \
    ADD_U8(((R & 0b111) << 3) | 0b100);                                        \
    \  
      ADD_U8(0x25);                                                            \
    ADD_U32(O);                                                                \
    break;                                                                     \
  }                                                                            \
  if (I == -1 && R12 != B && B != RIP && B != RSP) {                           \
    if (!O && B != RBP)                                                        \
      ADD_U8(((R & 0b111) << 3) | (B & 0b111))                                 \
    else if (-0x7f <= O && O <= 0x7f) {                                        \
      ADD_U8(0b01000000 | ((R & 0b111) << 3) | (B & 0b111));                   \
      ADD_U8(O);                                                               \
    } else {                                                                   \
      ADD_U8(0b10000000 | ((R & 0b111) << 3) | (B & 0b111));                   \
      ADD_U32(O);                                                              \
    }                                                                          \
    break;                                                                     \
  }                                                                            \
  if (B == RIP) {                                                              \
    ADD_U8(((R & 0b111) << 3) | 0b101)                                         \
    ADD_U32(O - (len + 4));                                                    \
    break;                                                                     \
  } else {                                                                     \
    if (B != -1) {                                                             \
      ADD_U8(MODRMSIB((R & 0b111)) |                                           \
             (((-0x7f <= O && O <= 0x7f) ? 0b01 : 0b10) << 6))                 \
    } else {                                                                   \
      ADD_U8(MODRMSIB(R & 0b111));                                             \
    }                                                                          \
  }                                                                            \
  if (S == -1) {                                                               \
    ADD_U8((RSP << 3) | (B & 0b111));                                          \
  } else if (B == -1) {                                                        \
    ADD_U8((S << 6) | ((I & 0b111) << 3) | RBP);                               \
  } else {                                                                     \
    ADD_U8((S << 6) | ((I & 0b111) << 3) | (B & 0b111));                       \
  }                                                                            \
  if ((-0x7f <= O && O <= 0x7f) && B != -1) {                                  \
    ADD_U8(O);                                                                 \
  } else {                                                                     \
    ADD_U32(O);                                                                \
  }                                                                            \
  }                                                                            \
  while (0)                                                                    \
    ;

static int64_t Is32Bit(int64_t num) {
  return num < 0x7fFFffFFll && num > -0x7fFFffFFll;
}
static uint8_t MODRMRegReg(int64_t a, int64_t b) {
  return (0b11 << 6) | ((a & 0b111) << 3) | (b & 0b111);
}
static uint8_t MODRMRip32(int64_t a) {
  return (0b00 << 6) | ((a & 0b111) << 4) | 0b101;
}

static uint8_t MODRMSIB(int64_t a) {
  return ((a & 0b111) << 3) | 0b100;
}

static int64_t X86CmpRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x3b);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86CmovaRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x47);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}
static int64_t X86CmovbRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x42);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86CmovlRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x4c);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}
static int64_t X86CmovgRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x4f);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86CmpSIB64Imm(char *to, int64_t imm, int64_t s, int64_t i,
                              int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 7, s, i, b, o);
  ADD_U8(0x81);
  SIB_END();
  ADD_U32(imm);
  return len;
}

static int64_t X86CmpSIB32Imm(char *to, int64_t imm, int64_t s, int64_t i,
                              int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 7, s, i, b, o);
  ADD_U8(0x81);
  SIB_END();
  ADD_U32(imm);
  return len;
}

static int64_t X86CmpSIB16Imm(char *to, int64_t imm, int64_t s, int64_t i,
                              int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, 7, s, i, b, o);
  ADD_U8(0x81);
  SIB_END();
  ADD_U16(imm);
  return len;
}

static int64_t X86CmpSIB8Imm(char *to, int64_t imm, int64_t s, int64_t i,
                             int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 7, s, i, b, o);
  ADD_U8(0x80);
  SIB_END();
  ADD_U8(imm);
  return len;
}

static int64_t X86CmpRegImm(char *to, int64_t a, int64_t imm) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0x81);
  ADD_U8(MODRMRegReg(7, a));
  ADD_U32(imm);
  return len;
}

int64_t X86MovRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, b, 0, a));
  ADD_U8(0x89);
  ADD_U8(MODRMRegReg(b, a));
  return len;
}

static int64_t X86CMovsRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x48);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

int64_t X86Leave(char *to, int64_t ul) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xc9);
  return len;
}

int64_t X86Ret(char *to, int64_t ul) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (!ul) {
    ADD_U8(0xc3);
  } else {
    ADD_U8(0xc2);
    ADD_U16(ul);
  }
  return len;
}

int64_t X86MovImm(char *to, int64_t a, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (((1ll << 32) - 1) >= off && off >= 0) {
    // Is unsigned 32bit
    ADD_U8(ErectREX(0, a, 0, a));
    ADD_U8(0xB8 + (a & 0x7));
    ADD_U32(off);
    return len;
  }
  if (((1ll << 32) - 1) / 2 >= off) {
    if (-((1ll << 32) - 1) / 2 <= off) {
      // Is Signed 32bit
      // REX.W C7 /0 id
      ADD_U8(ErectREX(1, 0, 0, a));
      ADD_U8(0xc7);
      ADD_U8(MODRMRegReg(0, a));
      ADD_U32(off);
      return len;
    }
  }
  // REX.W + B8+ tf io: movabs
  ADD_U8(ErectREX(1, a, 0, a));
  ADD_U8(0xB8 + (a & 0x7));
  ADD_U64(off);
  return len;
}

static int64_t X86AddIndir64Reg(char *to, int64_t r, int64_t s, int64_t i,
                                int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, r, s, i, b, o);
  ADD_U8(0x1);
  SIB_END();
  return len;
}
static int64_t X86AddIndir16Reg(char *to, int64_t r, int64_t s, int64_t i,
                                int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, r, s, i, b, o);
  ADD_U8(0x1);
  SIB_END();
  return len;
}

static int64_t X86AddIndir32Reg(char *to, int64_t r, int64_t s, int64_t i,
                                int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, r, s, i, b, o);
  ADD_U8(0x1);
  SIB_END();
  return len;
}

static int64_t X86MovF64RIP(char *to, int64_t a, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xF2);
  SIB_BEGIN(1, a, -1, -1, RIP, off);
  ADD_U8(0x0F);
  ADD_U8(0x10);
  SIB_END();
  return len;
}

int64_t X86AddImm32(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0x81);
  ADD_U8(MODRMRegReg(0, a));
  ADD_U32(b);
  return len;
}

static int64_t X86AddIndir64Imm32(char *to, int64_t imm, int64_t s, int64_t i,
                                  int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 0, s, i, b, o);
  ADD_U8(0x81);
  SIB_END();
  ADD_U32(imm);
  return len;
}
static int64_t X86AddIndir32Imm32(char *to, int64_t imm, int64_t s, int64_t i,
                                  int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 0, s, i, b, o);
  ADD_U8(0x81);
  SIB_END();
  ADD_U32(imm);
  return len;
}
static int64_t X86AddIndir16Imm32(char *to, int64_t imm, int64_t s, int64_t i,
                                  int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, 0, s, i, b, o);
  ADD_U8(0x81);
  SIB_END();
  ADD_U16(imm);
  return len;
}
static int64_t X86AddIndir8Imm32(char *to, int64_t imm, int64_t s, int64_t i,
                                 int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 0, s, i, b, o);
  ADD_U8(0x80);
  SIB_END();
  ADD_U8(imm);
  return len;
}
//
static int64_t X86SubIndir64Imm32(char *to, int64_t imm, int64_t s, int64_t i,
                                  int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 5, s, i, b, o);
  ADD_U8(0x81);
  SIB_END();
  ADD_U32(imm);
  return len;
}
static int64_t X86SubIndir32Imm32(char *to, int64_t imm, int64_t s, int64_t i,
                                  int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 5, s, i, b, o);
  ADD_U8(0x81);
  SIB_END();
  ADD_U32(imm);
  return len;
}
static int64_t X86SubIndir16Imm32(char *to, int64_t imm, int64_t s, int64_t i,
                                  int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, 5, s, i, b, o);
  ADD_U8(0x81);
  SIB_END();
  ADD_U16(imm);
  return len;
}
static int64_t X86SubIndir8Imm32(char *to, int64_t imm, int64_t s, int64_t i,
                                 int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 5, s, i, b, o);
  ADD_U8(0x80);
  SIB_END();
  ADD_U8(imm);
  return len;
}
//
static int64_t X86SubIndir64Reg(char *to, int64_t r, int64_t s, int64_t i,
                                int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, r, s, i, b, o);
  ADD_U8(0x29);
  SIB_END();
  return len;
}
static int64_t X86SubIndir16Reg(char *to, int64_t r, int64_t s, int64_t i,
                                int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, r, s, i, b, o);
  ADD_U8(0x29);
  SIB_END();
  return len;
}

static int64_t X86SubIndir32Reg(char *to, int64_t r, int64_t s, int64_t i,
                                int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, r, s, i, b, o);
  ADD_U8(0x29);
  SIB_END();
  return len;
}

int64_t X86SubImm32(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 5, 0, a));
  ADD_U8(0x81);
  ADD_U8(MODRMRegReg(5, a));
  ADD_U32(b);
  return len;
}

static int64_t X86SubReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x2b);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86AddReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x03);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86AddSIB64(char *to, int64_t a, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, a, s, i, b, off);
  ADD_U8(0x03);
  SIB_END();
  return len;
}

static int64_t X86SubSIB64(char *to, int64_t a, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, a, s, i, b, off);
  ADD_U8(0x2b);
  SIB_END();
  return len;
}

static int64_t X86IncReg(char *to, int64_t a) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xff);
  ADD_U8(MODRMRegReg(0, a));
  return len;
}

static int64_t X86IncSIB64(char *to, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 0, s, i, b, off);
  ADD_U8(0xff);
  SIB_END();
  return len;
}

static int64_t X86DecReg(char *to, int64_t a) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xff);
  ADD_U8(MODRMRegReg(1, a));
  return len;
}

static int64_t X86DecSIB64(char *to, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 1, s, i, b, off);
  ADD_U8(0xff);
  SIB_END();
  return len;
}

static int64_t X86CVTTSD2SIRegSIB64(char *to, int64_t a, int64_t s, int64_t i,
                                    int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  SIB_BEGIN(1, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x2c);
  SIB_END();
  return len;
}

static int64_t X86CVTTSD2SIRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x2c);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86CVTTSI2SDRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x2a);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86CVTTSI2SDRegSIB64(char *to, int64_t a, int64_t s, int64_t i,
                                    int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  SIB_BEGIN(1, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x2a);
  SIB_END();
  return len;
}

static int64_t X86AndPDReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x54);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86AndPDSIB64(char *to, int64_t a, int64_t s, int64_t i,
                             int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x54);
  SIB_END();
  return len;
}

static int64_t X86OrPDReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x56);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86OrPDSIB64(char *to, int64_t a, int64_t s, int64_t i,
                            int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x56);
  SIB_END();
  return len;
}

static int64_t X86XorPDReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x57);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86XorPDSIB64(char *to, int64_t a, int64_t s, int64_t i,
                             int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x57);
  SIB_END();
  return len;
}

static int64_t X86COMISDRegReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x2f);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86Test(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x85);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86AddSDReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x58);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86AddSDSIB64(char *to, int64_t a, int64_t s, int64_t i,
                             int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  SIB_BEGIN(0, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x58);
  SIB_END();
  return len;
}

static int64_t X86SubSDReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x5c);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86SubSDSIB64(char *to, int64_t a, int64_t s, int64_t i,
                             int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  SIB_BEGIN(0, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x5c);
  SIB_END();
  return len;
}

static int64_t X86RoundSD(char *to, int64_t a, int64_t b, int64_t mode) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x3a);
  ADD_U8(0x0b);
  ADD_U8(MODRMRegReg(a, b));
  ADD_U8(mode);
  return len;
}

static int64_t X86SqrtSIB(char *to, int64_t a, int64_t s, int64_t i, int64_t b,
                          int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  SIB_BEGIN(0, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x51);
  SIB_END();
  return len;
}
static int64_t X86SqrtReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x51);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86MulSDReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x59);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86MulSDSIB64(char *to, int64_t a, int64_t s, int64_t i,
                             int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  SIB_BEGIN(0, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x59);
  SIB_END();
  return len;
}

static int64_t X86DivSDReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0x5E);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86DivSDSIB64(char *to, int64_t a, int64_t s, int64_t i,
                             int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  SIB_BEGIN(0, a, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0x5e);
  SIB_END();
  return len;
}

static int64_t X86AndReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x23);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

int64_t X86AndImm(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0x81);
  ADD_U8(MODRMRegReg(4, a));
  ADD_U32(b);
  return len;
}

static int64_t X86AndSIB64(char *to, int64_t a, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, a, s, i, b, off);
  ADD_U8(0x23);
  SIB_END();
  return len;
}

static int64_t X86OrReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0b);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86OrSIB64(char *to, int64_t a, int64_t s, int64_t i, int64_t b,
                          int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, a, s, i, b, off);
  ADD_U8(0xb);
  SIB_END();
  return len;
}

static int64_t X86OrImm(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0x81);
  ADD_U8(MODRMRegReg(1, a));
  ADD_U32(b);
  return len;
}

static int64_t X86XorReg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x33);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86XorSIB64(char *to, int64_t a, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, a, s, i, b, off);
  ADD_U8(0x33);
  SIB_END();
  return len;
}

static int64_t X86XorImm(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0x81);
  ADD_U8(MODRMRegReg(6, a));
  ADD_U32(b);
  return len;
}

static int64_t X86Cqo(char *to, int64_t unused) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, 0));
  ADD_U8(0x99);
  return len;
}

static int64_t X86DivReg(char *to, int64_t a) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 6, 0, a));
  ADD_U8(0xf7);
  ADD_U8(MODRMRegReg(6, a));
  return len;
}

static int64_t X86DivSIB64(char *to, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 6, s, i, b, off);
  ADD_U8(0xf7);
  SIB_END();
  return len;
}

static int64_t X86IDivReg(char *to, int64_t a) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xf7);
  ADD_U8(MODRMRegReg(7, a));
  return len;
}

static int64_t X86IDivSIB64(char *to, int64_t s, int64_t i, int64_t b,
                            int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 7, s, i, b, off);
  ADD_U8(0xf7);
  SIB_END();
  return len;
}

static int64_t X86SarRCX(char *to, int64_t a) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xd3);
  ADD_U8(MODRMRegReg(7, a));
  return len;
}

static int64_t X86SarRCXSIB64(char *to, int64_t s, int64_t i, int64_t b,
                              int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 7, s, i, b, off);
  ADD_U8(0xd3);
  SIB_END();
  return len;
}

static int64_t X86SarImm(char *to, int64_t a, int64_t imm) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xc1);
  ADD_U8(MODRMRegReg(7, a));
  ADD_U8(imm);
  return len;
}

static int64_t X86SarImmSIB64(char *to, int64_t s, int64_t i, int64_t b,
                              int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 7, s, i, b, off);
  ADD_U8(0xc1);
  SIB_END();
  return len;
}

static int64_t X86ShrRCX(char *to, int64_t a) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xd3);
  ADD_U8(MODRMRegReg(5, a));
  return len;
}

static int64_t X86ShrImm(char *to, int64_t a, int64_t imm) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xc1);
  ADD_U8(MODRMRegReg(5, a));
  ADD_U8(imm);
  return len;
}

static int64_t X86ShlRCX(char *to, int64_t a) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xd3);
  ADD_U8(MODRMRegReg(4, a));
  return len;
}

static int64_t X86ShlImm(char *to, int64_t a, int64_t imm) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xc1);
  ADD_U8(MODRMRegReg(4, a));
  ADD_U8(imm);
  return len;
}

static int64_t X86MulReg(char *to, int64_t a) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xf7);
  ADD_U8(MODRMRegReg(4, a));
  return len;
}

static int64_t X86MulSIB64(char *to, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 4, s, i, b, off);
  ADD_U8(0xf7);
  SIB_END();
  return len;
}

static int64_t X86IMulReg(char *to, int64_t a) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 0, 0, a));
  ADD_U8(0xf7);
  ADD_U8(MODRMRegReg(5, a));
  return len;
}
static int64_t X86IMulSIB64(char *to, int64_t s, int64_t i, int64_t b,
                            int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 5, s, i, b, off);
  ADD_U8(0xf7);
  SIB_END();
  return len;
}

static int64_t X86IMul2Reg(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0xaf);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

static int64_t X86IMul2SIB64(char *to, int64_t r, int64_t s, int64_t i,
                             int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, r, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0xaf);
  SIB_END();
  return len;
}

int64_t X86Call32(char *to, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xe8);
  ADD_U32(off - 5);
  return len;
}

int64_t X86CallReg(char *to, int64_t reg) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 2, 0, reg));
  ADD_U8(0xff);
  ADD_U8(MODRMRegReg(2, reg));
  return len;
}

int64_t X86CallSIB64(char *to, int64_t s, int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 2, s, i, b, o);
  ADD_U8(0xff);
  SIB_END();
  return len;
}

int64_t X86PushReg(char *to, int64_t reg) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (reg >= R8)
    ADD_U8(ErectREX(0, 0, 0, reg));
  ADD_U8(0x50 + (reg & 0x7));
  return len;
}

int64_t X86PushM16(char *to, int64_t s, int64_t i, int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 6, s, i, b, off);
  ADD_U8(0x66);
  ADD_U8(0xff);
  SIB_END();
  return len;
}
int64_t X86PushM32(char *to, int64_t s, int64_t i, int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 6, s, i, b, off);
  ADD_U8(0xff);
  SIB_END();
  return len;
}
int64_t X86PushM64(char *to, int64_t s, int64_t i, int64_t b, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 6, s, i, b, off);
  ADD_U8(0xff);
  SIB_END();
  return len;
}
int64_t X86PushImm(char *to, int64_t imm) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x68);
  ADD_U32(imm);
  return len;
}

int64_t X86PopReg(char *to, int64_t reg) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (reg >= R8)
    ADD_U8(ErectREX(0, 0, 0, reg));
  ADD_U8(0x58 + (reg & 0x7));
  return len;
}

static int64_t X86CmpRegSIB64(char *to, int64_t a, int64_t s, int64_t i,
                              int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, a, s, i, b, o);
  ADD_U8(0x3b);
  SIB_END();
  return len;
}

static int64_t X86CmpSIBReg64(char *to, int64_t a, int64_t s, int64_t i,
                              int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, a, s, i, b, o);
  ADD_U8(0x39);
  SIB_END();
  return len;
}

static int64_t X86MovQF64I64(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  ADD_U8(ErectREX(1, a, 0, b));
  ADD_U8(0x0F)
  ADD_U8(0x6e);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

int64_t X86LeaSIB(char *to, int64_t a, int64_t s, int64_t i, int64_t b,
                  int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, a, s, i, b, off);
  ADD_U8(0x8D);
  SIB_END();
  return len;
}

static int64_t X86MovQI64F64(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  ADD_U8(ErectREX(1, b, 0, a));
  ADD_U8(0x0F)
  ADD_U8(0x7e);
  ADD_U8(MODRMRegReg(b, a));
  return len;
}

static int64_t X86Jmp(char *to, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xe9);
  ADD_U32(off - 5);
  return len;
}

static int64_t X86JmpSIB(char *to, int64_t scale, int64_t idx, int64_t base,
                         int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 4, scale, idx, base, off);
  ADD_U8(0xff);
  SIB_END();
  return len;
}

int64_t X86JmpReg(char *to, int64_t r) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (r >= R8)
    ADD_U8(ErectREX(0, 4, 0, r));
  ADD_U8(0xff);
  ADD_U8(MODRMRegReg(4, r));
  return len;
}

static int64_t X86NegReg(char *to, int64_t r) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 3, 0, r));
  ADD_U8(0xf7);
  ADD_U8(MODRMRegReg(3, r));
  return len;
}

static int64_t X86NegSIB64(char *to, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 3, s, i, b, off);
  ADD_U8(0xf7);
  SIB_END();
  return len;
}

static int64_t X86NotReg(char *to, int64_t r) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 2, 0, r));
  ADD_U8(0xf7);
  ADD_U8(MODRMRegReg(2, r));
  return len;
}

static int64_t X86NotSIB64(char *to, int64_t s, int64_t i, int64_t b,
                           int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 2, s, i, b, off);
  ADD_U8(0xf7);
  SIB_END();
  return len;
}

// MOVSD
static int64_t X86MovRegRegF64(char *to, int64_t a, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  if ((a & 0b1000) || (b & 0b1000))
    ADD_U8(ErectREX(0, a, 0, b));
  ADD_U8(0x0F)
  ADD_U8(0x10);
  ADD_U8(MODRMRegReg(a, b));
  return len;
}

int64_t X86MovIndirRegF64(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x0F);
  ADD_U8(0x11);
  SIB_END();
  return len;
}
int64_t X86MovRegIndirF64(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf2);
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x0F);
  ADD_U8(0x10);
  SIB_END();
  return len;
}

int64_t X86MovRegIndirI64(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x8b);
  SIB_END();
  return len;
}

int64_t X86MovIndirI8Imm(char *to, int64_t imm, int64_t scale, int64_t index,
                         int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 0, scale, index, base, off);
  ADD_U8(0xc6);
  SIB_END();
  ADD_U8(imm);
  return len;
}

int64_t X86MovIndirI16Imm(char *to, int64_t imm, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, 0, scale, index, base, off);
  ADD_U8(0xc7);
  SIB_END();
  ADD_U16(imm);
  return len;
}

int64_t X86MovIndirI32Imm(char *to, int64_t imm, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, 0, scale, index, base, off);
  ADD_U8(0xc7);
  SIB_END();
  ADD_U32(imm);
  return len;
}

int64_t X86MovIndirI64Imm(char *to, int64_t imm, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 0, scale, index, base, off);
  ADD_U8(0xc7);
  SIB_END();
  ADD_U32(imm);
  return len;
}

int64_t X86MovIndirRegI64(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x89);
  SIB_END();
  return len;
}

int64_t X86XChgIndirReg32(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, reg, scale, index, base, off);
  ADD_U8(0x87);
  SIB_END();
  return len;
}

int64_t X86XChgIndirReg8(char *to, int64_t reg, int64_t scale, int64_t index,
                         int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, reg, scale, index, base, off);
  ADD_U8(0x86);
  SIB_END();
  return len;
}

int64_t X86XChgIndirReg16(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, reg, scale, index, base, off);
  ADD_U8(0x87);
  SIB_END();
  return len;
}

int64_t X86XChgIndirReg64(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x87);
  SIB_END();
  return len;
}

int64_t X86MovRegIndirI32(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, reg, scale, index, base, off);
  ADD_U8(0x8b);
  SIB_END();
  return len;
}

int64_t X86MovIndirRegI32(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, reg, scale, index, base, off);
  ADD_U8(0x89);
  SIB_END();
  return len;
}

int64_t X86MovRegIndirI16(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, reg, scale, index, base, off);
  ADD_U8(0x8b);
  SIB_END();
  return len;
}

static int64_t X86MovZXRegIndirI8(char *to, int64_t reg, int64_t scale,
                                  int64_t index, int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x0f);
  ADD_U8(0xb6);
  SIB_END();
  return len;
}

static int64_t X86MovZXRegRegI8(char *to, int64_t reg, int64_t reg2) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, reg, 0, reg2))
  ADD_U8(0x0f);
  ADD_U8(0xb6);
  ADD_U8(MODRMRegReg(reg, reg2));
  return len;
}

static int64_t X86MovZXRegIndirI16(char *to, int64_t reg, int64_t scale,
                                   int64_t index, int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x0f);
  ADD_U8(0xb7);
  SIB_END();
  return len;
}

static int64_t X86MovSXRegIndirI8(char *to, int64_t reg, int64_t scale,
                                  int64_t index, int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x0f);
  ADD_U8(0xbe);
  SIB_END();
  return len;
}

static int64_t X86MovSXRegIndirI16(char *to, int64_t reg, int64_t scale,
                                   int64_t index, int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x0f);
  ADD_U8(0xbf);
  SIB_END();
  return len;
}

static int64_t X86MovSXRegIndirI32(char *to, int64_t reg, int64_t scale,
                                   int64_t index, int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, reg, scale, index, base, off);
  ADD_U8(0x63);
  SIB_END();
  return len;
}

int64_t X86MovIndirRegI16(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x66);
  SIB_BEGIN(0, reg, scale, index, base, off);
  ADD_U8(0x89);
  SIB_END();
  return len;
}

int64_t X86MovRegIndirI8(char *to, int64_t reg, int64_t scale, int64_t index,
                         int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, reg, scale, index, base, off);
  ADD_U8(0x88);
  SIB_END();
  return len;
}

int64_t X86MovIndirRegI8(char *to, int64_t reg, int64_t scale, int64_t index,
                         int64_t base, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(0, reg, scale, index, base, off);
  ADD_U8(0x88);
  SIB_END();
  return len;
}

int64_t X86BtRegReg(char *to, int64_t r, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, b, 0, r));
  ADD_U8(0x0f);
  ADD_U8(0xa3);
  ADD_U8(MODRMRegReg(b, r))
  return len;
}

int64_t X86BtSIBReg(char *to, int64_t r, int64_t s, int64_t i, int64_t b,
                    int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, r, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0xa3);
  SIB_END();
  return len;
}

int64_t X86BtRegImm(char *to, int64_t r, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 4, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0xba);
  ADD_U8(MODRMRegReg(4, b))
  ADD_U8(b);
  return len;
}

int64_t X86BtSIBImm(char *to, int64_t r, int64_t s, int64_t i, int64_t b,
                    int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 4, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0xba);
  SIB_END();
  ADD_U8(r);
  return len;
}

int64_t X86BtcRegReg(char *to, int64_t r, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, b, 0, r));
  ADD_U8(0x0f);
  ADD_U8(0xbb);
  ADD_U8(MODRMRegReg(b, r))
  return len;
}

int64_t X86BtcSIBReg(char *to, int64_t r, int64_t s, int64_t i, int64_t b,
                     int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, r, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0xbb);
  SIB_END();
  return len;
}

static int64_t X86Lock(char *to, int64_t ul) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0xf0);
  return len;
}
int64_t X86BtcRegImm(char *to, int64_t r, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, r, 0, 7));
  ADD_U8(0x0f);
  ADD_U8(0xba);
  ADD_U8(MODRMRegReg(7, r))
  ADD_U8(b);
  return len;
}

int64_t X86BtcSIBImm(char *to, int64_t r, int64_t s, int64_t i, int64_t b,
                     int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 7, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0xba);
  SIB_END();
  ADD_U8(r);
  return len;
}

int64_t X86BtsRegReg(char *to, int64_t r, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, b, 0, r));
  ADD_U8(0x0f);
  ADD_U8(0xab);
  ADD_U8(MODRMRegReg(b, r))
  return len;
}

int64_t X86BtsSIBReg(char *to, int64_t r, int64_t s, int64_t i, int64_t b,
                     int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, r, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0xab);
  SIB_END();
  return len;
}

int64_t X86BtsRegImm(char *to, int64_t r, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 5, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0xba);
  ADD_U8(MODRMRegReg(5, b))
  ADD_U8(b);
  return len;
}

int64_t X86BtsSIBImm(char *to, int64_t r, int64_t s, int64_t i, int64_t b,
                     int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 5, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0xba);
  SIB_END();
  ADD_U8(r);
  return len;
}

int64_t X86BtrRegReg(char *to, int64_t r, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, b, 0, r));
  ADD_U8(0x0f);
  ADD_U8(0xb3);
  ADD_U8(MODRMRegReg(b, r))
  return len;
}

int64_t X86BtrSIBReg(char *to, int64_t r, int64_t s, int64_t i, int64_t b,
                     int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, r, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0xb3);
  SIB_END();
  return len;
}

int64_t X86BtrRegImm(char *to, int64_t r, int64_t b) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(ErectREX(1, 6, 0, b));
  ADD_U8(0x0f);
  ADD_U8(0xba);
  ADD_U8(MODRMRegReg(6, b))
  ADD_U8(b);
  return len;
}

int64_t X86BtrSIBImm(char *to, int64_t r, int64_t s, int64_t i, int64_t b,
                     int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  SIB_BEGIN(1, 6, s, i, b, off);
  ADD_U8(0x0f);
  ADD_U8(0xba);
  SIB_END();
  ADD_U8(r);
  return len;
}

//
// Codegen section
//

static int64_t IsTerminalInst(CRPN *r) {
  switch (r->type) {
    break;
  case IC_RET:
  case IC_GOTO:
    return 1;
  }
  return 0;
}

static int64_t IsConst(CRPN *rpn) {
  switch (rpn->type) {
  case IC_CHR:
  case IC_F64:
  case IC_I64:
    return 1;
  }
  return 0;
}

static int64_t ConstVal(CRPN *rpn) {
  switch (rpn->type) {
  case IC_CHR:
  case IC_I64:
    return rpn->integer;
  case IC_F64:
    return rpn->flt;
  }
  return 0;
}
//
// If we call a function the tmp regs go to poo poo land
//
static int64_t SpillsTmpRegs(CRPN *rpn) {
  int64_t idx;
  if (rpn->flags & ICF_SPILLS_TMP_REGS)
    return 1;
  if (rpn->flags & ICF_DOESNT_SPILL_TMP_REGS)
    return 0;
  switch (rpn->type) {
  case IC_FS:
  case IC_GS:
    goto fail;
  case __IC_CALL:
  case IC_CALL:
    goto pass;
    break;
  case __IC_VARGS:
    for (idx = 0; idx != rpn->length; idx++)
      if (SpillsTmpRegs(ICArgN(rpn, idx)))
        goto pass;
    goto fail;
  case IC_TO_F64:
  case IC_TO_I64:
  case IC_NEG:
  case IC_SQRT:
  case IC_TO_BOOL:
  unop:
    if (SpillsTmpRegs(rpn->base.next))
      goto pass;
    else
      goto fail;
    break;
  case IC_POS:
    goto unop;
    break;
  case IC_POW:
    goto pass; // calls pow
  binop:
    if (SpillsTmpRegs(rpn->base.next))
      goto pass;
    if (SpillsTmpRegs(ICFwd(rpn->base.next)))
      goto pass;
    break;
  case IC_ADD:
    goto binop;
    break;
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
  case IC_MOD_EQ:
    goto binop;
    break;
  case IC_BT:
  case IC_BTS:
  case IC_BTR:
  case IC_BTC:
  case IC_LBTS:
  case IC_LBTR:
  case IC_LBTC:
  case IC_STORE:
    goto binop;
  case IC_TYPECAST:
    goto unop;
  }
fail:;
  rpn->flags |= ICF_DOESNT_SPILL_TMP_REGS;
  return 0;
pass:
t:
  rpn->flags |= ICF_SPILLS_TMP_REGS;
  return 1;
}

//
// Will overwrite the arg's value,but it's ok as we are about to use it's
// value(and discard it)
//
static int64_t PutICArgIntoReg(CCmpCtrl *cctrl, CICArg *arg, int64_t raw_type,
                               int64_t fallback, char *bin, int64_t code_off) {
  CICArg tmp = {0};
  if (arg->mode == MD_REG) {
    if ((raw_type == RT_F64) ^ (arg->raw_type == RT_F64)) {
      // They differ so continue as usaul
    } else { // They are the same
      return code_off;
    }
  }
  tmp.raw_type = raw_type;
  tmp.reg = MIR(cctrl, fallback);
  tmp.mode = MD_REG;
  code_off = ICMov(cctrl, &tmp, arg, bin, code_off);
  memcpy(arg, &tmp, sizeof(CICArg));
  return code_off;
}

static int64_t PushToStack(CCmpCtrl *cctrl, CICArg *arg, char *bin,
                           int64_t code_off) {
  CICArg tmp = *arg;
  if (tmp.mode == MD_I64 && tmp.raw_type != RT_F64 && Is32Bit(tmp.integer)) {
    AIWNIOS_ADD_CODE(X86PushImm, tmp.integer);
    return code_off;
  }
  switch (tmp.mode) {
  case __MD_X86_64_SIB:
    switch (tmp.raw_type) {
    case RT_I64i:
    case RT_U64i:
    case RT_F64:
      AIWNIOS_ADD_CODE(X86PushM64, tmp.__SIB_scale, tmp.reg2, tmp.reg, tmp.off);
      break;
    default:
      goto defacto;
    }
    return code_off;
  case MD_FRAME:
    if (RawTypeIs64(tmp.raw_type)) {
      AIWNIOS_ADD_CODE(X86PushM64, -1, -1, RBP, -tmp.off);
    } else
      goto defacto;
    return code_off;
  case MD_INDIR_REG:
    switch (tmp.raw_type) {
    case RT_I64i:
    case RT_U64i:
    case RT_F64:
      AIWNIOS_ADD_CODE(X86PushM64, -1, -1, tmp.reg, tmp.off);
      break;
    default:
      goto defacto;
    }
    return code_off;
  }
defacto:
  code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 0, bin, code_off);
  if (tmp.raw_type != RT_F64) {
    AIWNIOS_ADD_CODE(X86PushReg, tmp.reg);
  } else {
    AIWNIOS_ADD_CODE(X86PushReg, RAX);
    AIWNIOS_ADD_CODE(X86MovIndirRegF64, tmp.reg, -1, -1, RSP, 0);
  }
  return code_off;
}
static int64_t PopFromStack(CCmpCtrl *cctrl, CICArg *arg, char *bin,
                            int64_t code_off) {
  CICArg tmp = *arg;
  if (arg->mode == MD_REG) {
    // All is good
  } else {
    tmp.mode = MD_REG;
    tmp.reg = 0;
  }
  if (tmp.raw_type != RT_F64) {
    AIWNIOS_ADD_CODE(X86PopReg, tmp.reg);
  } else {
    AIWNIOS_ADD_CODE(X86MovRegIndirF64, tmp.reg, -1, -1, RSP, 0);
    AIWNIOS_ADD_CODE(X86PopReg, RAX);
  }
  if (arg->mode != MD_REG)
    code_off = ICMov(cctrl, arg, &tmp, bin, code_off);
  return code_off;
}

static int64_t __ICMoveI64(CCmpCtrl *cctrl, int64_t reg, uint64_t imm,
                           char *bin, int64_t code_off) {
  int64_t code;
  AIWNIOS_ADD_CODE(X86MovImm, MIR(cctrl, reg), imm);
  return code_off;
}

static CCodeMisc *GetF64LiteralMisc(CCmpCtrl *cctrl, double imm) {
  CCodeMisc *misc = cctrl->code_ctrl->code_misc->next;
  for (misc; misc != cctrl->code_ctrl->code_misc; misc = misc->base.next) {
    if (misc->type == CMT_FLOAT_CONST)
      if (misc->integer == *(int64_t *)&imm) {
        return misc;
      }
  }
  misc = A_CALLOC(sizeof(CCodeMisc), cctrl->hc);
  misc->type = CMT_FLOAT_CONST;
  misc->integer = *(int64_t *)&imm;
  QueIns(misc, cctrl->code_ctrl->code_misc->last);
  return misc;
}
static int64_t __ICMoveF64(CCmpCtrl *cctrl, int64_t reg, double imm, char *bin,
                           int64_t code_off) {
  CCodeMisc *misc = cctrl->code_ctrl->code_misc->next;
  char *dptr;
  for (misc; misc != cctrl->code_ctrl->code_misc; misc = misc->base.next) {
    if (misc->type == CMT_FLOAT_CONST)
      if (misc->integer == *(int64_t *)&imm) {
        goto found;
      }
  }
  misc = A_CALLOC(sizeof(CCodeMisc), cctrl->hc);
  misc->type = CMT_FLOAT_CONST;
  misc->integer = *(int64_t *)&imm;
  QueIns(misc, cctrl->code_ctrl->code_misc->last);
found:
  if (bin && misc->addr && cctrl->code_ctrl->final_pass) {
    AIWNIOS_ADD_CODE(X86MovF64RIP, MFR(cctrl, reg), 0);
    CodeMiscAddRef(misc, bin + code_off - 4);
  } else
    AIWNIOS_ADD_CODE(X86MovF64RIP, MFR(cctrl, reg), 0);
  return code_off;
}

static void PushSpilledTmp(CCmpCtrl *cctrl, CRPN *rpn) {
  int64_t raw_type = rpn->raw_type;
  CICArg *res = &rpn->res;
  if (rpn->flags & ICF_PRECOMPUTED)
    return;
  // These have no need to be put into a temporay
  switch (rpn->type) {
    break;
  case IC_I64:
    res->mode = MD_I64;
    res->integer = rpn->integer;
    res->raw_type = RT_I64i;
    return;
    break;
  case IC_F64:
    res->mode = MD_F64;
    res->flt = rpn->flt;
    res->raw_type = RT_F64;
    return;
    break;
  case IC_IREG:
  case IC_FREG:
    res->mode = MD_REG;
    res->raw_type = raw_type;
    res->reg = rpn->integer;
    return;
    break;
  case IC_BASE_PTR:
    res->mode = MD_FRAME;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
  case IC_STATIC:
    res->mode = MD_PTR;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
  case __IC_STATIC_REF:
    res->mode = MD_STATIC;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
  }
  rpn->flags |= ICF_SPILLED;
  res->is_tmp = 1;
  if (raw_type != RT_F64) {
    res->mode = MD_FRAME;
    res->raw_type = raw_type;
    if (cctrl->backend_user_data0 < (res->off = cctrl->backend_user_data1 +=
                                     8)) { // Will set ref->off before the top
      cctrl->backend_user_data0 = res->off;
    }
  } else {
    res->mode = MD_FRAME;
    res->raw_type = raw_type;
    if (cctrl->backend_user_data0 < (res->off = cctrl->backend_user_data1 +=
                                     8)) { // Will set ref->off before the top
      cctrl->backend_user_data0 = res->off;
    }
  }
  // See two above notes
  res->off -= 8;
  // In functions,wiggle room comes after the function locals.
  if (cctrl->cur_fun && res->mode == MD_FRAME)
    res->off += cctrl->cur_fun->base.sz + 16; //+16 for LR/FP
  else if (res->mode == MD_FRAME) {
    if (cctrl->backend_user_data4) {
      res->off += cctrl->backend_user_data4; // wiggle room start(if set)
    } else
      // Anonymus potatoes like to assume LR/FP pair too.
      res->off += 16;
  }
}
int64_t TmpRegToReg(int64_t r) {
  switch (r) {
    break;
  case 0:
    return R9;
  case 1:
    return RBX;
  default:
    abort();
  }
}

static void PushTmp(CCmpCtrl *cctrl, CRPN *rpn) {
  int64_t raw_type = rpn->raw_type;
  CICArg *res = &rpn->res;
  if (rpn->flags & ICF_PRECOMPUTED)
    return;
  // These have no need to be put into a temporay
  switch (rpn->type) {
    break;
  case __IC_STATIC_REF:
    res->mode = MD_STATIC;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
  case IC_STATIC:
    res->mode = MD_PTR;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
    // These dudes will compile down to a MD_I64/MD_F64
  case IC_I64:
    res->mode = MD_I64;
    res->integer = rpn->integer;
    res->raw_type = RT_I64i;
    return;
    break;
  case IC_F64:
    res->mode = MD_F64;
    res->flt = rpn->flt;
    res->raw_type = RT_F64;
    return;
  case IC_IREG:
  case IC_FREG:
    res->mode = MD_REG;
    res->raw_type = raw_type;
    res->reg = rpn->integer;
    return;
    break;
  case IC_BASE_PTR:
    res->mode = MD_FRAME;
    res->raw_type = raw_type;
    res->off = rpn->integer;
    return;
    return;
  }
use_defacto:
  if (raw_type != RT_F64) {
    if (cctrl->backend_user_data2 < AIWNIOS_TMP_IREG_CNT) {
      res->mode = MD_REG;
      res->raw_type = raw_type;
      res->off = 0;
      res->reg = TmpRegToReg(cctrl->backend_user_data2++);
    } else {
      PushSpilledTmp(cctrl, rpn);
    }
  } else {
    if (cctrl->backend_user_data3 < AIWNIOS_TMP_FREG_CNT) {
      res->mode = MD_REG;
      res->raw_type = raw_type;
      res->off = 0;
      res->reg = AIWNIOS_TMP_FREG_START + cctrl->backend_user_data3++;
    } else {
      PushSpilledTmp(cctrl, rpn);
    }
  }
}

static void PopTmp(CCmpCtrl *cctrl, CRPN *rpn) {
  int64_t raw_type = rpn->raw_type;
  // We didnt modift the res with PushTmp,so dont modify the results here
  if (rpn->flags & ICF_PRECOMPUTED)
    return;
  if (rpn->flags & ICF_TMP_NO_UNDO)
    return;
  // These have no need to be put into a temporay
  switch (rpn->type) {
    break;
  case IC_I64:
  case IC_F64:
  case IC_IREG:
  case IC_FREG:
  case IC_BASE_PTR:
  case IC_STATIC:
  case __IC_STATIC_REF:
    return;
  }
  if (rpn->flags & ICF_SPILLED) {
    cctrl->backend_user_data1 -= 8;
  } else {
    if (raw_type != RT_F64) {
    indir:
      if (rpn->flags & (ICF_SIB | ICF_INDIR_REG)) {
        cctrl->backend_user_data2 -= rpn->res.pop_n_tmp_regs;
        assert(cctrl->backend_user_data2 >= 0);
      } else {
        --cctrl->backend_user_data2;
      }
    } else {
      if (rpn->flags & (ICF_SIB | ICF_INDIR_REG))
        goto indir;
      --cctrl->backend_user_data3;
    }
  }
}

static CRPN *__AddScale(CRPN *r, int64_t *const_val) {
  CRPN *n0 = ICArgN(r, 0), *n1 = ICArgN(r, 1);
  if (r->type == IC_LSH) {
    if (IsConst(n0) && Is32Bit(ConstVal(n0))) {
      switch (ConstVal(n0)) {
      case 1:
      case 2:
      case 3:
        if (const_val)
          *const_val = 1 << ConstVal(n0);
        return n1;
      }
    }
    return NULL;
  }
  if (r->type != IC_MUL)
    return NULL;
  if (IsConst(n0) && Is32Bit(ConstVal(n0))) {
    switch (ConstVal(n0)) {
    case 1:
    case 2:
    case 4:
    case 8:
      if (const_val)
        *const_val = ConstVal(n0);
      return n1;
    }
  }
  if (IsConst(n1) && Is32Bit(ConstVal(n1))) {
    switch (ConstVal(n1)) {
    case 1:
    case 2:
    case 4:
    case 8:
      if (const_val)
        *const_val = ConstVal(n1);
      return n0;
    }
  }
  if (const_val)
    *const_val = 1;
  return NULL;
}

// Returns the non-constant side of a +Const

static CRPN *__AddSIBOffset(CRPN *r, int64_t *const_val) {
  if (r->type != IC_ADD && r->type != IC_SUB)
    return NULL;
  int64_t mul = (r->type == IC_ADD) ? 1 : -1;
  CRPN *n0 = ICArgN(r, 0), *n1 = ICArgN(r, 1);
  if (n0->type != IC_F64 && IsConst(n0) && Is32Bit(ConstVal(n0))) {
    if (const_val)
      *const_val = mul * ConstVal(n0);
    return n1;
  }
  // Dont do 256-idx(ONLY DO idx-256)
  if (n1->type != IC_F64 && IsConst(n1) && Is32Bit(ConstVal(n1)) &&
      r->type != IC_SUB) {
    if (const_val)
      *const_val = mul * ConstVal(n1);
    return n0;
  }
  return NULL;
}
static int64_t IsCompare(int64_t c) {
  switch (c) {
  case IC_GT:
  case IC_LT:
  case IC_GE:
  case IC_LE:
    return 1;
  }
  return 0;
}

static int64_t GetSIBParts(CRPN *r, int64_t *off, CRPN **_b, CRPN **_idx,
                           int64_t *scale, int64_t allow_put_in_reg) {
  CRPN *b = NULL, *idx = NULL;
  int64_t tmp, i = 0, i2 = -1, is_sib = 0;
  CRPN *arg = r;
  while (__AddSIBOffset(arg, &tmp)) {
    arg = __AddSIBOffset(arg, &tmp);
    i += tmp;
  }
  if (arg->type == IC_ADD) {
    if (idx = __AddScale(ICArgN(arg, 0), &i2))
      b = ICArgN(arg, 1);
    else if (idx = __AddScale(ICArgN(arg, 1), &i2))
      b = ICArgN(arg, 0);
    else {
      b = ICArgN(arg, 0), idx = ICArgN(arg, 1), i2 = 1;
    }
    is_sib = 1;
  } else if (arg->type == IC_IREG) {
    b = arg;
    is_sib = 1;
  } else if (arg->type == IC_MUL) {
    b = NULL;
    if (idx = __AddScale(ICArgN(arg, 0), &i2))
      is_sib = 1;
    else if (idx = __AddScale(ICArgN(arg, 1), &i2))
      is_sib = 1;
  } else if (allow_put_in_reg) {
    b = arg;
    is_sib = 1;
  }
  // Adjust for offesets in base/index
  if (b)
    while (__AddSIBOffset(b, &tmp)) {
      b = __AddSIBOffset(b, &tmp);
      i += tmp;
    }
  if (idx)
    while (__AddSIBOffset(idx, &tmp)) {
      idx = __AddSIBOffset(idx, &tmp);
      i += tmp * i2;
    }
  if (_b)
    *_b = b;
  if (_idx)
    *_idx = idx;
  if (off)
    *off = i;
  if (scale)
    *scale = i2;
  if (b)
    if (b->res.raw_type == RT_F64)
      is_sib = 0;
  if (idx)
    if (idx->res.raw_type == RT_F64)
      is_sib = 0;
  return is_sib && Is32Bit(i);
}

static int64_t IsGoodIReg(r) {
  return IsSavedIReg(r) || r == R9 || r == RBX;
}

// Returns 1 if we hit the bottom
static int64_t PushTmpDepthFirst(CCmpCtrl *cctrl, CRPN *r, int64_t spilled) {
  switch (r->type) {
  case __IC_STATIC_REF:
  case IC_STATIC:
  case IC_GLOBAL:
  case IC_I64:
  case IC_F64:
  case IC_IREG:
  case IC_FREG:
  case IC_BASE_PTR:
  case IC_STR:
  case IC_CHR:
  case IC_FS:
  case IC_GS:
    if (!spilled)
      PushTmp(cctrl, r);
    else
      PushSpilledTmp(cctrl, r);
    return 1;
  }
  int64_t a, argc, old_icnt = cctrl->backend_user_data2,
                   old_fcnt = cctrl->backend_user_data3, i, i2, tmp,
                   old_scnt = cctrl->backend_user_data1;
  CRPN *arg, *arg2, *d, *b, *idx, *orig, **array, *new;
  switch (r->type) {
    break;
  case IC_MAX_I64:
  case IC_MAX_U64:
  case IC_MIN_I64:
  case IC_MIN_U64:
    goto binop;
  case IC_SHORT_ADDR:
    goto fin;
  case IC_CALL:
  case __IC_CALL:
    for (i = 0; i < r->length + 1; i++) {
      PushTmpDepthFirst(cctrl, ICArgN(r, i), 0);
      PopTmp(cctrl, ICArgN(r, i));
    }
    goto fin;
    break;
  case IC_TO_I64:
  case IC_TO_BOOL:
  unop:
    PushTmpDepthFirst(cctrl, r->base.next, 0);
    PopTmp(cctrl, r->base.next);
    goto fin;
    break;
  case IC_TO_F64:
    goto unop;
    break;
  case IC_PAREN:
    goto unop;
    break;
  case IC_SQRT:
  case IC_NEG:
    goto unop;
    break;
  case IC_POS:
    goto unop;
    break;
  case IC_LT:
  case IC_GT:
  case IC_GE:
  case IC_LE:
  cmp_binop:
    arg = ICArgN(r, 1);
    arg2 = ICArgN(r, 0);
    if (!IsCompoundCompare(r)) {
      PushTmpDepthFirst(cctrl, arg, SpillsTmpRegs(arg2));
      PushTmpDepthFirst(cctrl, arg2, 1);
      PopTmp(cctrl, arg2);
      PopTmp(cctrl, arg);
      goto fin;
    }
    // There are 2 compares here,but 3 args
    // a<b<c
    argc = 1;
    d = r;
    while (IsCompare(d->type)) {
      d = ICFwd(d->base.next);
      argc++;
    }
    array = A_MALLOC(sizeof(CRPN *) * argc, NULL);
    argc = 0;
    d = r;
    while (IsCompare(d->type)) {
      array[argc++] = d->base.next;
      PushTmpDepthFirst(cctrl, d->base.next, 1);
      d = ICFwd(d->base.next);
    }
    // See above note(d here is the 3)
    array[argc++] = d;
    PushTmpDepthFirst(cctrl, d, 1);
    //
    while (argc--)
      PopTmp(cctrl, array[argc]);
    A_FREE(array);
    goto fin;
    break;
  case IC_BT:
  case IC_BTC:
  case IC_BTR:
  case IC_BTS:
  case IC_LBTC:
  case IC_LBTR:
  case IC_LBTS:
  case IC_POW:
  binop:
    arg = ICArgN(r, 1);
    arg2 = ICArgN(r, 0);
    PushTmpDepthFirst(cctrl, arg, SpillsTmpRegs(arg2));
    PushTmpDepthFirst(cctrl, arg2, 0);
    PopTmp(cctrl, arg2);
    PopTmp(cctrl, arg);
    goto fin;
    break;
  case IC_ADD:
    orig = r;
    tmp = 0;
    if (r->raw_type != RT_F64) {
      if (!spilled && GetSIBParts(orig, &i, &b, &idx, &i2, 0)) {
        if (b)
          PushTmpDepthFirst(cctrl, b, 0);
        if (idx)
          PushTmpDepthFirst(cctrl, idx, 0);
        if (b && idx) {
          if (b->res.mode == MD_REG || b->res.mode == __MD_X86_64_LEA_SIB)
            tmp++;
          if (idx->res.mode == MD_REG || idx->res.mode == __MD_X86_64_LEA_SIB)
            tmp++;
          if (AIWNIOS_TMP_IREG_CNT - cctrl->backend_user_data2 - 1 < 0 ||
              tmp != 2)
            goto add_dft;
          if (SpillsTmpRegs(idx))
            goto add_dft;

        lea_sib:
          if (idx)
            PopTmp(cctrl, idx);
          if (b)
            PopTmp(cctrl, b);

          orig->flags |= ICF_INDIR_REG;
          orig->res.mode = __MD_X86_64_LEA_SIB;
          orig->res.__SIB_scale = idx ? i2 : -1;
          orig->res.off = i;
          if (idx) {
            if (idx->res.mode == __MD_X86_64_LEA_SIB)
              orig->res.reg2 = idx->res.fallback_reg;
            else
              orig->res.reg2 = idx->res.reg;
            // Watch out for  SetKeepTmps
            if (!IsGoodIReg(orig->res.reg2))
              goto add_dft;
          } else
            orig->res.reg2 = -1;
          orig->res.__sib_idx_rpn = idx;
          orig->res.__sib_base_rpn = b;
          orig->res.raw_type = r->raw_type;
          if (b) {
            if (b->res.mode == __MD_X86_64_LEA_SIB)
              orig->res.reg = b->res.fallback_reg;
            else
              orig->res.reg = b->res.reg;
            // Watch out for  SetKeepTmps
            if (!IsGoodIReg(orig->res.reg))
              goto add_dft;
          } else
            orig->res.reg = -1;

          orig->res.pop_n_tmp_regs = 1;
          orig->res.fallback_reg = TmpRegToReg(cctrl->backend_user_data2++);
          return 1;
        } else if (b) {
          if (b->res.mode == MD_REG)
            tmp++;
          else if (b->integer == 13) // R13 has quirks when used with SIBs
            goto add_dft;
          if (AIWNIOS_TMP_IREG_CNT - cctrl->backend_user_data2 - 1 < 0 ||
              tmp != 1)
            goto add_dft;
          goto lea_sib;
        } else if (idx && 0) {
          if (idx->res.mode == MD_REG)
            tmp++;
          else if (idx->integer == 13) // R13 has quirks when used with SIBs
            goto add_dft;
          if (AIWNIOS_TMP_IREG_CNT - cctrl->backend_user_data2 - 1 < 0 ||
              tmp != 1)
            goto add_dft;
          goto lea_sib;
        }
      }
    }
  add_dft:
    // Reset flags and stuff from attemptd SIB-ify
    if (!spilled) {
      cctrl->backend_user_data1 = old_scnt;
      cctrl->backend_user_data2 = old_icnt;
      cctrl->backend_user_data3 = old_fcnt;
      CRPN *clear_to = ICFwd(orig), *cur = orig;
      while (cur != clear_to) {
        cur->flags &= ICF_DEAD_CODE | ICF_IS_BOOL | ICF_LOCK_EXPR |
                      ICF_NO_JUMP | ICF_DOESNT_SPILL_TMP_REGS |
                      ICF_SPILLS_TMP_REGS;
        cur->res.mode = 0;
        cur = cur->base.next;
      }
      r = orig;
    }
    goto binop;
    break;
  case IC_EQ:
    PushTmpDepthFirst(cctrl, ICArgN(r, 0), SpillsTmpRegs(ICArgN(r, 1)));
    PushTmpDepthFirst(cctrl, ICArgN(r, 1), 0);
    PopTmp(cctrl, ICArgN(r, 1));
    PopTmp(cctrl, ICArgN(r, 0));
    goto fin;
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
  case IC_DEREF:
    orig = r;
    //
    // We can directly use the code ref(ICF_PRECOMPUTED to avoid recomputing)
    //
    if ((b = ICArgN(r, 0))->type == IC_SHORT_ADDR) {
      r->flags |= ICF_TMP_NO_UNDO;
      r->res.mode = MD_CODE_MISC_PTR;
      r->res.raw_type = r->raw_type;
      r->res.code_misc = b->code_misc;
      // PUSH in case someone tries to bybass IC_DEREF
      PushTmpDepthFirst(cctrl, b, 0);
      PopTmp(cctrl, b);
      r->flags |= ICF_PRECOMPUTED;
      return 1;
    }
    tmp = 0;
    arg = r->base.next;
    if (!spilled && GetSIBParts(arg, &i, &b, &idx, &i2, 0)) {
      if (b)
        PushTmpDepthFirst(cctrl, b, 0);
      if (idx)
        PushTmpDepthFirst(cctrl, idx, 0);
      if (b && idx) {
        tmp = 0;
        if (b->res.mode == MD_REG)
          tmp++;
        if (idx->res.mode == MD_REG)
          tmp++;
        if (tmp && spilled) {
          goto deref_norm;
        }
        if (AIWNIOS_TMP_IREG_CNT - cctrl->backend_user_data2 - tmp - 1 < 0 ||
            tmp != 2)
          goto deref_norm;
        if (SpillsTmpRegs(idx))
          goto deref_norm;
      sib:
        if (idx)
          PopTmp(cctrl, idx);
        if (b)
          PopTmp(cctrl, b);
        if (cctrl->backend_user_data2 + 1 > AIWNIOS_TMP_IREG_CNT)
          goto deref_norm;
        orig->flags |= ICF_SIB;
        orig->res.mode = __MD_X86_64_SIB;
        orig->res.__SIB_scale = idx ? i2 : -1;
        orig->res.off = i;
        if (idx) {
          if (idx->res.mode == __MD_X86_64_LEA_SIB)
            orig->res.reg2 = idx->res.fallback_reg;
          else
            orig->res.reg2 = idx->res.reg;
          // Watch out for  SetKeepTmps
          if (!IsGoodIReg(orig->res.reg2))
            goto deref_norm;
        } else
          orig->res.reg2 = -1;
        orig->res.raw_type = r->raw_type;
        orig->res.__sib_base_rpn = b;
        orig->res.__sib_idx_rpn = idx;
        if (b) {
          if (b->res.mode == __MD_X86_64_LEA_SIB)
            orig->res.reg = b->res.fallback_reg;
          else
            orig->res.reg = b->res.reg;
          // Watch out for  SetKeepTmps
          if (!IsGoodIReg(orig->res.reg))
            goto deref_norm;
        } else
          orig->res.reg = -1;
        orig->res.pop_n_tmp_regs = 1;
        orig->res.fallback_reg = TmpRegToReg(cctrl->backend_user_data2++);
        return 1;
      } else if (b) {
        if (b->res.mode == MD_REG)
          tmp++;
        else if (b->integer == 13) // R13 has quirks when used with SIBs
          goto binop;
        if (tmp && spilled)
          goto deref_norm;
        if (AIWNIOS_TMP_IREG_CNT - cctrl->backend_user_data2 - tmp - 1 < 0 ||
            tmp != 1)
          goto deref_norm;
        goto sib;
      } else if (idx) {
        if (idx->res.mode == MD_REG)
          tmp++;
        else if (idx->integer == 13) // R13 has quirks when used with SIBs
          goto binop;
        if (tmp && spilled)
          goto deref_norm;
        if (AIWNIOS_TMP_IREG_CNT - cctrl->backend_user_data2 - tmp - 1 < 0 ||
            tmp != 1)
          goto deref_norm;
        goto sib;
      }
    }
    // Reset flags and stuff from attemptd SIB-ify
  deref_norm:
    if (!spilled) {
      cctrl->backend_user_data1 = old_scnt;
      cctrl->backend_user_data2 = old_icnt;
      cctrl->backend_user_data3 = old_fcnt;
      CRPN *clear_to = ICFwd(orig), *cur = orig;
      while (cur != clear_to) {
        cur->flags &= ICF_DEAD_CODE | ICF_IS_BOOL | ICF_LOCK_EXPR |
                      ICF_NO_JUMP | ICF_DOESNT_SPILL_TMP_REGS |
                      ICF_SPILLS_TMP_REGS;
        cur->res.mode = 0;
        cur = cur->base.next;
      }
    }
    PushTmpDepthFirst(cctrl, r->base.next, 0);
    PopTmp(cctrl, r->base.next);
    goto fin;
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
  case IC_LNOT:
    goto unop;
    break;
  case IC_BNOT:
    goto unop;
    break;
  case IC_POST_INC:
  post_op:
    // This is the hack i was refering to mother-foofer
    //
    //  Doing R9=(*R9)++ is a bad idea as we need to keep R9's pointer for
    //  storing the result
    //
    //  MOV R9,[R9]
    //  INC R9
    //  MOV [R9], R9
    //
    //  I will Keep 2 differnet tmps ,no sharing result reigster with the source
    //  reigster
    arg = ICArgN(r, 0);
    if (!spilled)
      PushTmp(cctrl, r);
    else
      PushSpilledTmp(cctrl, r);
    PushTmpDepthFirst(cctrl, arg, 0);
    PopTmp(cctrl, arg);
    return 1;
    break;
  case IC_POST_DEC:
    goto post_op;
    break;
  case IC_PRE_INC:
    goto post_op;
    break;
  case IC_PRE_DEC:
    goto post_op;
    break;
  case IC_AND_AND:
  logical_binop:
    arg = ICArgN(r, 1);
    arg2 = ICArgN(r, 0);
    PushTmpDepthFirst(cctrl, arg, SpillsTmpRegs(arg2));
    PushTmpDepthFirst(cctrl, arg2, 0);
    PopTmp(cctrl, arg2);
    PopTmp(cctrl, arg);
    goto fin;
    break;
  case IC_OR_OR:
    goto logical_binop;
    break;
  case IC_XOR_XOR:
    goto logical_binop;
    break;
  case IC_EQ_EQ:
    goto binop;
    break;
  case IC_NE:
    goto binop;
    break;
  case IC_LSH:
    goto binop;
    break;
  case IC_RSH:
    goto binop;
  assign_xop:
    arg = ICArgN(r, 1);
    arg2 = ICArgN(r, 0);
    // Equals (when used with IC_DEREF) are weird,they will preserve the
    // IC_DEREF->next
    //  and the DEREF
    //  *(arg->next)+=arg2
    //  IS TRAETED THE SAME AS
    //  tmp=arg->next;
    //  *tmp=*tmp+arg2;
    //
    //  arg is same as *(arg->next)
    // WE WILL NEED TO KEEP arg->next loaded in a register and such
    d = NULL;
    PushTmpDepthFirst(cctrl, arg2, SpillsTmpRegs(arg));
    if (arg->type == IC_DEREF) {
      PushTmpDepthFirst(cctrl, d = arg->base.next, 0);
      PushTmpDepthFirst(cctrl, arg, 0);
    } else
      PushTmpDepthFirst(cctrl, arg, 0);
    if (arg)
      PopTmp(cctrl, arg);
    if (d)
      PopTmp(cctrl, d);
    PopTmp(cctrl, arg2);
    goto fin;
    break;
  case IC_ADD_EQ:
    goto assign_xop;
    break;
  case IC_SUB_EQ:
    goto assign_xop;
    break;
  case IC_MUL_EQ:
    goto assign_xop;
    break;
  case IC_DIV_EQ:
    goto assign_xop;
    break;
  case IC_LSH_EQ:
    goto assign_xop;
    break;
  case IC_RSH_EQ:
    goto assign_xop;
    break;
  case IC_AND_EQ:
    goto assign_xop;
    break;
  case IC_OR_EQ:
    goto assign_xop;
    break;
  case IC_XOR_EQ:
    goto assign_xop;
    break;
  case IC_MOD_EQ:
    goto assign_xop;
    break;
  case IC_COMMA:
    PushTmpDepthFirst(cctrl, ICArgN(r, 1), 0);
    PopTmp(cctrl, ICArgN(r, 1));
    ICArgN(r, 1)->res.mode = MD_NULL;
    PushTmpDepthFirst(cctrl, ICArgN(r, 0), 0);
    PopTmp(cctrl, ICArgN(r, 0));
    goto fin;
    break;
  case IC_TYPECAST:
    goto unop;
    break;
  case __IC_VARGS:
    if (!spilled)
      PushTmp(cctrl, r);
    else
      PushSpilledTmp(cctrl, r);
    for (a = 0; a != r->length; a++) {
      PushTmpDepthFirst(cctrl, ICArgN(r, a), 0);
      PopTmp(cctrl, ICArgN(r, a));
    }
    break;
  case IC_RELOC:
    goto fin;
    break;
  default:
    return 0;
  fin:
    if (!spilled)
      PushTmp(cctrl, r);
    else
      PushSpilledTmp(cctrl, r);
  }
  return 1;
}

static int64_t DstRegAffectsMode(CICArg *d, CICArg *arg) {
  if (d->mode != MD_REG)
    return 0;
  int64_t r = d->reg;
  switch (arg->mode) {
    break;
  case MD_REG:
    return r == arg->reg;
    break;
  case MD_INDIR_REG:
    return r == arg->reg;
    break;
  case __MD_X86_64_SIB:
    return r == arg->reg || r == arg->reg2;
  }
  return 0;
}
static int64_t __ICFCallTOS(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                            int64_t code_off) {
  int64_t i, has_vargs = 0;
  CICArg tmp = {0};
  CRPN *rpn2;
  int64_t to_pop = rpn->length * 8;
  void *fptr;
  for (i = 0; i < rpn->length; i++) {
    rpn2 = ICArgN(rpn, i);
    if (rpn2->type == __IC_VARGS) {
      to_pop -= 8; // We dont count argv
      to_pop += rpn2->length * 8;
      has_vargs = 1;
      code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
    } else {
      rpn2->res.keep_in_tmp = 1;
      code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
      code_off = PushToStack(cctrl, &rpn2->res, bin, code_off);
    }
  }
  rpn2 = ICArgN(rpn, rpn->length);
  if (rpn2->type == IC_SHORT_ADDR) {
    AIWNIOS_ADD_CODE(X86Call32, 0)
    if (bin)
      CodeMiscAddRef(rpn2->code_misc, bin + code_off - 4);
    goto after_call;
  }
  if (rpn2->type == IC_GLOBAL) {
    if (rpn2->global_var->base.type & HTT_FUN) {
      fptr = ((CHashFun *)rpn2->global_var)->fun_ptr;
      if (!fptr || fptr == &DoNothing)
        goto defacto;
      if (!Is32Bit((int64_t)fptr - (int64_t)(bin + code_off)))
        goto defacto;
      if (cctrl->code_ctrl->final_pass) {
        AIWNIOS_ADD_CODE(X86Call32, (int64_t)fptr - (int64_t)(bin + code_off));
      } else
        goto defacto;
    } else
      goto defacto;
  } else {
  defacto:
    rpn2->res.raw_type = RT_PTR; // Not set for some reason
    if (rpn2->type == IC_RELOC) {

      AIWNIOS_ADD_CODE(X86CallSIB64, -1, -1, RIP, 0);
      if (bin)
        CodeMiscAddRef(rpn2->code_misc, bin + code_off - 4);
    } else if (rpn2->type == IC_I64 &&
               Is32Bit(rpn2->integer - (int64_t)(bin + code_off))) {

      AIWNIOS_ADD_CODE(X86Call32, rpn2->integer - (int64_t)(bin + code_off));
    } else {
      code_off = __OptPassFinal(cctrl, rpn2, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &rpn2->res, RT_PTR,
                                 AIWNIOS_TMP_IREG_POOP, bin, code_off);

      AIWNIOS_ADD_CODE(X86CallReg, rpn2->res.reg);
    }
  }
after_call:
  if (has_vargs)
    AIWNIOS_ADD_CODE(X86AddImm32, RSP, to_pop);
  if (rpn->raw_type != RT_U0 && rpn->res.mode != MD_NULL) {
    tmp.reg = 0;
    tmp.mode = MD_REG;
    tmp.raw_type =
        rpn->raw_type == RT_F64 ? RT_F64 : RT_I64i; // Promote to 64bits
    rpn->tmp_res = tmp;
    if (rpn->res.keep_in_tmp) {
      rpn->res = tmp;
      rpn->res.keep_in_tmp = 1;
    }
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
  }
  return code_off;
}
int64_t ModeEqual(CICArg *dst, CICArg *src) {
  if (dst->raw_type == src->raw_type) {
    if (dst->mode == src->mode) {
      switch (dst->mode) {
        break;
      case __MD_X86_64_SIB:
        if (dst->off == src->off && dst->__SIB_scale == src->__SIB_scale &&
            dst->reg == src->reg && dst->reg2 == src->reg2) {
          return 1;
        }
        break;
      case MD_STATIC:
        if (dst->off == src->off)
          return 1;
        break;
      case MD_PTR:
        if (dst->off == src->off)
          return 1;
        break;
      case MD_REG:
        if (dst->reg == src->reg)
          return 1;
        break;
      case MD_FRAME:
        if (dst->off == src->off)
          return 1;
        break;
      case MD_INDIR_REG:
        if (dst->off == src->off && dst->reg == src->reg)
          return 1;
      }
    }
  } else if ((dst->raw_type == RT_F64) == (src->raw_type == RT_F64))
    return dst->mode == MD_REG && src->mode == MD_REG &&
           dst->reg == src->reg; // All Registers are promoted to 64bit
  return 0;
}
int64_t ICMov(CCmpCtrl *cctrl, CICArg *dst, CICArg *src, char *bin,
              int64_t code_off) {
  int64_t use_reg, use_reg2, restore_from_tmp = 0, indir_off = 0,
                             indir_off2 = 0, opc;
  CICArg tmp = {0}, tmp2 = {0}, *new;
  assert(src->mode > 0 || src->mode == -1);
  assert(dst->mode > 0 || dst->mode == -1);
  if (dst->mode == MD_NULL)
    goto ret;
  if (ModeEqual(dst, src))
    goto ret;
  switch (dst->mode) {
    break;
  case MD_CODE_MISC_PTR:
    if (src->mode == MD_REG &&
        ((src->raw_type == RT_F64) == (dst->raw_type == RT_F64))) {
      switch (dst->raw_type) {
        break;
      case RT_U8i:
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86MovIndirRegI8, MIR(cctrl, src->reg), -1, -1, RIP,
                         0);
        break;
      case RT_U16i:
      case RT_I16i:
        AIWNIOS_ADD_CODE(X86MovIndirRegI16, MIR(cctrl, src->reg), -1, -1, RIP,
                         0);
        break;
      case RT_U32i:
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86MovIndirRegI32, MIR(cctrl, src->reg), -1, -1, RIP,
                         0);
        break;
      case RT_PTR:
      case RT_U64i:
      case RT_I64i:
      case RT_FUNC:
        AIWNIOS_ADD_CODE(X86MovIndirRegI64, MIR(cctrl, src->reg), -1, -1, RIP,
                         0);
        break;
      case RT_F64:
        AIWNIOS_ADD_CODE(X86MovIndirRegF64, MFR(cctrl, src->reg), -1, -1, RIP,
                         0);
        break;
      default:
        abort();
      }
      if (bin)
        CodeMiscAddRef(dst->code_misc, bin + code_off - 4);
      goto ret;
    }
    goto dft;
  case __MD_X86_64_SIB:
    if (src->mode == MD_I64 && Is32Bit(src->integer) &&
        ((src->raw_type == RT_F64) == (dst->raw_type == RT_F64))) {
      switch (dst->raw_type) {
        break;
      case RT_U8i:
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86MovIndirI8Imm, src->integer, dst->__SIB_scale,
                         dst->reg2, dst->reg, dst->off);
        break;
      case RT_U16i:
      case RT_I16i:

        AIWNIOS_ADD_CODE(X86MovIndirI16Imm, src->integer, dst->__SIB_scale,
                         dst->reg2, dst->reg, dst->off);
        break;
      case RT_U32i:
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86MovIndirI32Imm, src->integer, dst->__SIB_scale,
                         dst->reg2, dst->reg, dst->off);
        break;
      case RT_PTR:
      case RT_U64i:
      case RT_I64i:
      case RT_FUNC:
        AIWNIOS_ADD_CODE(X86MovIndirI64Imm, src->integer, dst->__SIB_scale,
                         dst->reg2, dst->reg, dst->off);
        break;
      default:
        abort();
      }
      goto ret;
    } else if (src->mode == MD_REG &&
               ((src->raw_type == RT_F64) == (dst->raw_type == RT_F64))) {
      switch (dst->raw_type) {
        break;
      case RT_U8i:
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86MovIndirRegI8, src->reg, dst->__SIB_scale,
                         dst->reg2, dst->reg, dst->off);
        break;
      case RT_U16i:
      case RT_I16i:
        AIWNIOS_ADD_CODE(X86MovIndirRegI16, src->reg, dst->__SIB_scale,
                         dst->reg2, dst->reg, dst->off);
        break;
      case RT_U32i:
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86MovIndirRegI32, src->reg, dst->__SIB_scale,
                         dst->reg2, dst->reg, dst->off);
        break;
      case RT_PTR:
      case RT_U64i:
      case RT_I64i:
      case RT_FUNC:
        AIWNIOS_ADD_CODE(X86MovIndirRegI64, src->reg, dst->__SIB_scale,
                         dst->reg2, dst->reg, dst->off);
        break;
      case RT_F64:
        // TODO atomic
        AIWNIOS_ADD_CODE(X86MovIndirRegF64, src->reg, dst->__SIB_scale,
                         dst->reg2, dst->reg, dst->off);
        break;
      default:
        abort();
      }
      goto ret;
    }
    goto dft;
    break;
  case MD_STATIC:
    if (src->mode == MD_I64 && Is32Bit(src->integer) &&
        ((src->raw_type == RT_F64) == (dst->raw_type == RT_F64))) {
      switch (dst->raw_type) {
        break;
      case RT_U8i:
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86MovIndirI8Imm, src->integer, -1, -1, RIP, 1000);
        indir_off2 = 1;
        break;
      case RT_U16i:
      case RT_I16i:
        AIWNIOS_ADD_CODE(X86MovIndirI16Imm, src->integer, -1, -1, RIP, 1000);
        indir_off2 = 2;
        break;
      case RT_U32i:
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86MovIndirI32Imm, src->integer, -1, -1, RIP, 1000);
        indir_off2 = 4;
        break;
      case RT_PTR:
      case RT_U64i:
      case RT_I64i:
      case RT_FUNC:
      case RT_F64:
        AIWNIOS_ADD_CODE(X86MovIndirI64Imm, src->integer, -1, -1, RIP, 1000);
        indir_off2 = 4;
        break;
      default:
        abort();
      }
      if (bin) {
        // Heres the deal
        // These are encoded as CX05[offset][immediate]
        // When we do the relocation,be sure to modify the offset and not the
        // immeidate Also add indir_off2 to point to the end of the instriction
        // as the relocation only points to the offset
        CodeMiscAddRef(cctrl->statics_label, bin + code_off - 4 - indir_off2)
            ->offset = dst->off - indir_off2;
      }
      goto ret;
    } else if (src->mode == MD_REG &&
               (dst->raw_type == RT_F64) == (src->raw_type == RT_F64)) {
      switch (dst->raw_type) {
        break;
      case RT_U8i:
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86MovIndirRegI8, MIR(cctrl, src->reg), -1, -1, RIP,
                         1000);
        break;
      case RT_U16i:
      case RT_I16i:
        AIWNIOS_ADD_CODE(X86MovIndirRegI16, MIR(cctrl, src->reg), -1, -1, RIP,
                         1000);
        break;
      case RT_U32i:
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86MovIndirRegI32, src->reg, -1, -1, RIP, 1000);
        break;
      case RT_PTR:
      case RT_U64i:
      case RT_I64i:
      case RT_FUNC:
        AIWNIOS_ADD_CODE(X86MovIndirRegI64, MIR(cctrl, src->reg), -1, -1, RIP,
                         1000);
        break;
      case RT_F64:
        AIWNIOS_ADD_CODE(X86MovIndirRegF64, MFR(cctrl, src->reg), -1, -1, RIP,
                         1000);
        break;
      default:
        abort();
      }
      if (bin)
        CodeMiscAddRef(cctrl->statics_label, bin + code_off - 4)->offset =
            dst->off;
      goto ret;
    }
    goto dft;
    break;
  case MD_INDIR_REG:
    use_reg2 = dst->reg;
    indir_off = dst->off;
  indir_r2:
    if (src->raw_type == dst->raw_type && src->raw_type == RT_F64 &&
        src->mode == MD_REG) {
      use_reg = src->reg;
    } else if ((src->raw_type == RT_F64) ^
               (RT_F64 == dst->raw_type)) { // Do they differ
      goto dft;
    } else if (src->mode == MD_FRAME || src->mode == MD_PTR ||
               src->mode == MD_INDIR_REG) {
      goto dft;
    } else if (src->mode == MD_REG)
      use_reg = src->reg;
    else if (src->mode == MD_I64 && Is32Bit(src->integer) &&
             ((src->raw_type == RT_F64) == (dst->raw_type == RT_F64))) {
      switch (dst->raw_type) {
        break;
      case RT_U8i:
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86MovIndirI8Imm, src->integer, -1, -1, use_reg2,
                         indir_off);
        break;
      case RT_U16i:
      case RT_I16i:
        AIWNIOS_ADD_CODE(X86MovIndirI16Imm, src->integer, -1, -1, use_reg2,
                         indir_off);
        break;
      case RT_U32i:
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86MovIndirI32Imm, src->integer, -1, -1, use_reg2,
                         indir_off);
        break;
      case RT_PTR:
      case RT_U64i:
      case RT_I64i:
      case RT_FUNC:
        AIWNIOS_ADD_CODE(X86MovIndirI64Imm, src->integer, -1, -1, use_reg2,
                         indir_off);
        break;
      default:
        abort();
      }
      goto ret;
    } else {
    dft:
      tmp.raw_type = src->raw_type;
      if (dst->raw_type != RT_F64)
        use_reg = tmp.reg = AIWNIOS_TMP_IREG_POOP;
      else
        use_reg = tmp.reg = 0;
      if (dst->mode == MD_REG)
        use_reg = tmp.reg = dst->reg;
      tmp.mode = MD_REG;
      if (dst->raw_type == RT_F64 && src->raw_type != dst->raw_type) {
        tmp.raw_type = RT_F64;
        if (src->mode == MD_REG) {
          AIWNIOS_ADD_CODE(X86CVTTSI2SDRegReg, MFR(cctrl, use_reg), src->reg);
        } else if (src->mode == MD_INDIR_REG && RawTypeIs64(src->raw_type)) {
          AIWNIOS_ADD_CODE(X86CVTTSI2SDRegSIB64, MFR(cctrl, use_reg), -1, -1,
                           src->reg, src->off);
        } else if (src->mode == MD_FRAME && RawTypeIs64(src->raw_type)) {
          AIWNIOS_ADD_CODE(X86CVTTSI2SDRegSIB64, MFR(cctrl, use_reg), -1, -1,
                           RBP, -src->off);
        } /*else if(src->mode==__MD_X86_64_SIB) {
          AIWNIOS_ADD_CODE(X86CVTTSI2SDRegSIB64, use_reg, src->__SIB_scale,
        src->reg2, src->reg, src->off);
        } */
        else {
          tmp2 = tmp;
          tmp2.mode = MD_REG;
          tmp2.reg = 0;
          tmp2.raw_type = src->raw_type;
          code_off = ICMov(cctrl, &tmp2, src, bin, code_off);
          code_off = ICMov(cctrl, &tmp, &tmp2, bin, code_off);
        }
      } else if (src->raw_type == RT_F64 && src->raw_type != dst->raw_type) {
        tmp.raw_type = RT_I64i;
        if (src->mode == MD_REG) {
          AIWNIOS_ADD_CODE(X86CVTTSD2SIRegReg, MIR(cctrl, use_reg), src->reg);
        } else if (src->mode == MD_INDIR_REG && RawTypeIs64(src->raw_type)) {
          AIWNIOS_ADD_CODE(X86CVTTSD2SIRegSIB64, MIR(cctrl, use_reg), -1, -1,
                           src->reg, src->off);
        } else if (src->mode == MD_FRAME && RawTypeIs64(src->raw_type)) {
          AIWNIOS_ADD_CODE(X86CVTTSD2SIRegSIB64, MIR(cctrl, use_reg), -1, -1,
                           RBP, -src->off);
        } /*else if(src->mode==__MD_X86_64_SIB) {
          AIWNIOS_ADD_CODE(X86CVTTSD2SIRegSIB64, use_reg, src->__SIB_scale,
        src->reg2, src->reg, src->off);
        } */
        else {
          tmp2 = tmp;
          tmp2.mode = MD_REG;
          tmp2.reg = 0;
          tmp2.raw_type = src->raw_type;
          code_off = ICMov(cctrl, &tmp2, src, bin, code_off);
          code_off = ICMov(cctrl, &tmp, &tmp2, bin, code_off);
        }
      } else
        code_off = ICMov(cctrl, &tmp, src, bin, code_off);
      code_off = ICMov(cctrl, dst, &tmp, bin, code_off);
      goto ret;
    }
    goto store_r2;
    break;
  case MD_PTR:
    if (src->mode != MD_REG ||
        ((src->raw_type == RT_F64) != (dst->raw_type == RT_F64)))
      goto dft;
    use_reg = src->reg;

    if (Is32Bit((char *)dst->off - (char *)MemGetExecPtr(bin + code_off))) {
      indir_off = (char *)dst->off - (char *)MemGetExecPtr(bin + code_off);
      use_reg2 = RIP;
      goto store_r2;
    }
    use_reg2 =
        (cctrl->flags & CCF_ICMOV_NO_USE_RAX) ? AIWNIOS_TMP_IREG_POOP2 : RAX;
    code_off = __ICMoveI64(cctrl, use_reg2, dst->off, bin, code_off);
    indir_off = 0;
    goto store_r2;
  store_r2:
    switch (dst->raw_type) {
    case RT_U0:
      break;
    case RT_F64:
      AIWNIOS_ADD_CODE(X86MovIndirRegF64, MFR(cctrl, use_reg), -1, -1, use_reg2,
                       indir_off);
      break;
    case RT_U8i:
    case RT_I8i:
      AIWNIOS_ADD_CODE(X86MovIndirRegI8, MIR(cctrl, use_reg), -1, -1, use_reg2,
                       indir_off);
      break;
    case RT_U16i:
    case RT_I16i:
      AIWNIOS_ADD_CODE(X86MovIndirRegI16, MIR(cctrl, use_reg), -1, -1, use_reg2,
                       indir_off);
      break;
    case RT_U32i:
    case RT_I32i:
      AIWNIOS_ADD_CODE(X86MovIndirRegI32, MIR(cctrl, use_reg), -1, -1, use_reg2,
                       indir_off);
      break;
    case RT_U64i:
    case RT_I64i:
    case RT_FUNC: // func ptr
    case RT_PTR:
      AIWNIOS_ADD_CODE(X86MovIndirRegI64, MIR(cctrl, use_reg), -1, -1, use_reg2,
                       indir_off);
      break;
    default:
      abort();
    }
    break;
  case MD_FRAME:
    use_reg2 = X86_64_BASE_REG;
    indir_off = -dst->off;
    goto indir_r2;
    break;
  case MD_REG:
    use_reg = dst->reg;
    if (src->mode == MD_FRAME) {
      if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
        use_reg2 = X86_64_BASE_REG;
        indir_off = -src->off;
        goto load_r2;
      } else if ((src->raw_type == RT_F64) ^ (RT_F64 == dst->raw_type)) {
        goto dft;
      }
      use_reg2 = X86_64_BASE_REG;
      indir_off = -src->off;
      goto load_r2;
    } else if (src->mode == __MD_X86_64_SIB) {
      if ((src->raw_type == RT_F64) != (dst->raw_type == RT_F64))
        goto dft;
      switch (src->raw_type) {
      case RT_U0:
        break;
      case RT_U8i:
        AIWNIOS_ADD_CODE(X86MovZXRegIndirI8, MIR(cctrl, dst->reg),
                         src->__SIB_scale, src->reg2, src->reg, src->off);
        break;
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86MovSXRegIndirI8, MIR(cctrl, dst->reg),
                         src->__SIB_scale, src->reg2, src->reg, src->off);
        break;
      case RT_U16i:
        AIWNIOS_ADD_CODE(X86MovZXRegIndirI16, MIR(cctrl, dst->reg),
                         src->__SIB_scale, src->reg2, src->reg, src->off);
        break;
      case RT_I16i:
        AIWNIOS_ADD_CODE(X86MovSXRegIndirI16, MIR(cctrl, dst->reg),
                         src->__SIB_scale, src->reg2, src->reg, src->off);
        break;
      case RT_U32i:
        AIWNIOS_ADD_CODE(X86MovRegIndirI32, MIR(cctrl, dst->reg),
                         src->__SIB_scale, src->reg2, src->reg, src->off);
        break;
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86MovSXRegIndirI32, MIR(cctrl, dst->reg),
                         src->__SIB_scale, src->reg2, src->reg, src->off);
        break;
      case RT_U64i:
      case RT_PTR:
      case RT_FUNC:
      case RT_I64i:
        AIWNIOS_ADD_CODE(X86MovRegIndirI64, MIR(cctrl, dst->reg),
                         src->__SIB_scale, src->reg2, src->reg, src->off);
        break;
      case RT_F64:
        AIWNIOS_ADD_CODE(X86MovRegIndirF64, MFR(cctrl, dst->reg),
                         src->__SIB_scale, src->reg2, src->reg, src->off);
      }
      goto ret;
    } else if (src->mode == MD_CODE_MISC_PTR) {
      if ((src->raw_type == RT_F64) != (dst->raw_type == RT_F64))
        goto dft;
      switch (src->raw_type) {
      case RT_U0:
        break;
      case RT_U8i:
        AIWNIOS_ADD_CODE(X86MovZXRegIndirI8, MIR(cctrl, dst->reg), -1, -1, RIP,
                         0);
        break;
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86MovSXRegIndirI8, MIR(cctrl, dst->reg), -1, -1, RIP,
                         0);
        break;
      case RT_U16i:
        AIWNIOS_ADD_CODE(X86MovZXRegIndirI16, MIR(cctrl, dst->reg), -1, -1, RIP,
                         0);
        break;
      case RT_I16i:
        AIWNIOS_ADD_CODE(X86MovSXRegIndirI16, MIR(cctrl, dst->reg), -1, -1, RIP,
                         0);
        break;
      case RT_U32i:
        AIWNIOS_ADD_CODE(X86MovRegIndirI32, MIR(cctrl, dst->reg), -1, -1, RIP,
                         0);
        break;
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86MovSXRegIndirI32, MIR(cctrl, dst->reg), -1, -1, RIP,
                         0);
        break;
      case RT_U64i:
      case RT_PTR:
      case RT_FUNC:
      case RT_I64i:
        AIWNIOS_ADD_CODE(X86MovRegIndirI64, MIR(cctrl, dst->reg), -1, -1, RIP,
                         0);
        break;
      case RT_F64:
        AIWNIOS_ADD_CODE(X86MovRegIndirF64, MFR(cctrl, dst->reg), -1, -1, RIP,
                         0);
      }
      if (bin)
        CodeMiscAddRef(src->code_misc, bin + code_off - 4);
      goto ret;
    } else if (src->mode == MD_REG) {
      if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
        AIWNIOS_ADD_CODE(X86MovRegRegF64, MFR(cctrl, dst->reg), src->reg);
      } else if (src->raw_type != RT_F64 && dst->raw_type != RT_F64) {
        AIWNIOS_ADD_CODE(X86MovRegReg, MIR(cctrl, dst->reg), src->reg);
      } else if (src->raw_type == RT_F64 && dst->raw_type != RT_F64) {
        goto dft;
      } else if (src->raw_type != RT_F64 && dst->raw_type == RT_F64) {
        goto dft;
      }
    } else if (src->mode == MD_PTR) {
      if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
        use_reg2 = (cctrl->flags & CCF_ICMOV_NO_USE_RAX)
                       ? AIWNIOS_TMP_IREG_POOP2
                       : RAX;
        code_off = __ICMoveI64(cctrl, use_reg2, src->off, bin, code_off);
        goto load_r2;
      } else if ((src->raw_type == RT_F64) ^ (RT_F64 == dst->raw_type)) {
        goto dft;
      }
      if (Is32Bit((char *)src->integer -
                  (char *)MemGetExecPtr(bin + code_off))) {
        switch (src->raw_type) {
        case RT_U0:
          break;
        case RT_U8i:
          AIWNIOS_ADD_CODE(
              X86MovZXRegIndirI8, MIR(cctrl, dst->reg), -1, -1, RIP,
              (char *)src->integer - (char *)MemGetExecPtr(bin + code_off));
          break;
        case RT_I8i:
          AIWNIOS_ADD_CODE(
              X86MovSXRegIndirI8, MIR(cctrl, dst->reg), -1, -1, RIP,
              (char *)src->integer - (char *)MemGetExecPtr(bin + code_off));
          break;
        case RT_U16i:
          AIWNIOS_ADD_CODE(
              X86MovZXRegIndirI16, MIR(cctrl, dst->reg), -1, -1, RIP,
              (char *)src->integer - (char *)MemGetExecPtr(bin + code_off));
          break;
        case RT_I16i:
          AIWNIOS_ADD_CODE(
              X86MovSXRegIndirI16, MIR(cctrl, dst->reg), -1, -1, RIP,
              (char *)src->integer - (char *)MemGetExecPtr(bin + code_off));
          break;
        case RT_U32i:
          AIWNIOS_ADD_CODE(X86MovRegIndirI32, MIR(cctrl, dst->reg), -1, -1, RIP,
                           (char *)src->integer -
                               (char *)MemGetExecPtr(bin + code_off));
          break;
        case RT_I32i:
          AIWNIOS_ADD_CODE(
              X86MovSXRegIndirI32, MIR(cctrl, dst->reg), -1, -1, RIP,
              (char *)src->integer - (char *)MemGetExecPtr(bin + code_off));
          break;
        case RT_U64i:
        case RT_PTR:
        case RT_FUNC:
        case RT_I64i:
          AIWNIOS_ADD_CODE(X86MovRegIndirI64, MIR(cctrl, dst->reg), -1, -1, RIP,
                           (char *)src->integer -
                               (char *)MemGetExecPtr(bin + code_off));
          break;
        case RT_F64:
          AIWNIOS_ADD_CODE(X86MovRegIndirF64, MFR(cctrl, dst->reg), -1, -1, RIP,
                           (char *)src->integer -
                               (char *)MemGetExecPtr(bin + code_off));
        }
      } else {
        use_reg2 = (cctrl->flags & CCF_ICMOV_NO_USE_RAX)
                       ? AIWNIOS_TMP_IREG_POOP2
                       : RAX;
        code_off = __ICMoveI64(cctrl, use_reg2, src->off, bin, code_off);
        goto load_r2;
      }
      goto ret;
    load_r2:
      switch (src->raw_type) {
      case RT_U0:
        break;
      case RT_U8i:
        AIWNIOS_ADD_CODE(X86MovZXRegIndirI8, MIR(cctrl, dst->reg), -1, -1,
                         use_reg2, indir_off);
        break;
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86MovSXRegIndirI8, MIR(cctrl, dst->reg), -1, -1,
                         use_reg2, indir_off);
        break;
      case RT_U16i:
        AIWNIOS_ADD_CODE(X86MovZXRegIndirI16, MIR(cctrl, dst->reg), -1, -1,
                         use_reg2, indir_off);
        break;
      case RT_I16i:
        AIWNIOS_ADD_CODE(X86MovSXRegIndirI16, MIR(cctrl, dst->reg), -1, -1,
                         use_reg2, indir_off);
        break;
      case RT_U32i:
        AIWNIOS_ADD_CODE(X86MovRegIndirI32, MIR(cctrl, dst->reg), -1, -1,
                         use_reg2, indir_off);
        break;
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86MovSXRegIndirI32, MIR(cctrl, dst->reg), -1, -1,
                         use_reg2, indir_off);
        break;
      case RT_U64i:
      case RT_PTR:
      case RT_FUNC:
      case RT_I64i:
        AIWNIOS_ADD_CODE(X86MovRegIndirI64, MIR(cctrl, dst->reg), -1, -1,
                         use_reg2, indir_off);
        break;
      case RT_F64:
        AIWNIOS_ADD_CODE(X86MovRegIndirF64, MFR(cctrl, dst->reg), -1, -1,
                         use_reg2, indir_off);
        break;
      default:
        abort();
      }
    } else if (src->mode == MD_INDIR_REG) {
      use_reg2 = src->reg;
      indir_off = src->off;
      if (src->raw_type == RT_F64 && src->raw_type == dst->raw_type) {
        use_reg2 = src->reg;
      } else if ((src->raw_type == RT_F64) ^ (RT_F64 == dst->raw_type)) {
        goto dft;
      }
      goto load_r2;
    } else if (src->mode == MD_I64 && dst->raw_type != RT_F64) {
      code_off = __ICMoveI64(cctrl, dst->reg, src->integer, bin, code_off);
    } else if (src->mode == MD_F64 && dst->raw_type == RT_F64) {
      code_off = __ICMoveF64(cctrl, dst->reg, src->flt, bin, code_off);
    } else if (src->mode == MD_I64 && dst->raw_type == RT_F64) {
      code_off = __ICMoveF64(cctrl, dst->reg, src->integer, bin, code_off);
    } else if (src->mode == MD_F64 && dst->raw_type != RT_F64) {
      code_off = __ICMoveI64(cctrl, dst->reg, src->flt, bin, code_off);
    } else if (src->mode == MD_STATIC) {
      if (dst->mode == MD_REG &&
          (dst->raw_type == RT_F64) == (src->raw_type == RT_F64)) {
        switch (src->raw_type) {
        case RT_U0:
          break;
        case RT_U8i:
          AIWNIOS_ADD_CODE(X86MovZXRegIndirI8, MIR(cctrl, dst->reg), -1, -1,
                           RIP, 1000);
          break;
        case RT_I8i:
          AIWNIOS_ADD_CODE(X86MovSXRegIndirI8, MIR(cctrl, dst->reg), -1, -1,
                           RIP, 1000);
          break;
        case RT_U16i:
          AIWNIOS_ADD_CODE(X86MovZXRegIndirI16, MIR(cctrl, dst->reg), -1, -1,
                           RIP, 1000);
          break;
        case RT_I16i:
          AIWNIOS_ADD_CODE(X86MovSXRegIndirI16, MIR(cctrl, dst->reg), -1, -1,
                           RIP, 1000);
          break;
        case RT_U32i:
          AIWNIOS_ADD_CODE(X86MovRegIndirI32, MIR(cctrl, dst->reg), -1, -1, RIP,
                           1000);
          break;
        case RT_I32i:
          AIWNIOS_ADD_CODE(X86MovSXRegIndirI32, MIR(cctrl, dst->reg), -1, -1,
                           RIP, 1000);
          break;
        case RT_U64i:
        case RT_PTR:
        case RT_FUNC:
        case RT_I64i:
          AIWNIOS_ADD_CODE(X86MovRegIndirI64, MIR(cctrl, dst->reg), -1, -1, RIP,
                           1000);
          break;
        case RT_F64:
          AIWNIOS_ADD_CODE(X86MovRegIndirF64, MFR(cctrl, dst->reg), -1, -1, RIP,
                           1000);
        }
        if (bin)
          CodeMiscAddRef(cctrl->statics_label, bin + code_off - 4)->offset =
              src->off;
      } else
        goto dft;
      goto ret;
    } else
      goto dft;
    break;
  case MD_I64:
  case MD_F64:
    break;
  default:
    abort();
  }
ret:
  return code_off;
}

static int64_t DerefToICArg(CCmpCtrl *cctrl, CICArg *res, CRPN *rpn,
                            int64_t base_reg_fallback, char *bin,
                            int64_t code_off) {
  if (rpn->res.mode == __MD_X86_64_SIB) {
    if (rpn->res.__sib_base_rpn) {
      code_off = __OptPassFinal(cctrl, rpn->res.__sib_base_rpn, bin, code_off);
      rpn->res.reg = rpn->res.__sib_base_rpn->res.reg;
    }
    if (rpn->res.__sib_idx_rpn) {
      code_off = __OptPassFinal(cctrl, rpn->res.__sib_idx_rpn, bin, code_off);
      rpn->res.reg2 = rpn->res.__sib_idx_rpn->res.reg;
    }
    *res = rpn->res;
    return code_off;
  }
  if (rpn->type != IC_DEREF) {
    if (rpn->type == IC_BASE_PTR) {
      res->raw_type = rpn->raw_type;
      res->mode = __MD_X86_64_SIB;
      res->reg = X86_64_BASE_REG;
      res->reg2 = -1;
      res->off = -rpn->integer;
      res->__SIB_scale = -1;
      return code_off;
    }
    code_off = __OptPassFinal(cctrl, rpn, bin, code_off);
    if (rpn->res.mode == MD_FRAME) {
      res->raw_type = rpn->raw_type;
      res->mode = __MD_X86_64_SIB;
      res->reg = X86_64_BASE_REG;
      res->reg2 = -1;
      res->off = -rpn->res.off;
      res->__SIB_scale = -1;
      return code_off;
    }
    *res = rpn->res;
    return code_off;
  }
  int64_t r = rpn->raw_type, rsz, off, mul, lea = 0, reg;
  CRPN *next = rpn->base.next, *next2, *next3, *next4, *tmp;
  CRPN *base, *index;
  switch (r) {
    break;
  case RT_I8i:
    rsz = 1;
    break;
  case RT_U8i:
    rsz = 1;
    break;
  case RT_I16i:
    rsz = 2;
    break;
  case RT_U16i:
    rsz = 2;
    break;
  case RT_I32i:
    rsz = 4;
    break;
  case RT_U32i:
    rsz = 4;
    break;
  default:
    rsz = 8;
  }

  if (GetSIBParts(next, &off, &base, &index, &mul, 1)) {
    if (index && base) {
      // TODO idx may mutate base's registers and vice versa
      goto defacto;
    }
    if (!base)
      goto defacto;
    res->reg2 = -1;
    res->reg = -1;
    if (base) {
      code_off = __OptPassFinal(cctrl, base, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &base->res, RT_I64i, base_reg_fallback,
                                 bin, code_off);
      res->reg = base->res.reg;
    }
    res->mode = __MD_X86_64_SIB;
    res->off = off;
    res->__SIB_scale = mul;
    res->raw_type = r;
  } else {
  defacto:
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    code_off = PutICArgIntoReg(cctrl, &next->res, RT_PTR, base_reg_fallback,
                               bin, code_off);

    res->mode = __MD_X86_64_SIB;
    res->reg = next->res.reg;
    res->reg2 = -1;
    res->off = 0;
    res->__SIB_scale = -1;
    res->raw_type = r;
  }
  return code_off;
}

#define TYPECAST_ASSIGN_BEGIN(DST, SRC)                                        \
  {                                                                            \
    int64_t pop = 0, pop2 = 0;                                                 \
    CICArg _orig = {0};                                                        \
    CRPN *_tc = DST;                                                           \
    CRPN *SRC2 = SRC;                                                          \
    DST = DST->base.next;                                                      \
    if (DST->type == IC_DEREF) {                                               \
      code_off = DerefToICArg(cctrl, &_orig, DST, 1, bin, code_off);           \
      DST->res = _orig;                                                        \
      DST->raw_type = _tc->raw_type;                                           \
    } else if ((DST->raw_type == RT_F64) ^ (SRC2->raw_type == RT_F64)) {       \
      pop2 = pop = 1;                                                          \
      _orig = DST->res;                                                        \
      code_off =                                                               \
          PutICArgIntoReg(cctrl, &DST->res, DST->raw_type, 0, bin, code_off);  \
      code_off = PushToStack(cctrl, &DST->res, bin, code_off);                 \
      DST->res.mode = MD_INDIR_REG;                                            \
      DST->res.off = 0;                                                        \
      DST->res.reg = X86_64_STACK_REG;                                         \
      DST->res.raw_type = _tc->raw_type;                                       \
      DST->raw_type = _tc->raw_type;                                           \
    } else {                                                                   \
      pop2 = 1;                                                                \
      DST->res.raw_type = _tc->raw_type;                                       \
      DST->raw_type = _tc->raw_type;                                           \
    }
#define TYPECAST_ASSIGN_END(DST)                                               \
  if (pop)                                                                     \
    code_off = PopFromStack(cctrl, &_orig, bin, code_off);                     \
  }

static int64_t __SexyPreOp(
    CCmpCtrl *cctrl, CRPN *rpn, int64_t (*i_imm)(char *, int64_t, int64_t),
    int64_t (*ireg)(char *, int64_t, int64_t),
    int64_t (*freg)(char *, int64_t, int64_t), int64_t (*incr)(char *, int64_t),
    int64_t (*incr_sib)(char *, int64_t, int64_t, int64_t, int64_t, int64_t),
    int64_t (*i_imm_sib)(char *, int64_t, int64_t, int64_t, int64_t, int64_t,
                         int64_t),
    char *bin, int64_t code_off) {
  int64_t code, sz;
  CRPN *next = rpn->base.next, *tc;
  CICArg derefed = {0}, tmp = {0}, tmp2 = {0};
  if (next->type == IC_TYPECAST) {
    TYPECAST_ASSIGN_BEGIN(next, next);
    switch (next->raw_type) {
    case RT_I8i:
    case RT_U8i:
      sz = 1;
      break;
    case RT_I16i:
    case RT_U16i:
      sz = 2;
      break;
    case RT_I32i:
    case RT_U32i:
      sz = 4;
      break;
    default:
      sz = 8;
      break;
    }
    if (next->raw_type == RT_F64) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      tmp = derefed; //==RT_F64
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 1, bin, code_off);
      code_off = __ICMoveF64(cctrl, 2, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(freg, MFR(cctrl, tmp.reg), 2);
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else if (rpn->integer != 1) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      tmp = derefed;
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 2, bin, code_off);
      AIWNIOS_ADD_CODE(i_imm, MIR(cctrl, tmp.reg), rpn->integer);
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else if (ModeIsDerefToSIB(next)) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      AIWNIOS_ADD_CODE(incr_sib, sz, derefed.__SIB_scale, derefed.reg2,
                       derefed.reg, derefed.off);
      code_off = ICMov(cctrl, &rpn->res, &derefed, bin, code_off);
    } else {
      CICArg orig = {0};
      code_off = DerefToICArg(cctrl, &next->res, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      orig = next->res;
      tmp = next->res;
      code_off = PutICArgIntoReg(cctrl, &tmp, RT_I64i, RAX, bin, code_off);
      AIWNIOS_ADD_CODE(incr, MIR(cctrl, tmp.reg));
      code_off = ICMov(cctrl, &orig, &tmp, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    TYPECAST_ASSIGN_END(next);
  } else {
    switch (next->raw_type) {
    case RT_I8i:
    case RT_U8i:
      sz = 1;
      break;
    case RT_I16i:
    case RT_U16i:
      sz = 2;
      break;
    case RT_I32i:
    case RT_U32i:
      sz = 4;
      break;
    default:
      sz = 8;
      break;
    }
    if (next->raw_type == RT_F64) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      tmp = derefed; //==RT_F64
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type,
                                 AIWNIOS_TMP_IREG_POOP, bin, code_off);
      code_off = __ICMoveF64(cctrl, 2, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(freg, MFR(cctrl, tmp.reg), 2);
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else if (rpn->integer != 1 && ModeIsDerefToSIB(next)) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      if (rpn->flags & ICF_LOCK_EXPR)
        AIWNIOS_ADD_CODE(X86Lock, 0);
      AIWNIOS_ADD_CODE(i_imm_sib, sz, rpn->integer, derefed.__SIB_scale,
                       derefed.reg2, derefed.reg, derefed.off);
      code_off = ICMov(cctrl, &rpn->res, &derefed, bin, code_off);
    } else if (ModeIsDerefToSIB(next)) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      if (rpn->flags & ICF_LOCK_EXPR)
        AIWNIOS_ADD_CODE(X86Lock, 0);
      AIWNIOS_ADD_CODE(incr_sib, sz, derefed.__SIB_scale, derefed.reg2,
                       derefed.reg, derefed.off);
      code_off = ICMov(cctrl, &rpn->res, &derefed, bin, code_off);
    } else {
      CICArg orig = {0};
      code_off = DerefToICArg(cctrl, &next->res, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      orig = next->res;
      tmp = next->res;
      code_off = PutICArgIntoReg(cctrl, &tmp, RT_I64i, RAX, bin, code_off);
      if (rpn->integer == 1) {
        AIWNIOS_ADD_CODE(incr, MIR(cctrl, tmp.reg));
      } else {
        AIWNIOS_ADD_CODE(i_imm, MIR(cctrl, tmp.reg), rpn->integer);
      }
      code_off = ICMov(cctrl, &orig, &tmp, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
  }
  return code_off;
}

static int64_t X86AndIndirXImm(char *to, int64_t sz, int64_t imm, int64_t s,
                               int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66);
  SIB_BEGIN(sz == 8, 4, s, i, b, o);
  if (sz == 1)
    ADD_U8(0x80)
  else
    ADD_U8(0x81)
  SIB_END();
  switch (sz) {
    break;
  case 1:
    ADD_U8(imm);
    break;
  case 2:
    ADD_U16(imm);
    break;
  case 4:
    ADD_U32(imm);
    break;
  case 8:
    ADD_U32(imm);
  }
  return len;
}

static int64_t X86XorIndirXImm(char *to, int64_t sz, int64_t imm, int64_t s,
                               int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66);
  SIB_BEGIN(sz == 8, 6, s, i, b, o);
  if (sz == 1)
    ADD_U8(0x80)
  else
    ADD_U8(0x81)
  SIB_END();
  switch (sz) {
    break;
  case 1:
    ADD_U8(imm);
    break;
  case 2:
    ADD_U16(imm);
    break;
  case 4:
    ADD_U32(imm);
    break;
  case 8:
    ADD_U32(imm);
  }
  return len;
}

static int64_t X86XorIndirXReg(char *to, int64_t sz, int64_t r, int64_t s,
                               int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66);
  SIB_BEGIN(sz == 8, r, s, i, b, o);
  ADD_U8(0x31);
  SIB_END();
  return len;
}

static int64_t X86AndIndirXReg(char *to, int64_t sz, int64_t r, int64_t s,
                               int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66);
  SIB_BEGIN(sz == 8, r, s, i, b, o);
  ADD_U8(0x21);
  SIB_END();
  return len;
}

static int64_t X86ShlIndirXImm(char *to, int64_t sz, int64_t amt, int64_t s,
                               int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  switch (sz) {
  case 1:
    SIB_BEGIN(sz == 8, 4, s, i, b, o);
    ADD_U8(0xc0);
    SIB_END();
    break;
  case 2:
    ADD_U8(0x66);
  case 4:
  case 8:
    SIB_BEGIN(sz == 8, 4, s, i, b, o);
    ADD_U8(0xc1);
    SIB_END();
  }
  ADD_U8(amt);
  return len;
}

static int64_t X86ShrIndirXImm(char *to, int64_t sz, int64_t amt, int64_t s,
                               int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  switch (sz) {
  case 1:
    SIB_BEGIN(sz == 8, 5, s, i, b, o);
    ADD_U8(0xc0);
    SIB_END();
    break;
  case 2:
    ADD_U8(0x66);
  case 4:
  case 8:
    SIB_BEGIN(sz == 8, 5, s, i, b, o);
    ADD_U8(0xc1);
    SIB_END();
  }
  ADD_U8(amt);
  return len;
}

static int64_t X86SarIndirXImm(char *to, int64_t sz, int64_t amt, int64_t s,
                               int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  switch (sz) {
  case 1:
    SIB_BEGIN(sz == 8, 7, s, i, b, o);
    ADD_U8(0xc0);
    SIB_END();
    break;
  case 2:
    ADD_U8(0x66);
  case 4:
  case 8:
    SIB_BEGIN(sz == 8, 7, s, i, b, o);
    ADD_U8(0xc1);
    SIB_END();
  }
  ADD_U8(amt);
  return len;
}
static int64_t X86OrIndirXReg(char *to, int64_t sz, int64_t r, int64_t s,
                              int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66);
  SIB_BEGIN(sz == 8, r, s, i, b, o);
  ADD_U8(0x09);
  SIB_END();
  return len;
}

static int64_t X86OrIndirXImm(char *to, int64_t sz, int64_t imm, int64_t s,
                              int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66);
  SIB_BEGIN(sz == 8, 1, s, i, b, o);
  if (sz == 1)
    ADD_U8(0x80)
  else
    ADD_U8(0x81)
  SIB_END();
  switch (sz) {
    break;
  case 1:
    ADD_U8(imm);
    break;
  case 2:
    ADD_U16(imm);
    break;
  case 4:
    ADD_U32(imm);
    break;
  case 8:
    ADD_U32(imm);
  }
  return len;
}

static int64_t __SexyAssignOp(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                              int64_t code_off) {
  CRPN *next = ICArgN(rpn, 1), *next2 = ICArgN(rpn, 0), *next_next;
  CICArg old = rpn->res, dummy = {0}, old2 = {0};
enter:;
  cctrl->backend_user_data9 = 1;
  int64_t old_type = rpn->type, od = cctrl->backend_user_data2,
          next_old = next->type, decr = 0, old_next2 = next2->type, pop = 0,
          use_tc = 0, use_raw_type = rpn->raw_type, use_f64 = 0,
          old_raw_type = rpn->raw_type, old_flags = next->flags,
          old_flags2 = next2->flags, set_flags = 0, old_flags3;
  if (next->type == IC_TYPECAST) {
    use_raw_type = next->raw_type;
    next = ICArgN(next, 0);
    use_tc = 1;
    goto enter;
  }
  switch (rpn->type) {
    break;
  case IC_ADD: // Will do assign on NORMAL tooo
  case IC_ADD_EQ:
    // Account for atomic/immediates
    if ((next->type == IC_DEREF || ModeIsDerefToSIB(next)) &&
        rpn->res.raw_type != RT_F64) {
      dummy = next->res;
      if (IsConst(next2) && Is32Bit(ConstVal(next2))) {
        code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                bin, code_off);
        if (rpn->flags & ICF_LOCK_EXPR)
          AIWNIOS_ADD_CODE(X86Lock, 0);
        switch (rpn->res.raw_type) {
        case RT_I8i:
        case RT_U8i:
          AIWNIOS_ADD_CODE(X86AddIndir8Imm32, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        case RT_I16i:
        case RT_U16i:
          AIWNIOS_ADD_CODE(X86AddIndir16Imm32, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        case RT_I32i:
        case RT_U32i:
          AIWNIOS_ADD_CODE(X86AddIndir32Imm32, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        default:
          AIWNIOS_ADD_CODE(X86AddIndir64Imm32, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        }
      } else if (rpn->res.raw_type != RT_I8i && rpn->res.raw_type != RT_U8i) {
        if (next->type == IC_DEREF) {
          if (next->res.mode == __MD_X86_64_SIB) {
            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                       AIWNIOS_TMP_IREG_POOP, bin, code_off);

          } else {
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);
          }
        } else {
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          code_off = __OptPassFinal(cctrl, next2, bin, code_off);
          old_flags3 = next->flags;
          next->flags |= ICF_PRECOMPUTED;
          code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                  bin, code_off);
          next->flags = old_flags3;
        }
        code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                   AIWNIOS_TMP_IREG_POOP, bin, code_off);
        switch (rpn->res.raw_type) {
        case RT_I8i:
        case RT_U8i:
          break;
        case RT_I16i:
        case RT_U16i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86AddIndir16Reg, next2->res.reg, dummy.__SIB_scale,
                           dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        case RT_I32i:
        case RT_U32i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86AddIndir32Reg, next2->res.reg, dummy.__SIB_scale,
                           dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        default:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86AddIndir64Reg, next2->res.reg, dummy.__SIB_scale,
                           dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        }
      }
    }
// PushTmp will set next->res
#define XXX_EQ_OP(op)                                                          \
  rpn->type = op;                                                              \
  use_f64 = next2->raw_type == RT_F64 || next->raw_type == RT_F64;             \
  switch (next->type) {                                                        \
  case IC_IREG:                                                                \
  case IC_FREG:                                                                \
  case IC_BASE_PTR:                                                            \
    rpn->res = next->res;                                                      \
    if (use_f64) {                                                             \
      if (next->res.raw_type != RT_F64) {                                      \
        rpn->res.raw_type = RT_F64;                                            \
        rpn->res.reg = 0;                                                      \
        rpn->res.mode = MD_REG;                                                \
      }                                                                        \
      rpn->raw_type = RT_F64;                                                  \
    }                                                                          \
    code_off = __OptPassFinal(cctrl, rpn, bin, code_off);                      \
    set_flags = rpn->res.set_flags;                                            \
    rpn->raw_type = old_raw_type;                                              \
    code_off = ICMov(cctrl, &next->res, &rpn->res, bin, code_off);             \
    code_off = ICMov(cctrl, &old, &rpn->res, bin, code_off);                   \
    break;                                                                     \
  case IC_DEREF:                                                               \
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);                    \
    code_off = __OptPassFinal(cctrl, next->base.next, bin, code_off);          \
    dummy = ((CRPN *)next->base.next)->res;                                    \
    old2 = dummy;                                                              \
    code_off =                                                                 \
        PutICArgIntoReg(cctrl, &next2->res, next2->res.raw_type,               \
                        use_f64 ? AIWNIOS_TMP_IREG_POOP2 : 1, bin, code_off);  \
    code_off = PutICArgIntoReg(cctrl, &dummy, RT_U64i, AIWNIOS_TMP_IREG_POOP,  \
                               bin, code_off);                                 \
    next->res.reg = dummy.reg;                                                 \
    next->res.mode = MD_INDIR_REG;                                             \
    next->res.off = 0;                                                         \
    code_off = PutICArgIntoReg(                                                \
        cctrl, &next->res, use_f64 ? RT_F64 : next->res.raw_type,              \
        use_f64 ? 0 : AIWNIOS_TMP_IREG_POOP, bin, code_off);                   \
    next->flags |= ICF_PRECOMPUTED;                                            \
    next2->flags |= ICF_PRECOMPUTED;                                           \
    rpn->res.mode = MD_REG;                                                    \
    rpn->res.reg = use_f64 ? 1 : RAX;                                          \
    if (use_f64)                                                               \
      rpn->raw_type = RT_F64;                                                  \
    rpn->res.raw_type = rpn->raw_type;                                         \
    code_off = __OptPassFinal(cctrl, rpn, bin, code_off);                      \
    set_flags = rpn->res.set_flags;                                            \
    rpn->raw_type = old_raw_type;                                              \
    next->flags &= ~ICF_PRECOMPUTED;                                           \
    next2->flags &= ~ICF_PRECOMPUTED;                                          \
    code_off = PutICArgIntoReg(cctrl, &old2, RT_PTR, RDX, bin, code_off);      \
    if (!use_tc)                                                               \
      old2.raw_type = next->raw_type;                                          \
    else                                                                       \
      old2.raw_type = use_raw_type;                                            \
    old2.mode = MD_INDIR_REG;                                                  \
    old2.off = 0;                                                              \
    code_off = ICMov(cctrl, &old2, &rpn->res, bin, code_off);                  \
    code_off = ICMov(cctrl, &old, &rpn->res, bin, code_off);                   \
    next->flags = old_flags, next2->flags = old_flags2;                        \
    break;                                                                     \
  default:                                                                     \
    rpn->res = next->res;                                                      \
    if (use_f64) {                                                             \
      if (next->res.raw_type != RT_F64) {                                      \
        rpn->res.raw_type = RT_F64;                                            \
        rpn->res.reg = 0;                                                      \
        rpn->res.mode = MD_REG;                                                \
      }                                                                        \
      rpn->raw_type = RT_F64;                                                  \
    }                                                                          \
    code_off = __OptPassFinal(cctrl, rpn, bin, code_off);                      \
    set_flags = rpn->res.set_flags;                                            \
    rpn->raw_type = old_raw_type;                                              \
    code_off = ICMov(cctrl, &next->res, &rpn->res, bin, code_off);             \
    code_off = ICMov(cctrl, &old, &rpn->res, bin, code_off);                   \
  }                                                                            \
  rpn->type = old_type, rpn->res = old;                                        \
  next->type = next_old;                                                       \
  next2->type = old_next2;
    XXX_EQ_OP(IC_ADD);
    break;
  case IC_SUB:
  case IC_SUB_EQ:
    // Account for atomic/immediates
    if ((next->type == IC_DEREF || ModeIsDerefToSIB(next)) &&
        rpn->res.raw_type != RT_F64) {
      dummy = next->res;
      if (IsConst(next2) && Is32Bit(ConstVal(next2))) {
        code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                bin, code_off);
        if (rpn->flags & ICF_LOCK_EXPR)
          AIWNIOS_ADD_CODE(X86Lock, 0);
        switch (rpn->res.raw_type) {
        case RT_I8i:
        case RT_U8i:
          AIWNIOS_ADD_CODE(X86SubIndir8Imm32, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        case RT_I16i:
        case RT_U16i:
          AIWNIOS_ADD_CODE(X86SubIndir16Imm32, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        case RT_I32i:
        case RT_U32i:
          AIWNIOS_ADD_CODE(X86SubIndir32Imm32, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        default:
          AIWNIOS_ADD_CODE(X86SubIndir64Imm32, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        }
      } else if (rpn->res.raw_type != RT_I8i && rpn->res.raw_type != RT_U8i) {
        if (next->type == IC_DEREF) {
          if (next->res.mode == __MD_X86_64_SIB) {
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                       AIWNIOS_TMP_IREG_POOP, bin, code_off);

            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);
          } else {
            old_flags3 = ((CRPN *)next->base.next)->flags;
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            code_off = __OptPassFinal(cctrl, next->base.next, bin, code_off);
            ((CRPN *)next->base.next)->flags |= ICF_PRECOMPUTED;
            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);
            ((CRPN *)next->base.next)->flags = old_flags3;
          }
        } else {
          code_off = __OptPassFinal(cctrl, next2, bin, code_off);
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          old_flags3 = next->flags;
          next->flags |= ICF_PRECOMPUTED;
          code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                  bin, code_off);
          next->flags = old_flags3;
        }
        code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                   AIWNIOS_TMP_IREG_POOP, bin, code_off);
        switch (rpn->res.raw_type) {
        case RT_I16i:
        case RT_U16i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86SubIndir16Reg, next2->res.reg, dummy.__SIB_scale,
                           dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        case RT_I32i:
        case RT_U32i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86SubIndir32Reg, next2->res.reg, dummy.__SIB_scale,
                           dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        default:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86SubIndir64Reg, next2->res.reg, dummy.__SIB_scale,
                           dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        }
      }
    }

    XXX_EQ_OP(IC_SUB);
    break;
  case IC_MUL:
  case IC_MUL_EQ:
    XXX_EQ_OP(IC_MUL);
    break;
  case IC_DIV:
  case IC_DIV_EQ:
    XXX_EQ_OP(IC_DIV);
    break;
  case IC_LSH:
  case IC_LSH_EQ:
    if (IsConst(next2) && (next->type == IC_DEREF || ModeIsDerefToSIB(next)) &&
        rpn->res.raw_type != RT_F64) {
      dummy = next->res;
      code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2, bin,
                              code_off);
      switch (rpn->res.raw_type) {
      case RT_I8i:
      case RT_U8i:
        AIWNIOS_ADD_CODE(X86ShlIndirXImm, 1, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      case RT_I16i:
      case RT_U16i:
        AIWNIOS_ADD_CODE(X86ShlIndirXImm, 2, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      case RT_I32i:
      case RT_U32i:
        AIWNIOS_ADD_CODE(X86ShlIndirXImm, 4, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      default:
        AIWNIOS_ADD_CODE(X86ShlIndirXImm, 8, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
      }
      code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
      goto fin;
    }
    XXX_EQ_OP(IC_LSH);
    break;
  case IC_RSH:
  case IC_RSH_EQ:
    if (IsConst(next2) && (next->type == IC_DEREF || ModeIsDerefToSIB(next)) &&
        rpn->res.raw_type != RT_F64) {
      dummy = next->res;
      code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2, bin,
                              code_off);
      switch (rpn->res.raw_type) {
      case RT_I8i:
        AIWNIOS_ADD_CODE(X86SarIndirXImm, 1, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      case RT_U8i:
        AIWNIOS_ADD_CODE(X86ShrIndirXImm, 1, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      case RT_I16i:
        AIWNIOS_ADD_CODE(X86SarIndirXImm, 2, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      case RT_U16i:
        AIWNIOS_ADD_CODE(X86ShrIndirXImm, 2, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      case RT_I32i:
        AIWNIOS_ADD_CODE(X86SarIndirXImm, 4, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      case RT_U32i:
        AIWNIOS_ADD_CODE(X86ShrIndirXImm, 4, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      case RT_I64i:
        AIWNIOS_ADD_CODE(X86SarIndirXImm, 8, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
        break;
      case RT_U64i:
        AIWNIOS_ADD_CODE(X86ShrIndirXImm, 8, ConstVal(next2), dummy.__SIB_scale,
                         dummy.reg2, dummy.reg, dummy.off);
      }
      code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
      goto fin;
    }
    XXX_EQ_OP(IC_RSH);
    break;
  case IC_AND:
  case IC_AND_EQ:
    // Account for atomic/immediates
    if ((next->type == IC_DEREF || ModeIsDerefToSIB(next)) &&
        rpn->res.raw_type != RT_F64) {
      dummy = next->res;
      if (IsConst(next2) && Is32Bit(ConstVal(next2))) {
        code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                bin, code_off);
        if (rpn->flags & ICF_LOCK_EXPR)
          AIWNIOS_ADD_CODE(X86Lock, 0);
        switch (rpn->res.raw_type) {
        case RT_I8i:
        case RT_U8i:
          AIWNIOS_ADD_CODE(X86AndIndirXImm, 1, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          break;
        case RT_I16i:
        case RT_U16i:
          AIWNIOS_ADD_CODE(X86AndIndirXImm, 2, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          break;
        case RT_I32i:
        case RT_U32i:
          AIWNIOS_ADD_CODE(X86AndIndirXImm, 4, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          break;
        default:
          AIWNIOS_ADD_CODE(X86AndIndirXImm, 8, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
        }
        code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
        goto fin;
      } else if (rpn->res.raw_type != RT_I8i && rpn->res.raw_type != RT_U8i) {
        if (next->type == IC_DEREF) {
          if (next->res.mode == __MD_X86_64_SIB) {
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                       AIWNIOS_TMP_IREG_POOP, bin, code_off);
            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);

          } else {
            old_flags3 = ((CRPN *)next->base.next)->flags;
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            code_off = __OptPassFinal(cctrl, next->base.next, bin, code_off);
            ((CRPN *)next->base.next)->flags |= ICF_PRECOMPUTED;
            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);
            ((CRPN *)next->base.next)->flags = old_flags3;
          }
        } else {
          code_off = __OptPassFinal(cctrl, next2, bin, code_off);
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          old_flags3 = next->flags;
          next->flags |= ICF_PRECOMPUTED;
          code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                  bin, code_off);
          next->flags = old_flags3;
        }
        code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                   AIWNIOS_TMP_IREG_POOP, bin, code_off);
        switch (rpn->res.raw_type) {
        case RT_I16i:
        case RT_U16i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86AndIndirXReg, 2, next2->res.reg,
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        case RT_I32i:
        case RT_U32i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86AndIndirXReg, 4, next2->res.reg,
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        default:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86AndIndirXReg, 8, next2->res.reg,
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        }
      }
    }
    XXX_EQ_OP(IC_AND);
    break;
  case IC_OR:
  case IC_OR_EQ:
    // Account for atomic/immediates
    if ((next->type == IC_DEREF || ModeIsDerefToSIB(next)) &&
        rpn->res.raw_type != RT_F64) {
      dummy = next->res;
      if (IsConst(next2) && Is32Bit(ConstVal(next2))) {
        code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                bin, code_off);
        if (rpn->flags & ICF_LOCK_EXPR)
          AIWNIOS_ADD_CODE(X86Lock, 0);
        switch (rpn->res.raw_type) {
        case RT_I8i:
        case RT_U8i:
          AIWNIOS_ADD_CODE(X86OrIndirXImm, 1, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          break;
        case RT_I16i:
        case RT_U16i:
          AIWNIOS_ADD_CODE(X86OrIndirXImm, 2, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          break;
        case RT_I32i:
        case RT_U32i:
          AIWNIOS_ADD_CODE(X86OrIndirXImm, 4, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          break;
        default:
          AIWNIOS_ADD_CODE(X86OrIndirXImm, 8, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
        }
        code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
        goto fin;
      } else if (rpn->res.raw_type != RT_I8i && rpn->res.raw_type != RT_U8i) {
        if (next->type == IC_DEREF) {
          if (next->res.mode == __MD_X86_64_SIB) {
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);
            code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                       AIWNIOS_TMP_IREG_POOP, bin, code_off);

          } else {
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            old_flags3 = ((CRPN *)next->base.next)->flags;
            code_off = __OptPassFinal(cctrl, next->base.next, bin, code_off);
            ((CRPN *)next->base.next)->flags |= ICF_PRECOMPUTED;
            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);
            ((CRPN *)next->base.next)->flags = old_flags3;
          }
        } else {
          code_off = __OptPassFinal(cctrl, next2, bin, code_off);
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          old_flags3 = next->flags;
          next->flags |= ICF_PRECOMPUTED;
          code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                  bin, code_off);
          next->flags = old_flags3;
        }
        code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                   AIWNIOS_TMP_IREG_POOP, bin, code_off);
        switch (rpn->res.raw_type) {
        case RT_I16i:
        case RT_U16i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86OrIndirXReg, 2, next2->res.reg, dummy.__SIB_scale,
                           dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        case RT_I32i:
        case RT_U32i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86OrIndirXReg, 4, next2->res.reg, dummy.__SIB_scale,
                           dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        default:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86OrIndirXReg, 8, next2->res.reg, dummy.__SIB_scale,
                           dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        }
        abort();
      }
    }
    XXX_EQ_OP(IC_OR);
    break;
  case IC_XOR:
  case IC_XOR_EQ:
    if ((next->type == IC_DEREF || ModeIsDerefToSIB(next)) &&
        rpn->res.raw_type != RT_F64) {
      dummy = next->res;
      if (IsConst(next2) && Is32Bit(ConstVal(next2))) {
        code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                bin, code_off);
        if (rpn->flags & ICF_LOCK_EXPR)
          AIWNIOS_ADD_CODE(X86Lock, 0);
        switch (rpn->res.raw_type) {
        case RT_I8i:
        case RT_U8i:
          AIWNIOS_ADD_CODE(X86XorIndirXImm, 1, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          break;
        case RT_I16i:
        case RT_U16i:
          AIWNIOS_ADD_CODE(X86XorIndirXImm, 2, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          break;
        case RT_I32i:
        case RT_U32i:
          AIWNIOS_ADD_CODE(X86XorIndirXImm, 4, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          break;
        default:
          AIWNIOS_ADD_CODE(X86XorIndirXImm, 8, ConstVal(next2),
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
        }
        code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
        goto fin;
      } else if (rpn->res.raw_type != RT_I8i && rpn->res.raw_type != RT_U8i) {
        if (next->type == IC_DEREF) {
          if (next->res.mode == __MD_X86_64_SIB) {
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);
            code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                       AIWNIOS_TMP_IREG_POOP, bin, code_off);

          } else {
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            old_flags3 = ((CRPN *)next->base.next)->flags;
            code_off = __OptPassFinal(cctrl, next->base.next, bin, code_off);
            ((CRPN *)next->base.next)->flags |= ICF_PRECOMPUTED;
            code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                    bin, code_off);
            ((CRPN *)next->base.next)->flags = old_flags3;
          }
        } else {
          code_off = __OptPassFinal(cctrl, next2, bin, code_off);
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          old_flags3 = next->flags;
          next->flags |= ICF_PRECOMPUTED;
          code_off = DerefToICArg(cctrl, &dummy, next, AIWNIOS_TMP_IREG_POOP2,
                                  bin, code_off);
          next->flags = old_flags3;
        }
        code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                   AIWNIOS_TMP_IREG_POOP, bin, code_off);
        switch (rpn->res.raw_type) {
        case RT_I16i:
        case RT_U16i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86XorIndirXReg, 2, next2->res.reg,
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        case RT_I32i:
        case RT_U32i:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86XorIndirXReg, 4, next2->res.reg,
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        default:
          if (rpn->flags & ICF_LOCK_EXPR)
            AIWNIOS_ADD_CODE(X86Lock, 0);
          AIWNIOS_ADD_CODE(X86XorIndirXReg, 8, next2->res.reg,
                           dummy.__SIB_scale, dummy.reg2, dummy.reg, dummy.off);
          code_off = ICMov(cctrl, &rpn->res, &dummy, bin, code_off);
          goto fin;
        }
      }
    }
    XXX_EQ_OP(IC_XOR);
    break;
  case IC_MOD:
  case IC_MOD_EQ:
    XXX_EQ_OP(IC_MOD);
  }
fin:
  if (set_flags)
    rpn->res.set_flags = 1;
  cctrl->backend_user_data9 = 0;
  return code_off;
}

static int64_t IsSavedIReg(int64_t r) {
  switch (r) {
  case RSI:
  case RDI:
  case RSP:
  case RBP:
  case R10:
  case R11:
  case R12:
  case R13:
  case R14:
  case R15:;
    return 1;
  }
  return 0;
}

// This is used for FuncProlog/Epilog. It is used for finding the non-violatle
// registers that must be saved and pushed to the stack
//
// to_push is a char[16] ,1 is push,0 is not push
//
// Returns number of pushed regs
//
static int64_t __FindPushedIRegs(CCmpCtrl *cctrl, char *to_push) {
  CMemberLst *lst;
  CRPN *r;
  int64_t cnt = 0;
  int64_t idx = 0;
  memset(to_push, 0, 16);
  if (!cctrl->cur_fun) {
    if (!cctrl->found_used_iregs) {
      // Perhaps we are being called from HolyC and we aren't using a "cur_fun"
      for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
           r = r->base.next) {
        if (r->type == IC_IREG && r->raw_type != RT_F64) {
          if (!to_push[r->integer])
            if (IsSavedIReg(r->integer)) {
              Misc_Bts(&cctrl->used_iregs_bmp, r->integer);
              to_push[r->integer] = 1;
              cnt++;
            }
        }
      }
      cctrl->found_used_iregs = 1;
    } else {
      for (idx = 0; idx != 16; idx++) {
        if (Misc_Bt(&cctrl->used_iregs_bmp, idx)) {
          to_push[idx] = 1;
          cnt++;
        }
      }
    }
    return cnt;
  }
  for (lst = cctrl->cur_fun->base.members_lst; lst; lst = lst->next) {
    if (lst->reg != REG_NONE && lst->member_class->raw_type != RT_F64) {
      assert(lst->reg >= 0);
      to_push[lst->reg] = 1;
      cnt++;
    }
  }
  return cnt;
}

static int64_t __FindPushedFRegs(CCmpCtrl *cctrl, char *to_push) {
  memset(to_push, 0, 16);
  CMemberLst *lst;
  int64_t cnt = 0;
  int64_t idx;
  CRPN *r;
  if (!cctrl->cur_fun) {
    // Perhaps we are being called from HolyC and we aren't using a "cur_fun"
    if (!cctrl->found_used_fregs) {
      cctrl->found_used_fregs = 1;
      for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
           r = r->base.next) {
        if (r->type == IC_FREG && r->raw_type == RT_F64) {
          if (!to_push[r->integer])
            if (IsSavedFReg(r->integer)) {
              to_push[r->integer] = 1;
              Misc_Bts(&cctrl->backend_user_data8, r->integer);
              Misc_Bts(&cctrl->used_fregs_bmp, r->integer);
              cnt++;
            }
        }
      }
    } else {
      for (idx = 0; idx != 16; idx++) {
        if (Misc_Bt(&cctrl->used_fregs_bmp, idx)) {
          to_push[idx] = 1;
          cnt++;
        }
      }
    }
    return cctrl->backend_user_data7 = cnt;
  }
  for (lst = cctrl->cur_fun->base.members_lst; lst; lst = lst->next) {
    if (lst->reg != REG_NONE && lst->member_class->raw_type == RT_F64) {
      assert(lst->reg >= 0);
      to_push[lst->reg] = 1;
      Misc_Bts(&cctrl->backend_user_data8, lst->reg);
      cnt++;
    }
  }
  return cctrl->backend_user_data7 = cnt;
}

// Add 1 in first bit for oppositive
#define X86_COND_A  0x10
#define X86_COND_AE 0x20
#define X86_COND_B  0x30
#define X86_COND_BE 0x40
#define X86_COND_C  0x50
#define X86_COND_E  0x60
#define X86_COND_Z  X86_COND_E
#define X86_COND_G  0x70
#define X86_COND_GE 0x80
#define X86_COND_L  0x90
#define X86_COND_LE 0xa0
#define X86_COND_S  0xb0
#define X86_COND_NS 0xb1
static int64_t X86Jcc(char *to, int64_t cond, int64_t off) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  ADD_U8(0x0f);
  switch (cond) {
    break;
  case X86_COND_A:
    ADD_U8(0x87);
    break;
  case X86_COND_AE:
    ADD_U8(0x83);
    break;
  case X86_COND_B:
    ADD_U8(0x82);
    break;
  case X86_COND_BE:
    ADD_U8(0x86);
    break;
  case X86_COND_C:
    ADD_U8(0x82);
    break;
  case X86_COND_E:
    ADD_U8(0x84);
    break;
  case X86_COND_G:
    ADD_U8(0x8f);
    break;
  case X86_COND_GE:
    ADD_U8(0x8d);
    break;
  case X86_COND_L:
    ADD_U8(0x8c);
    break;
  case X86_COND_LE:
    ADD_U8(0x8e);
    break;
  case X86_COND_S:
    ADD_U8(0x88);
    break;
  case X86_COND_S | 1:
    ADD_U8(0x89);
    break;
  case X86_COND_A | 1:
    ADD_U8(0x86);
    break;
  case X86_COND_AE | 1:
    ADD_U8(0x82);
    break;
  case X86_COND_B | 1:
    ADD_U8(0x83);
    break;
  case X86_COND_BE | 1:
    ADD_U8(0x87);
    break;
  case X86_COND_C | 1:
    ADD_U8(0x83);
    break;
  case X86_COND_E | 1:
    ADD_U8(0x85);
    break;
  case X86_COND_G | 1:
    ADD_U8(0x8e);
    break;
  case X86_COND_GE | 1:
    ADD_U8(0x8c);
    break;
  case X86_COND_L | 1:
    ADD_U8(0x8d);
    break;
  case X86_COND_LE | 1:
    ADD_U8(0x8f);
  }
  ADD_U32(off - 6); //-6 as inst is 6 bytes long(RIP comes after INST);
  return len;
}
// 1st MOV RDI,1
//...
// 6th MOV R8,6
// PUSH 7
// CALL FOO
// ADD RSP,8 //To remove 7 from the stack

static int64_t X86Setcc(char *to, int64_t cond, int64_t r) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if ((r & 0b1000) || r == RSI || r == RDI)
    ADD_U8(ErectREX(0, 0, 0, r))
  ADD_U8(0x0f);
  switch (cond) {
    break;
  case X86_COND_A:
    ADD_U8(0x97);
    break;
  case X86_COND_AE:
    ADD_U8(0x93);
    break;
  case X86_COND_B:
    ADD_U8(0x92);
    break;
  case X86_COND_BE:
    ADD_U8(0x96);
    break;
  case X86_COND_C:
    ADD_U8(0x92);
    break;
  case X86_COND_E:
    ADD_U8(0x94);
    break;
  case X86_COND_G:
    ADD_U8(0x9f);
    break;
  case X86_COND_GE:
    ADD_U8(0x9d);
    break;
  case X86_COND_L:
    ADD_U8(0x9c);
    break;
  case X86_COND_LE:
    ADD_U8(0x9e);

    break;
  case X86_COND_A | 1:
    ADD_U8(0x96);
    break;
  case X86_COND_AE | 1:
    ADD_U8(0x92);
    break;
  case X86_COND_B | 1:
    ADD_U8(0x93);
    break;
  case X86_COND_BE | 1:
    ADD_U8(0x97);
    break;
  case X86_COND_C | 1:
    ADD_U8(0x93);
    break;
  case X86_COND_E | 1:
    ADD_U8(0x95);
    break;
  case X86_COND_G | 1:
    ADD_U8(0x9e);
    break;
  case X86_COND_GE | 1:
    ADD_U8(0x9c);
    break;
  case X86_COND_L | 1:
    ADD_U8(0x9d);
    break;
  case X86_COND_LE | 1:
    ADD_U8(0x9f);
  }
  ADD_U8(MODRMRegReg(0, r));
  if (!to)
    len += X86MovZXRegRegI8(NULL, r, r);
  else
    len += X86MovZXRegRegI8(to + len, r, r);
  return len;
}

static int64_t FuncProlog(CCmpCtrl *cctrl, char *bin, int64_t code_off) {
  char push_ireg[16];
  char push_freg[16];
  char ilist[16];
  char flist[16];
  int64_t i, i2, i3, r, r2, ireg_arg_cnt, freg_arg_cnt, stk_arg_cnt, fsz, code,
      off;
  int64_t has_vargs = 0, arg_cnt = 0;
  CMemberLst *lst;
  CICArg fun_arg = {0}, write_to = {0}, stack = {0}, tmp = {0};
  CRPN *rpn, *arg;
  if (cctrl->cur_fun) {
    fsz = cctrl->cur_fun->base.sz + 16; //+16 for RBP/return address
    arg_cnt = cctrl->cur_fun->argc;
    if (cctrl->cur_fun->base.flags & CLSF_VARGS)
      arg_cnt--; // argv
  } else {
    fsz = 16;
    for (rpn = cctrl->code_ctrl->ir_code->next;
         rpn != cctrl->code_ctrl->ir_code; rpn = rpn->base.next) {
      switch (rpn->type) {
      case __IC_SET_FRAME_SIZE:
        fsz = rpn->integer;
        break;
      case IC_GET_VARGS_PTR:
        has_vargs = 1;
        break;
      case __IC_ARG:
        arg_cnt++;
        break;
      }
    }
  }
  // ALIGN TO 8
  i2 = __FindPushedIRegs(cctrl, push_ireg);
  i3 = __FindPushedFRegs(cctrl, push_freg);
  int64_t to_push = cctrl->backend_user_data0 + fsz, old_regs_start;
  cctrl->backend_user_data4 = fsz;
  old_regs_start = fsz + cctrl->backend_user_data0;

  i2 = 0;
  for (i = 0; i != 16; i++)
    if (push_ireg[i])
      ilist[i2++] = i;
  i3 = 0;
  for (i = 0; i != 16; i++)
    if (push_freg[i])
      flist[i3++] = i;
  AIWNIOS_ADD_CODE(X86PushReg, RBP);
  AIWNIOS_ADD_CODE(X86MovRegReg, RBP, RSP);
  if (to_push - 16 > 0)
    AIWNIOS_ADD_CODE(X86SubImm32, RSP, to_push);
  off = -old_regs_start;
  for (i = 0; i != i2; i++) {
    AIWNIOS_ADD_CODE(X86PushReg, ilist[i]);
    off -= 8;
  }
  if (i3) {
    AIWNIOS_ADD_CODE(X86SubImm32, RSP, i3 * 8)
    for (i = 0; i != i3; i++) {
      AIWNIOS_ADD_CODE(X86MovIndirRegF64, flist[i], -1, -1, RSP, i * 8);
    }
  }
  stk_arg_cnt = ireg_arg_cnt = freg_arg_cnt = 0;
  if (cctrl->cur_fun) {
    lst = cctrl->cur_fun->base.members_lst;
    for (i = 0; i != cctrl->cur_fun->argc; i++) {
      fun_arg.mode = MD_INDIR_REG;
      fun_arg.raw_type = lst->member_class->raw_type;
      fun_arg.reg = RBP;
      fun_arg.off = 16 + stk_arg_cnt++ * 8;
      // This *shoudlnt* mutate any of the argument registers
      if (lst->reg >= 0 && lst->reg != REG_NONE) {
        write_to.mode = MD_REG;
        write_to.reg = lst->reg;
        write_to.off = 0;
        write_to.raw_type = lst->member_class->raw_type;
      } else {
        write_to.mode = MD_FRAME;
        write_to.off = lst->off;
        write_to.raw_type = lst->member_class->raw_type;
      }
      if ((cctrl->cur_fun->base.flags & CLSF_VARGS) &&
          !strcmp("argv", lst->str)) {
        AIWNIOS_ADD_CODE(X86LeaSIB, 0, -1, -1, RBP, 16 + arg_cnt * 8);
        fun_arg.reg = 0;
        fun_arg.mode = MD_REG;
        fun_arg.raw_type = RT_I64i;
      }
      code_off = ICMov(cctrl, &write_to, &fun_arg, bin, code_off);
      lst = lst->next;
    }
  } else {
    // Things from the HolyC side use __IC_ARG
    // We go backwards as this is REVERSE polish notation
    for (i = 0, rpn = cctrl->code_ctrl->ir_code->last;
         rpn != cctrl->code_ctrl->ir_code; rpn = rpn->base.last) {
      if (rpn->type == __IC_ARG) {
        fun_arg.mode = MD_INDIR_REG;
        fun_arg.raw_type =
            (arg = ICArgN(rpn, 0))->raw_type == RT_F64 ? RT_F64 : RT_I64i;
        fun_arg.reg = RBP;
        fun_arg.off = 16 + stk_arg_cnt++ * 8;
        PushTmp(cctrl, arg);
        PopTmp(cctrl, arg);
        if (fun_arg.off == -arg->res.off && arg->res.mode == MD_FRAME)
          continue;
        code_off = ICMov(cctrl, &arg->res, &fun_arg, bin, code_off);
      } else if (rpn->type == IC_GET_VARGS_PTR) {
        arg = ICArgN(rpn, 0);
        PushTmp(cctrl, arg);
        PopTmp(cctrl, arg);
        if (arg->res.mode == MD_REG) {
          AIWNIOS_ADD_CODE(X86LeaSIB, arg->res.reg, -1, -1, RBP,
                           16 + arg_cnt * 8);
        } else {
          AIWNIOS_ADD_CODE(X86LeaSIB, 0, -1, -1, RBP, 16 + arg_cnt * 8);
          tmp.reg = 0;
          tmp.mode = MD_REG;
          tmp.raw_type = RT_I64i;
          code_off = ICMov(cctrl, &arg->res, &tmp, bin, code_off);
        }
      }
    }
  }
  return code_off;
}

//
// DO NOT CHANGE WITHOUT LOOKING CLOSEY AT FuncProlog
//
static int64_t FuncEpilog(CCmpCtrl *cctrl, char *bin, int64_t code_off) {
  char push_ireg[16], ilist[16];
  char push_freg[16], flist[16];
  int64_t i, r, r2, fsz, i2, i3, off, is_vargs = 0, arg_cnt = 0;
  int64_t fsave_area;
  CICArg spill_loc = {0}, write_to = {0};
  CRPN *rpn;
  /* <== OLD SP
   * saved regs
   * wiggle room
   * locals
   * OLD FP,LR<===FP=SP
   */
  if (cctrl->cur_fun) {
    fsz = cctrl->cur_fun->base.sz + 16; //+16 for LR/FP
    arg_cnt = cctrl->cur_fun->argc;
    is_vargs = !!(cctrl->cur_fun->base.flags & CLSF_VARGS);
  } else {
    fsz = 16;
    for (rpn = cctrl->code_ctrl->ir_code->next;
         rpn != cctrl->code_ctrl->ir_code; rpn = rpn->base.next) {
      if (rpn->type == __IC_SET_FRAME_SIZE) {
        fsz = rpn->integer;
        break;
      } else if (rpn->type == IC_GET_VARGS_PTR) {
        is_vargs = 1;
      } else if (rpn->type == __IC_ARG) {
        arg_cnt++;
      }
    }
  }
  // ALIGN TO 8
  i2 = __FindPushedIRegs(cctrl, push_ireg);
  i3 = __FindPushedFRegs(cctrl, push_freg);
  int64_t to_push = fsz + cctrl->backend_user_data0 + 16,
          old_regs_start; // old_FP,old_LR
  old_regs_start = fsz + cctrl->backend_user_data0;
  i2 = 0;
  for (i = 0; i != 16; i++)
    if (push_ireg[i])
      ilist[i2++] = i;
  i3 = 0;
  for (i = 0; i != 16; i++)
    if (push_freg[i])
      flist[i3++] = i;
  //<==== OLD SP
  // first saved reg pair<==-16
  // next saved reg pair<===-32
  //
  fsave_area = -old_regs_start - 8 * i2;
  off = 0;
  for (i = 0; i != i3; i++) {
    AIWNIOS_ADD_CODE(X86MovRegIndirF64, flist[i], -1, -1, RSP, off);
    off += 8;
  }
  if (off) {
    AIWNIOS_ADD_CODE(X86AddImm32, RSP, off);
  }
  for (i = i2 - 1; i >= 0; i--) {
    AIWNIOS_ADD_CODE(X86PopReg, ilist[i]);
  }

  off = -old_regs_start;
  AIWNIOS_ADD_CODE(X86Leave, 0);
  if (is_vargs) {
    AIWNIOS_ADD_CODE(X86Ret, 0);
  } else {
    AIWNIOS_ADD_CODE(X86Ret, 8 * arg_cnt);
  }
  return code_off;
}
static int64_t X86IncIndirX(char *to, int64_t sz, int64_t s, int64_t i,
                            int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66)
  SIB_BEGIN(sz == 8, 0, s, i, b, o);
  if (sz == 1) {
    ADD_U8(0xfe);
  } else
    ADD_U8(0xff)
  SIB_END();
  return len;
}
static int64_t X86DecIndirX(char *to, int64_t sz, int64_t s, int64_t i,
                            int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66)
  SIB_BEGIN(sz == 8, 1, s, i, b, o);
  if (sz == 1) {
    ADD_U8(0xfe);
  } else
    ADD_U8(0xff)
  SIB_END();
  return len;
}
static int64_t X86AddIndirXImm32(char *to, int64_t sz, int64_t imm, int64_t s,
                                 int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66)
  SIB_BEGIN(sz == 8, 0, s, i, b, o);
  if (sz == 1) {
    ADD_U8(0x80);
  } else
    ADD_U8(0x81)
  SIB_END();
  switch (sz) {
  case 1:
    ADD_U8(imm);
    break;
  case 2:
    ADD_U16(imm);
    break;
  default:
    ADD_U32(imm);
    break;
  }
  return len;
}

static int64_t X86SubIndirXImm32(char *to, int64_t sz, int64_t imm, int64_t s,
                                 int64_t i, int64_t b, int64_t o) {
  int64_t len = 0;
  char buf[16];
  if (!to)
    to = buf;
  if (sz == 2)
    ADD_U8(0x66)
  SIB_BEGIN(sz == 8, 5, s, i, b, o);
  if (sz == 1) {
    ADD_U8(0x80);
  } else
    ADD_U8(0x81)
  SIB_END();
  switch (sz) {
  case 1:
    ADD_U8(imm);
    break;
  case 2:
    ADD_U16(imm);
    break;
  default:
    ADD_U32(imm);
    break;
  }
  return len;
}

static int64_t __SexyPostOp(
    CCmpCtrl *cctrl, CRPN *rpn, int64_t (*i_imm)(char *, int64_t, int64_t),
    int64_t (*ireg)(char *, int64_t, int64_t),
    int64_t (*freg)(char *, int64_t, int64_t), int64_t (*incr)(char *, int64_t),
    int64_t (*incr_sib)(char *, int64_t, int64_t, int64_t, int64_t, int64_t),
    int64_t (*i_imm_sib)(char *, int64_t, int64_t, int64_t, int64_t, int64_t,
                         int64_t),
    char *bin, int64_t code_off) {
  //
  // See hack in PushTmpDepthFirst motherfucker
  //
  CRPN *next = rpn->base.next, *tc;
  int64_t code;
  int64_t sz;
  CICArg derefed = {0}, tmp = {0}, tmp2 = {0};
  if (next->type == IC_TYPECAST) {
    TYPECAST_ASSIGN_BEGIN(next, next);
    switch (next->raw_type) {
    case RT_I8i:
    case RT_U8i:
      sz = 1;
      break;
    case RT_I16i:
    case RT_U16i:
      sz = 2;
      break;
    case RT_I32i:
    case RT_U32i:
      sz = 4;
      break;
    default:
      sz = 8;
      break;
    }
    if (next->raw_type == RT_F64) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      tmp = derefed; //==RT_F64
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 1, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      code_off = __ICMoveF64(cctrl, 2, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(freg, MFR(cctrl, tmp.reg), 2);
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
    } else if (rpn->integer != 1) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      tmp = derefed;
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type,
                                 AIWNIOS_TMP_IREG_POOP, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      AIWNIOS_ADD_CODE(i_imm, MIR(cctrl, tmp.reg), rpn->integer);
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
    } else if (ModeIsDerefToSIB(next)) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &derefed, bin, code_off);
      if (rpn->flags & ICF_LOCK_EXPR)
        AIWNIOS_ADD_CODE(X86Lock, 0);
      AIWNIOS_ADD_CODE(incr_sib, sz, derefed.__SIB_scale, derefed.reg2,
                       derefed.reg, derefed.off);
    } else {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      tmp = derefed;
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type,
                                 AIWNIOS_TMP_IREG_POOP, bin, code_off);
      AIWNIOS_ADD_CODE(incr, MIR(cctrl, tmp.reg));
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    TYPECAST_ASSIGN_END(next);
  } else {
    switch (next->raw_type) {
    case RT_I8i:
    case RT_U8i:
      sz = 1;
      break;
    case RT_I16i:
    case RT_U16i:
      sz = 2;
      break;
    case RT_I32i:
    case RT_U32i:
      sz = 4;
      break;
    default:
      sz = 8;
      break;
    }
    if (next->raw_type == RT_F64) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      tmp = derefed; //==RT_F64
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type, 1, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      code_off = __ICMoveF64(cctrl, 2, rpn->integer, bin, code_off);
      AIWNIOS_ADD_CODE(freg, MFR(cctrl, tmp.reg), 2);
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
    } else if (rpn->integer != 1 && ModeIsDerefToSIB(next)) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      tmp = derefed;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      if (rpn->flags & ICF_LOCK_EXPR)
        AIWNIOS_ADD_CODE(X86Lock, 0);
      AIWNIOS_ADD_CODE(i_imm_sib, sz, rpn->integer, derefed.__SIB_scale,
                       derefed.reg2, derefed.reg, derefed.off);
    } else if (ModeIsDerefToSIB(next) && rpn->integer == 1) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &derefed, bin, code_off);
      if (rpn->flags & ICF_LOCK_EXPR)
        AIWNIOS_ADD_CODE(X86Lock, 0);
      AIWNIOS_ADD_CODE(incr_sib, sz, derefed.__SIB_scale, derefed.reg2,
                       derefed.reg, derefed.off);
    } else {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      tmp = derefed;
      code_off = PutICArgIntoReg(cctrl, &tmp, tmp.raw_type,
                                 AIWNIOS_TMP_IREG_POOP, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      if (rpn->integer == 1)
        AIWNIOS_ADD_CODE(incr, MIR(cctrl, tmp.reg))
      else
        AIWNIOS_ADD_CODE(i_imm, MIR(cctrl, tmp.reg), rpn->integer);
      code_off = ICMov(cctrl, &derefed, &tmp, bin, code_off);
    }
  }
  return code_off;
}

static int64_t SEG_GS(char *to, int64_t dummy) {
  char buf[16];
  if (!to)
    to = buf;
  to[0] = 0x65;
  return 1;
}
static int64_t SEG_FS(char *to, int64_t dummy) {
  char buf[16];
  if (!to)
    to = buf;
  to[0] = 0x64;
  return 1;
}
static int64_t MovRAXMoffs32(char *to, int64_t off) {
  char buf[16];
  int64_t len = 0;
  if (!to)
    to = buf;
  ADD_U8(0x48);
  ADD_U8(0xa1);
  ADD_U64(off);
  return 2 + 8;
}

static int64_t IsCompoundCompare(CRPN *r) {
  CRPN *next = r->base.next;
  switch (r->type) {
  case IC_LT:
  case IC_GT:
  case IC_LE:
  case IC_GE:
    next = ICFwd(next);
    switch (next->type) {
    case IC_LT:
    case IC_GT:
    case IC_LE:
    case IC_GE:
      return 1;
    }
    return 0;
    /*case IC_NE:
    case IC_EQ_EQ:
      next = ICFwd(next);
      switch (next->type) {
      case IC_EQ_EQ:
      case IC_NE:
        return 1;
      }
      return 0;
      */
  }
  return 0;
}
static int64_t CanKeepInTmp(CRPN *me, CRPN *have, CRPN *other,
                            int64_t is_left_side) {
  uint64_t mask;
  if (!(have->tmp_res.mode == MD_REG))
    return 0; // No need to stuff in tmp
  if (is_left_side) {
    if (other)
      mask = other->changes_iregs2 | me->changes_iregs;
    else
      mask = me->changes_iregs;
  } else {
    mask = me->changes_iregs;
  }
  if (have->tmp_res.mode == MD_REG && have->tmp_res.raw_type != RT_F64)
    if (mask & (1ull << have->tmp_res.reg))
      return 0;
  if (have->tmp_res.mode == __MD_X86_64_LEA_SIB ||
      have->tmp_res.mode == __MD_X86_64_SIB) {
    if ((mask & (1ull << have->tmp_res.reg)) ||
        (mask & (1ull << have->tmp_res.reg2)) ||
        (mask & (1ull << have->tmp_res.fallback_reg)))
      return 0;
  }

  if (is_left_side) {
    if (other)
      mask = other->changes_fregs2 | me->changes_fregs;
    else
      mask = me->changes_fregs;
  } else {
    mask = me->changes_fregs;
  }
  if (have->tmp_res.mode == MD_REG && have->tmp_res.raw_type == RT_F64)
    if (mask & (1ull << have->tmp_res.reg))
      return 0;

  return 1;
}
//
// Sets keep_in_tmp
//
static void SetKeepTmps(CRPN *rpn) {
  int64_t idx;
  CRPN *left, *right, *last;
  switch (rpn->type) {
  case IC_FS:
  case IC_GS:
    return;
  case __IC_CALL:
  case IC_CALL:
    for (idx = 0; idx != rpn->length; idx++) {
      right = ICArgN(rpn, idx);
      if (right->tmp_res.mode) {
        right->res = right->tmp_res;
        right->res.keep_in_tmp = 1;
      }
    }
    break;
  case __IC_VARGS:
    for (idx = 0; idx != rpn->length; idx++) {
      right = ICArgN(rpn, idx);
      if (right->tmp_res.mode) {
        right->res = right->tmp_res;
        right->res.keep_in_tmp = 1;
      }
    }
    return;
  case IC_RET:
  case IC_GOTO_IF:
  case IC_TYPECAST:
  case IC_TO_F64:
  case IC_TO_I64:
  case IC_SQRT:
  case IC_NEG:
  case IC_LNOT:
  case IC_BNOT:
  case IC_POS:
  case IC_TO_BOOL:
  case IC_DEREF:
  case IC_ADDR_OF:
    right = ICArgN(rpn, 0);
    right->res = right->tmp_res;
    if (right->tmp_res.mode)
      right->res.keep_in_tmp = 1;
    break;
  case IC_POST_INC:
  case IC_POST_DEC:
  case IC_PRE_INC:
  case IC_PRE_DEC:
    // These are special
    break;
  case IC_BT:
  case IC_BTS:
  case IC_BTR:
  case IC_BTC:
  case IC_LBTS:
  case IC_LBTR:
  case IC_LBTC:
  case IC_STORE:
  case IC_AND_AND:
  case IC_OR_OR:
  case IC_XOR_XOR:
  case IC_LSH:
  case IC_RSH:
  // case IC_ADD: add does SIB stuff
  case IC_SUB:
  case IC_DIV:
  case IC_MUL:
  case IC_AND:
  case IC_XOR:
  case IC_MOD:
  case IC_OR:
  case IC_POW:
    left = ICArgN(rpn, 1);
    right = ICArgN(rpn, 0);
  normal_binop:
    if (CanKeepInTmp(rpn, right, left, 0) && !SpillsTmpRegs(left) &&
        right->tmp_res.mode) {
      right->res = right->tmp_res;
      right->res.keep_in_tmp = 1;
      break;
    }
    break;
  // These get strange(compound compare)
  case IC_LE:
  case IC_GE:
  case IC_LT:
  case IC_GT:
    break;
  case IC_EQ_EQ:
  case IC_NE:
    // Nroot here,there are 2 vaiants of IC_EQ_EQ
    //   One with IC_GOTO_IF
    //   One use as a normal operator.
    //   Lets just reutrn 0 to make things simpler.
    break;
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
    left = ICArgN(rpn, 1);
    right = ICArgN(rpn, 0);
    if (left->type == IC_IREG || left->type == IC_FREG ||
        left->type == IC_BASE_PTR) {
      if (right->tmp_res.mode) {
        right->res = right->tmp_res;
        right->res.keep_in_tmp = 1;
      }
    }
    break;
  }
}

//
// ALWAYS ASSUME WORST CASE if we dont have a ALLOC'ed peice of RAM
//
int64_t __OptPassFinal(CCmpCtrl *cctrl, CRPN *rpn, char *bin,
                       int64_t code_off) {
  CRPN *old_rpn = cctrl->cur_rpn;
  cctrl->cur_rpn = rpn;
  CCodeMisc *misc, *misc2;
  CRPN *next, *next2, **range, **range_args, *next3, *a, *b;
  CICArg tmp = {0}, orig_dst = {0}, tmp2 = {0}, derefed = {0};
  int64_t i = 0, cnt, i2, use_reg, a_reg, b_reg, into_reg, use_flt_cmp, reverse,
          is_typecast, use_lock_prefix = 0;
  int64_t *range_cmp_types, use_flags = rpn->res.want_use_flags;
  CCodeMisc *old_fail_misc = (CCodeMisc *)cctrl->backend_user_data5,
            *old_pass_misc = (CCodeMisc *)cctrl->backend_user_data6;
  CCodeMiscRef *cm_ref;
  cctrl->backend_user_data5 = 0;
  cctrl->backend_user_data6 = 0;
  rpn->res.set_flags = 0;
  if (cctrl->code_ctrl->final_pass == 2)
    rpn->start_ptr = bin + code_off;
  if (rpn->flags & ICF_PRECOMPUTED)
    goto ret;
  char *enter_addr2, *enter_addr, *exit_addr, **fail1_addr, **fail2_addr,
      ***range_fail_addrs;
  if (cctrl->code_ctrl->dbg_info && cctrl->code_ctrl->final_pass == 2 &&
      rpn->ic_line) { // Final run
    if (MSize(cctrl->code_ctrl->dbg_info) / 8 >
        rpn->ic_line - cctrl->code_ctrl->min_ln) {
      i = (int64_t)(cctrl->code_ctrl
                        ->dbg_info[rpn->ic_line - cctrl->code_ctrl->min_ln]);
      if (!i)
        cctrl->code_ctrl->dbg_info[rpn->ic_line - cctrl->code_ctrl->min_ln] =
            MemGetExecPtr(bin + code_off);
      else
        cctrl->code_ctrl->dbg_info[rpn->ic_line - cctrl->code_ctrl->min_ln] =
            MemGetExecPtr(bin + code_off);
    }
  }
  static const void *poop_ants[IC_CNT] = {
      [IC_FS] = &&ic_fs,
      [IC_GS] = &&ic_gs,
      [IC_LOCK] = &&ic_lock,
      [IC_GOTO] = &&ic_goto,
      [IC_GOTO_IF] = &&ic_goto_if,
      [IC_TO_BOOL] = &&ic_to_bool,
      [IC_TO_I64] = &&ic_to_i64,
      [IC_TO_F64] = &&ic_to_f64,
      [IC_LABEL] = &&ic_label,
      [IC_STATIC] = &&ic_static,
      [IC_GLOBAL] = &&ic_global,
      [IC_NOP] = &&ic_nop,
      [IC_NEG] = &&ic_neg,
      [IC_SQRT] = &&ic_sqrt,
      [IC_POS] = &&ic_pos,
      [IC_STR] = &&ic_str,
      [IC_CHR] = &&ic_chr,
      [IC_POW] = &&ic_pow,
      [IC_ADD] = &&ic_add,
      [IC_EQ] = &&ic_eq,
      [IC_SUB] = &&ic_sub,
      [IC_DIV] = &&ic_div,
      [IC_MUL] = &&ic_mul,
      [IC_DEREF] = &&ic_deref,
      [IC_AND] = &&ic_and,
      [IC_ADDR_OF] = &&ic_addr_of,
      [IC_XOR] = &&ic_xor,
      [IC_MOD] = &&ic_mod,
      [IC_OR] = &&ic_or,
      [IC_LT] = &&ic_lt,
      [IC_GT] = &&ic_gt,
      [IC_LNOT] = &&ic_lnot,
      [IC_BNOT] = &&ic_bnot,
      [IC_POST_INC] = &&ic_post_inc,
      [IC_POST_DEC] = &&ic_post_dec,
      [IC_PRE_INC] = &&ic_pre_inc,
      [IC_PRE_DEC] = &&ic_pre_dec,
      [IC_AND_AND] = &&ic_and_and,
      [IC_OR_OR] = &&ic_or_or,
      [IC_XOR_XOR] = &&ic_xor_xor,
      [IC_EQ_EQ] = &&ic_eq_eq,
      [IC_NE] = &&ic_ne,
      [IC_LE] = &&ic_le,
      [IC_GE] = &&ic_ge,
      [IC_LSH] = &&ic_lsh,
      [IC_RSH] = &&ic_rsh,
      [IC_ADD_EQ] = &&ic_add_eq,
      [IC_SUB_EQ] = &&ic_sub_eq,
      [IC_MUL_EQ] = &&ic_mul_eq,
      [IC_DIV_EQ] = &&ic_div_eq,
      [IC_LSH_EQ] = &&ic_lsh_eq,
      [IC_RSH_EQ] = &&ic_rsh_eq,
      [IC_AND_EQ] = &&ic_and_eq,
      [IC_OR_EQ] = &&ic_or_eq,
      [IC_XOR_EQ] = &&ic_xor_eq,
      [IC_MOD_EQ] = &&ic_mod_eq,
      [IC_I64] = &&ic_i64,
      [IC_F64] = &&ic_f64,
      [IC_ARRAY_ACC] = &&ic_array_acc,
      [IC_RET] = &&ic_ret,
      [IC_CALL] = &&ic_call,
      [IC_COMMA] = &&ic_comma,
      [IC_UNBOUNDED_SWITCH] = &&ic_unbounded_switch,
      [IC_BOUNDED_SWITCH] = &&ic_bounded_switch,
      [IC_SUB_RET] = &&ic_sub_ret,
      [IC_SUB_PROLOG] = &&ic_sub_prolog,
      [IC_SUB_CALL] = &&ic_sub_call,
      [IC_TYPECAST] = &&ic_typecast,
      [IC_BASE_PTR] = &&ic_base_ptr,
      [IC_IREG] = &&ic_ireg,
      [IC_FREG] = &&ic_freg,
      [__IC_VARGS] = &&ic_vargs,
      [IC_RELOC] = &&ic_reloc,
      [__IC_CALL] = &&ic_call,
      [__IC_STATIC_REF] = &&ic_static_ref,
      [__IC_SET_STATIC_DATA] = &&ic_set_static_data,
      [IC_SHORT_ADDR] = &&ic_short_addr,
      [IC_BT] = &&ic_bt,
      [IC_BTC] = &&ic_btc,
      [IC_BTS] = &&ic_bts,
      [IC_BTR] = &&ic_btr,
      [IC_LBTC] = &&ic_lbtc,
      [IC_LBTS] = &&ic_lbts,
      [IC_LBTR] = &&ic_lbtr,
      [IC_MAX_I64] = &&ic_max_i64,
      [IC_MIN_I64] = &&ic_min_i64,
      [IC_MAX_U64] = &&ic_max_u64,
      [IC_MIN_U64] = &&ic_min_u64,
      [IC_SIGN_I64] = &&ic_sign_i64,
      [IC_SQR_I64] = &&ic_sqr_i64,
      [IC_SQR_U64] = &&ic_sqr_u64,
      [IC_SQR] = &&ic_sqr,
      [IC_ABS] = &&ic_abs,
      [IC_SQRT] = &&ic_sqrt,
      [IC_SIN] = &&ic_sin,
      [IC_COS] = &&ic_cos,
      [IC_TAN] = &&ic_tan,
      [IC_ATAN] = &&ic_atan,
      [IC_RAW_BYTES] = &&ic_raw_bytes,
  };
  if (!poop_ants[rpn->type])
    goto ret;
  goto *poop_ants[rpn->type];
  do {
  ic_max_u64:
#define IC_MXX_X64(OP)                                                         \
  {                                                                            \
    a = ICArgN(rpn, 1);                                                        \
    b = ICArgN(rpn, 0);                                                        \
    code_off = __OptPassFinal(cctrl, a, bin, code_off);                        \
    code_off = __OptPassFinal(cctrl, b, bin, code_off);                        \
    into_reg = 0;                                                              \
    tmp.mode = MD_REG;                                                         \
    tmp.raw_type = rpn->res.raw_type;                                          \
    tmp.reg = into_reg;                                                        \
    code_off =                                                                 \
        PutICArgIntoReg(cctrl, &b->res, tmp.raw_type, RDX, bin, code_off);     \
    code_off = ICMov(cctrl, &tmp, &a->res, bin, code_off);                     \
    AIWNIOS_ADD_CODE(X86CmpRegReg, into_reg, b->res.reg);                      \
    AIWNIOS_ADD_CODE(OP, MIR(cctrl, into_reg), b->res.reg);                    \
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                   \
  }
    IC_MXX_X64(X86CmovbRegReg);
    break;
  ic_max_i64:
    IC_MXX_X64(X86CmovlRegReg);
    break;
  ic_min_u64:
    IC_MXX_X64(X86CmovaRegReg);
    break;
  ic_min_i64:
    IC_MXX_X64(X86CmovgRegReg);
    break;
  ic_gs:
    next = rpn->base.next;
    if (next->type != IC_RELOC && next->type != IC_I64) {
      printf("Expected a relocation at IC_GS,aborting(Contact nrootcoauto for "
             "more info)\n");
      abort();
    }
    goto segment;
    break;
  ic_fs:
    next = rpn->base.next;
    if (next->type != IC_RELOC && next->type != IC_I64) {
      printf("Expected a relocation at IC_FS,aborting(Contact nrootcoauto for "
             "more info)\n");
      abort();
    }
  segment:
    // %gs is used as a TLS reg for all OS'es
    into_reg = 0;
#ifdef __OpenBSD__
    AIWNIOS_ADD_CODE(SEG_FS, 0);
#else
    AIWNIOS_ADD_CODE(SEG_GS, 0);
#endif
    if (rpn->res.mode == MD_REG && next->type == IC_I64 &&
        Is32Bit(next->integer)) {
      into_reg = rpn->res.reg;
      AIWNIOS_ADD_CODE(X86MovRegIndirI64, into_reg, -1, -1, -1, 0x12345);
    } else {
      into_reg = 0;
      AIWNIOS_ADD_CODE(X86MovRegIndirI64, into_reg, -1, -1, -1, 0x12345);
    }
    MIR(cctrl, into_reg);
    if (next->type == IC_I64) {
      if (bin)
        *(int32_t *)(bin + code_off - 4) = (int32_t)next->integer;
    } else if (bin)
      CodeMiscAddRef(next->code_misc, bin + code_off - 4);
    tmp.mode = MD_REG;
    tmp.reg = into_reg;
    tmp.raw_type = RT_I64i;
    rpn->tmp_res = tmp;
    if (rpn->res.keep_in_tmp)
      rpn->res = tmp;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_lock:
    cctrl->is_lock_expr = 1;
    code_off = __OptPassFinal(cctrl, rpn->base.next, bin, code_off);
    cctrl->is_lock_expr = 0;
    break;
  ic_short_addr:
    if (rpn->res.mode == MD_REG)
      into_reg = rpn->res.reg;
    else
      into_reg = 0;
    tmp.mode = __MD_X86_64_SIB;
    tmp.raw_type = RT_I64i;
    tmp.reg = RIP;
    tmp.__SIB_scale = -1;
    tmp.reg2 = -1;
    // CCF_AOT_COMPILE will trigger a IET_REL_I32 on this location
    tmp.off = rpn->integer;
    AIWNIOS_ADD_CODE(X86LeaSIB, MIR(cctrl, into_reg), -1, -1, RIP, tmp.off);
    if (bin)
      CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
    if (rpn->res.mode != MD_REG) {
      tmp.mode = MD_REG;
      tmp.raw_type = RT_I64i;
      tmp.reg = 0;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_reloc:
    if (rpn->res.mode == MD_REG)
      into_reg = rpn->res.reg;
    else
      into_reg = 0;
    misc = rpn->code_misc;
    i = -code_off + misc->aot_before_hint;
    AIWNIOS_ADD_CODE(
        X86MovRegIndirI64, MIR(cctrl, into_reg), -1, -1, RIP,
        X86MovRegIndirI64(NULL, into_reg, -1, -1, RIP,
                          0)); // X86MovRegIndirI64 Will return inst length
    CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
    if (rpn->res.mode != MD_REG) {
      tmp.mode = MD_REG;
      tmp.raw_type = RT_I64i;
      tmp.reg = 0;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_goto_if:
    reverse = 0;
    next = rpn->base.next;
  gti_enter:
    if (!IsCompoundCompare(next))
      switch (next->type) {
        break;
      case IC_LNOT:
        reverse = !reverse;
        next = next->base.next;
        goto gti_enter;
        break;
      case IC_EQ_EQ:
#define CMP_AND_JMP(COND)                                                      \
  {                                                                            \
    next3 = next->base.next;                                                   \
    next2 = ICFwd(next3);                                                      \
    PushTmpDepthFirst(cctrl, next3, SpillsTmpRegs(next2));                     \
    PushTmpDepthFirst(cctrl, next2, 0);                                        \
    PopTmp(cctrl, next2);                                                      \
    PopTmp(cctrl, next3);                                                      \
    if (next3->raw_type != RT_F64 && next2->raw_type != RT_F64 &&              \
        IsConst(next3) && Is32Bit(next3->integer)) {                           \
      if (ModeIsDerefToSIB(next2)) {                                           \
        code_off = DerefToICArg(cctrl, &tmp, next2, AIWNIOS_TMP_IREG_POOP,     \
                                bin, code_off);                                \
        switch (next2->raw_type) {                                             \
        case RT_I8i:                                                           \
        case RT_U8i:                                                           \
          AIWNIOS_ADD_CODE(X86CmpSIB8Imm, next3->integer, tmp.__SIB_scale,     \
                           tmp.reg2, tmp.reg, tmp.off);                        \
          break;                                                               \
        case RT_I16i:                                                          \
        case RT_U16i:                                                          \
          AIWNIOS_ADD_CODE(X86CmpSIB16Imm, next3->integer, tmp.__SIB_scale,    \
                           tmp.reg2, tmp.reg, tmp.off);                        \
          break;                                                               \
        case RT_I32i:                                                          \
        case RT_U32i:                                                          \
          AIWNIOS_ADD_CODE(X86CmpSIB32Imm, next3->integer, tmp.__SIB_scale,    \
                           tmp.reg2, tmp.reg, tmp.off);                        \
          break;                                                               \
        default:                                                               \
          AIWNIOS_ADD_CODE(X86CmpSIB64Imm, next3->integer, tmp.__SIB_scale,    \
                           tmp.reg2, tmp.reg, tmp.off);                        \
        }                                                                      \
      } else {                                                                 \
        next2->res.keep_in_tmp = 1;                                            \
        code_off = __OptPassFinal(cctrl, next2, bin, code_off);                \
        code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,                \
                                   AIWNIOS_TMP_IREG_POOP2, bin, code_off);     \
        AIWNIOS_ADD_CODE(X86CmpRegImm, next2->res.reg, next3->integer);        \
      }                                                                        \
    } else if (next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {       \
      code_off = __OptPassFinal(cctrl, next3, bin, code_off);                  \
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);                  \
      code_off =                                                               \
          PutICArgIntoReg(cctrl, &next2->res, RT_F64, 1, bin, code_off);       \
      code_off =                                                               \
          PutICArgIntoReg(cctrl, &next3->res, RT_F64, 0, bin, code_off);       \
      AIWNIOS_ADD_CODE(X86COMISDRegReg, next2->res.reg, next3->res.reg);       \
    } else if (ModeIsDerefToSIB(next3) && RawTypeIs64(next3->res.raw_type)) {  \
      code_off = DerefToICArg(cctrl, &tmp, next3, AIWNIOS_TMP_IREG_POOP, bin,  \
                              code_off);                                       \
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);                  \
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,                  \
                                 AIWNIOS_TMP_IREG_POOP2, bin, code_off);       \
      AIWNIOS_ADD_CODE(X86CmpRegSIB64, next2->res.reg, tmp.__SIB_scale,        \
                       tmp.reg2, tmp.reg, tmp.off);                            \
    } else if (ModeIsDerefToSIB(next2) && RawTypeIs64(next2->res.raw_type)) {  \
      code_off = __OptPassFinal(cctrl, next3, bin, code_off);                  \
      code_off = DerefToICArg(cctrl, &tmp, next2, AIWNIOS_TMP_IREG_POOP, bin,  \
                              code_off);                                       \
      code_off = PutICArgIntoReg(cctrl, &next3->res, RT_I64i,                  \
                                 AIWNIOS_TMP_IREG_POOP2, bin, code_off);       \
      AIWNIOS_ADD_CODE(X86CmpSIBReg64, next3->res.reg, tmp.__SIB_scale,        \
                       tmp.reg2, tmp.reg, tmp.off);                            \
    } else {                                                                   \
      code_off = __OptPassFinal(cctrl, next3, bin, code_off);                  \
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);                  \
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,                  \
                                 AIWNIOS_TMP_IREG_POOP2, bin, code_off);       \
      code_off = PutICArgIntoReg(cctrl, &next3->res, RT_I64i,                  \
                                 AIWNIOS_TMP_IREG_POOP, bin, code_off);        \
      AIWNIOS_ADD_CODE(X86CmpRegReg, next2->res.reg, next3->res.reg);          \
    }                                                                          \
    AIWNIOS_ADD_CODE(X86Jcc, (COND) ^ reverse, 6);                             \
    CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);                        \
  }
        CMP_AND_JMP(X86_COND_E);
        goto ret;
        break;
      case IC_NE:
        CMP_AND_JMP(X86_COND_E | 1);
        goto ret;
        break;
      case IC_LT:
        next3 = next->base.next;
        next2 = ICFwd(next3);
        if (next3->raw_type == RT_U64i || next2->raw_type == RT_U64i ||
            next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {
          CMP_AND_JMP(X86_COND_B);
        } else {
          CMP_AND_JMP(X86_COND_L);
        }
        goto ret;
        break;
      case IC_GT:
        next3 = next->base.next;
        next2 = ICFwd(next3);
        if (next3->raw_type == RT_U64i || next2->raw_type == RT_U64i ||
            next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {
          CMP_AND_JMP(X86_COND_A);
        } else {
          CMP_AND_JMP(X86_COND_G);
        }
        goto ret;
        break;
      case IC_LE:
        next3 = next->base.next;
        next2 = ICFwd(next3);
        if (next3->raw_type == RT_U64i || next2->raw_type == RT_U64i ||
            next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {
          CMP_AND_JMP(X86_COND_BE);
        } else {
          CMP_AND_JMP(X86_COND_LE);
        }
        goto ret;
        break;
      case IC_GE:
        next3 = next->base.next;
        next2 = ICFwd(next3);
        if (next3->raw_type == RT_U64i || next2->raw_type == RT_U64i ||
            next3->raw_type == RT_F64 || next2->raw_type == RT_F64) {
          CMP_AND_JMP(X86_COND_AE);
        } else {
          CMP_AND_JMP(X86_COND_GE);
        }
        goto ret;
      case IC_BT:
      case IC_BTC:
      case IC_BTS:
      case IC_BTR:
      case IC_LBTC:
      case IC_LBTS:
      case IC_LBTR:
        PushTmpDepthFirst(cctrl, next, 0);
        PopTmp(cctrl, next);
        if (!rpn->code_misc2)
          rpn->code_misc2 = CodeMiscNew(cctrl, CMT_LABEL);
        if (!reverse) {
          cctrl->backend_user_data5 = (int64_t)rpn->code_misc2;
          cctrl->backend_user_data6 = (int64_t)rpn->code_misc;
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          rpn->code_misc2->addr = bin + code_off;
          goto ret;
        } else {
          cctrl->backend_user_data5 = (int64_t)rpn->code_misc;
          cctrl->backend_user_data6 = (int64_t)rpn->code_misc2;
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          rpn->code_misc2->addr = bin + code_off;
          goto ret;
        }
      case IC_OR_OR:
      case IC_AND_AND:
        PushTmpDepthFirst(cctrl, next, 0);
        PopTmp(cctrl, next);
        if (!rpn->code_misc2)
          rpn->code_misc2 = CodeMiscNew(cctrl, CMT_LABEL);
        if (!reverse) {
          cctrl->backend_user_data5 = (int64_t)rpn->code_misc2;
          cctrl->backend_user_data6 = (int64_t)rpn->code_misc;
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          rpn->code_misc2->addr = bin + code_off;
          goto ret;
        } else {
          cctrl->backend_user_data5 = (int64_t)rpn->code_misc;
          cctrl->backend_user_data6 = (int64_t)rpn->code_misc2;
          code_off = __OptPassFinal(cctrl, next, bin, code_off);
          rpn->code_misc2->addr = bin + code_off;
          goto ret;
        }
      }
    PushTmpDepthFirst(cctrl, next, 0);
    PopTmp(cctrl, next);
    next->res.want_use_flags = 1;
    next->res.keep_in_tmp = 1;
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    //
    // res.keep_in_tmp always stores into RAX/XMM0,so
    //
    // So later on don't use RAX/XMM0 as it is the tmp register
    //
    // See Below now
    //
    if (next->res.set_flags) {
      if (!reverse) {
        AIWNIOS_ADD_CODE(X86Jcc, X86_COND_E | 1, 6);
        CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
      } else {
        AIWNIOS_ADD_CODE(X86Jcc, X86_COND_E, 6);
        CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
      }
    } else if (next->raw_type == RT_F64) {
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_F64, 2, bin, code_off);
      // See Above note
      code_off = __ICMoveF64(cctrl, 1, 0, bin, code_off);
      AIWNIOS_ADD_CODE(X86COMISDRegReg, next->res.reg, 1);
      if (!reverse) {
        AIWNIOS_ADD_CODE(X86Jcc, X86_COND_E | 1, 6);
        CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
      } else {
        AIWNIOS_ADD_CODE(X86Jcc, X86_COND_E, 6);
        CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
      }
    } else {
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_I64i, 0, bin, code_off);
      AIWNIOS_ADD_CODE(X86Test, next->res.reg, next->res.reg);
      if (!reverse) {
        AIWNIOS_ADD_CODE(X86Jcc, X86_COND_E | 1, 6);
        CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
      } else {
        AIWNIOS_ADD_CODE(X86Jcc, X86_COND_E, 6);
        CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
      }
    }
    break;
  ic_to_bool:
    into_reg = 0;
    tmp.mode = MD_REG;
    tmp.raw_type = RT_I64i;
    if (rpn->res.mode == MD_REG)
      into_reg = rpn->res.reg;
    tmp.reg = into_reg;
    next = rpn->base.next;
    if (ModeIsDerefToSIB(next) && next->raw_type != RT_F64) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP, bin,
                              code_off);
      switch (tmp.raw_type) {
      case RT_I8i:
      case RT_U8i:
        AIWNIOS_ADD_CODE(X86CmpSIB8Imm, 0, derefed.__SIB_scale, derefed.reg2,
                         derefed.reg, derefed.off);
        break;
      case RT_I16i:
      case RT_U16i:
        AIWNIOS_ADD_CODE(X86CmpSIB16Imm, 0, derefed.__SIB_scale, derefed.reg2,
                         derefed.reg, derefed.off);
        break;
      case RT_I32i:
      case RT_U32i:
        AIWNIOS_ADD_CODE(X86CmpSIB32Imm, 0, derefed.__SIB_scale, derefed.reg2,
                         derefed.reg, derefed.off);
        break;
      default:
        AIWNIOS_ADD_CODE(X86CmpSIB64Imm, 0, derefed.__SIB_scale, derefed.reg2,
                         derefed.reg, derefed.off);
        break;
      }
      AIWNIOS_ADD_CODE(X86Setcc, X86_COND_E | 1, MIR(cctrl, into_reg));
    } else {
      next->res.keep_in_tmp = 1;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_I64i, 0, bin, code_off);
      AIWNIOS_ADD_CODE(X86CmpRegImm, next->res.reg, 0);
      AIWNIOS_ADD_CODE(X86Setcc, X86_COND_E | 1, MIR(cctrl, into_reg));
    }
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
    // These both do the same thing,only thier type detirmines what happens
  ic_to_i64:
  ic_to_f64:
    next = rpn->base.next;
    next->res.keep_in_tmp = 1;
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
    break;
  ic_typecast:
    next = rpn->base.next;
    if (next->raw_type == RT_F64 && rpn->raw_type != RT_F64) {
      next->res.keep_in_tmp = 1;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next->res, next->raw_type, 0, bin, code_off);
      tmp.mode = MD_REG;
      tmp.reg = 0;
      tmp.raw_type = rpn->raw_type;
      if (rpn->res.mode == MD_REG)
        tmp.reg = rpn->res.reg;
      AIWNIOS_ADD_CODE(X86MovQI64F64, MIR(cctrl, tmp.reg), next->res.reg);
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else if (next->raw_type != RT_F64 && rpn->raw_type == RT_F64) {
      next->res.keep_in_tmp = 1;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next->res, next->raw_type, 0, bin, code_off);
      tmp.mode = MD_REG;
      tmp.reg = 0;
      tmp.raw_type = rpn->raw_type;
      if (rpn->res.mode == MD_REG)
        tmp.reg = rpn->res.reg;
      AIWNIOS_ADD_CODE(X86MovQF64I64, MFR(cctrl, tmp.reg), next->res.reg);
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else if (next->type == IC_DEREF) {
      code_off = DerefToICArg(cctrl, &tmp, next, 1, bin, code_off);
      tmp.raw_type = rpn->raw_type;
      // Only keep in registers
      // if (rpn->res.keep_in_tmp)
      //   rpn->res = tmp;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      // Only keep in registers
      // if (rpn->res.keep_in_tmp)
      //   rpn->res = next->res;
      code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
    }
    break;
  ic_goto:
    if (!(rpn->flags & ICF_NO_JUMP)) {
      if (cctrl->code_ctrl->final_pass) {
        AIWNIOS_ADD_CODE(X86Jmp, 5); // 1 for opcode,4 for offsetr
        CodeMiscAddRef(rpn->code_misc, bin + code_off - 4)->from_ic = rpn;
      } else
        AIWNIOS_ADD_CODE(X86Jmp, 0);
    }
    break;
  ic_label:
    rpn->code_misc->addr = bin + code_off;
    break;
  ic_global:
    //
    // Here's the deal. Global arrays should be acceessed via
    // IC_ADDR_OF. So loading int register here
    //
    if (rpn->global_var->base.type & HTT_FUN) {
      next = rpn;
      goto get_glbl_ptr;
    } else {
      assert(!rpn->global_var->dim.next);
      tmp.mode = MD_PTR;
      tmp.off = (int64_t)rpn->global_var->data_addr;
      tmp.raw_type = rpn->global_var->var_class->raw_type;
      // if (rpn->res.keep_in_tmp)
      //   rpn->res = tmp;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_nop:
    // Bungis
    break;
    abort();
    break;
  ic_sqrt:
    into_reg = 0;
    if (rpn->res.mode == MD_REG)
      into_reg = rpn->res.reg;
    next = rpn->base.next;
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    if (ModeIsDerefToSIB(next)) {
      code_off = DerefToICArg(cctrl, &derefed, next, AIWNIOS_TMP_IREG_POOP2,
                              bin, code_off);
      AIWNIOS_ADD_CODE(X86SqrtSIB, MFR(cctrl, into_reg), derefed.__SIB_scale,
                       derefed.reg2, derefed.reg, derefed.off);

    } else {
      code_off =
          PutICArgIntoReg(cctrl, &next->res, RT_F64, into_reg, bin, code_off);
      AIWNIOS_ADD_CODE(X86SqrtReg, MFR(cctrl, into_reg), next->res.reg);
    }
    tmp.mode = MD_REG;
    tmp.reg = into_reg;
    tmp.raw_type = RT_F64;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_neg:
#define BACKEND_UNOP(flt_op, int_op)                                           \
  next = rpn->base.next;                                                       \
  next->res.keep_in_tmp = 1;                                                   \
  code_off = __OptPassFinal(cctrl, next, bin, code_off);                       \
  into_reg = 0;                                                                \
  if (rpn->res.mode == MD_REG)                                                 \
    into_reg = rpn->res.reg;                                                   \
  if (rpn->res.mode == MD_NULL)                                                \
    break;                                                                     \
  tmp.mode = MD_REG;                                                           \
  tmp.raw_type = RT_I64i;                                                      \
  tmp.reg = into_reg;                                                          \
  code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);                    \
  if (rpn->raw_type == RT_F64) {                                               \
    AIWNIOS_ADD_CODE(flt_op, MFR(cctrl, into_reg));                            \
  } else                                                                       \
    AIWNIOS_ADD_CODE(int_op, MFR(cctrl, into_reg));                            \
  rpn->tmp_res = tmp;                                                          \
  if (rpn->res.keep_in_tmp) {                                                  \
    rpn->res = tmp;                                                            \
    rpn->res.keep_in_tmp = 1;                                                  \
  }                                                                            \
  code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    if (rpn->raw_type == RT_F64) {
      next = ICArgN(rpn, 0);
      static double ul;
      ((int64_t *)&ul)[0] = 0x80000000ll << 32;
      next->res.keep_in_tmp = 1;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      tmp = rpn->res;
      code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &tmp, RT_F64, 1, bin, code_off);
      code_off = __ICMoveF64(cctrl, 2, ul, bin, code_off);
      AIWNIOS_ADD_CODE(X86XorPDReg, MFR(cctrl, tmp.reg), 2);
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    } else {
      BACKEND_UNOP(X86NegReg, X86NegReg);
      if (rpn->raw_type != RT_F64)
        rpn->res.set_flags = 1;
    }
    break;
  ic_pos:
    next = ICArgN(rpn, 0);
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    // Only keep in registers
    // if (rpn->res.keep_in_tmp)
    //   rpn->res = next->res;
    code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
    break;
    abort();
    break;
  ic_str:
    if (rpn->res.mode == MD_REG) {
      into_reg = rpn->res.reg;
    } else
      into_reg = 0; // Store into reg 0

    if (cctrl->flags & CCF_STRINGS_ON_HEAP) {
      //
      // README: We will "NULL"ify the rpn->code_misc->str later so we dont
      // free it
      //
      code_off = __ICMoveI64(cctrl, into_reg, (int64_t)rpn->code_misc->str, bin,
                             code_off);
    } else {
      AIWNIOS_ADD_CODE(X86LeaSIB, MIR(cctrl, into_reg), -1, -1, RIP,
                       0); // X86LeaSIB will return inst size
      CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
    }
    if (rpn->res.mode != MD_REG) {
      tmp.raw_type = rpn->raw_type;
      tmp.reg = 0;
      tmp.mode = MD_REG;
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_chr:
    if (rpn->res.mode == MD_REG) {
      into_reg = rpn->res.reg;
    } else
      into_reg = 0; // Store into reg 0

    code_off = __ICMoveI64(cctrl, into_reg, rpn->integer, bin, code_off);

    if (rpn->res.mode != MD_REG) {
      tmp.raw_type = rpn->raw_type;
      tmp.reg = 0;
      tmp.mode = MD_REG;
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_pow:
    // TODO
    break;
    break;
  ic_unbounded_switch:
    //
    // Load poop into tmp,then we continue the sexyness as normal
    //
    // Also make sure R8 has lo bound(See IC_BOUNDED_SWITCH)
    //
    next2 = ICArgN(rpn, 0);
    PushTmpDepthFirst(cctrl, next2, 0);
    PopTmp(cctrl, next2);
    next2->res.keep_in_tmp = 1;
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);
    tmp.raw_type = RT_I64i;
    tmp.mode = MD_REG;
    tmp.reg = 0;
    code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
    code_off = __ICMoveI64(cctrl, R8, rpn->code_misc->lo, bin, code_off); // X1
    goto jmp_tab_sexy;
    break;
  ic_bounded_switch:
    next2 = ICArgN(rpn, 0);
    PushTmpDepthFirst(cctrl, next2, 0);
    PopTmp(cctrl, next2);
    next2->res.keep_in_tmp = 1;
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);
    tmp.raw_type = RT_I64i;
    tmp.mode = MD_REG;
    tmp.reg = RCX;
    code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
    code_off = __ICMoveI64(cctrl, RAX, rpn->code_misc->hi, bin, code_off);
    code_off = __ICMoveI64(cctrl, R8, rpn->code_misc->lo, bin, code_off);
    AIWNIOS_ADD_CODE(X86CmpRegReg, tmp.reg, R8);
    fail1_addr = bin + code_off;
    AIWNIOS_ADD_CODE(X86Jcc, X86_COND_L, 6);
    CodeMiscAddRef(rpn->code_misc->dft_lab, bin + code_off - 4);
    AIWNIOS_ADD_CODE(X86CmpRegReg, tmp.reg, RAX);
    fail2_addr = bin + code_off;
    AIWNIOS_ADD_CODE(X86Jcc, X86_COND_G, 6);
    CodeMiscAddRef(rpn->code_misc->dft_lab, bin + code_off - 4);
  jmp_tab_sexy:
    AIWNIOS_ADD_CODE(X86LeaSIB, MIR(cctrl, RDX), -1, -1, RIP, 0);
    CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
    //-8*rpn->code_misc->lo offsets our tmp.reg by lo
    AIWNIOS_ADD_CODE(X86JmpSIB, 8, MIR(cctrl, tmp.reg), RDX,
                     -8 * rpn->code_misc->lo);
    break;
  ic_sub_ret:
    AIWNIOS_ADD_CODE(X86Ret, 0);
    break;
  ic_sub_prolog:
    break;
  ic_sub_call:
    AIWNIOS_ADD_CODE(X86Call32, 5);
    CodeMiscAddRef(rpn->code_misc, bin + code_off - 4);
    break;
  ic_add:
    if (rpn->res.mode == __MD_X86_64_LEA_SIB) {
      if (rpn->res.__sib_base_rpn) {
        code_off =
            __OptPassFinal(cctrl, rpn->res.__sib_base_rpn, bin, code_off);
      }
      if (rpn->res.__sib_idx_rpn) {
        code_off = __OptPassFinal(cctrl, rpn->res.__sib_idx_rpn, bin, code_off);
      }
      AIWNIOS_ADD_CODE(X86LeaSIB, MIR(cctrl, rpn->res.fallback_reg),
                       rpn->res.__SIB_scale, rpn->res.reg2, rpn->res.reg,
                       rpn->res.off);
      tmp.mode = MD_REG;
      tmp.raw_type = RT_I64i;
      tmp.reg = rpn->res.fallback_reg;
      rpn->res = tmp; // Swap out the old mode with the new one
      goto ret;
    }
//
//  /\   /\      ____
//  ||___||     /    \
//  | o_o | <==(README)
//  | \_/ |     \____/
//  ||   ||
//  \/   \/
// We compute the righthand argument first
// Because the lefthand side can be assigned into(and we will need
// the temporary register containg the pointer presetn and not messed up)
//
// RIGHT ==> may have func call that invalidates tmp regs
// LEFT ==> may have a MD_INDIR_REG(where reg is a tmp reg)
// *LEFT=RIHGT ==> Write into the left's address
//
#define BACKEND_BINOP(f_op, i_op, f_sib_op, i_sib_op)                          \
  do {                                                                         \
    next = ICArgN(rpn, 1);                                                     \
    next2 = ICArgN(rpn, 0);                                                    \
    orig_dst = next->res;                                                      \
    if (ModeIsDerefToSIB(next2) &&                                             \
        ((next2->raw_type == RT_F64) == (rpn->raw_type == RT_F64))) {          \
      if (next->raw_type != RT_F64 && RawTypeIs64(next2->raw_type)) {          \
        code_off = __OptPassFinal(cctrl, next, bin, code_off);                 \
        code_off = DerefToICArg(cctrl, &derefed, next2,                        \
                                AIWNIOS_TMP_IREG_POOP2, bin, code_off);        \
        if (rpn->res.mode == MD_NULL)                                          \
          break;                                                               \
        if (rpn->res.mode == MD_REG &&                                         \
            !DstRegAffectsMode(&rpn->res, &derefed)) {                         \
          into_reg = rpn->res.reg;                                             \
        } else                                                                 \
          into_reg = RDX;                                                      \
        tmp.mode = MD_REG;                                                     \
        tmp.raw_type = rpn->raw_type;                                          \
        tmp.reg = into_reg;                                                    \
        code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);              \
        if (rpn->raw_type == RT_F64) {                                         \
          AIWNIOS_ADD_CODE(f_sib_op, MFR(cctrl, tmp.reg), derefed.__SIB_scale, \
                           derefed.reg2, derefed.reg, derefed.off);            \
        } else {                                                               \
          AIWNIOS_ADD_CODE(i_sib_op, MIR(cctrl, tmp.reg), derefed.__SIB_scale, \
                           derefed.reg2, derefed.reg, derefed.off);            \
        }                                                                      \
        if (rpn->res.mode != MD_NULL) {                                        \
          code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);             \
        }                                                                      \
        break;                                                                 \
      }                                                                        \
    } else if (next2->type == IC_F64 && rpn->raw_type == RT_F64) {             \
      code_off = __OptPassFinal(cctrl, next, bin, code_off);                   \
      if (rpn->res.mode == MD_REG &&                                           \
          !DstRegAffectsMode(&rpn->res, &next->res)) {                         \
        into_reg = rpn->res.reg;                                               \
      } else                                                                   \
        into_reg = RDX;                                                        \
      tmp.mode = MD_REG;                                                       \
      tmp.raw_type = rpn->raw_type;                                            \
      tmp.reg = into_reg;                                                      \
      code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);                \
      AIWNIOS_ADD_CODE(f_sib_op, MFR(cctrl, into_reg), -1, -1, RIP,            \
                       0xb00b1e5);                                             \
      misc = GetF64LiteralMisc(cctrl, next2->flt);                             \
      if (bin)                                                                 \
        CodeMiscAddRef(misc, bin + code_off - 4);                              \
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                 \
      break;                                                                   \
    }                                                                          \
    code_off = __OptPassFinal(cctrl, next, bin, code_off);                     \
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);                    \
    if (rpn->res.mode == MD_NULL)                                              \
      break;                                                                   \
    if (rpn->res.mode == MD_REG &&                                             \
        !DstRegAffectsMode(&rpn->res, &next2->res)) {                          \
      into_reg = rpn->res.reg;                                                 \
    } else                                                                     \
      into_reg = RDX;                                                          \
    tmp.mode = MD_REG;                                                         \
    tmp.raw_type = rpn->raw_type;                                              \
    tmp.reg = into_reg;                                                        \
    code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);                  \
    if (rpn->raw_type == RT_F64) {                                             \
      code_off =                                                               \
          PutICArgIntoReg(cctrl, &tmp, rpn->raw_type, 0, bin, code_off);       \
      code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type, 1, bin,    \
                                 code_off);                                    \
    } else {                                                                   \
      code_off = PutICArgIntoReg(cctrl, &tmp, rpn->raw_type,                   \
                                 AIWNIOS_TMP_IREG_POOP, bin, code_off);        \
      code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type,            \
                                 AIWNIOS_TMP_IREG_POOP2, bin, code_off);       \
    }                                                                          \
    if (rpn->raw_type == RT_F64) {                                             \
      AIWNIOS_ADD_CODE(f_op, MFR(cctrl, tmp.reg), next2->res.reg);             \
    } else {                                                                   \
      AIWNIOS_ADD_CODE(i_op, MIR(cctrl, tmp.reg), next2->res.reg);             \
    }                                                                          \
    if (rpn->res.mode != MD_NULL) {                                            \
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                 \
    }                                                                          \
  } while (0);
#define BACKEND_BINOP_IMM(i_imm_op, i_op)                                      \
  next = ICArgN(rpn, 1);                                                       \
  next2 = ICArgN(rpn, 0);                                                      \
  if (rpn->raw_type != RT_F64 && IsConst(next2) && Is32Bit(ConstVal(next2)) && \
      next2->type != IC_F64 && next2->integer >= 0) {                          \
    code_off = __OptPassFinal(cctrl, next, bin, code_off);                     \
    tmp2 = next->res;                                                          \
    if (rpn->res.mode == MD_NULL)                                              \
      break;                                                                   \
    if (rpn->res.mode == MD_REG)                                               \
      into_reg = rpn->res.reg;                                                 \
    else                                                                       \
      into_reg = RDX;                                                          \
    code_off =                                                                 \
        PutICArgIntoReg(cctrl, &tmp2, RT_I64i, into_reg, bin, code_off);       \
    if (Is32Bit(next2->integer)) {                                             \
      tmp.reg = into_reg;                                                      \
      tmp.mode = MD_REG;                                                       \
      tmp.raw_type = rpn->raw_type;                                            \
      code_off = ICMov(cctrl, &tmp, &tmp2, bin, code_off);                     \
      AIWNIOS_ADD_CODE(i_imm_op, MIR(cctrl, into_reg), next2->integer);        \
    } else {                                                                   \
      tmp.mode = MD_REG;                                                       \
      tmp.reg = AIWNIOS_TMP_IREG_POOP;                                         \
      tmp.raw_type = RT_I64i;                                                  \
      code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);               \
      AIWNIOS_ADD_CODE(i_op, MIR(cctrl, into_reg), tmp.reg);                   \
    }                                                                          \
    if (rpn->res.mode != MD_REG && rpn->res.mode != MD_NULL) {                 \
      tmp.raw_type = rpn->raw_type;                                            \
      tmp.reg = into_reg;                                                      \
      tmp.mode = MD_REG;                                                       \
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                 \
    }                                                                          \
    break;                                                                     \
  }
    if (rpn->raw_type != RT_F64)
      rpn->res.set_flags = 1;
    BACKEND_BINOP_IMM(X86AddImm32, X86AddReg);
    BACKEND_BINOP(X86AddSDReg, X86AddReg, X86AddSDSIB64, X86AddSIB64);
    break;
  ic_comma:
    next = ICArgN(rpn, 1);
    next2 = ICArgN(rpn, 0);
    orig_dst = next->res;
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);
    next->res.keep_in_tmp = 1;
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    code_off = ICMov(cctrl, &rpn->res, &next2->res, bin, code_off);
    break;
  ic_eq:
    next = ICArgN(rpn, 1);
    next2 = ICArgN(rpn, 0);
    orig_dst = next->res;
    // Just store result in register if register operan
    if (next->type == IC_IREG || next->type == IC_FREG) {
      if (next2->res.mode == MD_REG &&
          (next2->res.raw_type == RT_F64) == (next->res.raw_type == RT_F64))
        next2->res = next->res;
    }
    code_off = __OptPassFinal(cctrl, next2, bin, code_off);
    if (next->type == IC_TYPECAST) {
      TYPECAST_ASSIGN_BEGIN(next, next2);
      if (rpn->res.mode != MD_NULL) {
        // Avoid a double-derefernce
        code_off = PutICArgIntoReg(
            cctrl, &next2->res, rpn->raw_type,
            rpn->raw_type == RT_F64 ? 0 : AIWNIOS_TMP_IREG_POOP, bin, code_off);
      }
      code_off = ICMov(cctrl, &next->res, &next2->res, bin, code_off);
      TYPECAST_ASSIGN_END(next);
    } else if (next->type == IC_DEREF) {
      tmp = next->res;
      code_off = DerefToICArg(cctrl, &tmp, next, AIWNIOS_TMP_IREG_POOP2, bin,
                              code_off);
      if (rpn->res.mode != MD_NULL) {
        // Avoid a double-derefernce
        code_off = PutICArgIntoReg(
            cctrl, &next2->res, rpn->raw_type,
            rpn->raw_type == RT_F64 ? 0 : AIWNIOS_TMP_IREG_POOP, bin, code_off);
      }
      code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      break;
    } else if (next->type == IC_GLOBAL) {
      next->res.mode = MD_PTR;
      next->res.raw_type = next->raw_type;
      if (next->global_var->base.type & HTT_GLBL_VAR) {
        next->res.off = (int64_t)next->global_var->data_addr;
      } else if (next->global_var->base.type & HTT_FUN) {
        next->res.off = (int64_t)((CHashFun *)next->global_var)->fun_ptr;
      }
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      if (rpn->res.mode != MD_NULL) {
        // Avoid a double-derefernce
        code_off = PutICArgIntoReg(
            cctrl, &next2->res, rpn->raw_type,
            rpn->raw_type == RT_F64 ? 0 : AIWNIOS_TMP_IREG_POOP, bin, code_off);
      }
      code_off = ICMov(cctrl, &next->res, &next2->res, bin, code_off);
    } else {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      if (rpn->res.mode != MD_NULL) {
        // Avoid a double-derefernce
        code_off = PutICArgIntoReg(
            cctrl, &next2->res, rpn->raw_type,
            rpn->raw_type == RT_F64 ? 0 : AIWNIOS_TMP_IREG_POOP, bin, code_off);
      }
      code_off = ICMov(cctrl, &next->res, &next2->res, bin, code_off);
    }
    code_off = ICMov(cctrl, &rpn->res, &next2->res, bin, code_off);
    break;
  ic_sub:
    if (rpn->raw_type != RT_F64)
      rpn->res.set_flags = 1;
    BACKEND_BINOP_IMM(X86SubImm32, X86SubReg);
    BACKEND_BINOP(X86SubSDReg, X86SubReg, X86SubSDSIB64, X86SubSIB64);
    break;
  ic_div:
    next = rpn->base.next;
    next2 = ICArgN(rpn, 1);
    if (next->type != IC_F64 && next2->raw_type != RT_F64) {
      if (IsConst(next))
        if (__builtin_popcountll(next->integer) == 1) {
          switch (next2->raw_type) {
          case RT_I8i:
          case RT_I16i:
          case RT_I32i:
          case RT_I64i:
            if (__builtin_ffsll(next->integer) >= 31)
              goto div_normal;
            // Big Brain stuff
            // https://stackoverflow.com/questions/63018450/which-kind-of-signed-integer-division-corresponds-to-bit-shift
            next2->res.keep_in_tmp = 1;
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            if (rpn->res.mode == MD_REG)
              into_reg = rpn->res.reg;
            else
              into_reg = RAX;
            tmp.mode = MD_REG;
            tmp.raw_type = RT_I64i;
            tmp.reg = into_reg;
            code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
            AIWNIOS_ADD_CODE(X86LeaSIB, MIR(cctrl, AIWNIOS_TMP_IREG_POOP), 1,
                             -1, into_reg, ConstVal(next) - 1);
            AIWNIOS_ADD_CODE(X86Test, into_reg, into_reg);
            AIWNIOS_ADD_CODE(X86CMovsRegReg, MIR(cctrl, into_reg),
                             AIWNIOS_TMP_IREG_POOP);
            AIWNIOS_ADD_CODE(X86SarImm, MIR(cctrl, into_reg),
                             __builtin_ffsll(next->integer) - 1);
            rpn->res.set_flags = 1;
            rpn->tmp_res = tmp;
            if (rpn->res.keep_in_tmp) {
              rpn->res = tmp;
              rpn->res.keep_in_tmp = 1;
            }
            code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
            break;
          default:
            next2->res.keep_in_tmp = 1;
            code_off = __OptPassFinal(cctrl, next2, bin, code_off);
            if (rpn->res.mode == MD_REG)
              into_reg = rpn->res.reg;
            else
              into_reg = RAX;
            tmp.mode = MD_REG;
            tmp.raw_type = RT_I64i;
            tmp.reg = into_reg;
            code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
            AIWNIOS_ADD_CODE(X86ShrImm, MIR(cctrl, into_reg),
                             __builtin_ffsll(next->integer) - 1);
            if (rpn->raw_type != RT_F64)
              rpn->res.set_flags = 1;
            break;
          }
          rpn->tmp_res = tmp;
          if (rpn->res.keep_in_tmp) {
            rpn->res = tmp;
            rpn->res.keep_in_tmp = 1;
          }
          code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
          break;
        }
    }
  div_normal:
    if (rpn->raw_type == RT_U64i) {
#define CQO_OP(use_reg, op, sib_op)                                            \
  next = ICArgN(rpn, 1);                                                       \
  next2 = ICArgN(rpn, 0);                                                      \
  orig_dst = next->res;                                                        \
  if (ModeIsDerefToSIB(next2) &&                                               \
      ((next2->raw_type == RT_F64) == (rpn->raw_type == RT_F64)) &&            \
      RawTypeIs64(next2->raw_type)) {                                          \
    code_off = __OptPassFinal(cctrl, next, bin, code_off);                     \
    code_off = DerefToICArg(cctrl, &derefed, next2, AIWNIOS_TMP_IREG_POOP2,    \
                            bin, code_off);                                    \
    tmp.raw_type = RT_I64i;                                                    \
    tmp.reg = RAX;                                                             \
    tmp.mode = MD_REG;                                                         \
    code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);                  \
    if (rpn->raw_type == RT_U64i) {                                            \
      AIWNIOS_ADD_CODE(X86XorReg, MIR(cctrl, RDX), RDX);                       \
    } else {                                                                   \
      MIR(cctrl, RAX);                                                         \
      MIR(cctrl, RDX);                                                         \
      AIWNIOS_ADD_CODE(X86Cqo, 0);                                             \
    }                                                                          \
    AIWNIOS_ADD_CODE(sib_op, derefed.__SIB_scale, derefed.reg2, derefed.reg,   \
                     derefed.off);                                             \
    tmp.reg = use_reg;                                                         \
    rpn->tmp_res = tmp;                                                        \
    if (rpn->res.keep_in_tmp) {                                                \
      rpn->res = tmp;                                                          \
      rpn->res.keep_in_tmp = 1;                                                \
    }                                                                          \
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                   \
    break;                                                                     \
  }                                                                            \
  tmp.raw_type = RT_I64i;                                                      \
  tmp.reg = RAX;                                                               \
  tmp.mode = MD_REG;                                                           \
  code_off = __OptPassFinal(cctrl, next, bin, code_off);                       \
  code_off = __OptPassFinal(cctrl, next2, bin, code_off);                      \
  code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);                    \
  code_off = PutICArgIntoReg(cctrl, &next2->res, RT_U64i, R8, bin, code_off);  \
  if (rpn->raw_type == RT_U64i) {                                              \
    AIWNIOS_ADD_CODE(X86XorReg, MIR(cctrl, RDX), RDX);                         \
  } else {                                                                     \
    MIR(cctrl, RAX);                                                           \
    MIR(cctrl, RDX);                                                           \
    AIWNIOS_ADD_CODE(X86Cqo, 0);                                               \
  }                                                                            \
  AIWNIOS_ADD_CODE(op, next2->res.reg);                                        \
  tmp.reg = use_reg;                                                           \
  rpn->tmp_res = tmp;                                                          \
  if (rpn->res.keep_in_tmp) {                                                  \
    rpn->res = tmp;                                                            \
    rpn->res.keep_in_tmp = 1;                                                  \
  }                                                                            \
  code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      CQO_OP(RAX, X86DivReg, X86DivSIB64);
    } else if (rpn->raw_type != RT_F64) {
      CQO_OP(RAX, X86IDivReg, X86IDivSIB64);
    } else {
      BACKEND_BINOP(X86DivSDReg, X86DivSDReg, X86DivSDSIB64, X86DivSDSIB64);
    }
    break;
  ic_mul:
    next = rpn->base.next;
    next2 = ICArgN(rpn, 1);
    if (next->type == IC_I64 && next2->raw_type != RT_F64) {
      if (__builtin_popcountll(next->integer) == 1) {
        next2->res.keep_in_tmp = 1;
        code_off = __OptPassFinal(cctrl, next2, bin, code_off);
        code_off = PutICArgIntoReg(cctrl, &next2->res, next2->raw_type,
                                   AIWNIOS_TMP_IREG_POOP, bin, code_off);
        if (rpn->res.mode == MD_REG) {
          code_off = ICMov(cctrl, &rpn->res, &next2->res, bin, code_off);
          AIWNIOS_ADD_CODE(X86ShlImm, MIR(cctrl, rpn->res.reg),
                           __builtin_ffsll(next->integer) - 1);
        } else {
          tmp.raw_type = RT_I64i;
          tmp.reg = RAX;
          tmp.mode = MD_REG;
          code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
          AIWNIOS_ADD_CODE(X86ShlImm, MIR(cctrl, tmp.reg),
                           __builtin_ffsll(next->integer) - 1);
          rpn->tmp_res = tmp;
          if (rpn->res.keep_in_tmp) {
            rpn->res = tmp;
            rpn->res.keep_in_tmp = 1;
          }
          code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
        }
        break;
      }
    }
    if (rpn->raw_type == RT_U64i) {
      CQO_OP(RAX, X86MulReg, X86MulSIB64);
    } else {
      BACKEND_BINOP(X86MulSDReg, X86IMul2Reg, X86MulSDSIB64, X86IMul2SIB64);
    }
    break;
  ic_deref:
    if (rpn->flags & ICF_SIB) {
      if (rpn->res.__sib_base_rpn)
        code_off =
            __OptPassFinal(cctrl, rpn->res.__sib_base_rpn, bin, code_off);
      if (rpn->res.__sib_idx_rpn)
        code_off = __OptPassFinal(cctrl, rpn->res.__sib_idx_rpn, bin, code_off);
      goto ret;
    }
    next = rpn->base.next;
    i = 0;
    //
    // Here's the Donald Trump deal,Derefernced function pointers
    // don't derefence(you dont copy a function into memory).
    //
    // I64 (*fptr)()=&Foo;
    // (*fptr)(); //Doesn't "dereference"
    //
    if (rpn->raw_type == RT_FUNC) {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
      goto ret;
    }
    code_off =
        DerefToICArg(cctrl, &tmp, rpn, AIWNIOS_TMP_IREG_POOP2, bin, code_off);
    // Only keep in registers
    // if (rpn->res.keep_in_tmp)
    //   rpn->res = tmp;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_and:
    if (rpn->raw_type != RT_F64)
      rpn->res.set_flags = 1;
    BACKEND_BINOP_IMM(X86AndImm, X86AndImm);
    BACKEND_BINOP(X86AndPDReg, X86AndReg, X86AndPDSIB64, X86AndSIB64);
    break;
  ic_addr_of:
    next = rpn->base.next;
    switch (next->type) {
      break;
    case __IC_STATIC_REF:
      if (rpn->res.mode == MD_REG)
        into_reg = rpn->res.reg;
      else
        into_reg = 0;
      AIWNIOS_ADD_CODE(X86LeaSIB, MIR(cctrl, into_reg), -1, -1, RIP, 0);
      if (bin)
        CodeMiscAddRef(cctrl->statics_label, bin + code_off - 4)->offset =
            next->integer;
      goto restore_reg;
      break;
    case IC_DEREF:
      next = rpn->base.next;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_PTR,
                                 AIWNIOS_TMP_IREG_POOP2, bin, code_off);
      code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
      break;
    case IC_BASE_PTR:
      if (rpn->res.mode == MD_REG)
        into_reg = rpn->res.reg;
      else
        into_reg = 0;
      AIWNIOS_ADD_CODE(X86LeaSIB, MIR(cctrl, into_reg), -1, -1, RBP,
                       -next->integer);
      goto restore_reg;
      break;
    case IC_STATIC:
      if (rpn->res.mode == MD_REG)
        into_reg = rpn->res.reg;
      else
        into_reg = 0;
      AIWNIOS_ADD_CODE(X86LeaSIB, MIR(cctrl, into_reg), -1, -1, RIP,
                       (char *)rpn->integer - (char *)(bin + code_off));
      goto restore_reg;
    case IC_GLOBAL:
    get_glbl_ptr:
      if (rpn->res.mode == MD_REG)
        into_reg = rpn->res.reg;
      else
        into_reg = 0;
      if (next->global_var->base.type & HTT_GLBL_VAR) {
        enter_addr = next->global_var->data_addr;
      } else if (next->global_var->base.type & HTT_FUN) {
        enter_addr = ((CHashFun *)next->global_var)->fun_ptr;
      } else
        abort();
      // Undefined?
      if (!enter_addr || enter_addr == &DoNothing) {
        misc = AddRelocMisc(cctrl, next->global_var->base.str);
        AIWNIOS_ADD_CODE(
            X86MovRegIndirI64, MIR(cctrl, into_reg), -1, -1, RIP,
            X86MovRegIndirI64(NULL, into_reg, -1, -1, RIP,
                              0)); // X86MovRegIndirI64 will return inst size
        CodeMiscAddRef(misc, bin + code_off - 4);
        goto restore_reg;
      } else if (Is32Bit(enter_addr - (char *)MemGetExecPtr(bin + code_off))) {
        AIWNIOS_ADD_CODE(X86LeaSIB, MIR(cctrl, into_reg), -1, -1, RIP,
                         enter_addr - (char *)MemGetExecPtr(bin + code_off));
        goto restore_reg;
      }
      code_off =
          __ICMoveI64(cctrl, into_reg, (int64_t)enter_addr, bin, code_off);
    restore_reg:
      tmp.mode = MD_REG;
      tmp.raw_type = rpn->raw_type;
      tmp.reg = into_reg;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      break;
    default:
      abort();
    }
    break;
  ic_xor:
    if (rpn->raw_type != RT_F64) {
      rpn->res.set_flags = 1;
      BACKEND_BINOP_IMM(X86XorImm, X86XorReg);
    }
    BACKEND_BINOP(X86XorPDReg, X86XorReg, X86XorPDSIB64, X86XorSIB64);
    break;
  ic_mod:
    next = ICArgN(rpn, 1);
    next2 = ICArgN(rpn, 0);
    if (rpn->raw_type != RT_F64) {
      if (IsConst(next2) && rpn->raw_type != RT_U64i) {
        if (__builtin_popcountll(ConstVal(next2)) == 1) {
          if (__builtin_ffsll(ConstVal(next2)) <= 31) {
            // https://compileroptimizations.com/category/integer_mod_optimization.htm
            int64_t j1;
            orig_dst = rpn->res;
            next->res.keep_in_tmp = 1;
            code_off = __OptPassFinal(cctrl, next, bin, code_off);
            code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
            code_off = PutICArgIntoReg(cctrl, &rpn->res, RT_I64i,
                                       AIWNIOS_TMP_IREG_POOP, bin, code_off);
            AIWNIOS_ADD_CODE(X86Test, rpn->res.reg, rpn->res.reg);
            AIWNIOS_ADD_CODE(X86Jcc, X86_COND_S, 0xffff);
            reverse = code_off;
            AIWNIOS_ADD_CODE(X86AndImm, MIR(cctrl, rpn->res.reg),
                             (1ll << (__builtin_ffsll(ConstVal(next2)) - 1)) -
                                 1);
            AIWNIOS_ADD_CODE(X86Jmp, 0xffff);
            j1 = code_off;
            if (bin)
              *(int32_t *)(bin + reverse - 4) = code_off - reverse;
            AIWNIOS_ADD_CODE(X86AndImm, MIR(cctrl, rpn->res.reg),
                             (1ll << (__builtin_ffsll(ConstVal(next2)) - 1)) -
                                 1);
            AIWNIOS_ADD_CODE(
                X86OrImm, MIR(cctrl, rpn->res.reg),
                ~((1ll << (__builtin_ffsll(ConstVal(next2)) - 1)) - 1));
            if (bin)
              *(int32_t *)(bin + j1 - 4) = code_off - j1;
            code_off = ICMov(cctrl, &orig_dst, &rpn->res, bin, code_off);
            break;
          }
        }
      } else {
        if (__builtin_popcountll(ConstVal(next2)) == 1) {
          if (__builtin_ffsll(ConstVal(next2)) <= 31) {
            orig_dst = rpn->res;
            next2->res.keep_in_tmp = 1;
            code_off = __OptPassFinal(cctrl, next, bin, code_off);
            code_off = ICMov(cctrl, &rpn->res, &next->res, bin, code_off);
            code_off = PutICArgIntoReg(cctrl, &rpn->res, RT_I64i,
                                       AIWNIOS_TMP_IREG_POOP, bin, code_off);
            AIWNIOS_ADD_CODE(X86AndImm, MIR(cctrl, rpn->res.reg),
                             (1ll << (__builtin_ffsll(ConstVal(next2)) - 1)) -
                                 1);
            code_off = ICMov(cctrl, &orig_dst, &rpn->res, bin, code_off);
            break;
          }
        }
      }
      if (rpn->raw_type == RT_U64i) {
        CQO_OP(RDX, X86DivReg, X86DivSIB64);
      } else {
        CQO_OP(RDX, X86IDivReg, X86IDivSIB64);
      }
    } else {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_F64, 1, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next->res, RT_F64, 0, bin, code_off);
      tmp2.reg = 2;
      tmp2.mode = MD_REG;
      tmp2.raw_type = RT_F64;
      code_off = ICMov(cctrl, &tmp2, &next->res, bin, code_off);
      AIWNIOS_ADD_CODE(X86DivSDReg, MFR(cctrl, tmp2.reg), next2->res.reg);
      AIWNIOS_ADD_CODE(X86CVTTSD2SIRegReg, MIR(cctrl, RAX), tmp2.reg);
      AIWNIOS_ADD_CODE(X86CVTTSI2SDRegReg, tmp2.reg, RAX);
      AIWNIOS_ADD_CODE(X86MulSDReg, tmp2.reg, next2->res.reg);
      tmp.reg = 3;
      tmp.mode = MD_REG;
      tmp.raw_type = RT_F64;
      code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off); // tmp.reg==0
      AIWNIOS_ADD_CODE(X86SubSDReg, MFR(cctrl, tmp.reg), tmp2.reg);
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_or:
    if (rpn->raw_type != RT_F64)
      rpn->res.set_flags = 1;
    BACKEND_BINOP_IMM(X86OrImm, X86OrReg);
    BACKEND_BINOP(X86OrPDReg, X86OrReg, X86OrPDSIB64, X86OrSIB64);
    break;
  ic_lt:
  ic_gt:
  ic_le:
  ic_ge:
    //
    // Ask nroot how this works if you want to know
    //
    // X/V3 is for lefthand side
    // X/V4 is for righthand side
    range = NULL;
    range_args = NULL;
    range_fail_addrs = NULL;
    range_cmp_types = NULL;
  get_range_items:
    cnt = 0;
    for (next = rpn; 1; next = ICFwd(next->base.next) // TODO explain
    ) {
      switch (next->type) {
      case IC_LT:
      case IC_GT:
      case IC_LE:
      case IC_GE:
        goto cmp_pass;
      }
      if (range_args)
        range_args[cnt] = next; // See below
      break;
    cmp_pass:
      //
      // <
      //    right rpn.next
      //    left  ICFwd(rpn.next)
      //
      if (range_args)
        range_args[cnt] = next->base.next;
      if (range)
        range[cnt] = next;
      cnt++;
    }
    if (!range) {
      // This are reversed
      range_args = A_MALLOC(sizeof(CRPN *) * (cnt + 1), cctrl->hc);
      range = A_MALLOC(sizeof(CRPN *) * cnt, cctrl->hc);
      range_fail_addrs = A_MALLOC(sizeof(CRPN *) * cnt, cctrl->hc);
      range_cmp_types = A_MALLOC(sizeof(int64_t) * cnt, cctrl->hc);
      goto get_range_items;
    }
    code_off = __OptPassFinal(cctrl, next = range_args[cnt], bin, code_off);
    for (i = cnt - 1; i >= 0; i--) {
      next2 = range_args[i];
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      memcpy(&tmp, &next->res, sizeof(CICArg));
      memcpy(&tmp2, &next2->res, sizeof(CICArg));
      use_flt_cmp = next->raw_type == RT_F64 || next2->raw_type == RT_F64;
      i2 = RT_I64i;
      i2 = next->res.raw_type > i2 ? next->res.raw_type : i2;
      i2 = next2->res.raw_type > i2 ? next2->res.raw_type : i2;
      code_off = PutICArgIntoReg(cctrl, &tmp, i2, AIWNIOS_TMP_IREG_POOP2, bin,
                                 code_off);
      code_off = PutICArgIntoReg(cctrl, &tmp2, i2, 0, bin, code_off);
      if (use_flt_cmp) {
        AIWNIOS_ADD_CODE(X86COMISDRegReg, tmp.reg, tmp2.reg);
      } else {
        AIWNIOS_ADD_CODE(X86CmpRegReg, tmp.reg, tmp2.reg);
      }
      //
      // We use the opposite compare because if fail we jump to the fail
      // zone
      //
      if (use_flt_cmp) {
      unsigned_cmp:
        switch (range[i]->type) {
          break;
        case IC_LT:
          range_cmp_types[i] = X86_COND_B | 1;
          break;
        case IC_GT:
          range_cmp_types[i] = X86_COND_A | 1;
          break;
        case IC_LE:
          range_cmp_types[i] = X86_COND_BE | 1;
          break;
        case IC_GE:
          range_cmp_types[i] = X86_COND_AE | 1;
        }
      } else if (next->raw_type == RT_U64i || next2->raw_type == RT_U64i) {
        goto unsigned_cmp;
      } else {
        switch (range[i]->type) {
          break;
        case IC_LT:
          range_cmp_types[i] = X86_COND_L | 1;
          break;
        case IC_GT:
          range_cmp_types[i] = X86_COND_G | 1;
          break;
        case IC_LE:
          range_cmp_types[i] = X86_COND_LE | 1;
          break;
        case IC_GE:
          range_cmp_types[i] = X86_COND_GE | 1;
        }
      }
      range_fail_addrs[i] = bin + code_off;
      AIWNIOS_ADD_CODE(X86Jcc, range_cmp_types[i],
                       0); // WILL BE FILLED IN LATER
      if (old_fail_misc)
        CodeMiscAddRef(old_fail_misc, bin + code_off - 4);
      next = next2;
    }
    if (!(old_fail_misc && old_pass_misc)) {
      tmp.mode = MD_I64;
      tmp.integer = 1;
      tmp.raw_type = RT_I64i;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      exit_addr = bin + code_off;
      AIWNIOS_ADD_CODE(X86Jmp, 0);
      if (bin)
        for (i = 0; i != cnt; i++) {
          X86Jcc(range_fail_addrs[i], range_cmp_types[i],
                 (bin + code_off) -
                     (char *)range_fail_addrs[i]); // TODO unsigned
        }
      tmp.mode = MD_I64;
      tmp.integer = 0;
      tmp.raw_type = RT_I64i;
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      if (bin)
        X86Jmp(exit_addr, (bin + code_off) - exit_addr);
    } else {
      if (!(rpn->flags & ICF_NO_JUMP)) {
        AIWNIOS_ADD_CODE(X86Jmp, 0);
        CodeMiscAddRef(old_pass_misc, bin + code_off - 4)->from_ic = rpn;
      }
    }
    A_FREE(range);
    A_FREE(range_args);
    A_FREE(range_fail_addrs);
    A_FREE(range_cmp_types);
    break;
  ic_lnot:
    next = rpn->base.next;
    next->res.keep_in_tmp = 1;
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    code_off =
        PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 0, bin, code_off);
    if (next->res.raw_type != RT_F64) {
      AIWNIOS_ADD_CODE(X86Test, next->res.reg, next->res.reg);
    } else {
      code_off = __ICMoveF64(cctrl, 1, 0, bin, code_off);
      AIWNIOS_ADD_CODE(X86COMISDRegReg, 1, 0);
    }
    if (old_pass_misc && old_fail_misc) {
      AIWNIOS_ADD_CODE(X86Jcc, X86_COND_Z, 0);
      if (bin)
        CodeMiscAddRef(old_pass_misc, bin + code_off - 4);
      if (!(rpn->flags & ICF_NO_JUMP)) {
        AIWNIOS_ADD_CODE(X86Jmp, 0);
        if (bin)
          CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->from_ic = rpn;
      }
      goto ret;
    }
    if (rpn->res.mode == MD_REG && rpn->res.mode != RT_F64) {
      into_reg = rpn->res.reg;
    } else
      into_reg = 0; // Store into reg 0
    AIWNIOS_ADD_CODE(X86Setcc, X86_COND_Z, MIR(cctrl, into_reg));
    if (!(rpn->res.mode == MD_REG && rpn->res.mode != RT_F64)) {
      tmp.raw_type = RT_I64i;
      tmp.reg = 0;
      tmp.mode = MD_REG;
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_bnot:
    if (rpn->raw_type == RT_F64) {
      next->res.keep_in_tmp = 1;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 1, bin, code_off);
      static double ul;
      *(int64_t *)&ul = ~0ll;
      code_off = __ICMoveF64(cctrl, 2, ul, bin, code_off);
      AIWNIOS_ADD_CODE(X86XorPDReg, MFR(cctrl, next->res.reg), 2);
      goto ret;
    }
    if (rpn->raw_type != RT_F64)
      rpn->res.set_flags = 1;
    BACKEND_UNOP(X86NotReg, X86NotReg);
    break;
  ic_post_inc:
    code_off =
        __SexyPostOp(cctrl, rpn, X86AddImm32, X86AddReg, X86AddSDReg, X86IncReg,
                     X86IncIndirX, X86AddIndirXImm32, bin, code_off);
    break;
  ic_post_dec:
    code_off =
        __SexyPostOp(cctrl, rpn, X86SubImm32, X86SubReg, X86SubSDReg, X86DecReg,
                     X86DecIndirX, X86SubIndirXImm32, bin, code_off);
    break;
  ic_pre_inc:
    code_off =
        __SexyPreOp(cctrl, rpn, X86AddImm32, X86AddReg, X86AddSDReg, X86IncReg,
                    X86IncIndirX, X86AddIndirXImm32, bin, code_off);
    break;
  ic_pre_dec:
    code_off =
        __SexyPreOp(cctrl, rpn, X86SubImm32, X86SubReg, X86SubSDReg, X86DecReg,
                    X86DecIndirX, X86SubIndirXImm32, bin, code_off);
    break;
  ic_and_and:
#define BACKEND_TEST(_res)                                                     \
  {                                                                            \
    int64_t rt = (_res)->raw_type;                                             \
    code_off = PutICArgIntoReg(cctrl, (_res), rt, 0, bin, code_off);           \
    int64_t reg = (_res)->reg;                                                 \
    if ((rt) != RT_F64) {                                                      \
      AIWNIOS_ADD_CODE(X86Test, reg, reg);                                     \
    } else {                                                                   \
      code_off = __ICMoveF64(cctrl, 2, 0, bin, code_off);                      \
      AIWNIOS_ADD_CODE(X86COMISDRegReg, reg, 2);                               \
    }                                                                          \
  }
    if (old_pass_misc && old_fail_misc) {
      a = ICArgN(rpn, 1);
      b = ICArgN(rpn, 0);
      a->res.keep_in_tmp = 1;
      code_off = __OptPassFinal(cctrl, a, bin, code_off);
      BACKEND_TEST(&a->res);
      AIWNIOS_ADD_CODE(X86Jcc, X86_COND_Z, 6);
      CodeMiscAddRef(old_fail_misc, bin + code_off - 4);
      b->res.keep_in_tmp = 1;
      code_off = __OptPassFinal(cctrl, b, bin, code_off);
      BACKEND_TEST(&b->res);
      AIWNIOS_ADD_CODE(X86Jcc, X86_COND_Z, 6);
      CodeMiscAddRef(old_fail_misc, bin + code_off - 4);
      if (!(rpn->flags & ICF_NO_JUMP)) {
        AIWNIOS_ADD_CODE(X86Jmp, 0);
        CodeMiscAddRef(old_pass_misc, bin + code_off - 4)->from_ic = rpn;
      }
      goto ret;
    }
#define BACKEND_BOOLIFY(to, reg, rt)                                           \
  if ((rt) != RT_F64) {                                                        \
    AIWNIOS_ADD_CODE(X86Test, reg, reg);                                       \
    AIWNIOS_ADD_CODE(X86Setcc, X86_COND_Z | 1, MIR(cctrl, to));                \
  } else {                                                                     \
    code_off = __ICMoveF64(cctrl, 2, 0, bin, code_off);                        \
    AIWNIOS_ADD_CODE(X86COMISDRegReg, reg, 2);                                 \
    AIWNIOS_ADD_CODE(X86Setcc, X86_COND_E | 1, MIR(cctrl, to));                \
  }
    if (!rpn->code_misc)
      rpn->code_misc = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc2)
      rpn->code_misc2 = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc3)
      rpn->code_misc3 = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc4)
      rpn->code_misc4 = CodeMiscNew(cctrl, CMT_LABEL);
    a = ICArgN(rpn, 1);
    b = ICArgN(rpn, 0);
    cctrl->backend_user_data5 = (int64_t)rpn->code_misc;
    cctrl->backend_user_data6 = (int64_t)rpn->code_misc2;
    code_off = __OptPassFinal(cctrl, a, bin, code_off);
    cctrl->backend_user_data6 = (int64_t)rpn->code_misc3;
    rpn->code_misc2->addr = bin + code_off;
    code_off = __OptPassFinal(cctrl, b, bin, code_off);
    rpn->code_misc->addr = bin + code_off;
    tmp.mode = MD_I64;
    tmp.raw_type = RT_I64i;
    tmp.integer = 0;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    AIWNIOS_ADD_CODE(X86Jmp, 0);
    CodeMiscAddRef(rpn->code_misc4, bin + code_off - 4);
    rpn->code_misc3->addr = bin + code_off;
    tmp.integer = 1;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    rpn->code_misc4->addr = bin + code_off;
    break;
  ic_or_or:
    if (old_pass_misc || old_fail_misc) {
      a = ICArgN(rpn, 1);
      b = ICArgN(rpn, 0);
      code_off = __OptPassFinal(cctrl, a, bin, code_off);
      BACKEND_TEST(&a->res);
      AIWNIOS_ADD_CODE(X86Jcc, X86_COND_Z ^ 1, 6);
      CodeMiscAddRef(old_pass_misc, bin + code_off - 4);
      code_off = __OptPassFinal(cctrl, b, bin, code_off);
      BACKEND_TEST(&b->res);
      AIWNIOS_ADD_CODE(X86Jcc, X86_COND_Z ^ 1, 6);
      CodeMiscAddRef(old_pass_misc, bin + code_off - 4);
      if (!(rpn->flags & ICF_NO_JUMP)) {
        AIWNIOS_ADD_CODE(X86Jmp, 0);
        CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->from_ic = rpn;
      }
      goto ret;
    }
    // A
    // rpn->code_misc3:
    // B
    // rpn->code_misc4:
    // res=0;
    // JMP rpn->code_misc2
    // rpn->code_misc:
    // res=1
    // rpn->code_misc2:
    if (!rpn->code_misc)
      rpn->code_misc = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc2)
      rpn->code_misc2 = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc3)
      rpn->code_misc3 = CodeMiscNew(cctrl, CMT_LABEL);
    if (!rpn->code_misc4)
      rpn->code_misc4 = CodeMiscNew(cctrl, CMT_LABEL);
    a = ICArgN(rpn, 1);
    b = ICArgN(rpn, 0);
    cctrl->backend_user_data5 = (int64_t)rpn->code_misc3;
    cctrl->backend_user_data6 = (int64_t)rpn->code_misc;
    code_off = __OptPassFinal(cctrl, a, bin, code_off);
    rpn->code_misc3->addr = bin + code_off;
    cctrl->backend_user_data5 = (int64_t)rpn->code_misc4;
    code_off = __OptPassFinal(cctrl, b, bin, code_off);
    rpn->code_misc4->addr = bin + code_off;
    tmp.mode = MD_I64;
    tmp.integer = 0;
    tmp.raw_type = RT_I64i;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    if (!(rpn->flags & ICF_NO_JUMP)) {
      AIWNIOS_ADD_CODE(X86Jmp, 0);
      CodeMiscAddRef(rpn->code_misc2, bin + code_off - 4)->from_ic = rpn;
    }
    rpn->code_misc->addr = bin + code_off;
    tmp.integer = 1;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    rpn->code_misc2->addr = bin + code_off;
    break;
  ic_xor_xor:
#define BACKEND_LOGICAL_BINOP(op)                                              \
  next = ICArgN(rpn, 1);                                                       \
  next2 = ICArgN(rpn, 0);                                                      \
  code_off = __OptPassFinal(cctrl, next, bin, code_off);                       \
  code_off = __OptPassFinal(cctrl, next2, bin, code_off);                      \
  if (rpn->res.mode == MD_REG) {                                               \
    into_reg = rpn->res.reg;                                                   \
  } else                                                                       \
    into_reg = 0;                                                              \
  code_off = PutICArgIntoReg(cctrl, &next->res, rpn->raw_type,                 \
                             AIWNIOS_TMP_IREG_POOP2, bin, code_off);           \
  code_off = PutICArgIntoReg(cctrl, &next2->res, rpn->raw_type,                \
                             AIWNIOS_TMP_IREG_POOP, bin, code_off);            \
  BACKEND_BOOLIFY(AIWNIOS_TMP_IREG_POOP2, next->res.reg, next->res.raw_type);  \
  BACKEND_BOOLIFY(into_reg, next2->res.reg, next2->res.raw_type);              \
  AIWNIOS_ADD_CODE(op, MIR(cctrl, into_reg), AIWNIOS_TMP_IREG_POOP2);          \
  if (rpn->res.mode != MD_REG) {                                               \
    tmp.raw_type = rpn->raw_type;                                              \
    tmp.reg = 0;                                                               \
    tmp.mode = MD_REG;                                                         \
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);                   \
  }
    BACKEND_LOGICAL_BINOP(X86XorReg);
    break;
  ic_eq_eq:
#define BACKEND_CMP                                                            \
  next = ICArgN(rpn, 1);                                                       \
  next2 = ICArgN(rpn, 0);                                                      \
  code_off = __OptPassFinal(cctrl, next, bin, code_off);                       \
  code_off = __OptPassFinal(cctrl, next2, bin, code_off);                      \
  i = RT_I64i;                                                                 \
  if (next->raw_type == RT_F64 || next2->raw_type == RT_F64)                   \
    i = RT_F64;                                                                \
  code_off = PutICArgIntoReg(cctrl, &next2->res, i,                            \
                             (i == RT_F64) ? 1 : AIWNIOS_TMP_IREG_POOP2, bin,  \
                             code_off);                                        \
  code_off = PutICArgIntoReg(cctrl, &next->res, i, 0, bin, code_off);          \
  if (i == RT_F64) {                                                           \
    AIWNIOS_ADD_CODE(X86COMISDRegReg, next->res.reg, next2->res.reg);          \
  } else {                                                                     \
    AIWNIOS_ADD_CODE(X86CmpRegReg, next->res.reg, next2->res.reg);             \
  }
    BACKEND_CMP;
    if (old_pass_misc && old_fail_misc) {
      AIWNIOS_ADD_CODE(X86Jcc, X86_COND_Z, 0);
      CodeMiscAddRef(old_pass_misc, bin + code_off - 4);
      if (!(rpn->flags & ICF_NO_JUMP)) {
        AIWNIOS_ADD_CODE(X86Jmp, 0);
        CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->from_ic = rpn;
      }
    }
    if (use_flags)
      goto ret;
    if (rpn->res.mode == MD_REG && rpn->res.mode != RT_F64) {
      into_reg = rpn->res.reg;
    } else
      into_reg = 0;
    AIWNIOS_ADD_CODE(X86Setcc, X86_COND_E, MIR(cctrl, into_reg));
    if (!(rpn->res.mode == MD_REG && rpn->res.mode != RT_F64)) {
      tmp.raw_type = RT_I64i;
      tmp.reg = 0;
      tmp.mode = MD_REG;
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_ne:
    BACKEND_CMP;
    if (old_fail_misc && old_pass_misc) {
      AIWNIOS_ADD_CODE(X86Jcc, X86_COND_Z ^ 1, 0);
      CodeMiscAddRef(old_pass_misc, bin + code_off - 4);
      if (!(rpn->flags & ICF_NO_JUMP)) {
        AIWNIOS_ADD_CODE(X86Jmp, 0);
        CodeMiscAddRef(old_fail_misc, bin + code_off - 4)->from_ic = rpn;
      }
    }
    if (use_flags)
      goto ret;
    if (rpn->res.mode == MD_REG && rpn->res.mode != RT_F64) {
      into_reg = rpn->res.reg;
    } else
      into_reg = 0;
    AIWNIOS_ADD_CODE(X86Setcc, X86_COND_E | 1, MIR(cctrl, into_reg));
    if (!(rpn->res.mode == MD_REG && rpn->res.mode != RT_F64)) {
      tmp.raw_type = RT_I64i;
      tmp.reg = 0;
      tmp.mode = MD_REG;
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_lsh:
    if (rpn->raw_type != RT_F64) {
      next2 = ICArgN(rpn, 0);
      next = ICArgN(rpn, 1);
      if (IsConst(next2) && Is32Bit(ConstVal(next2))) {
        if (rpn->raw_type != RT_F64)
          rpn->res.set_flags = 1;
        BACKEND_BINOP_IMM(X86ShlImm,
                          X86AddReg); // X86AddReg wont be used here
        break;
      }
      tmp.raw_type = RT_I64i;
      tmp.reg = RCX;
      tmp.mode = MD_REG;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next2->res, RT_U64i, RCX, bin, code_off);
      tmp2.mode = MD_REG; // TODO IMPROVE
      tmp2.raw_type = RT_PTR;
      tmp2.reg = 0;
      code_off = ICMov(cctrl, &tmp2, &next->res, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &tmp2, RT_U64i, AIWNIOS_TMP_IREG_POOP,
                                 bin, code_off);
      code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
      AIWNIOS_ADD_CODE(X86ShlRCX, MIR(cctrl, tmp2.reg));
      if (rpn->raw_type != RT_F64)
        rpn->res.set_flags = 1;
      rpn->tmp_res = tmp2;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp2;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
    } else {
      next = ICArgN(rpn, 1);
      next2 = ICArgN(rpn, 0);
      if (IsConst(next2) && Is32Bit(ConstVal(next2))) {
        if (rpn->raw_type != RT_F64)
          rpn->res.set_flags = 1;
        BACKEND_BINOP_IMM(X86ShlImm,
                          X86AddReg); // X86AddReg wont be used here
      }
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 1, bin, code_off);
      code_off = PushToStack(cctrl, &next->res, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i, 0, bin, code_off);
      tmp.raw_type = RT_I64i;
      tmp.mode = MD_REG;
      tmp.reg = RAX;
      code_off = PopFromStack(cctrl, &next->res, bin, code_off);
      AIWNIOS_ADD_CODE(X86ShlImm, MIR(cctrl, tmp.reg), next2->res.integer);
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    }
    break;
  ic_rsh:
    next = ICArgN(rpn, 1);
    next2 = ICArgN(rpn, 0);
    if (rpn->raw_type == RT_F64) {
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next->res, rpn->raw_type, 0, bin, code_off);
      code_off = PushToStack(cctrl, &next->res, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &next2->res, RT_I64i,
                                 AIWNIOS_TMP_IREG_POOP2, bin, code_off);
      tmp.raw_type = RT_I64i;
      tmp.mode = MD_REG;
      tmp.reg = RAX;
      code_off = PopFromStack(cctrl, &next->res, bin, code_off);
      AIWNIOS_ADD_CODE(X86ShrImm, MIR(cctrl, tmp.reg), next2->res.reg);
      if (rpn->raw_type != RT_F64)
        rpn->res.set_flags = 1;
      rpn->tmp_res = tmp;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
      break;
    }
    switch (rpn->raw_type) {
    case RT_U8i:
    case RT_U16i:
    case RT_U32i:
    case RT_U64i:
      if (IsConst(next2) && Is32Bit(ConstVal(next2))) {
        if (rpn->raw_type != RT_F64)
          rpn->res.set_flags = 1;
        BACKEND_BINOP_IMM(X86ShrImm,
                          X86AddReg); // X86AddReg wont be used here
      }
      next2 = ICArgN(rpn, 0);
      next = ICArgN(rpn, 1);
      tmp.raw_type = RT_I64i;
      tmp.reg = RCX;
      tmp.mode = MD_REG;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next2->res, RT_U64i, RCX, bin, code_off);
      tmp2.mode = MD_REG; // TODO IMPROVE
      tmp2.raw_type = RT_PTR;
      tmp2.reg = 0;
      code_off = ICMov(cctrl, &tmp2, &next->res, bin, code_off);
      // POOP2 is RCX
      code_off = PutICArgIntoReg(cctrl, &tmp2, RT_U64i, AIWNIOS_TMP_IREG_POOP,
                                 bin, code_off);
      code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
      AIWNIOS_ADD_CODE(X86ShrRCX, MIR(cctrl, tmp.reg2));
      if (rpn->raw_type != RT_F64)
        rpn->res.set_flags = 1;
      rpn->tmp_res = tmp2;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp2;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
      break;
    default:
      if (IsConst(next2) && Is32Bit(ConstVal(next2))) {
        if (rpn->raw_type != RT_F64)
          rpn->res.set_flags = 1;
        BACKEND_BINOP_IMM(X86SarImm,
                          X86AddReg); // X86AddReg wont be used here
      }
      tmp.raw_type = RT_I64i;
      tmp.reg = RCX;
      tmp.mode = MD_REG;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      code_off = __OptPassFinal(cctrl, next2, bin, code_off);
      code_off =
          PutICArgIntoReg(cctrl, &next2->res, RT_U64i, RCX, bin, code_off);
      tmp2.mode = MD_REG; // TODO IMPROVE
      tmp2.raw_type = RT_PTR;
      tmp2.reg = 0;
      code_off = ICMov(cctrl, &tmp2, &next->res, bin, code_off);
      code_off = PutICArgIntoReg(cctrl, &tmp2, RT_U64i, AIWNIOS_TMP_IREG_POOP,
                                 bin, code_off);
      code_off = ICMov(cctrl, &tmp, &next2->res, bin, code_off);
      AIWNIOS_ADD_CODE(X86SarRCX, MIR(cctrl, tmp2.reg));
      if (rpn->raw_type != RT_F64)
        rpn->res.set_flags = 1;
      rpn->tmp_res = tmp2;
      if (rpn->res.keep_in_tmp) {
        rpn->res = tmp2;
        rpn->res.keep_in_tmp = 1;
      }
      code_off = ICMov(cctrl, &rpn->res, &tmp2, bin, code_off);
    }
    break;
  ic_call:
    //
    // ()_()_()_()
    // /          \     // When we call a function,all temp registers
    // | O______O |     // are invalidated,so we can reset the temp register
    // | \______/ |===> // counts (for the new expressions).
    // \__________/     //
    //   \./   \./      // We will restore the temp register counts after the
    //   call
    //
    i = cctrl->backend_user_data2;
    i2 = cctrl->backend_user_data3;
    cctrl->backend_user_data2 = 0;
    cctrl->backend_user_data3 = 0;
    code_off = __ICFCallTOS(cctrl, rpn, bin, code_off);
    cctrl->backend_user_data2 = i;
    cctrl->backend_user_data3 = i2;
    break;
  ic_vargs:
    //  ^^^^^^^^
    // (        \      ___________________
    // ( (*) (*)|     /                   \
    //  \   ^   |    |__IFCall            |
    //   \  <>  | <==| Will unwind for you| //TODO validate for X86_64
    //    \    /      \__________________/
    //      \_/
    for (i = rpn->length - 1; i >= 0; i--) {
      next = ICArgN(rpn, rpn->length - i - 1);
      next->res.keep_in_tmp = 1;
      code_off = __OptPassFinal(cctrl, next, bin, code_off);
      tmp = next->res;
      code_off = PushToStack(cctrl, &tmp, bin, code_off);
    }
    break;
  ic_add_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_sub_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_mul_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_div_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_lsh_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_rsh_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_and_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_or_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_xor_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
    abort();
    break;
    abort();
    break;
  ic_mod_eq:
    code_off = __SexyAssignOp(cctrl, rpn, bin, code_off);
    break;
  ic_i64:
    tmp.mode = MD_I64;
    tmp.raw_type = RT_I64i;
    tmp.integer = rpn->integer;
    if (rpn->res.mode != MD_I64)
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_f64:
    tmp.mode = MD_F64;
    tmp.raw_type = RT_F64;
    tmp.flt = rpn->flt;
    if (rpn->res.mode != MD_F64)
      code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_array_acc:
    abort();
    break;
  ic_ret:
    next = ICArgN(rpn, 0);
    PushTmpDepthFirst(cctrl, next, 0);
    PopTmp(cctrl, next);
    next->res.keep_in_tmp = 1;
    if (next->res.mode == MD_REG) // Save a move
      next->res.reg = 0;
    code_off = __OptPassFinal(cctrl, next, bin, code_off);
    if (cctrl->cur_fun)
      tmp.raw_type = cctrl->cur_fun->return_class->raw_type;
    else if (rpn->ic_class) {
      // No return type so just return what we have
      tmp.raw_type = rpn->ic_class->raw_type;
    } else
      // No return type so just return what we have
      tmp.raw_type = next->raw_type;
    tmp.reg = 0; // 0 is return register
    tmp.mode = MD_REG;
    code_off = ICMov(cctrl, &tmp, &next->res, bin, code_off);
    // TempleOS will always store F64 result in RAX(integer register)
    // Let's merge the two togheter
    if (tmp.raw_type == RT_F64)
      AIWNIOS_ADD_CODE(X86MovQI64F64, MIR(cctrl, 0), 0);
    {
      // You need to always jump to epilog to avoid pass-through(A function with
      // no registers can stil have multiple returns)
      if (bin) {
        if (!(rpn->flags & ICF_NO_JUMP)) {
          AIWNIOS_ADD_CODE(X86Jmp, 0);
          CodeMiscAddRef(cctrl->epilog_label, bin + code_off - 4)->from_ic =
              rpn;
        }
      }
    }
    break;
  ic_base_ptr:
    tmp.raw_type = rpn->raw_type;
    tmp.off = rpn->integer;
    tmp.reg = RBP;
    tmp.mode = MD_FRAME;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_static:
    tmp.raw_type = rpn->raw_type;
    tmp.off = rpn->integer;
    tmp.mode = MD_PTR;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_ireg:
    tmp.raw_type = RT_I64i;
    tmp.reg = rpn->integer;
    tmp.mode = MD_REG;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_freg:
    tmp.raw_type = RT_F64;
    tmp.reg = rpn->integer;
    tmp.mode = MD_REG;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
    abort();
    break;
    abort();
    break;
  ic_bt:
#define BTX_OP(lock, f_imm_sib, f_imm_reg, f_sib, f_reg)                       \
  a = ICArgN(rpn, 1);                                                          \
  b = ICArgN(rpn, 0);                                                          \
  if (IsConst(b) && ConstVal(b) <= 63) {                                       \
    if (a->res.mode == __MD_X86_64_LEA_SIB) {                                  \
      if (a->res.__sib_base_rpn) {                                             \
        code_off =                                                             \
            __OptPassFinal(cctrl, a->res.__sib_base_rpn, bin, code_off);       \
        a->res.reg = a->res.__sib_base_rpn->res.reg;                           \
      }                                                                        \
      if (a->res.__sib_idx_rpn) {                                              \
        code_off = __OptPassFinal(cctrl, a->res.__sib_idx_rpn, bin, code_off); \
        a->res.reg2 = a->res.__sib_idx_rpn->res.reg;                           \
      }                                                                        \
      if (lock)                                                                \
        AIWNIOS_ADD_CODE(X86Lock, 0);                                          \
      AIWNIOS_ADD_CODE(f_imm_sib, ConstVal(b), a->res.__SIB_scale,             \
                       a->res.reg2, a->res.reg, a->res.off);                   \
    } else {                                                                   \
      a->res.keep_in_tmp = 1;                                                  \
      code_off = __OptPassFinal(cctrl, a, bin, code_off);                      \
      code_off = PutICArgIntoReg(cctrl, &a->res, RT_I64i,                      \
                                 AIWNIOS_TMP_IREG_POOP2, bin, code_off);       \
      if (lock)                                                                \
        AIWNIOS_ADD_CODE(X86Lock, 0);                                          \
      AIWNIOS_ADD_CODE(f_imm_sib, ConstVal(b), -1, -1, a->res.reg, 0);         \
    }                                                                          \
  } else {                                                                     \
    code_off = __OptPassFinal(cctrl, b, bin, code_off);                        \
    if (a->res.mode == __MD_X86_64_LEA_SIB) {                                  \
      if (a->res.__sib_base_rpn) {                                             \
        code_off =                                                             \
            __OptPassFinal(cctrl, a->res.__sib_base_rpn, bin, code_off);       \
        a->res.reg = a->res.__sib_base_rpn->res.reg;                           \
      }                                                                        \
      if (a->res.__sib_idx_rpn) {                                              \
        code_off = __OptPassFinal(cctrl, a->res.__sib_idx_rpn, bin, code_off); \
        a->res.reg2 = a->res.__sib_idx_rpn->res.reg;                           \
      }                                                                        \
      code_off = PutICArgIntoReg(cctrl, &b->res, RT_I64i,                      \
                                 AIWNIOS_TMP_IREG_POOP, bin, code_off);        \
      if (lock)                                                                \
        AIWNIOS_ADD_CODE(X86Lock, 0);                                          \
      AIWNIOS_ADD_CODE(f_sib, MFR(cctrl, b->res.reg), a->res.__SIB_scale,      \
                       a->res.reg2, a->res.reg, a->res.off);                   \
    } else {                                                                   \
      code_off = __OptPassFinal(cctrl, a, bin, code_off);                      \
      code_off = PutICArgIntoReg(cctrl, &a->res, RT_I64i,                      \
                                 AIWNIOS_TMP_IREG_POOP2, bin, code_off);       \
      code_off = PutICArgIntoReg(cctrl, &b->res, RT_I64i,                      \
                                 AIWNIOS_TMP_IREG_POOP, bin, code_off);        \
      if (lock)                                                                \
        AIWNIOS_ADD_CODE(X86Lock, 0);                                          \
      AIWNIOS_ADD_CODE(f_sib, MIR(cctrl, b->res.reg), -1, -1, a->res.reg, 0);  \
    }                                                                          \
  }                                                                            \
  if (old_fail_misc && old_pass_misc) {                                        \
    AIWNIOS_ADD_CODE(X86Jcc, X86_COND_C, 0);                                   \
    CodeMiscAddRef(old_pass_misc, bin + code_off - 4);                         \
    AIWNIOS_ADD_CODE(X86Jmp, 0);                                               \
    CodeMiscAddRef(old_fail_misc, bin + code_off - 4);                         \
    goto ret;                                                                  \
  }                                                                            \
  if (rpn->res.mode != MD_NULL) {                                              \
    into_reg = 0;                                                              \
    if (rpn->res.mode == MD_REG)                                               \
      into_reg = rpn->res.reg;                                                 \
    AIWNIOS_ADD_CODE(X86Setcc, X86_COND_C, MIR(cctrl, into_reg));              \
    goto restore_reg;                                                          \
  }
    BTX_OP(0, X86BtSIBImm, X86BtRegImm, X86BtSIBReg, X86BtRegReg);
    break;
  ic_btc:
    BTX_OP(0, X86BtcSIBImm, X86BtcRegImm, X86BtcSIBReg, X86BtcRegReg);
    break;
  ic_bts:
    BTX_OP(0, X86BtsSIBImm, X86BtsRegImm, X86BtsSIBReg, X86BtsRegReg);
    break;
  ic_btr:
    BTX_OP(0, X86BtrSIBImm, X86BtrRegImm, X86BtrSIBReg, X86BtrRegReg);
    break;
  ic_lbtc:
    BTX_OP(1, X86BtcSIBImm, X86BtcRegImm, X86BtcSIBReg, X86BtcRegReg);
    break;
  ic_lbts:
    BTX_OP(1, X86BtsSIBImm, X86BtsRegImm, X86BtsSIBReg, X86BtsRegReg);
    break;
  ic_lbtr:
    BTX_OP(1, X86BtrSIBImm, X86BtrRegImm, X86BtrSIBReg, X86BtrRegReg);
  ic_sign_i64:
  ic_sqr_i64:
  ic_sqr_u64:
  ic_sqr:
  ic_abs:
  ic_cos:
  ic_sin:
  ic_tan:
  ic_atan:
    /*break;case IC_MAX_I64:
    break;case IC_MIN_I64:
    break;case IC_MAX_U64:
    break;case IC_MIN_U64:
    break;case IC_SIGN_I64:
    break;case IC_SQR_I64:
    break;case IC_SQR_U64:
    break;case IC_SQR:
    break;case IC_ABS:
    break;case IC_SQRT:
    break;case IC_SIN:
    break;case IC_COS:
    break;case IC_TAN:
    break;case IC_ATAN:*/
    break;
  ic_static_ref:
    tmp.raw_type = rpn->raw_type;
    tmp.off = rpn->integer;
    tmp.mode = MD_STATIC;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
    break;
  ic_set_static_data:
    // TODO later
    break;
  ic_raw_bytes:
    if (cctrl->code_ctrl->final_pass) {
      memcpy(bin + code_off, rpn->raw_bytes, rpn->length);
    }
    code_off += rpn->length;
  } while (0);
ret:
  if (old_fail_misc || old_pass_misc) {
    misc = old_fail_misc;
    misc2 = old_pass_misc;
    switch (rpn->type) {
    case IC_OR_OR:
    case IC_AND_AND:
    case IC_LT:
    case IC_GT:
    case IC_GE:
    case IC_LE:
    case IC_EQ_EQ:
    case IC_NE:
    case IC_LNOT:
    case IC_BT:
    case IC_BTS:
    case IC_BTR:
    case IC_BTC:
    case IC_LBTS:
    case IC_LBTR:
    case IC_LBTC:
      break;
    default:
      if (rpn->res.raw_type == RT_F64) {
        code_off = PutICArgIntoReg(cctrl, &rpn->res, RT_F64, 1, bin, code_off);
        code_off = __ICMoveF64(cctrl, 2, 0, bin, code_off);
        AIWNIOS_ADD_CODE(X86COMISDRegReg, rpn->res.reg, 2);
      } else {
        tmp = rpn->res;
        code_off = PutICArgIntoReg(cctrl, &tmp, RT_I64i, RAX, bin, code_off);
        AIWNIOS_ADD_CODE(X86Test, tmp.reg, tmp.reg);
      }
      // Pass address
      AIWNIOS_ADD_CODE(X86Jcc, X86_COND_Z | 1, 0);
      CodeMiscAddRef(misc2, bin + code_off - 4);
      // Fail address
      if (!(rpn->flags & ICF_NO_JUMP)) {
        AIWNIOS_ADD_CODE(X86Jmp, 5);
        CodeMiscAddRef(misc, bin + code_off - 4)->from_ic = rpn;
      }
    }
  }
  if (rpn->flags & ICF_STUFF_IN_REG) {
    tmp = rpn->res;
    if (rpn->res.raw_type != RT_F64)
      rpn->res.raw_type = RT_I64i;
    else
      rpn->res.raw_type = RT_F64;
    rpn->res.mode = MD_REG;
    rpn->res.reg = rpn->stuff_in_reg;
    code_off = ICMov(cctrl, &rpn->res, &tmp, bin, code_off);
  }
  cctrl->backend_user_data5 = (int64_t)old_fail_misc;
  cctrl->backend_user_data6 = (int64_t)old_pass_misc;
  if (old_rpn) {
    old_rpn->changes_iregs2 |= rpn->changes_iregs;
    old_rpn->changes_fregs2 |= rpn->changes_fregs;
  }
  if (cctrl->code_ctrl->final_pass == 2)
    rpn->end_ptr = bin + code_off;
  cctrl->cur_rpn = old_rpn;
  return code_off;
}

// This dude calls __OptPassFinal 3 times.
// 1. Get size of WORST CASE compiled body,and generate any extra needed
// CCodeMisc's
//    This pass  will also asign CRPN->res with tmp registers/FRAME offsets
// 2. Fill in function body,accounting for not worst case jumps
// 3. Fill in the poo poo's
//
char *OptPassFinal(CCmpCtrl *cctrl, int64_t *res_sz, char **dbg_info,
                   CHeapCtrl *heap) {
  CacheRPNArgs(cctrl);
  int64_t code_off, run, idx, cnt = 0, cnt2, final_size, is_terminal;
  int64_t min_ln = 0, max_ln = 0, statics_sz = 0;
  char *bin = NULL, *xbin = NULL;
  char *ptr;
  CCodeMisc *misc;
  CCodeMiscRef *cm_ref, *cm_ref_tmp;
  CHashImport *import;
  CRPN *r;
  cctrl->fregs_restore_label = CodeMiscNew(cctrl, CMT_LABEL);
  cctrl->fregs_save_label = CodeMiscNew(cctrl, CMT_LABEL);
  cctrl->epilog_label = CodeMiscNew(cctrl, CMT_LABEL);
  cctrl->statics_label = CodeMiscNew(cctrl, CMT_LABEL);
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = r->base.next) {
    if (r->ic_line) {
      if (!min_ln)
        min_ln = r->ic_line;
      min_ln = (min_ln < r->ic_line) ? min_ln : r->ic_line;
      if (!max_ln)
        max_ln = r->ic_line;
      max_ln = (max_ln > r->ic_line) ? max_ln : r->ic_line;
    }
    if (r->type == IC_BASE_PTR) {
    } else if (r->type == __IC_STATICS_SIZE)
      statics_sz = r->integer;
  }
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = ICFwd(r)) {
    cnt++;
  }
  if (dbg_info) {
    // Dont allocate on cctrl->hc heap ctrl as we want to share our datqa
    cctrl->code_ctrl->dbg_info = dbg_info;
    cctrl->code_ctrl->min_ln = min_ln;
  }
  CRPN *forwards[cnt2 = cnt];
  cnt = 0;
  for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
       r = ICFwd(r)) {
    forwards[cnt2 - ++cnt] = r;
  }
  // We DONT reset the wiggle room
  cctrl->backend_user_data0 = 0;
  // 0 get maximum code size,
  // 1 compute bin
  for (run = 0; run < 3; run++) {
    cctrl->code_ctrl->final_pass = run;
    cctrl->backend_user_data1 = 0;
    cctrl->backend_user_data2 = 0;
    cctrl->backend_user_data3 = 0;
    if (run == 0) {
      code_off = 0;
      bin = NULL;
    } else if (run == 1) {
      xbin = A_CALLOC(256 + code_off, heap ?: Fs->code_heap);
      bin = MemGetWritePtr(xbin);
      code_off = 0;
    } else if (run == 2) {
      // Final run,be sure to set SetKeepTmps(we see the changed reigsters in
      // first/second pass)
      for (r = cctrl->code_ctrl->ir_code->last; r != cctrl->code_ctrl->ir_code;
           r = r->base.last) {
        // Do first(REVERSE polish notation) things first
        SetKeepTmps(r);
      }
      A_FREE(bin);
      xbin = A_CALLOC(256 + code_off, heap ?: Fs->code_heap);
      bin = MemGetWritePtr(xbin);
      code_off = 0;
    }
    code_off = FuncProlog(cctrl, bin, code_off);
    for (cnt = 0; cnt < cnt2; cnt++) {
    enter:
      cctrl->backend_user_data1 = 0;
      cctrl->backend_user_data2 = 0;
      cctrl->backend_user_data3 = 0;
      r = forwards[cnt];
      {
        // Clear previous attributes
        CRPN *clear_to = ICFwd(r), *cur = r;
        while (cur != clear_to) {
          cur->flags &= ICF_DEAD_CODE | ICF_IS_BOOL | ICF_LOCK_EXPR |
                        ICF_NO_JUMP | ICF_DOESNT_SPILL_TMP_REGS |
                        ICF_SPILLS_TMP_REGS;
          cur->res.mode = 0;
          cur = cur->base.next;
        }
      }

      if (PushTmpDepthFirst(cctrl, r, 0))
        PopTmp(cctrl, r);

      // These modes expect you run ->__sib_base_rpn/__sib_index_rpn
      if (r->res.mode != __MD_X86_64_LEA_SIB &&
          r->res.mode != __MD_X86_64_SIB) {
        r->res.mode = MD_NULL;
      }
      code_off = __OptPassFinal(cctrl, r, bin, code_off);
      if (IsTerminalInst(r)) {
        cnt++;
        for (; cnt < cnt2; cnt++) {
          if (forwards[cnt]->type == IC_LABEL)
            goto enter;
        }
      }
    }
    cctrl->epilog_label->addr = bin + code_off;
    code_off = FuncEpilog(cctrl, bin, code_off);
    for (misc = cctrl->code_ctrl->code_misc->next;
         misc != cctrl->code_ctrl->code_misc; misc = misc->base.next) {
      int64_t old_code_off = code_off;
      switch (misc->type) {
        break;
      case CMT_INT_CONST:
        if (code_off % 8)
          code_off += 8 - code_off % 8;
        misc->addr = bin + code_off;
        if (bin)
          *(int64_t *)(bin + code_off) = misc->integer;
        code_off += 8;
      fill_in_refs:
        for (cm_ref = misc->refs; cm_ref; cm_ref = cm_ref_tmp) {
          cm_ref_tmp = cm_ref->next;
          if (run) {
            if (cm_ref->is_abs64) {
              // Filled in by HolyC side
              if (misc->patch_addr)
                *misc->patch_addr = MemGetExecPtr(cm_ref->add_to);
            } else
              *cm_ref->add_to = (char *)misc->addr - (char *)cm_ref->add_to -
                                4 + cm_ref->offset;
            // Set ICF_NO_JUMP if necessary
            if (cm_ref->from_ic) {
              if ((char *)misc->addr - (char *)cm_ref->add_to -
                      cm_ref->offset ==
                  4) {
                cm_ref->from_ic->flags |= ICF_NO_JUMP;
              }
            }
          }
          A_FREE(cm_ref);
        }
        misc->refs = NULL;
        break;
      case CMT_FLOAT_CONST:
        if (code_off % 8)
          code_off += 8 - code_off % 8;
        misc->addr = bin + code_off;
        if (bin)
          *(double *)(bin + code_off) = misc->flt;
        code_off += 8;
        goto fill_in_refs;
        break;
      case CMT_JMP_TAB:
        if (code_off % 8)
          code_off += 8 - code_off % 8;
        misc->addr = bin + code_off;
        if (misc->patch_addr)
          *misc->patch_addr = MemGetExecPtr(misc->addr);
        if (bin) {
          for (idx = 0; idx <= misc->hi - misc->lo; idx++)
            *(void **)(bin + code_off + idx * 8) =
                (char *)MemGetExecPtr(misc->jmp_tab[idx]->addr);
        }
        code_off += (misc->hi - misc->lo + 1) * 8;
        goto fill_in_refs;
        break;
      case CMT_LABEL:
        if (misc->patch_addr)
          *misc->patch_addr = MemGetExecPtr(misc->addr);
        // We assign the statics offset later
        if (misc == cctrl->statics_label)
          break;
        goto fill_in_refs;
      case CMT_SHORT_ADDR:
        if (run == 2 && misc->patch_addr) {
          *misc->patch_addr = MemGetExecPtr(misc->addr);
        } else if (run != 2) {
          for (cm_ref = misc->refs; cm_ref; cm_ref = cm_ref_tmp) {
            cm_ref_tmp = cm_ref->next;
            A_FREE(cm_ref);
          }
          misc->refs = NULL;
        }
        break;
      case CMT_RELOC_U64:
        if (code_off % 8)
          code_off += 8 - code_off % 8;
        misc->addr = bin + code_off;
        if (bin)
          *(void **)((char *)MemGetWritePtr(bin) + code_off) = &DoNothing;
        code_off += 8;
        if (run == 2) {
          *(import = A_CALLOC(sizeof(CHashImport), NULL)) = (CHashImport){
              .base =
                  {
                      .type = HTT_IMPORT_SYS_SYM,
                      .str = A_STRDUP(misc->str, NULL),
                  },
              .address = misc->addr,
          };
          HashAdd(import, Fs->hash_table);
        }
        if (run == 2 && misc->patch_addr) {
          *misc->patch_addr = MemGetExecPtr(misc->addr);
        }
        goto fill_in_refs;
        break;
      case CMT_STRING:
        if (!(cctrl->flags & CCF_STRINGS_ON_HEAP)) {
          misc->addr = bin + code_off;
          if (bin)
            memcpy(MemGetWritePtr(bin) + code_off, misc->str, misc->str_len);
          code_off += misc->str_len;
        } else {
          if (run == 2) {
            // See IC_STR: We "steal" the ->str because it's already on the
            // heap.
            //  IC_STR steals the ->str point so we dont want to Free it
            misc->str = NULL;
          }
        }
        goto fill_in_refs;
      }
    }
    if (code_off % 8) // Align to 8
      code_off += 8 - code_off % 8;
    cctrl->code_ctrl->statics_offset = code_off;
    cctrl->statics_label->addr = bin + cctrl->code_ctrl->statics_offset;
    // Fill in the static references
    if (bin)
      for (r = cctrl->code_ctrl->ir_code->next; r != cctrl->code_ctrl->ir_code;
           r = r->base.next) {
        if (r->type == __IC_SET_STATIC_DATA) {
          memcpy(MemGetWritePtr(bin) + cctrl->code_ctrl->statics_offset +
                     r->code_misc->integer,
                 r->code_misc->str, r->code_misc->str_len);
        }
      }
    if (bin) {
      for (cm_ref = cctrl->statics_label->refs; cm_ref; cm_ref = cm_ref_tmp) {
        cm_ref_tmp = cm_ref->next;
        *cm_ref->add_to = (char *)cctrl->statics_label->addr -
                          (char *)cm_ref->add_to - 4 + cm_ref->offset;
        A_FREE(cm_ref);
      }
    }
    if (statics_sz)
      code_off += statics_sz + 8;
  }
  final_size = code_off;
  if (dbg_info) {
    cnt = MSize(dbg_info) / 8;
    dbg_info[0] = xbin;
  }
  if (res_sz)
    *res_sz = final_size;
  return xbin;
}
