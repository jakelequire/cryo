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


// <createStringType>
llvm::StructType *createStringType(llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    llvm::StructType *stringType = createStringStruct(context);
    std::vector<llvm::Type *> elements = {
        builder.getInt8Ty()->getPointerTo(),    // buffer
        builder.getInt32Ty(),                   // length
        builder.getInt32Ty(),                   // maxlen
        builder.getInt32Ty()                    // factor
    };
    stringType->setBody(elements);
    return stringType;
}
// </createStringType>

// <createString>
llvm::Value *createString(llvm::IRBuilder<> &builder, llvm::Module &module, const std::string &str) {
    std::cout << "[CPP_DEBUG - createString] Creating string\n - Value: " << str << "\n";
    // Set up the string type & struct
    llvm::LLVMContext &context = module.getContext();
    std::cout << "[CPP_DEBUG - createString] Setting uqp string type & struct\n";
    
    llvm::StructType *stringType = createStringType(context, builder);
    std::cout << "[CPP_DEBUG - createString] String type set up\n";

    // Use the ConstantDataArray to create a buffer
    llvm::Constant *buffer = llvm::ConstantDataArray::getString(context, str);
    std::cout << "[CPP_DEBUG - createString] Buffer created\n";

    // Create a global variable for the buffer
    llvm::GlobalVariable *globalBuffer = new llvm::GlobalVariable(module, buffer->getType(), true, llvm::GlobalValue::PrivateLinkage, buffer);
    std::cout << "[CPP_DEBUG - createString] Global buffer created\n";

    // Create a constant struct with the buffer, length, maxlen, and factor
    llvm::Constant *length = llvm::ConstantInt::get(builder.getInt32Ty(), str.size());
    llvm::Constant *maxlen = llvm::ConstantInt::get(builder.getInt32Ty(), str.size());
    llvm::Constant *factor = llvm::ConstantInt::get(builder.getInt32Ty(), 16);
    std::cout << "[CPP_DEBUG - createString] Constants created\n";

    // Create the struct
    std::vector<llvm::Constant *> elements = {
        llvm::ConstantExpr::getPointerCast(globalBuffer, builder.getInt8Ty()->getPointerTo()),
        length,
        maxlen,
        factor
    };
    std::cout << "[CPP_DEBUG - createString] Elements created\n";

    // Return the constant struct
    return llvm::ConstantStruct::get(stringType, elements);
}
// </createString>
