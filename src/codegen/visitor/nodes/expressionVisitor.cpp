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
    // Expressions
    void CodeGenVisitor::visitLiteralExpr(ASTNode *node)
    {
        if (!node || !node->data.literal)
            return;

        LiteralNode *literal = node->data.literal;
        llvm::Value *value = nullptr;

        switch (literal->type->container->typeOf)
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
            value = context.getInstance().initializer->getInitializerValue(node);
            break;
        // Null & Void literals
        case PRIM_NULL:
            value = llvm::ConstantPointerNull::get(builder.getInt8Ty()->getPointerTo());
            break;
        case PRIM_VOID:
            value = llvm::UndefValue::get(builder.getVoidTy());
            break;
        default:
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Unhandled literal type: %s",
                       DTM->debug->dataTypeToString(literal->type));
            return;
        }
        }
    }

    void CodeGenVisitor::visitVarName(ASTNode *node)
    {
        if (!node || !node->data.varName)
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Variable name node is NULL");
            return;
        }
        logMessage(LMI, "DEBUG", "CodeGenVisitor", "Visiting variable name node: %s",
                   node->data.varName->varName);
        // Find the variable in the symbol table

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
    }

    void CodeGenVisitor::visitFunctionCall(ASTNode *node)
    {
        if (!node || !node->data.functionCall)
            return;

        logMessage(LMI, "INFO", "CodeGenVisitor", "Visiting function call node: %s",
                   node->data.functionCall->name);

        FunctionCallNode *call = node->data.functionCall;

        // Find the function in the symbol table
        IRFunctionSymbol *funcSymbol = symbolTable->findFunction(node->data.functionCall->name);
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
            llvm::Value *argValue = getLLVMValue(call->args[i]);
            if (argValue)
            {
                logMessage(LMI, "INFO", "CodeGenVisitor",
                           "Argument %d value: %s", i, argValue->getName().str().c_str());
                // Check if the argument is a string literal
                bool isStringLiteral = false;
                DataType *argType = DTM->astInterface->getTypeofASTNode(call->args[i]);
                if (argType && argType->container->primitive == PRIM_STRING || argType->container->primitive == PRIM_STR)
                {
                    isStringLiteral = true;
                }

                logMessage(LMI, "INFO", "CodeGenVisitor",
                           "Argument %d is a string literal: %s", i, isStringLiteral ? "true" : "false");
                // For string literals, don't do any additional processing
                if (isStringLiteral)
                {
                    logMessage(LMI, "INFO", "CodeGenVisitor",
                               "Argument %d is a string literal: %s", i, argValue->getName().str().c_str());
                    // Just use the pointer as is
                    args.push_back(argValue);
                    continue;
                }

                logMessage(LMI, "INFO", "CodeGenVisitor",
                           "Argument %d is not a string literal: %s", i, argValue->getName().str().c_str());
                // For other types, continue with your existing logic
                if (i < funcSymbol->function->getFunctionType()->getNumParams())
                {
                    llvm::Type *paramType = funcSymbol->function->getFunctionType()->getParamType(i);
                    logMessage(LMI, "INFO", "CodeGenVisitor",
                               "Argument %d type: %s", i, paramType->getStructName().str().c_str());
                    // Check if argument is a pointer but parameter isn't
                    if (argValue->getType()->isPointerTy() && !paramType->isPointerTy())
                    {
                        logMessage(LMI, "INFO", "CodeGenVisitor",
                                   "Argument %d is a pointer but parameter is not: %s",
                                   i, argValue->getName().str().c_str());
                        // For opaque pointers, we need to determine the type to load in another way
                        if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(argValue))
                        {
                            logMessage(LMI, "INFO", "CodeGenVisitor",
                                       "Argument %d is a pointer but parameter is not: %s",
                                       i, argValue->getName().str().c_str());
                            llvm::Type *allocatedType = allocaInst->getAllocatedType();
                            argValue = context.getInstance().builder.CreateLoad(allocatedType, argValue,
                                                                                argValue->getName() + ".load");
                        }
                        else
                        {
                            logMessage(LMI, "ERROR", "CodeGenVisitor",
                                       "Argument %d is a pointer but parameter is not: %s",
                                       i, argValue->getName().str().c_str());
                            argValue = context.getInstance().builder.CreateLoad(paramType, argValue,
                                                                                argValue->getName() + ".load");
                        }
                    }
                }
                logMessage(LMI, "INFO", "CodeGenVisitor",
                           "Argument %d name: %s", i, argValue->getName().str().c_str());
                std::string argName = argValue->getName().str();
                argValue->setName("fc_idx_" + std::to_string(i) + "." + argName);
                args.push_back(argValue);
            }
        }

        // Create the function call
        std::string funcName = call->name;
        logMessage(LMI, "INFO", "CodeGenVisitor", "Function call: %s", funcName.c_str());
        llvm::Value *result = nullptr;
        if (funcSymbol->returnType->getTypeID() != llvm::Type::VoidTyID)
        {
            result = builder.CreateCall(funcSymbol->function, args, funcName);
            llvm::AllocaInst *alloca = builder.CreateAlloca(funcSymbol->returnType, nullptr, "calltmp");
            builder.CreateStore(result, alloca);
            lastValue = alloca;
        }
        else
        {
            result = builder.CreateCall(funcSymbol->function, args);
            lastValue = nullptr;
        }
    }

    void CodeGenVisitor::visitMethodCall(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }
    void CodeGenVisitor::visitUnaryExpr(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }
    void CodeGenVisitor::visitArrayLiteral(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }
    void CodeGenVisitor::visitIndexExpr(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }
    void CodeGenVisitor::visitTypeofExpr(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }
} // namespace Cryo
