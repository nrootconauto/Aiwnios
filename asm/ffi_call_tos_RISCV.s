.text
.global FFI_CALL_TOS_0
.global FFI_CALL_TOS_1
.global FFI_CALL_TOS_2
.global FFI_CALL_TOS_3
.global FFI_CALL_TOS_4
.global FFI_CALL_TOS_CUSTOM_BP
FFI_CALL_TOS_CUSTOM_BP:
addi t0,a0,0
addi sp,sp,-32
sd s0,16(sp)
sd ra,8(sp)
addi s0,a0,0
addi a0,a1,0
jalr t0
ld ra,8(sp)
ld s0,16(sp)
addi sp,sp,32
ret

FFI_CALL_TOS_0:
jr a0

FFI_CALL_TOS_1:
addi t0,a0,0
mv a0,a1
jr t0

FFI_CALL_TOS_2:
addi t0,a0,0
mv a0,a1
mv a1,a2
jr t0

FFI_CALL_TOS_3:
addi t0,a0,0
mv a0,a1
mv a1,a2
jr t0

FFI_CALL_TOS_4:
addi t0,a0,0
mv a0,a1
mv a1,a2
mv a2,a3
mv a3,a4
jr t0
