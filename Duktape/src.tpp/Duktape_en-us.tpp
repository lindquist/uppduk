topic "";
[H6;0 $$1,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$2,0#37138531426314131252341829483370:codeitem]
[l288;2 $$3,0#27521748481378242620020725143825:desc]
[0 $$4,0#96390100711032703541132217272105:end]
[ $$0,0#00000000000000000000000000000000:Default]
[{_}%EN-US 
[s1;%- &]
[s1;%- &]
[s2;:DuktapeRegisterPrint`(Duktape`&`,const char`*`):%- [@(0.0.255) void] 
[* DuktapeRegisterPrint](Duktape[@(0.0.255) `&] [*@3 duk], [@(0.0.255) const] 
[@(0.0.255) char] [@(0.0.255) `*][*@3 container] [@(0.0.255) `=] [@(0.0.255) nullptr])&]
[s3; Registers simple `'print`' function in the Duktape context [%-*@3 duk] 
.&]
[s3; If [%-*@3 container] is nullptr, the function is inserted into 
`'globalThis`' otherwise the function is inserted into the global 
[%-*@3 container] .&]
[s4;%- &]
[s2;:DukBindGlobal`(duk`_context`*`,const T`&`,const char`*`):%- [@(0.0.255) template] 
<[@(0.0.255) typename] T> [@(0.0.255) bool] [* DukBindGlobal](duk`_context 
[@(0.0.255) `*][*@3 ctx], [@(0.0.255) const] T[@(0.0.255) `&] [*@3 value], 
[@(0.0.255) const] [@(0.0.255) char] [@(0.0.255) `*][*@3 name])&]
[s3; Binds a global variable in context [%-*@3 ctx] .&]
[s3; Stores [%-*@3 value] into [* globalThis.][%-*@3 name] .&]
[s3; If [%-*@3 value] is a function, a binding wrapper is created.&]
[s3; If [%-*@3 value] is a value/variable the value [/ at the time of 
binding] is stored. .&]
[s0; &]
[s0; &]
[s4;%- &]
[s0; ]]