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
#include "cpp/codegen.hpp"

namespace Cryo
{

    void WhileStatements::handleWhileLoop(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "WhileStatements", "Handling While Loop");

        // Get the condition
        ASTNode *condition = node->data.whileStatement->condition;
        if (!condition)
        {
            debugger.logMessage("ERROR", __LINE__, "WhileStatements", "Condition not found");
            CONDITION_FAILED;
        }

        // Get the block
        ASTNode *block = node->data.whileStatement->body;
        if (!block)
        {
            debugger.logMessage("ERROR", __LINE__, "WhileStatements", "Block not found");
            CONDITION_FAILED;
        }

        // Create the loop
        llvm::Function *function = compiler.getContext().builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *conditionBlock = llvm::BasicBlock::Create(compiler.getContext().context, "condition", function);
        llvm::BasicBlock *loopBlock = llvm::BasicBlock::Create(compiler.getContext().context, "loop", function);
        llvm::BasicBlock *afterBlock = llvm::BasicBlock::Create(compiler.getContext().context, "afterloop", function);

        // Create the condition block
        compiler.getContext().builder.CreateBr(conditionBlock);
        compiler.getContext().builder.SetInsertPoint(conditionBlock);

        // Evaluate the condition
        llvm::Value *conditionValue = compiler.getGenerator().getInitilizerValue(condition);
        if (!conditionValue)
        {
            debugger.logMessage("ERROR", __LINE__, "WhileStatements", "Condition value not found");
            CONDITION_FAILED;
        }

        // Create the conditional branch
        compiler.getContext().builder.CreateCondBr(conditionValue, loopBlock, afterBlock);

        // Create the loop block
        compiler.getContext().builder.SetInsertPoint(loopBlock);

        // Process the block
        compiler.getGenerator().parseTree(block);

        // Create the branch back to the condition
        compiler.getContext().builder.CreateBr(conditionBlock);

        // Create the after block
        compiler.getContext().builder.SetInsertPoint(afterBlock);

        debugger.logMessage("INFO", __LINE__, "WhileStatements", "While Loop Handled");

        return;
    }

} // namespace Cryo