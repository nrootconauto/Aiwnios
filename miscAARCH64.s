.text	
.global Misc_LBts
.global Misc_LBtr
.global Misc_LBtc
.global Misc_Bt
.global Misc_Btr
.global Misc_Btc
.global Misc_Bts
.global Misc_Caller
.global Misc_BP
# For MacOS
.global _Misc_LBts
.global _Misc_LBtr
.global _Misc_LBtc
.global _Misc_Bt
.global _Misc_Btr
.global _Misc_Btc
.global _Misc_Bts
.global _Misc_Caller
.global _Misc_BP
_Misc_BP:
Misc_BP:
  mov x0,x29
  ret
_Misc_Bt:
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
_Misc_LBts:
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
_Misc_Btc:
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
_Misc_Bts:
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
_Misc_Btr:
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

_Misc_LBtc:
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

_Misc_LBtr:
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
_Misc_Caller:
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
