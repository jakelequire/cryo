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

    llvm::Value *UnaryExpressions::handleUnaryExpression(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Expression");
        if (node->metaData->type != NODE_UNARY_EXPR)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Node is not a unary expression");
            exit(1);
        }
        CryoUnaryOpNode *unaryNode = node->data.unary_op;
        CryoTokenType op = unaryNode->op;
        ASTNode *operand = unaryNode->operand;

        switch (op)
        {
        case TOKEN_MINUS:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Minus Expression");
            llvm::Value *operandValue = createUnaryExpression(operand);
            return compiler.getContext().builder.CreateNeg(operandValue);
        }
        case TOKEN_PLUS:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Plus Expression");
            llvm::Value *operandValue = createUnaryExpression(operand);
            return operandValue;
        }
        case TOKEN_OP_NOT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Not Expression");
            llvm::Value *operandValue = createUnaryExpression(operand);
            return compiler.getContext().builder.CreateNot(operandValue);
        }
        case TOKEN_INCREMENT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Increment Expression");
            llvm::Value *operandValue = createUnaryExpression(operand);
            return compiler.getContext().builder.CreateAdd(
                operandValue, llvm::ConstantInt::get(llvm::Type::getInt32Ty(compiler.getContext().context), 1));
        }
        case TOKEN_DECREMENT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Decrement Expression");
            llvm::Value *operandValue = createUnaryExpression(operand);
            return compiler.getContext().builder.CreateSub(
                operandValue, llvm::ConstantInt::get(llvm::Type::getInt32Ty(compiler.getContext().context), 1));
        }
        case TOKEN_STAR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Dereference Expression");
            llvm::Value *operandValue = createUnaryExpression(operand);
            return compiler.getContext().builder.CreateLoad(operandValue->getType(), operandValue);
        }
        case TOKEN_AMPERSAND:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Address Of Expression");
            llvm::Value *operandValue = createUnaryExpression(operand);
            return operandValue;
        }
        case TOKEN_DEREFERENCE:
        {
            DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Dereference Expression");
            llvm::Value *operandValue = createUnaryExpression(operand);
            return compiler.getContext().builder.CreateLoad(operandValue->getType(), operandValue);
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Unknown unary operator");
            std::string opStr = CryoTokenToString(op);
            std::cout << "Received: " << opStr << std::endl;

            CONDITION_FAILED;
        }
        }

        logMessage(LMI, "ERROR", "CodeGen", "Unknown unary operator");
        return nullptr;
    }

    llvm::Value *UnaryExpressions::createUnaryExpression(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Creating Unary Expression");
        llvm::Value *operandValue = compiler.getGenerator().getInitilizerValue(node);
        if (!operandValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "CodeGen", "Operand value not found");
            exit(1);
        }
        return operandValue;
    }

} // namespace Cryo
