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

llvm::StructType* CodeGen::createStringStruct() {
    return llvm::StructType::create(context, "String");
}

llvm::StructType* CodeGen::createStringType() {
    llvm::StructType* stringType = llvm::StructType::create(context, "String");
    std::vector<llvm::Type*> elements = {
        builder.getInt8Ty()->getPointerTo(),
        builder.getInt32Ty(),
        builder.getInt32Ty(),
        builder.getInt32Ty()
    };
    stringType->setBody(elements);
    return stringType;
}

llvm::Value* CodeGen::createString(const std::string& str) {
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

llvm::Value* CodeGen::createNumber(int num) {
    std::cout << "[CPP - DEBUG] Creating number constant: " << num << "\n";
    llvm::Constant* numConstant = llvm::ConstantInt::get(context, llvm::APInt(32, num, true));
    std::cout << "[CPP - DEBUG] Created number constant\n";
    return numConstant;
}

llvm::Constant* CodeGen::createConstantInt(int value) {
    return llvm::ConstantInt::get(builder.getInt32Ty(), value, true);
}

llvm::Value* CodeGen::createReferenceInt(int value) {
    llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
        *module, builder.getInt32Ty(), true,
        llvm::GlobalValue::PrivateLinkage, createConstantInt(value), ".int");
    return globalVar;
}

} // namespace Cryo
