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
#include "codegen_rewrite/visitor.hpp"

namespace Cryo
{
    // Expressions
    void CodeGenVisitor::visitLiteralExpr(ASTNode *node)
    {
        if (!node || !node->data.literal)
            return;

        LiteralNode *literal = node->data.literal;
        llvm::Value *value = nullptr;

        switch (literal->type->container->baseType)
        {
        // Integer literals
        case PRIM_INT:
            value = llvm::ConstantInt::get(builder.getInt32Ty(), literal->value.intValue);
            break;
        case PRIM_I8:
            value = llvm::ConstantInt::get(builder.getInt8Ty(), literal->value.intValue);
            break;
        case PRIM_I16:
            value = llvm::ConstantInt::get(builder.getInt16Ty(), literal->value.intValue);
            break;
        case PRIM_I32:
            value = llvm::ConstantInt::get(builder.getInt32Ty(), literal->value.intValue);
            break;
        case PRIM_I64:
            value = llvm::ConstantInt::get(builder.getInt64Ty(), literal->value.intValue);
            break;
        case PRIM_I128:
            value = llvm::ConstantInt::get(builder.getInt128Ty(), literal->value.intValue);
            break;
        case PRIM_FLOAT:
            value = llvm::ConstantFP::get(builder.getDoubleTy(), literal->value.floatValue);
            break;
        // Boolean literals
        case PRIM_BOOLEAN:
            value = llvm::ConstantInt::get(builder.getInt1Ty(), literal->value.booleanValue);
            break;
        // String literals
        case PRIM_STRING:
            value = builder.CreateGlobalStringPtr(literal->value.stringValue);
            break;
        // Null & Void literals
        case PRIM_NULL:
            value = llvm::ConstantPointerNull::get(builder.getInt8Ty()->getPointerTo());
            break;
        case PRIM_VOID:
            value = llvm::UndefValue::get(builder.getVoidTy());
            break;
        }

        // Store the generated value in the current node's context
        // This will be used by parent nodes
        node->data.literal->value.intValue = reinterpret_cast<intptr_t>(value);
    }

    void CodeGenVisitor::visitVarName(ASTNode *node)
    {
        if (!node || !node->data.varName)
            return;

        VariableNameNode *varName = node->data.varName;
        IRVariableSymbol *symbol = symbolTable->findVariable(varName->varName);

        if (!symbol)
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor",
                       "Variable %s not found", varName->varName);
            return;
        }

        // Load the variable if needed
        if (symbol->allocaType == AllocaType::AllocaAndLoad ||
            symbol->allocaType == AllocaType::AllocaLoadStore)
        {
            symbol->allocation.load(builder, std::string(varName->varName) + "_load");
        }
    }

    void CodeGenVisitor::visitBinaryExpr(ASTNode *node)
    {
        if (!node || !node->data.bin_op)
            return;

        // Visit left and right operands first
        visit(node->data.bin_op->left);
        visit(node->data.bin_op->right);

        // Get the values generated for left and right operands
        llvm::Value *left = getLLVMValue(node->data.bin_op->left);
        llvm::Value *right = getLLVMValue(node->data.bin_op->right);

        if (!left || !right)
            return;

        llvm::Value *result = nullptr;
        switch (node->data.bin_op->op)
        {
        case OPERATOR_ADD:
            result = builder.CreateAdd(left, right, "addtmp");
            break;
        case OPERATOR_SUB:
            result = builder.CreateSub(left, right, "subtmp");
            break;
            // Add other operators as needed
        }

        // Store the result for parent nodes
        storeValueInNode(node, result);
    }


    void CodeGenVisitor::visitFunctionCall(ASTNode *node)
    {
        if (!node || !node->data.functionCall)
            return;

        FunctionCallNode *call = node->data.functionCall;

        // Find the function in the symbol table
        IRFunctionSymbol *funcSymbol = symbolTable->findFunction(call->name);
        if (!funcSymbol)
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor",
                       "Function %s not found", call->name);
            return;
        }

        // Process arguments
        std::vector<llvm::Value *> args;
        for (int i = 0; i < call->argCount; i++)
        {
            visit(call->args[i]);
            llvm::Value *argValue = getLLVMValue(call->args[i]);
            if (argValue)
                args.push_back(argValue);
        }

        // Create the function call
        llvm::Value *result = builder.CreateCall(funcSymbol->function, args);
        storeValueInNode(node, result);
    }

    void CodeGenVisitor::visitMethodCall(ASTNode *node) {}
    void CodeGenVisitor::visitUnaryExpr(ASTNode *node) {}
    void CodeGenVisitor::visitArrayLiteral(ASTNode *node) {}
    void CodeGenVisitor::visitIndexExpr(ASTNode *node) {}
    void CodeGenVisitor::visitTypeofExpr(ASTNode *node) {}
} // namespace Cryo
