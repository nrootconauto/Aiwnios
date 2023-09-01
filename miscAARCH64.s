.text
.global Misc_LBts
.global Misc_LBtr
.global Misc_LBtc
.global Misc_Bt
.global Misc_Btr
.global Misc_Btc
.global Misc_Bts
.global Misc_Caller
Misc_Bt:
  lsr x3,x1,3
  and x1,x1,0x7
  mov x2,1
  lslv x1,x2,x1
  add x0,x3,x0
  ldxrb w3,[x0]
  tst w3,w1
  cset x0, ne
  ret

Misc_LBts:
  lsr x3,x1,3
  and x1,x1,0x7
  add x0,x3,x0
.L_Bts_0:
  ldaxrb w3,[x0]
  mov x4,1
  lslv w4,w4,w1
  tst w3,w4
  cset x5, ne
  orr w3,w3,w4
  stlxrb w4,w3,[x0]
  cbnz w4,.L_Bts_0
  mov x0,x5
  ret

Misc_Btc:
  lsr x3,x1,3
  and x1,x1,0x7
  add x0,x3,x0
  ldrb w3,[x0]
  mov x4,1
  lslv w4,w4,w1
  tst w3,w4
  cset x5, ne
  eor w3,w3,w4
  strb w3,[x0]
  mov x0,x5
  ret

Misc_Bts:
  lsr x3,x1,3
  and x1,x1,0x7
  add x0,x3,x0
  ldrb w3,[x0]
  mov x4,1
  lslv w4,w4,w1
  tst w3,w4
  cset x5, ne
  orr w3,w3,w4
  strb w3,[x0]
  mov x0,x5
  ret

Misc_Btr:
  lsr x3,x1,3
  and x1,x1,0x7
  add x0,x3,x0
  ldrb w3,[x0]
  mov x4,1
  lslv w4,w4,w1
  tst w3,w4
  cset x5, ne
  mvn w4,w4
  and w3,w3,w4
  strb w3,[x0]
  mov x0,x5
  ret


Misc_LBtc:
  lsr x3,x1,3
  and x1,x1,0x7
  add x0,x3,x0
.L_Btc_0:
  ldaxrb w3,[x0]
  mov x4,1
  lslv w4,w4,w1
  tst w3,w4
  cset x5, ne
  eor w3,w3,w4
  stlxrb w4,w3,[x0]
  cbnz w4,.L_Btc_0
  mov x0,x5
  ret


Misc_LBtr:
  lsr x3,x1,3
  and x1,x1,0x7
  add x0,x3,x0
.L_Btr_0:
  ldaxrb w3,[x0]
  mov w4,1
  lslv w4,w4,w1
  tst w3,w4
  cset x5, ne
  mvn w4,w4
  and w3,w3,w4
  stlxrb w4,w3,[x0]
  cbnz w4,.L_Btr_0
  mov x0,x5
  ret
Misc_Caller:
  add x1,x0,1
  mov x2,x29
.L_Caller_0:
  cbz x2,.L_Caller_fin
  ldr x0,[x2,8]
  ldr x2,[x2]
  sub x1,x1,1
  cbnz x1,.L_Caller_0
.L_Caller_fin:
  ret
