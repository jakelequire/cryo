# LLVM Memory Instruction Methods: An In-Depth Guide

## Introduction

LLVM provides a rich set of methods for creating memory-related instructions. These instructions are crucial for managing memory allocation, accessing memory, and performing atomic operations. This guide will explore the IRBuilder methods used to create these instructions, explaining their purpose, parameters, and providing examples of their use.

## Core Concepts

Before diving into the specific methods, let's review some core concepts:

1. **Stack Allocation**: Temporary memory allocated on the function's stack frame.
2. **Pointer Arithmetic**: Calculations performed on memory addresses.
3. **Memory Access**: Reading from and writing to memory locations.
4. **Atomic Operations**: Operations that are executed as a single, indivisible unit.
5. **Address Spaces**: Different regions of memory, often used in GPU programming.

Now, let's explore the memory instruction methods in detail.

## Stack Allocation

### CreateAlloca

```cpp
AllocaInst *CreateAlloca(Type *Ty, unsigned AddrSpace, Value *ArraySize = nullptr, const Twine &Name = "")
AllocaInst *CreateAlloca(Type *Ty, Value *ArraySize = nullptr, const Twine &Name = "")
```

These methods create an `alloca` instruction, which allocates memory on the stack frame of the currently executing function.

**In-depth explanation:**

- `Ty`: The type of the elements to be allocated.
- `AddrSpace`: The address space in which to allocate the memory (optional).
- `ArraySize`: If provided, allocates an array of elements instead of a single element.
- The alignment of the allocation is determined by the data layout of the module.

**Example:**

```cpp
// Allocate space for a single integer
AllocaInst *IntAlloca = Builder.CreateAlloca(Int32Ty, nullptr, "intVar");

// Allocate an array of 10 integers
Value *Ten = ConstantInt::get(Int32Ty, 10);
AllocaInst *IntArrayAlloca = Builder.CreateAlloca(Int32Ty, Ten, "intArray");
```

## Memory Access

### CreateLoad

```cpp
LoadInst *CreateLoad(Type *Ty, Value *Ptr, const char *Name)
LoadInst *CreateLoad(Type *Ty, Value *Ptr, const Twine &Name = "")
LoadInst *CreateLoad(Type *Ty, Value *Ptr, bool isVolatile, const Twine &Name = "")
```

These methods create a `load` instruction, which reads a value from memory.

**In-depth explanation:**

- `Ty`: The type of the value being loaded.
- `Ptr`: A pointer to the memory location to load from.
- `isVolatile`: If true, the load is marked as volatile (e.g., for memory-mapped I/O).

**Example:**

```cpp
// Assuming we have a pointer to an integer
Value *IntPtr = ...;
LoadInst *LoadedInt = Builder.CreateLoad(Int32Ty, IntPtr, "loadedValue");
```

### CreateStore

```cpp
StoreInst *CreateStore(Value *Val, Value *Ptr, bool isVolatile = false)
```

This method creates a `store` instruction, which writes a value to memory.

**In-depth explanation:**

- `Val`: The value to be stored.
- `Ptr`: A pointer to the memory location where the value should be stored.
- `isVolatile`: If true, the store is marked as volatile.

**Example:**

```cpp
// Assuming we have a value and a pointer to store it to
Value *IntValue = ...;
Value *IntPtr = ...;
StoreInst *StoredInt = Builder.CreateStore(IntValue, IntPtr);
```

### CreateAlignedLoad and CreateAlignedStore

```cpp
LoadInst *CreateAlignedLoad(Type *Ty, Value *Ptr, MaybeAlign Align, bool isVolatile = false, const Twine &Name = "")
StoreInst *CreateAlignedStore(Value *Val, Value *Ptr, MaybeAlign Align, bool isVolatile = false)
```

These methods are similar to `CreateLoad` and `CreateStore`, but allow specifying a particular alignment for the memory access.

**In-depth explanation:**

- `Align`: The alignment of the memory access. If not specified, it's determined by the ABI.
- Proper alignment can be crucial for performance on some architectures.

**Example:**

```cpp
// Load with 16-byte alignment
LoadInst *AlignedLoad = Builder.CreateAlignedLoad(Int32Ty, IntPtr, Align(16), "alignedLoad");

// Store with 16-byte alignment
StoreInst *AlignedStore = Builder.CreateAlignedStore(IntValue, IntPtr, Align(16));
```

## Atomic Operations

### CreateFence

```cpp
FenceInst *CreateFence(AtomicOrdering Ordering, SyncScope::ID SSID = SyncScope::System, const Twine &Name = "")
```

This method creates a `fence` instruction, which ensures memory ordering between threads or devices.

**In-depth explanation:**

- `Ordering`: Specifies the strictness of the ordering (e.g., `Acquire`, `Release`, `SequentiallyConsistent`).
- `SSID`: Specifies the scope of the fence (e.g., system-wide, single-thread).

**Example:**

```cpp
FenceInst *Fence = Builder.CreateFence(AtomicOrdering::SequentiallyConsistent, SyncScope::System, "memoryFence");
```

### CreateAtomicCmpXchg

```cpp
AtomicCmpXchgInst *CreateAtomicCmpXchg(Value *Ptr, Value *Cmp, Value *New, MaybeAlign Align,
                                       AtomicOrdering SuccessOrdering, AtomicOrdering FailureOrdering,
                                       SyncScope::ID SSID = SyncScope::System)
```

This method creates an atomic compare-and-swap instruction.

**In-depth explanation:**

- `Ptr`: Pointer to the memory location.
- `Cmp`: The value to compare against.
- `New`: The new value to store if the comparison succeeds.
- `SuccessOrdering` and `FailureOrdering`: Memory orderings for success and failure cases.

**Example:**

```cpp
Value *Ptr = ...;  // Pointer to memory
Value *Expected = ...;  // Expected current value
Value *New = ...;  // New value to store
AtomicCmpXchgInst *CmpXchg = Builder.CreateAtomicCmpXchg(
    Ptr, Expected, New, Align(8),
    AtomicOrdering::SequentiallyConsistent,
    AtomicOrdering::SequentiallyConsistent
);
```

### CreateAtomicRMW

```cpp
AtomicRMWInst *CreateAtomicRMW(AtomicRMWInst::BinOp Op, Value *Ptr, Value *Val, MaybeAlign Align,
                               AtomicOrdering Ordering, SyncScope::ID SSID = SyncScope::System)
```

This method creates an atomic read-modify-write instruction.

**In-depth explanation:**

- `Op`: The operation to perform (e.g., Add, And, Or, Xor).
- `Ptr`: Pointer to the memory location.
- `Val`: The value to use in the operation.
- `Ordering`: Memory ordering for the operation.

**Example:**

```cpp
Value *Ptr = ...;  // Pointer to memory
Value *Increment = ...;  // Value to add
AtomicRMWInst *AtomicAdd = Builder.CreateAtomicRMW(
    AtomicRMWInst::Add, Ptr, Increment, Align(8),
    AtomicOrdering::SequentiallyConsistent
);
```

## Pointer Arithmetic

### CreateGEP (GetElementPtr)

```cpp
Value *CreateGEP(Type *Ty, Value *Ptr, ArrayRef<Value *> IdxList, const Twine &Name = "", bool IsInBounds = false)
```

This method creates a `getelementptr` instruction, which performs pointer arithmetic.

**In-depth explanation:**

- `Ty`: The type of the object being indexed into.
- `Ptr`: The base pointer.
- `IdxList`: A list of indices for indexing into the object.
- `IsInBounds`: If true, the resultant pointer is marked as inbounds.

**Example:**

```cpp
// Assuming we have a pointer to an array of integers
Value *ArrayPtr = ...;
Value *Index = ConstantInt::get(Int32Ty, 5);
Value *ElementPtr = Builder.CreateGEP(Int32Ty, ArrayPtr, Index, "element");
```

### Convenience Methods for GEP

LLVM provides several convenience methods for common GEP operations:

- `CreateInBoundsGEP`: Creates an inbounds GEP.
- `CreateConstGEP1_32`, `CreateConstGEP2_32`: Creates a GEP with constant 32-bit indices.
- `CreateConstGEP1_64`, `CreateConstGEP2_64`: Creates a GEP with constant 64-bit indices.
- `CreateStructGEP`: Creates a GEP for accessing struct members.

These methods simplify the creation of GEP instructions for common use cases.

**Example:**

```cpp
// Access the third element of an array
Value *ThirdElement = Builder.CreateConstGEP1_32(Int32Ty, ArrayPtr, 2, "thirdElement");

// Access a struct member
Value *StructPtr = ...;
Value *SecondMember = Builder.CreateStructGEP(StructTy, StructPtr, 1, "secondMember");
```

## Global String Creation

### CreateGlobalStringPtr

```cpp
Constant *CreateGlobalStringPtr(StringRef Str, const Twine &Name = "", unsigned AddressSpace = 0, Module *M = nullptr)
```

This method creates a global string constant and returns a pointer to it.

**In-depth explanation:**

- `Str`: The string content.
- `Name`: Name for the global variable.
- `AddressSpace`: The address space for the global variable.
- `M`: The module in which to create the global variable.

**Example:**

```cpp
Constant *HelloWorld = Builder.CreateGlobalStringPtr("Hello, World!", "helloString");
```

## Conclusion

These memory instruction methods in LLVM's IRBuilder provide a powerful interface for manipulating memory at a low level. They allow you to perform stack allocations, load and store values, execute atomic operations, and perform pointer arithmetic.

Understanding these methods is crucial for working effectively with LLVM IR, especially when dealing with memory-intensive operations or when precise control over memory access patterns is required.

Remember that while these methods provide fine-grained control over memory operations, it's important to use them carefully. Incorrect usage can lead to memory errors, race conditions, or undefined behavior in your programs.

As you work more with LLVM, you'll find these methods to be essential tools for creating efficient and correct code, especially when implementing complex algorithms or optimizing memory access patterns.
