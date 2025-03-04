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
#include "codegen_rewrite/codegen.hpp"

namespace Cryo
{

    llvm::Value *Initilizer::getInitilizerValue(ASTNode *node)
    {
        ASSERT_NODE_NULLPTR_RET(node);
        std::string nodeTypeStr = CryoNodeTypeToString(node->metaData->type);
        logMessage(LMI, "INFO", "Initilizer", "Getting initializer value for node: %s", nodeTypeStr.c_str());

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
        default:
            logMessage(LMI, "ERROR", "Initilizer", "Unhandled node type: %s", nodeTypeStr.c_str());
            return nullptr;
        }
    }

    llvm::Value *Initilizer::generateLiteralExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initilizer", "Generating literal expression...");
        ASSERT_NODE_NULLPTR_RET(node);

        DataType *literalDataType = node->data.literal->type;
        TypeofDataType typeofDataType = literalDataType->container->baseType;
        switch (typeofDataType)
        {
        case PRIM_INT:
            return llvm::ConstantInt::get(context.context, llvm::APInt(32, node->data.literal->value.intValue, true));
        case PRIM_I8:
            return llvm::ConstantInt::get(context.context, llvm::APInt(8, node->data.literal->value.intValue, true));
        case PRIM_I16:
            return llvm::ConstantInt::get(context.context, llvm::APInt(16, node->data.literal->value.intValue, true));
        case PRIM_I32:
            return llvm::ConstantInt::get(context.context, llvm::APInt(32, node->data.literal->value.intValue, true));
        case PRIM_I64:
            return llvm::ConstantInt::get(context.context, llvm::APInt(64, node->data.literal->value.intValue, true));
        case PRIM_I128:
            return llvm::ConstantInt::get(context.context, llvm::APInt(128, node->data.literal->value.intValue, true));
        case PRIM_FLOAT:
            return llvm::ConstantFP::get(context.context, llvm::APFloat(node->data.literal->value.floatValue));
        case PRIM_BOOLEAN:
            return llvm::ConstantInt::get(context.context, llvm::APInt(1, node->data.literal->value.booleanValue, true));
        case PRIM_STRING:
            return context.builder.CreateGlobalStringPtr(node->data.literal->value.stringValue);
        default:
            logMessage(LMI, "ERROR", "Initilizer", "Unhandled literal type: %s", DataTypeToString(literalDataType));
            return nullptr;
        }
    }

    llvm::Value *Initilizer::generateVarName(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initilizer", "Generating variable name...");
        ASSERT_NODE_NULLPTR_RET(node);
        if (node->metaData->type != NODE_VAR_NAME)
        {
            logMessage(LMI, "ERROR", "Initilizer", "Node is not a variable name");
            return nullptr;
        }

        std::string varName = node->data.varName->varName;
        // TODO: Lookup variable in symbol table
    }

    llvm::Value *Initilizer::generateBinaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initilizer", "Generating binary expression...");
        ASSERT_NODE_NULLPTR_RET(node);
        // TODO: Generate binary expression
    }

    llvm::Value *Initilizer::generateFunctionCall(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initilizer", "Generating function call...");
        ASSERT_NODE_NULLPTR_RET(node);
        // TODO: Generate function call
    }

    llvm::Value *Initilizer::generateReturnStatement(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initilizer", "Generating return statement...");
        ASSERT_NODE_NULLPTR_RET(node);
        // TODO: Generate return statement
    }

    llvm::Value *Initilizer::generateUnaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initilizer", "Generating unary expression...");
        ASSERT_NODE_NULLPTR_RET(node);
        // TODO: Generate unary expression
    }

} // namespace Cryo
