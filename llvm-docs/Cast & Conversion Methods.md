# LLVM Cast and Conversion Methods: An In-Depth Guide

## Introduction

LLVM (Low Level Virtual Machine) is a powerful compiler infrastructure that provides a set of reusable compiler and toolchain technologies. One of the key aspects of working with LLVM is understanding how to manipulate and convert between different data types. This guide focuses on the cast and conversion methods provided by LLVM's IRBuilder class.

These methods are essential for type conversion in LLVM IR (Intermediate Representation), allowing you to change the way a value is interpreted without changing its bit pattern (in most cases) or to extend/truncate values when moving between types of different sizes.

## Core Concepts

Before diving into the specific methods, let's review some core concepts:

1. **Types in LLVM**: LLVM has a rich type system, including integers of various bit widths, floating-point types, pointers, arrays, and more.

2. **SSA Form**: LLVM IR uses Static Single Assignment (SSA) form, meaning each variable is assigned exactly once. This is why these methods return new `Value*` objects rather than modifying existing ones.

3. **IRBuilder**: This class provides a convenient interface for creating LLVM instructions and inserting them into a basic block.

Now, let's explore the cast and conversion methods in detail.

## Integer Conversions

### Truncation: `CreateTrunc`

```cpp
Value *CreateTrunc(Value *V, Type *DestTy, const Twine &Name = "")
```

Truncation reduces the bit width of an integer value by discarding the most significant bits.

**In-depth explanation:**

- Truncation is used when you need to convert a larger integer type to a smaller one.
- Only the least significant bits that fit in the destination type are kept.
- This operation can lead to loss of information if the discarded bits were non-zero.

**Example:**

```cpp
// Assume we have a 32-bit integer value
Value *Int32Value = ...;
// Create a 16-bit integer type
Type *Int16Ty = IntegerType::get(Context, 16);
// Truncate the 32-bit value to 16 bits
Value *TruncatedValue = Builder.CreateTrunc(Int32Value, Int16Ty, "trunc");
```

In this example, if `Int32Value` contained `0xABCD1234`, `TruncatedValue` would contain `0x1234`.

### Zero Extension: `CreateZExt`

```cpp
Value *CreateZExt(Value *V, Type *DestTy, const Twine &Name = "", bool IsNonNeg = false)
```

Zero extension increases the bit width of an integer by adding zero bits to the most significant bit positions.

**In-depth explanation:**

- Used to convert a smaller integer type to a larger one.
- The original bits are preserved, and the new bits are filled with zeros.
- This operation never loses information.
- The `IsNonNeg` parameter is a hint that can be used for optimizations.

**Example:**

```cpp
// Assume we have a 16-bit integer value
Value *Int16Value = ...;
// Create a 32-bit integer type
Type *Int32Ty = IntegerType::get(Context, 32);
// Zero-extend the 16-bit value to 32 bits
Value *ExtendedValue = Builder.CreateZExt(Int16Value, Int32Ty, "zext");
```

If `Int16Value` contained `0xABCD`, `ExtendedValue` would contain `0x0000ABCD`.

### Sign Extension: `CreateSExt`

```cpp
Value *CreateSExt(Value *V, Type *DestTy, const Twine &Name = "")
```

Sign extension increases the bit width of an integer by replicating the sign bit (most significant bit) to fill the new bit positions.

**In-depth explanation:**

- Used to convert a smaller integer type to a larger one while preserving the sign.
- The original bits are preserved, and the new bits are filled with copies of the sign bit.
- This operation preserves the signed value of the original number.

**Example:**

```cpp
// Assume we have a 16-bit integer value
Value *Int16Value = ...;
// Create a 32-bit integer type
Type *Int32Ty = IntegerType::get(Context, 32);
// Sign-extend the 16-bit value to 32 bits
Value *ExtendedValue = Builder.CreateSExt(Int16Value, Int32Ty, "sext");
```

If `Int16Value` contained `0xFBCD` (a negative number in two's complement), `ExtendedValue` would contain `0xFFFFFBCD`.

### Flexible Integer Resizing: `CreateZExtOrTrunc` and `CreateSExtOrTrunc`

These methods provide a convenient way to resize integers without needing to know in advance whether the operation will be an extension or a truncation.

```cpp
Value *CreateZExtOrTrunc(Value *V, Type *DestTy, const Twine &Name = "")
Value *CreateSExtOrTrunc(Value *V, Type *DestTy, const Twine &Name = "")
```

**In-depth explanation:**

- These methods automatically choose between extension (ZExt or SExt) and truncation based on the relative sizes of the source and destination types.
- They're particularly useful when working with integers of unknown or variable sizes.
- `CreateZExtOrTrunc` uses zero extension if an extension is needed.
- `CreateSExtOrTrunc` uses sign extension if an extension is needed.

**Example:**

```cpp
// This function will work correctly regardless of the sizes of SrcTy and DestTy
Value *ResizeInteger(IRBuilder<> &Builder, Value *V, Type *DestTy) {
    return Builder.CreateZExtOrTrunc(V, DestTy, "resized");
}
```

## Floating-Point Conversions

### Float to Integer: `CreateFPToUI` and `CreateFPToSI`

```cpp
Value *CreateFPToUI(Value *V, Type *DestTy, const Twine &Name = "")
Value *CreateFPToSI(Value *V, Type *DestTy, const Twine &Name = "")
```

These methods convert floating-point values to integers.

**In-depth explanation:**

- `CreateFPToUI` converts to an unsigned integer, while `CreateFPToSI` converts to a signed integer.
- The conversion follows the IEEE-754 rounding rules (round to nearest, ties to even).
- If the floating-point value is too large to fit in the integer type, the result is undefined.
- For `CreateFPToUI`, negative input values are treated as zero.

**Example:**

```cpp
// Assume we have a double-precision floating-point value
Value *DoubleValue = ...;
// Create a 32-bit integer type
Type *Int32Ty = IntegerType::get(Context, 32);
// Convert the double to a signed 32-bit integer
Value *IntValue = Builder.CreateFPToSI(DoubleValue, Int32Ty, "fp2int");
```

### Integer to Float: `CreateUIToFP` and `CreateSIToFP`

```cpp
Value *CreateUIToFP(Value *V, Type *DestTy, const Twine &Name = "")
Value *CreateSIToFP(Value *V, Type *DestTy, const Twine &Name = "")
```

These methods convert integers to floating-point values.

**In-depth explanation:**

- `CreateUIToFP` converts from an unsigned integer, while `CreateSIToFP` converts from a signed integer.
- The conversion is exact for integers that can be represented precisely in the floating-point format.
- For larger integers, the result is the nearest representable floating-point value.

**Example:**

```cpp
// Assume we have a 32-bit integer value
Value *IntValue = ...;
// Create a double-precision floating-point type
Type *DoubleTy = Type::getDoubleTy(Context);
// Convert the integer to a double
Value *DoubleValue = Builder.CreateSIToFP(IntValue, DoubleTy, "int2fp");
```

### Floating-Point Precision Changes: `CreateFPTrunc` and `CreateFPExt`

```cpp
Value *CreateFPTrunc(Value *V, Type *DestTy, const Twine &Name = "")
Value *CreateFPExt(Value *V, Type *DestTy, const Twine &Name = "")
```

These methods change the precision of floating-point values.

**In-depth explanation:**

- `CreateFPTrunc` reduces precision (e.g., double to float), which may lose information.
- `CreateFPExt` increases precision (e.g., float to double), which is always lossless.
- Truncation follows IEEE-754 rounding rules.

**Example:**

```cpp
// Assume we have a double-precision value
Value *DoubleValue = ...;
// Create a single-precision floating-point type
Type *FloatTy = Type::getFloatTy(Context);
// Truncate the double to a float
Value *FloatValue = Builder.CreateFPTrunc(DoubleValue, FloatTy, "dbl2flt");
```

## Pointer Conversions

### Pointer to Integer: `CreatePtrToInt`

```cpp
Value *CreatePtrToInt(Value *V, Type *DestTy, const Twine &Name = "")
```

This method converts a pointer to an integer.

**In-depth explanation:**

- The integer produced is the address held in the pointer.
- The size of the integer type should be large enough to hold the pointer value.
- This is often used for pointer arithmetic or when you need to manipulate the raw address.

**Example:**

```cpp
// Assume we have a pointer value
Value *PtrValue = ...;
// Create a 64-bit integer type
Type *Int64Ty = IntegerType::get(Context, 64);
// Convert the pointer to an integer
Value *IntValue = Builder.CreatePtrToInt(PtrValue, Int64Ty, "ptr2int");
```

### Integer to Pointer: `CreateIntToPtr`

```cpp
Value *CreateIntToPtr(Value *V, Type *DestTy, const Twine &Name = "")
```

This method converts an integer to a pointer.

**In-depth explanation:**

- The integer is interpreted as a memory address.
- This is often used to cast back a pointer that was previously converted to an integer.
- Care should be taken to ensure the integer represents a valid memory address.

**Example:**

```cpp
// Assume we have a 64-bit integer value representing an address
Value *IntValue = ...;
// Create a pointer type (e.g., pointer to i8)
Type *PtrTy = Type::getInt8PtrTy(Context);
// Convert the integer to a pointer
Value *PtrValue = Builder.CreateIntToPtr(IntValue, PtrTy, "int2ptr");
```

## Miscellaneous Conversions

### Bitcast: `CreateBitCast`

```cpp
Value *CreateBitCast(Value *V, Type *DestTy, const Twine &Name = "")
```

This method reinterprets the bits of a value as a different type.

**In-depth explanation:**

- The source and destination types must have the same bit width.
- This is often used to cast between pointer types or to reinterpret floating-point values as integers (or vice versa).
- No bits are changed; it's purely a reinterpretation of the existing bit pattern.

**Example:**

```cpp
// Assume we have a float value
Value *FloatValue = ...;
// Create a 32-bit integer type
Type *Int32Ty = IntegerType::get(Context, 32);
// Reinterpret the float as an integer
Value *IntValue = Builder.CreateBitCast(FloatValue, Int32Ty, "float2int");
```

### Address Space Cast: `CreateAddrSpaceCast`

```cpp
Value *CreateAddrSpaceCast(Value *V, Type *DestTy, const Twine &Name = "")
```

This method converts a pointer from one address space to another.

**In-depth explanation:**

- Address spaces are used to represent different memory regions (e.g., global memory, shared memory in GPUs).
- This cast doesn't change the pointer value, only how it's interpreted.
- The source and destination types must be pointers, differing only in their address space.

**Example:**

```cpp
// Assume we have a pointer in address space 1
Value *Ptr1 = ...;
// Create a pointer type in address space 2
Type *PtrTy2 = Type::getInt8PtrTy(Context, 2);
// Cast the pointer to the new address space
Value *Ptr2 = Builder.CreateAddrSpaceCast(Ptr1, PtrTy2, "addrcast");
```
