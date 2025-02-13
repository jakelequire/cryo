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
#ifndef VISITOR_H
#define VISITOR_H
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "codegen/devDebugger/devDebugger.hpp"
#include "frontend/AST.h"
#include "frontend/dataTypes.h"

namespace Cryo
{

    class ASTNode; // Forward declaration

    class Visitor
    {
    public:
        virtual void visitLiteralExpr(ASTNode *node) = 0;
        virtual void visitVarName(ASTNode *node) = 0;
        virtual void visitBinaryExpr(ASTNode *node) = 0;
        virtual void visitFunctionCall(ASTNode *node) = 0;
        virtual void visitReturnStatement(ASTNode *node) = 0;
        // Add other visit methods as needed
    };

    class CodeGenVisitor : public Visitor
    {
    public:
        void visitLiteralExpr(ASTNode *node) override;

        void visitVarName(ASTNode *node) override;

        void visitBinaryExpr(ASTNode *node) override;

        void visitFunctionCall(ASTNode *node) override;

        void visitReturnStatement(ASTNode *node) override;

        // Implement other visit methods as needed...
    };

} // namespace Cryo

#endif // VISITOR_H
