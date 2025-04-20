.text	
#ifdef __APPLE__ // https://ariadne.space/2023/04/12/writing-portable-arm-assembly.html
#define PROC(p) _##p
#else
#define PROC(p) p
#endif
.global PROC(LBts)
.global PROC(LBtr)
.global PROC(LBtc)
.global PROC(Bt)
.global PROC(Btr)
.global PROC(Btc)
.global PROC(Bts)
.global PROC(Misc_Caller)
.global PROC(Misc_BP)
PROC(Misc_BP):
  mov x0,x29
  ret
PROC(Bt):
  lsr x3,x1,3
  and x1,x1,0x7
  mov x2,1
  lslv x1,x2,x1
  add x0,x3,x0
  ldxrb w3,[x0]
  tst w3,w1
  cset x0, ne
  ret
PROC(LBts):
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
PROC(Btc):
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
PROC(Bts):
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
PROC(Btr):
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

PROC(LBtc):
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

PROC(LBtr):
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
PROC(Misc_Caller):
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
