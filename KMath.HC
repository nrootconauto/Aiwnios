F64 Pow10I64(I64 i)
{//F64 int powers of ten.
  return Pow10(i);
}

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

F64 ToF64(I64 i) {
  return i;
}

I64 ToI64(F64 i) {
  return i;
}

I64 ClampI64(I64 i,I64 lo,I64 hi) {
  if(i<lo) return lo;
  if(i>hi) return hi;
  return i;
  
}
I64 MinI64(I64 a,I64 b) {
  if(a<b)
    return a;
  return b;
}
I64 MaxI64(I64 a,I64 b) {
  if(a>b)
    return a;
  return b;
}

I64 SqrI64(I64 i) {
  return i*i;
}

I64 DistSqrI64(I64 x1,I64 y1,I64 x2,I64 y2)
{//Distance-squared between 2 points.
  I64 dx=x1-x2,dy=y1-y2;
  return dx*dx+dy*dy;
}

public U0 R2P(F64 *_mag=NULL,F64 *_arg=NULL,F64 x,F64 y)
{//Rect to polar
//Returns angle in range (-ã,ã]
  if (_arg)
    *_arg=Arg(x,y);
  if (_mag)
    *_mag=Sqrt(x*x+y*y);
}

public U0 P2R(F64 *_x=NULL,F64 *_y=NULL,F64 mag,F64 arg)
{//Polar to Rect
  if (_x)
    *_x=mag*Cos(arg);
  if (_y)
    *_y=mag*Sin(arg);
}

F64 Abs(F64 a) {
  if(a<0.)
    return -a;
  return a;
}


F64 Trunc(F64 a) {
  I64 b=a;
  return b;
} 

I64 SignI64(I64 a) {
  if(!a) return a;
  if(a>0)
    return  1;
  else
    return 0;
}

F64 Sign(F64 a) {
  if(!a) return a;
  if(a>0)
    return  1;
  else
    return 0;
}

F64 Sqr(F64 a) {
  return a*a;
}

F64 Wrap(F64 a,F64 base=-pi)
{//Returns angle in range [base,base+2*ã)
  F64 res=a%(2*pi);
  if (res>=base+2*pi)
    res-=2*pi;
  else if (res<base)
    res+=2*pi;
  return res;
}
