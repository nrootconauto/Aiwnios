#include "aiwn_mem.h"
#include "aiwn_multic.h"
#include <string.h>
#ifdef __x86_64__
#  include "aiwn_lexparser.h" //For reigster names
int64_t X86PushReg(char *to, int64_t reg);
int64_t X86MovRegReg(char *to, int64_t reg, int64_t);
int64_t X86AndImm(char *to, int64_t reg, int64_t);
int64_t X86SubImm(char *to, int64_t reg, int64_t);
int64_t X86MovRegIndirF64(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off);
int64_t X86MovIndirRegF64(char *to, int64_t reg, int64_t scale, int64_t index,
                          int64_t base, int64_t off);
#  define A(f, a...) code_off += f(bin ? bin + code_off : NULL, a)
#  ifdef _WIN64
void *GenFFIBinding(void *fptr, int64_t arity) {
  int64_t code_off = 0;
  uint8_t *bin = NULL;
  while (1) {
    A(X86PushReg, RBP);
    A(X86MovRegReg, RBP, RSP);
    A(X86AndImm, RSP, ~0xF);
    A(X86PushReg, R10);
    A(X86PushReg, R11);
    A(X86SubImm32, RSP, 0x20);
    A(X86MovImm, RAX, fptr);
    A(X86LeaSIB, RCX, -1, -1, RBP, 0x10);
    A(X86CallReg, RAX);
    A(X86AddImm32, RSP, 0x20);
    A(X86PopReg, R11);
    A(X86PopReg, R10);
    A(X86Leave, 0);
    A(X86Ret, arity * 8);
    if (bin)
      break;
    bin = A_MALLOC(code_off, Fs->code_heap);
    code_off = 0;
  }
  return bin;
}
void *GenFFIBindingNaked(void *fptr, int64_t arity) {
  int64_t code_off = 0;
  uint8_t *bin = NULL;
  while (1) {
    A(X86LeaSIB, RCX, -1, -1, RSP, 0x8);
    A(X86MovImm, RAX, fptr);
    A(X86JmpReg, RAX);
    if (bin)
      break;
    bin = A_MALLOC(code_off, Fs->code_heap);
    code_off = 0;
  }
  return bin;
}
#  elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
void *GenFFIBinding(void *fptr, int64_t arity) {
  int64_t code_off = 0;
  uint8_t *bin = NULL, *xbin = NULL;
  int64_t reg;
  while (1) {
    A(X86PushReg, RBP);
    A(X86MovRegReg, RBP, RSP);
    A(X86SubImm32, RSP, AIWNIOS_FREG_CNT * 8);
    for (reg = 0; reg != AIWNIOS_FREG_CNT; reg++) {
      A(X86MovIndirRegF64, AIWNIOS_FREG_START + reg, -1, -1, RBP,
        -(1 + reg) * 8);
    }
    A(X86AndImm, RSP, ~0xF);
    A(X86PushReg, RSI);
    A(X86PushReg, RDI);
    A(X86PushReg, R10);
    A(X86PushReg, R11);
    A(X86LeaSIB, RDI, -1, -1, RBP, 0x10);
    A(X86MovImm, RAX, fptr);
    A(X86CallReg, RAX);
    A(X86PopReg, R11);
    A(X86PopReg, R10);
    A(X86PopReg, RDI);
    A(X86PopReg, RSI);
    for (reg = 0; reg != AIWNIOS_FREG_CNT; reg++) {
      A(X86MovRegIndirF64, AIWNIOS_FREG_START + reg, -1, -1, RBP,
        -(1 + reg) * 8);
    }
    A(X86Leave, 0);
    A(X86Ret, arity * 8);
    if (bin)
      break;
    xbin = A_MALLOC(code_off, Fs->code_heap);
    bin = MemGetWritePtr(xbin);
    code_off = 0;
  }
  return xbin;
}
void *GenFFIBindingNaked(void *fptr, int64_t arity) {
  int64_t code_off = 0;
  uint8_t *bin = NULL, *xbin = NULL;
  while (1) {
    A(X86MovImm, RAX, fptr);
    A(X86JmpReg, RAX);
    if (bin)
      break;
    xbin = A_MALLOC(code_off, Fs->code_heap);
    bin = MemGetWritePtr(xbin);
    code_off = 0;
  }
  return xbin;
}
#  endif
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#  include "aiwn_arm.h"
void *GenFFIBinding(void *fptr, int64_t arity) {
  int32_t *Xblob = A_MALLOC(8 * 21 + arity * 4, Fs->code_heap), ptr = 0;
  int32_t *blob = MemGetWritePtr(Xblob); // OpenBSD sexy mapping
  int64_t arg, fill;
  int64_t pop = 0, pad = 0;
  int old = SetWriteNP(0);
  if (arity & 1)
    arity++, pad = 8;
  if (arity)
    ;
  blob[ptr++] = ARM_subImmX(31, 31, pop = (arity < 8 ? arity : 8) * 8);
  for (arg = 0; arg < arity - !!pad; arg++) {
    if (arg >= 8)
      break;
    blob[ptr++] = ARM_strRegImmX(arg, 31, arg * 8 + pad);
  }
  blob[ptr++] =
      ARM_addImmX(0, 31, pad); // 31 is stack pointer,0 is 1st argument
  blob[ptr++] = ARM_stpPreImmX(29, 30, 31, -16);
  blob[ptr++] = ARM_movRegX(29, 31);
  fill = ptr;
  blob[ptr++] = ARM_ldrLabelX(8, 0); // 6 is 1st temporoary
  blob[ptr++] = ARM_blr(8);
  blob[ptr++] = ARM_ldpPostImmX(29, 30, 31, 16);
  if (pop)
    blob[ptr++] = ARM_addImmX(31, 31, pop);
  blob[ptr++]=ARM_fmovF64I64(0,0);
  blob[ptr++] = ARM_ret();
  if (ptr & 1)
    ptr++;                       // Align to 8
  *(void **)(blob + ptr) = fptr; // 16 aligned
  blob[fill] = ARM_ldrLabelX(8, (ptr - fill) * 4);
  SetWriteNP(old);
  return Xblob;
}
void *GenFFIBindingNaked(void *fptr, int64_t arity) {
  return fptr;
}
#endif
#if defined(__riscv__) || defined(__riscv)
#  include "aiwn_riscv.h"
void *GenFFIBinding(void *fptr, int64_t arity) {
  int32_t *blob = A_MALLOC(8 * 21 + arity * 4, Fs->code_heap), ptr = 0;
  int64_t arg, fill;
  int64_t top = 0;
  if (arity)
    blob[ptr++] = RISCV_ADDI(2, 2, -(arity < 8 ? arity : 8) * 8);
  for (arg = 0; arg < arity; arg++) {
    if (arg >= 8)
      break;
    top += 8;
    blob[ptr++] = RISCV_SD(10 + arg, 2, arg * 8);
  }
  blob[ptr++] = RISCV_ADDI(10, 2, 0); // 2 is stack pointer,10 is 1st argument
  blob[ptr++] = RISCV_SD(1, 2, -8);   // 1 is return address
  blob[ptr++] = RISCV_SD(8, 2, -16);  // 10 is old stack start
  blob[ptr++] = RISCV_ADDI(2, 2, -16);
  blob[ptr++] = RISCV_ADDI(8, 2, 16);
  blob[ptr++] = RISCV_ANDI(2, 2, ~0xf);
  blob[ptr++] = RISCV_AUIPC(6, 0); // 6 is 1st temporoary
  fill = ptr;
  blob[ptr++] = RISCV_LD(6, 6, 0); //+4 for LD,+4 for JALR,+4 for
                                   // AUIPC(address starts at AUIPC)
  blob[ptr++] = RISCV_JALR(1, 6, 0);
  blob[ptr++] = RISCV_LD(1, 8, -8); // 1 is return address
  blob[ptr++] = RISCV_LD(11, 2, -8);
  blob[ptr++] = RISCV_FMV_D_X(10, 10);
  blob[ptr++] = RISCV_ADDI(2, 8, top);
  blob[ptr++] = RISCV_LD(8, 8, -16);
  blob[ptr++] = RISCV_JALR(0, 1, 0);
  if (ptr & 1)
    ptr++;                       // Align to 8
  *(void **)(blob + ptr) = fptr; // 16 aligned
  blob[fill] = RISCV_LD(6, 6, (1 + ptr - fill) * 4);
  return blob;
}
void *GenFFIBindingNaked(void *fptr, int64_t arity) {
  return fptr;
}
#endif
