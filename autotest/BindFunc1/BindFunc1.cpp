#include <Core/Core.h>
#include <Duktape/Duktape.h>

using namespace Upp;

int g = 0;
void func() { g = 42; }

int ifunc() { return 13; }

void funci(int i) { g = i + 1; }

int isum(int a, int b) { return a + b; }

bool gt(double a, int b) { return a > b; }

Value vfunci() { return 17; }
Value vfuncd() { return 0.5; }
Value vfuncb() { return true; }
Value vfuncs() { return "hello world"; }

CONSOLE_APP_MAIN
{
	Duktape duk;
	
	// void(void)
	duk.BindGlobal(func, "func");
	ASSERT(g == 0);
	duk.Eval("func()");
	ASSERT(g == 42);
	
	// int(void)
	duk.BindGlobal(ifunc, "ifunc");
	ASSERT(duk.Eval("ifunc()") == 13);
	
	// void(int)
	duk.BindGlobal(funci, "funci");
	duk.Eval("funci(10)");
	ASSERT(g == 11);
	
	// int(int,int)
	duk.BindGlobal(isum, "isum");
	ASSERT(duk.Eval("isum(7,14)") == 21);
	
	// bool(double, int)
	duk.BindGlobal(gt, "gt");
	Value v = duk.Eval("gt(3.1415, 2)");
	ASSERT(v.GetType() == BOOL_V);
	ASSERT(v == true);
	v = duk.Eval("gt(0.707, 2)");
	ASSERT(v == false);
	
	// Value
	duk.BindGlobal(vfunci, "vfunci");
	duk.BindGlobal(vfuncd, "vfuncd");
	duk.BindGlobal(vfuncb, "vfuncb");
	duk.BindGlobal(vfuncs, "vfuncs");
	ASSERT(duk.Eval("vfunci()") == 17);
	ASSERT(duk.Eval("vfuncd()") == 0.5);
	ASSERT(duk.Eval("vfuncb()") == true);
	ASSERT(duk.Eval("vfuncs()") == "hello world");
}
