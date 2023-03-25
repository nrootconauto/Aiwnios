public F64 Sinh(F64 x)
{//Hyperbolic Sine.
  return 0.5*(Exp(x)-Exp(-x));
}

public F64 Cosh(F64 x)
{//Hyperbolic Cosine.
  return 0.5*(Exp(x)+Exp(-x));
}

#help_index "Math/Complex;Data Types/Complex"
public Complex *CAdd(Complex *sum,Complex *n1,Complex *n2)
{//sum=n1+n2
  sum->x=n1->x+n2->x;
  sum->y=n1->y+n2->y;
  return sum;
}

public Complex *CSub(Complex *diff,Complex *n1,Complex *n2)
{//diff=n1-n2
  diff->x=n1->x-n2->x;
  diff->y=n1->y-n2->y;
  return diff;
}

public Complex *CMul(Complex *prod,Complex *n1,Complex *n2)
{//prod=n1*n2
  prod->x=n1->x*n2->x-n1->y*n2->y;
  prod->y=n1->x*n2->y+n1->y*n2->x;
  return prod;
}

public Complex *CDiv(Complex *quot,Complex *n1,Complex *n2)
{//quot=n1/n2
  F64 m1,arg1,m2,arg2;
  R2P(&m1,&arg1,n1->x,n1->y);
  R2P(&m2,&arg2,n2->x,n2->y);
  m1/=m2;
  arg1-=arg2;
  quot->x=m1*Cos(arg1);
  quot->y=m1*Sin(arg1);
  return quot;
}

public Complex *CScale(Complex *dst,F64 s)
{//dst*=s
  dst->x*=s;
  dst->y*=s;
  return dst;
}

public Complex *CCopy(Complex *dst,Complex *src)
{//dst=src
  dst->x=src->x;
  dst->y=src->y;
  return dst;
}

public Complex *CEqu(Complex *dst,F64 x,F64 y)
{//dst=(x,y)
  dst->x=x;
  dst->y=y;
  return dst;
}

public Complex *CPoly(Complex *dst,I64 n,Complex *zeros,Complex *x)
{//Eval complex polynomial
  I64 i;
  Complex n1,n2;
  if (n>0) {
    CSub(dst,x,&zeros[0]);
    for (i=1;i<n;i++) {
      CCopy(&n1,dst);
      CMul(dst,&n1,CSub(&n2,x,&zeros[i]));
    }
  } else
    CEqu(dst,1.0,0.0);
  return dst;
}
