.intel_syntax noprefix

.global FFI_CALL_TOS_0
.global FFI_CALL_TOS_1
.global FFI_CALL_TOS_2
.global FFI_CALL_TOS_3
.global FFI_CALL_TOS_4
.global FFI_CALL_TOS_CUSTOM_BP

FFI_CALL_TOS_0:
  push rbp
  mov rbp,rsp
  AND RSP,-0x10
  push 0
  push rbx
  call rdi
  pop rbx
  leave
  ret


FFI_CALL_TOS_1:
  push rbp
  mov rbp,rsp
  AND RSP,-0x10
  push rbx
  push rsi
  call rdi
  pop rbx
  leave
  ret

FFI_CALL_TOS_2:
  push rbp
  mov rbp,rsp
  AND RSP,-0x10
  push 0
  push rbx
  push rdx
  push rsi
  call rdi
  pop rbx
  leave
  ret

FFI_CALL_TOS_3:
  push rbp
  mov rbp,rsp
  AND RSP,-0x10
  push rbx
  push rcx
  push rdx
  push rsi
  call rdi
  pop rbx
  leave
  ret

FFI_CALL_TOS_4:
  push rbp
  mov rbp,rsp
  AND RSP,-0x10
  push 0
  push rbx
  push r8
  push rcx
  push rdx
  push rsi
  call rdi
  pop rbx
  leave
  ret

FFI_CALL_TOS_CUSTOM_BP:
	push	rbp
	push	rbx
	mov	rbp,rdi
	push	rdx
	call	rsi
	pop	rbx
	pop	rbp
	ret
