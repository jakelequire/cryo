# <img src="./assets/cryo-logo.svg" width="90" height="90" alt="Cryo Logo" align="center"> Cryo

General Purpose, Strongly Typed, OOP, Programming Language witten in C & C++.

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0)]
[![GitHub stars](https://img.shields.io/github/stars/jakelequire/cryo)]
[![Version](https://img.shields.io/badge/Version-0.0.1-blue)]

This is a fun side Project maintained and developed by me, [Jake LeQuire](https://github.com/jakelequire). The goal of this project is to create a general purpose programming language that is simple and easy to use. This project is not meant to be a serious project and is just for fun.

I started this project to learn more about compilers and programming languages.

## Development Milestones

- [x] Compiles to native instruction set

- [X] [Turing Complete.](https://en.wikipedia.org/wiki/Rule_110)

- [X] Standard Library Created.

- [ ] Optimized

- [ ] [Self-hosted.](<https://en.wikipedia.org/wiki/Self-hosting_(compilers)>) (Later Goal)


## Table of Contents

- [Installation](#installation)
  - [Dependencies](#dependencies)
  - [Compiling with Cryo](#compiling-with-cryo)
  - [Using the CLI](#using-the-cli)
- [Language Introduction](#language-introduction)
  - [Keywords / Reserve Words / Data Types](#keywords--reserve-words--data-types)
  - [Operators](#operators)
  - [Hello, world!](#hello-world)
  - [Fibonacci Sequence](#fibonacci-sequence)
  - [Structures](#structures)
- [Conclusion](#conclusion)
- [License](#license)


## Installation
### Dependencies
- [Clang](https://clang.llvm.org/)
- [Make](https://www.gnu.org/software/make/)
- [LLVM 18](https://llvm.org/)

In the repository, there is a `install.sh` file. This will install the required dependencies and build the project. You can run this by running:
```bash
./install.sh
```
Note: This will only work on a Debian-Linux based system. Sudo permissions are required to install the dependencies.

### Compiling with Cryo
After running the `install.sh` file, the compiler has been build and the CLI tool is installed on your system. With the CLI tool, you can navigate to an empty directory and run the following command to create a new Cryo project:
```bash
cryo init
```

This will create the `main.cryo` file under the `src/` directory as wel as the `cryoconfig` file in the root directory. You can then run the following command to compile the project:
```bash
cryo build
```

### Using the CLI

There are some helpful commands in the CLI that can be used. You can run the following command to see the help menu:
```bash
cryo help
```

If you want to display more information about a specific command, you can run:
```bash
cryo help <command>
```

## Language Introduction

This will be a basic overview of the language and its features. This will be updated as the language is developed. As of now, the language is still in the early stages of development and is not fully functional. However, there are some working features that I will outline below. 

**<u><i>Note: This will only work and compile on a Debian-Linux based system.**</u></i>

### <u>Keywords / Reserve Words / Data Types</u>

| Keywords      |          |             | Primitive Types | Reserved Words |
| --------      | -------- |------------ | --------------- | -------------- |
| `extern`      | `struct` | `using`     | `i8...i128`     | `if`           |
| `function`    | `return` | `module`    | `str`           | `else`         |
| `const`       | `true`   | `implement` | `boolean`       | `while`        |
| `mut`         | `false`  | `extends`   | `float`         | `for`          |
| `namespace`   | `public` | `type`      | `void`          | `do`           |
| `private`     | `class`  |             |                 |                |
| `constructor` | `static` |             |                 |                |
| `this`        | `new`    |             |                 |                |


### <u>Operators</u>
| Arithmetic | Logical      | Comparison           | Assignment  | Unary                |
| -----------| -------------| -------------------- | ------------| -------------------- |
| `+`        | `&&` - AND   | `==` - Strict EQ     | `=`         | `&` - Address Of     |
| `-`        | `\|\|` - OR  | `!=` - NOT EQ        | `+=`        | `*` - Deref (expr)   |
| `*`        | `!` - NOT    | `>`  - GT            | `-=`        | `*` - Pointer (type) |
| `/`        |              | `<`  - LT            | `*=`        | 
| `%`        |              | `>=` - GTE           | `/=`        |
|            |              | `<=` - LTE           | `%=`        |

### <u>Hello, world!</u>

Here is a simple hello world program in Cryo:

```cpp
namespace Main;

function main() -> void {
    printf("Hello, world!");

    const example: str = "Hello, world!";
    printf("%s", example);
    
    return;
}
```

This will print `Hello, world!` to the console.

### <u>Fibonacci Sequence</u>
Here is a simple program that will print the first 16 numbers of the Fibonacci sequence.

```cpp
namespace Fibonacci;

function fibonacci(n: int) -> void {
    mut a: int = 0;
    mut b: int = 1;
    mut c: int = 0;

    while (n > 0) {
        c = a + b;
        a = b;
        b = c;
        printf("%d ", c);
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
    printf("%s", num.toString());
    return;
}
```

**<u><i>
Note: 
There must be a `main` function in the program in order to execute.
Also for Cryo projects, the `main.cryo` file must be in the `src/` directory.
</i></u>**

### LSP
There is a small LSP within the project under the [`./assets/`](./assets/) directory. This is a VS Code Extension that will provide syntax highlighting for the Cryo language. You can just double click the `.vsix` file to install it in VS Code.

## Conclusion

Once again, this is just a side project that I wanted to build for fun and to learn new things. The only real goal of this project is to have a very minimalistic general purpose programming language. I have no intention on a serious roadmap and just adding things as I see fit.

To view more about the language, you can view the language overview [here](./docs/lang-overview.md).

### License

This project is licensed under the Apache 2.0 License - see the [Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0) for details.
