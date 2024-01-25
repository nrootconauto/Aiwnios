.intel_syntax noprefix
.global Misc_Btc
.global Misc_LBtc
.global Misc_Caller
.global Misc_Bt
.global Misc_Bts
.global Misc_Btr
.global Misc_LBtr
.global Misc_LBts
.global Misc_BP

Misc_Btc:
  btc qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

Misc_LBtc:
  lock btc qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

Misc_Bt:
  bt qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

Misc_Bts:
  bts qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

Misc_Btr:
  btr qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret 

Misc_LBtr:
  lock btr qword ptr [rdi],rsi
  setc al
  movzx rax,al
  ret

Misc_LBts:
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
