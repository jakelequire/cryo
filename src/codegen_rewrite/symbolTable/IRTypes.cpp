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
#include "codegen_rewrite/symTable/IRSymbolTable.hpp"

namespace Cryo
{
    llvm::Type *IRSymbolTable::getLLVMType(DataType *dataType)
    {
        if (!dataType)
        {
            std::cerr << "Data type is null" << std::endl;
            return nullptr;
        }

        switch (dataType->container->typeOf)
        {
        case PRIM_INT:
            return llvmTypes.i32Ty;
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
        case PRIM_STRING:
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

        if (dataType->container->typeOf != PRIM_OBJECT)
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
        if (dataType->container->typeOf != PRIM_OBJECT)
        {
            std::cerr << "Data type is not an object" << std::endl;
            return nullptr;
        }

        // Check if the struct already exists in the current module via the symbol table
        std::string structName = dataType->container->type.structType->name;
        IRTypeSymbol *typeSymbol = this->findType(structName);
        if (typeSymbol)
        {
            return typeSymbol->type.structTy;
        }
        // Create a new struct type
        llvm::StructType *llvmStructType = llvm::StructType::create(currentModule->getContext(), structName);
        if (!llvmStructType)
        {
            std::cerr << "Failed to create LLVM struct type" << std::endl;
            return nullptr;
        }

        // Set the struct body
        std::vector<llvm::Type *> memberTypes;
        for (size_t i = 0; i < dataType->container->type.structType->propertyCount; ++i)
        {
            DataType *memberType = dataType->container->type.structType->properties[i]->type;
            llvm::Type *llvmMemberType = getLLVMType(memberType);
            if (!llvmMemberType)
            {
                std::cerr << "Failed to get LLVM type for member" << std::endl;
                return nullptr;
            }
            memberTypes.push_back(llvmMemberType);
        }

        llvmStructType->setBody(memberTypes);
        if (!llvmStructType->isLayoutIdentical(llvmStructType))
        {
            std::cerr << "Failed to set LLVM struct type body" << std::endl;
            return nullptr;
        }
        logMessage(LMI, "INFO", "IRSymbolTable", "LLVM struct type created successfully");
        // Add the struct type to the symbol table
        IRTypeSymbol tySymbol(llvmStructType, structName, std::vector<IRPropertySymbol>(),
                                std::vector<IRMethodSymbol>());
        if (!addType(tySymbol))
        {
            std::cerr << "Failed to add struct type to symbol table" << std::endl;
            return nullptr;
        }
        logMessage(LMI, "INFO", "IRSymbolTable", "Struct type added to symbol table successfully");
        // Return the LLVM struct type
        return llvmStructType;
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

    void IRSymbolTable::initLLVMTypes()
    {
        llvmTypes.i1Ty = llvm::Type::getInt1Ty(currentModule->getContext());
        llvmTypes.i8Ty = llvm::Type::getInt8Ty(currentModule->getContext());
        llvmTypes.i16Ty = llvm::Type::getInt16Ty(currentModule->getContext());
        llvmTypes.i32Ty = llvm::Type::getInt32Ty(currentModule->getContext());
        llvmTypes.i64Ty = llvm::Type::getInt64Ty(currentModule->getContext());
        llvmTypes.i128Ty = llvm::Type::getInt128Ty(currentModule->getContext());

        llvmTypes.halfTy = llvm::Type::getHalfTy(currentModule->getContext());
        llvmTypes.floatTy = llvm::Type::getFloatTy(currentModule->getContext());
        llvmTypes.doubleTy = llvm::Type::getDoubleTy(currentModule->getContext());
        llvmTypes.fp128Ty = llvm::Type::getFP128Ty(currentModule->getContext());
        llvmTypes.x86_fp80 = llvm::Type::getX86_FP80Ty(currentModule->getContext());

        llvmTypes.voidTy = llvm::Type::getVoidTy(currentModule->getContext());
        llvmTypes.ptrTy = llvm::Type::getInt64Ty(currentModule->getContext());

        this->typesInitialized = true;
    }

} // namespace Cryo
