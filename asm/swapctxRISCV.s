.text
.global AIWNIOS_getcontext
.global AIWNIOS_setcontext
.global AIWNIOS_makecontext
AIWNIOS_getcontext:
  mv t0,a0
  sd ra,(t0)
  sd sp,8(t0)
  sd fp,16(t0)
  sd s1,24(t0) #s0  is frame ptr
  sd s2,32(t0)
  sd s3,40(t0)
  sd s4,48(t0)
  sd s5,56(t0)
  sd s6,64(t0)
  sd s7,72(t0)
  sd s8,80(t0)
  sd s9,88(t0)
  sd s10,96(t0)
  sd s11,104(t0)
  fsd fs0,112(t0)
  fsd fs1,120(t0)
  fsd fs2,128(t0)
  fsd fs3,136(t0)
  fsd fs4,144(t0)
  fsd fs5,152(t0)
  fsd fs6,160(t0)
  fsd fs7,168(t0)
  fsd fs8,176(t0)
  fsd fs9,184(t0)
  fsd fs10,192(t0)
  fsd fs11,200(t0)
  li a0,0
  ret
AIWNIOS_setcontext:
  mv t0,a0
  ld ra,(t0)
  ld sp,8(t0)
  ld fp,16(t0)
  ld s1,24(t0) #s0  is frame ptr
  ld s2,32(t0)
  ld s3,40(t0)
  ld s4,48(t0)
  ld s5,56(t0)
  ld s6,64(t0)
  ld s7,72(t0)
  ld s8,80(t0)
  ld s9,88(t0)
  ld s10,96(t0)
  ld s11,104(t0)
  fld fs0,112(t0)
  fld fs1,120(t0)
  fld fs2,128(t0)
  fld fs3,136(t0)
  fld fs4,144(t0)
  fld fs5,152(t0)
  fld fs6,160(t0)
  fld fs7,168(t0)
  fld fs8,176(t0)
  fld fs9,184(t0)
  fld fs10,192(t0)
  fld fs11,200(t0)
  li a0,1
  ret
AIWNIOS_makecontext:
  sd a1,(a0)
  sd a2,8(a0)
  ret
