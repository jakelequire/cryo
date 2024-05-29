
## Table of Contents

- [Basic Syntax](#basic-syntax)\
        - [Creating a variable](#creating-a-variable)\
                - [Difference between const vs mut](#difference-between-const-vs-mut)\
        - [Creating a function](#creating-a-function)\
                - [Function Declaration](#function-declaration)\
                - [Function Methods](#function-methods)
                
- [Standard Library](#the-standard-library)\
        - [Standard Library Overview](#standard-library-overview)


## Basic Syntax
This will be an overview of the syntax of Cryo. How to write simple programs, the grammar, and more.
There are many of features missing and or incomplete. If anything, this is a draft of how the language will look in the future.


### Creating a variable
Creating a variable is simple and similar to other languages. Let's look at the variables `foo` and `bar` below:
```java //placeholder for syntax highlighting
const foo: int = 34;
mut bar: int = 35;
print(foo + bar);
```

Let's break it down. There are two ways you can make a variable, with the keyword `const` or `mut`. Comparatively, you can think of them like `const` and `let` in JavaScript.

Since Cryo is a _strongly typed_ language, every variable declaration needs to have a type. Let's look at the structure of `foo`:
```java
        const        foo      :   int         =           34;
//    <keyword>  <identifier>    <type>  <assignment>   {block|value}

// Note: the `:` is to declare a variables type.
// the `->` is the return type of a function or structure definition to a class.
```

#### Difference between `const` vs `mut`
A variable declaration can have two ways it holds it's value, the raw definition of a variable is as follows:
```
<var-declaration> ::= ("const" | "mut") <type> <identifier> ["=" <expression>] ";"
```

The compiler views both `const` and `mut` in the same way, but one is a <u>__constant__</u> reference to a variable pointer, and one is a <u>__muteable__</u> reference. Let's view a small example of how this works:
```cpp
// Here we simply add the two variables together.
const exampleOne: int = 2;
// Either `const` or `mut` will work fine here.
const exampleTwo: int = 3;
// We output the result to stdout.
print("{exampleOne + exampleTwo}");
// output: 5
```
Like mentioned above, using either `const` or `mut` will compile just fine. However, if we try to reassign one of the variables if it's a `const`, we will get a <u>compile-time</u> error.
```cpp
const exampleOne: int = 2;
const exampleTwo: int = 3;

exampleOne: int = 7;
// We output the result to stdout.
print("{exampleOne + exampleTwo}");
// output: 
// [UNHANDLED_ERROR]: "You may not reassign a constant variable."
// exampleOne = 7
//            ^
//       [Assignment not allowed to constant {exmapleOne}]
```

---

### Creating a function
A function has a few rules that it goes by.
- Every function by default is `private` <u>unless</u> it's the main entry point which is <u>required</u> to be public.
- All functions must declare a return type with the `->` identifier followed by it's type.
- Function parameters are <u>pass-by-reference</u> unless specified with a `&` symbol to declare <u>pass-by-value</u>
- A function with no return statement will always be assumed void.
```
<function-declaration>  ::=  <visibility> "fn" <identifier> "(" [<param-list>] ")" ["->" <type>] <block>
```
#### Function Declaration
Let's take a look at the main entry point of the program:
```cpp
public fn main() -> void {
        print("Hello, world!");
}
```
We can break down the function signature as so:
```cpp
     public        fn         main           ()           ->       void  { /*<function_block>*/ }
// <visibility>  <keyword>  <identifier>  <parameters>  <symbol>  <type>
```
A function with parameters would look like:
```cpp
public fn example(numOne: int, numTwo: int) {
        return numOne + numTwo;
}
```

The body of a function will encapsulate all of the data within and that will create a block. By default, each block will be garbage-collected. In the future, the implementation of the `unsafe` keyword will allow for manual memory allocation.

Let's look at what the Pascal's Triangle implementation looks like in Cryo:
```cpp
#use 'stdlib' import *;

public fn pascal(rows: int) -> Array<Array<int>> {
        if(rows == 1) return [[1]];
        if(rows == 0) return [];

        const prev: int = (pascal(rows - 1));
        const last: int = prev[prev.length - 1];
        const next: Array<int>

        for($i; i < last.length - 1; i++) {
                next.push(last[i] + last[i + 1]);
        }

        next.push(1);
        prev.push(next);

        return prev;
}

print(pascal(5));
```

#### Function Methods
A Function Method is a function within a function but is <u>__only__</u> accessable by the block in which it's declared in.
```
<function-method>    ::=  <function-declaration> "method fn" <identifier> "(" [<param-list>] ")" ["->" <type>] <block>
```

Let's take a look a simple example using Function Methods:
```cpp
public fn example() -> void {
        method fn foo(num: int) -> int {
                return num + 35;
        }

        return foo(34);
}
example()
```

### Creating a Class *
_Note: This feature is currently not implemented or supported, however, in the future this will fully implemented._

#### Class Creation
Let's create a simple class. There are a few rules for classes but it is less restrictive than functions:

- All classes are also private by default. Using the `public` keyword will make it accessable for import.
- A structure declaration _(`->` symbol)_ is <u>__not__</u> required, but reccomended for optimization.
- A class may extend another class but must use the `super()` keyword call to instanciate the extended classes methods. The methods by extension may not be altered within the callee.
- All classes are a child class of the `Object` interface and `super()` is 

A simple class:
```cpp
#use "stdlib" import *;

public class Foo {
        // Example class property to set a muteable variable to "".
        exampleText: string = "";
        // If you declare its type, you don't have to give it an initial value.
        name: string;

        // This is the constructor of the class, this is required.
        // However, it is not requried to set anything.
        construct(name: string) {
                // You will pass in your constructor variables here:
                this.exampleText = "Hello, world";
                this.name = name;
        };

        // Here is an example method to print to stdout.
        method sayHello() -> void {
                print(this.exampleText);
        };

        // Example on using the provided initial paramaters.
        method sayName() -> void {
                print("Hi, my name is ${this.name}!");
        };
}

const fooInstance = new Foo("Jake");
fooInstance.sayHello();
// Hello, world!
fooInstance.sayName();
// Hi, my name is Jake!
```

#### Class Structure Definitions
Later on, we will go over structures and interfaces to make type definitions more organized. For a breif overview, an `interface` is a structural definition for a class or function.

Here is an example:
```cpp
#use "stdlib" import *;

interface Foo {
        addTwoNums(numOne: int, numTwo: int);
};

public class ExampleTwo -> Foo {
        construct() {};

        method addTwoNums(numOne: int, numTwo: int) -> int {
                return numOne + numTwo;
        };
};

const bar = new ExampleTwo();
bar.addTwoNums(15, 5);
// Output: 20

```

#### Extending from another class
Classes can inherit from other classes. When inheriting from another class, in the `construct`, you must call `super()` at the top-level of the constructor to instanciate the parent classes structure.
```cpp
construct(params: string) {
        super();
        this.args = params;
}
// ...
```

Lets look at a full example extending another class:
```cpp
#use "stdlib" import *;

// Structural prototypes for the class implementations.
interface IAnimal {
        health: int;
        stamina: int;
        hunger: int;
        method eat(foodGiven: int) -> void;
        method walk(duration: int) -> void;
};

interface IDog {
        method isuper(IAnimal);
        method bark(amount: int) -> void;
};

// Class implementations
public class Animal -> IAnimal {
        health: int;
        stamina: int;
        hunger: int;

        constructor() {
                this.health = 100;
                this.stamina = 125;
                this.hunger = 50;
        };

        method eat(foodGiven: int) -> void {
                this.hunger = this.hunger - foodGiven;
                print("-${foodGiven} hunger level.");
                this.health = this.health + 5;
                print("+5 Health Points given!");
        };
        

        method walk(duration: int) -> void {
                const staminaBurn = (duration / 30);
                this.stamina = this.stamina - staminaBurn;
                print("-${staminaBurn} has been spent!");
        };
}

// Extending class structure to the `Dog` class.
public class Dog extends Animal -> Idog {
        construct() {
                super();
        }

        method bark(amount: int) -> void {
                for($i .. amount) {
                        const staminaBurn = i;
                        this.stamina = this.stamina - staminaBurn;
                        print("Dog: *barks*");
                }
        };
}

const dogExample = new Dog();
dogExample.bark(2);
// Dog: *barks*
// Dog: *barks*
dogExample.eat(10);
// -10 hunger level.
// +5 Health Points given!
```

---

## The Standard Library
The standard library is something that will take a significant amount of time to complete and many common-core functionalities will be unfinished and delayed.

_Note: this <u>has not been developed yet</u> and is just a draft of the future documentation_

As of right now, the plan for the standard library is as follows:

- The primary `stdlib` will have the most essential APIs to develop Cryo code.
- Other core functionalities will be split such as `stdlib/sys` and so on.
- Initially, APIs under `stdlib` may be migrated into new namespaces.

__Note: By default, `stdlib` will be assumed and compiled together. All other sub-namespaces will need to be manually imported.__

### Standard Library Overview
This is the overview / documentation of the Cryo Standard Library functions.

---
### `stdlib`

#### print
 `print(message: string);`

Simple barebone print statement that logs the output to `stdout`. Takes in a `string` parameter and returns `void`.
```cpp
print(message: string);
```
---
#### dbgprint
`dbgprint(message: string, value: any);`

A verbose print statement to `stdout`. Extracts verbose logging data from the provided `value` parameter if avaliable. The `message` parameters is additional user-provided context to the message.
```cpp
dbgprint(message: string, value: any);
```
---
#### printerr
`printerr(message: string, value: any, customError?: Error);`

This print statement <u>will result in a __panic__</u>. By default, it will crash the program and log the custom `message` parameter, extract any logging information out of the `value` parameter, and output it to stdout & stderr. Any custom `Error` parameter set will change the functionality of this function.
```cpp
printerr(message: string, value: any, customError?: Error);
```
---




