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
#include "codegen/codegen.hpp"

namespace Cryo
{
    // Statements
    void CodeGenVisitor::visitIfStatement(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting if statement...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }

        if (node->metaData->type != NODE_IF_STATEMENT)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is not an if statement");
            return;
        }

        IfStatementNode *ifStmt = node->data.ifStatement;
        if (!ifStmt)
        {
            logMessage(LMI, "ERROR", "Visitor", "If statement node is null");
            return;
        }

        // Get the current function
        llvm::Function *function = builder.GetInsertBlock()->getParent();

        // Create the basic blocks for then, else, and merge
        llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(context.getInstance().context, "then", function);
        llvm::BasicBlock *elseBB = nullptr;
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(context.getInstance().context, "ifcont");

        if (ifStmt->elseBranch)
        {
            elseBB = llvm::BasicBlock::Create(context.getInstance().context, "else");
        }

        // Generate the condition code
        llvm::Value *conditionVal = getLLVMValue(ifStmt->condition);
        if (!conditionVal)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to generate condition");
            return;
        }

        // If condition is a pointer, load it
        if (conditionVal->getType()->isPointerTy())
        {
            // Handle pointer loading safely with opaque pointers
            if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(conditionVal))
            {
                llvm::Type *allocatedType = allocaInst->getAllocatedType();
                conditionVal = builder.CreateLoad(allocatedType, conditionVal, "condload");
            }
            else
            {
                // Try to get type from the AST
                DataType *condType = DTM->astInterface->getTypeofASTNode(ifStmt->condition);
                if (condType)
                {
                    llvm::Type *loadType = symbolTable->getLLVMType(condType);
                    if (loadType)
                    {
                        conditionVal = builder.CreateLoad(loadType, conditionVal, "condload");
                    }
                    else
                    {
                        logMessage(LMI, "WARNING", "Visitor", "Using i8 as fallback type for condition");
                        conditionVal = builder.CreateLoad(builder.getInt8Ty(), conditionVal, "condload");
                    }
                }
            }
        }

        // Convert condition to boolean if needed
        if (!conditionVal->getType()->isIntegerTy(1))
        {
            if (conditionVal->getType()->isIntegerTy())
            {
                // Integer comparison with 0
                conditionVal = builder.CreateICmpNE(
                    conditionVal,
                    llvm::ConstantInt::get(conditionVal->getType(), 0),
                    "cond");
            }
            else if (conditionVal->getType()->isFloatingPointTy())
            {
                // Floating point comparison with 0.0
                conditionVal = builder.CreateFCmpONE(
                    conditionVal,
                    llvm::ConstantFP::get(conditionVal->getType(), 0.0),
                    "cond");
            }
            else
            {
                logMessage(LMI, "ERROR", "Visitor", "Unsupported condition type");
                return;
            }
        }

        // Create the conditional branch
        if (elseBB)
        {
            builder.CreateCondBr(conditionVal, thenBB, elseBB);
        }
        else
        {
            builder.CreateCondBr(conditionVal, thenBB, mergeBB);
        }

        // Generate the 'then' branch code
        builder.SetInsertPoint(thenBB);

        // Push a new scope for the 'then' branch
        symbolTable->pushScope();

        // Visit the 'then' branch
        visit(ifStmt->thenBranch);

        // Pop the scope
        symbolTable->popScope();

        // Add a branch to the merge block if needed
        if (!builder.GetInsertBlock()->getTerminator())
        {
            builder.CreateBr(mergeBB);
        }

        // Generate the 'else' branch code if it exists
        if (elseBB)
        {
            // Add the else block to the function - without using getBasicBlockList()
            elseBB->insertInto(function); // Or use this alternative approach
            builder.SetInsertPoint(elseBB);

            // Push a new scope for the 'else' branch
            symbolTable->pushScope();

            // Visit the 'else' branch
            visit(ifStmt->elseBranch);

            // Pop the scope
            symbolTable->popScope();

            // Add a branch to the merge block if needed
            if (!builder.GetInsertBlock()->getTerminator())
            {
                builder.CreateBr(mergeBB);
            }
        }

        // Add the merge block to the function - without using getBasicBlockList()
        mergeBB->insertInto(function); // Or use this alternative approach
        builder.SetInsertPoint(mergeBB);

        logMessage(LMI, "INFO", "Visitor", "If statement processed successfully");
    }

    void CodeGenVisitor::visitWhileStatement(ASTNode *node)
    {
        if (!node || !node->data.whileStatement)
            return;
        logMessage(LMI, "INFO", "Visitor", "Visiting while statement node");

        // Get the current function
        llvm::Function *function = context.getInstance().builder.GetInsertBlock()->getParent();

        // Create blocks with proper naming and order
        llvm::BasicBlock *condBB = llvm::BasicBlock::Create(context.getInstance().context, "while.cond", function);
        llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(context.getInstance().context, "while.body", function);
        llvm::BasicBlock *exitBB = llvm::BasicBlock::Create(context.getInstance().context, "while.exit", function);
        llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(context.getInstance().context, "while.after", function);

        // Set break/continue targets
        context.getInstance().symbolTable->setBreakBlock(exitBB);
        context.getInstance().symbolTable->setContinueBlock(condBB);

        // Branch to condition
        context.getInstance().builder.CreateBr(condBB);

        context.getInstance().builder.SetInsertPoint(condBB);
        llvm::Value *conditionVal = getLLVMValue(node->data.whileStatement->condition);
        if (!conditionVal)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to generate condition");
            return;
        }

        // If condition is a pointer, load it
        if (conditionVal->getType()->isPointerTy())
        {
            // Handle pointer loading safely with opaque pointers
            if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(conditionVal))
            {
                llvm::Type *allocatedType = allocaInst->getAllocatedType();
                conditionVal = context.getInstance().builder.CreateLoad(allocatedType, conditionVal, "condload");
            }
            else
            {
                // Try to get type from the AST
                DataType *condType = DTM->astInterface->getTypeofASTNode(node->data.whileStatement->condition);
                if (condType)
                {
                    llvm::Type *loadType = symbolTable->getLLVMType(condType);
                    if (loadType)
                    {
                        conditionVal = context.getInstance().builder.CreateLoad(loadType, conditionVal, "condload");
                    }
                    else
                    {
                        logMessage(LMI, "WARNING", "Visitor", "Using i8 as fallback type for condition");
                        conditionVal = context.getInstance().builder.CreateLoad(context.getInstance().builder.getInt8Ty(), conditionVal, "condload");
                    }
                }
            }
        }
        // Convert condition to boolean if needed
        if (!conditionVal->getType()->isIntegerTy(1))
        {
            if (conditionVal->getType()->isIntegerTy())
            {
                // Integer comparison with 0
                conditionVal = context.getInstance().builder.CreateICmpNE(
                    conditionVal,
                    llvm::ConstantInt::get(conditionVal->getType(), 0),
                    "cond");
            }
            else if (conditionVal->getType()->isFloatingPointTy())
            {
                // Floating point comparison with 0.0
                conditionVal = context.getInstance().builder.CreateFCmpONE(
                    conditionVal,
                    llvm::ConstantFP::get(conditionVal->getType(), 0.0),
                    "cond");
            }
            else
            {
                logMessage(LMI, "ERROR", "Visitor", "Unsupported condition type");
                return;
            }
        }

        // Branch based on condition to body or exit
        context.getInstance().builder.CreateCondBr(conditionVal, bodyBB, exitBB);

        // Generate loop body
        context.getInstance().builder.SetInsertPoint(bodyBB);
        symbolTable->pushScope();
        visit(node->data.whileStatement->body);
        symbolTable->popScope();

        // Add branch back to condition if no terminator
        if (!context.getInstance().builder.GetInsertBlock()->getTerminator())
        {
            context.getInstance().builder.CreateBr(condBB);
        }

        // Set up exit block that branches to after block
        context.getInstance().builder.SetInsertPoint(exitBB);
        context.getInstance().builder.CreateBr(afterBB);

        // Set insert point for code after the loop
        context.getInstance().builder.SetInsertPoint(afterBB);

        // Clear break/continue targets
        context.getInstance().symbolTable->setBreakBlock(nullptr);
        context.getInstance().symbolTable->setContinueBlock(nullptr);

        logMessage(LMI, "INFO", "Visitor", "While statement processed successfully");
        return;
    }

    void CodeGenVisitor::visitForStatement(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }
} // namespace Cryo
