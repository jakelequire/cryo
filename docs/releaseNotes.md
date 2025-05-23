# Cryo Release Notes

## Table of Contents
- [v0.0.2](#v002)
- [v0.0.1](#v001)

---

## v0.0.2
This is the second release of Cryo. This version of the compiler is still in its early stages, but it has made significant progress since the first release.
This version of the compiler has a more advanced Type System, improved Code Generation, and a more advanced Parser.

### Features
- **Improved Type System** - Enhanced type checking and inference capabilities
- **Improved Code Generation** - More efficient LLVM IR generation
- **Object Oriented Programming Support** - Classes, inheritance, and constructors _(early stages)_
- **Multi-file Support** - Import and compile multiple `.cryo` files
- **Scope Management** - Proper variable scoping across functions and blocks
- **Enhanced Error Handling** - Better error messages and recovery
- **Cryo CLI** - CLI for compilation and project management, try `cryo --help` for more information
- **C Standard Library Integration** - Access to standard C functions via `extern`

### New Keywords
- `class` - Used to define a Class.
- `type` - Used to define a type alias or a new type.
- `extends` - Used to define inheritance.
- `this` - Used to refer to the current instance of an Object.
- `import` - Used to import a module or a file.
- `static` - Used to define a static member of an Object.
- `implement` - Used to define an implementation of an interface.
- `constructor` - Used to define a constructor for a Class or Struct.


### New Operators
- `&` - Used to define a reference to a variable. This is similar to the `&` operator in C/C++.
- `*` - Used to define a pointer to a variable. This is similar to the `*` operator in C/C++.

### Improvements
- `extern` - Used to define an external function or variable. It may be used to define a single function or variable, or a list of functions or variables. The syntax is as follows:
```c
// Single function
extern function printf(__str__: str, ...) -> i32;

// List of functions 
// Note: "C" is not a keyword in Cryo, but is used here to indicate that the functions are from the C standard library, which is the only standard library supported by Cryo
extern "C" {
    function printf(__str__: str, ...) -> i32;
    function scanf(__str__: str, ...) -> i32;
}
```
- `Pointers` - Pointers are now supported in Cryo. Pointers are used to reference a variable or an object. The syntax is as follows:
```c
// Pointer to a variable
const x: i32 = 10;
const y: i64 * = &x;
const z: i32 = *y;
```

### Known Issues
- Object Oriented Programming is still in its early stages and is not fully implemented.
- The Type System has been improved, but there are still issues with type checking and type inference.
- Code Generation can produce invalid IR due to the complexity & limitations of the Type System.
- The Semantic  Analyzer is still in its very early stages and may produce false positives or false negatives.

### Examples
See the [examples directory](../examples/) for sample programs demonstrating new features:
- [Fibonacci](../examples/fibonacci.cryo)
- [Arrays](../examples/array.cryo)
- [Classes](../examples/class.cryo)
- [Structs](../examples/struct.cryo)

---

## v0.0.1
This is the first release of Cryo. This version of the compiler is in its most primitive and basic form. Many of the components of the compiler in this version will be replaced in future versions.

### Features
- Basic parsing of the Cryo language
- Single File Code Generation

### Known Issues
- No support for multiple files
- No Error Handling
- Only supports a single type of variable
