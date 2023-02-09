# Duktape for U++
This repository contains a set of C++ template bindings, which allow you to use Duktape in your U++ applications.

Duktape is a lightweight embeddable Javascript engine, with a focus on portability and compact footprint.

Support for U++ specifics like `String` and `Value` types is provided, as well as a set of templates for generating bindings.

Everything you see here is an early work in progress, and the API is subject to change. Feedback and help of any kind is also very much appreciated - I'm sure the templates can be improved alot.

## Examples
See the `examples` and `autotest` directories for examples of how to use the bindings. 

The `autotest` directory contains a set of U++ autotest style testcases.

## Package
The `Duktape` package has the following dependencies:
- `Core`
- `libduktape`

The `Duktape` package is set to link the `libduktape` library by default, assuming a system wide installation. At present, it has only been tested on MSYS2 Clang64. Likely, the flags will need to be adjusted for other platforms and/or custom Duktape builds.

## Features
- Support for basic types
- Basic support for `String` and `Value` types
- Binding global functions
- Binding global values (not variables)
- Binding struct/class types
- Binding constructor
- Binding methods

## Missing features
- Binding C++ variables
- Inheritance
- Overloading
- Lots more :)

## Binding details
Functions bindings generate a wrapper Duktype C function, and store the original function pointer in a field `DUK_HIDDEN_SYMBOL("funcptr")` of the wrapper.

Class bindings generate a `prototype` object which is stored in the heap stash. For example for a class `C`, the `prototype` object is stored in the heap stash under the key:
```cpp
String key;
key << DUK_HIDDEN_SYMBOL("__class__");
key << typeid(C).name()
```

The `this` pointer is stored in the object under `DUK_HIDDEN_SYMBOL("thisptr")`.

Class methods are stored in a hidden field their corresponding C function, under `DUK_HIDDEN_SYMBOL("methodobj")`. The value is the method pointer itself, saved in a Duktype `lstring`.

The bindings provide a simple `print` function, which can be used to print to the console. It can be registered using the function `DuktapeRegisterPrint`. By default the output stream is `Cout()`. This can be changed by calling `DukPrintSetStream`.
The stream bound to `print` can be returned with `DukOut()`.

`print` take a variable number of arguments, and will print them separated by a space. The arguments are converted to strings using `duk_safe_to_string`.
