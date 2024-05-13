.intel_syntax noprefix

.global TempleOS_CallN
.global TempleOS_Call
.global TempleOS_CallVaArgs
# I64 Call(fptr)
TempleOS_Call:
  push rbp
  mov  rbp,rsp
  mov rax,qword ptr[rbp+16]
  test rax,rax
  jz .Lnull
  call rax
.Lnull:
  pop rbp
  ret 8
# I64 CallArgs(fptr,argc,argv)
TempleOS_CallN:
  push rbp
  mov rbp,rsp
  push rsi
  push rdi
  mov rcx,[rbp+3*8] #argc
  mov rsi,[rbp+4*8] #argv 
  shl rcx,3
  sub rsp,rcx
  mov rdi,rsp
  rep movsb
  call qword ptr [rbp+2*8] #fptr
  pop rdi
  pop rsi
  pop rbp
  ret 3*8
# I64 CallVaArgs(fptr,argc1,argv1,...)
# fptr is a variadic fun with argc normal args
# argc1/argv1 are propogated for fptr's varargs
TempleOS_CallVaArgs:
  push rbp
  mov rbp,rsp
  push rsi
  push rdi

  mov rcx,[rbp+3*8] #argc1
  mov rsi,[rbp+4*8] #argv1
  shl rcx,3
  sub rsp,rcx
  mov rdi,rsp
  rep movsb
  push qword ptr[rbp+3*8]

  mov rcx,[rbp+5*8] #argc
  lea rsi,[rbp+6*8] #argv 
  mov rax,rcx
  neg rax
  lea rsp,[rsp+8*rax]
  mov rdi,rsp
  rep movsq

  call qword ptr [rbp+2*8] #fptr
  mov rcx,[rbp+3*8]#argc1
  shl rcx,3
  add rsp,rcx
  mov rcx,[rbp+5*8]#argc
  inc rcx#we pushed argc
  shl rcx,3
  add rsp,rcx
  pop rdi
  pop rsi
  pop rbp
  ret
