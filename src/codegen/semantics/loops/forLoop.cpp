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
#include "codegen/oldCodeGen.hpp"

namespace Cryo
{
    void Loops::handleForLoop(ASTNode *node)
    {
        Variables &variables = compiler.getVariables();
        OldTypes &types = compiler.getTypes();
        BinaryExpressions &binExp = compiler.getBinaryExpressions();
        llvm::LLVMContext &context = compiler.getContext().context;
        llvm::IRBuilder<> &builder = compiler.getContext().builder;

        DevDebugger::logMessage("INFO", __LINE__, "Loops", "Handling For Loop");

        if (!node || !node->data.forStatement)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Loops", "Invalid For Loop node!");
            return;
        }

        llvm::Function *currentFunction = builder.GetInsertBlock()->getParent();
        if (!currentFunction)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Loops", "Current function is null!");
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
            // Reveiving a NODE_VAR_DECLARATION here
            DevDebugger::logMessage("INFO", __LINE__, "Loops", "Creating Initializer");
            // compiler.getGenerator().parseTree(node->data.forStatement->initializer);
        }

        // Get the created initializer value
        llvm::Value *initializerValue = nullptr;
        if (node->data.forStatement->initializer)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Loops", "Getting Initializer Value");
            // Create a load instruction for the initializer
            // initializerValue = compiler.getGenerator().getInitilizerValue(node->data.forStatement->initializer);
            // if (!initializerValue)
            // {
            //     DevDebugger::logMessage("ERROR", __LINE__, "Loops", "Failed to get initializer value");
            //     return;
            // }
            // llvm::Value *initVal = compiler.getGenerator().getInitilizerValue(node->data.forStatement->initializer);
            initializerValue = variables.createLocalVariable(node->data.forStatement->initializer);
            if (!initializerValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Loops", "Failed to create initializer value");
                return;
            }
            // Make sure the initializer has its value attached to it
            initializerValue->setName(node->data.forStatement->initializer->data.varDecl->name);
            // TODO: Somehow get the literal ints value and replace the `0` in the function below.
            llvm::ConstantInt *valueOfInit = types.getLiteralIntValue(0);
            // Store the initializer value
            builder.CreateStore(valueOfInit, initializerValue);
        }

        // Branch to the loop block
        builder.CreateBr(loopBB);
        builder.SetInsertPoint(loopBB);

        // Create the comparison
        DevDebugger::logMessage("INFO", __LINE__, "Loops", "Creating Comparison Expression");
        // Create the load instruction
        builder.CreateLoad(llvm::Type::getInt32Ty(context), initializerValue);

        // Emit the condition
        llvm::Value *conditionValue = nullptr;
        if (node->data.forStatement->condition)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Loops", "Creating Condition Value");
            conditionValue = binExp.createComparisonExpression(
                node->data.forStatement->condition->data.bin_op->left,
                node->data.forStatement->condition->data.bin_op->right,
                node->data.forStatement->condition->data.bin_op->op,
                loopBB);
        }
        else
        {
            DevDebugger::logMessage("INFO", __LINE__, "Loops", "Creating True Condition Value");
            conditionValue = llvm::ConstantInt::getTrue(context);
        }
        DevDebugger::logMessage("INFO", __LINE__, "Loops", "Condition Value Created");
        builder.CreateCondBr(conditionValue, bodyBB, exitBB);

        // Emit the loop body
        builder.SetInsertPoint(bodyBB);
        if (node->data.forStatement->body)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Loops", "Creating Loop Body");
            compiler.getGenerator().parseTree(node->data.forStatement->body);
        }
        builder.CreateBr(incrementBB);

        // Emit the increment
        builder.SetInsertPoint(incrementBB);
        if (node->data.forStatement->increment)
        {
            // Find the variable in the symbol table
            ASTNode *incrementNode = node->data.forStatement->increment;
            if (incrementNode->metaData->type == NODE_UNARY_EXPR)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Loops", "Processing Increment");
                CryoTokenType op = incrementNode->data.unary_op->op;
                std::string varName = incrementNode->data.unary_op->operand->data.varName->varName;
                // Find the variable in the symbol table
                std::cout << "Variable Name: " << varName << std::endl;
                ASTNode *varNode = compiler.getSymTable().getASTNode(compiler.getContext().currentNamespace, NODE_VAR_DECLARATION, varName);
                if (!varNode)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Loops", "Variable not found");
                    CONDITION_FAILED;
                }
                if (incrementNode->metaData->type == NODE_UNARY_EXPR)
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Loops", "Processing Increment");
                    CryoTokenType op = incrementNode->data.unary_op->op;
                    std::string varName = incrementNode->data.unary_op->operand->data.varName->varName;
                    // Find the variable in the symbol table
                    std::cout << "Variable Name: " << varName << std::endl;
                    llvm::Value *indexVar = variables.getLocalScopedVariable(varName);
                    llvm::Value *currentValue = builder.CreateLoad(llvm::Type::getInt32Ty(context), indexVar);
                    llvm::Value *incrementedValue;

                    if (op == TOKEN_INCREMENT)
                    {
                        incrementedValue = builder.CreateAdd(currentValue, llvm::ConstantInt::get(context, llvm::APInt(32, 1)));
                    }
                    else if (op == TOKEN_DECREMENT)
                    {
                        incrementedValue = builder.CreateSub(currentValue, llvm::ConstantInt::get(context, llvm::APInt(32, 1)));
                    }
                    else
                    {
                        DevDebugger::logMessage("ERROR", __LINE__, "Loops", "Unknown increment operator");
                        return;
                    }

                    builder.CreateStore(incrementedValue, indexVar);
                }
            }
            // DEBUG_BREAKPOINT;
        }
        builder.CreateBr(loopBB);

        // Continue with code after the loop
        builder.SetInsertPoint(exitBB);

        DevDebugger::logMessage("INFO", __LINE__, "Loops", "For Loop Handled");
    }

} // namespace Cryo