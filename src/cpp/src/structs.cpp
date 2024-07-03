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



// >===------------------------------------------===< 
// >===        String Struct Functions           ===<
// >===------------------------------------------===<

// <createStringStruct>
llvm::StructType *createStringStruct(llvm::LLVMContext &context) {
    return llvm::StructType::create(context, "String");
}
// </createStringStruct>

/*
ptr @1, i32 19, i32 19, i32 16 })
*/
// <createStringType>
llvm::StructType* createStringType(llvm::LLVMContext& context, llvm::IRBuilder<>& builder) {
    llvm::StructType* stringType = llvm::StructType::create(context, "String");
    std::vector<llvm::Type*> elements = {
        builder.getInt8Ty()->getPointerTo(),    // ptr
        builder.getInt32Ty(),                   // length
        builder.getInt32Ty(),                   // maxlen
        builder.getInt32Ty()                    // factor
    };
    stringType->setBody(elements);
    return stringType;
}

// </createStringType>

// <createString>
llvm::Value* createString(llvm::IRBuilder<>& builder, llvm::Module& module, const std::string& str) {
    llvm::LLVMContext& context = module.getContext();
    llvm::Constant* strConstant = llvm::ConstantDataArray::getString(context, str, true);
    llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(module, strConstant->getType(), true,
                                                               llvm::GlobalValue::PrivateLinkage, strConstant, "stringLiteral");

    // Return the global string pointer
    return builder.CreatePointerCast(globalStr, builder.getInt8Ty()->getPointerTo());
}

// </createString>
