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
#include "cpp/codegen.h"

namespace Cryo
{

    llvm::StructType *CryoTypes::createStringStruct()
    {
        CryoContext &cryoContext = compiler.getContext();

        return llvm::StructType::create(cryoContext.context, "String");
    }

    llvm::StructType *CryoTypes::createStringType()
    {
        CryoContext &cryoContext = compiler.getContext();

        llvm::StructType *stringType = llvm::StructType::create(cryoContext.context, "String");
        std::vector<llvm::Type *> elements = {
            cryoContext.builder.getInt8Ty()->getPointerTo(), // char* data
            cryoContext.builder.getInt32Ty(),                // int length
            cryoContext.builder.getInt32Ty()                 // int capacity
        };
        stringType->setBody(elements);
        return stringType;
    }

    llvm::StructType *CryoTypes::createArrayType(llvm::Type *elementType)
    {
        CryoContext &cryoContext = compiler.getContext();

        std::vector<llvm::Type *> elements = {
            elementType->getPointerTo(),      // T* data
            cryoContext.builder.getInt32Ty(), // int size
            cryoContext.builder.getInt32Ty()  // int capacity
        };
        return llvm::StructType::create(cryoContext.context, elements, "Array");
    }

    llvm::StructType *CryoTypes::createPairType(llvm::Type *firstType, llvm::Type *secondType)
    {
        CryoContext &cryoContext = compiler.getContext();
        std::vector<llvm::Type *> elements = {firstType, secondType};
        return llvm::StructType::create(cryoContext.context, elements, "Pair");
    }

    llvm::StructType *CryoTypes::createTupleType(const std::vector<llvm::Type *> &types)
    {
        CryoContext &cryoContext = compiler.getContext();
        return llvm::StructType::create(cryoContext.context, types, "Tuple");
    }

    llvm::StructType *CryoTypes::createOptionalType(llvm::Type *valueType)
    {
        CryoContext &cryoContext = compiler.getContext();
        std::vector<llvm::Type *> elements = {
            cryoContext.builder.getInt1Ty(), // bool hasValue
            valueType                        // T value
        };
        return llvm::StructType::create(cryoContext.context, elements, "Optional");
    }

    llvm::StructType *CryoTypes::createVariantType(const std::vector<llvm::Type *> &types)
    {
        CryoContext &cryoContext = compiler.getContext();
        llvm::Type *largestType = *std::max_element(types.begin(), types.end(),
                                                    [](llvm::Type *a, llvm::Type *b)
                                                    {
                                                        return a->getPrimitiveSizeInBits() || b->getPrimitiveSizeInBits();
                                                    });

        std::vector<llvm::Type *> elements = {
            cryoContext.builder.getInt32Ty(), // int typeIndex
            llvm::ArrayType::get(cryoContext.builder.getInt8Ty(),
                                 largestType->getPrimitiveSizeInBits()) // byte[] data
        };
        return llvm::StructType::create(cryoContext.context, elements, "Variant");
    }

    llvm::StructType *CryoTypes::createFunctionType(llvm::Type *returnType, const std::vector<llvm::Type *> &paramTypes)
    {
        CryoContext &cryoContext = compiler.getContext();
        llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes, false);
        std::vector<llvm::Type *> elements = {
            funcType->getPointerTo(),                       // function pointer
            cryoContext.builder.getInt8Ty()->getPointerTo() // void* context (for closures)
        };
        return llvm::StructType::create(cryoContext.context, elements, "Function");
    }

    llvm::Constant *CryoTypes::createString(const std::string &str)
    {
        CryoContext &cryoContext = compiler.getContext();

        llvm::GlobalValue *global = new llvm::GlobalVariable(
            *cryoContext.module,
            llvm::ArrayType::get(cryoContext.builder.getInt8Ty(), str.size() + 1),
            true,
            llvm::GlobalValue::PrivateLinkage,
            llvm::ConstantDataArray::getString(cryoContext.context, str),
            ".str");

        return global;
    }

    llvm::Value *CryoTypes::createNumber(int num)
    {
        CryoContext &cryoContext = compiler.getContext();
        std::cout << "[CPP - DEBUG] Creating number constant: " << num << "\n";
        llvm::Constant *numConstant = llvm::ConstantInt::get(cryoContext.context, llvm::APInt(32, num, true));
        std::cout << "[CPP - DEBUG] Created number constant\n";
        return numConstant;
    }

    llvm::Constant *CryoTypes::createConstantInt(int value)
    {
        CryoContext &cryoContext = compiler.getContext();
        return llvm::ConstantInt::get(cryoContext.builder.getInt32Ty(), value, true);
    }

    llvm::Value *CryoTypes::createReferenceInt(int value)
    {
        CryoContext &cryoContext = compiler.getContext();
        llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(
            *cryoContext.module, cryoContext.builder.getInt32Ty(), true,
            llvm::GlobalValue::PrivateLinkage, createConstantInt(value), ".int");
        return globalVar;
    }

} // namespace Cryo
