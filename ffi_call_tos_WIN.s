.intel_syntax noprefix
.global FFI_CALL_TOS_0
.global FFI_CALL_TOS_1
.global FFI_CALL_TOS_2
.global FFI_CALL_TOS_3
.global FFI_CALL_TOS_4
.global FFI_CALL_TOS_CUSTOM_BP
# Poo Poo FFI for Windows64 to TempleOS

FFI_CALL_TOS_0:
  push rbp
  mov rbp,rsp
  and rsp,-0x10
  push rbx
  call rcx
  pop rbx
  leave
  ret

FFI_CALL_TOS_1:
  push rbp
  mov rbp,rsp
  and rsp,-0x10
  push rbx
  push rdx
  call rcx
  pop rbx 
  leave
  ret

FFI_CALL_TOS_2:
  push rbp
  mov rbp,rsp
  and rsp,-0x10
  push rbx
  push r8
  push rdx
  call rcx
  pop rbx
  leave
  ret

FFI_CALL_TOS_3:
  push rbp
  mov rbp,rsp
  and rsp,-0x10
  push rbx
  push r9
  push r8
  push rdx
  call rcx
  pop rbx
  leave
  ret

# https://docs.microsoft.com/en-us/cpp/build/stack-usage?view=msvc-170
# Fist stack arg:0x30
# R9 HOME+0x28 
# R8 HOME+0x20
# RDX HOME+0x18
# RCX HOME+0x10
# RET ADDR+8
# RBP
FFI_CALL_TOS_4:
  push rbp
  mov rbp,rsp
  and rsp,-0x10
  push rbx
  push qword ptr [rbp+0x30]
  push r9
  push r8
  push rdx
  call rcx
  pop rbx
  leave
  ret

FFI_CALL_TOS_CUSTOM_BP:
	push	rbp
	mov	rbp,rcx
	push	rbx
	push	r8
	call	rdx
	pop	rbx
	pop	rbp
	ret
