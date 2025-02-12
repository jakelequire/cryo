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

#include "frontend/dataTypes.h"
#include "frontend/AST.h"

namespace Cryo
{

    typedef struct IRFunctionSymbol IRFunctionSymbol;
    typedef struct IRVariableSymbol IRVariableSymbol;
    typedef struct IRTypeSymbol IRTypeSymbol;
    typedef struct IRPropertySymbol IRPropertySymbol;
    typedef struct IRMethodSymbol IRMethodSymbol;
    typedef struct ASTNode ASTnode;
    typedef struct DataType DataType;

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

        // Constructor for parameters
        Allocation(llvm::Value *value)
            : type(AllocaType::Parameter), allocaInst(nullptr), storeInst(nullptr),
              loadInst(nullptr), global(nullptr), phi(nullptr), dynamicSize(nullptr) {}

        Allocation(std::nullptr_t)
            : type(AllocaType::None), allocaInst(nullptr), storeInst(nullptr),
              loadInst(nullptr), global(nullptr), phi(nullptr), dynamicSize(nullptr) {}

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
        llvm::Function *function;         // LLVM function object
        llvm::Type *returnType;           // Cache the return type
        llvm::FunctionType *functionType; // Cache the function type
        std::string name;                 // Function name

        ASTNode *astNode;           // AST node
        DataType *returnDataType;   // Return data type
        DataType *functionDataType; // Function data type

        llvm::BasicBlock *entryBlock;                        // Entry block
        std::vector<llvm::Value *> localVariables;           // Local variables
        std::vector<llvm::Value *> allocaVariables;          // Alloca variables
        std::vector<IRVariableSymbol> parameters;            // Function parameters
        std::map<std::string, IRVariableSymbol> symbolTable; // Local scope symbol table

        bool isVariadic; // For variadic functions
        bool isExternal; // For external functions

        // Constructor to initialize the function symbol
        IRFunctionSymbol(llvm::Function *func, const std::string &nm, llvm::Type *retType,
                         llvm::FunctionType *funcType, llvm::BasicBlock *entry, bool variadic = false,
                         bool external = false)
            : function(func), returnType(retType), functionType(funcType), entryBlock(entry),
              name(nm), astNode(nullptr), returnDataType(nullptr), functionDataType(nullptr),
              isVariadic(variadic), isExternal(external) {}
    } IRFunctionSymbol;

    typedef struct IRVariableSymbol
    {
        llvm::Value *value;
        llvm::Type *type;
        llvm::Function *parentFunction;
        std::string name;

        ASTNode *astNode;
        DataType *dataType;

        AllocaType allocaType;
        Allocation allocation;

        // Constructor to initialize the variable symbol with allocation
        IRVariableSymbol(llvm::Value *val, llvm::Type *typ, const std::string &nm, AllocaType allocType, const Allocation &alloc)
            : value(val), type(typ), name(nm), astNode(nullptr), dataType(nullptr), allocaType(allocType), allocation(alloc) {}

        // Constructor to initialize the variable symbol with parent function (Local variables)
        IRVariableSymbol(llvm::Function *func, llvm::Value *val, llvm::Type *typ, const std::string &nm, AllocaType allocType)
            : value(val), type(typ), parentFunction(func), name(nm), astNode(nullptr), dataType(nullptr), allocaType(allocType), allocation(nullptr) {}

        void createAlloca(llvm::IRBuilder<> &builder);
    } IRVariableSymbol;

    struct IRTypeSymbol
    {
        llvm::Type *type;
        std::string name;

        ASTNode *astNode;
        DataType *dataType;

        // For aggregate types (structs/classes)
        std::vector<IRPropertySymbol> members;
        std::vector<IRMethodSymbol> methods;

        // For arrays/vectors
        llvm::Type *elementType;
        size_t size; // Fixed size if known

        bool isAggregate() const;
        bool isArray() const;
        size_t getSizeInBytes(llvm::DataLayout &layout) const;

        // Get the type of a member
        llvm::Type *getMemberType(const std::string &memberName) const;

        // Get the offset of a member
        size_t getMemberOffset(const std::string &memberName, llvm::DataLayout &layout) const;

        // Constructor to initialize the type symbol
        IRTypeSymbol(llvm::Type *typ, const std::string &nm)
            : type(typ), name(nm), astNode(nullptr), dataType(nullptr), elementType(nullptr), size(0) {}
    };

    struct IRPropertySymbol
    {
        llvm::Type *type;
        std::string name;
        size_t offset; // Byte offset within struct
        bool isPublic; // For visibility

        ASTNode *astNode;
        DataType *dataType;

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

        ASTNode *astNode;
        DataType *dataType;

        IRTypeSymbol *parentType; // Owning class/struct

        // Constructor to initialize the method symbol
        IRMethodSymbol(IRFunctionSymbol func, bool virt, bool stat, bool ctor, bool dtor, size_t vtableIdx, IRTypeSymbol *parent)
            : function(func), isVirtual(virt), isStatic(stat), isConstructor(ctor), isDestructor(dtor),
              vtableIndex(vtableIdx), parentType(parent), astNode(nullptr), dataType(nullptr) {}
    };

    // This class is used to create symbols for the IRSymbolTable.
    // It is suppose to be an easy interface to create symbols.
    class IRSymbolManager
    {
    public:
        IRSymbolManager() = default;
        ~IRSymbolManager() = default;

        // Create a new function symbol
        static IRFunctionSymbol createFunctionSymbol(llvm::Function *function, const std::string &name,
                                                     llvm::Type *returnType, llvm::FunctionType *functionType,
                                                     llvm::BasicBlock *entryBlock, bool isVariadic = false,
                                                     bool isExternal = false);

        // Create a new variable symbol
        static IRVariableSymbol createVariableSymbol(llvm::Value *value, llvm::Type *type, const std::string &name,
                                                     AllocaType allocaType, const Allocation &allocation);

        static IRVariableSymbol createVariableSymbol(llvm::Function *func, llvm::Value *value, llvm::Type *type,
                                                     const std::string &name, AllocaType allocaType);

        // Create a new type symbol
        static IRTypeSymbol createTypeSymbol(llvm::Type *type, const std::string &name);

        // Create a new property symbol
        static IRPropertySymbol createPropertySymbol(llvm::Type *type, const std::string &name,
                                                     size_t offset, bool isPublic = true);

        // Create a new method symbol
        static IRMethodSymbol createMethodSymbol(IRFunctionSymbol function, bool isVirtual = false,
                                                 bool isStatic = false, bool isConstructor = false,
                                                 bool isDestructor = false, size_t vtableIndex = 0,
                                                 IRTypeSymbol *parentType = nullptr);

        // Add a member to a type symbol
        static void addMemberToTypeSymbol(IRTypeSymbol &type, const IRPropertySymbol &member);
        // Add a method to a type symbol
        static void addMethodToTypeSymbol(IRTypeSymbol &type, const IRMethodSymbol &method);
        // Add a parameter to a function symbol
        static void addParameterToFunctionSymbol(IRFunctionSymbol &function, const IRVariableSymbol &parameter);
        // Add a symbol to a symbol table
        static void addSymbolToSymbolTable(IRFunctionSymbol &function, const IRVariableSymbol &symbol);

        Allocation createAlloca(IRVariableSymbol &symbol, llvm::IRBuilder<> &builder);
    };

} // namespace Cryo
