.text
.global AIWNIOS_getcontext
.global AIWNIOS_setcontext
.global AIWNIOS_makecontext
#For MacOS
.global _AIWNIOS_getcontext
.global _AIWNIOS_setcontext
.global _AIWNIOS_makecontext
_AIWNIOS_makecontext:
AIWNIOS_makecontext:
  str x1,[x0,8*12]
  str x2,[x0,8*21]
  ret
_AIWNIOS_getcontext:
AIWNIOS_getcontext:
  stp x29,x30,[sp,-16]!
  str x30,[sp,8]
#  str x18,[x0]
  str x19,[x0,8]
  str x20,[x0,8*2]
  str x21,[x0,8*3]
  str x22,[x0,8*4]
  str x23,[x0,8*5]
  str x24,[x0,8*6]
  str x25,[x0,8*7]
  str x26,[x0,8*8]
  str x27,[x0,8*9]
#  str x28,[x0,8*10] #Used as TLS register in Aiwnios
  str x29,[x0,8*11]
  str x30,[x0,8*12]
  str d8,[x0,8*13]
  str d9,[x0,8*14]
  str d10,[x0,8*15]
  str d11,[x0,8*16]
  str d12,[x0,8*17]
  str d13,[x0,8*18]
  str d14,[x0,8*19]
  str d15,[x0,8*20]
  add x1,sp,16
  str x1,[x0,8*21]
  ldp x29,x30,[sp],16
  mov x0,0
  ret
_AIWNIOS_setcontext:
AIWNIOS_setcontext:
  mov x8,x0
#  ldr x18,[x0]
  ldr x19,[x0,8]
  ldr x20,[x0,8*2]
  ldr x21,[x0,8*3]
  ldr x22,[x0,8*4]
  ldr x23,[x0,8*5]
  ldr x24,[x0,8*6]
  ldr x25,[x0,8*7]
  ldr x26,[x0,8*8]
  ldr x27,[x0,8*9]
#  ldr x28,[x0,8*10] #Used As TLS register in aiwnios
  ldr x29,[x0,8*11]
  ldr x30,[x0,8*12]
  ldr d8,[x0,8*13]
  ldr d9,[x0,8*14]
  ldr d10,[x0,8*15]
  ldr d11,[x0,8*16]
  ldr d12,[x0,8*17]
  ldr d13,[x0,8*18]
  ldr d14,[x0,8*19]
  ldr d15,[x0,8*20]
  ldr x1,[x0,8*21]
  mov sp,x1
  mov x0,1
  ret 
