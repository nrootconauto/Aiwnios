class CABC {
  I64i a,b,c;
};
class CNest {
  CNest *self;
  CABC abc;
};
//Array Test
I64i Foo1(I64i a) {
  I64i array[2][2];
  array[0][0]=1,array[0][1]=2,array[1][0]=3,array[1][1]=4;
  return array[0][0]+array[0][1]+array[1][0]+array[1][1];
} 
//Recursion test
I64i Foo2(I64i a) {
  if(a<=2)
    return a;
  return Foo2(a-1)+Foo2(a-2);
}
//Member test
I64i Foo3(I64i o) {
  CNest var,*ptr=&var;
  var.self=ptr;
  var.abc.a=1,var.abc.b=o;var.abc.c=3;
  return ptr->self->abc.a+ptr->self->abc.b+ptr->self->abc.c;
}
//For test
I64i Foo4(I64i o) {
  I32i array[4],idx;
  for(idx=0;idx!=4;idx++) {
    array[idx]=idx+1;
  }
  return array[0]+array[1]+array[2]+array[3];
}

//If test(pass with o==8)
I64i Foo5(I64i o) {
  I64i sum=0;
  if(o==8)
    sum+=1;
  if(o!=8)
    ;
  else
    sum+=10;
  if(o>=8)
    sum+=100;
  return sum;
}
//While test
I64i Foo6(I64i o) {
  I64i sum=0;
  while(--o>=0) {
    sum+=o+1;
  }
  return sum;
} 
//Do test
I64i Foo7(I64i o) {
  I64i tests=0;
  do {
    tests++;
  } while(0);
  o--;
  do {
    tests*=10;
    tests++;
  } while(--o>=0);
  return tests;
}

//Switch test 1
I64i Foo8(I64i o) {
  I64i sum=0;
  while(--o>=0) {
    sum*=10;
    switch(o) {
      break;case 1:
      sum+=1;
      break;case 2:
      sum+=2;
      break;case 3:
      sum+=3;
      break;case 4:
      sum+=4;
      break;case 5:
      sum+=5;
      break;default:
      sum+=9;
    }
  }
  return sum;
}

extern U0i PutS(U8i *);
//Switch Test2
I64i Foo9(I64i o) {
  I64i sum=0;
  while(--o>=0) {
    switch(o) {
      start:
        PutS("[");
        case 1: PutS("1");
        break;case 3: PutS("3");
        break;case 5: PutS("5");
        break;case 7: PutS("7");
        break;case 9: PutS("9");
      end:
        PutS("]\n");
      break;case 2: PutS("2\n");
      break;case 4: PutS("4\n");
      break;case 6: PutS("6\n");
      break;case 8: PutS("8\n");
      break;case 10: PutS("10\n");
    }
  }
  return sum;
}
//Vargs test
I64i foo10(I64i a,...) {
  while(--argc>=0)
    a+=argv[argc];
  return a;
}
I64i Foo10(I64i a) {
  return foo10(a,100,200,300,400);
} 

I64i foo11() {
  static I64i x=10;
  static I64i a;
  if(x==10)
    a=100;
  return a+ ++x;
}

I64i Foo11() {
  return foo11;
}

I64i Foo12(I64i o) {
  I64i (*fib)(I64i);
  fib=&Foo2;
  return (*fib)(o);
}

I64i Foo13(I64i o) {
  I64i t;
  t(I64i)=-1;
  return t;
}
I64i Foo14(I64i *o) {
  o(U8i*)+=1;
  return (10+o)|(o+10)<<32;
}
