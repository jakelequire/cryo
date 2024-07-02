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
#include "cpp/lib.h"


// <generatePrintStatement>
// void generatePrintStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
//     ASTNode* expr = node->data.printStatement.expr;
//     llvm::Value* value = generateExpression(expr, builder, module);
//     llvm::Function* printFunc = getCryoPrintFunction(module, value->getType());
// 
//     if (!printFunc) {
//         std::cerr << "[CPP] Error: Failed to get print function\n";
//         return;
//     }
// 
//     builder.CreateCall(printFunc, value);
// }
// </generatePrintStatement>

// <getPrintfFunction>
llvm::Function* getPrintfFunction(llvm::Module& module) {
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::IntegerType::getInt32Ty(module.getContext()), // Return type
        llvm::PointerType::get(llvm::Type::getInt8Ty(module.getContext()), 0), // First argument type
        true // Variable number of arguments
    );

    llvm::Function* func = llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        module
    );

    if (!func) {
        std::cerr << "[CPP] Error: Failed to create printf function\n";
    }

    return func;
}
// </getPrintfFunction>




