.text
.global TempleOS_CallN
.global TempleOS_Call
.global Misc_Btc
.global Misc_LBtc
.global Misc_Caller
.global Misc_Bt
.global Misc_Bts
.global Misc_Btr
.global Misc_LBtr
.global Misc_LBts
.global Misc_BP
.global TempleOS_CallVaArgs
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
  addi fp,sp,16
  addi t1,a0,0
  addi a0,zero,0
  add t2,fp,zero
.L_loop:
  beq t2,zero,.L_fail
  ld a0,-8(t2)
  beq zero,t1,.L_fin2
.L_fin:
  addi t1,t1,-1
  ld t2,-16(t2)
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
  and t2,t2,t4
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

TempleOS_Call:
  ret

TempleOS_CallN:
  ret
# I64 CallVaArgs(fptr,argc1,argv1,...)
# fptr is a variadic fun with argc normal args
# argc1/argv1 are propogated for fptr's varargs
TempleOS_CallVaArgs:
  addi sp,sp,-16
  sd fp,(sp)
  sd ra,8(sp)
  addi fp,sp,16
  addi a4,sp,16 # point to argv
  addi t5,a3,1 #t5 has args + (1 argc)
  
  # sz=argc1+argc+1 
  addi t1,a1,1
  add t1,t1,a3
    
  # *=8
  slli t1,t1,3
  
  # Make room
  sub sp,sp,t1
  
  add t4,t1,sp
  
  slli a3,a3,3
.Lregular_start:
  beq zero,a3,.Lregular_end
  addi a3,a3,-8
  add t1,sp,a3
  add t2,a4,a3
  ld t2,(t2)
  sd t2,(t1)
  j .Lregular_start
.Lregular_end:
  
  add t1,a1,zero
  slli a1,a1,3
  addi t4,t4,-8
.Lvarg_start:
  beq zero,a1,.Lvarg_end
  addi a1,a1,-8
  add t2,a2,a1
  ld t2,(t2)
  sd t2,(t4)
  addi t4,t4,-8
  j .Lvarg_start
.Lvarg_end:
  sd t1,(t4)
  add t1,a0,zero
  addi t2,zero,8
  bge t5,t2, .Lbig 
  slli t5,t5,3
  ld a7,7*8(sp)
  ld a6,6*8(sp)
  ld a5,5*8(sp)
  ld a4,4*8(sp)
  ld a3,3*8(sp)
  ld a2,2*8(sp)
  ld a1,1*8(sp)
  ld a0,0*8(sp)
  add sp,sp,t5
.Lend:
  jalr t1
  addi sp,fp,-16
  ld fp,(sp)
  ld ra,8(sp)
  addi sp,sp,16
  ret
.Lbig:
  ld a7,7*8(sp)
  ld a6,6*8(sp)
  ld a5,5*8(sp)
  ld a4,4*8(sp)
  ld a3,3*8(sp)
  ld a2,2*8(sp)
  ld a1,1*8(sp)
  ld a0,0*8(sp)
  addi sp,sp,-8*8
   j .Lend
Misc_BP:
  mv a0,s0
  ret
