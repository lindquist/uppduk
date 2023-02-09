#include <Core/Core.h>
#include <Duktape/Duktape.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	Duktape duk;
	
	// availability
	ASSERT(IsError(duk.Eval("print('foobar')")));
	DuktapeRegisterPrint(duk); // 'print'
	ASSERT(!IsError(duk.Eval("print('foobar')")));
	
	// expected output
	StringStream ss;
	DukPrintSetStream(ss);
	duk.Eval("print('foobar')");
	String s = ss;
	ASSERT(s == "foobar\n" || s == "foobar\r\n");
	DukOut() << "foobar" << EOL;
	String s2 = ss;
	ASSERT(s2 == s+s);
	
	// multiple args
	ss.Create();
	duk.Eval("print(1,2,3)");
	s = ss;
	ASSERT(TrimBoth(s) == "1 2 3");
	
	// empty args
	ss.Create();
	duk.Eval("print()");
	s = ss;
	ASSERT(s == "\n" || s == "\r\n");
}
