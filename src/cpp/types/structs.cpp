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

namespace Cryo {

llvm::StructType* CryoTypes::createStringStruct() {
    return llvm::StructType::create(context, "String");
}

llvm::StructType* CryoTypes::createStringType() {
    llvm::StructType* stringType = llvm::StructType::create(context, "String");
    std::vector<llvm::Type*> elements = {
        builder.getInt8Ty()->getPointerTo(), // char* data
        builder.getInt32Ty(),                // int length
        builder.getInt32Ty()                 // int capacity
    };
    stringType->setBody(elements);
    return stringType;
}

llvm::StructType* CryoTypes::createArrayType(llvm::Type* elementType) {
    std::vector<llvm::Type*> elements = {
        elementType->getPointerTo(),  // T* data
        builder.getInt32Ty(),         // int size
        builder.getInt32Ty()          // int capacity
    };
    return llvm::StructType::create(context, elements, "Array");
}

llvm::StructType* CryoTypes::createPairType(llvm::Type* firstType, llvm::Type* secondType) {
    std::vector<llvm::Type*> elements = {firstType, secondType};
    return llvm::StructType::create(context, elements, "Pair");
}

llvm::StructType* CryoTypes::createTupleType(const std::vector<llvm::Type*>& types) {
    return llvm::StructType::create(context, types, "Tuple");
}

llvm::StructType* CryoTypes::createOptionalType(llvm::Type* valueType) {
    std::vector<llvm::Type*> elements = {
        builder.getInt1Ty(),  // bool hasValue
        valueType             // T value
    };
    return llvm::StructType::create(context, elements, "Optional");
}

llvm::StructType* CryoTypes::createVariantType(const std::vector<llvm::Type*>& types) {
    llvm::Type* largestType = *std::max_element(types.begin(), types.end(),
        [](llvm::Type* a, llvm::Type* b) {
            return a->getPrimitiveSizeInBits() < b->getPrimitiveSizeInBits();
        });

    std::vector<llvm::Type*> elements = {
        builder.getInt32Ty(),                   // int typeIndex
        llvm::ArrayType::get(builder.getInt8Ty(), 
            largestType->getPrimitiveSizeInBits() / 8)  // byte[] data
    };
    return llvm::StructType::create(context, elements, "Variant");
}

llvm::StructType* CryoTypes::createFunctionType(llvm::Type* returnType, const std::vector<llvm::Type*>& paramTypes) {
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    std::vector<llvm::Type*> elements = {
        funcType->getPointerTo(),  // function pointer
        builder.getInt8PtrTy()     // void* context (for closures)
    };
    return llvm::StructType::create(context, elements, "Function");
}

llvm::Value* CryoTypes::createString(const std::string& str) {
    llvm::Constant* strConstant = llvm::ConstantDataArray::getString(module->getContext(), str);
    llvm::GlobalVariable* strVar = new llvm::GlobalVariable(
        *module,
        strConstant->getType(),
        true,
        llvm::GlobalValue::PrivateLinkage,
        strConstant,
        ".str"
    );
    strVar->setAlignment(llvm::MaybeAlign(1));
    return builder.CreatePointerCast(strVar, llvm::Type::getInt8Ty(module->getContext())->getPointerTo());
}

llvm::Value* CryoTypes::createNumber(int num) {
    std::cout << "[CPP - DEBUG] Creating number constant: " << num << "\n";
    llvm::Constant* numConstant = llvm::ConstantInt::get(context, llvm::APInt(32, num, true));
    std::cout << "[CPP - DEBUG] Created number constant\n";
    return numConstant;
}

llvm::Constant* CryoTypes::createConstantInt(int value) {
    return llvm::ConstantInt::get(builder.getInt32Ty(), value, true);
}

llvm::Value* CryoTypes::createReferenceInt(int value) {
    llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
        *module, builder.getInt32Ty(), true,
        llvm::GlobalValue::PrivateLinkage, createConstantInt(value), ".int");
    return globalVar;
}

} // namespace Cryo
