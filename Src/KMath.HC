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

#ifdef BOOTSTRAP
F64 ToF64(I64 i) {
  return i;
}

I64 ToI64(F64 i) {
  return i;
}
#endif

I64 ClampI64(I64 i,I64 lo,I64 hi) {
  if(i<lo) return lo;
  if(i>hi) return hi;
  return i;
  
}
#ifdef BOOTSTRAP
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
U64 MinU64(U64 a,U64 b) {
  if(a<b)
    return a;
  return b;
}
U64 MaxU64(U64 a,U64 b) {
  if(a>b)
    return a;
  return b;
}
#else
//Both X86/AARCH64 have these as interns
#endif

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
    return -1;
}

F64 Sign(F64 a) {
  if(!a) return a;
  if(a>0)
    return  1;
  else
    return -1;
}

F64 Sqr(F64 a) {
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

F64 D3Norm(CD3 *norm) {
  F64 x=norm->x,y=norm->y,z=norm->z;
  return Sqrt((x*x)+(y*y)+(z*z));
}
F64 D3NormSqr(CD3 *norm) {
  F64 r=D3Norm(norm);
  return r*r;
}
CD3 *D3Zero(CD3 *z) {
  z->x=0,z->y=0,z->z=0;
  return z;	
}
CD3 *D3MulEqu(CD3 *z,F64 s) {
  z->x*=s,z->y*=s,z->z*=s;
  return z;	
}
CD3 *D3Sub(CD3 *a,CD3 *b,CD3 *c) {
  a->x=b->x-c->x;
  a->y=b->y-c->y;
  a->z=b->z-c->z; 
  return a;
}
CD3 *D3Equ(CD3 *a,F64 x=0.,F64 y=0.,F64 z=0.) {
  a->x=x,a->y=y,a->z=z;
  return a;
} 
CD3 *D3Mul(CD3 *a,F64 s,CD3 *c) {
  a->x=s*c->x;
  a->y=s*c->y;
  a->z=s*c->z; 
  return a;
}
CD3 *D3SubEqu(CD3 *a,CD3 *b) {
  return D3Sub(a,a,b);
}
CD3 *D3Div(CD3 *a,CD3 *c,F64 s) {
  a->x=c->x/s;
  a->y=c->y/s;
  a->z=c->z/s; 
  return a;
}
CD3 *D3DivEqu(CD3 *a,F64 s) {
  return D3Div(a,a,s);
}
F64 D3Dist(CD3 *a,CD3 *b) {
  F64 x=a->x-b->x;
  F64 y=a->y-b->y;
  F64 z=a->z-b->z;
  return Sqrt(x*x+y*y+z*z);
}
F64 D3DistSqr(CD3 *a,CD3 *b) {
  F64 x=a->x-b->x;
  F64 y=a->y-b->y;
  F64 z=a->z-b->z;
  return x*x+y*y+z*z;
}
CD3 *D3Add(CD3 *a,CD3 *b,CD3 *c) {
  a->x=b->x+c->x;
  a->y=b->y+c->y;
  a->z=b->z+c->z; 
  return a;
}
CD3 *D3AddEqu(CD3 *a,CD3 *b) {
  return D3Add(a,a,b);
}
CD3 *D3Copy(CD3 *a,CD3 *b) {
  a->x=b->x,a->y=b->y,a->z=b->z;
  return a;
}
CD3 *D3Cross(CD3 *cross,CD3 *a,CD3 *b) {
  cross->x=a->y*b->z-a->z*b->y;
  cross->y=a->z*b->x-a->x*b->z;
  cross->z=a->x*b->y-a->y*b->x;
  return cross;
}
F64 D3Dot(CD3 *a,CD3 *b) {
  return a->x*b->x+a->y*b->y+a->z+b->z;
}
F64 D3Unit(CD3 *unit) {
  F64 dist=D3Norm(unit);
  if(dist) {
    unit->x/=dist;
    unit->y/=dist;
    unit->z/=dist;
  }
  return unit;
}
