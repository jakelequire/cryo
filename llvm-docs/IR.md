# LLVM IR: A Comprehensive Guide for LLVM 18

## Table of Contents

1. [Introduction to LLVM IR](#introduction-to-llvm-ir)
2. [LLVM IR Basics](#llvm-ir-basics)
3. [Types in LLVM IR](#types-in-llvm-ir)
4. [Instructions and Operators](#instructions-and-operators)
5. [Control Flow](#control-flow)
6. [Functions and Calling Conventions](#functions-and-calling-conventions)
7. [Memory Management](#memory-management)
8. [Metadata and Debug Information](#metadata-and-debug-information)
9. [Optimization Passes](#optimization-passes)
10. [LLVM 18 Specific Features](#llvm-18-specific-features)
11. [Conclusion](#conclusion)

## Introduction to LLVM IR

LLVM IR (Intermediate Representation) is a crucial component of the LLVM (Low Level Virtual Machine) compiler infrastructure. It serves as a bridge between high-level programming languages and machine code, providing a powerful and flexible platform for code analysis, optimization, and generation.

Key points:

- LLVM IR is designed to be language-independent, allowing it to support a wide range of programming languages.
- It uses a Static Single Assignment (SSA) form, which simplifies many compiler optimizations.
- LLVM IR can be represented in three forms: in-memory representation, bitcode, and human-readable assembly-like language.

In this lecture, we'll focus primarily on the human-readable form of LLVM IR, as it's the most accessible for learning and understanding the concepts.

## LLVM IR Basics

LLVM IR is structured around a few fundamental concepts:

1. **Modules**: The top-level container for all IR entities.
2. **Functions**: Collections of basic blocks that define a sequence of instructions.
3. **Basic Blocks**: Linear sequences of instructions with a single entry point and a single exit point.
4. **Instructions**: The atomic units of execution in LLVM IR.
5. **Global Variables**: Variables with global scope within a module.

Let's look at a simple example of LLVM IR:

```llvm
; Module-level declarations
@global_var = global i32 0

; Function definition
define i32 @main() {
entry:
  %result = add i32 1, 2
  ret i32 %result
}
```

Key observations:

- Comments start with a semicolon (`;`).
- Global variables are prefixed with `@`.
- Local variables (in functions) are prefixed with `%`.
- Functions are defined using the `define` keyword, followed by the return type and function name.
- Basic blocks are labeled (e.g., `entry:`).
- Instructions operate on variables and constants.

## Types in LLVM IR

LLVM IR has a rich type system that includes both primitive and complex types. Understanding these types is crucial for working with LLVM IR effectively.

### Primitive Types

- `void`: Represents no value
- `i1`: 1-bit integer (boolean)
- `i8`, `i16`, `i32`, `i64`: Integer types of various bit widths
- `float`: 32-bit floating-point
- `double`: 64-bit floating-point

### Complex Types

- **Arrays**: Fixed-size collections of elements, e.g., `[4 x i32]`
- **Vectors**: SIMD vector types, e.g., `<4 x float>`
- **Pointers**: Memory addresses, e.g., `i32*` (pointer to i32)
- **Structures**: Composite types, e.g., `{ i32, float }`
- **Function Types**: Represent function signatures, e.g., `i32 (i8*, ...)*`

Example of type usage:

```llvm
%struct.example = type { i32, float }
@global_array = global [10 x i32] zeroinitializer
%local_vector = <4 x float> <float 1.0, float 2.0, float 3.0, float 4.0>
```

## Instructions and Operators

LLVM IR provides a wide range of instructions for various operations. Here are some of the most common categories:

### Arithmetic Operations

- `add`, `sub`, `mul`, `udiv`, `sdiv`, `urem`, `srem`

Example:

```llvm
%result = add i32 %a, %b
```

### Bitwise Operations

- `and`, `or`, `xor`, `shl`, `lshr`, `ashr`

Example:

```llvm
%result = and i32 %a, 15
```

### Comparison Operations

- `icmp` (integer comparison), `fcmp` (floating-point comparison)

Example:

```llvm
%cond = icmp sgt i32 %a, 10
```

### Memory Operations

- `alloca`, `load`, `store`, `getelementptr`

Example:

```llvm
%ptr = alloca i32
store i32 42, i32* %ptr
%val = load i32, i32* %ptr
```

### Conversion Operations

- `trunc`, `zext`, `sext`, `fptrunc`, `fpext`, `fptoui`, `fptosi`, `uitofp`, `sitofp`, `ptrtoint`, `inttoptr`, `bitcast`

Example:

```llvm
%extended = zext i32 %value to i64
```

### Other Important Instructions

- `phi`: Used for SSA form in control flow junctions
- `select`: Conditional selection between two values
- `call`: Function invocation
- `ret`: Return from a function

## Control Flow

LLVM IR uses basic blocks and branching instructions to represent control flow.

### Unconditional Branch

```llvm
br label %target
```

### Conditional Branch

```llvm
%cond = icmp eq i32 %a, 0
br i1 %cond, label %true_branch, label %false_branch
```

### Switch Statement

```llvm
switch i32 %val, label %default [
  i32 0, label %case0
  i32 1, label %case1
]
```

### Phi Instruction

Used to select a value based on which basic block was previously executed:

```llvm
entry:
  br i1 %cond, label %then, label %else

then:
  %x = add i32 1, 2
  br label %merge

else:
  %y = sub i32 5, 3
  br label %merge

merge:
  %result = phi i32 [ %x, %then ], [ %y, %else ]
```

## Functions and Calling Conventions

Functions in LLVM IR are defined using the `define` keyword, followed by the return type, function name, and parameter list.

```llvm
define i32 @sum(i32 %a, i32 %b) {
entry:
  %result = add i32 %a, %b
  ret i32 %result
}
```

### Function Attributes

LLVM IR supports various function attributes to provide additional information to the optimizer:

- `nounwind`: The function never throws exceptions
- `readonly`: The function doesn't write to memory
- `norecurse`: The function doesn't recursively call itself

Example:

```llvm
define i32 @pure_function(i32 %x) nounwind readonly {
  ; function body
}
```

### Calling Conventions

LLVM supports different calling conventions, which determine how arguments are passed and how the stack is managed. Some common calling conventions include:

- `ccc`: C calling convention
- `fastcc`: Fast calling convention (platform-specific optimizations)
- `coldcc`: Cold calling convention (for rarely called functions)

Example:

```llvm
define fastcc void @fast_function(i32 %x) {
  ; function body
}
```

## Memory Management

LLVM IR provides several instructions for memory management:

### Stack Allocation

The `alloca` instruction is used for stack allocation:

```llvm
%ptr = alloca i32
```

### Heap Allocation

LLVM IR doesn't have built-in heap allocation instructions. Instead, you typically call external functions like `malloc` and `free`:

```llvm
%size = i64 40
%ptr = call i8* @malloc(i64 %size)
; ... use the allocated memory ...
call void @free(i8* %ptr)
```

### Accessing Structured Data

The `getelementptr` instruction is used to compute pointers into structured data types:

```llvm
%struct.example = type { i32, float }
@global_struct = global %struct.example { i32 42, float 3.14 }

define float @get_float() {
  %ptr = getelementptr %struct.example, %struct.example* @global_struct, i32 0, i32 1
  %value = load float, float* %ptr
  ret float %value
}
```

## Metadata and Debug Information

LLVM IR supports attaching metadata to various IR constructs, which can be used for debug information, optimization hints, or other purposes.

### Named Metadata

```llvm
!0 = !{i32 42, null, !"string"}
!named_metadata = !{!0}
```

### Metadata Attached to Instructions

```llvm
%result = add i32 %a, %b, !dbg !1
```

### Debug Information

LLVM uses a specific set of metadata nodes to represent debug information, including source locations, variable definitions, and type information.

```llvm
define i32 @main() #0 !dbg !4 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  ret i32 0, !dbg !10
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, ...)
!1 = !DIFile(filename: "example.c", directory: "/path/to/source")
```

## Optimization Passes

LLVM provides a wide range of optimization passes that operate on the IR. Some common categories include:

- **Analysis Passes**: Gather information about the code (e.g., dominator tree, alias analysis)
- **Transform Passes**: Modify the code to improve performance or reduce size
- **Utility Passes**: Perform tasks like printing or verifying the IR

Examples of important optimization passes:

- **Constant Propagation**: Replaces variables with their constant values
- **Dead Code Elimination**: Removes unreachable or unused code
- **Function Inlining**: Replaces function calls with the function body
- **Loop Unrolling**: Duplicates loop bodies to reduce branching overhead
- **Vectorization**: Converts scalar operations to vector operations for SIMD processing

To apply optimizations in LLVM, you typically use the `opt` tool or programmatically through the LLVM API.

## LLVM 18 Specific Features

LLVM 18, released in 2024, introduces several new features and improvements. While the core concepts of LLVM IR remain largely the same, there are some additions and enhancements worth noting:

### Enhanced Vector Support

LLVM 18 improves support for vector operations, including:

- New intrinsics for efficient vector manipulation
- Better handling of mixed vector types
- Improved vectorization passes

Example of using a vector intrinsic:

```llvm
declare <4 x float> @llvm.vector.reduce.fadd.v4f32(<4 x float>, <4 x float>)

define float @sum_vector(<4 x float> %vec) {
  %result = call <4 x float> @llvm.vector.reduce.fadd.v4f32(<4 x float> %vec, <4 x float> zeroinitializer)
  %scalar = extractelement <4 x float> %result, i32 0
  ret float %scalar
}
```

### Improved Debug Information

LLVM 18 enhances debug information support, allowing for more accurate source-level debugging:

- Better handling of inlined functions
- Improved location tracking for optimized code
- Enhanced support for language-specific debug information

### New Optimization Passes

LLVM 18 introduces new optimization passes and improves existing ones:

- **MemCpyOptimizer**: Enhances the optimization of memory copy and set operations
- **LoopVectorize**: Improvements in auto-vectorization of loops
- **GlobalOpt**: Better global variable optimization

### Coroutine Support

LLVM 18 continues to improve support for coroutines, making it easier to implement and optimize languages with coroutine features:

```llvm
declare i8* @llvm.coro.begin(i8*, i8*)
declare i8 @llvm.coro.end(i8*, i1)
declare i8* @llvm.coro.free(i8*)
declare i8* @llvm.coro.resume(i8*)
declare void @llvm.coro.suspend(i8*, i1)

define i8* @simple_coroutine(i32 %x) {
entry:
  %id = call token @llvm.coro.id(i32 0, i8* null, i8* null, i8* null)
  %size = call i32 @llvm.coro.size.i32()
  %alloc = call i8* @malloc(i32 %size)
  %hdl = call noalias i8* @llvm.coro.begin(i8* %alloc, i8* null)
  ; ... coroutine body ...
  call i1 @llvm.coro.suspend(token none, i1 false)
  ; ... more coroutine body ...
  call void @llvm.coro.end(i8* %hdl, i1 false)
  ret i8* %hdl
}
```

### Improved Support for GPU Targets

LLVM 18 enhances support for GPU programming models:

- Better integration with CUDA and OpenCL
- Improved optimization passes for GPU code
- Enhanced support for GPU-specific intrinsics

### Security Features

LLVM 18 introduces new security-related features:

- Improved support for Control Flow Integrity (CFI)
- Enhanced stack protection mechanisms
- New intrinsics for hardware-backed security features

Example of using a security-related intrinsic:

```llvm
declare void @llvm.instrprof.increment(i8*, i64, i32, i32)

define void @instrumented_function() {
  call void @llvm.instrprof.increment(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @__profn_instrumented_function, i32 0, i32 0), i64 12345, i32 1, i32 0)
  ; ... function body ...
  ret void
}
```

## Conclusion

LLVM IR is a powerful and flexible intermediate representation that forms the backbone of the LLVM compiler infrastructure. Its design allows for efficient optimization and code generation across a wide range of target architectures.

Key takeaways:

- LLVM IR uses an SSA form, which simplifies many compiler optimizations.
- It provides a rich set of types and instructions to represent complex programs.
- Control flow is represented using basic blocks and branching instructions.
- LLVM IR supports various memory management techniques and structured data access.
- Metadata and debug information can be attached to IR constructs for enhanced analysis and debugging.
- LLVM provides a wide range of optimization passes that operate on the IR.
- LLVM 18 introduces several enhancements, including improved vector support, better debug information, new optimization passes, and enhanced security features.

Understanding LLVM IR is crucial for compiler developers, language implementers, and anyone working on low-level code optimization. As LLVM continues to evolve, staying up-to-date with the latest features and best practices will help you make the most of this powerful tool.
