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

#include "codegen/devDebugger/devDebugger.hpp"
#include "frontend/AST.h"
#include "codegen_rewrite/symTable/IRSymbolTable.hpp"
#include "codegen_rewrite/codegen.hpp"
#include "codegen_rewrite/codegenDebug.hpp"

typedef struct ASTNode ASTNode;

namespace Cryo
{
    // Forward declarations
    class IRSymbolTable;
    class Visitor;
    class DefaultVisitor;
    class CodeGenVisitor;
    class CodegenContext;
    class Initializer;

    // Base Visitor class
    class Visitor
    {
    public:
        virtual ~Visitor() = default;

        // Program structure
        virtual void visitProgram(ASTNode *node) = 0;
        virtual void visitFunctionBlock(ASTNode *node) = 0;
        virtual void visitBlock(ASTNode *node) = 0;
        virtual void visitNamespace(ASTNode *node) = 0;
        virtual void visitImport(ASTNode *node) = 0;
        virtual void visitUsing(ASTNode *node) = 0;
        virtual void visitModule(ASTNode *node) = 0;

        // Declarations
        virtual void visitFunctionDecl(ASTNode *node) = 0;
        virtual void visitExternFuncDecl(ASTNode *node) = 0;
        virtual void visitVarDecl(ASTNode *node) = 0;
        virtual void visitStructDecl(ASTNode *node) = 0;
        virtual void visitClassDecl(ASTNode *node) = 0;
        virtual void visitEnumDecl(ASTNode *node) = 0;
        virtual void visitGenericDecl(ASTNode *node) = 0;

        // Expressions
        virtual void visitLiteralExpr(ASTNode *node) = 0;
        virtual void visitVarName(ASTNode *node) = 0;
        virtual void visitBinaryExpr(ASTNode *node) = 0;
        virtual void visitUnaryExpr(ASTNode *node) = 0;
        virtual void visitFunctionCall(ASTNode *node) = 0;
        virtual void visitMethodCall(ASTNode *node) = 0;
        virtual void visitArrayLiteral(ASTNode *node) = 0;
        virtual void visitIndexExpr(ASTNode *node) = 0;
        virtual void visitTypeofExpr(ASTNode *node) = 0;

        // Statements
        virtual void visitIfStatement(ASTNode *node) = 0;
        virtual void visitForStatement(ASTNode *node) = 0;
        virtual void visitWhileStatement(ASTNode *node) = 0;
        virtual void visitReturnStatement(ASTNode *node) = 0;

        // Object-oriented features
        virtual void visitProperty(ASTNode *node) = 0;
        virtual void visitMethod(ASTNode *node) = 0;
        virtual void visitConstructor(ASTNode *node) = 0;
        virtual void visitPropertyAccess(ASTNode *node) = 0;
        virtual void visitPropertyReassignment(ASTNode *node) = 0;
        virtual void visitThis(ASTNode *node) = 0;

        // Common entry point for visiting any node
        virtual void visit(ASTNode *node);
    };

    // Code Generation Visitor
    class CodeGenVisitor : public Visitor
    {
        friend class CodegenContext;
        friend class IRSymbolTable;
        friend class CodeGenDebug;
        friend class Initializer;

    private:
        CodegenContext &context;
        llvm::IRBuilder<> &builder;
        std::unique_ptr<IRSymbolTable> &symbolTable;
        std::unique_ptr<Initializer> &initializer;
        llvm::Value *lastValue;

    public:
        CodeGenVisitor(CodegenContext &ctx);

        // Get the last generated value
        llvm::Value *getValue() const { return lastValue; }

        // Program structure
        void visitProgram(ASTNode *node) override;
        void visitFunctionBlock(ASTNode *node) override;
        void visitBlock(ASTNode *node) override;
        void visitNamespace(ASTNode *node) override;
        void visitImport(ASTNode *node) override;
        void visitUsing(ASTNode *node) override;
        void visitModule(ASTNode *node) override;

        // Declarations
        void visitFunctionDecl(ASTNode *node) override;
        void visitExternFuncDecl(ASTNode *node) override;
        void visitVarDecl(ASTNode *node) override;
        void visitStructDecl(ASTNode *node) override;
        void visitClassDecl(ASTNode *node) override;
        void visitEnumDecl(ASTNode *node) override;
        void visitGenericDecl(ASTNode *node) override;

        // Expressions
        void visitLiteralExpr(ASTNode *node) override;
        void visitVarName(ASTNode *node) override;
        void visitBinaryExpr(ASTNode *node) override;
        void visitUnaryExpr(ASTNode *node) override;
        void visitFunctionCall(ASTNode *node) override;
        void visitMethodCall(ASTNode *node) override;
        void visitArrayLiteral(ASTNode *node) override;
        void visitIndexExpr(ASTNode *node) override;
        void visitTypeofExpr(ASTNode *node) override;

        // Statements
        void visitIfStatement(ASTNode *node) override;
        void visitForStatement(ASTNode *node) override;
        void visitWhileStatement(ASTNode *node) override;
        void visitReturnStatement(ASTNode *node) override;

        // Object-oriented features
        void visitProperty(ASTNode *node) override;
        void visitMethod(ASTNode *node) override;
        void visitConstructor(ASTNode *node) override;
        void visitPropertyAccess(ASTNode *node) override;
        void visitPropertyReassignment(ASTNode *node) override;
        void visitThis(ASTNode *node) override;

    private:
        llvm::Value *getLLVMValue(ASTNode *node);
    };

} // namespace Cryo

#endif // VISITOR_H