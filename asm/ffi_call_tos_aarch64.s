.text
.global FFI_CALL_TOS_0
.global FFI_CALL_TOS_1
.global FFI_CALL_TOS_2
.global FFI_CALL_TOS_3
.global FFI_CALL_TOS_4
#For MacOS
.global _FFI_CALL_TOS_0
.global _FFI_CALL_TOS_1
.global _FFI_CALL_TOS_2
.global _FFI_CALL_TOS_3
.global _FFI_CALL_TOS_4

_FFI_CALL_TOS_0:
FFI_CALL_TOS_0:
  stp x29,x30,[sp,-16]!
  mov x8,0
  stp x8,x8,[sp,-16]!
  mov x29,sp
  blr x0
  add sp,sp,16
  ldp x29,x30,[sp],16
  ret
_FFI_CALL_TOS_1:
FFI_CALL_TOS_1:
  stp x29,x30,[sp,-16]!
  stp x8,x8,[sp,-16]!
  mov x29,sp
  mov x8,x0
  mov x0,x1
  blr x8
  add sp,sp,16
  ldp x29,x30,[sp],16
  ret
_FFI_CALL_TOS_2:
FFI_CALL_TOS_2:
  mov x8,0
  stp x29,x30,[sp,-16]!
  stp x8,x8,[sp,-16]!
  mov x29,sp
  mov x8,x0
  mov x0,x1
  mov x1,x2
  blr x8
  add sp,sp,16
  ldp x29,x30,[sp],16
  ret
_FFI_CALL_TOS_3:
FFI_CALL_TOS_3:
  mov x8,0
  stp x29,x30,[sp,-16]!
  stp x8,x8,[sp,-16]!
  mov x29,sp
  mov x8,x0
  mov x0,x1
  mov x1,x2
  mov x2,x3
  blr x8
  add sp,sp,16
  ldp x29,x30,[sp],16
  ret
_FFI_CALL_TOS_4:
FFI_CALL_TOS_4:
  mov x8,0
  stp x29,x30,[sp,-16]!
  stp x8,x8,[sp,-16]!
  mov x29,sp
  mov x8,x0
  mov x0,x1
  mov x1,x2
  mov x2,x3
  mov x3,x4
  blr x8
  add sp,sp,16
  ldp x29,x30,[sp],16
  ret
