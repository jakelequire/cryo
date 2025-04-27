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

    llvm::Value *Initializer::getInitializerValue(ASTNode *node)
    {
        ASSERT_NODE_NULLPTR_RET(node);
        std::string nodeTypeStr = CryoNodeTypeToString(node->metaData->type);
        logMessage(LMI, "INFO", "Initializer", "Getting initializer value for node: %s", nodeTypeStr.c_str());

        switch (node->metaData->type)
        {
        case NODE_LITERAL_EXPR:
            return generateLiteralExpr(node);
        case NODE_VAR_NAME:
            return generateVarName(node);
        case NODE_BINARY_EXPR:
            return generateBinaryExpr(node);
        case NODE_FUNCTION_CALL:
            return generateFunctionCall(node);
        case NODE_RETURN_STATEMENT:
            return generateReturnStatement(node);
        case NODE_UNARY_EXPR:
            return generateUnaryExpr(node);
        case NODE_PARAM:
            return generateParam(node);
        case NODE_PROPERTY_ACCESS:
            return generatePropertyAccess(node);
        case NODE_SCOPED_FUNCTION_CALL:
            return generateScopedFunctionCall(node);
        case NODE_VAR_DECLARATION:
            return generateVarDeclaration(node);
        case NODE_OBJECT_INST:
            return generateObjectInst(node);
        case NODE_NULL_LITERAL:
            return generateNullLiteral();
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled node type: %s", nodeTypeStr.c_str());
            return nullptr;
        }
    }

    llvm::Value *Initializer::generateReturnStatement(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating return statement...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_RETURN_STATEMENT)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a return statement");
            return nullptr;
        }

        return getInitializerValue(node->data.returnStatement->expression);
    }

    llvm::Value *Initializer::generateParam(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating parameter...");
        ASSERT_NODE_NULLPTR_RET(node);
        if (node->metaData->type != NODE_PARAM)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a parameter");
            return nullptr;
        }

        // All parameters are locally allocated variables
        // They are allocated when the function is created and should already be existing in the current scope

        std::string paramName = node->data.param->name;
        logMessage(LMI, "INFO", "Initializer", "Parameter name: %s", paramName.c_str());

        IRVariableSymbol *paramSymbol = context.getInstance().symbolTable->findVariable(paramName);
        if (!paramSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Parameter %s not found", paramName.c_str());
            // Print the symbol table for debugging
            context.getInstance().symbolTable->debugPrint();
            DEBUG_BREAKPOINT;
            return nullptr;
        }
        logMessage(LMI, "INFO", "Initializer", "Parameter %s found", paramName.c_str());

        // Load the parameter if needed
        if (paramSymbol->allocaType == AllocaType::AllocaAndLoad ||
            paramSymbol->allocaType == AllocaType::AllocaLoadStore)
        {
            // paramSymbol->allocation.load(context.getInstance().builder, paramName + ".load");
        }

        return paramSymbol->value;
    }

    llvm::Value *Initializer::generateVarDeclaration(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating variable declaration...");
        DEBUG_BREAKPOINT;
    }

    llvm::Value *Initializer::generateNullLiteral()
    {
        logMessage(LMI, "INFO", "Initializer", "Generating null literal...");
        llvm::Type *nullType = context.getInstance().builder.getInt8Ty();
        llvm::Value *nullValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(nullType, 0));
        return nullValue;
    }

} // namespace Cryo
