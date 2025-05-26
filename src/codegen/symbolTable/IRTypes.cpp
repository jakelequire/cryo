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
#include "codegen/symTable/IRSymbolTable.hpp"

namespace Cryo
{
    llvm::Type *IRSymbolTable::getLLVMType(DataType *dataType)
    {
        if (!dataType)
        {
            std::cerr << "@getLLVMType Data type is null" << std::endl;
            return nullptr;
        }

        bool isPointer = dataType->isPointer;
        bool isReference = dataType->isReference;
        bool isConst = dataType->isConst;

        if (dataType->container->typeOf == POINTER_TYPE)
        {
            logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMType Data type is a pointer");
            logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMType Pointer type: %s", DTM->debug->dataTypeToString(dataType));
            llvm::Type *baseType = getLLVMType(dataType->container->type.pointerType->baseType);
            if (!baseType)
            {
                std::cerr << "@getLLVMType Failed to get base type for pointer" << std::endl;
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
        case PRIM_ARRAY:
        {
            DataType *baseDataType = dataType->container->type.arrayType->baseType;
            if (baseDataType->container->primitive == PRIM_STR &&
                dataType->container->typeOf == ARRAY_TYPE &&
                dataType->container->type.arrayType->isDynamic &&
                dataType->container->type.arrayType->elementCount <= 0 &&
                dataType->container->type.arrayType->size <= 0)
            {
                logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMType Data type is a dynamic string array");
                llvm::PointerType *llvmPtrType = llvm::PointerType::get(llvmTypes.i8Ty, 0);
                return llvmPtrType;
            }
            return getLLVMArrayType(dataType);
        }
        case PRIM_AUTO:
            return nullptr;
        case PRIM_UNDEFINED:
            return nullptr;
        case PRIM_UNKNOWN:
            return nullptr;
        default:
        {
            std::cerr << "@getLLVMType Unknown data type" << std::endl;
            std::string typeName = dataType->typeName;
            std::string primitiveStr = DTM->debug->primitiveDataTypeToString(dataType->container->primitive);
            std::cerr << "Data type: " << typeName << ", Primitive: " << primitiveStr << std::endl;
            return nullptr;
        }
        }
    }

    std::vector<llvm::Type *> IRSymbolTable::getLLVMTypes(DataType **dataTypes)
    {
        if (!dataTypes)
        {
            std::cerr << "Data types array is null" << std::endl;
            return {};
        }

        std::vector<llvm::Type *> llvmTypes;
        for (int i = 0; dataTypes[i] != nullptr; i++)
        {
            llvm::Type *llvmType = getLLVMType(dataTypes[i]);
            if (llvmType)
            {
                llvmTypes.push_back(llvmType);
            }
            else
            {
                std::cerr << "Failed to get LLVM type for data type: " << DTM->debug->dataTypeToString(dataTypes[i]) << std::endl;
            }
        }
        return llvmTypes;
    }

    llvm::ArrayType *IRSymbolTable::getLLVMArrayType(DataType *dataType)
    {
        if (!dataType)
        {
            std::cerr << "Data type is null" << std::endl;
            CONDITION_FAILED;
            return nullptr;
        }
        if (dataType->container->primitive != PRIM_ARRAY)
        {
            std::cerr << "Data type is not an array" << std::endl;
            CONDITION_FAILED;
            return nullptr;
        }

        logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMArrayType Creating LLVM array type for data type: %s",
                   DTM->debug->dataTypeToString(dataType));

        DataType *baseDataType = dataType->container->type.arrayType->baseType;
        llvm::Type *baseType = getLLVMType(baseDataType);
        if (!baseType)
        {
            std::cerr << "Failed to get base type for array" << std::endl;
            CONDITION_FAILED;
            return nullptr;
        }

        logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMArrayType Base type: %s", DTM->debug->dataTypeToString(baseDataType));

        int size = dataType->container->type.arrayType->size;
        int elementCount = dataType->container->type.arrayType->elementCount;
        int dimensionCount = dataType->container->type.arrayType->dimensionCount;
        bool isDynamic = dataType->container->type.arrayType->isDynamic;

        if (size <= 0 && elementCount <= 0)
        {
            std::cerr << "Array size is invalid" << std::endl;

            std::cerr << "Base data type: " << DTM->debug->dataTypeToString(baseDataType) << std::endl;
            baseDataType->debug->printVerbosType(baseDataType);
            std::cerr << "Array data type: " << DTM->debug->dataTypeToString(dataType) << std::endl;
            dataType->debug->printVerbosType(dataType);

            // Check if it's `str[]`
            if (baseDataType->container->primitive == PRIM_STR &&
                dataType->container->typeOf == ARRAY_TYPE &&
                dataType->container->type.arrayType->isDynamic)
            {
                std::cout << "Array is a dynamic string array" << std::endl;
                llvm::PointerType *llvmPtrType = llvm::PointerType::get(baseType, 0);
            }

            return nullptr;
        }

        llvm::ArrayType *llvmArrayType = llvm::ArrayType::get(baseType, size);
        if (!llvmArrayType)
        {
            std::cerr << "Failed to create LLVM array type" << std::endl;
            CONDITION_FAILED;
            return nullptr;
        }

        logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMArrayType LLVM array type created successfully");
        logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMArrayType Array type: %s", DTM->debug->dataTypeToString(dataType));
        logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMArrayType Element count: %d", size);

        return llvmArrayType;
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

        std::string structName = "struct." + std::string(dataType->typeName);
        logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMStructType Finding struct type: %s", structName.c_str());

        // First try to find existing type in LLVM context
        llvm::StructType *existingType = llvm::StructType::getTypeByName(
            context.getInstance().context,
            structName);

        if (existingType)
        {
            logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMStructType Found existing LLVM type: %s",
                       existingType->getName().str().c_str());
            return existingType;
        }

        // If not in LLVM, check our symbol table
        IRTypeSymbol *typeSymbol = findType(structName);
        if (typeSymbol == nullptr)
        {
            logMessage(LMI, "ERROR", "IRSymbolTable", "Type symbol is null");
            CONDITION_FAILED;
        }

        if (typeSymbol->getType() == nullptr)
        {
            logMessage(LMI, "ERROR", "IRSymbolTable", "Type symbol has null type");
            CONDITION_FAILED;
        }

        if (typeSymbol && typeSymbol->getType())
        {
            llvm::Type *type = typeSymbol->getType();
            llvm::StructType *structType = llvm::dyn_cast<llvm::StructType>(type);

            if (structType)
            {
                logMessage(LMI, "INFO", "IRSymbolTable", "@getLLVMStructType Found type in symbol table: %s",
                           structType->getName().str().c_str());
                return structType;
            }
            else
            {
                logMessage(LMI, "ERROR", "IRSymbolTable", "@getLLVMStructType Type is not a struct type");
                CONDITION_FAILED;
            }
        }

        // If we get here, the type wasn't found
        logMessage(LMI, "ERROR", "IRSymbolTable", "@getLLVMStructType Type not found: %s", structName.c_str());
        CONDITION_FAILED;
    }

    // Class Types
    llvm::StructType *IRSymbolTable::getLLVMClassType(DataType *dataType)
    {
        DEBUG_BREAKPOINT;
    }

    // Function Types
    llvm::FunctionType *IRSymbolTable::getLLVMFunctionType(DataType *dataType)
    {
        if (!dataType)
        {
            std::cerr << "Data type is null" << std::endl;
            return nullptr;
        }
        if (dataType->container->primitive != PRIM_FUNCTION)
        {
            std::cerr << "Data type is not a function" << std::endl;
            return nullptr;
        }

        logMessage(LMI, "INFO", "IRSymbolTable", "Creating LLVM function type for data type: %s",
                   DTM->debug->dataTypeToString(dataType));

        llvm::Type *returnType = getLLVMType(dataType->container->type.functionType->returnType);
        if (!returnType)
        {
            std::cerr << "Failed to get return type" << std::endl;
            return nullptr;
        }
        std::vector<llvm::Type *> paramTypes = getLLVMTypes(dataType->container->type.functionType->paramTypes);

        llvm::FunctionType *llvmFuncType = llvm::FunctionType::get(
            returnType,
            paramTypes,
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