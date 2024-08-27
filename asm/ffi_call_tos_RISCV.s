.text
.global FFI_CALL_TOS_0
.global FFI_CALL_TOS_1
.global FFI_CALL_TOS_2
.global FFI_CALL_TOS_3
.global FFI_CALL_TOS_4
.global FFI_CALL_TOS_CUSTOM_BP
FFI_CALL_TOS_CUSTOM_BP:
addi sp,sp,-32
sd s0,24(sp)
sd ra,16(sp)
sd sp,8(sp)
sd a2,0(sp) #"POP"ed off
addi s0,a0,0
jalr a1
ld ra,8(sp)
ld s0,16(sp)
addi sp,sp,24
ret

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
