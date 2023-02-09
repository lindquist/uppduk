#include <Core/Core.h>
#include <Duktape/Duktape.h>

using namespace Upp;

struct S {
	int i;
	int f() { return i; }
};

S func() { return S{42}; }

S func2(int i) { return S{i}; }
S sum(S a, S b) { return S{a.i + b.i}; }

struct T {
	double d;
	T() { d = 1.0; }
	T(double dd) { d = dd; }
	double value() { return d; }
};

CONSOLE_APP_MAIN
{
	Duktape duk;
	DuktapeRegisterPrint(duk);
	
	// bind struct
	duk.BindClass<S>();
	duk.BindGlobal(func, "func");
	duk.Eval("var s = func(); print(s)");
	
	// bind method
	duk.BindMethod(&S::f, "f");
	Value v = duk.Eval("s.f()");
	ASSERT(v == 42);
	
	// more params
	duk.BindGlobal(func2, "func2");
	duk.BindGlobal(sum, "sum");
	duk.Eval("var s1 = func2(14)");
	duk.Eval("var s2 = func2(7);");
	v = duk.Eval("sum(s1,s2).f()");
	ASSERT(v == 21);
	
	// constructor
	duk.BindClass<T>();
	duk.BindConstructor<T,int>("T");
	duk.BindMethod(&T::value, "value");
	duk.Eval("var t = new T(13)");
	v = duk.Eval("t.value()");
	ASSERT(v == 13);
	
	Cout() << "all ok" << EOL;
}
