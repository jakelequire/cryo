
# The Cryo Language Overview

The Cryo language is still under heavy development, and as such, this document is subject to change. The language is designed to be a high-level, statically-typed language with a focus on simplicity and readability. The language is designed to be easy to learn and use, with a syntax that is similar to other popular languages such as Python and JavaScript with the capabilities of languages like C++ and Rust.

## Primitive Types

Cryo has a number of primitive types that are used to represent basic data types. These types include:

- `int`: A 32-bit signed integer.
- `i8`: An 8-bit signed integer.
- `i16`: A 16-bit signed integer.
- `i32`: A 32-bit signed integer.
- `i64`: A 64-bit signed integer.

- `float`: A 32-bit floating point number.
- `boolean`: A boolean value that can be either `true` or `false`.
- `string`: A string of characters.
- `void`: A type that represents the absence of a value.
- `null`: A type that represents the absence of a value.
- `any`: A type that represents any value (similar to `void*` in C, still under development).

## Variables

Variables in Cryo are declared using the `const` keyword. Variables are required to have a type annotation, and the type of the variable cannot be changed once it has been declared. Variables can be initialized with a value at the time of declaration, or they can be declared without a value and initialized later.

```c
const x: int = 10;
const z: string = "Hello, world!";
```

## Functions

Functions in Cryo are declared using the `function` keyword. Functions can take zero or more arguments, and they can return a value. Functions can be declared with a return type, or they can be declared with the `void` return type if they do not return a value.

A Cryo program must have a `main` function that serves as the entry point of the program. It may return void or an integer. The `main` function can *optionally* take `argc: int` and `argv: string[]` as arguments.

```c
namespace Main; // This is required for any cryo file.

function add(x: int, y: int) -> int {
    return x + y;
}

function main(argc: int, argv: string[]) -> int {
    const result: int = add(10, 20);
    return result;
}
```

Functions can also have a visibility modifier, which can be `public` or `private`. By default, functions are `public`. Public functions can be accessed from outside the modules namespace, while private functions can only be accessed from within the modules namespace.

```c
namespace Main;

public function add(x: int, y: int) -> int {
    return x + y;
}

private function subtract(x: int, y: int) -> int {
    return x - y;
}

function main(argc: int, argv: string[]) -> int {
    const result: int = add(10, 20);
    return result;
}
```

## Control Structures

Cryo has a number of control structures that can be used to control the flow of a program. These control structures include:

- `if` statements: Used to execute a block of code if a condition is true.
- `else` statements: Used to execute a block of code if a condition is false.
- `while` loops: Used to execute a block of code while a condition is true.
- `for` loops: Used to execute a block of code a fixed number of times (still under development).

```c
namespace Main;

function loop() -> int {
    mut x: int = 0;
    while (x < 10) {
        x = x + 1;
    }
    return x;
}

function main(argc: int, argv: string[]) -> int {
    if (argc > 1) {
        return 1;
    } else {
        loop();
        return 0;
    }
}
```

## Structs

The Cryo Struct is a user-defined data type that can contain multiple fields. Structs are declared using the `struct` keyword, and they can be used to group related data together.

Structs work similarly to C++ structs. They can have fields, methods, and constructors.

```c
namespace Main;

struct Point {
    x: int;
    y: int;

    constructor(x: int, y: int) {
        this.x = x;
        this.y = y;
    }

    add(other: Point) -> Point {
        const xVal = this.x + other.x;
        const yVal = this.y + other.y;
        return new Point(xVal, yVal);
    }
}

function main(argc: int, argv: string[]) -> int {
    const p1: Point = new Point(1, 2);
    const p2: Point = new Point(3, 4);
    const p3: Point = p1.add(p2);

    const result: int = p3.x + p3.y;
    printInt(result);

    return 0;
}
```

## Classes

Classes in Cryo are similar to structs, but they are not public by default. Classes can have fields, methods, and constructors, and they can be used to group related data and functionality together. Class fields and methods must be declared with a visibility modifier, which can be `public`,`private`, or `protected`.


```c
namespace Main;

class Point {
    public x: int;
    public y: int;

    constructor(x: int, y: int) {
        this.x = x;
        this.y = y;
    }

    public add(other: Point) -> Point {
        const xVal = this.x + other.x;
        const yVal = this.y + other.y;
        return new Point(xVal, yVal);
    }
}

```

Classes also have support for `static` fields and methods, which can be accessed without creating an instance of the class.

```c
namespace Main;

class Logger {
    public static log(message: string) {
        printString(message);
    }
}

function main(argc: int, argv: string[]) -> int {

    Logger::log("Hello, world!");
    return 0;
}

// Note: The `::` operator is used to access static fields and methods of a class.
```

## Built-in Functions

Cryo is starting to build up a library of built-in functions that can be used to perform common tasks. These functions are available by default and do not need to be imported. Some of the built-in functions include:

- `printInt`: Used to print an integer to the console.
- `printString`: Used to print a string to the console.
- `sys_exit`: Used to exit the program with a status code.
- `intToString`: Used to convert an integer to a string.

```c
// This is the runtime file that contains the built-in functions. (<CRYO_PATH>/cryo/runtime.cryo)
namespace Runtime;

extern function __c_printInt(i: int) -> void;
extern function __c_printStr(s: string) -> void;
extern function __c_sys_exit(code: int) -> void;
extern function __c_intToString(num: int) -> string;

function sys_exit(code: int) -> void {
    __c_sys_exit(code);
    return;
}

function printInt(i: int) -> void {
    __c_printInt(i);
    return;
}

function printStr(s: string) -> void {
    __c_printStr(s);
    return;
}

function intToString(i: int) -> string {
    return __c_intToString(i);
}
```

The runtime functions are implemented in C and are linked to the Cryo runtime. The runtime functions are declared in the `Runtime` namespace and are available to all Cryo programs by default. The runtime functions are used to perform low-level operations that are not possible in Cryo itself in its current state. This is something that will be improved in the future.
