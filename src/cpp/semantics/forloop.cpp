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

    void Loops::handleForLoop(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BinaryExpressions &binExp = compiler.getBinaryExpressions();
        llvm::LLVMContext &context = compiler.getContext().context;
        llvm::IRBuilder<> &builder = compiler.getContext().builder;

        debugger.logMessage("INFO", __LINE__, "Loops", "Handling For Loop");

        if (!node || !node->data.forStatement)
        {
            debugger.logMessage("ERROR", __LINE__, "Loops", "Invalid For Loop node!");
            return;
        }

        llvm::Function *currentFunction = builder.GetInsertBlock()->getParent();
        if (!currentFunction)
        {
            debugger.logMessage("ERROR", __LINE__, "Loops", "Current function is null!");
            return;
        }

        // Create basic blocks
        llvm::BasicBlock *preheaderBB = builder.GetInsertBlock();
        llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(context, "loop", currentFunction);
        llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(context, "body", currentFunction);
        llvm::BasicBlock *incrementBB = llvm::BasicBlock::Create(context, "increment", currentFunction);
        llvm::BasicBlock *exitBB = llvm::BasicBlock::Create(context, "exit", currentFunction);

        // Emit the initializer
        if (node->data.forStatement->initializer)
        {
            compiler.getGenerator().parseTree(node->data.forStatement->initializer);
        }

        // Branch to the loop block
        builder.CreateBr(loopBB);
        builder.SetInsertPoint(loopBB);

        // Emit the condition
        llvm::Value *conditionValue = nullptr;
        if (node->data.forStatement->condition)
        {
            ASTNode *conditionNode = node->data.forStatement->condition;
            CryoNodeType conditionType = conditionNode->metaData->type;
            std::cout << "-------------------" << std::endl;
            debugger.logNode(conditionNode);
            std::cout << "-------------------" << std::endl;
            if (conditionType == NODE_BINARY_EXPR)
            {
                ASTNode *left = conditionNode->data.bin_op->left;
                ASTNode *right = conditionNode->data.bin_op->right;
                CryoOperatorType op = conditionNode->data.bin_op->op;

                llvm::Value *compareExpr = binExp.createComparisonExpression(left, right, op);
                if (!compareExpr)
                {
                    debugger.logMessage("ERROR", __LINE__, "Loops", "Failed to create comparison expression");
                    exit(1);
                    return;
                }
                conditionValue = compareExpr;
            }
            debugger.logMessage("INFO", __LINE__, "Loops", "Loop Condition Created");

            // Create the condition
        }
        else
        {
            debugger.logMessage("INFO", __LINE__, "Loops", "Creating True Condition");
        }
        debugger.logMessage("INFO", __LINE__, "Loops", "Condition Value Created");
        builder.CreateCondBr(conditionValue, bodyBB, exitBB);

        // Emit the loop body
        builder.SetInsertPoint(bodyBB);
        if (node->data.forStatement->body)
        {
            compiler.getGenerator().parseTree(node->data.forStatement->body);
        }
        builder.CreateBr(incrementBB);

        // Emit the increment
        builder.SetInsertPoint(incrementBB);
        if (node->data.forStatement->increment)
        {
            compiler.getGenerator().parseTree(node->data.forStatement->increment);
        }
        builder.CreateBr(loopBB);

        // Continue with code after the loop
        builder.SetInsertPoint(exitBB);

        debugger.logMessage("INFO", __LINE__, "Loops", "For Loop Handled");
    }

} // namespace Cryo
