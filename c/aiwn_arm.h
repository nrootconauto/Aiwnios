#pragma once
#include <stddef.h>
#include <stdint.h>
#define ARM_ERR         -1
#define ARM_ERR_INV_OFF -2
#define ARM_ERR_INV_REG -3

#define LSL  0
#define LSR  1
#define ASR  2
#define ROR  3
#define MSL  4
#define NONE 5

#define EXT_NONE   0xffff
#define UXTB_32    0b000
#define UXTH_32    0b001
#define EXT_LSL_32 0b010
#define UXTW_32    0b010
#define UXTX_32    0b011
#define SXTB_32    0b100
#define SXTH_32    0b101
#define SXTW_32    0b110
#define SXTX_32    0b111

#define UXTB_64    0b000
#define UXTH_64    0b001
#define EXT_LSL_64 0b010
#define UXTW_64    0b010
#define UXTX_64    0b011
#define SXTB_64    0b100
#define SXTH_64    0b101
#define SXTW_64    0b110
#define SXTX_64    0b111

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

int64_t ARM_ldrsbRegRegX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrshRegRegX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrswRegRegX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrsbX(int64_t r, int64_t a, int64_t off);
int64_t ARM_ldrshX(int64_t r, int64_t a, int64_t off);
int64_t ARM_ldrswX(int64_t r, int64_t a, int64_t off);
int64_t ARM_ldur(int64_t r, int64_t a, int64_t off);
int64_t ARM_stur(int64_t r, int64_t a, int64_t off);
int64_t ARM_ldurswX(int64_t r, int64_t a, int64_t off);
int64_t ARM_ldurshX(int64_t r, int64_t a, int64_t off);
int64_t ARM_ldursbX(int64_t r, int64_t a, int64_t off);
int64_t ARM_sturw(int64_t r, int64_t a, int64_t off);
int64_t ARM_sturh(int64_t r, int64_t a, int64_t off);
int64_t ARM_sturb(int64_t r, int64_t a, int64_t off);
int64_t ARM_fmovF64I64(int64_t d, int64_t s);
int64_t ARM_fmovI64F64(int64_t d, int64_t s);
int64_t ARM_ldrRegRegF64(int64_t d, int64_t n, int64_t m);
int64_t ARM_strRegRegF64(int64_t d, int64_t n, int64_t m);
int64_t ARM_ldrPreImmF64(int64_t d, int64_t p, int64_t off);
int64_t ARM_strPreImmF64(int64_t d, int64_t p, int64_t off);
int64_t ARM_ldrPostImmF64(int64_t d, int64_t p, int64_t off);
int64_t ARM_strPostImmF64(int64_t d, int64_t p, int64_t off);
int64_t ARM_movkImmX(int64_t d, int64_t i16, int64_t sh);
int64_t ARM_movnImmX(int64_t d, int64_t i16, int64_t sh);
int64_t ARM_movzImmX(int64_t d, int64_t i16, int64_t sh);
int64_t ARM_asrvRegX(int64_t d, int64_t n, int64_t m);
int64_t ARM_lsrvRegX(int64_t d, int64_t n, int64_t m);
int64_t ARM_lslvRegX(int64_t d, int64_t n, int64_t m);
int64_t ARM_sdivRegX(int64_t d, int64_t n, int64_t m);
int64_t ARM_fcmp(int64_t a, int64_t b);
int64_t ARM_fmovReg(int64_t d, int64_t s);
int64_t ARM_fnegReg(int64_t d, int64_t s);
int64_t ARM_strRegImmF64(int64_t r, int64_t n, uint64_t off);
int64_t ARM_ldrRegImmF64(int64_t r, int64_t n, uint64_t off);
int64_t ARM_ldpPostImmX(int64_t r, int64_t r2, int64_t b, int64_t off);
int64_t ARM_ldpPostImm(int64_t r, int64_t r2, int64_t b, int64_t off);
int64_t ARM_stpPostImmX(int64_t r, int64_t r2, int64_t b, int64_t off);
int64_t ARM_stpPostImm(int64_t r, int64_t r2, int64_t b, int64_t off);
int64_t ARM_ldpPreImmX(int64_t r, int64_t r2, int64_t b, int64_t off);
int64_t ARM_ldpPreImm(int64_t r, int64_t r2, int64_t b, int64_t off);
int64_t ARM_stpPreImmX(int64_t r, int64_t r2, int64_t b, int64_t off);
int64_t ARM_stpPreImm(int64_t r, int64_t r2, int64_t b, int64_t off);
int64_t ARM_ldrPreImmX(int64_t r, int64_t b, int64_t off);
int64_t ARM_strPreImmX(int64_t r, int64_t b, int64_t off);
int64_t ARM_ldrPostImmX(int64_t r, int64_t b, int64_t off);
int64_t ARM_strPostImmX(int64_t r, int64_t b, int64_t off);
int64_t ARM_ldrPreImm(int64_t r, int64_t b, int64_t off);
int64_t ARM_strPreImm(int64_t r, int64_t b, int64_t off);
int64_t ARM_ldrPostImm(int64_t r, int64_t b, int64_t off);
int64_t ARM_strPostImm(int64_t r, int64_t b, int64_t off);
int64_t ARM_ldrLabelF64(int64_t d, int64_t label);
int64_t ARM_ldrLabelX(int64_t d, int64_t label);
int64_t ARM_ldrLabel(int64_t d, int64_t label);
int64_t ARM_fcvtzs(int64_t d, int64_t n);
int64_t ARM_ucvtf(int64_t d, int64_t n);
int64_t ARM_scvtf(int64_t d, int64_t n);
int64_t ARM_fsubReg(int64_t d, int64_t n, int64_t m);
int64_t ARM_faddReg(int64_t d, int64_t n, int64_t m);
int64_t ARM_fdivReg(int64_t d, int64_t n, int64_t m);
int64_t ARM_fmulReg(int64_t d, int64_t n, int64_t m);
int64_t ARM_ldrRegRegX(int64_t r, int64_t a, int64_t b);
int64_t ARM_strRegRegX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrRegImmX(int64_t r, int64_t n, int64_t off);
int64_t ARM_strRegImmX(int64_t r, int64_t n, int64_t off);
int64_t ARM_ldrRegImm(int64_t r, int64_t n, int64_t off);
int64_t ARM_ldrRegReg(int64_t r, int64_t a, int64_t b);
int64_t ARM_strRegReg(int64_t r, int64_t a, int64_t b);
int64_t ARM_strRegImm(int64_t r, int64_t n, int64_t off);
int64_t ARM_ldrhRegImm(int64_t r, int64_t n, int64_t off);
int64_t ARM_ldrhRegReg(int64_t r, int64_t a, int64_t b);
int64_t ARM_strhRegImm(int64_t r, int64_t n, int64_t off);
int64_t ARM_strhRegReg(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrbRegReg(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrbRegImm(int64_t r, int64_t n, int64_t off);
int64_t ARM_strbRegImm(int64_t r, int64_t n, int64_t off);
int64_t ARM_strbRegReg(int64_t r, int64_t a, int64_t b);
int64_t ARM_tbnz(int64_t rt, int64_t bit, int64_t label);
int64_t ARM_tbz(int64_t rt, int64_t bit, int64_t label);
int64_t ARM_cbnzX(int64_t r, int64_t label);
int64_t ARM_cbnz(int64_t r, int64_t label);
int64_t ARM_cbzX(int64_t r, int64_t label);
int64_t ARM_cbz(int64_t r, int64_t label);
int64_t ARM_bl(int64_t lab);
int64_t ARM_b(int64_t lab);
int64_t ARM_bcc(int64_t cond, int64_t lab);
int64_t ARM_retReg(int64_t r);
int64_t ARM_blr(int64_t r);
int64_t ARM_br(int64_t r);
int64_t ARM_eret();
int64_t ARM_ret();
int64_t ARM_lsrImmX(int64_t d, int64_t n, int64_t imm);
int64_t ARM_lsrImm(int64_t d, int64_t n, int64_t imm);
int64_t ARM_lslImmX(int64_t d, int64_t n, int64_t imm);
int64_t ARM_lslImm(int64_t d, int64_t n, int64_t imm);
int64_t ARM_asrImmX(int64_t d, int64_t n, int64_t imm);
int64_t ARM_asrImm(int64_t d, int64_t n, int64_t imm);
int64_t ARM_sxtwX(int64_t d, int64_t n);
int64_t ARM_sxtw(int64_t d, int64_t n);
int64_t ARM_sxthX(int64_t d, int64_t n);
int64_t ARM_sxth(int64_t d, int64_t n);
int64_t ARM_sxtbX(int64_t d, int64_t n);
int64_t ARM_sxtb(int64_t d, int64_t n);
int64_t ARM_uxtwX(int64_t d, int64_t n);
int64_t ARM_uxthX(int64_t d, int64_t n);
int64_t ARM_uxth(int64_t d, int64_t n);
int64_t ARM_uxtbX(int64_t d, int64_t n);
int64_t ARM_uxtb(int64_t d, int64_t n);
int64_t ARM_mulRegX(int64_t d, int64_t n, int64_t m);
int64_t ARM_csetX(int64_t d, int64_t cond);
int64_t ARM_cset(int64_t d, int64_t cond);
int64_t ARM_cselX(int64_t d, int64_t n, int64_t m, int64_t cond);
int64_t ARM_csel(int64_t d, int64_t n, int64_t m, int64_t cond);
int64_t ARM_cmpRegX(int64_t n, int64_t m);
int64_t ARM_negsRegX(int64_t d, int64_t m);
int64_t ARM_subRegX(int64_t d, int64_t n, int64_t m);
int64_t ARM_addsRegX(int64_t d, int64_t n, int64_t m);
int64_t ARM_addRegX(int64_t d, int64_t n, int64_t m);
int64_t ARM_movRegX(int64_t d, int64_t n);
int64_t ARM_movReg(int64_t d, int64_t n);
int64_t ARM_andsRegX(int64_t rd, int64_t rn, int64_t rm);
int64_t ARM_eorRegX(int64_t rd, int64_t rn, int64_t rm);
int64_t ARM_eorShiftX(int64_t rd, int64_t rn, int64_t rm, int64_t shmod,
                      int64_t sh);
int64_t ARM_mvnRegX(int64_t rd, int64_t rm);
int64_t ARM_mvnReg(int64_t rd, int64_t rm);
int64_t ARM_mvnShiftX(int64_t rd, int64_t rm, int64_t shmod, int64_t sh);
int64_t ARM_mvnShift(int64_t rd, int64_t rm, int64_t shmod, int64_t sh);
int64_t ARM_orrRegX(int64_t rd, int64_t rn, int64_t rm);
int64_t ARM_andRegX(int64_t rd, int64_t rn, int64_t rm);
int64_t ARM_cmpImmX(int64_t n, int64_t imm);
int64_t ARM_subImmX(int64_t d, int64_t n, int64_t imm);
int64_t ARM_addImmX(int64_t d, int64_t n, int64_t imm);
int64_t ARM_adrX(int64_t d, int64_t pc_off);
int64_t ARM_strbRegRegShift(int64_t a, int64_t n, int64_t m);
int64_t ARM_ldrbRegRegShift(int64_t a, int64_t n, int64_t m);
int64_t ARM_strbRegRegShift(int64_t a, int64_t n, int64_t m);
int64_t ARM_ldrhRegRegShift(int64_t a, int64_t n, int64_t m);
int64_t ARM_strhRegRegShift(int64_t a, int64_t n, int64_t m);
int64_t ARM_ldrRegRegShift(int64_t a, int64_t n, int64_t m);
int64_t ARM_strRegRegShift(int64_t a, int64_t n, int64_t m);
int64_t ARM_ldrRegRegShiftX(int64_t a, int64_t n, int64_t m);
int64_t ARM_strRegRegShiftX(int64_t a, int64_t n, int64_t m);
int64_t ARM_ldpImmX(int64_t r1, int64_t r2, int64_t ra, int64_t off);
int64_t ARM_stpImmX(int64_t r1, int64_t r2, int64_t ra, int64_t off);
int64_t ARM_fcsel(int64_t d, int64_t a, int64_t b, int64_t c);

int64_t ARM_ldaxrb(int64_t, int64_t);
int64_t ARM_ldaxrh(int64_t, int64_t);
int64_t ARM_ldaxr(int64_t, int64_t);
int64_t ARM_ldaxrX(int64_t, int64_t);
int64_t ARM_stlxrb(int64_t, int64_t, int64_t);
int64_t ARM_stlxrh(int64_t, int64_t, int64_t);
int64_t ARM_stlxr(int64_t, int64_t, int64_t);
int64_t ARM_stlxrX(int64_t, int64_t, int64_t);
int64_t ARM_subShiftRegX(int64_t d, int64_t n, int64_t m, int64_t sh);
int64_t ARM_addShiftRegX(int64_t d, int64_t n, int64_t m, int64_t sh);

int64_t ARM_eorImmX(int64_t d, int64_t s, int64_t i);
int64_t ARM_orrImmX(int64_t d, int64_t s, int64_t i);
int64_t ARM_andImmX(int64_t d, int64_t s, int64_t i);
int64_t ARM_andsImmX(int64_t d, int64_t n, int64_t imm);
int64_t ARM_udivX(int64_t d, int64_t n, int64_t m);
int64_t ARM_umullX(int64_t d, int64_t n, int64_t m);
int64_t ARM_subsRegX(int64_t d, int64_t n, int64_t m);
int64_t ARM_subsImmX(int64_t d, int64_t n, int64_t m);
int64_t ARM_addsImmX(int64_t d, int64_t n, int64_t m);
int64_t ARM_subShiftRegXs(int64_t d, int64_t n, int64_t m, int64_t sh);
int64_t ARM_addShiftRegXs(int64_t d, int64_t n, int64_t m, int64_t sh);

int64_t ARM_ldrsbRegRegShiftX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrshRegRegShiftX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrswRegRegShiftX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrbRegRegShiftX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldhbRegRegShiftX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrRegRegShiftX(int64_t r, int64_t a, int64_t b);
int64_t ARM_ldrRegRegShiftF64(int64_t r, int64_t a, int64_t b);
int64_t ARM_strsbRegRegShiftX(int64_t r, int64_t a, int64_t b);
int64_t ARM_strhbRegRegShiftX(int64_t r, int64_t a, int64_t b);
int64_t ARM_strRegRegShiftX(int64_t r, int64_t a, int64_t b);
int64_t ARM_strRegRegShiftF64(int64_t r, int64_t a, int64_t b);
int64_t ARM_stpImmF64(int64_t r, int64_t r2, int64_t b, int64_t o);
int64_t ARM_ldpImmF64(int64_t r, int64_t r2, int64_t b, int64_t o);
