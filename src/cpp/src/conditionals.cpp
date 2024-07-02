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
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(builder.getContext(), "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(builder.getContext(), "ifcont");

    llvm::Value* condValue = generateExpression(node->data.ifStmt.condition, builder, module);
    builder.CreateCondBr(condValue, thenBB, elseBB);

    // Then block
    builder.SetInsertPoint(thenBB);
    generateStatement(node->data.ifStmt.thenBranch, builder, module);
    builder.CreateBr(mergeBB);

    // Else block
    function->splice(function->end(), function, elseBB->getIterator());
    builder.SetInsertPoint(elseBB);
    if (node->data.ifStmt.elseBranch) {
        generateStatement(node->data.ifStmt.elseBranch, builder, module);
    }
    builder.CreateBr(mergeBB);

    // Merge block
    function->splice(function->end(), function, mergeBB->getIterator());
    builder.SetInsertPoint(mergeBB);
}
// </generateIfStatement>







