.intel_syntax noprefix
.global Btc
.global LBtc
.global Bt
.global Bts
.global Btr
.global LBtr
.global LBts
.global Misc_Caller
.global Misc_ForceYield
.global Misc_BP
.extern ForceYield0
Btc:
  btc qword ptr [rcx],rsi
  setc al
  movzx rax,al
  ret 

LBtc:
  lock btc qword ptr [rcx],rdx
  setc al
  movzx rax,al
  ret 

Bt:
  bt qword ptr [rcx],rdx
  setc al
  movzx rax,al
  ret 

Bts:
  bts qword ptr [rcx],rdx
  setc al
  movzx rax,al
  ret 

Btr:
  btr qword ptr [rcx],rdx
  setc al
  movzx rax,al
  ret 

LBtr:
  lock btr qword ptr [rcx],rdx
  setc al
  movzx rax,al
  ret

LBts:
  lock bts qword ptr [rcx],rdx
  setc al
  movzx rax,al
  ret 

Misc_Caller:
  push rbp
  mov rbp,rsp
  mov rax,rbp
.L_loop:
  test rcx,rcx
  jz .L_fin
  test rax,rax 
  jz .L_fail
  mov rax,qword ptr [rax]
  dec rcx
  jmp .L_loop
.L_fin:
  test rax,rax
  jz .L_fail
  mov rax,qword ptr[rax+8]
  leave
  ret
.L_fail:
  mov rax,0
  leave
  ret
Misc_ForceYield:
  PUSH RAX
  PUSH RBX
  PUSH RCX
  PUSH RDX
  PUSH RBP
  PUSH RSI
  PUSH RDI
  PUSH R8
  PUSH R9
  PUSH R10
  PUSH R11
  PUSH R12
  PUSH R13
  PUSH R14
  PUSH R15
  SUB RSP,0x30
  MOVSD QWORD PTR [RSP+0x28],XMM5
  MOVSD QWORD PTR [RSP+0x20],XMM4
  MOVSD QWORD PTR [RSP+0x18],XMM3
  MOVSD QWORD PTR [RSP+0x10],XMM2
  MOVSD QWORD PTR [RSP+0x8],XMM1
  MOVSD QWORD PTR [RSP],XMM0
  CALL ForceYield0
  MOVSD XMM5,QWORD PTR [RSP+0x28]
  MOVSD XMM4,QWORD PTR [RSP+0x20]
  MOVSD XMM3,QWORD PTR [RSP+0x18]
  MOVSD XMM2,QWORD PTR [RSP+0x10]
  MOVSD XMM1,QWORD PTR [RSP+0x8]
  MOVSD XMM0,QWORD PTR [RSP]
  ADD RSP,0x30
  POP R15
  POP R14
  POP R13
  POP R12
  POP R11
  POP R10
  POP R9
  POP R8
  POP RDI
  POP RSI
  POP RBP
  POP RDX
  POP RCX
  POP RBX
  POP RAX
  ret 
Misc_BP:
  mov rax,rbp
  ret
