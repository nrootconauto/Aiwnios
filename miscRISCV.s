.text
.global TempleOS_CallN
.global Misc_Btc
.global Misc_LBtc
.global Misc_Caller
.global Misc_Bt
.global Misc_Bts
.global Misc_Btr
.global Misc_LBtr
.global Misc_LBts
.global Misc_BP
# I dont know the psudeo ops,sorry profressionals
Misc_Btc:
  srli t0,a1,3 # divide by 8 bits
  add t0,t0,a0
  andi t1,a1,0x7
  li t2,1
  sll t1,t2,t1
  lb t2,(t0)
  and a0,t2,t1
  xor t2,t2,t1
  sb t2,(t0)
  snez a0,a0
  jalr zero,ra

Misc_LBtc:
#move the pointer down to align to 4 bytes,or else BUS ERROR
  andi t0,a0,3 #off=%4
  andi a0,a0,~3 #align a0 to 4
  slli t0,t0,3 #off=off*8
  add a1,t0,a1 #bit+=off

  srli t0,a1,5 # divide by 32 bits
  slli t0,t0,2 #*4(bytes)
  
  add t0,t0,a0
  andi t1,a1,0x1f
  addi t2,zero,1
  sll t1,t2,t1
  amoxor.w t2,t1,(t0)
  and a0,t2,t1
  snez a0,a0
  jalr zero,ra

Misc_Caller:
  addi sp,sp,-16
  sd fp,(sp)
  sd ra,8(sp)
  addi fp,sp,0
  addi t1,a0,0
  addi a0,zero,0
  add t2,fp,zero
.L_loop:
  beq t2,zero,.L_fail
  ld a0,8(t2)
  bne zero,a1,.L_fin2
  addi a1,a1,-1
  ld t2,(t2)
  j .L_loop
.L_fin2:
  ld fp,(sp)
  addi sp,sp,16
  jalr zero,ra
.L_fail:
  li a0,0
  j .L_fin2
  

Misc_Bt:
#move the pointer down to align to 4 bytes,or else BUS ERROR
  andi t0,a0,3 #off=%4
  andi a0,a0,~3 #align a0 to 4
  slli t0,t0,3 #off=off*8
  add a1,t0,a1 #bit+=off

  srli t0,a1,5 # divide by 32 bits
  slli t0,t0,2 #*4(bytes)
  
  add t0,t0,a0
  andi t1,a1,0x1f
  li t2,1
  sll t1,t2,t1
  lr.w t2,(t0)
  and a0,t2,t1
  snez a0,a0
  jalr zero,ra

Misc_Bts:
  srli t0,a1,3 # divide by 8 bits
  add t0,t0,a0
  andi t1,a1,0x7
  li t2,1
  sll t1,t2,t1
  lb t2,(t0)
  and a0,t2,t1
  or t2,t2,t1
  sb t2,(t0)
  snez a0,a0
  jalr zero,ra

Misc_LBts:
#move the pointer down to align to 4 bytes,or else BUS ERROR
  andi t0,a0,3 #off=%4
  andi a0,a0,~3 #align a0 to 4
  slli t0,t0,3 #off=off*8
  add a1,t0,a1 #bit+=off

  srli t0,a1,5 # divide by 32 bits
  slli t0,t0,2 #*4(bytes)
  
  add t0,t0,a0
  andi t1,a1,0x1f
  li t2,1
  sll t1,t2,t1
  amoor.w t2,t1,(t0)
  and a0,t2,t1
  snez a0,a0
  jalr zero,ra

Misc_Btr:
  srli t0,a1,3 # divide by 8 bits
  add t0,t0,a0
  andi t1,a1,0x7
  li t2,1
  sll t1,t2,t1
  lb t2,(t0)
  and a0,t2,t1
  not t4,t1
  and t3,t2,t4
  sb t2,(t0)
  snez a0,a0
  jalr zero,ra

Misc_LBtr:
#move the pointer down to align to 4 bytes,or else BUS ERROR
  andi t0,a0,3 #off=%4
  andi a0,a0,~3 #align a0 to 4
  slli t0,t0,3 #off=off*8
  add a1,t0,a1 #bit+=off

  srli t0,a1,5 # divide by 32 bits
  slli t0,t0,2 #*4(bytes)
  
  add t0,t0,a0
  andi t1,a1,0x1f
  li t2,1
  sll t1,t2,t1
  not t4,t1
  amoand.w t2,t4,(t0)
  and a0,t2,t1
  snez a0,a0
  jalr zero,ra

TempleOS_CallN:
  ret

Misc_BP:
  mv a0,s0
  ret
