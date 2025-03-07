.intel_syntax noprefix
.global Btc
.global LBtc
.global Misc_Caller
.global Bt
.global Bts
.global Btr
.global LBtr
.global LBts
.global Misc_BP

Btc:
  btc qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

LBtc:
  lock btc qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

Bt:
  bt qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

Bts:
  bts qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

Btr:
  btr qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

LBtr:
  lock btr qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret

LBts:
  lock bts qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

Misc_Caller:
  push rbp
  mov rbp,rsp
  mov rcx,rdi
  mov rax,rbp
.L_loop: # .L means local label
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
Misc_BP:
  mov rax,rbp
  ret
