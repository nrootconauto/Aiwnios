#include "aiwn.h"
void *GenFFIBinding(void *fptr,int64_t arity) {
	#ifdef USE_TEMPLEOS_ABI
	//Here's the deal,i will save RDI/RSI on windows and SYSTEMV 
	/*
0:  55                      push   rbp
1:  48 89 e5                mov    rbp,rsp
4:  48 83 e4 f0             and    rsp,0xfffffffffffffff0
8:  48 83 ec 58             sub    rsp,0x58
c:  f2 0f 11 74 24 08       movsd  QWORD PTR [rsp+0x8],xmm6
12: f2 0f 11 7c 24 10       movsd  QWORD PTR [rsp+0x10],xmm7
18: f2 44 0f 11 44 24 18    movsd  QWORD PTR [rsp+0x18],xmm8
1f: f2 44 0f 11 4c 24 20    movsd  QWORD PTR [rsp+0x20],xmm9
26: f2 44 0f 11 54 24 28    movsd  QWORD PTR [rsp+0x28],xmm10
2d: f2 44 0f 11 5c 24 30    movsd  QWORD PTR [rsp+0x30],xmm11
34: f2 44 0f 11 64 24 38    movsd  QWORD PTR [rsp+0x38],xmm12
3b: f2 44 0f 11 6c 24 40    movsd  QWORD PTR [rsp+0x40],xmm13
42: f2 44 0f 11 74 24 48    movsd  QWORD PTR [rsp+0x48],xmm14
49: f2 44 0f 11 7c 24 50    movsd  QWORD PTR [rsp+0x50],xmm15
50: 53                      push   rbx
51: 57                      push   rdi
52: 56                      push   rsi
53: 48 8d 7d 10             lea    rdi,[rbp+0x10]
57: 48 b8 55 44 33 22 11    movabs rax,0x1122334455
5e: 00 00 00
61: ff d0                   call   rax
63: 5f                      pop    rdi
64: 5e                      pop    rsi
65: 5b                      pop    rbx
66: f2 0f 10 74 24 08       movsd  xmm6,QWORD PTR [rsp+0x8]
6c: f2 0f 10 7c 24 10       movsd  xmm7,QWORD PTR [rsp+0x10]
72: f2 44 0f 10 44 24 18    movsd  xmm8,QWORD PTR [rsp+0x18]
79: f2 44 0f 10 4c 24 20    movsd  xmm9,QWORD PTR [rsp+0x20]
80: f2 44 0f 10 54 24 28    movsd  xmm10,QWORD PTR [rsp+0x28]
87: f2 44 0f 10 5c 24 30    movsd  xmm11,QWORD PTR [rsp+0x30]
8e: f2 44 0f 10 64 24 38    movsd  xmm12,QWORD PTR [rsp+0x38]
95: f2 44 0f 10 6c 24 40    movsd  xmm13,QWORD PTR [rsp+0x40]
9c: f2 44 0f 10 74 24 48    movsd  xmm14,QWORD PTR [rsp+0x48]
a3: f2 44 0f 10 7c 24 50    movsd  xmm15,QWORD PTR [rsp+0x50]
aa: 66 48 0f 6e c0          movq   xmm0,rax
af: c9                      leave
b0: c3                      ret 
*/ 
	//Look at the silly sauce at https://defuse.ca/online-x86-assembler.htm#disassembly
	//Is 0x22 bytes long
	const char *ffi_binding="\x55\x48\x89\xE5\x48\x83\xE4\xF0\x48\x83\xEC\x58\xF2\x0F\x11\x74\x24\x08\xF2\x0F\x11\x7C\x24\x10\xF2\x44\x0F\x11\x44\x24\x18\xF2\x44\x0F\x11\x4C\x24\x20\xF2\x44\x0F\x11\x54\x24\x28\xF2\x44\x0F\x11\x5C\x24\x30\xF2\x44\x0F\x11\x64\x24\x38\xF2\x44\x0F\x11\x6C\x24\x40\xF2\x44\x0F\x11\x74\x24\x48\xF2\x44\x0F\x11\x7C\x24\x50\x53\x57\x56\x48\x8D\x7D\x10\x48\xB8\x55\x44\x33\x22\x11\x00\x00\x00\xFF\xD0\x5F\x5E\x5B\xF2\x0F\x10\x74\x24\x08\xF2\x0F\x10\x7C\x24\x10\xF2\x44\x0F\x10\x44\x24\x18\xF2\x44\x0F\x10\x4C\x24\x20\xF2\x44\x0F\x10\x54\x24\x28\xF2\x44\x0F\x10\x5C\x24\x30\xF2\x44\x0F\x10\x64\x24\x38\xF2\x44\x0F\x10\x6C\x24\x40\xF2\x44\x0F\x10\x74\x24\x48\xF2\x44\x0F\x10\x7C\x24\x50\x66\x48\x0F\x6E\xC0\xC9\xC3";
	char *ret=A_MALLOC(0xb1,NULL);
	memcpy(ret,ffi_binding,0xb1);
	*(int64_t*)(ret+0x59)=fptr;
	return ret;
	#else
	return fptr;
	#endif
}
void *GenFFIBindingNaked(void *fptr,int64_t arity) {
/*
0:  48 8d 7c 24 08          lea    rdi,[rsp+0x8]
5:  48 b8 55 44 33 22 11    movabs rax,0x1122334455
c:  00 00 00
f:  ff e0 					jmp rax
*/
	const char  *ffi_binding="\x48\x8D\x7C\x24\x08\x48\xB8\x55\x44\x33\x22\x11\x00\x00\x00\xFF\xe0";
	#ifdef USE_TEMPLEOS_ABI
	char *ret=A_MALLOC(0x12,NULL);
	memcpy(ret,ffi_binding,0x12);
	*(int64_t*)(ret+0x7)=fptr;
	return ret;
	#else
	return fptr;
	#endif
}
