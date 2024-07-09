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

void CodeGen::generateIfStatement(ASTNode* node) {
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(builder.getContext(), "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(builder.getContext(), "else", function);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(builder.getContext(), "ifcont", function);

    llvm::Value* condValue = generateExpression(node->data.ifStmt.condition);
    if (!condValue) {
        std::cerr << "[CPP] Error generating if statement condition\n";
        return;
    }
    std::cout << "[CPP] Condition value generated: " << condValue << "\n";
    builder.CreateCondBr(condValue, thenBB, elseBB);

    // Then block
    builder.SetInsertPoint(thenBB);
    std::cout << "[CPP] Generating 'then' block\n";
    generateBlock(node->data.ifStmt.thenBranch);
    std::cout << "[CPP] 'Then' block generated\n";
    builder.CreateBr(mergeBB);
    std::cout << "[CPP] 'Then' block branch created\n";
    thenBB = builder.GetInsertBlock(); // Update thenBB after generating the block
    std::cout << "[CPP] 'Then' block generated\n";

    // Else block
    builder.SetInsertPoint(elseBB);
    if (node->data.ifStmt.elseBranch) {
        std::cout << "[CPP] Generating 'else' block\n";
        generateBlock(node->data.ifStmt.elseBranch);
    }
    builder.CreateBr(mergeBB);
    elseBB = builder.GetInsertBlock(); // Update elseBB after generating the block
    std::cout << "[CPP] 'Else' block generated\n";

    // Merge block
    function->splice(function->end(), function, mergeBB->getIterator());
    builder.SetInsertPoint(mergeBB);
    std::cout << "[CPP] 'If' statement generated with 'then' and 'else' branches\n";
}

void CodeGen::generateForLoop(ASTNode* node) {
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    std::cout << "[CPP] Generating for loop\n";

    // Create blocks for loop entry, body, increment, and after the loop
    llvm::BasicBlock* loopEntryBB = llvm::BasicBlock::Create(builder.getContext(), "loop_entry", function);
    llvm::BasicBlock* loopBodyBB = llvm::BasicBlock::Create(builder.getContext(), "loop_body");
    llvm::BasicBlock* loopIncrementBB = llvm::BasicBlock::Create(builder.getContext(), "loop_increment");
    llvm::BasicBlock* afterLoopBB = llvm::BasicBlock::Create(builder.getContext(), "after_loop");

    std::cout << "[CPP] Blocks created\n";

    // Generate initialization code
    llvm::Value* initValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(module->getContext()), 0); // Initialize iterator to 0
    std::cout << "[DEBUG] Created iterator initial value: " << initValue << std::endl;

    // Simplified allocation
    llvm::AllocaInst* iterator = builder.CreateAlloca(llvm::Type::getInt32Ty(module->getContext()), nullptr, "iter");
    std::cout << "[CPP] Iterator allocated: " << iterator << std::endl;

    if (!iterator) {
        std::cerr << "[ERROR] Failed to allocate iterator\n";
        return;
    }

    builder.CreateStore(initValue, iterator);
    std::cout << "[CPP] Initialization code generated\n";

    namedValues["i"] = iterator;

    // Insert the loop entry block
    builder.CreateBr(loopEntryBB);
    builder.SetInsertPoint(loopEntryBB);

    std::cout << "[CPP] Loop entry block inserted\n";

    // Generate condition code
    llvm::Value* condValue = generateExpression(node->data.forStmt.condition);
    if (!condValue) {
        std::cerr << "[ERROR] Failed to generate condition value\n";
        return;
    }
    builder.CreateCondBr(condValue, loopBodyBB, afterLoopBB);
    std::cout << "[CPP] Condition code generated\n";

    // Generate loop body code
    std::cout << "[CPP - DEBUG] Generating loop body code\n";
    builder.SetInsertPoint(loopBodyBB);
    std::cout << "[CPP] Loop body block inserted\n";
    generateBlock(node->data.forStmt.body);
    std:: cout << "[CPP] Loop body generated\n";
    builder.CreateBr(loopIncrementBB);
    std::cout << "[CPP] Loop body code generated\n";

    // Generate increment code
    function->splice(function->end(), function, loopIncrementBB->getIterator());
    builder.SetInsertPoint(loopIncrementBB);
    llvm::Value* incrementValue = generateExpression(node->data.forStmt.increment);
    if (!incrementValue) {
        std::cerr << "[ERROR] Failed to generate increment value\n";
        return;
    }
    llvm::Value* currentValue = builder.CreateLoad(iterator->getType(), iterator);
    llvm::Value* nextValue = builder.CreateAdd(currentValue, incrementValue, "next_iter_val");
    builder.CreateStore(nextValue, iterator);
    builder.CreateBr(loopEntryBB);
    std::cout << "[CPP] Increment code generated\n";

    // Insert after loop block
    function->splice(function->end(), function, afterLoopBB->getIterator());
    builder.SetInsertPoint(afterLoopBB);
    std::cout << "[CPP] After loop block inserted\n";
}

} // namespace Cryo
