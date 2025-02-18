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
#include "codegen_rewrite/symTable/IRSymbolTable.hpp"
#include "codegen_rewrite/codegen.hpp"

typedef struct ASTNode ASTNode;

namespace Cryo
{
    // Forward declarations
    class CodegenContext;
    class IRSymbolTable;
    class Visitor;
    class DefaultVisitor;
    class CodeGenVisitor;

    // Base Visitor class
    class Visitor
    {
    public:
        virtual ~Visitor() = default;

        // Program structure
        virtual void visitProgram(ASTNode *node) = 0;
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
        virtual void visitBlock(ASTNode *node) = 0;
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
        virtual void visit(ASTNode *node)
        {
            if (!node)
                return;

            switch (node->metaData->type)
            {
            case NODE_PROGRAM:
                visitProgram(node);
                break;
            case NODE_NAMESPACE:
                visitNamespace(node);
                break;
            case NODE_IMPORT_STATEMENT:
                visitImport(node);
                break;
            case NODE_USING:
                visitUsing(node);
                break;
            case NODE_MODULE:
                visitModule(node);
                break;
            case NODE_FUNCTION_DECLARATION:
                visitFunctionDecl(node);
                break;
            case NODE_EXTERN_FUNCTION:
                visitExternFuncDecl(node);
                break;
            case NODE_VAR_DECLARATION:
                visitVarDecl(node);
                break;
            case NODE_STRUCT_DECLARATION:
                visitStructDecl(node);
                break;
            case NODE_CLASS:
                visitClassDecl(node);
                break;
            case NODE_ENUM:
                visitEnumDecl(node);
                break;
            case NODE_GENERIC_DECL:
                visitGenericDecl(node);
                break;
            case NODE_LITERAL_EXPR:
                visitLiteralExpr(node);
                break;
            case NODE_VAR_NAME:
                visitVarName(node);
                break;
            case NODE_BINARY_EXPR:
                visitBinaryExpr(node);
                break;
            case NODE_UNARY_EXPR:
                visitUnaryExpr(node);
                break;
            case NODE_FUNCTION_CALL:
                visitFunctionCall(node);
                break;
            case NODE_METHOD_CALL:
                visitMethodCall(node);
                break;
            case NODE_ARRAY_LITERAL:
                visitArrayLiteral(node);
                break;
            case NODE_INDEX_EXPR:
                visitIndexExpr(node);
                break;
            case NODE_TYPEOF:
                visitTypeofExpr(node);
                break;
            case NODE_BLOCK:
                visitBlock(node);
                break;
            case NODE_IF_STATEMENT:
                visitIfStatement(node);
                break;
            case NODE_FOR_STATEMENT:
                visitForStatement(node);
                break;
            case NODE_WHILE_STATEMENT:
                visitWhileStatement(node);
                break;
            case NODE_RETURN_STATEMENT:
                visitReturnStatement(node);
                break;
            case NODE_PROPERTY:
                visitProperty(node);
                break;
            case NODE_METHOD:
                visitMethod(node);
                break;
            case NODE_STRUCT_CONSTRUCTOR:
            case NODE_CLASS_CONSTRUCTOR:
                visitConstructor(node);
                break;
            case NODE_PROPERTY_ACCESS:
                visitPropertyAccess(node);
                break;
            case NODE_PROPERTY_REASSIGN:
                visitPropertyReassignment(node);
                break;
            case NODE_THIS:
                visitThis(node);
                break;
            default:
                // Handle unknown node types or provide a default behavior
                break;
            }
        }
    };

    // Default empty implementation of Visitor
    class DefaultVisitor : public Visitor
    {
    public:
        virtual ~DefaultVisitor() = default;

        // Program structure
        void visitProgram(ASTNode *node) override {}
        void visitNamespace(ASTNode *node) override {}
        void visitImport(ASTNode *node) override {}
        void visitUsing(ASTNode *node) override {}
        void visitModule(ASTNode *node) override {}

        // Declarations
        void visitFunctionDecl(ASTNode *node) override {}
        void visitExternFuncDecl(ASTNode *node) override {}
        void visitVarDecl(ASTNode *node) override {}
        void visitStructDecl(ASTNode *node) override {}
        void visitClassDecl(ASTNode *node) override {}
        void visitEnumDecl(ASTNode *node) override {}
        void visitGenericDecl(ASTNode *node) override {}

        // Expressions
        void visitLiteralExpr(ASTNode *node) override {}
        void visitVarName(ASTNode *node) override {}
        void visitBinaryExpr(ASTNode *node) override {}
        void visitUnaryExpr(ASTNode *node) override {}
        void visitFunctionCall(ASTNode *node) override {}
        void visitMethodCall(ASTNode *node) override {}
        void visitArrayLiteral(ASTNode *node) override {}
        void visitIndexExpr(ASTNode *node) override {}
        void visitTypeofExpr(ASTNode *node) override {}

        // Statements
        void visitBlock(ASTNode *node) override {}
        void visitIfStatement(ASTNode *node) override {}
        void visitForStatement(ASTNode *node) override {}
        void visitWhileStatement(ASTNode *node) override {}
        void visitReturnStatement(ASTNode *node) override {}

        // Object-oriented features
        void visitProperty(ASTNode *node) override {}
        void visitMethod(ASTNode *node) override {}
        void visitConstructor(ASTNode *node) override {}
        void visitPropertyAccess(ASTNode *node) override {}
        void visitPropertyReassignment(ASTNode *node) override {}
        void visitThis(ASTNode *node) override {}
    };

    // Code Generation Visitor
    class CodeGenVisitor : public DefaultVisitor
    {
    private:
        CodegenContext &context;
        llvm::IRBuilder<> &builder;
        std::unique_ptr<IRSymbolTable> &symbolTable;
        llvm::Value *lastValue;

    public:
        CodeGenVisitor(CodegenContext &ctx)
            : context(ctx), builder(ctx.builder), symbolTable(ctx.symbolTable), lastValue(nullptr) {}

        // Get the last generated value
        llvm::Value *getValue() const { return lastValue; }

        // Override only the methods you want to implement for code generation
        void visitLiteralExpr(ASTNode *node) override;
        void visitVarName(ASTNode *node) override;
        void visitBinaryExpr(ASTNode *node) override;
        void visitFunctionCall(ASTNode *node) override;
        void visitReturnStatement(ASTNode *node) override;

    private:
        llvm::Value *getLLVMValue(ASTNode *node);
        void storeValueInNode(ASTNode *node, llvm::Value *value);
    };

} // namespace Cryo

#endif // VISITOR_H