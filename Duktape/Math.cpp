#include "Duktape.h"

#include <cmath>

namespace {

// following are macros so we have to wrap them for now at least

int fpclassify_wrap(double x) {
    return fpclassify(x);
}
bool isfinite_wrap(double x) {
    return isfinite(x);
}
bool isinf_wrap(double x) {
    return isinf(x);
}
bool isnan_wrap(double x) {
    return isnan(x);
}
bool isnormal_wrap(double x) {
    return isnormal(x);
}
bool signbit_wrap(double x) {
    return signbit(x);
}

bool isgreater_wrap(double a, double b) {
    return isgreater(a, b);
}
bool isgreaterequal_wrap(double a, double b) {
    return isgreaterequal(a, b);
}
bool isless_wrap(double a, double b) {
    return isless(a, b);
}
bool islessequal_wrap(double a, double b) {
    return islessequal(a, b);
}
bool islessgreater_wrap(double a, double b) {
    return islessgreater(a, b);
}
bool isunordered_wrap(double a, double b) {
    return isunordered(a, b);
}

// can't map the pointer parameter right now
// and ecmascript 5.1 does not have multiple return values so, meh

duk_ret_t remquo_wrap(duk_context *ctx) {
    if (duk_get_top(ctx) != 2) {
        duk_push_undefined(ctx);
        return 1;
    }
    
    double x = duk_get_number(ctx, 0);
    double y = duk_get_number(ctx, 1);
    int quo;
    double r = remquo(x,y,&quo);
    
    int i = duk_push_array(ctx);
    duk_push_number(ctx, r);
    duk_put_prop_index(ctx, i, 0);
    duk_push_int(ctx, quo);
    duk_put_prop_index(ctx, i, 1);
    
    return 1;
}

duk_ret_t frexp_wrap(duk_context *ctx) {
    if (duk_get_top(ctx) != 1) {
        duk_push_undefined(ctx);
        return 1;
    }
    
    double x = duk_get_number(ctx, 0);
    int exp;
    double a = frexp(x,&exp);
    
    int i = duk_push_array(ctx);
    duk_push_number(ctx, a);
    duk_put_prop_index(ctx, i, 0);
    duk_push_int(ctx, exp);
    duk_put_prop_index(ctx, i, 1);
    
    return 1;
}

duk_ret_t modf_wrap(duk_context *ctx) {
    if (duk_get_top(ctx) != 1) {
        duk_push_undefined(ctx);
        return 1;
    }
    
    double arg = duk_get_number(ctx, 0);
    double intg;
    double frac = modf(arg,&intg);
    
    int i = duk_push_array(ctx);
    duk_push_number(ctx, frac);
    duk_put_prop_index(ctx, i, 0);
    duk_push_number(ctx, intg);
    duk_put_prop_index(ctx, i, 1);
    
    return 1;
}

} // namespace

void DuktapeRegisterCMath(Duktape& duk, const char* container) {
    bool ok = true;
    
    //ok &= duk.BindGlobalField<double(*)(double)>(fabs, "abs", container);
    ok &= duk.BindGlobalField<double(*)(double,double)>(fmod, "mod", container);
    ok &= duk.BindGlobalField<double(*)(double,double)>(remainder, "remainder", container);
    ok &= duk.BindGlobal(remquo_wrap, 2, "remquo", container);
    ok &= duk.BindGlobalField<double(*)(double,double,double)>(fma, "ma", container);
    //ok &= duk.BindGlobalField<double(*)(double,double)>(fmax, "max", container);
    //ok &= duk.BindGlobalField<double(*)(double,double)>(fmin, "min", container);
    ok &= duk.BindGlobalField<double(*)(double,double)>(fdim, "dim", container);
    //ok &= duk.BindGlobalField(nan, "nan", container);
    
    //ok &= duk.BindGlobalField<double(*)(double)>(exp, "exp", container);
    ok &= duk.BindGlobalField<double(*)(double)>(exp2, "exp2", container);
    ok &= duk.BindGlobalField<double(*)(double)>(expm1, "expm1", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(log, "log", container);
    ok &= duk.BindGlobalField<double(*)(double)>(log10, "log10", container);
    ok &= duk.BindGlobalField<double(*)(double)>(log2, "log2", container);
    ok &= duk.BindGlobalField<double(*)(double)>(log1p, "log1p", container);
    
    //ok &= duk.BindGlobalField<double(*)(double,double)>(pow, "pow", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(sqrt, "sqrt", container);
    ok &= duk.BindGlobalField<double(*)(double)>(cbrt, "cbrt", container);
    ok &= duk.BindGlobalField<double(*)(double,double)>(hypot, "hypot", container);
    
    //ok &= duk.BindGlobalField<double(*)(double)>(sin, "sin", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(cos, "cos", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(tan, "tan", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(asin, "asin", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(acos, "acos", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(atan, "atan", container);
    //ok &= duk.BindGlobalField<double(*)(double,double)>(atan2, "atan2", container);
    
    ok &= duk.BindGlobalField<double(*)(double)>(sinh, "sinh", container);
    ok &= duk.BindGlobalField<double(*)(double)>(cosh, "cosh", container);
    ok &= duk.BindGlobalField<double(*)(double)>(tanh, "tanh", container);
    ok &= duk.BindGlobalField<double(*)(double)>(asinh, "asinh", container);
    ok &= duk.BindGlobalField<double(*)(double)>(acosh, "acosh", container);
    ok &= duk.BindGlobalField<double(*)(double)>(atanh, "atanh", container);
    
    ok &= duk.BindGlobalField<double(*)(double)>(erf, "erf", container);
    ok &= duk.BindGlobalField<double(*)(double)>(erfc, "erfc", container);
    ok &= duk.BindGlobalField<double(*)(double)>(tgamma, "tgamma", container);
    ok &= duk.BindGlobalField<double(*)(double)>(lgamma, "lgamma", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(ceil, "ceil", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(floor, "floor", container);
    ok &= duk.BindGlobalField<double(*)(double)>(trunc, "trunc", container);
    //ok &= duk.BindGlobalField<double(*)(double)>(round, "round", container);
    ok &= duk.BindGlobalField<double(*)(double)>(nearbyint, "nearbyint", container);
    ok &= duk.BindGlobalField<double(*)(double)>(rint, "rint", container);
    
    ok &= duk.BindGlobal(frexp_wrap, 1, "frexp", container);
    ok &= duk.BindGlobalField<double(*)(double,int)>(ldexp, "ldexp", container);
    ok &= duk.BindGlobal(modf_wrap, 1, "modf", container);
    ok &= duk.BindGlobalField<double(*)(double,int)>(scalbn, "scalbn", container);
    ok &= duk.BindGlobalField<int(*)(double)>(ilogb, "ilogb", container);
    ok &= duk.BindGlobalField<double(*)(double)>(logb, "logb", container);
    ok &= duk.BindGlobalField<double(*)(double,double)>(nextafter, "nextafter", container);
    ok &= duk.BindGlobalField<double(*)(double,long double)>(nexttoward, "nexttoward", container);
    ok &= duk.BindGlobalField<double(*)(double,double)>(copysign, "copysign", container);
    
    ok &= duk.BindGlobalField(fpclassify_wrap, "fpclassify", container);
    ok &= duk.BindGlobalField(isfinite_wrap, "isfinite", container);
    ok &= duk.BindGlobalField(isinf_wrap, "isinf", container);
    ok &= duk.BindGlobalField(isnan_wrap, "isnan", container);
    ok &= duk.BindGlobalField(isnormal_wrap, "isnormal", container);
    ok &= duk.BindGlobalField(signbit_wrap, "signbit", container);
    ok &= duk.BindGlobalField(isgreater_wrap, "isgreater", container);
    ok &= duk.BindGlobalField(isgreaterequal_wrap, "isgreaterequal", container);
    ok &= duk.BindGlobalField(isless_wrap, "isless", container);
    ok &= duk.BindGlobalField(islessequal_wrap, "islessequal", container);
    ok &= duk.BindGlobalField(islessgreater_wrap, "islessgreater", container);
    ok &= duk.BindGlobalField(isunordered_wrap, "isunordered", container);
    
    //ok &= duk.BindGlobalField(HUGE_VAL, "HUGE_VAL", container);
    
    // upp math constants
    ok &= duk.BindGlobalField(2.71828182845904523536,     "E",        container);
    ok &= duk.BindGlobalField(1.44269504088896340736,     "LOG2E",    container);
    ok &= duk.BindGlobalField(0.434294481903251827651,    "LOG10E",   container);
    ok &= duk.BindGlobalField(0.693147180559945309417,    "LN2",      container);
    ok &= duk.BindGlobalField(2.30258509299404568402,     "LN10",     container);
    //ok &= duk.BindGlobalField(3.14159265358979323846,     "PI",       container);
    ok &= duk.BindGlobalField(1.57079632679489661923,     "PI_2",     container);
    ok &= duk.BindGlobalField(0.785398163397448309616,    "PI_4",     container);
    //ok &= duk.BindGlobalField(0.318309886183790671538,    "1_PI",     container);
    //ok &= duk.BindGlobalField(0.636619772367581343076,    "2_PI",     container);
    //ok &= duk.BindGlobalField(0.564189583547756286948,    "1_SQRTPI", container);
    //ok &= duk.BindGlobalField(1.12837916709551257390,     "2_SQRTPI", container);
    //ok &= duk.BindGlobalField(1.41421356237309504880,     "SQRT2",    container);
    //ok &= duk.BindGlobalField(0.707106781186547524401,    "SQRT_2",   container);
}
