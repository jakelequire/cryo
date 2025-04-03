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
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled node type: %s", nodeTypeStr.c_str());
            return nullptr;
        }
    }

    llvm::Value *Initializer::generateLiteralExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating literal expression...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_LITERAL_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a literal expression");

            return nullptr;
        }

        DataType *literalDataType = node->data.literal->type;
        logMessage(LMI, "INFO", "Initializer", "Literal data type: %s", DTM->debug->dataTypeToString(literalDataType));

        TypeofDataType typeofDataType = literalDataType->container->typeOf;
        logMessage(LMI, "INFO", "Initializer", "Type of data: %s", DTM->debug->typeofDataTypeToString(typeofDataType));

        node->print(node);

        switch (literalDataType->container->primitive)
        {
        case PRIM_INT:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(32, node->data.literal->value.intValue, true));
        case PRIM_I8:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(8, node->data.literal->value.intValue, true));
        case PRIM_I16:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(16, node->data.literal->value.intValue, true));
        case PRIM_I32:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(32, node->data.literal->value.intValue, true));
        case PRIM_I64:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(64, node->data.literal->value.intValue, true));
        case PRIM_I128:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(128, node->data.literal->value.intValue, true));
        case PRIM_FLOAT:
            return llvm::ConstantFP::get(context.getInstance().context, llvm::APFloat(node->data.literal->value.floatValue));
        case PRIM_BOOLEAN:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(1, node->data.literal->value.booleanValue, true));
        case PRIM_STRING:
            return generateStringLiteral(node);
        case PRIM_NULL:
            return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(context.getInstance().context), 0));
        case PRIM_VOID:
            return llvm::UndefValue::get(llvm::Type::getVoidTy(context.getInstance().context));
        case PRIM_ANY:
            return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(context.getInstance().context), 0));
        case PRIM_OBJECT:
            return nullptr;
        case PRIM_AUTO:
            return nullptr;
        case PRIM_UNDEFINED:
            return nullptr;
        case PRIM_UNKNOWN:
            return nullptr;
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled literal type: %s", DTM->debug->dataTypeToString(literalDataType));
            return nullptr;
        }

        std::cerr << "Unhandled literal type: " << DTM->debug->dataTypeToString(literalDataType) << std::endl;
        return nullptr;
    }

    llvm::Value *Initializer::generateVarName(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating variable name...");
        ASSERT_NODE_NULLPTR_RET(node);
        if (node->metaData->type != NODE_VAR_NAME)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a variable name");
            return nullptr;
        }

        std::string varName = node->data.varName->varName;
        logMessage(LMI, "INFO", "Initializer", "Variable name: %s", varName.c_str());
        IRVariableSymbol *varSymbol = getSymbolTable()->findVariable(varName);
        if (!varSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Variable %s not found", varName.c_str());
            // Print the symbol table for debugging
            context.getInstance().symbolTable->debugPrint();
            return nullptr;
        }

        // Load the variable if needed
        if (varSymbol->allocaType == AllocaType::AllocaAndLoad ||
            varSymbol->allocaType == AllocaType::AllocaLoadStore)
        {
            // varSymbol->allocation.load(context.getInstance().builder, varName + "_load");
        }
        return varSymbol->allocation.getValue();
    }

    llvm::Value *Initializer::generateBinaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating binary expression...");
        ASSERT_NODE_NULLPTR_RET(node);
        // TODO: Generate binary expression
    }

    llvm::Value *Initializer::generateFunctionCall(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating function call...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_FUNCTION_CALL)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a function call");
            return nullptr;
        }

        // Find the function in the symbol table
        IRFunctionSymbol *funcSymbol = getSymbolTable()->findFunction(node->data.functionCall->name);
        if (!funcSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function %s not found", node->data.functionCall->name);
            return nullptr;
        }

        // Process arguments
        std::vector<llvm::Value *> args;
        int argCount = node->data.functionCall->argCount;
        for (int i = 0; i < argCount; i++)
        {
            args.push_back(getInitializerValue(node->data.functionCall->args[i]));
        }
        std::string funcName = node->data.functionCall->name;
        logMessage(LMI, "INFO", "Initializer", "Function call: %s", funcName.c_str());

        return context.getInstance().builder.CreateCall(funcSymbol->function, args, funcName);
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

    llvm::Value *Initializer::generateUnaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating unary expression...");
        ASSERT_NODE_NULLPTR_RET(node);
        // TODO: Generate unary expression
    }

} // namespace Cryo
