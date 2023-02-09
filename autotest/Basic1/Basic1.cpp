#include <Core/Core.h>
#include <Duktape/Duktape.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	Duktape duk;
	
	// basic type conversions
	ASSERT(duk.Eval("123") == 123);
	ASSERT(duk.Eval("'foobar'") == "foobar");
	ASSERT(duk.Eval("0.5") == 0.5);
	ASSERT(duk.Eval("null").IsNull());
	ASSERT(duk.Eval("undefined").IsVoid());
	ASSERT(duk.Eval("true") == true);
	ASSERT(duk.Eval("false") == false);
	
	// cross-eval ref
	duk.Eval("function f(x) { return x+1; }");
	ASSERT(duk.Eval("f(2)") == 3);
	ASSERT(duk.Eval("").IsVoid());
	
	// error check
	Value err = duk.Eval("f(2");
	ASSERT(err.IsError());
	ASSERT(GetErrorText(err).Find("SyntaxError") >= 0);
	
	// type checks
	Value v;
	v = duk.Eval("247");
	ASSERT(v.GetType() == INT_V);
	v = duk.Eval("2.47");
	ASSERT(v.GetType() == DOUBLE_V);
	v = duk.Eval("undefined");
	ASSERT(v.GetType() == VOID_V);
	v = duk.Eval("call(x");
	ASSERT(v.GetType() == ERROR_V);
	v = duk.Eval("'foo'");
	ASSERT(v.GetType() == STRING_V);
	v = duk.Eval("true");
	ASSERT(v.GetType() == BOOL_V);
	v = duk.Eval("false");
	ASSERT(v.GetType() == BOOL_V);
	
	Cout() << "all ok" << EOL;
}
