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
    void CodeGenVisitor::visitLiteralExpr(ASTNode *node)
    {
        if (!node || !node->data.literal)
            return;

        LiteralNode *literal = node->data.literal;
        llvm::Value *value = nullptr;

        switch (literal->type->container->baseType)
        {
        case PRIM_INT:
            value = llvm::ConstantInt::get(builder.getInt32Ty(), literal->value.intValue);
            break;
        case PRIM_FLOAT:
            value = llvm::ConstantFP::get(builder.getDoubleTy(), literal->value.floatValue);
            break;
        case PRIM_BOOLEAN:
            value = llvm::ConstantInt::get(builder.getInt1Ty(), literal->value.booleanValue);
            break;
            // Add other literal types as needed
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
        visitASTNode(node->data.bin_op->left);
        visitASTNode(node->data.bin_op->right);

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
            visitASTNode(call->args[i]);
            llvm::Value *argValue = getLLVMValue(call->args[i]);
            if (argValue)
                args.push_back(argValue);
        }

        // Create the function call
        llvm::Value *result = builder.CreateCall(funcSymbol->function, args);
        storeValueInNode(node, result);
    }

    void CodeGenVisitor::visitReturnStatement(ASTNode *node)
    {
        if (!node || !node->data.returnStatement)
            return;

        // Visit the return value expression
        visitASTNode(node->data.returnStatement->returnValue);

        // Get the generated value
        llvm::Value *retVal = getLLVMValue(node->data.returnStatement->returnValue);
        if (retVal)
        {
            builder.CreateRet(retVal);
        }
        else
        {
            builder.CreateRetVoid();
        }
    }

    // ==========================================================
    // Private methods

    void CodeGenVisitor::visitASTNode(ASTNode *node)
    {
        if (!node)
            return;

        switch (node->metaData->type)
        {
        case NODE_LITERAL_EXPR:
            visitLiteralExpr(node);
            break;
        case NODE_VAR_NAME:
            visitVarName(node);
            break;
        case NODE_BINARY_EXPR:
            visitBinaryExpr(node);
            break;
        case NODE_FUNCTION_CALL:
            visitFunctionCall(node);
            break;
        case NODE_RETURN_STATEMENT:
            visitReturnStatement(node);
            break;
            // Add other node types as needed
        }
    }

    llvm::Value *CodeGenVisitor::getLLVMValue(ASTNode *node)
    {
        // Helper to extract LLVM value from a node
        // Implementation depends on how you store generated values
        return nullptr; // Implement actual value retrieval
    }

    void CodeGenVisitor::storeValueInNode(ASTNode *node, llvm::Value *value)
    {
        // Helper to store LLVM value in a node for parent nodes to use
        // Implementation depends on your storage strategy
    }

} // namespace Cryo
