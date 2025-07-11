/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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

#include "frontend/AST.h"
#include "codegen/symTable/IRSymbolTable.hpp"
#include "codegen/codegen.hpp"
#include "codegen/codegenDebug.hpp"

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
        virtual void visitModule(ASTNode *node) = 0;
        virtual void visitFunctionBlock(ASTNode *node) = 0;
        virtual void visitBlock(ASTNode *node) = 0;
        virtual void visitNamespace(ASTNode *node) = 0;
        virtual void visitImport(ASTNode *node) = 0;
        virtual void visitUsing(ASTNode *node) = 0;

        // Declarations
        virtual void visitFunctionDecl(ASTNode *node) = 0;
        virtual void visitExternFuncDecl(ASTNode *node) = 0;
        virtual void visitVarDecl(ASTNode *node) = 0;
        virtual void visitStructDecl(ASTNode *node) = 0;
        virtual void visitClassDecl(ASTNode *node) = 0;
        virtual void visitEnumDecl(ASTNode *node) = 0;
        virtual void visitGenericDecl(ASTNode *node) = 0;
        virtual void visitTypeDecl(ASTNode *node) = 0;
        virtual void visitScopedFunctionCall(ASTNode *node) = 0;

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
        virtual void visitVarReassignment(ASTNode *node) = 0;

        // Statements
        virtual void visitIfStatement(ASTNode *node) = 0;
        virtual void visitForStatement(ASTNode *node) = 0;
        virtual void visitWhileStatement(ASTNode *node) = 0;
        virtual void visitReturnStatement(ASTNode *node) = 0;
        virtual void visitBreakStatement(ASTNode *node) = 0;
        virtual void visitContinueStatement(ASTNode *node) = 0;

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
        void visitModule(ASTNode *node) override;
        void visitFunctionBlock(ASTNode *node) override;
        void visitBlock(ASTNode *node) override;
        void visitNamespace(ASTNode *node) override;
        void visitImport(ASTNode *node) override;
        void visitUsing(ASTNode *node) override;

        // Declarations
        void visitFunctionDecl(ASTNode *node) override;
        void visitExternFuncDecl(ASTNode *node) override;
        void visitVarDecl(ASTNode *node) override;
        void visitStructDecl(ASTNode *node) override;
        void visitClassDecl(ASTNode *node) override;
        void visitEnumDecl(ASTNode *node) override;
        void visitGenericDecl(ASTNode *node) override;
        void visitTypeDecl(ASTNode *node) override;
        void visitScopedFunctionCall(ASTNode *node) override;

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
        void visitVarReassignment(ASTNode *node) override;

        // Statements
        void visitIfStatement(ASTNode *node) override;
        void visitForStatement(ASTNode *node) override;
        void visitWhileStatement(ASTNode *node) override;
        void visitReturnStatement(ASTNode *node) override;
        void visitBreakStatement(ASTNode *node) override;
        void visitContinueStatement(ASTNode *node) override;

        // Object-oriented features
        void visitProperty(ASTNode *node) override;
        void visitMethod(ASTNode *node) override;
        void visitConstructor(ASTNode *node) override;
        void visitPropertyAccess(ASTNode *node) override;
        void visitPropertyReassignment(ASTNode *node) override;
        void visitThis(ASTNode *node) override;

    private:
        // Node validation
        bool validateNode(ASTNode *node, const char *nodeType);

        // Type checking helpers
        bool isStringWithLiteralInitializer(DataType *varType, ASTNode *initializer);
        bool isStructWithInitializer(DataType *varType, ASTNode *initializer);
        bool isArrayWithInitializer(DataType *varType, ASTNode *initializer);
        bool isIndexExpression(ASTNode *node);

        // Variable declaration handlers
        void handleStringVarDecl(const std::string &varName, DataType *varType, ASTNode *initializer);
        void handleStructVarDecl(const std::string &varName, DataType *varType, ASTNode *initializer);
        void handlePrimitiveVarDecl(const std::string &varName, DataType *varType, ASTNode *initializer);
        void handleArrayInitializer(const std::string &varName, DataType *varType, ASTNode *initializer);
        void handleNoInitializer(const std::string &varName, DataType *varType);

        // Initializer handlers
        llvm::Value *generateObjectInstance(llvm::AllocaInst *structAlloca, ASTNode *initializer);
        void storeInitializerValue(llvm::Value *initVal, llvm::AllocaInst *allocaInst, llvm::Type *llvmType);
        void handlePrimitiveInitializer(llvm::AllocaInst *allocaInst, ASTNode *initializer, llvm::Type *llvmType);
        void handleIndexExpression(const std::string &varName, DataType *varType, ASTNode *initializer);

        // Type conversion and alignment
        llvm::Value *convertInitializerType(llvm::Value *initVal, llvm::Type *targetType);
        void setAppropriateAlignment(llvm::AllocaInst *allocaInst, llvm::Type *llvmType);

        // Symbol table operations
        void addVariableToSymbolTable(const std::string &varName, DataType *varType,
                                      llvm::Value *value, llvm::Type *pointerType,
                                      AllocaType allocaType);

    private:
        // Class declaration helpers
        void processProperties(
            void *members,
            std::vector<llvm::Type *> &propertyTypes,
            std::vector<IRPropertySymbol> &propertySymbols,
            bool isPublic);

        void processMethods(
            void *members,
            std::vector<IRMethodSymbol> &methodSymbols,
            const std::string &className);

    private:
        llvm::Value *getLLVMValue(ASTNode *node);
        llvm::Value *performTypeCast(llvm::Value *value, llvm::Type *targetType);
        llvm::Value *convertValueToTargetType(llvm::Value *value, DataType *sourceType, DataType *targetType);
    };

} // namespace Cryo

#endif // VISITOR_H