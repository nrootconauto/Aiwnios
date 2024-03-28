.text
.global TempleOS_Call
.global TempleOS_CallN
.global TempleOS_CallVaArgs
TempleOS_Call:
  ldr x0,[sp]
  stp x29,x30,[sp,-16]!
  mov x29,sp
  blr x0
  mov sp,x29
  ldp x29,x30,[sp],16
  ret
TempleOS_CallN:
  ldr x0,[sp]
  ldr x1,[sp,8]
  ldr x2,[sp,16]
  stp x29,x30,[sp,-16]!
  mov x29,sp
  and x3,x1,1  /* %1*/
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
  blr x0
  mov sp,x29
  ldp x29,x30,[sp],16
  ret
TempleOS_CallVaArgs:
  ldr x0,[sp]
  ldr x1,[sp,8]
  ldr x2,[sp,16]
  ldr x6,[sp,24]
  sub x7,sp,32
  stp x29,x30,[sp,-16]!
  mov x29,sp

  add x4,x1,x6
  add x4,1
  and x4,x1,1  /* %1*/
  add x1,x1,x4 /* If remainder of x1 is 1,add 1 to align to 2(we multiply by 8 to align to 16) */
  lsl x5,x1,3 /* times 8 */
  sub sp,sp,x5
  cbz x5,.L1e
.L1s:
  sub x5,x5,8
  ldr x4,[x2,x5]
  str x4,[sp,x5]
  cbnz x5,.L1s
.L1e:

  ldr x1,[x29,16]
  stp x1,[sp,-8]!

  lsl x5,x6,3 /* times 8 */
  sub sp,sp,x5
  cbz x5,.L2e
.L2s:
  sub x5,x5,8
  ldr x4,[x7,x5]
  str x4,[sp,x5]
  cbnz x5,.L2s
.L2e:

  blr x0
  mov sp,x29
  ldp x29,x30,[sp],16
  ret
