# -*- mode:intel-assembly; indent-tabs-mode:t; tab-width:8; coding:utf-8 -*-
# vi: set noet ft=asm ts=8 sw=8 fenc=utf-8 :vi
.intel_syntax noprefix

.global TempleOS_CallN
.global TempleOS_Call
.global TempleOS_CallVaArgs

# I64 Call(fptr)
TempleOS_Call:
	push	rbp
	mov	rbp,rsp
	mov	rax,qword ptr[rbp+16]
	test	rax,rax
	jz	0f
	call	rax
0:	pop	rbp
	ret	8

# I64 CallArgs(fptr,argc,argv)
TempleOS_CallN:
	push	rbp
	mov	rbp,rsp
	push	rsi
	push	rdi
	mov	edi,[rbp+3*8] #argc
	test	edi,edi
	jz	1f
	mov	rcx,[rbp+4*8] #argv	
	mov	esi,edi
	shl	esi,3
	sub	rsp,rsi
	sub	edi,1
0:	mov	rbx,[rcx+rdi*8]
	mov	[rsp+rdi*8],rbx
	sub	edi,1
	jnb	0b
1:	call	qword ptr[rbp+2*8] #fptr
	pop	rdi
	pop	rsi
	pop	rbp
	ret	3*8

# I64 CallVaArgs(fptr,argc1,argv1,...)
# fptr is a variadic fun with argc normal args
# argc1/argv1 are propogated for fptr's varargs
TempleOS_CallVaArgs:
	push	rbp
	mov	rbp,rsp
	push	rsi
	push	rdi
	mov	ecx,dword ptr[rbp+3*8] #argc1
	test	ecx,ecx
	jz	1f
	mov	rsi,qword ptr[rbp+4*8] #argv1
	mov	edi,ecx
	shl	edi,3
	sub	rsp,rdi
	sub	ecx,1
0:	mov	rbx,[rsi+rcx*8]
	mov	[rsp+rcx*8],rbx
	sub	ecx,1
	jnb	0b
1:	push	qword ptr[rbp+3*8]

	mov	ecx,dword ptr[rbp+5*8] #argc
	test	ecx,ecx
	jz	3f
	lea	rsi,[rbp+6*8] #argv	
	mov	edi,ecx
	shl	edi,3
	sub	rsp,rdi
	sub	ecx,1
2:	mov	rbx,[rsi+rcx*8]
	mov	[rsp+rcx*8],rbx
	sub	ecx,1
	jnb	2b

3:	call	qword ptr[rbp+2*8] #fptr
	mov	ecx,[rbp+3*8]
	add	ecx,[rbp+5*8]
	lea	rsp,[rsp+rcx*8+8]
	pop	rdi
	pop	rsi
	pop	rbp
	ret
