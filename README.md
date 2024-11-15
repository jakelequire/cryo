# <img src="./assets/cryo-logo.svg" width="90" height="90" alt="Cryo Logo" align="center"> Cryo

General Purpose, Strongly Typed, OOP, Programming Language witten in C & C++.

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0)]
[![GitHub issues](https://img.shields.io/github/issues/jakelequire/cryo)]
[![GitHub forks](https://img.shields.io/github/forks/jakelequire/cryo)]
[![GitHub stars](https://img.shields.io/github/stars/jakelequire/cryo)]
[![Version](https://img.shields.io/badge/Version-0.0.1-blue)]

This is a fun side Project maintained and developed by me, [Jake LeQuire](https://github.com/jakelequire). The goal of this project is to create a general purpose programming language that is simple and easy to use. This project is not meant to be a serious project and is just for fun.

I started this project to learn more about compilers and programming languages.

## Development Milestones

- [x] Compiles to native instruction set

- [ ] [Turing Complete.](https://en.wikipedia.org/wiki/Rule_110)

- [ ] Standard Library Created.

- [ ] Optimized

- [ ] [Self-hosted.](<https://en.wikipedia.org/wiki/Self-hosting_(compilers)>) (Later Goal)

## Table of Contents

- [Language Introduction](#language-introduction)
  - [Keywords / Reserve Words / Data Types](#keywords--reserve-words--data-types)
  - [Operators](#operators)
  - [Hello, world!](#hello-world)
  - [Fibonacci Sequence](#fibonacci-sequence)
  - [Structures](#structures)
- [Compile & Run](#compile--run)
  - [Dependencies](#dependencies)
  - [Building](#building)
  - [Environment Variables](#environment-variables)
  - [Using the compiler](#using-the-compiler)
  - [LSP](#lsp)
- [Conclusion](#conclusion)
- [License](#license)

## Language Introduction

This will be a basic overview of the language and its features. This will be updated as the language is developed. As of now, the language is still in the early stages of development and is not fully functional. However, there are some working features that I will outline below. 

**<u><i>Note: This will only work and compile on a Debian-Linux based system.**</u></i>

### <u>Keywords / Reserve Words / Data Types</u>

| Keywords      |          | Data Types | Reserved Words |
| --------      | -------- | ---------- | -------------- |
| `extern`      | `struct` | `int`      | `if`           |
| `function`    | `return` | `string`   | `else`         |
| `const`       | `true`   | `boolean`  | `while`        |
| `mut`         | `false`  | `float`    | `for`          |
| `namespace`   | `public` | `void`     |                |
| `private`     |          |            |                |


### <u>Operators</u>
| Arithmetic Operators | Logical Operators | Comparison Operators | Assignment Operators |
| -------------------- | ----------------- | -------------------- | -------------------- |
| `+`                  | `&&` - AND        | `==` - Strict EQ     | `=`                  |
| `-`                  | `\|\|` - OR       | `!=` - NOT EQ        | `+=`                 |
| `*`                  | `!` - NOT         | `>`  - GT            | `-=`                 |
| `/`                  |                   | `<`  - LT            | `*=`                 |
| `%`                  |                   | `>=` - GTE           | `/=`                 |
|                      |                   | `<=` - LTE           | `%=`                 |

### <u>Hello, world!</u>
Since there is no standard library yet, the language is very limited in what it can do. When the project builds, it will use a C file to borrow functions from. This is located in [`./src/cryo/std.c`](./src/cryo/std.c).

Here is a simple hello world program in Cryo:

```cpp
namespace Main;

extern function printStr(str: string) -> void;

function main() -> void {
    printStr("Hello, world!");

    const example: string = "Hello, world!";
    printStr(example);
    
    return;
}
```

This will print `Hello, world!` to the console.

### <u>Fibonacci Sequence</u>
Here is a simple program that will print the first 16 numbers of the Fibonacci sequence.

```cpp
namespace Fibonacci;

extern function printStr(str: string) -> void;
extern function printInt(num: int) -> void;

function fibonacci(n: int) -> void {
    mut a: int = 0;
    mut b: int = 1;
    mut c: int = 0;

    while (n > 0) {
        c = a + b;
        a = b;
        b = c;

        printInt(a);
        n = n - 1;
    }

    return;
}


function main() -> void {
    fibonacci(16);
    return;
}
```

### <u>Structures</u>
Still early in development, but this is the early implementation of structures in Cryo.

```cpp
namespace Structures;

extern function intToStr(num: int) -> string;
extern function printStr(str: string) -> void;

struct Int {
    value: int;

    constructor(val: int) {
        this.value = val;
    }

    toString() -> string {
        return intToStr(this.value);
    }
}

function main() -> void {
    const num: Int = 10;
    printStr(num.toString());
    return;
}
```

**<u><i>
Note: 
There must be a `main` function in the program in order to execute.
</i></u>**

**<u><i>
`argc` and `argv` are not yet implemented.
</i></u>**

## Compile & Run
There are some dependencies that you will need to have installed on your system to compile the language.

#### Dependencies
- [Clang](https://clang.llvm.org/)
- [Make](https://www.gnu.org/software/make/)
- [LLVM 18](https://llvm.org/)


To compile the language, you will need to have `clang` and `make` installed on your system. You can install it by running:
```bash
sudo apt-get install clang make
```

For the LLVM library might be a little more difficult to install. You can follow the instructions on the [LLVM website](https://llvm.org/) to install it on your system. The makefile should be able to find the library if it is installed correctly.

#### Building

To compile the project, you will first need to build it. You can do this by running:
```bash
make all
```
This will build the source code of the project and create an executable file called `compiler` in [`./bin/`](./bin/).

#### Environment Variables
There is only one environmental variable you need to set in order to run the program. This is something that I will fix in the future, but for now, you will need to set the `CRYO_PATH` variable for the program to work. You can set this by running:
```bash
export CRYO_PATH=<path-to-cryo-directory>
```

#### Using the compiler
There is a build script in the project directory that will compile and execute a Cryo file. As of right now, it can only compile one file at a time. You can run the build script by running:
```bash
./build.sh -f <file>
```

### LSP
There is a small LSP within the project under the [`./assets/`](./assets/) directory. This is a VS Code Extension that will provide syntax highlighting for the Cryo language. You can just double click the `.vsix` file to install it in VS Code.

## Conclusion

Once again, this is just a side project that I wanted to build for fun and to learn new things. The only real goal of this project is to have a very minimalistic general purpose programming language. I have no intention on a serious roadmap and just adding things as I see fit.

### License

This project is licensed under the Apache 2.0 License - see the [Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0) for details.
