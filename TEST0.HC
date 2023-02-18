//TEST0.HC
// December 18,a week until christmas
I64i Sum(...) {
  if(100>argc>0)
    PrintI("PASS",1);
  if(argc>=0)
    PrintI("PASS",2);
  if(argc<100)
    PrintI("PASS",3);
  if(argc<=3)
    PrintI("PASS",4);
  if(argc==3)
    PrintI("PASS",5);
  if(argc!=4)
    PrintI("PASS",6);
  if(!(argc>0))
    ;
  else
    PrintI("FAIL",1);
  if(!(argc>=0))
    ;
  else
    PrintI("FAIL",2);
  if(!(argc<100))
    ;
  else
    PrintI("FAIL",3);
  if(!(argc<=3))
    ;
  else
    PrintI("FAIL",4);
  if(!(argc==3))
    ;
  else
    PrintI("FAIL",5);
  if(!(argc!=4))
    ;
  else
    PrintI("FAIL",6);
  
  I64i a,sum;
  for(sum=a=0;a!=argc;a+=1) 
    sum+=argv[a];
  return sum;
}
I64i Foo() {
  return Sum(1,2,3);
}
I64i Swit(I64i a) {
  switch(a) {
  case 1:
  case 2:
  case 3:
    return a+100;
  }
  return 0;
}
I64i Fib(I64i n) {
  if(n<2)
    return n;
  return Fib(n-1)+Fib(n-2);
}
I64i SubSwit(I64i a) {
  switch(a) {
  case 0: break;
  case 2: break;
  case 4: break;
  case 6: break;
  start:
    a+=100;
  case 1: break;
  case 3: break;
  case 5: break;
  case 7: break;
  end:
    a+=1000;
   }
   return a;
}
I64i DeadCode() {
  goto @@a;
  1+2;
  3+4;
  100+200;
@@a:
  return 1;
  5+6;
}
extern U0 PutS(U8i *);
U0 Bar() {
  U8i array[128];
  array[0]='a';
  array[1]='b';
  array[2]='c';
  array[3]=0;
  PutS(array);
}
I64i union I64 {
  U8i u8[8];
  I8i i8[8];
  U16i u16[4];
  I16i i16[4];
  U32i u32[2];
  I32i i32[2];
};
//Store the 1,2,3 in the 2nd byte,see below
I64 array[3]={0x100,0x200,0x300};
U0 UnionTest() {
  I64 abcdef12=0xaabbccddeeff1122;
  PutS("Enter");
  if(array[0].u8[1]==0x1)
    PutS("A1");
  if(array[1].u8[1]==0x2)
    PutS("A2");
  if(array[2].u8[1]==0x3)
    PutS("A3");
  if(abcdef12.u8[0]==0x22)
    PutS("0xaa");
  if(abcdef12.u8[1]==0x11)
    PutS("0xbb");
  if(abcdef12.u16[0]==0x1122)
    PutS("0xaabb");
}
U0 FlowTest() {
  I64 z=0,c;
  if(!z) {
    PutS("pass1");
  }
  if(z) {
    PutS("FAIL");
  }
  if(!z) {
    PutS("pass2");
  }  else
    PutS("FAIL");
  if(z) {
    PutS("FAIL");
  }  else
    PutS("pass3");
  for(c=0;c!=1;c++) 
    PutS("pass4");
  PutS("Till3");
  for(c=0;c!=1000;c++) {
    PutS("*Till3*");
    if(c==3) {
      PutS("pass5");
      break;
    }
  }
  if(0) {
e:
    PutS("pass6");
  } else {
    goto e;
  }
  c=0;
  do {
    PutS("Do-w");
    if(c==3) {
      PutS("pass7");
      break;
    }
    c++;
  } while(1);
  c=0;
  while(1) {
    c++;
    PutS("While-W");
    if(c==3) {
      PutS("pass8");
      break;
    }
  }
  c=10;
  do{
    PutS("*Till10*");
    c--;
  } while(c>=0);
  PutS("pass9");
  c=10;
  while(c>=0) {
    PutS("*Till10*");
    c--;
  }
  PutS("pass10");
}
