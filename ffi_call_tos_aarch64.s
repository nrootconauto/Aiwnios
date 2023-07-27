.text
.global FFI_CALL_TOS_0
.global FFI_CALL_TOS_1
.global FFI_CALL_TOS_2
.global FFI_CALL_TOS_3
.global FFI_CALL_TOS_4
FFI_CALL_TOS_0:
  stp x29,x30,[sp,-16]!
  blr x0
  ldp x29,x30,[sp],16
  ret
FFI_CALL_TOS_1:
  stp x29,x30,[sp,-16]!
  str x1,[sp,-16]
  blr x0
  add sp,sp,16
  ldp x29,x30,[sp],16
  ret
FFI_CALL_TOS_2:
  stp x29,x30,[sp,-16]!
  stp x1,x2,[sp,-16]!
  blr x0
  add sp,sp,16
  ldp x29,x30,[sp],16
  ret
FFI_CALL_TOS_3:
  stp x29,x30,[sp,-16]!
  str x3,[sp,-16]!
  stp x1,x2,[sp,-16]!
  blr x0
  add sp,sp,32
  ldp x29,x30,[sp],16
  ret
FFI_CALL_TOS_4:
  stp x29,x30,[sp,-16]!
  stp x3,x4,[sp,-16]!
  stp x1,x2,[sp,-16]!
  blr x0
  add sp,sp,32
  ldp x29,x30,[sp],16
  ret
