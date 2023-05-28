#define AIWN_BOOTSTRAP
#define AIWNIOS_TESTS
#include "aiwn.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#ifdef AIWNIOS_TESTS
// Import PrintI first
static void PrintI(char* str, int64_t i) { printf("%s:%ld\n", str, i); }
static void PrintF(char* str, double f) { printf("%s:%lf\n", str, f); }
static void PrintPtr(char* str, void* f) { printf("%s:%p\n", str, f); }
static int64_t STK_PrintI(int64_t*);
static int64_t STK_PrintF(double*);
static int64_t STK_PrintPtr(int64_t* stk) { PrintPtr(stk[0], stk[1]); }
static void PrsAddSymbol(char* name, void* ptr, int64_t arity)
{
	PrsBindCSymbol(name, ptr);
}
static void PrsAddSymbolNaked(char* name, void* ptr, int64_t arity)
{
	PrsBindCSymbolNaked(name, ptr);
}
static void FuzzTest1()
{
	int64_t i, i2, o;
	char tf[TMP_MAX];
	strcpy(tf, "FUZZ1.HC");
	char buf[TMP_MAX + 64];
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
				fprintf(f, "PrintI(\"%s(123) %s %s(6) ==\", %s %s %s);\n", operandsA[i],
					bopers[o], operandsB[i2], operandsA[i], bopers[o],
					operandsB[i2]);
			}
	}
	for (o = 0; o != sizeof(assignOps) / sizeof(char*); o++) {
		// Last 2 of operandsA/B are immuatable,so dont change them
		for (i = 0; i != sizeof(operandsA) / sizeof(char*) - 2; i++)
			for (i2 = 0; i2 != sizeof(operandsB) / sizeof(char*); i2++) {
				fprintf(f, "%s = 123;\n", operandsA[i]);
				fprintf(f, "%s %s %s;\nPrintI(\"%s(123) %s %s(6) ==\", %s);\n", operandsA[i],
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
	PrsAddSymbol("PrintI", &STK_PrintI, 2);
	ccmp->cur_fun = HashFind("Fuzz", Fs->hash_table, HTT_FUN, 1);
	int64_t (*poop5)() = ccmp->cur_fun->fun_ptr;
	FFI_CALL_TOS_0(poop5);
}
static void FuzzTest2()
{
	int64_t i, i2, o;
	char tf[TMP_MAX];
	strcpy(tf, "FUZZ2.HC");
	char buf[TMP_MAX + 64];
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
	PrsAddSymbol("PrintF", &STK_PrintF, 2);
	ccmp->cur_fun = HashFind("Fuzz", Fs->hash_table, HTT_FUN, 1);
	int64_t (*poopf)() = ccmp->cur_fun->fun_ptr;
	FFI_CALL_TOS_0(poopf);
}
static void FuzzTest3()
{
	int64_t i, i2, o;
	char tf[TMP_MAX];
	strcpy(tf, "FUZZ3.HC");
	char buf[TMP_MAX + 64];
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

	// Test subtraction between ptr and ptr
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
	PrsAddSymbol("PrintPtr", &STK_PrintPtr, 2);
	ccmp->cur_fun = HashFind("Fuzz", Fs->hash_table, HTT_FUN, 1);
	int64_t (*poop_ptr)() = ccmp->cur_fun->fun_ptr;
	FFI_CALL_TOS_0(poop_ptr);
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

// This a "symbolic" Fs only used from the HolyC part
static void SetHolyFs(void* fs)
{
	HolyFs = fs;
}
static int64_t __GetTicksHP()
{
	struct timespec ts;
	static int64_t initial = 0;
	int64_t theTick = 0U;
	if (!initial) {
		clock_gettime(CLOCK_REALTIME, &ts);
		theTick = ts.tv_nsec / 1000;
		theTick += ts.tv_sec * 1000000U;
		initial = theTick;
		return 0;
	}
	clock_gettime(CLOCK_REALTIME, &ts);
	theTick = ts.tv_nsec / 1000;
	theTick += ts.tv_sec * 1000000U;
	return theTick - initial;
}
static int64_t __GetTicks()
{
	return __GetTicksHP() / 1000;
}
static void __SleepHP(int64_t us)
{
	usleep(us);
}
static void* GetHolyFs()
{
	return HolyFs;
}
static __thread void* HolyGs;
void* GetHolyGs()
{
	return HolyGs;
}
void SetHolyGs(void* ptr)
{
	HolyGs = ptr;
}
static int64_t MemCmp(char* a, char* b, int64_t s)
{
	return memcmp(a, b, s);
}
int64_t UnixNow()
{
	return (int64_t)time(NULL);
}
static double Arg(double x, double y)
{
	return atan2(y, x);
}
static void __Sleep(int64_t ms)
{
	SDL_Delay(ms);
}
static char* AiwniosGetClipboard()
{
	char *has, *ret;
	if (!SDL_HasClipboardText())
		return A_STRDUP("", NULL);
	has = SDL_GetClipboardText();
	ret = A_STRDUP(has, NULL);
	SDL_free(has);
	return ret;
}
static void AiwniosSetClipboard(char* c)
{
	if (c)
		SDL_SetClipboardText(c);
}

static void TaskContextSetRIP(int64_t* ctx, void* p)
{
	ctx[0] = p;
}

static STK_TaskContextSetRIP(int64_t* stk)
{
	TaskContextSetRIP(stk[0], stk[1]);
}

// strcmp returns int(32 bit on some platforms)
static int64_t StrCmp(char* a, char* b)
{
	return strcmp(a, b);
}

static int64_t STK_GenerateFFIForFun(void** stk)
{
	TaskContextSetRIP(stk[0], stk[1]);
}

static int64_t STK_AIWNIOS_makecontext(void** stk)
{
	return AIWNIOS_makecontext(stk[0], stk[1], stk[2]);
}

static int64_t STK___HC_SetAOTRelocBeforeRIP(void** stk)
{
	__HC_SetAOTRelocBeforeRIP(stk[0], stk[1]);
}

static int64_t STK___HC_CodeMiscIsUsed(void** stk)
{
	__HC_CodeMiscIsUsed(stk[0]);
}

static int64_t STK_AiwniosSetClipboard(void** stk)
{
	AiwniosSetClipboard(stk[0]);
}

static int64_t STK_AiwniosGetClipboard(void** stk)
{
	return AiwniosGetClipboard();
}

static int64_t STK_CmpCtrlDel(void** stk)
{
	CmpCtrlDel(stk[0]);
}

static int64_t STK_cos(double* stk)
{
	double r = cos(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_sin(double* stk)
{
	double r = sin(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_tan(double* stk)
{
	double r = tan(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_Arg(double* stk)
{
	double r = Arg(stk[0], stk[1]);
	return *(int64_t*)&r;
}

static int64_t STK_acos(double* stk)
{
	double r = acos(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_asin(double* stk)
{
	double r = asin(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_atan(double* stk)
{
	double r = atan(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_Misc_Btc(int64_t* stk)
{
	return Misc_Btc(stk[0], stk[1]);
}

static int64_t STK_Misc_Btr(int64_t* stk)
{
	return Misc_Btr(stk[0], stk[1]);
}
static int64_t STK_Misc_LBtr(int64_t* stk)
{
	return Misc_LBtr(stk[0], stk[1]);
}
static int64_t STK_Misc_LBts(int64_t* stk)
{
	return Misc_LBts(stk[0], stk[1]);
}
static int64_t STK_Misc_Bt(int64_t* stk)
{
	return Misc_Bt(stk[0], stk[1]);
}
static int64_t STK_Misc_LBtc(int64_t* stk)
{
	return Misc_LBtc(stk[0], stk[1]);
}
static int64_t STK_Misc_Bts(int64_t* stk)
{
	return Misc_Bts(stk[0], stk[1]);
}

static int64_t STK_HeapCtrlInit(int64_t* stk)
{
	return HeapCtrlInit(stk[0], stk[1],stk[2]);
}

static int64_t STK_HeapCtrlDel(int64_t* stk)
{
	HeapCtrlDel(stk[0]);
}

static int64_t STK___AIWNIOS_MAlloc(int64_t* stk)
{
	return __AIWNIOS_MAlloc(stk[0], stk[1]);
}
static int64_t STK___AIWNIOS_CAlloc(int64_t* stk)
{
	return __AIWNIOS_CAlloc(stk[0], stk[1]);
}

static int64_t STK___AIWNIOS_Free(int64_t* stk)
{
	__AIWNIOS_Free(stk[0]);
}

static int64_t STK_MSize(int64_t* stk)
{
	return MSize(stk[0]);
}

static int64_t STK___SleepHP(int64_t* stk)
{
	__SleepHP(stk[0]);
}

static int64_t STK___GetTicksHP(int64_t* stk)
{
	return __GetTicksHP(stk[0]);
}

static int64_t STK___AIWNIOS_StrDup(int64_t* stk)
{
	return __AIWNIOS_StrDup(stk[0], stk[1]);
}

static int64_t STK_memcpy(int64_t* stk)
{
	return memcpy(stk[0], stk[1], stk[2]);
}

static int64_t STK_memset(int64_t* stk)
{
	return memset(stk[0], stk[1], stk[2]);
}

static int64_t STK_MemSetU16(int64_t* stk)
{
	return MemSetU16(stk[0], stk[1], stk[2]);
}

static int64_t STK_MemSetU32(int64_t* stk)
{
	return MemSetU32(stk[0], stk[1], stk[2]);
}

static int64_t STK_MemSetU64(int64_t* stk)
{
	return MemSetU64(stk[0], stk[1], stk[2]);
}

static int64_t STK_strlen(int64_t* stk)
{
	return strlen(stk[0]);
}

static int64_t STK_strcmp(int64_t* stk)
{
	return strcmp(stk[0], stk[1]);
}

static int64_t STK_toupper(int64_t* stk)
{
	return toupper(stk[0]);
}

static int64_t STK_log10(double* stk)
{
	double r = log10(stk[0]);
	return *(int64_t*)&r;
}
static int64_t STK_log2(double* stk)
{
	double r = log2(stk[0]);
	return *(int64_t*)&r;
}
static int64_t STK_Pow10(double* stk)
{
	double r = Pow10(stk[0]);
	return *(int64_t*)&r;
}
static int64_t STK_sqrt(double* stk)
{
	double r = sqrt(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_pow(double* stk)
{
	double r = pow(stk[0], stk[1]);
	return *(int64_t*)&r;
}
static int64_t STK_exp(double* stk)
{
	double r = pow(stk[0], stk[1]);
	return *(int64_t*)&r;
}

static int64_t STK_PrintI(int64_t* stk)
{
	PrintI(stk[0], stk[1]);
}

static int64_t STK_PrintF(double* stk)
{
	PrintF(((int64_t*)stk)[0], stk[1]);
}

static int64_t STK_round(double* stk)
{
	double r = round(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_log(double* stk)
{
	double r = log(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_floor(double* stk)
{
	double r = floor(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_ceil(double* stk)
{
	double r = ceil(stk[0]);
	return *(int64_t*)&r;
}

static int64_t STK_memcmp(int64_t* stk)
{
	return memcmp(stk[0], stk[1], stk[2]);
}

static int64_t STK_Bt(int64_t* stk)
{
	return Misc_Bt(stk[0], stk[1]);
}

static int64_t STK_LBts(int64_t* stk)
{
	return Misc_LBts(stk[0], stk[1]);
}
static int64_t STK_LBtr(int64_t* stk)
{
	return Misc_LBtr(stk[0], stk[1]);
}
static int64_t STK_LBtc(int64_t* stk)
{
	return Misc_LBtc(stk[0], stk[1]);
}
static int64_t STK_Btr(int64_t* stk)
{
	return Misc_Btr(stk[0], stk[1]);
}
static int64_t STK_Bts(int64_t* stk)
{
	return Misc_Bts(stk[0], stk[1]);
}
static int64_t STK_Bsf(int64_t* stk)
{
	return Bsf(stk[0]);
}
static int64_t STK_Bsr(int64_t* stk)
{
	return Bsr(stk[0]);
}

static int64_t STK_DbgPutS(int64_t* stk)
{
	puts(stk[0]);
}
static int64_t STK_PutS(int64_t* stk)
{
	puts(stk[0]);
}
static int64_t STK_SetHolyFs(int64_t* stk)
{
	SetHolyFs(stk[0]);
}

static int64_t STK_GetHolyFs(int64_t* stk)
{
	return GetHolyFs();
}

static int64_t STK_SpawnCore(int64_t* stk)
{
	SpawnCore(stk[0], stk[1], stk[2]);
}

static int64_t STK_MPSleepHP(int64_t* stk)
{
	MPSleepHP(stk[0]);
}

static int64_t STK_MPAwake(int64_t* stk)
{
	MPAwake(stk[0]);
}

static int64_t STK_mp_cnt(int64_t* stk)
{
	return mp_cnt();
}

static int64_t STK_SetHolyGs(int64_t* stk)
{
	SetHolyGs(stk[0]);
}
static int64_t STK___GetTicks(int64_t* stk)
{
	return __GetTicks();
}

static int64_t STK___Sleep(int64_t* stk)
{
	__Sleep(stk[0]);
}

static int64_t STK_ImportSymbolsToHolyC(int64_t* stk)
{
	ImportSymbolsToHolyC(stk[0]);
}
static int64_t STK_AIWNIOS_getcontext(int64_t* stk)
{
	return AIWNIOS_getcontext(stk[0]);
}

static int64_t STK_AIWNIOS_setcontext(int64_t* stk)
{
	AIWNIOS_setcontext(stk[0]);
}

static int64_t STK_IsValidPtr(int64_t* stk)
{
	return IsValidPtr(stk[0]);
}
static int64_t STK___HC_CmpCtrl_SetAOT(int64_t* stk)
{
	__HC_CmpCtrl_SetAOT(stk[0]);
}

static int64_t STK___HC_ICAdd_Typecast(int64_t* stk)
{
	return __HC_ICAdd_Typecast(stk[0], stk[1], stk[2]);
}

static int64_t STK___HC_ICAdd_SubCall(int64_t* stk)
{
	return __HC_ICAdd_SubCall(stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_SubProlog(int64_t* stk)
{
	return __HC_ICAdd_SubProlog(stk[0]);
}

static int64_t STK___HC_ICAdd_SubRet(int64_t* stk)
{
	return __HC_ICAdd_SubRet(stk[0]);
}

static int64_t STK___HC_ICAdd_Switch(int64_t* stk)
{
	return __HC_ICAdd_Switch(stk[0], stk[1], stk[2]);
}

static int64_t STK___HC_ICAdd_UnboundedSwitch(int64_t* stk)
{
	return __HC_ICAdd_UnboundedSwitch(stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_PreInc(int64_t* stk)
{
	return __HC_ICAdd_PreInc(stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_Call(int64_t* stk)
{
	return __HC_ICAdd_Call(stk[0], stk[1], stk[2], stk[3]);
}

static int64_t STK___HC_ICAdd_F64(int64_t* stk)
{
	return __HC_ICAdd_F64(stk[0], ((double*)stk)[1]);
}

static int64_t STK___HC_ICAdd_I64(int64_t* stk)
{
	return __HC_ICAdd_I64(stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_PreDec(int64_t* stk)
{
	return __HC_ICAdd_PreDec(stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_PostInc(int64_t* stk)
{
	return __HC_ICAdd_PostInc(stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_PostDec(int64_t* stk)
{
	return __HC_ICAdd_PostDec(stk[0], stk[1]);
}

static int64_t STK___HC_ICAdd_Pow(int64_t* stk)
{
	return __HC_ICAdd_Pow(stk[0]);
}

static int64_t STK___HC_ICAdd_Eq(int64_t* stk)
{
	return __HC_ICAdd_Eq(stk[0]);
}

static int64_t STK___HC_ICAdd_Div(int64_t* stk)
{
	return __HC_ICAdd_Div(stk[0]);
}

static int64_t STK___HC_ICAdd_Sub(int64_t* stk)
{
	return __HC_ICAdd_Sub(stk[0]);
}

static int64_t STK___HC_ICAdd_Mul(int64_t* stk)
{
	return __HC_ICAdd_Mul(stk[0]);
}

static int64_t STK___HC_ICAdd_Add(int64_t* stk)
{
	return __HC_ICAdd_Add(stk[0]);
}

static int64_t STK___HC_ICAdd_Deref(int64_t* stk)
{
	return __HC_ICAdd_Deref(stk[0], stk[1], stk[2]);
}

static int64_t STK___HC_ICAdd_Comma(int64_t* stk)
{
	return __HC_ICAdd_Comma(stk[0]);
}

static int64_t STK___HC_ICAdd_Addr(int64_t* stk)
{
	return __HC_ICAdd_Addr(stk[0]);
}

static int64_t STK___HC_ICAdd_Xor(int64_t* stk)
{
	return __HC_ICAdd_Xor(stk[0]);
}

static int64_t STK___HC_ICAdd_Mod(int64_t* stk)
{
	return __HC_ICAdd_Mod(stk[0]);
}
static int64_t STK___HC_ICAdd_Or(int64_t* stk)
{
	return __HC_ICAdd_Or(stk[0]);
}
static int64_t STK___HC_ICAdd_Lt(int64_t* stk)
{
	return __HC_ICAdd_Lt(stk[0]);
}
static int64_t STK___HC_ICAdd_Gt(int64_t* stk)
{
	return __HC_ICAdd_Gt(stk[0]);
}
static int64_t STK___HC_ICAdd_Ge(int64_t* stk)
{
	return __HC_ICAdd_Ge(stk[0]);
}
static int64_t STK___HC_ICAdd_Le(int64_t* stk)
{
	return __HC_ICAdd_Le(stk[0]);
}
static int64_t STK___HC_ICAdd_LNot(int64_t* stk)
{
	return __HC_ICAdd_LNot(stk[0]);
}
static int64_t STK___HC_ICAdd_Vargs(int64_t* stk)
{
	return __HC_ICAdd_Vargs(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_BNot(int64_t* stk)
{
	return __HC_ICAdd_BNot(stk[0]);
}
static int64_t STK___HC_ICAdd_AndAnd(int64_t* stk)
{
	return __HC_ICAdd_AndAnd(stk[0]);
}
static int64_t STK___HC_ICAdd_OrOr(int64_t* stk)
{
	return __HC_ICAdd_OrOr(stk[0]);
}
static int64_t STK___HC_ICAdd_XorXor(int64_t* stk)
{
	return __HC_ICAdd_XorXor(stk[0]);
}
static int64_t STK___HC_ICAdd_Ne(int64_t* stk)
{
	return __HC_ICAdd_Ne(stk[0]);
}
static int64_t STK___HC_ICAdd_Lsh(int64_t* stk)
{
	return __HC_ICAdd_Lsh(stk[0]);
}
static int64_t STK___HC_ICAdd_Rsh(int64_t* stk)
{
	return __HC_ICAdd_Rsh(stk[0]);
}
static int64_t STK___HC_ICAdd_AddEq(int64_t* stk)
{
	return __HC_ICAdd_AddEq(stk[0]);
}
static int64_t STK___HC_ICAdd_SubEq(int64_t* stk)
{
	return __HC_ICAdd_SubEq(stk[0]);
}
static int64_t STK___HC_ICAdd_MulEq(int64_t* stk)
{
	return __HC_ICAdd_MulEq(stk[0]);
}
static int64_t STK___HC_ICAdd_DivEq(int64_t* stk)
{
	return __HC_ICAdd_DivEq(stk[0]);
}
static int64_t STK___HC_ICAdd_LshEq(int64_t* stk)
{
	return __HC_ICAdd_LshEq(stk[0]);
}
static int64_t STK___HC_ICAdd_RshEq(int64_t* stk)
{
	return __HC_ICAdd_RshEq(stk[0]);
}
static int64_t STK___HC_ICAdd_AndEq(int64_t* stk)
{
	return __HC_ICAdd_AndEq(stk[0]);
}
static int64_t STK___HC_ICAdd_OrEq(int64_t* stk)
{
	return __HC_ICAdd_OrEq(stk[0]);
}
static int64_t STK___HC_ICAdd_XorEq(int64_t* stk)
{
	return __HC_ICAdd_XorEq(stk[0]);
}
static int64_t STK___HC_ICAdd_ModEq(int64_t* stk)
{
	return __HC_ICAdd_ModEq(stk[0]);
}
static int64_t STK___HC_ICAdd_IReg(int64_t* stk)
{
	return __HC_ICAdd_IReg(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK___HC_ICAdd_FReg(int64_t* stk)
{
	return __HC_ICAdd_FReg(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_Frame(int64_t* stk)
{
	return __HC_ICAdd_Frame(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK___HC_CodeMiscStrNew(int64_t* stk)
{
	return __HC_CodeMiscStrNew(stk[0], stk[1], stk[2]);
}
static int64_t STK___HC_CodeMiscLabelNew(int64_t* stk)
{
	return __HC_CodeMiscLabelNew(stk[0]);
}
static int64_t STK___HC_CmpCtrlNew(int64_t* stk)
{
	return __HC_CmpCtrlNew();
}
static int64_t STK___HC_CodeCtrlPush(int64_t* stk)
{
	return __HC_CodeCtrlPush(stk[0]);
}
static int64_t STK___HC_CodeCtrlPop(int64_t* stk)
{
	return __HC_CodeCtrlPop(stk[0]);
}
static int64_t STK___HC_Compile(int64_t* stk)
{
	return __HC_Compile(stk[0], stk[1], stk[2]);
}
static int64_t STK___HC_ICAdd_Label(int64_t* stk)
{
	return __HC_ICAdd_Label(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_Goto(int64_t* stk)
{
	return __HC_ICAdd_Goto(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_GotoIf(int64_t* stk)
{
	return __HC_ICAdd_GotoIf(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_Str(int64_t* stk)
{
	return __HC_ICAdd_Str(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_And(int64_t* stk)
{
	return __HC_ICAdd_And(stk[0]);
}
static int64_t STK___HC_ICAdd_EqEq(int64_t* stk)
{
	return __HC_ICAdd_EqEq(stk[0]);
}
static int64_t STK___HC_ICAdd_Neg(int64_t* stk)
{
	return __HC_ICAdd_Neg(stk[0]);
}
static int64_t STK___HC_ICAdd_Ret(int64_t* stk)
{
	return __HC_ICAdd_Ret(stk[0]);
}
static int64_t STK___HC_ICAdd_Arg(int64_t* stk)
{
	return __HC_ICAdd_Arg(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_SetFrameSize(int64_t* stk)
{
	return __HC_ICAdd_SetFrameSize(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_Reloc(int64_t* stk)
{
	return __HC_ICAdd_Reloc(stk[0], stk[1], stk[2], stk[3], stk[4], stk[5]);
}
static int64_t STK___HC_ICSetLine(int64_t* stk)
{
	__HC_ICSetLine(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_StaticRef(int64_t* stk)
{
	return __HC_ICAdd_StaticRef(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK___HC_ICAdd_StaticData(int64_t* stk)
{
	return __HC_ICAdd_StaticData(stk[0], stk[1], stk[2], stk[3], stk[4]);
}
static int64_t STK___HC_ICAdd_SetStaticsSize(int64_t* stk)
{
	return __HC_ICAdd_SetStaticsSize(stk[0], stk[1]);
}
static int64_t STK___HC_ICAdd_ToI64(int64_t* stk)
{
	return __HC_ICAdd_ToI64(stk[0]);
}
static int64_t STK___HC_ICAdd_BT(int64_t* stk)
{
	return __HC_ICAdd_BT(stk[0]);
}
static int64_t STK___HC_ICAdd_BTS(int64_t* stk)
{
	return __HC_ICAdd_BTS(stk[0]);
}
static int64_t STK___HC_ICAdd_BTR(int64_t* stk)
{
	return __HC_ICAdd_BTR(stk[0]);
}
static int64_t STK___HC_ICAdd_BTC(int64_t* stk)
{
	return __HC_ICAdd_BTC(stk[0]);
}

static int64_t STK___HC_ICAdd_LBTS(int64_t* stk)
{
	return __HC_ICAdd_LBTS(stk[0]);
}
static int64_t STK___HC_ICAdd_LBTR(int64_t* stk)
{
	return __HC_ICAdd_LBTR(stk[0]);
}
static int64_t STK___HC_ICAdd_LBTC(int64_t* stk)
{
	return __HC_ICAdd_LBTC(stk[0]);
}

static int64_t STK___HC_ICAdd_ToF64(int64_t* stk)
{
	return __HC_ICAdd_ToF64(stk[0]);
}
static int64_t STK___HC_ICAdd_ShortAddr(int64_t* stk)
{
	return __HC_ICAdd_ShortAddr(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK_Misc_Caller(int64_t* stk)
{
	return Misc_Caller(stk[0]);
}
static int64_t STK_VFsSetPwd(int64_t* stk)
{
	VFsSetPwd(stk[0]);
}
static int64_t STK_VFsFileExists(int64_t* stk)
{
	return VFsFileExists(stk[0]);
}
static int64_t STK_VFsIsDir(int64_t* stk)
{
	return VFsIsDir(stk[0]);
}
static int64_t STK_VFsFileRead(int64_t* stk)
{
	return VFsFileRead(stk[0], stk[1]);
}
static int64_t STK_VFsFileWrite(int64_t* stk)
{
	return VFsFileWrite(stk[0], stk[1], stk[2]);
}
static int64_t STK_VFsDel(int64_t* stk)
{
	return VFsDel(stk[0]);
}
static int64_t STK_VFsDir(int64_t* stk)
{
	return VFsDir(stk[0]);
}
static int64_t STK_VFsDirMk(int64_t* stk)
{
	return VFsDirMk(stk[0]);
}
static int64_t STK_VFsBlkRead(int64_t* stk)
{
	return VFsFBlkRead(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK_VFsBlkWrite(int64_t* stk)
{
	return VFsFBlkWrite(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK_VFsFOpenW(int64_t* stk)
{
	return VFsFOpenW(stk[0]);
}
static int64_t STK_VFsFOpenR(int64_t* stk)
{
	return VFsFOpenR(stk[0]);
}
static int64_t STK_VFsFClose(int64_t* stk)
{
	return VFsFClose(stk[0]);
}
static int64_t STK_VFsFSeek(int64_t* stk)
{
	return VFsFSeek(stk[0], stk[1]);
}
static int64_t STK_VFsSetDrv(int64_t* stk)
{
	VFsSetDrv(stk[0]);
}
static int64_t STK_VFsUnixTime(int64_t* stk)
{
	return VFsUnixTime(stk[0]);
}
static int64_t STK_VFsTrunc(int64_t* stk)
{
	return VFsTrunc(stk[0], stk[1]);
}
static int64_t STK_UpdateScreen(int64_t* stk)
{
	UpdateScreen(stk[0], stk[1], stk[2], stk[3]);
}
static int64_t STK_VFsFSize(int64_t* stk)
{
	return VFsFSize(stk[0]);
}
static int64_t STK_UnixNow(int64_t* stk)
{
	return UnixNow();
}
static int64_t STK_GrPaletteColorSet(int64_t* stk)
{
	GrPaletteColorSet(stk[0], stk[1]);
}
static int64_t STK_DrawWindowNew(int64_t* stk)
{
	DrawWindowNew();
}
static int64_t STK_SetKBCallback(int64_t* stk)
{
	SetKBCallback(stk[0]);
}
static int64_t STK_SndFreq(int64_t* stk)
{
	SndFreq(stk[0]);
}
static int64_t STK_SetMSCallback(int64_t* stk)
{
	SetMSCallback(stk[0]);
}
static int64_t STK_InteruptCore(int64_t* stk)
{
	InteruptCore(stk[0]);
}
static int64_t STK___HC_CodeMiscInterateThroughRefs(int64_t* stk)
{
	__HC_CodeMiscInterateThroughRefs(stk[0], stk[1], stk[2]);
}
static int64_t STK___HC_CodeMiscJmpTableNew(int64_t* stk)
{
	return __HC_CodeMiscJmpTableNew(stk[0], stk[1], stk[2], stk[3]);
}
void BootAiwnios()
{
	// WIP
	CLexer* lex = LexerNew("None", "1+1;//#include\"STAGE0.HC\";");
	CCmpCtrl* ccmp = CmpCtrlNew(lex);
	void (*to_run)();
	CodeCtrlPush(ccmp);
	Lex(lex);
	while (PrsStmt(ccmp)) {
		to_run = Compile(ccmp, NULL, NULL);
		FFI_CALL_TOS_0(to_run);
		A_FREE(to_run);
		CodeCtrlPop(ccmp);
		CodeCtrlPush(ccmp);
		// TODO make a better way of doing this
		PrsAddSymbol("TaskContextSetRIP", STK_TaskContextSetRIP, 2);
		PrsAddSymbol("MakeContext", STK_AIWNIOS_makecontext, 3);
		PrsAddSymbol("__HC_SetAOTRelocBeforeRIP", STK___HC_SetAOTRelocBeforeRIP, 2);
		PrsAddSymbol("__HC_CodeMiscIsUsed", STK___HC_CodeMiscIsUsed, 1);
		PrsAddSymbol("AiwniosSetClipboard", STK_AiwniosSetClipboard, 1);
		PrsAddSymbol("AiwniosGetClipboard", STK_AiwniosGetClipboard, 0);
		PrsAddSymbol("__HC_CmpCtrlDel", STK_CmpCtrlDel, 1);
		PrsAddSymbol("Cos", STK_cos, 1);
		PrsAddSymbol("Sin", STK_sin, 1);
		PrsAddSymbol("Tan", STK_tan, 1);
		PrsAddSymbol("Arg", STK_Arg, 1);
		PrsAddSymbol("ACos", STK_acos, 1);
		PrsAddSymbol("ASin", STK_asin, 1);
		PrsAddSymbol("ATan", STK_atan, 1);
		PrsAddSymbol("Exp", STK_exp, 1);
		PrsAddSymbol("Btc", STK_Misc_Btc, 2);
		PrsAddSymbol("HeapCtrlInit", STK_HeapCtrlInit, 3);
		PrsAddSymbol("HeapCtrlDel", STK_HeapCtrlDel, 1);
		PrsAddSymbol("__MAlloc", STK___AIWNIOS_MAlloc, 2);
		PrsAddSymbol("__CAlloc", STK___AIWNIOS_CAlloc, 2);
		PrsAddSymbol("Free", STK___AIWNIOS_Free, 1);
		PrsAddSymbol("MSize", STK_MSize, 1);
		PrsAddSymbol("__SleepHP", STK___SleepHP, 1);
		PrsAddSymbol("__GetTicksHP", STK___GetTicksHP, 1);
		PrsAddSymbol("__StrNew", STK___AIWNIOS_StrDup, 1);
		PrsAddSymbol("MemCpy", STK_memcpy, 3);
		PrsAddSymbol("MemSet", STK_memset, 3);
		PrsAddSymbol("MemSetU16", STK_MemSetU16, 3);
		PrsAddSymbol("MemSetU32", STK_MemSetU32, 3);
		PrsAddSymbol("MemSetU64", STK_MemSetU64, 3);
		PrsAddSymbol("StrLen", STK_strlen, 1);
		PrsAddSymbol("StrCmp", STK_strcmp, 2);
		PrsAddSymbol("ToUpper", STK_toupper, 1);
		PrsAddSymbol("Log10", STK_log10, 1);
		PrsAddSymbol("Log2", STK_log2, 1);
		PrsAddSymbol("Pow10", STK_Pow10, 1);
		PrsAddSymbol("Pow", STK_pow, 2);
		PrsAddSymbol("PrintI", STK_PrintI, 1);
		PrsAddSymbol("PrintF", STK_PrintF, 1);
		PrsAddSymbol("Round", STK_round, 1);
		PrsAddSymbol("Ln", STK_log, 1);
		PrsAddSymbol("Floor", STK_floor, 1);
		PrsAddSymbol("Ceil", STK_ceil, 1);
		PrsAddSymbol("Sqrt", STK_sqrt, 1);
		PrsAddSymbol("MemCmp", STK_memcmp, 3);
		PrsAddSymbol("Bt", STK_Misc_Bt, 2);
		PrsAddSymbol("LBtc", STK_Misc_LBtc, 2);
		PrsAddSymbol("LBts", STK_Misc_LBts, 2);
		PrsAddSymbol("LBtr", STK_Misc_LBtr, 2);
		PrsAddSymbol("Bts", STK_Misc_Bts, 2);
		PrsAddSymbol("Btr", STK_Misc_Btr, 2);
		PrsAddSymbol("Bsf", STK_Bsf, 2);
		PrsAddSymbol("Bsr", STK_Bsr, 2);
		PrsAddSymbol("DbgPutS", STK_PutS, 1);
		PrsAddSymbol("PutS", STK_PutS, 1);
		PrsAddSymbol("SetFs", STK_SetHolyFs, 1);
		PrsAddSymbol("Fs", GetHolyFs, 0); // Gs just calls Thread local storage on linux(not mutations in saved registers)
		PrsAddSymbol("SpawnCore", STK_SpawnCore, 1);
		PrsAddSymbol("MPSleepHP", STK_MPSleepHP, 1);
		PrsAddSymbol("MPAwake", STK_MPAwake, 1);
		PrsAddSymbol("mp_cnt", STK_mp_cnt, 0);
		PrsAddSymbol("Gs", GetHolyGs, 0); // Gs just calls Thread local storage on linux(not mutations in saved registers)
		PrsAddSymbol("SetGs", STK_SetHolyGs, 1);
		PrsAddSymbol("__GetTicks", STK___GetTicks, 1);
		PrsAddSymbol("__Sleep", STK___Sleep, 1);
		PrsAddSymbol("ImportSymbolsToHolyC", STK_ImportSymbolsToHolyC, 1);
		// These dudes will expected to return to a location on the stack,SO DONT MUDDY THE STACK WITH ABI "translations"
		PrsAddSymbolNaked("AIWNIOS_SetJmp", AIWNIOS_getcontext, 1);
		PrsAddSymbolNaked("AIWNIOS_LongJmp", AIWNIOS_setcontext, 1);
		PrsAddSymbol("IsValidPtr", STK_IsValidPtr, 1);
		PrsAddSymbol("__HC_CmpCtrl_SetAOT", STK___HC_CmpCtrl_SetAOT, 1);
		PrsAddSymbol("__HC_ICAdd_Typecast", STK___HC_ICAdd_Typecast, 3);
		PrsAddSymbol("__HC_ICAdd_SubCall", STK___HC_ICAdd_SubCall, 2);
		PrsAddSymbol("__HC_ICAdd_SubProlog", STK___HC_ICAdd_SubProlog, 1);
		PrsAddSymbol("__HC_ICAdd_SubRet", STK___HC_ICAdd_SubRet, 1);
		PrsAddSymbol("__HC_ICAdd_BoundedSwitch", STK___HC_ICAdd_Switch, 1);
		PrsAddSymbol("__HC_ICAdd_UnboundedSwitch", STK___HC_ICAdd_UnboundedSwitch, 1);
		PrsAddSymbol("__HC_ICAdd_PreInc", STK___HC_ICAdd_PreInc, 1);
		PrsAddSymbol("__HC_ICAdd_Call", STK___HC_ICAdd_Call, 4);
		PrsAddSymbol("__HC_ICAdd_F64", STK___HC_ICAdd_F64, 2);
		PrsAddSymbol("__HC_ICAdd_I64", STK___HC_ICAdd_I64, 2);
		PrsAddSymbol("__HC_ICAdd_PreDec", STK___HC_ICAdd_PreDec, 1);
		PrsAddSymbol("__HC_ICAdd_PostDec", STK___HC_ICAdd_PostDec, 1);
		PrsAddSymbol("__HC_ICAdd_PostInc", STK___HC_ICAdd_PostInc, 1);
		PrsAddSymbol("__HC_ICAdd_Pow", STK___HC_ICAdd_Pow, 1);
		PrsAddSymbol("__HC_ICAdd_Eq", STK___HC_ICAdd_Eq, 1);
		PrsAddSymbol("__HC_ICAdd_Div", STK___HC_ICAdd_Div, 1);
		PrsAddSymbol("__HC_ICAdd_Sub", STK___HC_ICAdd_Sub, 1);
		PrsAddSymbol("__HC_ICAdd_Mul", STK___HC_ICAdd_Mul, 1);
		PrsAddSymbol("__HC_ICAdd_Add", STK___HC_ICAdd_Add, 1);
		PrsAddSymbol("__HC_ICAdd_Deref", STK___HC_ICAdd_Deref, 3);
		PrsAddSymbol("__HC_ICAdd_Comma", STK___HC_ICAdd_Comma, 1);
		PrsAddSymbol("__HC_ICAdd_Addr", STK___HC_ICAdd_Addr, 1);
		PrsAddSymbol("__HC_ICAdd_Xor", STK___HC_ICAdd_Xor, 1);
		PrsAddSymbol("__HC_ICAdd_Mod", STK___HC_ICAdd_Mod, 1);
		PrsAddSymbol("__HC_ICAdd_Or", STK___HC_ICAdd_Or, 1);
		PrsAddSymbol("__HC_ICAdd_Lt", STK___HC_ICAdd_Lt, 1);
		PrsAddSymbol("__HC_ICAdd_Gt", STK___HC_ICAdd_Gt, 1);
		PrsAddSymbol("__HC_ICAdd_Le", STK___HC_ICAdd_Le, 1);
		PrsAddSymbol("__HC_ICAdd_Ge", STK___HC_ICAdd_Ge, 1);
		PrsAddSymbol("__HC_ICAdd_LNot", STK___HC_ICAdd_LNot, 1);
		PrsAddSymbol("__HC_ICAdd_Vargs", STK___HC_ICAdd_Vargs, 1);
		PrsAddSymbol("__HC_ICAdd_BNot", STK___HC_ICAdd_BNot, 1);
		PrsAddSymbol("__HC_ICAdd_AndAnd", STK___HC_ICAdd_AndAnd, 1);
		PrsAddSymbol("__HC_ICAdd_OrOr", STK___HC_ICAdd_OrOr, 1);
		PrsAddSymbol("__HC_ICAdd_XorXor", STK___HC_ICAdd_XorXor, 1);
		PrsAddSymbol("__HC_ICAdd_Ne", STK___HC_ICAdd_Ne, 1);
		PrsAddSymbol("__HC_ICAdd_Lsh", STK___HC_ICAdd_Lsh, 1);
		PrsAddSymbol("__HC_ICAdd_Rsh", STK___HC_ICAdd_Rsh, 1);
		PrsAddSymbol("__HC_ICAdd_AddEq", STK___HC_ICAdd_AddEq, 1);
		PrsAddSymbol("__HC_ICAdd_SubEq", STK___HC_ICAdd_SubEq, 1);
		PrsAddSymbol("__HC_ICAdd_MulEq", STK___HC_ICAdd_MulEq, 1);
		PrsAddSymbol("__HC_ICAdd_DivEq", STK___HC_ICAdd_DivEq, 1);
		PrsAddSymbol("__HC_ICAdd_LshEq", STK___HC_ICAdd_LshEq, 1);
		PrsAddSymbol("__HC_ICAdd_RshEq", STK___HC_ICAdd_RshEq, 1);
		PrsAddSymbol("__HC_ICAdd_AndEq", STK___HC_ICAdd_AndEq, 1);
		PrsAddSymbol("__HC_ICAdd_OrEq", STK___HC_ICAdd_OrEq, 1);
		PrsAddSymbol("__HC_ICAdd_XorEq", STK___HC_ICAdd_XorEq, 1);
		PrsAddSymbol("__HC_ICAdd_ModEq", STK___HC_ICAdd_ModEq, 1);
		PrsAddSymbol("__HC_ICAdd_FReg", STK___HC_ICAdd_FReg, 2);
		PrsAddSymbol("__HC_ICAdd_IReg", STK___HC_ICAdd_IReg, 4);
		PrsAddSymbol("__HC_ICAdd_Frame", STK___HC_ICAdd_Frame, 4);
		PrsAddSymbol("__HC_CodeMiscStrNew", STK___HC_CodeMiscStrNew, 3);
		PrsAddSymbol("__HC_CodeMiscLabelNew", STK___HC_CodeMiscLabelNew, 1);
		PrsAddSymbol("__HC_CmpCtrlNew", STK___HC_CmpCtrlNew, 0);
		PrsAddSymbol("__HC_CodeCtrlPush", STK___HC_CodeCtrlPush, 1);
		PrsAddSymbol("__HC_CodeCtrlPop", STK___HC_CodeCtrlPop, 1);
		PrsAddSymbol("__HC_Compile", STK___HC_Compile, 3);
		PrsAddSymbol("__HC_CodeMiscLabelNew", STK___HC_CodeMiscLabelNew, 1);
		PrsAddSymbol("__HC_CodeMiscStrNew", STK___HC_CodeMiscStrNew, 3);
		PrsAddSymbol("__HC_CodeMiscJmpTableNew", STK___HC_CodeMiscJmpTableNew, 4);
		PrsAddSymbol("__HC_ICAdd_Label", STK___HC_ICAdd_Label, 2);
		PrsAddSymbol("__HC_ICAdd_Goto", STK___HC_ICAdd_Goto, 2);
		PrsAddSymbol("__HC_ICAdd_GotoIf", STK___HC_ICAdd_GotoIf, 2);
		PrsAddSymbol("__HC_ICAdd_Str", STK___HC_ICAdd_Str, 2);
		PrsAddSymbol("__HC_ICAdd_And", STK___HC_ICAdd_And, 1);
		PrsAddSymbol("__HC_ICAdd_EqEq", STK___HC_ICAdd_EqEq, 1);
		PrsAddSymbol("__HC_ICAdd_Neg", STK___HC_ICAdd_Neg, 1);
		PrsAddSymbol("__HC_ICAdd_Ret", STK___HC_ICAdd_Ret, 1);
		PrsAddSymbol("__HC_ICAdd_Arg", STK___HC_ICAdd_Arg, 2);
		PrsAddSymbol("__HC_ICAdd_SetFrameSize", STK___HC_ICAdd_SetFrameSize, 2);
		PrsAddSymbol("__HC_ICAdd_Reloc", STK___HC_ICAdd_Reloc, 5);
		PrsAddSymbol("__HC_ICSetLine", STK___HC_ICSetLine, 2);
		PrsAddSymbol("__HC_ICAdd_StaticRef", STK___HC_ICAdd_StaticRef, 4);
		PrsAddSymbol("__HC_ICAdd_StaticData", STK___HC_ICAdd_StaticData, 4);
		PrsAddSymbol("__HC_ICAdd_SetStaticsSize", STK___HC_ICAdd_SetStaticsSize, 2);
		PrsAddSymbol("__HC_ICAdd_ToI64", STK___HC_ICAdd_ToI64, 1);
		PrsAddSymbol("__HC_ICAdd_ToF64", STK___HC_ICAdd_ToF64, 1);
		PrsAddSymbol("__HC_ICAdd_ShortAddr", STK___HC_ICAdd_ShortAddr, 4);
		PrsAddSymbol("__HC_CodeMiscInterateThroughRefs", STK___HC_CodeMiscInterateThroughRefs, 3);
		PrsAddSymbol("__HC_ICAdd_BT", STK___HC_ICAdd_BT, 1);
		PrsAddSymbol("__HC_ICAdd_BTS", STK___HC_ICAdd_BTS, 1);
		PrsAddSymbol("__HC_ICAdd_BTR", STK___HC_ICAdd_BTR, 1);
		PrsAddSymbol("__HC_ICAdd_BTC", STK___HC_ICAdd_BTC, 1);
		PrsAddSymbol("__HC_ICAdd_LBTS", STK___HC_ICAdd_LBTS, 1);
		PrsAddSymbol("__HC_ICAdd_LBTR", STK___HC_ICAdd_LBTR, 1);
		PrsAddSymbol("__HC_ICAdd_LBTC", STK___HC_ICAdd_LBTC, 1);
		PrsAddSymbol("Caller", STK_Misc_Caller, 1);
		PrsAddSymbol("VFsSetPwd", STK_VFsSetPwd, 1);
		PrsAddSymbol("VFsExists", STK_VFsFileExists, 1);
		PrsAddSymbol("VFsIsDir", STK_VFsIsDir, 1);
		PrsAddSymbol("VFsFSize", STK_VFsFSize, 1);
		PrsAddSymbol("VFsFRead", STK_VFsFileRead, 2);
		PrsAddSymbol("VFsFWrite", STK_VFsFileWrite, 3);
		PrsAddSymbol("VFsDel", STK_VFsDel, 1);
		PrsAddSymbol("VFsDir", STK_VFsDir, 1);
		PrsAddSymbol("VFsDirMk", STK_VFsDirMk, 1);
		PrsAddSymbol("VFsFBlkRead", STK_VFsBlkRead, 4);
		PrsAddSymbol("VFsFBlkWrite", STK_VFsBlkWrite, 4);
		PrsAddSymbol("VFsFOpenW", STK_VFsFOpenW, 1);
		PrsAddSymbol("VFsFOpenR", STK_VFsFOpenR, 1);
		PrsAddSymbol("VFsFClose", STK_VFsFClose, 1);
		PrsAddSymbol("VFsFSeek", STK_VFsFSeek, 1);
		PrsAddSymbol("VFsSetDrv", STK_VFsSetDrv, 1);
		PrsAddSymbol("FUnixTime", STK_VFsUnixTime, 1);
		PrsAddSymbol("FSize", STK_VFsFSize, 1);
		PrsAddSymbol("VFsFTrunc", STK_VFsTrunc, 1);
		PrsAddSymbol("UnixNow", STK_UnixNow, 0);
		PrsAddSymbol("__GrPaletteColorSet", STK_GrPaletteColorSet, 2);
		PrsAddSymbol("DrawWindowNew", STK_DrawWindowNew, 0);
		PrsAddSymbol("UpdateScreen", STK_UpdateScreen, 4);
		PrsAddSymbol("SetKBCallback", STK_SetKBCallback, 1);
		PrsAddSymbol("SndFreq", STK_SndFreq, 1);
		PrsAddSymbol("SetMSCallback", STK_SetMSCallback, 1);
		PrsAddSymbol("InteruptCore", STK_InteruptCore, 1);
	}
}
static void Boot()
{
	char* bin = "HCRT2.BIN";
	Fs = calloc(sizeof(CTask), 1);
	InstallDbgSignalsForThread();
	TaskInit(Fs, NULL, 0);
	VFsMountDrive('T', "./");
	FuzzTest1();
	FuzzTest2();
	FuzzTest3();

	BootAiwnios();
	glbl_table = Fs->hash_table;
	if (bin)
		Load(bin);
}
int main(int argc, char* argv[])
{
	int64_t quit = 0;
	SDL_Init(SDL_INIT_EVERYTHING);
	InitSound();
	user_ev_num = SDL_RegisterEvents(1);
	SpawnCore(&Boot, NULL, 0);
	InputLoop(&quit);
	return EXIT_SUCCESS;
}
