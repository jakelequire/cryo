/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          *
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/GlobalVariable.h"

enum class AllocaType
{
    // No alloca type (constants and immediate values such as literals).
    None,

    // `AllocaOnly` is used for variables that are only allocated on the stack.
    // Requires `alloca`.
    AllocaOnly,

    // `AllocaAndLoad` is used for variables that are allocated on the stack and loaded.
    // Requires `alloca` and `load`.
    AllocaAndLoad,

    // `AllocaAndStore` is used for variables that are allocated on the stack and stored.
    // Requires `alloca` and `store`.
    AllocaAndStore,

    // `AllocaLoadStore` is used for variables that are allocated on the stack, loaded, and stored.
    // Requires `alloca`, `load`, and `store`.
    AllocaLoadStore,

    // `Global` is used for variables that are allocated in the global scope.
    Global,

    // `Parameter` is used for function parameters.
    Parameter,

    // `Temporary` is used for temporary variables.
    Temporary,

    // `Aggregate` is used for aggregate types (structs, arrays, etc.).
    Aggregate,

    // `phi` is used for phi nodes in SSA form.
    PhiNode,

    // `DynamicArray` is used for dynamic arrays.
    DynamicArray,

    // `StackSave` is used for saving the stack pointer.
    StackSave,
};

struct Allocation
{
    // The type of this allocation
    AllocaType type;

    // Basic allocation instructions
    llvm::AllocaInst *allocaInst;
    llvm::StoreInst *storeInst;
    llvm::LoadInst *loadInst;

    // Global variable (for global allocations)
    llvm::GlobalVariable *global;

    // For phi nodes in SSA form
    llvm::PHINode *phi;

    // For dynamic arrays, store the size computation
    llvm::Value *dynamicSize;

    // For aggregates, store the member accesses
    std::vector<llvm::GetElementPtrInst *> memberAccesses;

    // Constructor for local variables
    Allocation(AllocaType t, llvm::AllocaInst *a = nullptr,
               llvm::StoreInst *s = nullptr, llvm::LoadInst *l = nullptr)
        : type(t), allocaInst(a), storeInst(s), loadInst(l), global(nullptr),
          phi(nullptr), dynamicSize(nullptr) {}

    // Constructor for global variables
    Allocation(llvm::GlobalVariable *g)
        : type(AllocaType::Global), allocaInst(nullptr), storeInst(nullptr),
          loadInst(nullptr), global(g), phi(nullptr), dynamicSize(nullptr) {}

    // Get the value represented by this allocation
    llvm::Value *getValue() const
    {
        switch (type)
        {
        case AllocaType::None:
            return nullptr;
        case AllocaType::AllocaOnly:
            return allocaInst;
        case AllocaType::AllocaAndLoad:
        case AllocaType::AllocaLoadStore:
            return loadInst;
        case AllocaType::Global:
            return global;
        case AllocaType::PhiNode:
            return phi;
        default:
            return allocaInst;
        }
    }

    // Get the pointer to the allocated memory
    llvm::Value *getPointer() const
    {
        if (global)
            return global;
        return allocaInst;
    }

    // Check if this allocation is valid
    bool isValid() const
    {
        // Basic validity checks based on allocation type
        switch (type)
        {
        case AllocaType::None:
            return true;
        case AllocaType::AllocaOnly:
            return allocaInst != nullptr;
        case AllocaType::AllocaAndLoad:
            return allocaInst != nullptr && loadInst != nullptr;
        case AllocaType::AllocaAndStore:
            return allocaInst != nullptr && storeInst != nullptr;
        case AllocaType::AllocaLoadStore:
            return allocaInst != nullptr && storeInst != nullptr && loadInst != nullptr;
        case AllocaType::Global:
            return global != nullptr;
        case AllocaType::PhiNode:
            return phi != nullptr;
        case AllocaType::DynamicArray:
            return allocaInst != nullptr && dynamicSize != nullptr;
        default:
            return allocaInst != nullptr;
        }
    }

    // Add a member access for aggregate types
    void addMemberAccess(llvm::GetElementPtrInst *gep)
    {
        memberAccesses.push_back(gep);
    }

    // Clear all instructions (useful for cleanup)
    void clear()
    {
        allocaInst = nullptr;
        storeInst = nullptr;
        loadInst = nullptr;
        global = nullptr;
        phi = nullptr;
        dynamicSize = nullptr;
        memberAccesses.clear();
    }

    // Add these methods:
    bool needsCleanup() const;
    void generateCleanupCode(llvm::IRBuilder<> &builder);
    llvm::Value *generateLifetimeStart(llvm::IRBuilder<> &builder);
    llvm::Value *generateLifetimeEnd(llvm::IRBuilder<> &builder);
};

typedef struct IRFunctionSymbol
{
    llvm::Function *function;
    llvm::Type *returnType;
    std::string name;

    llvm::BasicBlock *entryBlock;

    std::vector<llvm::Value *> localVariables;
    std::vector<llvm::Value *> allocaVariables;
    std::vector<IRVariableSymbol> parameters;            // Function parameters
    std::map<std::string, IRVariableSymbol> symbolTable; // Local scope symbol table
    bool isVariadic;                                     // For variadic functions
    llvm::FunctionType *functionType;                    // Cache the function type
} IRFunctionSymbol;

typedef struct IRVariableSymbol
{
    llvm::Value *value;
    llvm::Type *type;
    std::string name;
    AllocaType allocaType;
} IRVariableSymbol;

struct IRTypeSymbol
{
    llvm::Type *type;
    std::string name;

    // For aggregate types (structs/classes)
    std::vector<IRPropertySymbol> members;
    std::vector<IRMethodSymbol> methods;

    // For arrays/vectors
    llvm::Type *elementType;
    size_t size; // Fixed size if known

    bool isAggregate() const;
    bool isArray() const;
    size_t getSizeInBytes(llvm::DataLayout &layout) const;
};

struct IRPropertySymbol
{
    llvm::Type *type;
    std::string name;
    size_t offset; // Byte offset within struct
    bool isPublic; // For visibility

    // For class members
    IRMethodSymbol *getter;
    IRMethodSymbol *setter;
};

struct IRMethodSymbol
{
    IRFunctionSymbol function;
    bool isVirtual;
    bool isStatic;
    bool isConstructor;
    bool isDestructor;
    size_t vtableIndex; // For virtual methods

    IRTypeSymbol *parentType; // Owning class/struct
};
