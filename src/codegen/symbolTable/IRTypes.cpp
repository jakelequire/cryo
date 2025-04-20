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
#include "codegen/symTable/IRSymbolTable.hpp"

namespace Cryo
{
    llvm::Type *IRSymbolTable::getLLVMType(DataType *dataType)
    {
        if (!dataType)
        {
            std::cerr << "Data type is null" << std::endl;
            return nullptr;
        }

        bool isPointer = dataType->isPointer;
        bool isReference = dataType->isReference;
        bool isConst = dataType->isConst;

        if (dataType->container->typeOf == POINTER_TYPE)
        {
            logMessage(LMI, "INFO", "IRSymbolTable", "Data type is a pointer");
            logMessage(LMI, "INFO", "IRSymbolTable", "Pointer type: %s", DTM->debug->dataTypeToString(dataType));
            llvm::Type *baseType = getLLVMType(dataType->container->type.pointerType->baseType);
            if (!baseType)
            {
                std::cerr << "Failed to get base type for pointer" << std::endl;
                return nullptr;
            }
            return llvm::PointerType::get(baseType, 0);
        }

        switch (dataType->container->primitive)
        {
        case PRIM_I8:
            return llvmTypes.i8Ty;
        case PRIM_I16:
            return llvmTypes.i16Ty;
        case PRIM_I32:
            return llvmTypes.i32Ty;
        case PRIM_I64:
            return llvmTypes.i64Ty;
        case PRIM_I128:
            return llvmTypes.i128Ty;
        case PRIM_FLOAT:
            return llvmTypes.floatTy;
        case PRIM_CHAR:
            return llvmTypes.i8Ty;
        case PRIM_STR:
            return llvmTypes.ptrTy;
        case PRIM_BOOLEAN:
            return llvmTypes.i1Ty;
        case PRIM_VOID:
            return llvmTypes.voidTy;
        case PRIM_NULL:
            return llvmTypes.ptrTy;
        case PRIM_ANY:
            return llvmTypes.ptrTy;
        case PRIM_OBJECT:
            return getLLVMObjectType(dataType);
        case PRIM_FUNCTION:
            return getLLVMFunctionType(dataType);
        case PRIM_AUTO:
            return nullptr;
        case PRIM_UNDEFINED:
            return nullptr;
        case PRIM_UNKNOWN:
            return nullptr;
        default:
        {
            std::cerr << "Unknown data type" << std::endl;
            std::string typeName = dataType->typeName;
            std::string primitiveStr = DTM->debug->primitiveDataTypeToString(dataType->container->primitive);
            std::cerr << "Data type: " << typeName << ", Primitive: " << primitiveStr << std::endl;
            return nullptr;
        }
        }
    }

    std::vector<llvm::Type *> IRSymbolTable::getLLVMTypes(DataType **dataTypes)
    {
    }

    llvm::StructType *IRSymbolTable::getLLVMObjectType(DataType *dataType)
    {
        if (!dataType)
        {
            std::cerr << "Data type is null" << std::endl;
            return nullptr;
        }

        if (dataType->container->primitive != PRIM_OBJECT)
        {
            std::cerr << "Data type is not an object" << std::endl;
            return nullptr;
        }

        TypeofObjectType typeOfObject = dataType->container->objectType;
        switch (typeOfObject)
        {
        case STRUCT_OBJ:
            return getLLVMStructType(dataType);
        case CLASS_OBJ:
            return getLLVMClassType(dataType);
        case INTERFACE_OBJ:
            return nullptr;
        case OBJECT_OBJ:
            return nullptr;
        case VA_ARGS_OBJ:
            return getVA_ARGSType(dataType);
        case NON_OBJECT:
            return nullptr;
        case UNKNOWN_OBJECT:
            return nullptr;
        default:
        {
            std::cerr << "Unknown object type" << std::endl;
            return nullptr;
        }
        }
    }

    // Struct Types
    llvm::StructType *IRSymbolTable::getLLVMStructType(DataType *dataType)
    {
        if (!dataType)
        {
            std::cerr << "Data type is null" << std::endl;
            return nullptr;
        }
        if (dataType->container->primitive != PRIM_OBJECT)
        {
            std::cerr << "Data type is not an object" << std::endl;
            return nullptr;
        }

        // Check if the struct already exists in the current module via the symbol table
        std::string structName = "struct." + std::string(dataType->container->type.structType->name);
        IRTypeSymbol *typeSymbol = this->findType(structName);
        if (typeSymbol)
        {
            return typeSymbol->type.structTy;
        }
        else
        {
            // Don't create the new type, just return nullptr
            logMessage(LMI, "INFO", "IRSymbolTable", "Struct type %s not found in symbol table", structName.c_str());
            // Print the symbol table
            debugPrint();
            CONDITION_FAILED;
        }
    }

    // Class Types
    llvm::StructType *IRSymbolTable::getLLVMClassType(DataType *dataType)
    {
    }

    // Function Types
    llvm::FunctionType *IRSymbolTable::getLLVMFunctionType(DataType *dataType)
    {
        if (!dataType)
        {
            std::cerr << "Data type is null" << std::endl;
            return nullptr;
        }
        if (dataType->container->typeOf != PRIM_FUNCTION)
        {
            std::cerr << "Data type is not a function" << std::endl;
            return nullptr;
        }

        logMessage(LMI, "INFO", "IRSymbolTable", "Creating LLVM function type for data type: %s",
                   DTM->debug->dataTypeToString(dataType));

        llvm::FunctionType *llvmFuncType = llvm::FunctionType::get(
            getLLVMType(dataType->container->type.functionType->returnType),
            getLLVMTypes(dataType->container->type.functionType->paramTypes),
            false);

        if (!llvmFuncType)
        {
            std::cerr << "Failed to create LLVM function type" << std::endl;
            return nullptr;
        }

        logMessage(LMI, "INFO", "IRSymbolTable", "LLVM function type created successfully");
        return llvmFuncType;
    }

    // VA_ARGS Types
    llvm::StructType *IRSymbolTable::getVA_ARGSType(DataType *dataType)
    {
        DEBUG_BREAKPOINT;
    }

    void IRSymbolTable::initLLVMTypes()
    {
        llvmTypes.i1Ty = llvm::Type::getInt1Ty(context.getInstance().context);
        llvmTypes.i8Ty = llvm::Type::getInt8Ty(context.getInstance().context);
        llvmTypes.i16Ty = llvm::Type::getInt16Ty(context.getInstance().context);
        llvmTypes.i32Ty = llvm::Type::getInt32Ty(context.getInstance().context);
        llvmTypes.i64Ty = llvm::Type::getInt64Ty(context.getInstance().context);
        llvmTypes.i128Ty = llvm::Type::getInt128Ty(context.getInstance().context);

        llvmTypes.halfTy = llvm::Type::getHalfTy(context.getInstance().context);
        llvmTypes.floatTy = llvm::Type::getFloatTy(context.getInstance().context);
        llvmTypes.doubleTy = llvm::Type::getDoubleTy(context.getInstance().context);
        llvmTypes.fp128Ty = llvm::Type::getFP128Ty(context.getInstance().context);
        llvmTypes.x86_fp80 = llvm::Type::getX86_FP80Ty(context.getInstance().context);

        llvmTypes.voidTy = llvm::Type::getVoidTy(context.getInstance().context);
        llvmTypes.ptrTy = llvm::Type::getInt8Ty(context.getInstance().context)->getPointerTo();

        this->typesInitialized = true;
    }

} // namespace Cryo