.global LBts
.global LBtr
.global LBtc
.global Bt
.global Btr
.global Btc
.global Bts
.global Caller
Bt:
  lsr x3,x1,3
  and x1,x1,0x7
  mov x2,1
  lslv x1,x2,x1
  add x0,x3,x0
_Bt_0:
  ldxrb w3,[x0]
  tst w3,w1
  cset x0, ne
  ret

LBts:
  lsr x3,x1,3
  and x1,x1,0x7
  add x0,x3,x0
_Bts_0:
  ldxrb w3,[x0]
  mov x4,1
  lslv w4,w4,w1
  tst w3,w4
  cset x5, ne
  orr w3,w3,w4
  stxrb w4,w3,[x0]
  cbnz w4,_Bts_0
  mov x0,x5
  ret

Btc:
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

Bts:
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

Btr:
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


LBtc:
  lsr x3,x1,3
  and x1,x1,0x7
  add x0,x3,x0
_Btc_0:
  ldxrb w3,[x0]
  mov x4,1
  lslv w4,w4,w1
  tst w3,w4
  cset x5, ne
  eor w3,w3,w4
  stxrb w4,w3,[x0]
  cbnz w4,_Bts_0
  mov x0,x5
  ret


LBtr:
  lsr x3,x1,3
  and x1,x1,0x7
  add x0,x3,x0
_Btr_0:
  ldxrb w3,[x0]
  mov w4,1
  lslv w4,w4,w1
  tst w3,w4
  cset x5, ne
  mvn w4,w4
  and w3,w3,w4
  stxrb w4,w3,[x0]
  cbnz w4,_Btr_0
  mov x0,x5
  ret
Caller:
  add x1,x0,1
  mov x2,x29
_Caller_0:
  cbz x2,_Caller_fin
  ldr x0,[x2,8]
  ldr x2,[x2]
  sub x1,x1,1
  cbnz x1,_Caller_0
_Caller_fin:
  ret
