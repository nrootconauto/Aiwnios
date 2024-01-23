#include "aiwn.h"
#include <string.h>
#if defined(_WIN32) || defined(WIN32)
void *GenFFIBinding(void *fptr, int64_t arity) {
  /*
  0:  55                      push   rbp
  1:  48 89 e5                mov    rbp,rsp
  4:  48 83 e4 f0             and    rsp,0xfffffffffffffff0
  8:  41 52                   push   r10
  a:  41 53                   push   r11
  c:  48 83 ec 20             sub    rsp,0x20
  10: 48 b8 55 44 33 22 11    movabs rax,0x1122334455
  17: 00 00 00
  1a: 48 8d 4d 10             lea    rcx,[rbp+0x10]
  1e: ff d0                   call   rax
  20: 48 83 c4 20             add    rsp,0x20
  24: 41 5b                   pop    r11
  26: 41 5a                   pop    r10
  28: c9                      leave
  29: c2 34 12                ret    0x1234 */
  char *ffi_binding =
      "\x55\x48\x89\xE5\x48\x83\xE4\xF0\x41\x52\x41\x53\x48\x83\xEC\x20\x48\xB8"
      "\x55\x44\x33\x22\x11\x00\x00\x00\x48\x8D\x4D\x10\xFF\xD0\x48\x83\xC4\x20"
      "\x41\x5B\x41\x5A\xC9\xC2\x34\x12";
  char *ret = A_MALLOC(0x2c, NULL);
  memcpy(ret, ffi_binding, 0x2c);
  memcpy(ret + 0x12, &fptr, 0x8);
  arity *= 8;
  memcpy(ret + 0x2a, &arity, 0x2);
  return ret;
}
void *GenFFIBindingNaked(void *fptr, int64_t arity) {
  /*
  0:  \x48\x8D\x4C\x24\x08          lea    rcx,[rsp+0x8]
  5:  48 b8 55 44 33 22 11    movabs rax,0x1122334455
  c:  00 00 00
  f:  ff e0 					jmp rax
  */
  const char *ffi_binding =
      "\x48\x8D\x4C\x24\x08\x48\xB8\x55\x44\x33\x22\x11\x00\x00\x00\xFF\xe0";
  char *ret = A_MALLOC(0x12, NULL);
  memcpy(ret, ffi_binding, 0x12);
  memcpy(ret + 0x7, &fptr, 0x8);
  return ret;
}
#elif (defined(__linux__) || defined(__FreeBSD__)) && defined(__x86_64__)
void *GenFFIBinding(void *fptr, int64_t arity) {
  /*
0:  55                      push   rbp
1:  48 89 e5                mov    rbp,rsp
4:  48 83 e4 f0             and    rsp,0xfffffffffffffff0
8:  56                      push   rsi
9:  57                      push   rdi
a:  41 52                   push   r10
c:  41 53                   push   r11
e:  48 8d 7d 10             lea    rdi,[rbp+0x10]
12: 48 b8 55 44 33 22 11    movabs rax,0x1122334455
19: 00 00 00
1c: ff d0                   call   rax
1e: 41 5b                   pop    r11
20: 41 5a                   pop    r10
22: 5f                      pop    rdi
23: 5e                      pop    rsi
24: c9                      leave
25: c2 11 00                ret    0x11
*/
  // Look at the silly sauce at
  // https://defuse.ca/online-x86-assembler.htm#disassembly Is 0x22 bytes long
  const char *ffi_binding =
      "\x55\x48\x89\xE5\x48\x83\xE4\xF0\x56\x57\x41\x52\x41\x53\x48\x8D\x7D\x10"
      "\x48\xB8\x55\x44\x33\x22\x11\x00\x00\x00\xFF\xD0\x41\x5B\x41\x5A\x5F\x5E"
      "\xC9\xC2\x11\x00";
  char *ret = A_MALLOC(0x28, NULL);
  memcpy(ret, ffi_binding, 0x28);
  memcpy(ret + 0x14, &fptr, 0x8); // in place of 0x1122334455
  arity *= 8;
  memcpy(ret + 0x26, &arity, 0x2); // in place of 0x1122
  return ret;
}
void *GenFFIBindingNaked(void *fptr, int64_t arity) {
  /*
  0:  48 b8 55 44 33 22 11    movabs rax,0x1122334455
  7:  00 00 00
  a:  ff e0 					jmp rax
  */
  const char *ffi_binding = "\x48\xB8\x55\x44\x33\x22\x11\x00\x00\x00\xFF\xe0";
  char       *ret         = A_MALLOC(0xd, NULL);
  memcpy(ret, ffi_binding, 0xd);
  memcpy(ret + 0x2, &fptr, 0x8);
  return ret;
}
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
void *GenFFIBinding(void *fptr, int64_t arity) {
  // 0:  stp x29,x30[sp,-16]!
  // 4:  add x0,sp,16
  // 8:  ldr x1,label
  // c:  blr x1
  // 10: ldp x29,x30[sp],16
  // 14: ret
  // 18: label: fptr
  int32_t *blob = A_MALLOC(0x18 + 8, NULL);
  blob[0]       = ARM_stpPreImmX(29, 30, 31, -16);
  blob[1]       = ARM_addImmX(0, 31, 16);
  blob[2]       = ARM_ldrLabelX(1, 0x18 - 0x8);
  blob[3]       = ARM_blr(1);
  blob[4]       = ARM_ldpPostImmX(29, 30, 31, 16);
  blob[5]       = ARM_ret();
  memcpy(blob + 6, &fptr, sizeof(void *));
  return blob;
}
void *GenFFIBindingNaked(void *fptr, int64_t arity) {
  return fptr;
}
#endif
#if defined (__riscv__) || defined (__riscv)
void *GenFFIBinding(void *fptr, int64_t arity) {
  int32_t *blob = A_MALLOC(8*11, NULL);
  blob[0]=RISCV_ADDI(10,2,0); //2 is stack pointer,10 is 1st argument
  blob[1]=RISCV_ADDI(2,2,-48);
  blob[2]=RISCV_SD(1,2,40); //1 is return address
  blob[3]=RISCV_AUIPC(6,0);//6 is 1st temporoary
  blob[4]=RISCV_LD(6,6,4*(1+(10-4))); //+4 for LD,+4 for JALR,+4 for AUIPC(address starts at AUIPC)
  blob[5]=RISCV_JALR(1,6,0);
  blob[6]=RISCV_LD(1,2,40); //1 is return address
  blob[7]=RISCV_ADDI(2,2,48+arity*8);
  blob[8]=RISCV_JALR(0,1,0);
  *(void**)(blob+10)=fptr; //16 aligned
  return blob;
}
void *GenFFIBindingNaked(void *fptr, int64_t arity) {
  return fptr;
}
#endif
