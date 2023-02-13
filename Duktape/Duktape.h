#ifndef _Duktape_Duktape_h
#define _Duktape_Duktape_h

#include <utility> // index_sequence

#include <Core/Core.h>
#include <duktape.h>

// library registration functions

class Duktape;

void DuktapeRegisterPrint(Duktape& duk, const char* container = nullptr);
void DukPrintSetStream(Upp::Stream& stream);
Upp::Stream& DukOut();

void DuktapeRegisterCMath(Duktape& duk, const char* container = "cmath");

// duk_push_*

inline void DukPush(duk_context *ctx, bool b) {
    duk_push_boolean(ctx, b);
}

inline void DukPush(duk_context *ctx, int d) {
    duk_push_int(ctx, d);
}
inline void DukPush(duk_context *ctx, unsigned d) {
    duk_push_uint(ctx, d);
}

inline void DukPush(duk_context *ctx, float f) {
    duk_push_number(ctx, f);
}
inline void DukPush(duk_context *ctx, double f) {
    duk_push_number(ctx, f);
}
inline void DukPush(duk_context *ctx, long double f) {
    duk_push_number(ctx, f);
}

inline void DukPush(duk_context *ctx, const char* s) {
    duk_push_string(ctx, s);
}
inline void DukPush(duk_context *ctx, const Upp::String& s) {
    duk_push_string(ctx, ~s);
}

void DukPush(duk_context *ctx, const Upp::ValueMap& v);
void DukPush(duk_context *ctx, const Upp::ValueArray& v);
void DukPush(duk_context *ctx, const Upp::Value& v);

// duk_to_*

inline void DukGet(duk_context *ctx, int idx, bool& out) {
    out = duk_get_boolean(ctx, idx) != 0;
}

inline void DukGet(duk_context *ctx, int idx, int& out) {
    out = duk_get_int(ctx, idx);
}
inline void DukGet(duk_context *ctx, int idx, unsigned& out) {
    out = duk_get_uint(ctx, idx);
}

inline void DukGet(duk_context *ctx, int idx, float& out) {
    out = duk_get_number(ctx, idx);
}
inline void DukGet(duk_context *ctx, int idx, double& out) {
    out = duk_get_number(ctx, idx);
}
inline void DukGet(duk_context *ctx, int idx, long double& out) {
    out = duk_get_number(ctx, idx);
}

inline void DukGet(duk_context *ctx, int idx, const char*& out) {
    out = duk_get_string(ctx, idx);
}
inline void DukGet(duk_context *ctx, int idx, Upp::String& out) {
    duk_size_t len;
    auto ptr = duk_get_lstring(ctx, idx, &len);
    out.Set(ptr, len);
}

void DukGet(duk_context *ctx, int idx, Upp::Value& out);
void DukGet(duk_context *ctx, int idx, Upp::ValueMap& out);
void DukGet(duk_context *ctx, int idx, Upp::ValueArray& out);

// helpers for bindings

void DukSetWrapper(duk_context *ctx, void* func);
void* DukGetWrapper(duk_context *ctx);
bool DukFindAddGlobalObject(duk_context *ctx, const char* name);

template<typename Arg>
Arg DukGetArg(duk_context *ctx, int idx) {
    Arg arg;
    DukGet(ctx, idx, arg);
    /*if constexpr (std::is_base_of<Upp::Moveable<Arg>, Arg>::value)
        return pick(arg);
    else*/
        return arg;
}

// static function bindings

template<typename Ret, typename... Args, size_t... I>
Ret DukBindCallFunc(duk_context *ctx, Ret(*func)(Args...), std::index_sequence<I...>) {
    return func(DukGetArg<Args>(ctx, I)...);
}

template<typename Ret, typename... Args>
duk_ret_t DukBindWrapper(duk_context *ctx) {
    duk_push_current_function(ctx);
    auto fn = (Ret(*)(Args...))DukGetWrapper(ctx); // get the actual function pointer
    if constexpr (std::is_void<Ret>::value) {
        DukBindCallFunc(ctx, fn, std::index_sequence_for<Args...>{});
        return 0;
    }
    else {
        Ret ret = DukBindCallFunc(ctx, fn, std::index_sequence_for<Args...>{});
        DukPush(ctx, ret);
        return 1;
    }
}

template<typename Ret, typename... Args>
bool DukPush(duk_context *ctx, Ret(*func)(Args...)) {
    auto fnptr = DukBindWrapper<Ret, Args...>; // generate wrapper
    duk_push_c_function(ctx, fnptr, sizeof...(Args));
    DukSetWrapper(ctx, (void*)func); // store the function pointer called by the wrapper
    return true;
}

template<typename T>
bool DukBindGlobal(duk_context *ctx, const T& value, const char* name) {
    if (!name || !*name)
        return false;
    DukPush(ctx, value);
    return duk_put_global_string(ctx, name) == 1;
}

template<typename T>
bool DukBindGlobal(duk_context *ctx, const T& value, const char* name, const char* container) {
    if (!name || !*name)
        return false;
    else if (!container || !*container)
        duk_push_global_object(ctx);
    else if (!DukFindAddGlobalObject(ctx, container))
        return false;
    DukPush(ctx, value);
    bool ok = duk_put_prop_string(ctx, -2, name) == 1;
    duk_pop(ctx);
    return ok;
}

// class type binding

template<typename C>
bool DukBindClass(duk_context *ctx) {
    // get the heap stash
    duk_push_heap_stash(ctx); // [stash]

    // create prototype object
    duk_push_object(ctx); // [stash, prototype]
    
    // store in in a hidden symbol
    Upp::String cname = DUK_HIDDEN_SYMBOL("__class__");
    cname << typeid(C).name();
    duk_put_prop_string(ctx, -2, ~cname); // [stash]
    duk_pop(ctx); // []

    return true;
}

template<typename C>
void DukPushClassPrototype(duk_context *ctx) {
	duk_push_heap_stash(ctx); // [stash]
    Upp::String cname = DUK_HIDDEN_SYMBOL("__class__");
    cname << typeid(C).name();
    duk_get_prop_string(ctx, -1, ~cname); // [stash prototype]
    auto ty = duk_get_type(ctx, -1);
    ASSERT(ty == DUK_TYPE_OBJECT); // !!!NOTE!!! likely you failed to bind the class before use
    duk_swap_top(ctx, -2);  // [prototype stash]
    duk_pop(ctx); // [prototype]
}

template<typename C>
duk_ret_t DukFinalizer(duk_context *ctx) {
	// [object]
    ASSERT(duk_get_type(ctx, -1) == DUK_TYPE_OBJECT);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("thisptr")); // [object thisptr]
    auto ptr = (C*)duk_get_pointer(ctx, -1); // [object thisptr]
    delete ptr;
    return 0;
}

template<typename C>
void DukPushInstance(duk_context *ctx, C* obj) {
	duk_push_object(ctx); // [this]
    duk_push_pointer(ctx, (void*)obj); // [this, obj]
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("thisptr")); // [this]
    duk_push_c_function(ctx, DukFinalizer<C>, 1); // [this, finalizer]
    duk_set_finalizer(ctx, -2); // [this]
    DukPushClassPrototype<C>(ctx); // [this, prototype]
    duk_set_prototype(ctx, -2); // [this]
}

template<typename C>
void DukPush(duk_context *ctx, const C& c) {
	static_assert (std::is_class_v<C>);
	static_assert (std::is_trivial_v<C>);
	auto obj = new C; // need to copy it somewhere
	*obj = c;
	DukPushInstance<C>(ctx, obj);
}

template<typename C>
C* DukGetInstance(duk_context *ctx, int idx) {
	duk_get_prop_string(ctx, idx, DUK_HIDDEN_SYMBOL("thisptr")); // [thisptr]
	auto ptr = duk_get_pointer(ctx, -1); // [thisptr]
	duk_pop(ctx);
	return (C*)ptr;
}

template<typename C>
void DukGet(duk_context *ctx, int idx, C& out) {
	static_assert (std::is_class_v<C>);
	static_assert (std::is_trivial_v<C>);
	C* ptr = DukGetInstance<C>(ctx, idx);
    ASSERT(ptr != nullptr);
	out = *ptr;
}

// class constructor bindings

template<typename C, typename... Args, size_t... I>
C* DukCallConstructor(duk_context *ctx, std::index_sequence<I...>) {
	// [this]
    return new C(DukGetArg<Args>(ctx, I)...);
}

template<typename C, typename... Args>
duk_ret_t DukConstructor(duk_context *ctx) {
    if (!duk_is_constructor_call(ctx)) {
        return DUK_RET_TYPE_ERROR;
    }
    duk_push_this(ctx); // [this]
    auto newObj = DukCallConstructor<C, Args...>(ctx, std::index_sequence_for<Args...>{}); // [this]
    duk_push_pointer(ctx, (void*)newObj); // [this, newObj]
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("thisptr")); // [this]
    duk_push_c_function(ctx, DukFinalizer<C>, 1); // [this, finalizer]
    duk_set_finalizer(ctx, -2); // [this]
    return 0;
}

template<typename C, typename... Args>
bool DukBindConstructor(duk_context *ctx, const char* name) {
    if (!name || !*name)
        return false;

    duk_c_function cn = DukConstructor<C, Args...>;
    duk_push_c_function(ctx, cn, sizeof...(Args)); // [constructor]
	DukPushClassPrototype<C>(ctx); // [constructor, prototype]
    duk_put_prop_string(ctx, -2, "prototype"); // [constructor]
    duk_put_global_string(ctx, name); // []

    return true;
}

// class method bindings

template<typename C, typename Ret, typename... Args>
struct DukMethodStruct {
    Ret (C::*method)(Args...);
};

template<typename C, typename Ret, typename... Args, size_t... I>
Ret DukMethodCaller(duk_context *ctx, C* object, Ret (C::*method)(Args...), std::index_sequence<I...>) {
    return (object->*method)(DukGetArg<Args>(ctx, I)...);
}

template<typename C, typename Ret, typename... Args>
duk_ret_t DukMethodWrapper(duk_context *ctx) {
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("thisptr"));
    auto object = (C*)duk_get_pointer(ctx, -1);
    ASSERT(object != nullptr);
    duk_pop_n(ctx, 2);
    
    duk_push_current_function(ctx);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("methodobj"));
    size_t ptrlen;
    auto ptr = duk_get_lstring(ctx, -1, &ptrlen);
    ASSERT(ptr != nullptr);
    ASSERT(ptrlen == sizeof(DukMethodStruct<C,Ret,Args...>));
    auto ms = (DukMethodStruct<C,Ret,Args...>*)ptr;
    ASSERT(ms->method != nullptr);
    duk_pop(ctx);
    
    if constexpr (std::is_void<Ret>::value) {
        DukMethodCaller(ctx, object, ms->method, std::index_sequence_for<Args...>{});
        return 0;
    }
    else {
        Ret ret = DukMethodCaller(ctx, object, ms->method, std::index_sequence_for<Args...>{});
        DukPush(ctx, ret);
        return 1;
    }
}

template<typename C, typename Ret, typename... Args>
bool DukBindMethod(duk_context *ctx, Ret (C::*method)(Args...), const char* name) {
    // get prototype of C
    DukPushClassPrototype<C>(ctx); // [prototype]

    // push the duk_c_function method caller
    duk_c_function fn = DukMethodWrapper<C, Ret, Args...>;
    duk_push_c_function(ctx, fn, sizeof...(Args)); // [prototype wrapper]
    
    // store the method wrapper struct in a hidden lstring symbol of the c function
    DukMethodStruct<C,Ret,Args...> ms = { method };
    duk_push_lstring(ctx, (char*)&ms, sizeof(ms)); // [prototype wrapper methodBin]
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("methodobj")); // [prototype wrapper]
    
    // save the method in the class prototype
    duk_put_prop_string(ctx, -2, name); // [prototype]
    duk_pop(ctx); // []
    
    return true;
}

// js callable

template<typename Arg>
void DukPushAllArgs(duk_context *ctx, Arg arg) {
    DukPush(ctx, arg);
}
template<typename Arg, typename... Args>
void DukPushAllArgs(duk_context *ctx, Arg arg, Args... args) {
    DukPush(ctx, arg);
    DukPushAllArgs(ctx, args...);
}

template<typename Ret, typename... Args>
struct DukCallableWrapper : Upp::Moveable<DukCallableWrapper<Ret, Args...> > {
	duk_context *ctx;
	void* heapptr;
	Ret operator()(Args... args) {
		duk_push_heapptr(ctx, heapptr);
        DukPushAllArgs(ctx, args...);
        auto st = duk_pcall(ctx, sizeof...(Args));
        ASSERT(st == DUK_EXEC_SUCCESS); // fixme . return value or exception? maybe let the user select with a different template or a flag or something
		if constexpr (std::is_void<Ret>::value)
			duk_pop(ctx);
		else {
			Ret r = DukGetArg<Ret>(ctx, -1);
			duk_pop(ctx);
			return r;
		}
	}
};

template<typename Ret, typename... Args>
DukCallableWrapper<Ret, Args...> DukCallableFromJs(duk_context *ctx, int idx) {
    DukCallableWrapper<Ret, Args...> ret;
    ret.ctx = ctx;
    ret.heapptr = duk_get_heapptr(ctx, idx);
    return pick(ret);
}

template<typename Ret, typename... Args>
DukCallableWrapper<Ret, Args...> DukGetFunction(duk_context *ctx, const char* name) {
    duk_get_global_string(ctx, name);
    return DukCallableFromJs<Ret, Args...>(ctx, -1);
}

// context wrapper

struct Duktape
{
    Upp::Value Eval(const char* s) const;
    Upp::Value Eval(const char* s, int len) const;
    Upp::Value Eval(const Upp::String& s) const;
    Upp::Value Eval0() const;
    
    void PrintStack();
    
    void Fatal(const char* msg);
    
    operator duk_context *() const  { return ctx; }
    duk_context* operator~ () const { return ctx; }
    
    bool BindGlobal(duk_c_function dukfunc, int nparams, const char* name, const char* container = nullptr);
    
    template<typename T>
    bool BindGlobal(const T& value, const char* name) {
        return DukBindGlobal(ctx, value, name);
    }
    template<typename T>
    bool BindGlobalField(const T& value, const char* name, const char* container) {
        return DukBindGlobal(ctx, value, name, container);
    }
    
    template<typename C>
    bool BindClass() {
        return DukBindClass<C>(ctx);
    }
    template<typename C, typename... Args>
    bool BindConstructor(const char* name) {
        return DukBindConstructor<C, Args...>(ctx, name);
    }
    template<typename M>
    bool BindMethod(M method, const char* name) {
        return DukBindMethod(ctx, method, name);
    }
    
    template<typename T>
    void Push(const T& value) const { DukPush(ctx, value); }
    template<typename T>
    T To(int idx) const             { T t; DukGet(ctx, idx, t); return t; }
    
    Duktape();
    virtual ~Duktape();
    
protected:
    duk_context *ctx = nullptr;
};

#endif
