.global TempleOS_CallN
TempleOS_CallN:
  ldr x0,[sp]
  ldr x1,[sp,8]
  ldr x2,[sp,16]
  stp x29,x30,[sp,-16]!
  mov x29,sp
  mov x4,x1
  and x3,x1,1  /* %1*/
  add x1,x1,x3 /* If remainder of x1 is 1,add 1 to align to 2(we multiply by 8 to align to 16) */
  lsl x5,x1,3 /* times 8 */
  sub sp,sp,x5
  cbz x1,bye
loop:
  sub x1,x1,1
  ldr x5,[x2,x1,LSL 3]
  str x5,[sp,x1,LSL 3]
  cbnz x1,loop
bye:
  blr x0
  mov sp,x29
  ldp x29,x30,[sp],16
  ret
