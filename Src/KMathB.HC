F64 Clamp(F64 d,F64 lo,F64 hi)
{//Clamp to F64 [] range.
  if (d<lo)
    return lo;
  if (d>hi)
    return hi;
  return d;
}

U64 ClampU64(U64 d,U64 lo,U64 hi)
{//Clamp to U64 [] range.
  if (d<lo)
    return lo;
  if (d>hi)
    return hi;
  return d;
}

F64 Min(F64 n1,F64 n2)
{//Min of two F64s.
  if (n1<=n2)
    return n1;
  else
    return n2;
}

F64 Max(F64 n1,F64 n2)
{//Max of two F64s.
  if (n1>=n2)
    return n1;
  else
    return n2;
}

/*F64 Pow10I64(I64 i) RESTORE?
{//F64 int powers of ten.
  if (i>308)
    return inf;
  else if (i<-308)
    return 0.0;
  else
    return pow10_I64[i+309];
}*/

U64 FloorU64(U64 num,U64 to)
{//Int multiples of num.
  return num-num%to;
}

U64 CeilU64(U64 num,U64 to)
{//Int multiples of num.
  num+=to-1;
  return num-num%to;
}

I64 RoundI64(I64 num,I64 to)
{//Int multiples of num.
  return num-num%to;
}

I64 FloorI64(I64 num,I64 to)
{//Int multiples of num.
  if (num>=0)
    return num-num%to;
  else {
    num++;
    return num-num%to-to;
  }
}

I64 CeilI64(I64 num,I64 to)
{//Int multiples of num.
  if (num>=0) {
    num+=to-1;
    return num-num%to;
  } else {
    num+=to-1;
    return num-num%to-to;
  }
}

//See $LK,"::/Doc/Credits.DD"$.
#define LIN_CONGRUE_A	6364136223846793005
#define LIN_CONGRUE_C	1442695040888963407

I16 RandI16()
{//Random I16.
  I64 res=Fs->rand_seed;
  res=LIN_CONGRUE_A*res^(res&0xFFFFFFFF0000)>>16+LIN_CONGRUE_C;
//  if (!Bt(&(Fs->task_flags),TASKf_NONTIMER_RAND))
//      res^=__GetTicks;
  Fs->rand_seed=res;
  return res.i16[0];
}

U16 RandU16()
{//Random U16.
  I64 res=Fs->rand_seed;
  res=LIN_CONGRUE_A*res^(res&0xFFFFFFFF0000)>>16+LIN_CONGRUE_C;
  /*if (!Bt(&(Fs->task_flags),TASKf_NONTIMER_RAND)) //Nroot here,Why does this make our code super slow
    res^=__GetTicks;*/
  Fs->rand_seed=res;
  return res&0xffff;
}

I32 RandI32()
{//Random I32.
  I64 res=Fs->rand_seed;
  res=LIN_CONGRUE_A*res^(res&0xFFFFFFFF0000)>>16+LIN_CONGRUE_C;
//  if (!Bt(&(Fs->task_flags),TASKf_NONTIMER_RAND))
//    res^=__GetTicks;
  Fs->rand_seed=res;
  return res.i32[0];
}

U32 RandU32()
{//Random U32.
  I64 res=Fs->rand_seed;
  res=LIN_CONGRUE_A*res^(res&0xFFFFFFFF0000)>>16+LIN_CONGRUE_C;
//  if (!Bt(&(Fs->task_flags),TASKf_NONTIMER_RAND))
//    res^=__GetTicks;
  Fs->rand_seed=res;
  return res&0xffffFFFF;
}

I64 RandI64()
{//Random I64.
  I64 res=Fs->rand_seed;
  res=LIN_CONGRUE_A*res^(res&0xFFFFFFFF0000)>>16+LIN_CONGRUE_C;
//  if (!Bt(&(Fs->task_flags),TASKf_NONTIMER_RAND))
//    res^=__GetTicks;
  Fs->rand_seed=res;
  return res;
}

U64 RandU64()
{//Random U64.
  I64 res=Fs->rand_seed;
  res=LIN_CONGRUE_A*res^(res&0xFFFFFFFF0000)>>16+LIN_CONGRUE_C;
//  if (!Bt(&(Fs->task_flags),TASKf_NONTIMER_RAND))
//    res^=__GetTicks;
  Fs->rand_seed=res;
  return res;
}

F64 Rand()
{//Random F64.
  I64 res=Fs->rand_seed;
  res=LIN_CONGRUE_A*res^(res&0xFFFFFFFF0000)>>16+LIN_CONGRUE_C;
//  if (!Bt(&(Fs->task_flags),TASKf_NONTIMER_RAND))
//    res^=__GetTicks;
  Fs->rand_seed=res;
  return (res&0x3FFFFFFFFFFFFFFF)/ToF64(0x4000000000000000);
}

I64 Seed(I64 seed=0,CTask *task=NULL)
{//Set $LK,"Rand",A="MN:Rand"$() seed. Zero for timer-based.
  if (!task) task=Fs;
  if (seed) {
    LBts(&task->task_flags,TASKf_NONTIMER_RAND);
    return task->rand_seed=seed;
  } else {
    LBtr(&task->task_flags,TASKf_NONTIMER_RAND);
    return task->rand_seed^=cnts.jiffies;
  }
}


I64 AbsI64(I64 i) {
  if(i<0)
    return -i;
  return i;
}
