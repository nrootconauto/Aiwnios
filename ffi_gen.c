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
8:  48 83 ec 60             sub    rsp,0x60
c:  f2 0f 11 74 24 08       movsd  QWORD PTR [rsp+0x8],xmm6
12: f2 0f 11 7c 24 10       movsd  QWORD PTR [rsp+0x10],xmm7
18: f2 44 0f 11 44 24 18    movsd  QWORD PTR [rsp+0x18],xmm8
1f: f2 44 0f 11 4c 24 20    movsd  QWORD PTR [rsp+0x20],xmm9
26: f2 44 0f 11 54 24 28    movsd  QWORD PTR [rsp+0x28],xmm10
2d: f2 44 0f 11 5c 24 30    movsd  QWORD PTR [rsp+0x30],xmm11
34: f2 44 0f 11 64 24 38    movsd  QWORD PTR [rsp+0x38],xmm12
3b: f2 44 0f 11 6c 24 40    movsd  QWORD PTR [rsp+0x40],xmm13
42: f2 44 0f 11 74 24 50    movsd  QWORD PTR [rsp+0x50],xmm14
49: f2 44 0f 11 7c 24 58    movsd  QWORD PTR [rsp+0x58],xmm15
50: 56                      push   rsi
51: 57                      push   rdi
52: 41 52                   push   r10
54: 41 53                   push   r11
56: 48 8d 7d 10             lea    rdi,[rbp+0x10]
5a: 48 b8 55 44 33 22 11    movabs rax,0x1122334455
61: 00 00 00
64: ff d0                   call   rax
66: 41 5b                   pop   r11
68: 41 5a                   pop   r10
6a: 5f                      pop    rdi
6b: 5e                      pop    rsi
6c: f2 0f 10 74 24 08       movsd  xmm6,QWORD PTR [rsp+0x8]
72: f2 0f 10 7c 24 10       movsd  xmm7,QWORD PTR [rsp+0x10]
78: f2 44 0f 10 44 24 18    movsd  xmm8,QWORD PTR [rsp+0x18]
7f: f2 44 0f 10 4c 24 20    movsd  xmm9,QWORD PTR [rsp+0x20]
86: f2 44 0f 10 54 24 28    movsd  xmm10,QWORD PTR [rsp+0x28]
8d: f2 44 0f 10 5c 24 30    movsd  xmm11,QWORD PTR [rsp+0x30]
94: f2 44 0f 10 64 24 38    movsd  xmm12,QWORD PTR [rsp+0x38]
9b: f2 44 0f 10 6c 24 40    movsd  xmm13,QWORD PTR [rsp+0x40]
a2: f2 44 0f 10 74 24 50    movsd  xmm14,QWORD PTR [rsp+0x50]
a9: f2 44 0f 10 7c 24 58    movsd  xmm15,QWORD PTR [rsp+0x58]
b0: 66 48 0f 6e c0          movq   xmm0,rax
b5: c9                      leave
b6: c2 22 11                ret    0x1122
*/
  // Look at the silly sauce at
  // https://defuse.ca/online-x86-assembler.htm#disassembly Is 0x22 bytes long
  const char *ffi_binding =
      "\x55\x48\x89\xE5\x48\x83\xE4\xF0\x48\x83\xEC\x60\xF2\x0F\x11\x74\x24\x08"
      "\xF2\x0F\x11\x7C\x24\x10\xF2\x44\x0F\x11\x44\x24\x18\xF2\x44\x0F\x11\x4C"
      "\x24\x20\xF2\x44\x0F\x11\x54\x24\x28\xF2\x44\x0F\x11\x5C\x24\x30\xF2\x44"
      "\x0F\x11\x64\x24\x38\xF2\x44\x0F\x11\x6C\x24\x40\xF2\x44\x0F\x11\x74\x24"
      "\x50\xF2\x44\x0F\x11\x7C\x24\x58\x56\x57\x41\x52\x41\x53\x48\x8D\x7D\x10"
      "\x48\xB8\x55\x44\x33\x22\x11\x00\x00\x00\xFF\xD0\x41\x5B\x41\x5A\x5F\x5E"
      "\xF2\x0F\x10\x74\x24\x08\xF2\x0F\x10\x7C\x24\x10\xF2\x44\x0F\x10\x44\x24"
      "\x18\xF2\x44\x0F\x10\x4C\x24\x20\xF2\x44\x0F\x10\x54\x24\x28\xF2\x44\x0F"
      "\x10\x5C\x24\x30\xF2\x44\x0F\x10\x64\x24\x38\xF2\x44\x0F\x10\x6C\x24\x40"
      "\xF2\x44\x0F\x10\x74\x24\x50\xF2\x44\x0F\x10\x7C\x24\x58\x66\x48\x0F\x6E"
      "\xC0\xC9\xC2\x22\x11";
  char *ret = A_MALLOC(0xb9, NULL);
  memcpy(ret, ffi_binding, 0xb9);
  memcpy(ret + 0x5c, &fptr, 0x8); // in place of 0x1122334455
  arity *= 8;
  memcpy(ret + 0xb7, &arity, 0x2); // in place of 0x1122
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
  // TODO
  return fptr;
}
#endif
