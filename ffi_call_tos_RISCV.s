.text
.global FFI_CALL_TOS_0
.global FFI_CALL_TOS_1
.global FFI_CALL_TOS_2
.global FFI_CALL_TOS_3
.global FFI_CALL_TOS_4

FFI_CALL_TOS_0:
jr a0

FFI_CALL_TOS_1:
addi sp,sp,-8
sd a1,(sp)
jr a0

FFI_CALL_TOS_2:
addi sp,sp,-16
sd a2,8(sp)
sd a1,(sp)
jr a0

FFI_CALL_TOS_3:
addi sp,sp,-24
sd a3,16(sp)
sd a2,8(sp)
sd a1,(sp)
jr a0

FFI_CALL_TOS_4:
addi sp,sp,-32
sd a4,24(sp)
sd a3,16(sp)
sd a2,8(sp)
sd a1,(sp)
jr a0
