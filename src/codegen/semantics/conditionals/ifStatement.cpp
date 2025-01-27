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
    /// @public
    void IfStatements::handleIfStatement(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "Handling If Statement");
        assert(node != nullptr);

        // Current function were in
        llvm::Function *currentFunction = compiler.getContext().builder.GetInsertBlock()->getParent();

        // This includes the lables `ifBlock`, `thenBlock`, `elseBlock` and `mergeBlock`
        llvm::BasicBlock *ifConditionBlock = createIfStatement(node);

        DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "If Statement Handled");
    }

    llvm::BasicBlock *IfStatements::createIfStatement(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "Creating If Statement");

        IfStatementNode *ifNode = node->data.ifStatement;

        llvm::Function *currentFunction = compiler.getContext().builder.GetInsertBlock()->getParent();
        // Get the functions return type
        llvm::Type *returnType = currentFunction->getReturnType();

        // Get the condition
        ASTNode *condition = ifNode->condition;
        assert(condition != nullptr);

        // Get the then branch
        ASTNode *thenBranch = ifNode->thenBranch;
        assert(thenBranch != nullptr);

        // Get the else branch
        ASTNode *elseBranch = ifNode->elseBranch;

        // Create the condition
        auto [conditionBlock, conditionValue] = createIfCondition(condition);
        DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "Condition Block Created");

        // Create the then block
        llvm::BasicBlock *thenBlock = llvm::BasicBlock::Create(compiler.getContext().context, "thenBlock", currentFunction);

        // Create the else block
        llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(compiler.getContext().context, "elseBlock", currentFunction);

        // Create the merge block
        llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(compiler.getContext().context, "mergeBlock", currentFunction);

        // Create the conditional branch in the condition block
        // compiler.getContext().builder.SetInsertPoint(conditionBlock);
        compiler.getContext().builder.CreateCondBr(conditionValue, thenBlock, elseBlock);

        // Generate code for the then block
        compiler.getContext().builder.SetInsertPoint(thenBlock);
        if (!thenBranch)
        {
            // Create a branch to the merge block
            compiler.getContext().builder.CreateBr(mergeBlock);
        }
        else
        {
            compiler.getGenerator().parseTree(thenBranch);

            // If the then block doesn't have a terminator, create a branch to the merge block
            if (!thenBlock->getTerminator())
            {
                compiler.getContext().builder.CreateBr(mergeBlock);
            }
        }

        // Generate code for the else block
        compiler.getContext().builder.SetInsertPoint(elseBlock);
        if (elseBranch)
        {
            DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "Creating Else Block");
            compiler.getGenerator().parseTree(elseBranch);
        }
        if (!elseBlock->getTerminator() || elseBranch == nullptr)
        {
            DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "Creating Branch in Else Block");
            compiler.getContext().builder.CreateBr(mergeBlock);
        }

        // No clue why this worked below, but it did
        compiler.getContext().builder.ClearInsertionPoint();

        // Set the insert point to the merge block
        compiler.getContext().builder.SetInsertPoint(mergeBlock);

        DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "If Statement Created");

        return conditionBlock;
    }

    std::pair<llvm::BasicBlock *, llvm::Value *> IfStatements::createIfCondition(ASTNode *node)
    {
        BinaryExpressions &binExp = compiler.getBinaryExpressions();
        DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "Creating If Condition");

        assert(node != nullptr);

        CryoNodeType nodeType = node->metaData->type;
        std::cout << "<!>Node Type: " << CryoNodeTypeToString(nodeType) << std::endl;

        llvm::Function *currentFunction = compiler.getContext().builder.GetInsertBlock()->getParent();
        // Create the condition block
        llvm::BasicBlock *conditionBlock = llvm::BasicBlock::Create(compiler.getContext().context, "ifCondition", currentFunction);
        // Set the `br label %ifCondition` in the parent block
        compiler.getContext().builder.CreateBr(conditionBlock);
        // Set the insert point to the condition block
        compiler.getContext().builder.SetInsertPoint(conditionBlock);

        // Create the condition
        llvm::Value *condition = nullptr;
        switch (nodeType)
        {
        case NODE_LITERAL_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "Handling Literal Expression");

            break;
        }
        case NODE_VAR_NAME:
        {
            DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "Handling Variable Name");
            // Get the variable name
            char *varName = node->data.varName->varName;
            // Get the current namespace
            std::string namespaceName = compiler.getContext().currentNamespace;
            STVariable *stVarNode = compiler.getSymTable().getVariable(namespaceName, varName);
            if (!stVarNode)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "IfStatements", "Variable not found");
                exit(0);
            }

            llvm::Value *varValue = stVarNode->LLVMValue;
            if (!varValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "IfStatements", "Variable value not found");
                exit(0);
            }

            // Create the condition
            condition = compiler.getContext().builder.CreateICmpNE(varValue, llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(32, 0, true)), "ifCondition");

            break;
        }
        case NODE_BINARY_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "IfStatements", "Handling Binary Expression");
            // Get the left and right operands
            ASTNode *leftOperand = node->data.bin_op->left;
            ASTNode *rightOperand = node->data.bin_op->right;
            // Get the operator
            CryoOperatorType binaryOperator = node->data.bin_op->op;

            condition = binExp.createComparisonExpression(leftOperand, rightOperand, binaryOperator, conditionBlock);

            break;
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "IfStatements", "Unknown node type @ IfStatements::createIfCondition");
            std::cout << "Received: " << CryoNodeTypeToString(nodeType) << std::endl;
            break;
        }
        }

        return {conditionBlock, condition};
    }

} // namespace Cryo