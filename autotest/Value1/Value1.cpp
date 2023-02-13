#include <Core/Core.h>
#include <Duktape/Duktape.h>

using namespace Upp;

String func(Value v) {
	String json = AsJSON(v);
	Cout() << "func(v : " << v.GetTypeName() << ") := " << json << EOL;
	return pick(json);
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT);
	Duktape duk;
	
	duk.BindGlobal(&func, "func");
	ASSERT(duk.Eval("func(1)") == "1");
	ASSERT(duk.Eval("func({})") == "{}");
	ASSERT(duk.Eval("func({foo:23,bar:23})") == R"({"foo":23,"bar":23})");
	ASSERT(duk.Eval("func([])") == "[]");
	ASSERT(duk.Eval("func('foo')") == "\"foo\"");
	ASSERT(duk.Eval("func([1,'foo',{bar:[1,2,3]},true])") == R"([1,"foo",{"bar":[1,2,3]},true])");
	
	Cout() << "all ok" << EOL;
}
