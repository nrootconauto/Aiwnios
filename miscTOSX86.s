.intel_syntax noprefix

.global TempleOS_CallN
# I64 CallN(fptr,argc,argv)
TempleOS_CallN:
  push rbp
  mov rbp,rsp
  mov rdx,[rbp+4*8] #argv 
  mov rcx,0
  shl qword ptr [rbp+3*8],3
  sub rsp,qword ptr [rbp+3*8]
.L_loop:
  cmp rcx,[rbp+3*8] #argc
  jz .L_en
  mov rax,qword ptr [rdx+rcx]
  mov qword ptr [rsp+rcx],rax
  add rcx,8
  jmp .L_loop
.L_en:
  call qword ptr [rbp+2*8] #fptr
  leave
  ret 3*8
