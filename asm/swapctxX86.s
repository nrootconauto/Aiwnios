.intel_syntax noprefix
.global AIWNIOS_getcontext
.global AIWNIOS_setcontext
.global AIWNIOS_makecontext

AIWNIOS_getcontext:
  pop rdx
  pop rax
  mov qword ptr [rax+0*8],rdx
  mov qword ptr [rax+1*8],rsp
  mov qword ptr [rax+2*8],rbp
  mov qword ptr [rax+3*8],r11
  mov qword ptr [rax+4*8],r12
  mov qword ptr [rax+5*8],r13
  mov qword ptr [rax+6*8],r14
  mov qword ptr [rax+7*8],r15
  movsd qword ptr [rax+9*8],xmm6 # qword because double
  movsd qword ptr [rax+10*8],xmm7
  movsd qword ptr [rax+11*8],xmm8
  movsd qword ptr [rax+12*8],xmm9
  movsd qword ptr [rax+13*8],xmm10
  movsd qword ptr [rax+14*8],xmm11
  movsd qword ptr [rax+15*8],xmm12
  movsd qword ptr [rax+16*8],xmm13
  movsd qword ptr [rax+17*8],xmm14
  movsd qword ptr [rax+19*8],xmm15
  mov qword ptr [rax+20*8],r10
  mov qword ptr [rax+21*8],rdi
  mov qword ptr [rax+22*8],rsi
  mov rax,0
  jmp rdx

AIWNIOS_setcontext:
  mov rax,qword ptr [rsp+8]
  mov rcx,qword ptr [rax+0*8]
  mov rsp,qword ptr [rax+1*8]
  mov rbp,qword ptr [rax+2*8]
  mov r11,qword ptr [rax+3*8]
  mov r12,qword ptr [rax+4*8]
  mov r13,qword ptr [rax+5*8]
  mov r14,qword ptr [rax+6*8]
  mov r15,qword ptr [rax+7*8]
  movsd xmm6,qword ptr [rax+9*8]
  movsd xmm7,qword ptr [rax+10*8]
  movsd xmm8,qword ptr [rax+11*8]
  movsd xmm9,qword ptr [rax+12*8]
  movsd xmm10,qword ptr [rax+13*8]
  movsd xmm11,qword ptr [rax+14*8]
  movsd xmm12,qword ptr [rax+15*8]
  movsd xmm13,qword ptr [rax+16*8]
  movsd xmm14,qword ptr [rax+17*8]
  movsd xmm15,qword ptr [rax+19*8]
  mov r10,qword ptr [rax+20*8]
  mov rdi,qword ptr [rax+21*8]
  mov rsi,qword ptr [rax+22*8]
  mov rax,1
  jmp rcx

AIWNIOS_makecontext:
  mov qword ptr [rdi+0*8],rsi
  mov qword ptr [rdi+1*8],rdx
  ret
