#include "aiwn_arm.h"
#include "aiwn_except.h"
#include <stdint.h>
#define MASKn(v, bits, off) (((v) & ((1ull << (bits)) - 1)) << (off))

// https://math.stackexchange.com/questions/490813/range-twos-complement
#define ARM_FORCE_ALIGN(v, bits, chop)                                         \
  {                                                                            \
    if ((v) & ((1 << (chop)) - 1))                                             \
      return ARM_ERR_INV_OFF;                                                  \
    if ((v) >= 0) {                                                            \
      if (((v) >> chop) > (1l << ((bits) - 1)) - 1)                            \
        return ARM_ERR_INV_OFF;                                                \
    } else if ((-(v) >> chop) > (1l << ((bits) - 1)))                          \
      return ARM_ERR_INV_OFF;                                                  \
  }
#define ARM_FORCE_REG(r)                                                       \
  {                                                                            \
    if ((r) < 0)                                                               \
      return ARM_ERR_INV_REG;                                                  \
    if ((r) > 32)                                                              \
      return ARM_ERR_INV_REG;                                                  \
  }
// https://github.com/fujitsu/xbyak_aarch64
typedef struct {
  int64_t n, m;
  int64_t type;
  int64_t shift;
  int64_t init_sh;
} CARMAdrRegReg;
typedef struct {
  int64_t n;
  int64_t off;
} CARMAdrRegImm;
typedef struct {
  int64_t n;
  int64_t off;
} CARMAdrPostImm;
typedef struct {
  int64_t n;
  int64_t off;
} CARMAdrPreImm;
typedef struct {
  int64_t n;
  int64_t off;
  int64_t ext;
} CARMAdrScImm;

static int64_t LdStRegExt(int64_t sz, int64_t opc, int64_t rt, int64_t n,
                          int64_t m, int64_t mod) {
  int64_t s = 1;
  int64_t v = 0;
  return (sz << 30) | (7 << 27) | (v << 26) | (opc << 22) | (1 << 21) |
         (m << 16) | (mod << 13) | (s << 12) | (2 << 10) | (n << 5) | rt;
}

static int64_t MvWideImmX(uint32_t opc, int64_t d, int64_t imm, int64_t shift) {
  if (imm & ~0xffff)
    return ARM_ERR_INV_OFF;
  ARM_FORCE_REG(d);
  uint32_t hw = (shift >> 4) & 0b11;
  uint32_t imm16 = imm & 0xffff;
  return (1 << 31) | (opc << 29) | (0x25 << 23) | (hw << 21) | (imm16 << 5) | d;
}
static int64_t DataProc2Src(uint32_t opcode, int64_t rd, int64_t rn,
                            int64_t rm) {
  ARM_FORCE_REG(rn);
  ARM_FORCE_REG(rm);
  ARM_FORCE_REG(rd);
  uint32_t sf = 1;
  uint32_t S = 0;
  return (sf << 31) | (S << 29) | (0xd6 << 21) | (rm << 16) | (opcode << 10) |
         (rn << 5) | rd;
}

static int64_t FpComp(uint32_t M, uint32_t S, uint32_t type, uint32_t op,
                      uint32_t opcode2, int64_t vn, int64_t vm) {
  ARM_FORCE_REG(vn);
  ARM_FORCE_REG(vm);
  return (M << 31) | (S << 29) | (0xf << 25) | (type << 22) | (1 << 21) |
         (vm << 16) | (op << 14) | (1 << 13) | (vn << 5) | opcode2;
}

static int64_t FpDataProc1Reg(uint32_t M, uint32_t S, uint32_t type,
                              uint32_t opcode, int64_t vd, int64_t vn) {
  ARM_FORCE_REG(vn);
  ARM_FORCE_REG(vd);
  return (M << 31) | (S << 29) | (0xf << 25) | (type << 22) | (1 << 21) |
         (opcode << 15) | (1 << 14) | (vn << 5) | vd;
}

static int64_t LdStRegPairPostImm(int64_t opc, int64_t l, int64_t r, int64_t r2,
                                  CARMAdrPostImm *pre) {
  int64_t imm = pre->off;
  int64_t times = (opc == 2) ? 2 : 1;
  ARM_FORCE_ALIGN(imm, 7, times + 1);
  int64_t imm7 = MASKn(imm >> (times + 1), 7, 15);
  return (opc << 30) | (0x5 << 27) | (1 << 23) | (l << 22) | imm7 | (r2 << 10) |
         (pre->n << 5) | r;
}
static int64_t LdStRegPairPre(int64_t opc, int64_t L, int64_t rt1, int64_t rt2,
                              CARMAdrPreImm *adr) {
  int64_t imm = adr->off;
  int64_t times = (opc == 2) ? 2 : 1;
  ARM_FORCE_ALIGN(imm, 7, times + 1);
  uint32_t imm7 = MASKn(imm >> (times + 1), 7, 15);
  return (opc << 30) | (0x5 << 27) | (3 << 23) | (L << 22) | imm7 |
         (rt2 << 10) | (adr->n << 5) | rt1;
}

// For 64bit
static int64_t LdStSimdFpUnImm(int32_t opc, int64_t vt, int64_t n,
                               int64_t imm) {
  uint32_t imm12 = MASKn(imm >> 3, 12, 10);
  ARM_FORCE_ALIGN(imm, 12, 3);
  if (imm < 0)
    return ARM_ERR_INV_OFF;
  return (3 << 30) | (0x7 << 27) | (1 << 26) | (1 << 24) | (opc << 22) | imm12 |
         (n << 5) | vt;
}

static int64_t LdRegSimdFpLiteralEnc(int64_t vt, int64_t imm) {
  uint32_t imm19 = MASKn(imm >> 2, 19, 5);
  ARM_FORCE_ALIGN(imm, 19, 2);
  return (1 << 30) | (0x3 << 27) | (1 << 26) | imm19 | vt;
}

static int64_t LdStRegPre(int64_t sz, int64_t opc, int64_t r,
                          CARMAdrPreImm *adr) {
  ARM_FORCE_ALIGN(adr->off, 9, 0);
  return (sz << 30) | (0x7 << 27) | (0xf << 27) | (opc << 22) |
         MASKn(adr->off, 9, 12) | (3 << 10) | (adr->n << 5) | r;
}

static int64_t LdStRegPostImm(int64_t sz, int64_t opc, int64_t r,
                              CARMAdrPostImm *adr) {
  ARM_FORCE_ALIGN(adr->off, 9, 0);
  return (sz << 30) | (0xf << 27) | (0xf << 27) | (opc << 22) |
         MASKn(adr->off, 9, 12) | (1 << 10) | (adr->n << 5) | r;
}

static int64_t FpDataProc2Reg(int64_t M, int64_t s, int64_t type,
                              int64_t opcode, int64_t d, int64_t n, int64_t m) {
  return (M << 31) | (s << 29) | (0xf << 25) | (type << 22) | (1 << 21) |
         (m << 16) | (opcode << 12) | (2 << 10) | (n << 5) | d;
}

static int64_t LdRegLiteralEnc(int64_t opc, int64_t v, int64_t r,
                               int64_t label) {
  ARM_FORCE_ALIGN(label, 19, 2);
  return (opc << 30) | (0x3 << 27) | (v << 26) | MASKn(label >> 2, 19, 5) | r;
}
static int64_t ConversionFpIntIF(int64_t sf, int64_t s, int64_t type,
                                 int64_t rmode, int64_t opcode, int64_t d,
                                 int64_t n) {
  return (sf << 31) | (s << 29) | (0xf << 25) | (type << 22) | (1 << 21) |
         (rmode << 19) | (opcode << 16) | (n << 5) | d;
}
static int64_t ConversionFpIntFI(uint32_t sf, uint32_t S, uint32_t type,
                                 uint32_t rmode, uint32_t opcode, int32_t vd,
                                 int32_t rn) {
  return (sf << 31) | (S << 29) | (0xf << 25) | (type << 22) | (1 << 21) |
         (rmode << 19) | (opcode << 16) | (rn << 5) | (vd);
}
static int64_t LdStRegUnImm(int64_t sz, int64_t opc, int64_t r,
                            CARMAdrRegImm *adr) {
  ARM_FORCE_ALIGN(adr->off, 12, sz);
  return MASKn(adr->off >> sz, 12, 10) | (sz << 30) | (0x7 << 27) | (1 << 24) |
         (opc << 22) | (adr->n << 5) | (r);
}

static int64_t LdStRegReg(int64_t size, int64_t opc, int64_t rt, int64_t m,
                          int64_t n, int64_t shift) {
  //(1<<12) triggers shift
  int64_t S = shift;
  return (size << 30) | (0x7 << 27) | (opc << 22) | (1 << 21) | (n << 16) |
         (3 << 13) | (S << 12) | (2 << 10) | (m << 5) | rt;
}

static int64_t CompareBr(int64_t op, int64_t r, int64_t imm) {
  ARM_FORCE_ALIGN(imm, 19, 2);
  return (0x1a << 25) | (op << 24) | MASKn(imm >> 2, 19, 5) | r;
}

static int64_t CompareBrX(int64_t op, int64_t r, int64_t imm) {
  ARM_FORCE_ALIGN(imm, 19, 2);
  return (1 << 31) | (0x1a << 25) | (op << 24) | MASKn(imm >> 2, 19, 5) | r;
}

static int64_t UncondBrNoReg(int64_t opc, int64_t op2, int64_t op3, int64_t rn,
                             int64_t op4) {
  return (0x6b << 25) | (opc << 21) | (op2 << 16) | (op3 << 10) | (rn << 5) |
         op4;
}

static int64_t UncondBrImm(int64_t op, int64_t label) {
  ARM_FORCE_ALIGN(label, 26, 2);
  return (op << 31) | (5 << 26) | MASKn(label >> 2, 26, 0);
}

static int64_t UncondBr1Reg(int64_t opc, int64_t op2, int64_t op3, int64_t reg,
                            int64_t op4) {
  return (0x6b << 25) | (opc << 21) | (op2 << 16) | (op3 << 10) | (reg << 5) |
         op4;
}

static int64_t CondBrImm(int64_t cond, int64_t imm) {
  ARM_FORCE_ALIGN(imm, 19, 2);
  return (0x2a << 25) | MASKn(imm >> 2, 19, 5) | cond;
}
static int64_t AddSubImm(int64_t op, int64_t s, int64_t d, int64_t n,
                         int64_t imm, int64_t sh) {
  ARM_FORCE_ALIGN(imm, 12, 0);
  int32_t imm12 = MASKn(imm, 12, 10);
  int32_t shift = MASKn(sh, 1, 22);
  return imm12 | shift | MASKn(d, 6, 0) | MASKn(n, 6, 5) | (0x11ll << 24) |
         (s << 29) | (op << 30);
}

static int64_t TestBr(int64_t op, int64_t rt, int64_t imm, int64_t off) {
  int64_t d5 = (imm >> 5) & 1;
  int64_t d40 = MASKn(imm, 5, 0);
  int64_t imm14 = MASKn(off >> 2, 14, 5);
  ARM_FORCE_ALIGN(off, 14, 2);
  return (d5 << 31) | (0x1b << 25) | (op << 24) | (d40 << 19) | imm14 | rt;
}

static int64_t Bitfield(int64_t op, int64_t d, int64_t n, uint64_t immr,
                        uint64_t imms) {
  return (op << 29) | (0x26 << 23) | (0 << 22) | MASKn(immr, 6, 16) |
         MASKn(imms, 6, 10) | (n << 5) | d;
}

static int64_t BitfieldX(int64_t op, int64_t d, int64_t n, uint64_t immr,
                         uint64_t imms) {
  return (1 << 31) | (op << 29) | (0x26 << 23) | (1 << 22) |
         MASKn(immr, 6, 16) | MASKn(imms, 6, 10) | (n << 5) | d;
}

static int64_t AddSubImmX(int64_t op, int64_t s, int64_t d, int64_t n,
                          int64_t imm, int64_t sh) {
  if (imm < 0)
    return ARM_ERR_INV_OFF;
  ARM_FORCE_ALIGN(imm, 12, 0);
  int32_t imm12 = MASKn(imm, 12, 10);
  int32_t shift = MASKn(sh, 1, 22);
  return (1 << 31) | imm12 | shift | MASKn(d, 6, 0) | MASKn(n, 6, 5) |
         (0x11ll << 24) | (s << 29) | (op << 30);
}

static int64_t AddSubExtReg(int64_t opc, int64_t s, int64_t d, int64_t n,
                            int64_t m, int64_t emod, int64_t sh) {
  return (opc << 30) | (s << 29) | (0xb << 24) | (1 << 21) | (m << 16) |
         (emod << 13) | MASKn(sh, 3, 10) | (n << 5) | d;
}

static int64_t AddSubExtRegX(int64_t opc, int64_t s, int64_t d, int64_t n,
                             int64_t m, int64_t emod, int64_t sh) {
  uint32_t option = (emod == 12) ? 3 : emod;
  return (1 << 31) | (opc << 30) | (s << 29) | (0xb << 24) | (1 << 21) |
         (m << 16) | (option << 13) | MASKn(sh, 3, 10) | (n << 5) | d;
}

static int64_t AddSubShiftReg(int64_t opc, int64_t s, int64_t d, int64_t n,
                              int64_t m, int64_t shmod, int64_t sh,
                              int64_t alias) {
  int64_t rd_sp = d == 31;
  int64_t rn_sp = d == 31;
  if (((rd_sp + rn_sp) >= 1 + alias) && shmod == LSL) {
    return AddSubExtReg(opc, s, d, n, m, 12, sh);
  }
  return (opc << 30) | (s << 29) | (0xb << 24) | (shmod << 22) | (m << 16) |
         MASKn(sh, 6, 10) | (n << 5) | d;
}

static int64_t LdStSimdFpRegPostImm(int64_t opc, int64_t d, int64_t p,
                                    int64_t off) {
  ARM_FORCE_ALIGN(off, 9, 0);
  return (3 << 30) | (0x7 << 27) | (1 << 26) | (opc << 22) | MASKn(off, 9, 12) |
         (1 << 10) | (p << 5) | d;
}

static int64_t LdStSimdFpRegPreImm(int64_t opc, int64_t d, int64_t p,
                                   int64_t off) {
  ARM_FORCE_ALIGN(off, 9, 0);
  return (3 << 30) | (0x7 << 27) | (1 << 26) | (opc << 22) | MASKn(off, 9, 12) |
         (3 << 10) | (p << 5) | d;
}

static int64_t AddSubShiftRegX(int64_t opc, int64_t s, int64_t d, int64_t n,
                               int64_t m, int64_t shmod, int64_t sh,
                               int64_t alias) {
  int64_t rd_sp = d == 31;
  int64_t rn_sp = n == 31;
  if (((rd_sp + rn_sp) >= 1 + alias) && shmod == LSL) {
    return AddSubExtRegX(opc, s, d, n, m, 12, sh);
  }
  return (1 << 31) | (opc << 30) | (s << 29) | (0xb << 24) | (shmod << 22) |
         (m << 16) | MASKn(sh, 6, 10) | (n << 5) | d;
}

static int64_t DataProc3RegX(int64_t op54, int64_t op31, int64_t o0, int64_t d,
                             int64_t n, int64_t m, int64_t ra) {
  return (1 << 31) | (op54 << 29) | (0x1b << 24) | (op31 << 21) | (m << 16) |
         (o0 << 15) | (ra << 10) | (n << 5) | (d);
}

static int64_t LogicalShiftReg(int64_t opc, int64_t N, int64_t rd, int64_t rn,
                               int64_t rm, int64_t shmod, uint32_t sh) {
  return (opc << 29) | (0xa << 24) | (shmod << 22) | (N << 21) | (rm << 16) |
         MASKn(sh, 6, 10) | MASKn(rn, 6, 5) | rd;
}

static int64_t LogicalShiftRegX(int64_t opc, int64_t N, int64_t rd, int64_t rn,
                                int64_t rm, int64_t shmod, uint32_t sh) {
  return (1ll << 31) | (opc << 29) | (0xa << 24) | (shmod << 22) | (N << 21) |
         (rm << 16) | MASKn(sh, 6, 10) | MASKn(rn, 6, 5) | rd;
}

static int64_t CondSel(int64_t op, int64_t s, int64_t op2, int64_t d, int64_t n,
                       int64_t m, int64_t cond) {
  return (op << 30) | (s << 29) | (m << 16) | (cond << 12) | (op2 << 10) |
         (n << 5) | (d);
}

static int64_t CondSelX(int64_t op, int64_t s, int64_t op2, int64_t d,
                        int64_t n, int64_t m, int64_t cond) {
  return (1 << 31) | (op << 30) | (s << 29) | (0xd4 << 21) | (m << 16) |
         (cond << 12) | (op2 << 10) | (n << 5) | (d);
}

static int64_t MvReg(int64_t d, int64_t n) {
  if (d == 31 || n == 31) {
    return AddSubImm(0, 0, d, n, 0, 0);
  } else
    return LogicalShiftReg(1, 0, d, 31, n, LSL, 0);
}

static int64_t MvRegX(int64_t d, int64_t n) {
  if (d == 31 || n == 31) {
    return AddSubImmX(0, 0, d, n, 0, 0);
  } else
    return LogicalShiftRegX(1, 0, d, 31, n, LSL, 0);
}

int64_t LdStSimdFpRegRegShift(uint32_t opc, int64_t vt, int64_t rn,
                              int64_t rm) {
  uint32_t size = 3;
  uint32_t option = 3;
  uint32_t S = 1;
  uint32_t V = 1;
  return (size << 30) | (0x7 << 27) | (V << 26) | (opc << 22) | (1 << 21) |
         (rm << 16) | (option << 13) | (S << 12) | (2 << 10) | (rn << 5) | vt;
}

// dst+=label
int64_t ARM_adrX(int64_t d, int64_t pc_off) {
  int32_t inst = 0x10 << 24;
  ARM_FORCE_ALIGN(pc_off, 21, 0);
  inst |= MASKn(d, 6, 0);
  inst |= MASKn(pc_off >> 2, 19, 5);
  inst |= MASKn(pc_off, 2, 29);
  return inst;
}

int64_t ARM_addImmX(int64_t d, int64_t n, int64_t imm) {
  return AddSubImmX(0, 0, d, n, imm, 0);
}

int64_t ARM_subImmX(int64_t d, int64_t n, int64_t imm) {
  return AddSubImmX(1, 0, d, n, imm, 0);
}

int64_t ARM_subsImmX(int64_t d, int64_t n, int64_t imm) {
  return AddSubImmX(1, 1, d, n, imm, 0);
}

int64_t ARM_cmpImmX(int64_t n, int64_t imm) {
  return AddSubImmX(1, 1, 31, n, imm, 0);
}

int64_t ARM_andRegX(int64_t rd, int64_t rn, int64_t rm) {
  return LogicalShiftRegX(0, 0, rd, rn, rm, LSL, 0);
}

int64_t ARM_orrRegX(int64_t rd, int64_t rn, int64_t rm) {
  return LogicalShiftRegX(1, 0, rd, rn, rm, LSL, 0);
}

// MVN move negative
int64_t ARM_mvnShift(int64_t rd, int64_t rm, int64_t shmod, int64_t sh) {
  return LogicalShiftReg(1, 1, rd, 31, rm, shmod, sh);
}

int64_t ARM_mvnShiftX(int64_t rd, int64_t rm, int64_t shmod, int64_t sh) {
  return LogicalShiftRegX(1, 1, rd, 31, rm, shmod, sh);
}

// Psuedo inst i invented
int64_t ARM_mvnReg(int64_t rd, int64_t rm) {
  return LogicalShiftReg(1, 1, rd, 31, rm, LSL, 0);
}

int64_t ARM_mvnRegX(int64_t rd, int64_t rm) {
  return LogicalShiftRegX(1, 1, rd, 31, rm, LSL, 0);
}

// exclusive xor
int64_t ARM_eorShiftX(int64_t rd, int64_t rn, int64_t rm, int64_t shmod,
                      int64_t sh) {
  return LogicalShiftRegX(2, 0, rd, rn, rm, shmod, sh);
}

int64_t ARM_eorRegX(int64_t rd, int64_t rn, int64_t rm) {
  return ARM_eorShiftX(rd, rn, rm, LSL, 0);
}

int64_t ARM_andsRegX(int64_t rd, int64_t rn, int64_t rm) {
  return LogicalShiftRegX(3, 0, rd, rn, rm, LSL, 0);
}

int64_t ARM_movReg(int64_t d, int64_t n) {
  return MvReg(d, n);
}

int64_t ARM_movRegX(int64_t d, int64_t n) {
  return MvRegX(d, n);
}

int64_t ARM_addShiftRegX(int64_t d, int64_t n, int64_t m, int64_t sh) {
  return AddSubShiftRegX(0, 0, d, n, m, LSL, sh, 0);
}

int64_t ARM_subShiftRegX(int64_t d, int64_t n, int64_t m, int64_t sh) {
  return AddSubShiftRegX(1, 0, d, n, m, LSL, sh, 0);
}

int64_t ARM_addShiftRegXs(int64_t d, int64_t n, int64_t m, int64_t sh) {
  return AddSubShiftRegX(0, 1, d, n, m, LSL, sh, 0);
}

int64_t ARM_subShiftRegXs(int64_t d, int64_t n, int64_t m, int64_t sh) {
  return AddSubShiftRegX(1, 1, d, n, m, LSL, sh, 0);
}

int64_t ARM_addRegX(int64_t d, int64_t n, int64_t m) {
  return AddSubShiftRegX(0, 0, d, n, m, LSL, 0, 0);
}

int64_t ARM_addsRegX(int64_t d, int64_t n, int64_t m) {
  return AddSubShiftRegX(0, 1, d, n, m, LSL, 0, 0);
}

int64_t ARM_addsImmX(int64_t rd, int64_t rn, int64_t imm) {
  return AddSubImmX(0, 1, rd, rn, imm, 0);
}

int64_t ARM_subRegX(int64_t d, int64_t n, int64_t m) {
  return AddSubShiftRegX(1, 0, d, n, m, LSL, 0, 0);
}

int64_t ARM_subsRegX(int64_t d, int64_t n, int64_t m) {
  return AddSubShiftRegX(1, 1, d, n, m, LSL, 0, 0);
}

int64_t ARM_negsRegX(int64_t d, int64_t m) {
  return AddSubShiftRegX(1, 1, d, 31, m, LSL, 0, 1);
}

int64_t ARM_cmpRegX(int64_t n, int64_t m) {
  return AddSubShiftRegX(1, 1, 31, n, m, LSL, 0, 1);
}

int64_t ARM_csel(int64_t d, int64_t n, int64_t m, int64_t cond) {
  return CondSel(0, 0, 0, d, n, m, cond);
}

int64_t ARM_cselX(int64_t d, int64_t n, int64_t m, int64_t cond) {
  return CondSelX(0, 0, 0, d, n, m, cond);
}

int64_t ARM_cset(int64_t d, int64_t cond) {
  return CondSel(0, 0, 1, d, 31, 31, cond ^ 1);
}

int64_t ARM_csetX(int64_t d, int64_t cond) {
  return CondSelX(0, 0, 1, d, 31, 31, cond ^ 1);
}

int64_t ARM_mulRegX(int64_t d, int64_t n, int64_t m) {
  return DataProc3RegX(0, 0, 0, d, n, m, 31);
}

int64_t ARM_uxtb(int64_t d, int64_t n) {
  return Bitfield(2, d, n, 0, 7);
}

int64_t ARM_uxtbX(int64_t d, int64_t n) {
  return BitfieldX(2, d, n, 0, 7);
}

int64_t ARM_uxth(int64_t d, int64_t n) {
  return Bitfield(2, d, n, 0, 15);
}

int64_t ARM_uxthX(int64_t d, int64_t n) {
  return BitfieldX(2, d, n, 0, 15);
}

int64_t ARM_uxtwX(int64_t d, int64_t n) {
  return BitfieldX(2, d, n, 0, 31);
}

int64_t ARM_sxtb(int64_t d, int64_t n) {
  return BitfieldX(0, d, n, 0, 7);
}

int64_t ARM_sxtbX(int64_t d, int64_t n) {
  return BitfieldX(0, d, n, 0, 7);
}

int64_t ARM_sxth(int64_t d, int64_t n) {
  return Bitfield(0, d, n, 0, 15);
}

int64_t ARM_sxthX(int64_t d, int64_t n) {
  return BitfieldX(0, d, n, 0, 15);
}

int64_t ARM_sxtw(int64_t d, int64_t n) {
  return Bitfield(0, d, n, 0, 31);
}

int64_t ARM_sxtwX(int64_t d, int64_t n) {
  return BitfieldX(0, d, n, 0, 31);
}

int64_t ARM_asrImm(int64_t d, int64_t n, int64_t imm) {
  return Bitfield(0, d, n, imm, 31);
}

int64_t ARM_asrImmX(int64_t d, int64_t n, int64_t imm) {
  return BitfieldX(0, d, n, imm, 63);
}

int64_t ARM_lslImm(int64_t d, int64_t n, int64_t imm) {
  return Bitfield(2, d, n, MASKn(-imm % 32, 6, 0), 31 - imm);
}

int64_t ARM_lslImmX(int64_t d, int64_t n, int64_t imm) {
  return BitfieldX(2, d, n, MASKn(-imm % 64, 6, 0), 63 - imm);
}

int64_t ARM_lsrImm(int64_t d, int64_t n, int64_t imm) {
  return Bitfield(2, d, n, imm, 31);
}

int64_t ARM_lsrImmX(int64_t d, int64_t n, int64_t imm) {
  return BitfieldX(2, d, n, imm, 63);
}
int64_t ARM_ret() {
  return UncondBrNoReg(2, 31, 0, 30, 0);
}

int64_t ARM_eret() {
  return UncondBrNoReg(4, 31, 0, 31, 0);
}

int64_t ARM_br(int64_t r) {
  return UncondBr1Reg(0, 31, 0, r, 0);
}

// Func call
int64_t ARM_blr(int64_t r) {
  return UncondBr1Reg(1, 31, 0, r, 0);
}

int64_t ARM_retReg(int64_t r) {
  return UncondBr1Reg(2, 31, 0, r, 0);
}

int64_t ARM_bcc(int64_t cond, int64_t lab) {
  return CondBrImm(cond, lab);
}

int64_t ARM_b(int64_t lab) {
  return UncondBrImm(0, lab);
}

int64_t ARM_bl(int64_t lab) {
  return UncondBrImm(1, lab);
}

// Condition branch if not zero
int64_t ARM_cbz(int64_t r, int64_t label) {
  return CompareBr(0, r, label);
}

int64_t ARM_cbzX(int64_t r, int64_t label) {
  return CompareBrX(0, r, label);
}

// Vice versa
int64_t ARM_cbnz(int64_t r, int64_t label) {
  return CompareBr(1, r, label);
}

int64_t ARM_cbnzX(int64_t r, int64_t label) {
  return CompareBrX(1, r, label);
}

// This is one of Bungis's favorites,it tests if a bit is set and jumps
int64_t ARM_tbz(int64_t rt, int64_t bit, int64_t label) {
  return TestBr(0, rt, bit, label);
}

int64_t ARM_tbnz(int64_t rt, int64_t bit, int64_t label) {
  return TestBr(1, rt, bit, label);
}

int64_t ARM_strbRegReg(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(0, 0, r, a, b, 0);
}

int64_t ARM_strbRegImm(int64_t r, int64_t n, int64_t off) {
  CARMAdrRegImm adr;
  adr.n = n;
  adr.off = off;
  return LdStRegUnImm(0, 0, r, &adr);
}

int64_t ARM_ldrbRegImm(int64_t r, int64_t n, int64_t off) {
  CARMAdrRegImm adr;
  adr.n = n;
  adr.off = off;
  return LdStRegUnImm(0, 1, r, &adr);
}

int64_t ARM_ldrbRegReg(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(0, 1, r, a, b, 0);
}

int64_t ARM_strhRegReg(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(1, 0, r, a, b, 0);
}

int64_t ARM_strhRegImm(int64_t r, int64_t n, int64_t off) {
  CARMAdrRegImm adr;
  adr.n = n;
  adr.off = off;
  return LdStRegUnImm(1, 0, r, &adr);
}

int64_t ARM_ldrhRegReg(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(1, 1, r, a, b, 0);
}

int64_t ARM_ldrhRegImm(int64_t r, int64_t n, int64_t off) {
  CARMAdrRegImm adr;
  adr.n = n;
  adr.off = off;
  return LdStRegUnImm(1, 1, r, &adr);
}

int64_t ARM_strRegImm(int64_t r, int64_t n, int64_t off) {
  CARMAdrRegImm adr;
  adr.n = n;
  adr.off = off;
  return LdStRegUnImm(2, 0, r, &adr);
}

int64_t ARM_strRegReg(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(2, 0, r, a, b, 0);
}

int64_t ARM_ldrRegReg(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(2, 1, r, a, b, 0);
}

int64_t ARM_ldrRegImm(int64_t r, int64_t n, int64_t off) {
  CARMAdrRegImm adr;
  adr.n = n;
  adr.off = off;
  return LdStRegUnImm(2, 1, r, &adr);
}

int64_t ARM_strRegImmX(int64_t r, int64_t n, int64_t off) {
  CARMAdrRegImm adr;
  adr.n = n;
  adr.off = off;
  return LdStRegUnImm(3, 0, r, &adr);
}

int64_t ARM_ldrRegImmX(int64_t r, int64_t n, int64_t off) {
  CARMAdrRegImm adr;
  adr.n = n;
  adr.off = off;
  return LdStRegUnImm(3, 1, r, &adr);
}

int64_t ARM_strRegRegX(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(3, 0, r, a, b, 0);
}
int64_t ARM_ldrRegRegX(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(3, 1, r, a, b, 0);
}

int64_t ARM_fmulReg(int64_t d, int64_t n, int64_t m) {
  return FpDataProc2Reg(0, 0, 1, 0, d, n, m);
}

int64_t ARM_fdivReg(int64_t d, int64_t n, int64_t m) {
  return FpDataProc2Reg(0, 0, 1, 1, d, n, m);
}

int64_t ARM_faddReg(int64_t d, int64_t n, int64_t m) {
  return FpDataProc2Reg(0, 0, 1, 2, d, n, m);
}

int64_t ARM_fsubReg(int64_t d, int64_t n, int64_t m) {
  return FpDataProc2Reg(0, 0, 1, 3, d, n, m);
}

int64_t ARM_scvtf(int64_t d, int64_t n) {
  return ConversionFpIntFI(1, 0, 1, 0, 2, d, n);
}

int64_t ARM_ucvtf(int64_t d, int64_t n) {
  return ConversionFpIntFI(1, 0, 1, 0, 3, d, n);
}

int64_t ARM_fcvtzs(int64_t d, int64_t n) {
  return ConversionFpIntIF(1, 0, 1, 3, 0, d, n);
}

int64_t ARM_ldrLabel(int64_t d, int64_t label) {
  return LdRegLiteralEnc(0, 0, d, label);
}

int64_t ARM_ldrLabelX(int64_t d, int64_t label) {
  return LdRegLiteralEnc(1, 0, d, label);
}

int64_t ARM_ldrLabelF64(int64_t d, int64_t label) {
  return LdRegSimdFpLiteralEnc(d, label);
}

int64_t ARM_strPostImm(int64_t r, int64_t b, int64_t off) {
  CARMAdrPostImm adr = {b, off};
  return LdStRegPostImm(2, 0, r, &adr);
}

int64_t ARM_ldrPostImm(int64_t r, int64_t b, int64_t off) {
  CARMAdrPostImm adr = {b, off};
  return LdStRegPostImm(2, 1, r, &adr);
}

int64_t ARM_strPreImm(int64_t r, int64_t b, int64_t off) {
  CARMAdrPreImm adr = {b, off};
  return LdStRegPre(2, 0, r, &adr);
}

int64_t ARM_ldrPreImm(int64_t r, int64_t b, int64_t off) {
  CARMAdrPreImm adr = {b, off};
  return LdStRegPre(2, 1, r, &adr);
}

int64_t ARM_strPostImmX(int64_t r, int64_t b, int64_t off) {
  CARMAdrPostImm adr = {b, off};
  return LdStRegPostImm(3, 0, r, &adr);
}

int64_t ARM_ldrPostImmX(int64_t r, int64_t b, int64_t off) {
  CARMAdrPostImm adr = {b, off};
  return LdStRegPostImm(3, 1, r, &adr);
}

int64_t ARM_strPreImmX(int64_t r, int64_t b, int64_t off) {
  CARMAdrPreImm adr = {b, off};
  return LdStRegPre(3, 0, r, &adr);
}

int64_t ARM_ldrPreImmX(int64_t r, int64_t b, int64_t off) {
  CARMAdrPreImm adr = {b, off};
  return LdStRegPre(3, 1, r, &adr);
}

//
// STP store pair of registers
// LDP load a pair of registers
//

int64_t ARM_stpPreImm(int64_t r, int64_t r2, int64_t b, int64_t off) {
  CARMAdrPreImm adr = {b, off};
  return LdStRegPairPre(0, 0, r, r2, &adr);
}

int64_t ARM_stpPreImmX(int64_t r, int64_t r2, int64_t b, int64_t off) {
  CARMAdrPreImm adr = {b, off};
  return LdStRegPairPre(2, 0, r, r2, &adr);
}

int64_t ARM_ldpPreImm(int64_t r, int64_t r2, int64_t b, int64_t off) {
  CARMAdrPreImm adr = {b, off};
  return LdStRegPairPre(0, 1, r, r2, &adr);
}

int64_t ARM_ldpPreImmX(int64_t r, int64_t r2, int64_t b, int64_t off) {
  CARMAdrPreImm adr = {b, off};
  return LdStRegPairPre(2, 1, r, r2, &adr);
}

int64_t ARM_stpPostImm(int64_t r, int64_t r2, int64_t b, int64_t off) {
  CARMAdrPostImm adr = {b, off};
  return LdStRegPairPostImm(0, 0, r, r2, &adr);
}

int64_t ARM_stpPostImmX(int64_t r, int64_t r2, int64_t b, int64_t off) {
  CARMAdrPostImm adr = {b, off};
  return LdStRegPairPostImm(2, 0, r, r2, &adr);
}

int64_t ARM_ldpPostImm(int64_t r, int64_t r2, int64_t b, int64_t off) {
  CARMAdrPostImm adr = {b, off};
  return LdStRegPairPostImm(0, 1, r, r2, &adr);
}

int64_t ARM_ldpPostImmX(int64_t r, int64_t r2, int64_t b, int64_t off) {
  CARMAdrPostImm adr = {b, off};
  return LdStRegPairPostImm(2, 1, r, r2, &adr);
}

int64_t ARM_ldrRegImmF64(int64_t r, int64_t n, uint64_t off) {
  return LdStSimdFpUnImm(1, r, n, off);
}

int64_t ARM_strRegImmF64(int64_t r, int64_t n, uint64_t off) {
  return LdStSimdFpUnImm(0, r, n, off);
}

int64_t ARM_fnegReg(int64_t d, int64_t s) {
  return FpDataProc1Reg(0, 0, 1, 2, d, s);
}

int64_t ARM_fmovReg(int64_t d, int64_t s) {
  return FpDataProc1Reg(0, 0, 1, 0, d, s);
}

int64_t ARM_fcmp(int64_t a, int64_t b) {
  return FpComp(0, 0, 1, 0, 0, a, b);
}

int64_t ARM_sdivRegX(int64_t d, int64_t n, int64_t m) {
  return DataProc2Src(3, d, n, m);
}

int64_t ARM_lslvRegX(int64_t d, int64_t n, int64_t m) {
  return DataProc2Src(8, d, n, m);
}

int64_t ARM_lsrvRegX(int64_t d, int64_t n, int64_t m) {
  return DataProc2Src(9, d, n, m);
}

int64_t ARM_asrvRegX(int64_t d, int64_t n, int64_t m) {
  return DataProc2Src(10, d, n, m);
}

int64_t ARM_movzImmX(int64_t d, int64_t i16, int64_t sh) {
  switch (sh) {
  case 0:
  case 16:
  case 32:
  case 48:
    break;
  default:
    throw(*(uint32_t *)"ASM");
  }
  return MvWideImmX(2, d, i16, sh);
}
int64_t ARM_movkImmX(int64_t d, int64_t i16, int64_t sh) {
  switch (sh) {
  case 0:
  case 16:
  case 32:
  case 48:
    break;
  default:
    throw(*(uint32_t *)"ASM");
  }
  return MvWideImmX(3, d, i16, sh);
}

int64_t ARM_movnImmX(int64_t d, int64_t i16, int64_t sh) {
  switch (sh) {
  case 0:
  case 16:
  case 32:
  case 48:
    break;
  default:
    throw(*(uint32_t *)"ASM");
  }
  return MvWideImmX(0, d, i16, sh);
}

int64_t ARM_strPostImmF64(int64_t d, int64_t p, int64_t off) {
  return LdStSimdFpRegPostImm(0, d, p, off);
}
int64_t ARM_ldrPostImmF64(int64_t d, int64_t p, int64_t off) {
  return LdStSimdFpRegPostImm(1, d, p, off);
}

int64_t ARM_strPreImmF64(int64_t d, int64_t p, int64_t off) {
  return LdStSimdFpRegPreImm(0, d, p, off);
}
int64_t ARM_ldrPreImmF64(int64_t d, int64_t p, int64_t off) {
  return LdStSimdFpRegPreImm(1, d, p, off);
}
static int64_t LdStSimdFpReg(int64_t opc, int64_t d, int64_t n, int64_t m) {
  uint32_t size = 3;
  uint32_t option = 3;
  uint32_t S = 0;
  uint32_t V = 1;
  return (size << 30) | (0x7 << 27) | (V << 26) | (opc << 22) | (1 << 21) |
         (m << 16) | (option << 13) | (S << 12) | (2 << 10) | (n << 5) | d;
}

int64_t ARM_strRegRegF64(int64_t d, int64_t n, int64_t m) {
  return LdStSimdFpReg(0, d, n, m);
}

int64_t ARM_ldrRegRegF64(int64_t d, int64_t n, int64_t m) {
  return LdStSimdFpReg(1, d, n, m);
}

//
// The sexy section. You have been blessed by the divine energy,so send
// a prayer for the ARM developers. In HolyC,typecasts are "bit-for-bit"
// and ARM allows us to do "bit-for-bit" transfers easily with fmov and freinds
//
int64_t ARM_fmovI64F64(int64_t d, int64_t s) {
  return ConversionFpIntIF(1, 0, 1, 0, 6, d, s);
}
int64_t ARM_fmovF64I64(int64_t d, int64_t s) {
  return ConversionFpIntFI(1, 0, 1, 0, 7, d, s);
}

int64_t ARM_strhRegRegShift(int64_t a, int64_t n, int64_t m) {
  return LdStRegReg(1, 0, a, n, m, 1);
}

int64_t ARM_ldrhRegRegShift(int64_t a, int64_t n, int64_t m) {
  return LdStRegReg(1, 1, a, n, m, 1);
}

int64_t ARM_strRegRegShift(int64_t a, int64_t n, int64_t m) {
  return LdStRegReg(2, 0, a, n, m, 1);
}

int64_t ARM_ldrRegRegShift(int64_t a, int64_t n, int64_t m) {
  return LdStRegReg(2, 1, a, n, m, 1);
}

int64_t ARM_strRegRegShiftX(int64_t a, int64_t n, int64_t m) {
  return LdStRegReg(3, 0, a, n, m, 1);
}

int64_t ARM_ldrRegRegShiftX(int64_t a, int64_t n, int64_t m) {
  return LdStRegReg(3, 1, a, n, m, 1);
}

int64_t ARM_ldrRegRegShiftF64(int64_t a, int64_t n, int64_t m) {
  return LdStSimdFpRegRegShift(1, a, n, m);
}

int64_t ARM_strRegRegShiftF64(int64_t a, int64_t n, int64_t m) {
  return LdStSimdFpRegRegShift(0, a, n, m);
}

//
// https://dinfuehr.github.io/blog/encoding-of-immediate-values-on-aarch64/
//
// I would rather eat a pineapple than explain this code
//

// Return's -1 on poo poo unencodable
static int64_t GenNImmSImmR(int64_t imm) {
  // TODO write this stuff in assemblt
  if (imm == 0 || ~imm == 0)
    return ARM_ERR_INV_OFF;
  int64_t consec = 0, idx, zero_gap = 0, first_zeros, width, x;
  uint64_t pattern;
#define ROT(x, n) ((x) >> n) | ((x) << (64 - n))
  // Find first 1
  zero_gap = __builtin_ctzl(imm);
  // Count consecitive bits for the pattern
  consec = __builtin_ctzl((~imm) >> zero_gap);
  // If leading is zero,check the number of ones at the end
  if (zero_gap == 0) {
    // Zeros may be present at the other side of  the ones
    //"[10000111]"
    //    ^^^^
    if ((imm >> consec) == 0)
      zero_gap = 64 - consec;
    else
      zero_gap = __builtin_ctzl(imm >> consec) - consec;
    consec += __builtin_clzl(~imm);
  } else if (zero_gap != 0) {
    // add leading 0s to the gap amount
    zero_gap += __builtin_clzl(imm);
  }
  // Our pattern size is zero_gap+consec
  width = zero_gap + consec;
  switch (width) {
#define MAKE_PATT                                                              \
  x = consec - 1;                                                              \
  pattern = 0;                                                                 \
  for (idx = 0; idx != 64 / width; idx++)                                      \
    if (!idx)                                                                  \
      pattern |= (1l << consec) - 1l;                                          \
    else                                                                       \
      pattern |= ((1l << consec) - 1l) << (width * idx);

#define CHECK_PATT(n)                                                          \
  for (idx = 0; idx != width; idx++)                                           \
    if (pattern == imm)                                                        \
      return (n << 12) | (idx << 6) | x;                                       \
    else                                                                       \
      pattern = ROT(pattern, 1);
    break;
  case 2:
    MAKE_PATT;
    CHECK_PATT(0);
    break;
  case 4:
    MAKE_PATT;
    CHECK_PATT(0);
    break;
  case 8:
    MAKE_PATT;
    CHECK_PATT(0);
    break;
  case 16:
    MAKE_PATT;
    CHECK_PATT(0);
    break;
  case 32:
    MAKE_PATT;
    CHECK_PATT(0);
    break;
  case 64:
    MAKE_PATT;
    CHECK_PATT(1);
    break;
  default:;
  }
  return ARM_ERR_INV_OFF;
}

static int64_t LogicalImm(int64_t opc, int64_t d, int64_t s, int64_t i) {
  int64_t poodle = GenNImmSImmR(i);
  if (i == ARM_ERR_INV_OFF)
    return i;
  return MASKn(opc, 8, 29) | MASKn(0x24, 8, 23) | MASKn(s, 14, 10) |
         MASKn(s, 6, 5) | MASKn(d, 6, 0);
}

static int64_t LogicalImmX(int64_t opc, int64_t d, int64_t s, int64_t i) {
  int64_t poodle = GenNImmSImmR(i);
  if (poodle == ARM_ERR_INV_OFF)
    return poodle;
  return (1 << 31) | MASKn(opc, 8, 29) | MASKn(0x24, 8, 23) |
         MASKn(poodle, 14, 10) | MASKn(s, 6, 5) | MASKn(d, 6, 0);
}

int64_t ARM_andImmX(int64_t d, int64_t s, int64_t i) {
  return LogicalImmX(0, d, s, i);
}

int64_t ARM_orrImmX(int64_t d, int64_t s, int64_t i) {
  return LogicalImmX(1, d, s, i);
}

int64_t ARM_eorImmX(int64_t d, int64_t s, int64_t i) {
  return LogicalImmX(2, d, s, i);
}

static int64_t LdStRegPairImm(int64_t opc, int64_t L, int64_t r1, int64_t r2,
                              int64_t ra, int64_t off) {
  int64_t times = 2;
  if (off % times)
    return ARM_ERR_INV_OFF;
  if (off > times * 256)
    return ARM_ERR_INV_OFF;
  if (off < times * -256)
    return ARM_ERR_INV_OFF;
  return MASKn(opc, 32, 30) | MASKn(0x5, 32, 27) | MASKn(2, 32, 23) |
         MASKn(L, 32, 22) | MASKn(off / 8, 7, 15) | MASKn(r2, 5, 10) |
         MASKn(ra, 5, 5) | MASKn(r1, 5, 0);
}

int64_t ARM_stpImmX(int64_t r1, int64_t r2, int64_t ra, int64_t off) {
  return LdStRegPairImm(2, 0, r1, r2, ra, off);
}
int64_t ARM_ldpImmX(int64_t r1, int64_t r2, int64_t ra, int64_t off) {
  return LdStRegPairImm(2, 1, r1, r2, ra, off);
}

static int64_t LdStSimdFpPairImm(int64_t opc, int64_t L, int64_t r1, int64_t r2,
                                 int64_t ra, int64_t off) {
  int64_t times = 2;
  if (off % times)
    return ARM_ERR_INV_OFF;
  if (off > times * 256)
    return ARM_ERR_INV_OFF;
  if (off < times * -256)
    return ARM_ERR_INV_OFF;
  return MASKn(opc, 32, 30) | MASKn(0x5, 32, 27) | (1 << 26) |
         MASKn(2, 32, 23) | MASKn(L, 32, 22) | MASKn(off / 8, 7, 15) |
         MASKn(r2, 5, 10) | MASKn(ra, 5, 5) | MASKn(r1, 5, 0);
}

int64_t ARM_stpImmF64(int64_t r1, int64_t r2, int64_t ra, int64_t off) {
  return LdStSimdFpPairImm(1, 0, r1, r2, ra, off);
}
int64_t ARM_ldpImmF64(int64_t r1, int64_t r2, int64_t ra, int64_t off) {
  return LdStSimdFpPairImm(1, 1, r1, r2, ra, off);
}
int64_t ARM_andsImmX(int64_t d, int64_t n, int64_t imm) {
  return LogicalImmX(3, d, n, imm);
}

int64_t ARM_udivX(int64_t d, int64_t n, int64_t m) {
  return DataProc2Src(2, d, n, m);
}
int64_t ARM_umullX(int64_t d, int64_t n, int64_t m) {
  return DataProc3RegX(0, 5, 0, d, n, m, 31);
}

static int64_t LdStRegUnsImm(int64_t size, int64_t opc, int64_t rt,
                             int64_t adr_reg, int64_t off) {
  if (-256 > off)
    return ARM_ERR_INV_OFF;
  if (255 < off)
    return ARM_ERR_INV_OFF;
  return (size << 30) | (7 << 27) | (opc << 22) | MASKn(off, 9, 12) |
         (adr_reg << 5) | rt;
}

int64_t ARM_sturb(int64_t r, int64_t a, int64_t off) {
  return LdStRegUnsImm(0, 0, r, a, off);
}

int64_t ARM_sturh(int64_t r, int64_t a, int64_t off) {
  return LdStRegUnsImm(1, 0, r, a, off);
}

int64_t ARM_sturw(int64_t r, int64_t a, int64_t off) {
  return LdStRegUnsImm(2, 0, r, a, off);
}

int64_t ARM_ldursbX(int64_t r, int64_t a, int64_t off) {
  return LdStRegUnsImm(0, 2, r, a, off);
}
int64_t ARM_ldurshX(int64_t r, int64_t a, int64_t off) {
  return LdStRegUnsImm(1, 2, r, a, off);
}
int64_t ARM_ldurswX(int64_t r, int64_t a, int64_t off) {
  return LdStRegUnsImm(2, 2, r, a, off);
}

int64_t ARM_stur(int64_t r, int64_t a, int64_t off) {
  return LdStRegUnsImm(3, 0, r, a, off);
}
int64_t ARM_ldur(int64_t r, int64_t a, int64_t off) {
  return LdStRegUnsImm(3, 1, r, a, off);
}

int64_t ARM_ldrsbX(int64_t r, int64_t a, int64_t off) {
  CARMAdrRegImm addr = {a, off};
  return LdStRegUnImm(0, 2, r, &addr);
}
int64_t ARM_ldrshX(int64_t r, int64_t a, int64_t off) {
  CARMAdrRegImm addr = {a, off};
  return LdStRegUnImm(1, 2, r, &addr);
}
int64_t ARM_ldrswX(int64_t r, int64_t a, int64_t off) {
  CARMAdrRegImm addr = {a, off};
  return LdStRegUnImm(2, 2, r, &addr);
}

int64_t ARM_ldrsbRegRegX(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(0, 2, r, a, b, 0);
}
int64_t ARM_ldrshRegRegX(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(1, 2, r, a, b, 0);
}
int64_t ARM_ldrswRegRegX(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(2, 2, r, a, b, 0);
}

int64_t ARM_ldrsbRegRegShiftX(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(0, 2, r, a, b, 1);
}
int64_t ARM_ldrshRegRegShiftX(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(1, 2, r, a, b, 1);
}
int64_t ARM_ldrswRegRegShiftX(int64_t r, int64_t a, int64_t b) {
  return LdStRegReg(2, 2, r, a, b, 1);
}

static int64_t LdExclusive(int64_t sz, int64_t o0, int64_t d, int64_t s) {
  return (sz << 30) | (0x8 << 24) | (1 << 22) | (0x1f << 16) | (0x1f << 10) |
         (s << 5) | d;
}

int64_t ARM_ldaxrb(int64_t d, int64_t s) {
  return LdExclusive(0, 1, d, s);
}

int64_t ARM_ldaxrh(int64_t d, int64_t s) {
  return LdExclusive(1, 1, d, s);
}
int64_t ARM_ldaxr(int64_t d, int64_t s) {
  return LdExclusive(2, 1, d, s);
}
int64_t ARM_ldaxrX(int64_t d, int64_t s) {
  return LdExclusive(3, 1, d, s);
}

static int64_t StExclusive(int64_t sz, int64_t o0, int64_t pass, int64_t d,
                           int64_t s) {
  return (sz << 30) | (0x8 << 24) | (pass << 16) | (0x1f << 10) | (s << 5) | d |
         (o0 << 15);
}

int64_t ARM_stlxrb(int64_t p, int64_t d, int64_t s) {
  return StExclusive(0, 1, p, d, s);
}

int64_t ARM_stlxrh(int64_t p, int64_t d, int64_t s) {
  return StExclusive(1, 1, p, d, s);
}
int64_t ARM_stlxr(int64_t p, int64_t d, int64_t s) {
  return StExclusive(2, 1, p, d, s);
}
int64_t ARM_stlxrX(int64_t p, int64_t d, int64_t s) {
  return StExclusive(3, 1, p, d, s);
}

static int64_t FpCondSel(int64_t m, int64_t s, int64_t t, int64_t d, int64_t n,
                         int64_t M, int64_t cond) {
  return (m << 31) | (s << 29) | (0xf << 25) | (t << 22) | (1 << 21) |
         (M << 16) | (cond << 12) | (3 << 10) | (n << 5) | (d);
}

int64_t ARM_fcsel(int64_t d, int64_t a, int64_t b, int64_t c) {
  return FpCondSel(0, 0, 1, d, a, b, c);
}
