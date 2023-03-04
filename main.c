#define AIWN_BOOTSTRAP
#define AIWNIOS_TESTS
#include "aiwn.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#ifdef AIWNIOS_TESTS
// Import PrintI first
static void PrintI(char* str, int64_t i) { printf("%s:%ld\n", str, i); }
static void PrintF(char* str, double f) { printf("%s:%lf\n", str, f); }
static void PrintPtr(char* str, void* f) { printf("%s:%p\n", str, f); }
static void FuzzTest1()
{
	int64_t i, i2, o;
	char tf[TMP_MAX];
	char buf[TMP_MAX + 64];
	tmpnam(tf);
	FILE* f = fopen(tf, "w");
	fprintf(f, "extern U0 PrintI(U8i *,I64i);\n");
	// Do complicated expr to dirty some temp registers
	fprintf(f, "I64i Fa() {I64i a,b,c;a=50,b=2,c=1; return c+(10*b)+(a*(1+!!c))+b;}\n");
	fprintf(f, "I64i Fb() {return Fa-123+6;}\n");
	fprintf(f, "U0 Fuzz() {\n");
	fprintf(f, "    I64i ra,rb,na,nb,*pna,*pnb;\n");
	fprintf(f, "    pna=&na,pnb=&nb;\n");
	fprintf(f, "    ra=na=123;\n");
	fprintf(f, "    rb=nb=6;\n");
	char* operandsA[] = {
		"ra",
		"na",
		"*pna(I32i*)",
		"Fa",
		"123",
	};
	char* operandsB[] = {
		"rb",
		"nb",
		"*pnb(I32i*)",
		"Fb",
		"6",
	};
	char* bopers[] = {
		"+",
		"-",
		"*",
		"%",
		"/",
		"==",
		"!=",
		">",
		"<",
		">=",
		"<=",
		"&",
		"^",
		"|",
		"<<",
		">>",
	};
	char* assignOps[] = {
		"=", "+=", "-=", "*=", "%=", "/=", "<<=", ">>=", "&=", "|=", "^="
	};
	char* assignUOps[] = { "++", "--" };
	for (o = 0; o != sizeof(bopers) / sizeof(char*); o++) {
		for (i = 0; i != sizeof(operandsA) / sizeof(char*); i++)
			for (i2 = 0; i2 != sizeof(operandsB) / sizeof(char*); i2++) {
				fprintf(f, "PrintI(\"%s %s %s ==\", %s %s %s);\n", operandsA[i],
					bopers[o], operandsB[i2], operandsA[i], bopers[o],
					operandsB[i2]);
			}
	}
	for (o = 0; o != sizeof(assignOps) / sizeof(char*); o++) {
		// Last 2 of operandsA/B are immuatable,so dont change them
		for (i = 0; i != sizeof(operandsA) / sizeof(char*) - 2; i++)
			for (i2 = 0; i2 != sizeof(operandsB) / sizeof(char*); i2++) {
				fprintf(f, "%s = 123;\n", operandsA[i]);
				fprintf(f, "%s %s %s;\nPrintI(\"%s %s %s ==\", %s);\n", operandsA[i],
					assignOps[o], operandsB[i2], operandsA[i], assignOps[o],
					operandsB[i2], operandsA[i]);
			}
	}
	for (o = 0; o != sizeof(assignUOps) / sizeof(char*); o++) {
		// Last 2 of operandsA/B are immuatable,so dont change them
		for (i = 0; i != sizeof(operandsA) / sizeof(char*) - 2; i++) {
			fprintf(f, "%s = 123;\n", operandsA[i]);
			fprintf(f, "PrintI(\"(%s) %s\",(%s) %s);", operandsA[i], assignUOps[o],
				operandsA[i], assignUOps[o]);
			fprintf(f, "PrintI(\"%s\",%s);\n", operandsA[i], operandsA[i]);
			fprintf(f, "%s = 123;\n", operandsA[i]);
			fprintf(f, "PrintI(\"%s (%s)\",%s (%s));\n", assignUOps[o], operandsA[i],
				assignUOps[o], operandsA[i]);
			fprintf(f, "PrintI(\"%s\",%s);\n", operandsA[i], operandsA[i]);
		}
	}
	fprintf(f, "}\n");
	fclose(f);
	sprintf(buf, "#include \"%s\";\n", tf);
	CLexer* lex = LexerNew("None", buf);
	CCmpCtrl* ccmp = CmpCtrlNew(lex);
	CodeCtrlPush(ccmp);
	Lex(lex);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsBindCSymbol("PrintI", &PrintI);
	ccmp->cur_fun = HashFind("Fuzz", Fs->hash_table, HTT_FUN, 1);
	int64_t (*poop5)() = ccmp->cur_fun->fun_ptr;
	poop5();
}
static void FuzzTest2()
{
	int64_t i, i2, o;
	char tf[TMP_MAX];
	char buf[TMP_MAX + 64];
	tmpnam(tf);
	FILE* f = fopen(tf, "w");
	fprintf(f, "extern U0 PrintF(U8i *,F64);\n");
	// Do complicated expr to dirty some temp registers
	fprintf(f, "F64 Fa() {F64 a=50,b=2,c=1; return c+(10*b)+(a*(1+!!c))+b;}\n");
	fprintf(f, "F64 Fb() {return Fa-123. +6.;}\n");
	char* operandsA[] = {
		"ra",
		"na",
		"*pna",
		"Fa",
		"123.",
	};
	char* operandsB[] = {
		"rb",
		"nb",
		"*pnb",
		"Fb",
		"6.",
	};
	char* bopers[] = { "+", "-", "*", "%", "/", "==", "!=", ">", "<", ">=", "<=" };
	char* assignOps[] = { "=", "+=", "-=", "*=", "%=", "/=" };
	char* assignUOps[] = { "++", "--" };
	fprintf(f, "U0 Fuzz() {\n");
	fprintf(f, "    F64 ra,rb,na,nb,*pna,*pnb;\n");
	fprintf(f, "    pna=&na,pnb=&nb;\n");
	fprintf(f, "    ra=na=123.;\n");
	fprintf(f, "    rb=nb=6.;\n");
	for (o = 0; o != sizeof(bopers) / sizeof(*bopers); o++)
		for (i = 0; i != sizeof(operandsA) / sizeof(*operandsA); i++)
			for (i2 = 0; i2 != sizeof(operandsA) / sizeof(*operandsA); i2++) {
				fprintf(f, "PrintF(\"%s %s %s ==\", %s %s %s);\n", operandsA[i],
					bopers[o], operandsB[i2], operandsA[i], bopers[o],
					operandsB[i2]);
			}
	for (o = 0; o != sizeof(assignOps) / sizeof(*assignOps); o++)
		for (i = 0; i != sizeof(operandsA) / sizeof(*operandsA) - 2; i++)
			for (i2 = 0; i2 != sizeof(operandsB) / sizeof(*operandsB); i2++) {
				fprintf(f, "%s=123.;\n", operandsA[i]);
				fprintf(f, "PrintF(\"%s %s %s ==\", %s %s %s);\n", operandsA[i],
					assignOps[o], operandsB[i2], operandsA[i], assignOps[o],
					operandsB[i2]);
			}
	for (o = 0; o != sizeof(assignUOps) / sizeof(char*); o++) {
		// Last 2 of operandsA/B are immuatable,so dont change them
		for (i = 0; i != sizeof(operandsA) / sizeof(char*) - 2; i++) {
			fprintf(f, "%s=123;\n", operandsA[i]);
			fprintf(f, "PrintF(\"(%s)%s\",(%s)%s);", operandsA[i], assignUOps[o],
				operandsA[i], assignUOps[o]);
			fprintf(f, "PrintF(\"%s\",%s);\n", operandsA[i], operandsA[i]);
			fprintf(f, "%s=123;\n", operandsA[i]);
			fprintf(f, "PrintF(\"%s(%s)\",%s(%s));\n", assignUOps[o], operandsA[i],
				assignUOps[o], operandsA[i]);
			fprintf(f, "PrintF(\"%s\",%s);\n", operandsA[i], operandsA[i]);
		}
	}
	fprintf(f, "}\n");
	fclose(f);
	sprintf(buf, "#include \"%s\";\n", tf);
	CLexer* lex = LexerNew("None", buf);
	CCmpCtrl* ccmp = CmpCtrlNew(lex);
	CodeCtrlPush(ccmp);
	Lex(lex);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsBindCSymbol("PrintF", &PrintF);
	ccmp->cur_fun = HashFind("Fuzz", Fs->hash_table, HTT_FUN, 1);
	int64_t (*poopf)() = ccmp->cur_fun->fun_ptr;
	poopf();
}
static void FuzzTest3()
{
	int64_t i, i2, o;
	char tf[TMP_MAX];
	char buf[TMP_MAX + 64];
	tmpnam(tf);
	FILE* f = fopen(tf, "w");
	fprintf(f, "extern U0 PrintPtr(U8i *,I64i);\n");
	// Do complicated expr to dirty some temp registers
	fprintf(f, "I64i Fa() {I64i a=50,b=2,c=1; return (c+(10*b)+(a*(1+!!c))+b-23)/100*0x100;}\n");
	fprintf(f, "I64i Fb() {return Fa-0x100 +2;}\n");
	char* operandsA[] = {
		"ra",
		"na",
		"*pna",
		"Fa(I32i*)",
		"0x100(I32i*)",
	};
	char* operandsB[] = {
		"rb",
		"nb",
		"*pnb",
		"Fb",
		"2",
	};
	char* bopers[] = { "+", "-" };
	char* assignOps[] = { "=", "+=", "-=" };
	char* assignUOps[] = { "++", "--" };
	fprintf(f, "U0 Fuzz() {\n");
	fprintf(f, "    I32i *ra,*na,**pna;\n");
	fprintf(f, "    I64i rb,nb,*pnb;\n");
	fprintf(f, "    pna=&na,pnb=&nb;\n");
	fprintf(f, "    ra=na=0x100;\n");
	fprintf(f, "    rb=nb=2;\n");
	for (o = 0; o != sizeof(bopers) / sizeof(*bopers); o++)
		for (i = 0; i != sizeof(operandsA) / sizeof(*operandsA); i++)
			for (i2 = 0; i2 != sizeof(operandsA) / sizeof(*operandsA); i2++) {
				fprintf(f, "PrintPtr(\"%s %s %s ==\", %s %s %s);\n", operandsA[i],
					bopers[o], operandsB[i2], operandsA[i], bopers[o],
					operandsB[i2]);
			}

	//Test subtraction between ptr and ptr
	for (i = 0; i != sizeof(operandsA) / sizeof(char*) - 2; i++) {
		fprintf(f, "PrintPtr(\"%s-0x10(I32i*)\",%s-0x10(I32i*));\n",
			operandsA[i],
			operandsA[i]);
	}
	for (o = 0; o != sizeof(assignOps) / sizeof(*assignOps); o++)
		for (i = 0; i != sizeof(operandsA) / sizeof(*operandsA) - 2; i++)
			for (i2 = 0; i2 != sizeof(operandsB) / sizeof(*operandsB); i2++) {
				fprintf(f, "%s=0x100;\n", operandsA[i]);
				fprintf(f, "PrintPtr(\"%s %s %s ==\", %s %s %s);\n", operandsA[i],
					assignOps[o], operandsB[i2], operandsA[i], assignOps[o],
					operandsB[i2]);
			}
	for (o = 0; o != sizeof(assignUOps) / sizeof(char*); o++) {
		// Last 2 of operandsA/B are immuatable,so dont change them
		for (i = 0; i != sizeof(operandsA) / sizeof(char*) - 2; i++) {
			fprintf(f, "%s=0x100;\n", operandsA[i]);
			fprintf(f, "PrintPtr(\"(%s)%s\",(%s)%s);", operandsA[i], assignUOps[o],
				operandsA[i], assignUOps[o]);
			fprintf(f, "PrintPtr(\"%s\",%s);\n", operandsA[i], operandsA[i]);
			fprintf(f, "%s=0x100;\n", operandsA[i]);
			fprintf(f, "PrintPtr(\"%s(%s)\",%s(%s));\n", assignUOps[o], operandsA[i],
				assignUOps[o], operandsA[i]);
			fprintf(f, "PrintPtr(\"%s\",%s);\n", operandsA[i], operandsA[i]);
		}
	}
	fprintf(f, "}\n");
	fclose(f);
	sprintf(buf, "#include \"%s\";\n", tf);
	CLexer* lex = LexerNew("None", buf);
	CCmpCtrl* ccmp = CmpCtrlNew(lex);
	CodeCtrlPush(ccmp);
	Lex(lex);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsStmt(ccmp);
	PrsBindCSymbol("PrintPtr", &PrintPtr);
	ccmp->cur_fun = HashFind("Fuzz", Fs->hash_table, HTT_FUN, 1);
	int64_t (*poop_ptr)() = ccmp->cur_fun->fun_ptr;
	poop_ptr();
}
#endif
static double Pow10(double d)
{
	return pow(10, d);
}
static void* MemSetU16(int16_t* dst, int16_t with, int64_t cnt)
{
	while (--cnt >= 0) {
		dst[cnt] = with;
	}
	return dst;
}
static void* MemSetU32(int32_t* dst, int32_t with, int64_t cnt)
{
	while (--cnt >= 0) {
		dst[cnt] = with;
	}
	return dst;
}
static void* MemSetU64(int64_t* dst, int64_t with, int64_t cnt)
{
	while (--cnt >= 0) {
		dst[cnt] = with;
	}
	return dst;
}
static void PutS(char* s)
{
	printf("%s", s);
}

//This a "symbolic" Fs only used from the HolyC part
static void SetHolyFs(void* fs)
{
	HolyFs = fs;
}
static void* GetHolyFs()
{
	return HolyFs;
}
static int64_t MemCmp(char *a ,char *b,int64_t s) {
  return memcmp(a,b,s);
}
static int64_t __GetTicks()
{
	return SDL_GetTicks();
}
static int64_t IsValidPtr(int64_t ptr)
{
	static int64_t ps;
	if (!ps)
		ps = sysconf(_SC_PAGESIZE);
	ptr -= ps % ptr;
	return EINVAL != msync((void*)ptr, ps, MS_ASYNC);
}
extern void AIWNIOS_setcontext(void*);
extern void AIWNIOS_getcontext(void*);
int64_t UnixNow() {
  return (int64_t)time(NULL);
}
static double Arg(double x,double y) {
  return atan2(y,x);
}
static void __Sleep(int64_t ms) {
  SDL_Delay(ms);
}
void BootAiwnios()
{
	//WIP
	CLexer* lex = LexerNew("None", "#include\"FULL_PACKAGE.HC\";");
	CCmpCtrl* ccmp = CmpCtrlNew(lex);
	void (*to_run)();
	CodeCtrlPush(ccmp);
	Lex(lex);
	while (PrsStmt(ccmp)) {
		to_run = Compile(ccmp,NULL,NULL);
		to_run();
		A_FREE(to_run);
		CodeCtrlPop(ccmp);
		CodeCtrlPush(ccmp);
		// TODO make a better way of doing this
		PrsBindCSymbol("Cos", &cos);
		PrsBindCSymbol("Sin", &sin);
		PrsBindCSymbol("Tan", &tan);
    PrsBindCSymbol("Arg", &Arg);
    PrsBindCSymbol("ACos", &acos);
		PrsBindCSymbol("ASin", &asin);
		PrsBindCSymbol("ATan", &atan);
		PrsBindCSymbol("HeapCtrlInit", &HeapCtrlInit);
		PrsBindCSymbol("HeapCtrlDel", &HeapCtrlDel);
		PrsBindCSymbol("__MAlloc", &__AIWNIOS_MAlloc);
		PrsBindCSymbol("__CAlloc", &__AIWNIOS_CAlloc);
		PrsBindCSymbol("Free", &__AIWNIOS_Free);
		PrsBindCSymbol("MSize", &MSize);
		PrsBindCSymbol("__StrNew", &__AIWNIOS_StrDup);
		PrsBindCSymbol("MemCpy", &memcpy);
		PrsBindCSymbol("MemSet", &memset);
		PrsBindCSymbol("MemSetU16", &MemSetU16);
		PrsBindCSymbol("MemSetU32", &MemSetU32);
		PrsBindCSymbol("MemSetU64", &MemSetU64);
		PrsBindCSymbol("StrLen", &strlen);
		PrsBindCSymbol("StrCmp", &strcmp);
		PrsBindCSymbol("ToUpper", &ToUpper);
		PrsBindCSymbol("Log10", &log10);
		PrsBindCSymbol("Log2", &log2);
		PrsBindCSymbol("Pow10", &Pow10);
		PrsBindCSymbol("PrintI", &PrintI);
		PrsBindCSymbol("PrintF", &PrintF);
		PrsBindCSymbol("Round", &round);
		PrsBindCSymbol("Ln", &log);
		PrsBindCSymbol("Floor", &floor);
		PrsBindCSymbol("Ceil", &ceil);
		PrsBindCSymbol("Sqrt", &sqrt);
		PrsBindCSymbol("MemCmp", &MemCmp);
		PrsBindCSymbol("Bt", &Bt);
		PrsBindCSymbol("LBtc", &LBtc);
		PrsBindCSymbol("LBts", &LBts);
		PrsBindCSymbol("LBtr", &LBtr);
		PrsBindCSymbol("Bsf", &Bsf);
		PrsBindCSymbol("Bsr", &Bsr);
		PrsBindCSymbol("DbgPutS", &puts);
		PrsBindCSymbol("PutS", &PutS);
		PrsBindCSymbol("SetFs", &SetHolyFs);
		PrsBindCSymbol("Fs", &GetHolyFs);
		PrsBindCSymbol("__GetTicks", &__GetTicks);
    PrsBindCSymbol("__Sleep",__Sleep);
    PrsBindCSymbol("ImportSymbolsToHolyC",ImportSymbolsToHolyC);
		PrsBindCSymbol("AIWNIOS_SetJmp", &AIWNIOS_getcontext);
		PrsBindCSymbol("AIWNIOS_LongJmp", &AIWNIOS_setcontext);
		PrsBindCSymbol("IsValidPtr", &IsValidPtr);
		PrsBindCSymbol("__HC_ICAdd_Typecast", __HC_ICAdd_Typecast);
		PrsBindCSymbol("__HC_ICAdd_SubCall", __HC_ICAdd_SubCall);
		PrsBindCSymbol("__HC_ICAdd_SubProlog", __HC_ICAdd_SubProlog);
		PrsBindCSymbol("__HC_ICAdd_SubRet", __HC_ICAdd_SubRet);
		PrsBindCSymbol("__HC_ICAdd_BoundedSwitch", __HC_ICAdd_Switch);
		PrsBindCSymbol("__HC_ICAdd_UnboundedSwitch", __HC_ICAdd_UnboundedSwitch);
		PrsBindCSymbol("__HC_ICAdd_PreInc", __HC_ICAdd_PreInc);
		PrsBindCSymbol("__HC_ICAdd_Call", __HC_ICAdd_Call);
		PrsBindCSymbol("__HC_ICAdd_F64", __HC_ICAdd_F64);
		PrsBindCSymbol("__HC_ICAdd_I64", __HC_ICAdd_I64);
		PrsBindCSymbol("__HC_ICAdd_PreDec", __HC_ICAdd_PreDec);
		PrsBindCSymbol("__HC_ICAdd_PostDec", __HC_ICAdd_PostDec);
		PrsBindCSymbol("__HC_ICAdd_PostInc", __HC_ICAdd_PostInc);
		PrsBindCSymbol("__HC_ICAdd_Pow", __HC_ICAdd_Pow);
		PrsBindCSymbol("__HC_ICAdd_Eq", __HC_ICAdd_Eq);
		PrsBindCSymbol("__HC_ICAdd_Div", __HC_ICAdd_Div);
		PrsBindCSymbol("__HC_ICAdd_Sub", __HC_ICAdd_Sub);
		PrsBindCSymbol("__HC_ICAdd_Mul", __HC_ICAdd_Mul);
		PrsBindCSymbol("__HC_ICAdd_Add", __HC_ICAdd_Add);
		PrsBindCSymbol("__HC_ICAdd_Deref", __HC_ICAdd_Deref);
		PrsBindCSymbol("__HC_ICAdd_Comma", __HC_ICAdd_Comma);
		PrsBindCSymbol("__HC_ICAdd_Addr", __HC_ICAdd_Addr);
		PrsBindCSymbol("__HC_ICAdd_Xor", __HC_ICAdd_Xor);
		PrsBindCSymbol("__HC_ICAdd_Mod", __HC_ICAdd_Mod);
		PrsBindCSymbol("__HC_ICAdd_Or", __HC_ICAdd_Or);
		PrsBindCSymbol("__HC_ICAdd_Lt", __HC_ICAdd_Lt);
		PrsBindCSymbol("__HC_ICAdd_Gt", __HC_ICAdd_Gt);
		PrsBindCSymbol("__HC_ICAdd_Le", __HC_ICAdd_Le);
		PrsBindCSymbol("__HC_ICAdd_Ge", __HC_ICAdd_Ge);
		PrsBindCSymbol("__HC_ICAdd_LNot", __HC_ICAdd_LNot);
    PrsBindCSymbol("__HC_ICAdd_Vargs",__HC_ICAdd_Vargs);
		PrsBindCSymbol("__HC_ICAdd_BNot", __HC_ICAdd_BNot);
		PrsBindCSymbol("__HC_ICAdd_AndAnd", __HC_ICAdd_AndAnd);
		PrsBindCSymbol("__HC_ICAdd_OrOr", __HC_ICAdd_OrOr);
		PrsBindCSymbol("__HC_ICAdd_XorXor", __HC_ICAdd_XorXor);
		PrsBindCSymbol("__HC_ICAdd_Ne", __HC_ICAdd_Ne);
		PrsBindCSymbol("__HC_ICAdd_Lsh", __HC_ICAdd_Lsh);
		PrsBindCSymbol("__HC_ICAdd_Rsh", __HC_ICAdd_Rsh);
		PrsBindCSymbol("__HC_ICAdd_AddEq", __HC_ICAdd_AddEq);
		PrsBindCSymbol("__HC_ICAdd_SubEq", __HC_ICAdd_SubEq);
		PrsBindCSymbol("__HC_ICAdd_MulEq", __HC_ICAdd_MulEq);
		PrsBindCSymbol("__HC_ICAdd_DivEq", __HC_ICAdd_DivEq);
		PrsBindCSymbol("__HC_ICAdd_LshEq", __HC_ICAdd_LshEq);
		PrsBindCSymbol("__HC_ICAdd_RshEq", __HC_ICAdd_RshEq);
		PrsBindCSymbol("__HC_ICAdd_AndEq", __HC_ICAdd_AndEq);
		PrsBindCSymbol("__HC_ICAdd_OrEq", __HC_ICAdd_OrEq);
		PrsBindCSymbol("__HC_ICAdd_XorEq", __HC_ICAdd_XorEq);
		PrsBindCSymbol("__HC_ICAdd_ModEq", __HC_ICAdd_ModEq);
		PrsBindCSymbol("__HC_ICAdd_FReg", __HC_ICAdd_FReg);
		PrsBindCSymbol("__HC_ICAdd_IReg", __HC_ICAdd_IReg);
		PrsBindCSymbol("__HC_ICAdd_Frame", __HC_ICAdd_Frame);
		PrsBindCSymbol("__HC_CodeMiscStrNew", __HC_CodeMiscStrNew);
		PrsBindCSymbol("__HC_CodeMiscLabelNew", __HC_CodeMiscLabelNew);
		PrsBindCSymbol("__HC_CmpCtrlNew", __HC_CmpCtrlNew);
		PrsBindCSymbol("__HC_CodeCtrlPush", __HC_CodeCtrlPush);
		PrsBindCSymbol("__HC_CodeCtrlPop", __HC_CodeCtrlPop);
		PrsBindCSymbol("__HC_Compile", __HC_Compile);
		PrsBindCSymbol("__HC_CodeMiscLabelNew", __HC_CodeMiscLabelNew);
		PrsBindCSymbol("__HC_CodeMiscStrNew", __HC_CodeMiscStrNew);
		PrsBindCSymbol("__HC_CodeMiscJmpTableNew", __HC_CodeMiscJmpTableNew);
		PrsBindCSymbol("__HC_ICAdd_Label", __HC_ICAdd_Label);
		PrsBindCSymbol("__HC_ICAdd_Goto", __HC_ICAdd_Goto);
		PrsBindCSymbol("__HC_ICAdd_GotoIf", __HC_ICAdd_GotoIf);
		PrsBindCSymbol("__HC_ICAdd_Str", __HC_ICAdd_Str);
		PrsBindCSymbol("__HC_ICAdd_And", __HC_ICAdd_And);
		PrsBindCSymbol("__HC_ICAdd_EqEq", __HC_ICAdd_EqEq);
		PrsBindCSymbol("__HC_ICAdd_Neg", __HC_ICAdd_Neg);
		PrsBindCSymbol("__HC_ICAdd_Ret", __HC_ICAdd_Ret);
		PrsBindCSymbol("__HC_ICAdd_Arg", __HC_ICAdd_Arg);
		PrsBindCSymbol("__HC_ICAdd_SetFrameSize", __HC_ICAdd_SetFrameSize);
    PrsBindCSymbol("__HC_ICAdd_Reloc",__HC_ICAdd_Reloc);
    PrsBindCSymbol("__HC_ICSetLine",__HC_ICSetLine);
    PrsBindCSymbol("__HC_ICAdd_StaticRef",__HC_ICAdd_StaticRef);
    PrsBindCSymbol("__HC_ICAdd_StaticData",__HC_ICAdd_StaticData);
    PrsBindCSymbol("__HC_ICAdd_SetStaticsSize",__HC_ICAdd_SetStaticsSize);
    PrsBindCSymbol("__HC_ICAdd_ToI64",__HC_ICAdd_ToI64);
    PrsBindCSymbol("__HC_ICAdd_ToF64",__HC_ICAdd_ToF64);
    PrsBindCSymbol("Caller",Caller);
    PrsBindCSymbol("VFsSetPwd",VFsSetPwd);
    PrsBindCSymbol("VFsExists",VFsFileExists);
    PrsBindCSymbol("VFsIsDir",VFsIsDir);
    PrsBindCSymbol("VFsFSize",VFsFSize);
    PrsBindCSymbol("VFsFRead",VFsFileRead);
    PrsBindCSymbol("VFsFWrite",VFsFileWrite);
    PrsBindCSymbol("VFsDel",VFsDel);
    PrsBindCSymbol("VFsDir",VFsDir);
    PrsBindCSymbol("VFsDirMk",VFsDirMk);
    PrsBindCSymbol("VFsFBlkRead",VFsFBlkRead);
    PrsBindCSymbol("VFsFBlkWrite",VFsFBlkWrite);
    PrsBindCSymbol("VFsFOpenW",VFsFOpenW);
    PrsBindCSymbol("VFsFOpenR",VFsFOpenR);
    PrsBindCSymbol("VFsFClose",VFsFClose);
    PrsBindCSymbol("VFsFSeek",VFsFSeek);
    PrsBindCSymbol("VFsSetDrv",VFsSetDrv);
    PrsBindCSymbol("FUnixTime",VFsUnixTime);
    PrsBindCSymbol("FSize",VFsFSize);
    PrsBindCSymbol("VFsFTrunc",VFsTrunc);
    PrsBindCSymbol("UnixNow",UnixNow);
    PrsBindCSymbol("__GrPaletteColorSet",GrPaletteColorSet);
    PrsBindCSymbol("DrawWindowNew",DrawWindowNew);
    PrsBindCSymbol("UpdateScreen",UpdateScreen);
    PrsBindCSymbol("SetKBCallback",SetKBCallback);
    PrsBindCSymbol("SetMSCallback",SetMSCallback);
	}
}
static void Boot(char *bin) {
  Fs = calloc(sizeof(CTask), 1);
	TaskInit(Fs, NULL, 0);
  VFsMountDrive('T',"./");
  BootAiwnios();
  if(bin)
    Load(bin);
}
int main()
{
  int64_t z = 3;
	int64_t idx;
	LaunchSDL(&Boot,"HCRT2.BIN");
  try {
#ifdef AIWNIOS_TESTS
		assert(!LBts(&z, 63));
		assert(z == (3 | (1l << 63)));
		assert(LBts(&z, 63));
		assert(z == (3 | (1l << 63)));
		assert(LBtr(&z, 63));
		assert(z == 3);
		assert(!Bt(&z, 63));
		z = 4;
		assert(Bt(&z, 2));
		FuzzTest1();
		FuzzTest2();
		FuzzTest3();
		CLexer* lex = LexerNew("None", "#include\"TEST0.HC\";");
		CCmpCtrl* ccmp = CmpCtrlNew(lex);
		CodeCtrlPush(ccmp);
		Lex(lex);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsStmt(ccmp);
		PrsBindCSymbol("PutS", &puts);
		ccmp->cur_fun = HashFind("Foo", Fs->hash_table, HTT_FUN, 1);
		int64_t (*poop)() = ccmp->cur_fun->fun_ptr;
		printf("Foo:%d\n", poop());
		ccmp->cur_fun = HashFind("Swit", Fs->hash_table, HTT_FUN, 1);
		int64_t (*poop2)(int64_t) = ccmp->cur_fun->fun_ptr;
		for (idx = 0; idx != 5; idx++)
			printf("Swit(%d):%d\n", idx, poop2(idx));
		ccmp->cur_fun = HashFind("Fib", Fs->hash_table, HTT_FUN, 1);
		int64_t (*poop3)(int64_t) = ccmp->cur_fun->fun_ptr;
		printf("FIB(9)==%d\n", poop3(9));
		ccmp->cur_fun = HashFind("SubSwit", Fs->hash_table, HTT_FUN, 1);
		int64_t (*poop4)(int64_t) = ccmp->cur_fun->fun_ptr;
		for (idx = 0; idx != 8; idx++)
			printf("SubSwit(%d):%d\n", idx, poop4(idx));
		ccmp->cur_fun = HashFind("Bar", Fs->hash_table, HTT_FUN, 1);
		int64_t (*poop5)() = ccmp->cur_fun->fun_ptr;
		poop5();
		ccmp->cur_fun = HashFind("UnionTest", Fs->hash_table, HTT_FUN, 1);
		int64_t (*poop6)() = ccmp->cur_fun->fun_ptr;
		poop6();
		ccmp->cur_fun = HashFind("FlowTest", Fs->hash_table, HTT_FUN, 1);
		int64_t (*poop7)() = ccmp->cur_fun->fun_ptr;
		poop7();
#endif
    BootAiwnios();
#ifdef AIWNIOS_TESTS
		ccmp->cur_fun = HashFind("Main", Fs->hash_table, HTT_FUN, 1);
		int64_t (*poop8)() = ccmp->cur_fun->fun_ptr;
		poop8();
#endif
	} catch ({
		puts("POOP");
	});
  puts("HERE");
  Load("HCRT2.BIN");
	return EXIT_SUCCESS;
}
