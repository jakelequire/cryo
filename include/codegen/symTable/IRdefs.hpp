/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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

#include "dataTypes/dataTypes.h"
#include "frontend/AST.h"

namespace Cryo
{

    typedef struct IRFunctionSymbol IRFunctionSymbol;
    typedef struct IRVariableSymbol IRVariableSymbol;
    typedef struct IRTypeSymbol IRTypeSymbol;
    typedef struct IRPropertySymbol IRPropertySymbol;
    typedef struct IRMethodSymbol IRMethodSymbol;
    typedef struct ASTNode ASTnode;
    typedef struct DataType_t DataType;

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

    std::string AllocaTypeToString(AllocaType allocaType);

    struct Allocation
    {
        AllocaType type = AllocaType::None;

        // Basic allocation instructions
        llvm::AllocaInst *allocaInst = nullptr;
        llvm::StoreInst *storeInst = nullptr;
        llvm::LoadInst *loadInst = nullptr;

        // Global variable (for global allocations)
        llvm::GlobalVariable *global = nullptr;

        // For phi nodes in SSA form
        llvm::PHINode *phi = nullptr;

        // For dynamic arrays
        llvm::Value *dynamicSize = nullptr;

        // For parameters and other direct values
        llvm::Value *value = nullptr;

        // For aggregates, store the member accesses
        std::vector<llvm::GetElementPtrInst *> memberAccesses;

        // Constructors with builder pattern
        static Allocation createLocal(llvm::IRBuilder<> &builder, llvm::Type *type,
                                      const std::string &name, llvm::Value *initialValue = nullptr)
        {
            Allocation alloc;
            alloc.type = initialValue ? AllocaType::AllocaAndStore : AllocaType::AllocaOnly;
            alloc.allocaInst = builder.CreateAlloca(type, nullptr, name + ".alloca");

            if (initialValue)
            {
                alloc.storeInst = builder.CreateStore(initialValue, alloc.allocaInst);
            }

            alloc.value = alloc.allocaInst;

            return alloc;
        }

        static Allocation createGlobal(llvm::Module *module, llvm::Type *type,
                                       const std::string &name, llvm::Constant *initialValue = nullptr)
        {
            Allocation alloc;
            alloc.type = AllocaType::Global;
            alloc.global = new llvm::GlobalVariable(
                *module, type, false,
                llvm::GlobalValue::ExternalLinkage,
                initialValue ? initialValue : llvm::Constant::getNullValue(type),
                name);
            return alloc;
        }

        static Allocation createParameter(llvm::Value *param)
        {
            Allocation alloc;
            alloc.type = AllocaType::Parameter;
            alloc.value = param;
            return alloc;
        }

        static Allocation createDynamicArray(llvm::IRBuilder<> &builder, llvm::Type *elementType,
                                             llvm::Value *size, const std::string &name)
        {
            Allocation alloc;
            alloc.type = AllocaType::DynamicArray;
            alloc.dynamicSize = size;
            alloc.allocaInst = builder.CreateAlloca(
                llvm::ArrayType::get(elementType, 0),
                size,
                name);
            return alloc;
        }

        bool isValid() const
        {
            return type != AllocaType::None;
        }

        static llvm::Value *getDynamicArraySize(llvm::IRBuilder<> &builder, llvm::Value *array)
        {
            llvm::Value *size = builder.CreateExtractValue(array, 0);
            return builder.CreateIntCast(size, builder.getInt64Ty(), false);
        }

        // Value access methods
        llvm::Value *getValue() const
        {
            switch (type)
            {
            case AllocaType::None:
                return nullptr;
            case AllocaType::AllocaOnly:
            case AllocaType::AllocaAndStore:
                return allocaInst;
            case AllocaType::AllocaAndLoad:
            case AllocaType::AllocaLoadStore:
                return loadInst;
            case AllocaType::Global:
                return global;
            case AllocaType::Parameter:
                return value;
            case AllocaType::PhiNode:
                return phi;
            case AllocaType::DynamicArray:
                return allocaInst;
            default:
                std::cout << "@Allocation::getValue: Invalid allocation type" << std::endl;
                return nullptr;
            }
        }

        void setValue(llvm::Value *value)
        {
            this->value = value;
        }

        llvm::Value *getPointer() const
        {
            switch (type)
            {
            case AllocaType::Global:
                return global;
            case AllocaType::Parameter:
                return value;
            default:
                return allocaInst;
            }
        }

        // Mutation methods
        void load(llvm::IRBuilder<> &builder, const std::string &name = "")
        {
            if (!allocaInst && !global)
                return;

            llvm::Value *ptr = getPointer();
            llvm::Type *type = ptr->getType();
            loadInst = builder.CreateLoad(type, ptr, name);

            if (this->type == AllocaType::AllocaOnly)
            {
                this->type = AllocaType::AllocaAndLoad;
            }
            else if (this->type == AllocaType::AllocaAndStore)
            {
                this->type = AllocaType::AllocaLoadStore;
            }
        }

        void store(llvm::IRBuilder<> &builder, llvm::Value *value)
        {
            if (!allocaInst && !global)
                return;

            storeInst = builder.CreateStore(value, getPointer());

            if (this->type == AllocaType::AllocaOnly)
            {
                this->type = AllocaType::AllocaAndStore;
            }
            else if (this->type == AllocaType::AllocaAndLoad)
            {
                this->type = AllocaType::AllocaLoadStore;
            }
        }

        // Member access for aggregate types
        llvm::Value *accessMember(llvm::IRBuilder<> &builder, unsigned idx, const std::string &name = "")
        {
            if (!allocaInst)
                return nullptr;

            auto *gep = builder.CreateStructGEP(allocaInst->getAllocatedType(), allocaInst, idx, name);
            memberAccesses.push_back(llvm::dyn_cast<llvm::GetElementPtrInst>(gep));
            return gep;
        }

        // Array indexing
        llvm::Value *getArrayElement(llvm::IRBuilder<> &builder, llvm::Value *idx, const std::string &name = "")
        {
            if (!allocaInst)
                return nullptr;

            llvm::Value *indices[] = {
                llvm::ConstantInt::get(builder.getInt32Ty(), 0),
                idx};

            return builder.CreateGEP(allocaInst->getAllocatedType(), allocaInst, indices, name);
        }

        // Cleanup and lifetime management
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

        bool needsCleanup() const
        {
            return type == AllocaType::DynamicArray ||
                   (type == AllocaType::Aggregate && !memberAccesses.empty());
        }

        void generateCleanupCode(llvm::IRBuilder<> &builder)
        {
            if (!needsCleanup())
                return;

            if (type == AllocaType::DynamicArray)
            {
                // Generate array cleanup code
                if (allocaInst)
                {
                    // Add array deallocation if needed
                }
            }

            // Clear all instructions
            clear();
        }

        // Lifetime intrinsics
        llvm::Value *generateLifetimeStart(llvm::IRBuilder<> &builder)
        {
            if (!allocaInst)
                return nullptr;

            llvm::Function *lifetimeStart = llvm::Intrinsic::getDeclaration(
                builder.GetInsertBlock()->getModule(),
                llvm::Intrinsic::lifetime_start);

            llvm::Value *size = llvm::ConstantInt::get(builder.getInt64Ty(), -1);
            return builder.CreateCall(lifetimeStart, {size, allocaInst});
        }

        llvm::Value *generateLifetimeEnd(llvm::IRBuilder<> &builder)
        {
            if (!allocaInst)
                return nullptr;

            llvm::Function *lifetimeEnd = llvm::Intrinsic::getDeclaration(
                builder.GetInsertBlock()->getModule(),
                llvm::Intrinsic::lifetime_end);

            llvm::Value *size = llvm::ConstantInt::get(builder.getInt64Ty(), -1);
            return builder.CreateCall(lifetimeEnd, {size, allocaInst});
        }

        static std::string allocaTypeToString(AllocaType type)
        {
            switch (type)
            {
            case AllocaType::None:
                return "None";
            case AllocaType::AllocaOnly:
                return "AllocaOnly";
            case AllocaType::AllocaAndLoad:
                return "AllocaAndLoad";
            case AllocaType::AllocaAndStore:
                return "AllocaAndStore";
            case AllocaType::AllocaLoadStore:
                return "AllocaLoadStore";
            case AllocaType::Global:
                return "Global";
            case AllocaType::Parameter:
                return "Parameter";
            case AllocaType::Temporary:
                return "Temporary";
            case AllocaType::Aggregate:
                return "Aggregate";
            case AllocaType::PhiNode:
                return "PhiNode";
            case AllocaType::DynamicArray:
                return "DynamicArray";
            case AllocaType::StackSave:
                return "StackSave";
            default:
                return "Unknown";
            }
        }
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

        llvm::Function *getFunction() const
        {
            return function;
        }

        // Default constructor
        IRFunctionSymbol()
            : function(nullptr), returnType(nullptr), functionType(nullptr), name(""),
              astNode(nullptr), returnDataType(nullptr), functionDataType(nullptr),
              entryBlock(nullptr), isVariadic(false), isExternal(false) {}

        // Constructor to initialize the function symbol
        IRFunctionSymbol(llvm::Function *func, const std::string &nm, llvm::Type *retType,
                         llvm::FunctionType *funcType, llvm::BasicBlock *entry, bool variadic = false,
                         bool external = false)
            : function(func), returnType(retType), functionType(funcType), entryBlock(entry),
              name(nm), astNode(nullptr), returnDataType(nullptr), functionDataType(nullptr),
              isVariadic(variadic), isExternal(external) {}

        // Copy constructor
        IRFunctionSymbol(const IRFunctionSymbol &other)
            : function(other.function),
              returnType(other.returnType),
              functionType(other.functionType),
              name(other.name),
              astNode(other.astNode),
              returnDataType(other.returnDataType),
              functionDataType(other.functionDataType),
              entryBlock(other.entryBlock),
              localVariables(other.localVariables),
              allocaVariables(other.allocaVariables),
              parameters(other.parameters),
              symbolTable(other.symbolTable),
              isVariadic(other.isVariadic),
              isExternal(other.isExternal) {}

        // Copy assignment operator
        IRFunctionSymbol &operator=(const IRFunctionSymbol &other)
        {
            if (this != &other)
            {
                function = other.function;
                returnType = other.returnType;
                functionType = other.functionType;
                name = other.name;
                astNode = other.astNode;
                returnDataType = other.returnDataType;
                functionDataType = other.functionDataType;
                entryBlock = other.entryBlock;
                localVariables = other.localVariables;
                allocaVariables = other.allocaVariables;
                parameters = other.parameters;
                symbolTable = other.symbolTable;
                isVariadic = other.isVariadic;
                isExternal = other.isExternal;
            }
            return *this;
        }

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

        // Constructor for variables with allocation
        IRVariableSymbol(llvm::Value *val, llvm::Type *typ, const std::string &nm,
                         AllocaType allocType, const Allocation &alloc)
            : value(val), type(typ), parentFunction(nullptr), name(nm), astNode(nullptr),
              dataType(nullptr), allocaType(allocType), allocation(alloc) {}

        // Constructor for local variables with parent function
        IRVariableSymbol(llvm::Function *func, llvm::Value *val, llvm::Type *typ,
                         const std::string &nm, AllocaType allocType)
            : value(val), type(typ), parentFunction(func), name(nm), astNode(nullptr),
              dataType(nullptr), allocaType(allocType) {}

        // Constructor for function parameters
        IRVariableSymbol(llvm::Argument *arg, llvm::Type *typ, const std::string &nm,
                         AllocaType allocType)
            : value(arg), type(typ), parentFunction(nullptr), name(nm), astNode(nullptr),
              dataType(nullptr), allocaType(allocType) {}

        // Copy constructor
        IRVariableSymbol(const IRVariableSymbol &other)
            : value(other.value),
              type(other.type),
              parentFunction(other.parentFunction),
              name(other.name),
              astNode(other.astNode),
              dataType(other.dataType),
              allocaType(other.allocaType),
              allocation(other.allocation) {}

        // Copy assignment operator
        IRVariableSymbol &operator=(const IRVariableSymbol &other)
        {
            if (this != &other)
            {
                value = other.value;
                type = other.type;
                parentFunction = other.parentFunction;
                name = other.name;
                astNode = other.astNode;
                dataType = other.dataType;
                allocaType = other.allocaType;
                allocation = other.allocation;
            }
            return *this;
        }

    } IRVariableSymbol;

    enum class IRTypeKind
    {
        Primitive,
        Aggregate,
        Array,
        Vector,
        Function,
        Pointer,
        Object,
        Class,
        Struct,
        Enum,
    };

    struct IRTypeSymbol
    {

        IRTypeKind kind;
        union type
        {
            llvm::Type *typeTy;             // LLVM type
            llvm::StructType *structTy;     // Struct type
            llvm::FunctionType *functionTy; // Function type
            llvm::PointerType *pointerTy;   // Pointer type

            // Default constructor for the union
            type() : typeTy(nullptr) {}

            llvm::Type *getLLVMType()
            {
                if (typeTy)
                    return typeTy;
                if (structTy)
                    return llvm::cast<llvm::Type>(structTy);
                if (functionTy)
                    return llvm::cast<llvm::Type>(functionTy);
                if (pointerTy)
                    return llvm::cast<llvm::Type>(pointerTy);
                return nullptr;
            }
        } type;

        llvm::Type *getType() const
        {
            if (type.typeTy)
                return type.typeTy;
            if (type.structTy)
                return llvm::cast<llvm::Type>(type.structTy);
            if (type.functionTy)
                return llvm::cast<llvm::Type>(type.functionTy);
            if (type.pointerTy)
                return llvm::cast<llvm::Type>(type.pointerTy);
            return nullptr;
        }

        std::string name;

        ASTNode *astNode;
        DataType *dataType;

        // For aggregate types (structs/classes)
        std::vector<IRPropertySymbol> members;
        std::vector<IRMethodSymbol> methods;

        // For arrays/vectors
        llvm::Type *elementType;
        size_t size; // Fixed size if known

        bool isPrototype = false; // For prototype types

        bool isAggregate() const;
        bool isArray() const;
        size_t getSizeInBytes(llvm::DataLayout &layout) const;

        // Get the type of a member
        llvm::Type *getMemberType(const std::string &memberName) const;

        // Get the offset of a member
        size_t getMemberOffset(const std::string &memberName, llvm::DataLayout &layout) const;

        // Default constructor (add this before your other constructors)
        IRTypeSymbol()
            : kind(IRTypeKind::Primitive), name(""), astNode(nullptr), dataType(nullptr),
              elementType(nullptr), size(0)
        {
            // The union's default constructor will be called automatically
            isPrototype = false;
        }

        // Constructor to initialize the type symbol
        IRTypeSymbol(llvm::Type *typ, const std::string &nm)
            : kind(IRTypeKind::Primitive), name(nm), astNode(nullptr), dataType(nullptr),
              elementType(nullptr), size(0)
        {
            type.typeTy = typ; // Explicitly initialize the union member
            isPrototype = false;
        }

        // Constructor for aggregate types
        IRTypeSymbol(llvm::StructType *typ, const std::string &nm,
                     const std::vector<IRPropertySymbol> &mems,
                     const std::vector<IRMethodSymbol> &meths)
            : kind(IRTypeKind::Aggregate), name(nm), astNode(nullptr), dataType(nullptr),
              members(mems), methods(meths), elementType(nullptr), size(0)
        {
            type.structTy = typ; // Explicitly initialize the union member
            isPrototype = false;
        }

        // Constructor for function types
        IRTypeSymbol(llvm::FunctionType *typ, const std::string &nm)
            : kind(IRTypeKind::Function), name(nm), astNode(nullptr), dataType(nullptr),
              elementType(nullptr), size(0)
        {
            type.functionTy = typ; // Explicitly initialize the union member
            isPrototype = false;
        }

        // Constructor for prototype types
        IRTypeSymbol(llvm::StructType *typ, const std::string &nm, bool isProto)
            : kind(IRTypeKind::Aggregate), name(nm), astNode(nullptr), dataType(nullptr),
              elementType(nullptr), size(0)
        {
            type.structTy = typ; // Explicitly initialize the union member
            isPrototype = isProto;
        }

        // Copy constructor
        IRTypeSymbol(const IRTypeSymbol &other)
            : kind(other.kind),
              type(other.type), // Copy the union using its copy constructor
              name(other.name),
              astNode(other.astNode),
              dataType(other.dataType),
              members(other.members),
              methods(other.methods),
              elementType(other.elementType),
              size(other.size),
              isPrototype(other.isPrototype)
        {
            // The union's copy constructor will be called automatically
        }

        // Copy assignment operator
        IRTypeSymbol &operator=(const IRTypeSymbol &other)
        {
            if (this != &other)
            {
                kind = other.kind;
                type = other.type; // Copy the union using its copy assignment
                name = other.name;
                astNode = other.astNode;
                dataType = other.dataType;
                members = other.members;
                methods = other.methods;
                elementType = other.elementType;
                size = other.size;
                isPrototype = other.isPrototype;
            }
            return *this;
        }
    };

    class AllocaTypeInference
    {
    public:
        static AllocaType inferFromNode(ASTNode *node, bool isGlobal = false)
        {
            if (!node)
                return AllocaType::None;
            if (isGlobal)
                return AllocaType::Global;

            switch (node->metaData->type)
            {
            case NODE_VAR_DECLARATION:
                return inferFromVarDecl(node->data.varDecl);
            case NODE_PARAM:
                return inferFromParam(node->data.param);
            case NODE_ARRAY_LITERAL:
                return inferFromArray(node->data.array);
            case NODE_STRUCT_DECLARATION:
                return AllocaType::Aggregate;
            case NODE_CLASS:
                return AllocaType::Aggregate;
            case NODE_LITERAL_EXPR:
                return inferFromLiteral(node->data.literal, false);
            case NODE_STRING_LITERAL:
                return AllocaType::AllocaAndLoad;
            case NODE_VAR_NAME:
                return AllocaType::AllocaLoadStore;
            case NODE_FUNCTION_CALL:
            case NODE_METHOD_CALL:
                return AllocaType::AllocaAndStore;
            case NODE_BINARY_EXPR:
                return AllocaType::AllocaAndStore;
            case NODE_UNARY_EXPR:
                return AllocaType::AllocaAndStore;
            case NODE_INDEX_EXPR:
                return AllocaType::AllocaLoadStore;
            case NODE_PROPERTY_ACCESS:
                return AllocaType::AllocaLoadStore;
            case NODE_RETURN_STATEMENT:
                return AllocaType::AllocaAndStore;
            case NODE_BLOCK:
                return AllocaType::AllocaOnly;
            case NODE_IF_STATEMENT:
                return AllocaType::AllocaOnly;
            case NODE_FOR_STATEMENT:
                return AllocaType::AllocaOnly;
            case NODE_WHILE_STATEMENT:
                return AllocaType::AllocaOnly;
            case NODE_PROPERTY:
                return AllocaType::AllocaOnly;
            case NODE_METHOD:
                return AllocaType::AllocaOnly;
            case NODE_STRUCT_CONSTRUCTOR:
            case NODE_CLASS_CONSTRUCTOR:
                return AllocaType::AllocaOnly;
            default:
                return AllocaType::None;
            }
        }

    private:
        static AllocaType inferFromVarDecl(CryoVariableNode *varDecl)
        {
            if (!varDecl)
                return AllocaType::None;

            // Handle parameters
            if (varDecl->isIterator)
            {
                return AllocaType::Parameter;
            }

            // Check variable modifiers
            if (varDecl->isGlobal)
            {
                return AllocaType::Global;
            }

            if (varDecl->isReference)
            {
                return AllocaType::Parameter; // References are handled like parameters
            }

            // Check data type characteristics
            if (varDecl->type)
            {
                if (varDecl->type->isArray)
                {
                    // return varDecl->type->container->custom.arrayDef->dimensions > 0
                    //            ? AllocaType::DynamicArray
                    //            : AllocaType::Aggregate;
                }

                switch (varDecl->type->container->typeOf)
                {
                case OBJECT_TYPE:
                    return AllocaType::Aggregate;
                default:
                    break;
                }
            }

            // If there's no initializer, we only need allocation
            if (!varDecl->noInitializer)
            {
                return AllocaType::AllocaOnly;
            }

            // Handle initialization
            return inferFromInitializer(varDecl->initializer, !varDecl->isMutable);
        }

        static AllocaType inferFromParam(CryoParameterNode *param)
        {
            if (!param)
                return AllocaType::None;

            // Check if parameter is mutable
            if (param->isMutable)
            {
                return AllocaType::AllocaLoadStore;
            }

            // For parameters with default values
            if (param->hasDefaultValue)
            {
                return AllocaType::AllocaAndStore;
            }

            return AllocaType::Parameter;
        }

        static AllocaType inferFromArray(CryoArrayNode *array)
        {
            if (!array)
                return AllocaType::None;

            // Check if it's a dynamic array
            if (array->type && array->type->isArray)
            {
                return AllocaType::DynamicArray;
            }

            return AllocaType::Aggregate;
        }

        static AllocaType inferFromInitializer(ASTNode *initializer, bool isConst)
        {
            if (!initializer)
                return AllocaType::AllocaOnly;

            switch (initializer->metaData->type)
            {
            case NODE_LITERAL_EXPR:
                return inferFromLiteral(initializer->data.literal, isConst);

            case NODE_VAR_NAME:
                return AllocaType::AllocaLoadStore;

            case NODE_FUNCTION_CALL:
            case NODE_METHOD_CALL:
                return AllocaType::AllocaAndStore;

            case NODE_BINARY_EXPR:
                return AllocaType::AllocaAndStore;

            case NODE_ARRAY_LITERAL:
                return AllocaType::DynamicArray;

            case NODE_INDEX_EXPR:
                return AllocaType::AllocaLoadStore;

            case NODE_OBJECT_INST:
                return AllocaType::Aggregate;

            case NODE_PROPERTY_ACCESS:
                return AllocaType::AllocaLoadStore;

            case NODE_PARAM:
                return AllocaType::Parameter;

            default:
                return AllocaType::AllocaOnly;
            }
        }

        static AllocaType inferFromLiteral(LiteralNode *literal, bool isConst)
        {
            if (!literal)
                return AllocaType::None;

            // Special handling for strings since they're effectively arrays
            if (literal->type && DTM->validation->isStringType(literal->type))
            {
                return AllocaType::Aggregate;
            }

            // Constants with literals only need store
            return isConst ? AllocaType::AllocaAndStore : AllocaType::AllocaLoadStore;
        }

    private:
        static bool isAggregateType(DataType *type)
        {
            if (!type)
                return false;
            return type->container->typeOf == OBJECT_TYPE ||
                   (type->isArray);
        }

        static bool isDynamicArray(DataType *type)
        {
            if (!type)
                return false;
            return type->isArray;
        }
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

        // Copy constructor
        IRMethodSymbol(const IRMethodSymbol &other)
            : function(other.function), isVirtual(other.isVirtual), isStatic(other.isStatic),
              isConstructor(other.isConstructor), isDestructor(other.isDestructor),
              vtableIndex(other.vtableIndex), parentType(other.parentType),
              astNode(other.astNode), dataType(other.dataType) {}

        // Copy assignment operator
        IRMethodSymbol &operator=(const IRMethodSymbol &other)
        {
            if (this != &other)
            {
                function = other.function;
                isVirtual = other.isVirtual;
                isStatic = other.isStatic;
                isConstructor = other.isConstructor;
                isDestructor = other.isDestructor;
                vtableIndex = other.vtableIndex;
                parentType = other.parentType;
                astNode = other.astNode;
                dataType = other.dataType;
            }
            return *this;
        }
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

        // Create a new variable symbol (Stack or global)
        static IRVariableSymbol createVariableSymbol(llvm::Value *value, llvm::Type *type, const std::string &name,
                                                     AllocaType allocaType, const Allocation &allocation);

        // Create a new variable symbol (Stack or global) with parent function
        static IRVariableSymbol createVariableSymbol(llvm::Function *func, llvm::Value *value, llvm::Type *type,
                                                     const std::string &name, AllocaType allocaType);

        // Create a new variable symbol from function parameter(s)
        static IRVariableSymbol createParameterSymbol(llvm::Function *func, llvm::Value *value, llvm::Type *type,
                                                      const std::string &name, AllocaType allocaType);

        // Create a new type symbol
        static IRTypeSymbol createTypeSymbol(llvm::Type *type, const std::string &name);

        static IRTypeSymbol createTypeSymbol(llvm::StructType *type, const std::string &name,
                                             const std::vector<IRPropertySymbol> &members,
                                             const std::vector<IRMethodSymbol> &methods);

        static IRTypeSymbol createProtoTypeSymbol(llvm::StructType *type, const std::string &name, bool isProto);

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
