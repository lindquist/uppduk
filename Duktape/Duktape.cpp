#include "Duktape.h"

using namespace Upp;

namespace {

// we have to wrap the allocation in "ignore leaks" or things blow up

void* alloc_function(void *udata, duk_size_t size)
{
    //Cout() << "alloc " << size << EOL;
    if (size == 0)
        return nullptr;
    MemoryIgnoreLeaksBegin();
    auto p = MemoryAlloc(size);
    MemoryIgnoreLeaksEnd();
    if (p == nullptr)
        return nullptr;
    return p;
}

void* realloc_function(void *udata, void *ptr, duk_size_t size)
{
    if (ptr == nullptr)
        return alloc_function(udata, size);
    if (size == 0)
        return ptr;
    auto bsz = GetMemoryBlockSize(ptr);
    if (size <= bsz)
        return ptr;
    auto size2 = size;
    MemoryIgnoreLeaksBegin();
    if (MemoryTryRealloc(ptr, size2) && size == size2) {
        MemoryIgnoreLeaksEnd();
        return ptr;
    }
    auto mem = MemoryAlloc(size);
    MemoryIgnoreLeaksEnd();
    if (!mem)
        return nullptr;
    memcpy(mem, ptr, bsz);
    return mem;
}

void free_function(void *udata, void *ptr) {
    //Cout() << "free" << EOL;
    MemoryIgnoreLeaksBegin();
    MemoryFree(ptr);
    MemoryIgnoreLeaksEnd();
}

void fatal_function(void *udata, const char *msg)
{
    auto self = (Duktape*)udata;
    self->Fatal(msg);
}

} // namespace

//----------------------------------------------------------------------------

void DukSetWrapper(duk_context *ctx, void* func)
{
    duk_push_pointer(ctx, func);
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("funcptr"));
}

void *DukGetWrapper(duk_context *ctx)
{
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("funcptr"));
    auto ptr = duk_get_pointer(ctx, -1);
    duk_pop(ctx);
    return ptr;
}

bool DukFindAddGlobalObject(duk_context *ctx, const char *name)
{
    // get or create the target object
    duk_get_global_string(ctx, name);
    auto ty = duk_get_type(ctx, -1);
    if (ty == DUK_TYPE_UNDEFINED) {
        duk_pop(ctx);
        duk_push_object(ctx);
        duk_dup_top(ctx);
        duk_put_global_string(ctx, name);
    }
    else if (ty != DUK_TYPE_OBJECT) {
        duk_pop(ctx);
        return false;
    }
    // object on top of stack
    return true;
}

//----------------------------------------------------------------------------

void DukPush(duk_context *ctx, const Upp::Value& v)
{
    const dword typeNo = v.GetType();
    if (typeNo == ERROR_V)
        duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", ~GetErrorText(v));
    else if (v.IsVoid())
        duk_push_undefined(ctx);
    else if (v.IsNull())
        duk_push_null(ctx);
    else if (typeNo == BOOL_V)
        duk_push_boolean(ctx, bool(v));
    else if (IsIntegerValueTypeNo(typeNo))
        duk_push_int(ctx, int(v));
    else if (IsNumberValueTypeNo(typeNo))
        duk_push_number(ctx, double(v));
    else if (IsString(v))
        duk_push_string(ctx, ~v.ToString());
    else if (IsValueArray(v))
        DukPush(ctx, ValueArray(v));
    else if (IsValueMap(v))
        DukPush(ctx, ValueMap(v));
    else
        duk_push_string(ctx, ~Format("[Value(%s)]", ~v.ToString()));
}

void DukPush(duk_context *ctx, const Upp::ValueMap& map)
{
	auto idx = duk_push_object(ctx);
	int N = map.GetCount();
	for (int i = 0; i < N; ++i) {
		auto& key = map.GetKey(i);
		DukPush(ctx, key);
		auto& val = map.GetValue(i);
		DukPush(ctx, val);
		duk_put_prop(ctx, idx);
	}
}

void DukPush(duk_context *ctx, const Upp::ValueArray& array)
{
	auto idx = duk_push_array(ctx);
	int N = array.GetCount();
	for (int i = 0; i < N; ++i) {
		DukPush(ctx, array.Get(i));
		duk_put_prop_index(ctx, idx, i);
	}
}

void DukGet(duk_context *ctx, int idx, Upp::Value& out)
{
	if (duk_is_number(ctx, idx)) {
        duk_double_t number = duk_get_number(ctx, idx);
        duk_double_t intPart;
        duk_double_t fracPart = std::modf(number, &intPart);
        if ((fpclassify(fracPart) & FP_ZERO) && (intPart >= INT_MIN && intPart <= INT_MAX)) // integer
            out = int(intPart);
        else // real
            out = number;
    }
    else if (duk_is_string(ctx, idx))
        out = String(duk_get_string(ctx, idx));
	else if (duk_is_boolean(ctx, idx))
        out = duk_get_boolean(ctx, idx) != 0;
    else if (duk_is_null(ctx, idx))
        out = Null;
    else if (duk_is_undefined(ctx, idx))
        out = Value();
    else if (duk_is_array(ctx, idx)) {
        ValueArray array;
        DukGet(ctx, idx, array);
        out = pick(array);
    }
    else if (duk_is_object(ctx, idx)) {
        ValueMap map;
        DukGet(ctx, idx, map);
        out = pick(map);
    }
    else {
        duk_dup(ctx, idx);
        String str = duk_safe_to_string(ctx, -1);
        duk_pop(ctx);
        out = ErrorValue(str);
    }
}

void DukGet(duk_context *ctx, int idx, ValueMap& out)
{
	ASSERT(duk_is_object(ctx, idx)); // maybe use duk_is_object_coercible?
	ValueMap map;
	duk_enum(ctx, idx, 0); // [...enum
	while (duk_next(ctx, -1 , 1)) { // [...enum key value
		map.Add(DukGetArg<Value>(ctx, -2), DukGetArg<Value>(ctx, -1));
	    duk_pop_2(ctx); // [..enum
	}
	duk_pop(ctx); // [...
	out = pick(map);
}

void DukGet(duk_context *ctx, int idx, ValueArray& out)
{
	ASSERT(duk_is_array(ctx, idx));
	auto N = duk_get_length(ctx, idx);
	ValueArray array;
	array.SetCount(N);
	for (int i = 0; i < N; ++i) {
		duk_get_prop_index(ctx, idx, i);
		out.Set(i, DukGetArg<Value>(ctx, -1));
		duk_pop(ctx);
	}
}

//----------------------------------------------------------------------------

Duktape::Duktape()
{
    ctx = duk_create_heap(alloc_function,realloc_function,free_function,this,fatal_function);
    ASSERT(ctx != nullptr);
}
Duktape::~Duktape()
{
    if (ctx)
        duk_destroy_heap(ctx);
}

Value Duktape::Eval(const char* s) const
{
    duk_push_string(ctx, s);
    return Eval0();
}
Upp::Value Duktape::Eval(const char *s, int len) const
{
    duk_push_lstring(ctx, s, len);
    return Eval0();
}
Upp::Value Duktape::Eval(const String& s) const
{
    duk_push_lstring(ctx, ~s, s.GetCount());
    return Eval0();
}
Upp::Value Duktape::Eval0() const
{
    ASSERT(duk_get_top(ctx) > 0);
    ASSERT(duk_get_type(ctx, -1) == DUK_TYPE_STRING);
    Value r;
    if (duk_peval(ctx) == 0)
        DukGet(ctx, -1, r);
    else
        r = ErrorValue(duk_safe_to_string(ctx, -1));
    duk_pop(ctx);
    return r;
}

void Duktape::Fatal(const char* msg)
{
    String err;
    err << "fatal error: " << msg << "\n";
    Panic(~err);
}

void Duktape::PrintStack()
{
    Cout() << duk_safe_to_string(ctx, 0) << EOL;
}

bool Duktape::BindGlobal(duk_c_function dukfunc, int nparams, const char* name, const char* container)
{
    if (container) {
        if (!DukFindAddGlobalObject(ctx, container))
            return false;
        duk_push_c_function(ctx, dukfunc, nparams);
        bool ok = duk_put_prop_string(ctx, -2, name) == 1;
        duk_pop(ctx);
        return ok;
    }
    else {
        duk_push_c_function(ctx, dukfunc, nparams);
        duk_put_global_string(ctx, name);
        return true;
    }
}
