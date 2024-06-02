# Cryo
General Purpose, Strongly Typed, OOP, programming language witten in C.

<i>[Apache 2.0 License](#license)</i>

This is a fun side Project maintained and developed by me, [Jake LeQuire](https://github.com/jakelequire). Currently it is closed for PRs but when I have a more solid foundation of the project, I plan to open source the project completely.

___Note: This is a side project and not a production ready product. There may be several critical issues, please be advised.___

### Development Milestones


- [ ] Compiles to native instruction set (only x86_64 for now)

- [ ] [Turing Complete.](https://en.wikipedia.org/wiki/Rule_110)

- [ ] Standard Library Created.

- [ ] Optimized

- [ ] [Self-hosted.](https://en.wikipedia.org/wiki/Self-hosting_(compilers)) (Later Goal)

## Table of Contents

- [Getting Started](#getting-started)
    - [Installing the project.](#1-installing-the-project)
    - [Building The code.](#2-building-the-code)
    - [Using the Cryo Compiler.](#3-using-the-cryo-compiler)
- [Basic Project Structure](#basic-project-structure)
    - [Initialize a project](#initialize-a-project)

- [Language Introduction](#language-introduction)
    - [Keywords / Reserve Words](#keywords--reserve-words)
    - [Data Types](#data-types)
    - [Operators](#operators)
---

### Getting Started
There are a few dependencies that are needed to compile Cryo that you will need on your machine.

- make _(this project does not use cmake)_
- clang
- LLVM _(This will soon not be required to be a depenecy of the user)_

_Note: This has been developed on Windows and only has support for windows APIs internally, cross platform will be supported in the future._

#### 1. Installing the project.
However you prefer to install GitHub repos.

__Github CLI__
```sh
gh repo clone jakelequire/cryo
```
__HTTPS__
```sh
https://github.com/jakelequire/cryo.git
```

#### 2. Building The code.
The makefile within the project will take care of the compilation of the project, simply just type in the terminal:
```sh
make all
```
___Note:__ There may be warnings but the project should build and produce a `main.exe` file in `./src/bin`_


#### 3. Using the Cryo Compiler.
There are two ways you can compile a `.cryo` file. Manually passing it to the compiler (`main.exe`), and it will produce a single `.ll` file.

The __reccomended__ way to compile Cryo is to use the CLI provided within the repo. However, you will need to make sure your PATH Enviornmental Variables are able to view and execute the `cryo.exe` binary found within `./cli/bin/` in the project. To verify that the CLI is accessable, simply type in your terminal:
```sh
cryo -h
```
and the help menu should display.

---

### Basic Project Structure.

#### Initialize a project
With the CLI tools, you can initilize a barebone boilerplate project.
```sh
cryo --init
```
This will create a few things, this is what the basic outline looks like:

```
/cryo_project
|    /src
|    |  - main.cryo
|    - cryo.config.json
|    - cryo.init
```

There are a few required components in order to build a full project and not just a single file. Let's look at each one.

- `cryo.config.json` - This is the project configurations, dependencies, and compiler options.
- `cryo.init` - This is a file that shouldn't be modified and will automatically be updated. __This is for the compiler.__
- `/src` - The source code directory. All files / folders must be placed within here. Configurations and other external things may be played outside of the src directory.
- `main.cryo` - This is the entry point of the program. Having a main file & function is required. The main function may be passed `void` or `argv, argc` parameters.


When building a cryo project, you can do
```
cryo build ./
```
The compiler will look for the `cryo.init` file and set it's configurations. Next, it will compile your code to the root directory into a folder called `./build`. 

___Note: The build output will be a development build, in the future, a `release` flag will be avaliable and for an optimized production build.___

---

### Language Introduction
This will be a light overview of the language. There is additional resources that can be found to have a deeper look in the documentation of Cryo.

#### Keywords / Reserve Words

|            |            |            |            |            |            |
|------------|------------|------------|------------|------------|------------|
| any        | const      | if         | map        | return     | union      |
| array      | continue   | impl       | match      | self       | unsafe     |
| as         | else       | import     | module     | static     | use        |
| async      | enum       | instanceof | mut        | string     | volatile   |
| await      | extern     | int        | namespace  | struct     | while      |
| break      | false      | interface  | new        | super      |            |
| byte       | float      | let        | null       | true       |            |
| case       | fn         | long       | private    | type       |            |
| class      | for        | loop       | public     | typeof     |            |


#### Data Types
There is a set of basic data types in Cryo, in the future, there will be more to be implemented in the core data types.

- `int` -       /*TODO*/
- `string` -    /*TODO*/
- `boolean` -   /*TODO*/
- `symbol` -    /*TODO*/
- `void` -      /*TODO*/
- `null` -      /*TODO*/


#### Operators

- `+`  Addition operator.
- `-` Subtraction operator.
- `*` Multiplication operator.
- `/` Division operator.
- `%` Modulus operator.
- `==` Equality operator.
- `!=` Inequality operator.
- `>` Greater than operator.
- `<` Less than operator.
- `>=` Greater than or equal to operator.
- `<=` Less than or equal to operator.
- `&&` Logical AND operator.
- `||` Logical OR operator.
- `!` Logical NOT operator.



### Conclusion
Once again, this is just a side project that I wanted to build for fun and to learn new things. The only real goal of this project is to have a very minimalistic general purpose programming language. I have no intention on a serious roadmap and just adding things as I see fit.




### License
This project is licensed under the Apache 2.0 License - see the [Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0) for details.