#include "Duktape.h"

using namespace Upp;

namespace {

Stream* outputStream = &Cout();

// print(value, ...)
duk_ret_t DukPrint(duk_context *ctx) {
    int N = duk_get_top(ctx);
    for (int i = 0; i < N; ++i) {
        const char* delim = (i > 0) ? " " : "";
        DukOut() << delim << duk_safe_to_string(ctx, i);
    }
    DukOut() << EOL;
    return 0;
}

} // namespace

void DuktapeRegisterPrint(Duktape& duk, const char* container) {
    duk.BindGlobal(DukPrint, DUK_VARARGS, "print", container);
}

void DukPrintSetStream(Stream& stream)
{
    outputStream = &stream;
}

Stream& DukOut() {
    ASSERT(outputStream != nullptr);
    return *outputStream;
}
