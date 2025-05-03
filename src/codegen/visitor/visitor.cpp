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
    CodeGenVisitor::CodeGenVisitor(CodegenContext &ctx)
        : context(ctx), builder(ctx.builder), symbolTable(ctx.symbolTable), lastValue(nullptr), initializer(ctx.initializer)
    {
    }

    void Visitor::visit(ASTNode *node)
    {
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }

        std::string nodeTypeStr = CryoNodeTypeToString(node->metaData->type);
        logMessage(LMI, "INFO", "Visitor", "Visiting node: %s", nodeTypeStr.c_str());

        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
            visitProgram(node);
            break;
        case NODE_MODULE:
            visitModule(node);
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
        case NODE_FUNCTION_BLOCK:
            visitFunctionBlock(node);
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
        case NODE_TYPE:
            visitTypeDecl(node);
            break;
        case NODE_SCOPED_FUNCTION_CALL:
            visitScopedFunctionCall(node);
            break;
        case NODE_BREAK:
            visitBreakStatement(node);
            break;
        case NODE_CONTINUE:
            visitContinueStatement(node);
            break;
        case NODE_VAR_REASSIGN:
            visitVarReassignment(node);
            break;
        default:
            logMessage(LMI, "ERROR", "Visitor", "Unhandled node type: %s", nodeTypeStr.c_str());
            break;
        }
    }
 
    // ==========================================================
    // Private methods

    llvm::Value *CodeGenVisitor::getLLVMValue(ASTNode *node)
    {
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return nullptr;
        }

        return initializer->getInitializerValue(node);
    }

} // namespace Cryo
