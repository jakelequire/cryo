# LLVM 18 IR Function Attributes: A Comprehensive Guide

Function attributes in LLVM IR provide additional information about functions and their behaviors. They can be used to optimize code generation, enforce security measures, or provide hints to the optimizer. This guide covers the most important function attributes in LLVM 18 IR.

## Table of Contents

1. [Introduction](#introduction)
2. [Commonly Used Function Attributes](#commonly-used-function-attributes)
3. [Optimization Attributes](#optimization-attributes)
4. [Safety and Security Attributes](#safety-and-security-attributes)
5. [Calling Convention Attributes](#calling-convention-attributes)
6. [Example Usage](#example-usage)
7. [Best Practices](#best-practices)

## Introduction

Function attributes in LLVM IR are specified using the `attributes` keyword followed by a set of attribute names or key-value pairs. They can be applied to function declarations and definitions.

Syntax:
```llvm
define [return type] @[function name]([parameters]) [attributes] {
  ; Function body
}
```

## Commonly Used Function Attributes

1. `nounwind`: Indicates that the function never throws exceptions.
2. `readonly`: Specifies that the function does not write to memory.
3. `writeonly`: Indicates that the function does not read from memory.
4. `noreturn`: Specifies that the function never returns normally.
5. `argmemonly`: Indicates that the function only reads and writes its arguments.

## Optimization Attributes

1. `inlinehint`: Suggests that the function should be inlined.
2. `alwaysinline`: Indicates that the function must always be inlined.
3. `noinline`: Specifies that the function should never be inlined.
4. `optsize`: Optimizes the function for size rather than speed.
5. `minsize`: Optimizes the function for minimum size.
6. `hot`: Indicates that the function is likely to be executed frequently.
7. `cold`: Indicates that the function is unlikely to be executed.

## Safety and Security Attributes

1. `nonnull`: Specifies that a pointer argument or return value is not null.
2. `noalias`: Indicates that memory accessed through pointer arguments doesn't alias.
3. `nocapture`: Specifies that a pointer argument is not captured by the function.
4. `dereferenceable(N)`: Indicates that a pointer argument is dereferenceable for at least N bytes.
5. `sanitize_address`: Enables AddressSanitizer instrumentation.
6. `sanitize_thread`: Enables ThreadSanitizer instrumentation.
7. `sanitize_memory`: Enables MemorySanitizer instrumentation.

## Calling Convention Attributes

1. `ccc`: C calling convention (default).
2. `fastcc`: Fast calling convention.
3. `coldcc`: Calling convention for rarely executed functions.
4. `tailcc`: Tail call optimized calling convention.
5. `swiftcc`: Swift language calling convention.

## Example Usage

Here's an example of a function declaration with multiple attributes:

```llvm
declare i32 @example_function(i8* %ptr, i32 %val) 
    nounwind readonly noinline 
    "frame-pointer"="all" 
    "no-trapping-math"="true"
```

And a function definition with attributes:

```llvm
define i32 @optimized_function(i32 %a, i32 %b) 
    nounwind readnone alwaysinline 
{
  %result = add i32 %a, %b
  ret i32 %result
}
```

## Best Practices

1. Use attributes judiciously to provide accurate information about function behavior.
2. Combine complementary attributes for better optimization (e.g., `nounwind` with `readonly`).
3. Be cautious with `alwaysinline` and `noinline`, as they may interfere with the optimizer's decisions.
4. Use safety attributes like `nonnull` and `dereferenceable` to help catch bugs and improve optimizations.
5. Consider using calling convention attributes for performance-critical code or when interfacing with specific languages.

Remember that while attributes can significantly improve performance and catch potential bugs, they must be used correctly. Incorrect attribute usage can lead to undefined behavior or misoptimization.