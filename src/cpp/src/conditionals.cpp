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



// <generateIfStatement>
void generateIfStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(builder.getContext(), "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(builder.getContext(), "else", function);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(builder.getContext(), "ifcont", function);

    llvm::Value* condValue = generateExpression(node->data.ifStmt.condition, builder, module);
    if (!condValue) {
        std::cerr << "[CPP] Error generating if statement condition\n";
        return;
    }
    std::cout << "[CPP] Condition value generated: " << condValue << "\n";
    builder.CreateCondBr(condValue, thenBB, elseBB);

    // Then block
    builder.SetInsertPoint(thenBB);
    std::cout << "[CPP] Generating 'then' block\n";
    generateBlock(node->data.ifStmt.thenBranch, builder, module); // Doesn't make it past here.
    std::cout << "[CPP] 'Then' block generated\n";
    builder.CreateBr(mergeBB);
    std::cout << "[CPP] 'Then' block branch created\n";
    thenBB = builder.GetInsertBlock(); // Update thenBB after generating the block
    std::cout << "[CPP] 'Then' block generated\n";

    // Else block
    builder.SetInsertPoint(elseBB);
    if (node->data.ifStmt.elseBranch) {
        std::cout << "[CPP] Generating 'else' block\n";
        generateBlock(node->data.ifStmt.elseBranch, builder, module);
    }
    builder.CreateBr(mergeBB);
    elseBB = builder.GetInsertBlock(); // Update elseBB after generating the block
    std::cout << "[CPP] 'Else' block generated\n";

    // Merge block
    function->splice(function->end(), function, mergeBB->getIterator());
    builder.SetInsertPoint(mergeBB);
    std::cout << "[CPP] 'If' statement generated with 'then' and 'else' branches\n";
}
// </generateIfStatement>



