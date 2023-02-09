#include <Core/Core.h>
#include <Duktape/Duktape.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	Duktape duk;
	
	duk.Eval("function foo(a, b) { return a + b }");
	auto func = DukGetFunction<double, double, double>(duk, "foo");
	double x = func(2,4);
	ASSERT(x == 6);
	
	Cout() << "all ok" << EOL;
}
