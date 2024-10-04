.text
.global TempleOS_CallN
.global TempleOS_Call
.global TempleOS_CallVaArgs
.global _TempleOS_CallVaArgs
.global _TempleOS_Call
_TempleOS_Call:
TempleOS_Call:
  stp x29,x30,[sp,-16]!
  
  mov x29,sp
  blr x0
  mov sp,x29
  ldp x29,x30,[sp],16
  ret

#For MacOS
.global _TempleOS_CallN
_TempleOS_CallN:
TempleOS_CallN:
  stp x29,x30,[sp,-16]!
  mov x29,sp
  mov x8,x1
  mov x9,x0
  and x3,x1,1  /* %2*/
  add x1,x1,x3 /* If remainder of x1 is 1,add 1 to align to 2(we multiply by 8 to align to 16) */
  lsl x5,x1,3 /* times 8 */
  sub sp,sp,x5
  cbz x1,.L_bye
.L_loop:
  sub x1,x1,1
  ldr x5,[x2,x1,LSL 3]
  str x5,[sp,x1,LSL 3]
  cbnz x1,.L_loop
.L_bye:
  
  ldr x7,[sp,7*8]
  ldr x6,[sp,6*8]
  ldr x5,[sp,5*8]
  ldr x4,[sp,4*8]
  ldr x3,[sp,3*8]
  ldr x2,[sp,2*8]	
  ldr x1,[sp,1*8]
  ldr x0,[sp,0*8]
  cmp x8,8
  bls .L_no_sub
  sub sp,sp,8*8
.L_no_sub:
  blr x9
  mov sp,x29
  ldp x29,x30,[sp],16
  ret

__MemCpy:
  mov x12,0
.Lms:
  cmp x11,x12
  bne .Lms2
  ret
.Lms2:
  ldr x13,[x10,x12,LSL 3]
  str x13,[x9,x12,LSL 3]
  add x12,x12,1
  b .Lms

TempleOS_CallVaArgs:
_TempleOS_CallVaArgs:
  stp x29,x30,[sp,-16]!
  mov x29,sp
 
  mov x6,x3
  mov x7,x4

  sub sp,sp,128

  add x4,x1,x6/*argc1+argc*/
  add x4,x4,2 //Arm passes argv like a normal vairable,may change in future,so +1 for argc,+1 for argc
  and x5,x4,1  /* %1*/
  add x4,x4,x5
  lsl x5,x4,3
  sub sp,sp,x5

  mov x9,sp
  mov x10,x7
  mov x11,x6
  bl __MemCpy

  // write into argc
  str x1,[sp,x6,LSL 3]
  //write into argv
  add x6,x6,1
  lsl x13,x6,3
  add x13,sp,x13
  add x13,x13,8
  str x13,[sp,x6,LSL 3]
  //make "room"
  mov x9,x13
  mov x10,x2
  mov x11,x1
  bl __MemCpy


  mov x8,x0

  cmp x6,8 
  add x6,x6,2 /*argc,argv*/
  cmp x6,8
  bge .L_stk
    
  ldr x7,[sp,7*8]
  ldr x6,[sp,6*8]
  ldr x5,[sp,5*8]
  ldr x4,[sp,4*8]
  ldr x3,[sp,3*8]
  ldr x2,[sp,2*8]	
  ldr x1,[sp,1*8]
  ldr x0,[sp,0*8]

  blr x8
  b .L_fin

.L_stk:
  ldr x7,[sp,7*8]
  ldr x6,[sp,6*8]
  ldr x5,[sp,5*8]
  ldr x4,[sp,4*8]
  ldr x3,[sp,3*8]
  ldr x2,[sp,2*8]
  ldr x1,[sp,1*8]
  ldr x0,[sp,0*8]
  blr x8
  add sp,sp,8*8  
.L_fin:
  mov sp,x29
  ldp x29,x30,[sp],16
  ret
