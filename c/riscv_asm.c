#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#define RISCV_INV_OFF    -1
#define MASKn(v, off, w) (((v) & ((1 << (w)) - 1)) << (off))
int64_t RISCV_R(int64_t f7, int64_t s2, int64_t s1, int64_t f3, int64_t d,
                int64_t opc) {
  return MASKn(f7, 25, 7) | MASKn(s2, 20, 5) | MASKn(s1, 15, 5) |
         MASKn(f3, 12, 3) | MASKn(d, 7, 5) | MASKn(opc, 0, 7);
}
int64_t RISCV_I(int64_t imm, int64_t s1, int64_t f3, int64_t d, int64_t opc) {
  return MASKn(imm, 20, 12) | MASKn(s1, 15, 5) | MASKn(f3, 12, 3) |
         MASKn(d, 7, 5) | MASKn(opc, 0, 7);
}
int64_t RISCV_S(int64_t imm115, int64_t s1, int64_t d, int64_t f3,
                int64_t opc) {
  return MASKn(imm115 >> 5, 25, 12) | MASKn(s1, 20, 5) | MASKn(d, 15, 5) |
         MASKn(f3, 12, 3) | MASKn(imm115, 7, 5) | MASKn(opc, 0, 7);
}
int64_t RISCV_B(int64_t imm, int64_t s1, int64_t s2, int64_t f3, int64_t opc) {
  return MASKn(imm >> 12, 31, 1) | MASKn(imm >> 5, 25, 6) | MASKn(s2, 20, 5) |
         MASKn(s1, 15, 5) | MASKn(f3, 12, 3) | MASKn(imm >> 1, 8, 4) |
         MASKn(imm >> 11, 7, 1) | MASKn(opc, 0, 7);
}
int64_t RISCV_U(int64_t imm3112, int64_t d, int64_t opc) {
  int64_t r31_12 = imm3112 >> 12;
  return MASKn(imm3112, 12, 20) | MASKn(d, 7, 5) | MASKn(opc, 0, 7);
}
int64_t RISCV_J(int64_t imm_wtf, int64_t d, int64_t opc) {
  int64_t b20 = (imm_wtf >> 20) & 1;
  int64_t b10_1 = MASKn(imm_wtf >> 1, 0, 10);
  int64_t b11 = MASKn(imm_wtf >> 11, 0, 1);
  int64_t b19_12 = MASKn(imm_wtf >> 12, 0, 19 - 12 + 1);
  imm_wtf = (b20 << 30) | (b10_1 << 21) | (b11 << (20)) | (b19_12 << 12);
  return imm_wtf | MASKn(d, 7, 5) | MASKn(opc, 0, 7);
}

// RV32I
int64_t RISCV_LUI(int64_t d, int64_t off) {
  return RISCV_U(off, d, 0b0110111);
}
int64_t RISCV_AUIPC(int64_t d, int64_t off) {
  return RISCV_U(off, d, 0b0010111);
}

int64_t RISCV_JAL(int64_t d, int64_t off) {
  return RISCV_J(off, d, 0b1101111);
}
int64_t RISCV_JALR(int64_t d, int64_t s, int64_t off) {
  return RISCV_I(off, s, 0b000, d, 0b1100111);
}
int64_t RISCV_BEQ(int64_t s2, int64_t s, int64_t off) {
  return RISCV_B(off, s2, s, 0b000, 0b1100011);
}

int64_t RISCV_BNE(int64_t s2, int64_t s, int64_t off) {
  return RISCV_B(off, s2, s, 0b001, 0b1100011);
}

int64_t RISCV_BLT(int64_t s2, int64_t s, int64_t off) {
  return RISCV_B(off, s2, s, 0b100, 0b1100011);
}

int64_t RISCV_BGE(int64_t s2, int64_t s, int64_t off) {
  return RISCV_B(off, s2, s, 0b101, 0b1100011);
}
int64_t RISCV_BLTU(int64_t s2, int64_t s, int64_t off) {
  return RISCV_B(off, s2, s, 0b110, 0b1100011);
}
int64_t RISCV_BGEU(int64_t s2, int64_t s, int64_t off) {
  return RISCV_B(off, s2, s, 0b111, 0b1100011);
}
int64_t RISCV_LB(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b000, d, 0b11);
}
int64_t RISCV_LH(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b001, d, 0b11);
}
int64_t RISCV_LW(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b010, d, 0b11);
}
int64_t RISCV_LBU(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b100, d, 0b11);
}
int64_t RISCV_LHU(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b101, d, 0b11);
}
int64_t RISCV_SB(int64_t s1, int64_t s2, int64_t off) {
  return RISCV_S(off, s1, s2, 0b000, 0b0100011);
}
int64_t RISCV_SH(int64_t s1, int64_t s2, int64_t off) {
  return RISCV_S(off, s1, s2, 0b001, 0b0100011);
}
int64_t RISCV_SW(int64_t s1, int64_t s2, int64_t off) {
  return RISCV_S(off, s1, s2, 0b010, 0b0100011);
}
int64_t RISCV_ADDI(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b000, d, 0b0010011);
}
int64_t RISCV_SLTI(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b010, d, 0b0010011);
}
int64_t RISCV_SLTIU(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b011, d, 0b0010011);
}
int64_t RISCV_XORI(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b100, d, 0b0010011);
}
int64_t RISCV_ORI(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b110, d, 0b0010011);
}
int64_t RISCV_ANDI(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b111, d, 0b0010011);
}
int64_t RISCV_ADD(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000000, b, a, 0b000, d, 0b0110011);
}
int64_t RISCV_SUB(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0100000, b, a, 0b000, d, 0b0110011);
}
int64_t RISCV_SLL(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000000, b, a, 0b001, d, 0b0110011);
}
int64_t RISCV_SLT(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000000, b, a, 0b010, d, 0b010011);
}
int64_t RISCV_SLTU(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000000, b, a, 0b011, d, 0b0110011);
}
int64_t RISCV_XOR(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000000, b, a, 0b100, d, 0b0110011);
}
int64_t RISCV_SRL(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000000, b, a, 0b101, d, 0b0110011);
}
int64_t RISCV_SRA(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0100000, b, a, 0b101, d, 0b0110011);
}

int64_t RISCV_OR(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000000, b, a, 0b110, d, 0b0110011);
}
int64_t RISCV_AND(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000000, b, a, 0b111, d, 0b0110011);
}
int64_t RISCV_ECALL() {
  return 0b000000000000000000000001110011;
}
// RV64I
int64_t RISCV_LWU(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b110, d, 0b11);
}
int64_t RISCV_LD(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b011, d, 0b11);
}
int64_t RISCV_SD(int64_t s1, int64_t s2, int64_t imm) {
  return RISCV_S(imm, s1, s2, 0b11, 0b0100011);
}
int64_t RISCV_SLLI(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm & ((1 << 6) - 1), s, 0b01, d, 0b0010011);
}
int64_t RISCV_SRLI(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm & ((1 << 6) - 1), s, 0b101, d, 0b0010011);
}
int64_t RISCV_SRAI(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm & ((1 << 6) - 1), s, 0b101, d, 0b0010011) | (1 << 30);
}
int64_t RISCV_ADDIW(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b000, d, 0b011011);
}
int64_t RISCV_SLLIW(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm & ((1 << 6) - 1), s, 0b01, d, 0b011011);
}
int64_t RISCV_SRLIW(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm & ((1 << 6) - 1), s, 0b101, d, 0b011011);
}
int64_t RISCV_SRAIW(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm & ((1 << 6) - 1), s, 0b101, d, 0b011011) | (1 << 30);
}
int64_t RISCV_ADDW(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0, b, a, 0b000, d, 0b111011);
}
int64_t RISCV_SUBW(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0, b, a, 0b000, d, 0b111011) | (1 << 30);
}
int64_t RISCV_SLLW(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0, b, a, 0b000, d, 0b111011);
}
int64_t RISCV_SRLW(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0, b, a, 0b101, d, 0b111011);
}
int64_t RISCV_SRAW(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0, b, a, 0b101, d, 0b111011) | (1 << 30);
}
// RV32M
int64_t RISCV_MUL(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1, b, a, 0b000, d, 0b110011);
}
int64_t RISCV_MULH(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1, b, a, 0b001, d, 0b110011);
}
int64_t RISCV_MULHSU(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1, b, a, 0b010, d, 0b110011);
}
int64_t RISCV_MULHU(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1, b, a, 0b011, d, 0b110011);
}
int64_t RISCV_DIV(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1, b, a, 0b100, d, 0b110011);
}
int64_t RISCV_DIVU(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1, b, a, 0b101, d, 0b110011);
}
int64_t RISCV_REM(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1, b, a, 0b110, d, 0b110011);
}
int64_t RISCV_REMU(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1, b, a, 0b111, d, 0b110011);
}
// RV64M No way
// RV32A No way
// RV64A will do
// RV32F No way
// RV64F No way
// RV32D Yeah
int64_t RISCV_FLD(int64_t d, int64_t s, int64_t imm) {
  return RISCV_I(imm, s, 0b011, d, 0b0000111);
}
int64_t RISCV_FSD(int64_t a, int64_t b, int64_t imm) {
  return RISCV_S(imm, a, b, 0b11, 0b0100111);
}
#define ROUND_MODE 0b001 // Towards 0
int64_t RISCV_FADD_D(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000001, b, a, ROUND_MODE, d, 0b1010011);
}
int64_t RISCV_FSUB_D(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0000101, b, a, ROUND_MODE, d, 0b1010011);
}
int64_t RISCV_FMUL_D(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0001001, b, a, ROUND_MODE, d, 0b1010011);
}
int64_t RISCV_FDIV_D(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0001101, b, a, ROUND_MODE, d, 0b1010011);
}
int64_t RISCV_FSQRT_D(int64_t d, int64_t a) {
  return RISCV_R(0b0101101, 0, a, ROUND_MODE, d, 0b1010011);
}
int64_t RISCV_FMIN_D(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0010101, b, a, 0, d, 0b1010011);
}
int64_t RISCV_FMAX_D(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0010101, b, a, 1, d, 0b1010011);
}
int64_t RISCV_FEQ_D(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1010001, b, a, 0b10, d, 0b1010011);
}
int64_t RISCV_FLT_D(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1010001, b, a, 0b01, d, 0b1010011);
}
int64_t RISCV_FLE_D(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b1010001, b, a, 0b00, d, 0b1010011);
}
int64_t RISCV_SGNJ(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0010001, b, a, 0b00, d, 0b1010011);
}
int64_t RISCV_SGNJN(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0010001, b, a, 0b01, d, 0b1010011);
}
int64_t RISCV_SGNJX(int64_t d, int64_t a, int64_t b) {
  return RISCV_R(0b0010001, b, a, 0b10, d, 0b1010011);
}
// UNTESTED TODO TODO TODO
int64_t RISCV_FCVT_L_D(int64_t d, int64_t a) {
  return RISCV_I(0b110000100010, a, ROUND_MODE, d, 0b1010011);
}
int64_t RISCV_FCVT_D_L(int64_t d, int64_t a) {
  return RISCV_I(0b110100100010, a, ROUND_MODE, d, 0b1010011);
}
int64_t RISCV_FMV_X_D(int64_t d, int64_t a) {
  return RISCV_I(0b111000100000, a, 0, d, 0b1010011);
}
int64_t RISCV_FMV_D_X(int64_t d, int64_t a) {
  return RISCV_I(0b111100100000, a, 0, d, 0b1010011);
}
/*
int main() {
  int32_t test_pad[0x80],pad_ptr=0;
  test_pad[pad_ptr++]=RISCV_ADD(11,12,13);
  test_pad[pad_ptr++]=RISCV_ADDI(11,12,1234);
  test_pad[pad_ptr++]=RISCV_ADDIW(11,12,1234);
  test_pad[pad_ptr++]=RISCV_ADDW(11,12,13);
  test_pad[pad_ptr++]=RISCV_AND(11,12,13);
  test_pad[pad_ptr++]=RISCV_ANDI(11,12,1234);
  test_pad[pad_ptr++]=RISCV_AUIPC(11,1234);
  test_pad[pad_ptr++]=RISCV_BEQ(11,12,1);
  test_pad[pad_ptr++]=RISCV_BGE(11,12,1);
  test_pad[pad_ptr++]=RISCV_BGEU(11,12,1);
  test_pad[pad_ptr++]=RISCV_BLTU(11,12,1);
  test_pad[pad_ptr++]=RISCV_BLT(11,12,2);
  test_pad[pad_ptr++]=RISCV_BNE(11,12,1);
  test_pad[pad_ptr++]=RISCV_DIV(11,12,13);
  test_pad[pad_ptr++]=RISCV_DIVU(11,12,13);
  test_pad[pad_ptr++]=RISCV_ECALL();
  test_pad[pad_ptr++]=RISCV_FADD_D(11,12,13);
  test_pad[pad_ptr++]=RISCV_FDIV_D(11,12,13);
  test_pad[pad_ptr++]=RISCV_FMAX_D(11,12,13);
  test_pad[pad_ptr++]=RISCV_FMIN_D(11,12,13);
  test_pad[pad_ptr++]=RISCV_FMUL_D(11,12,13);
  test_pad[pad_ptr++]=RISCV_FSD(11,12,1234);
  test_pad[pad_ptr++]=RISCV_FLD(11,12,1234);
  test_pad[pad_ptr++]=RISCV_FSQRT_D(11,12);
  test_pad[pad_ptr++]=RISCV_FSUB_D(11,12,13);
  test_pad[pad_ptr++]=RISCV_JAL(11,4);
  test_pad[pad_ptr++]=RISCV_JALR(11,12,1234);
  test_pad[pad_ptr++]=RISCV_LB(11,12,1234);
  test_pad[pad_ptr++]=RISCV_LBU(11,12,1234);
  test_pad[pad_ptr++]=RISCV_LD(11,12,1234);
  test_pad[pad_ptr++]=RISCV_LH(11,12,1234);
  test_pad[pad_ptr++]=RISCV_LHU(11,12,1234);
  test_pad[pad_ptr++]=RISCV_LUI(11,1234);
  test_pad[pad_ptr++]=RISCV_LWU(11,12,1234);
  test_pad[pad_ptr++]=RISCV_MUL(11,12,13);
  test_pad[pad_ptr++]=RISCV_MULHSU(11,12,13);
  test_pad[pad_ptr++]=RISCV_MULH(11,12,13);
  test_pad[pad_ptr++]=RISCV_MULHU(11,12,13);
  test_pad[pad_ptr++]=RISCV_OR(11,12,13);
  test_pad[pad_ptr++]=RISCV_ORI(11,12,1234);
  test_pad[pad_ptr++]=RISCV_REM(11,12,13);
  test_pad[pad_ptr++]=RISCV_REMU(11,12,13);
  test_pad[pad_ptr++]=RISCV_SB(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SD(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SH(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SW(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SLL(11,12,13);
  test_pad[pad_ptr++]=RISCV_SLLI(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SLLIW(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SLLW(11,12,13);
  test_pad[pad_ptr++]=RISCV_SLT(11,12,13);
  test_pad[pad_ptr++]=RISCV_SLTI(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SLTIU(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SRAI(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SRAIW(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SRAW(11,12,13);
  test_pad[pad_ptr++]=RISCV_SRLI(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SRLIW(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SRLW(11,12,13);
  test_pad[pad_ptr++]=RISCV_SUB(11,12,13);
  test_pad[pad_ptr++]=RISCV_SUBW(11,12,13);
  test_pad[pad_ptr++]=RISCV_XOR(11,12,13);
  test_pad[pad_ptr++]=RISCV_XORI(11,12,1234);
  test_pad[pad_ptr++]=RISCV_SGNJ(11,12,13);
  test_pad[pad_ptr++]=RISCV_SGNJN(11,12,13);
  test_pad[pad_ptr++]=RISCV_SGNJX(11,12,13);
  printf("done,%n!",(double)pad_ptr);
  return 0;
}
*/
