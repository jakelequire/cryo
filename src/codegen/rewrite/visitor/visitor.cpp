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
#include "codegen/rewrite/visitor.hpp"

namespace Cryo
{
    void CodeGenVisitor::visitLiteralExpr(ASTNode *node) override
    {
        // Code generation logic for literal expressions
        llvm::Value *returnValue = generator.getInitilizerValue(node);
    }

    void CodeGenVisitor::visitVarName(ASTNode *node) override
    {
        // Code generation logic for variable names
        std::string varName = node->data.varName->varName;
        // TODO: Implement logic for variable names
    }

    void CodeGenVisitor::visitBinaryExpr(ASTNode *node) override
    {
        // Code generation logic for binary expressions
        llvm::Value *returnValue = generator.getInitilizerValue(node);
        // TODO: Implement logic for binary expressions
    }

    void CodeGenVisitor::visitFunctionCall(ASTNode *node) override
    {
        // Code generation logic for function calls
        llvm::Value *returnValue = createFunctionCall(node);
        // TODO: Implement logic for function calls
    }

    void CodeGenVisitor::visitReturnStatement(ASTNode *node) override
    {
        // Code generation logic for return statements
        llvm::Value *returnValue = generator.getInitilizerValue(node);
        cryoContext.builder.CreateRet(returnValue);
    }
} // namespace Cryo
