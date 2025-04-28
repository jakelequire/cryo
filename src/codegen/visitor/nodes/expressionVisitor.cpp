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
            // Get argument AST node and its type
            ASTNode *argNode = call->args[i];
            DataType *argType = DTM->astInterface->getTypeofASTNode(argNode);

            if (!argType)
            {
                logMessage(LMI, "ERROR", "CodeGenVisitor",
                           "Could not determine type of argument %d", i);
                continue;
            }

            // Get expected parameter type if available
            DataType *paramType = nullptr;
            if (funcSymbol->functionDataType &&
                funcSymbol->functionDataType->container->typeOf == FUNCTION_TYPE &&
                i < funcSymbol->functionDataType->container->type.functionType->paramCount)
            {

                paramType = funcSymbol->functionDataType->container->type.functionType->paramTypes[i];
            }

            // Get the LLVM value for the argument
            llvm::Value *argValue = getLLVMValue(argNode);
            if (!argValue)
            {
                logMessage(LMI, "ERROR", "CodeGenVisitor", "Argument %d value is null", i);
                continue;
            }

            if (argType->container->primitive == PRIM_STR)
            {
                // Load the string
                llvm::Value *strValue = builder.CreateLoad(argValue->getType(), argValue, "str.load");
                argValue = builder.CreateBitCast(strValue, builder.getInt8Ty()->getPointerTo(), "str.cast");
                logMessage(LMI, "INFO", "CodeGenVisitor", "Loaded string argument %d: %s",
                           i, argValue->getName().str().c_str());
            }
            if (argValue->getType()->isPointerTy() &&
                argType->container->primitive != PRIM_STR)
            {
                // Load the value if it's a pointer
                argValue = builder.CreateLoad(argValue->getType(), argValue, "arg.load");
            }

            // If we have type information for both argument and parameter, perform conversion
            if (argType && paramType)
            {
                logMessage(LMI, "INFO", "CodeGenVisitor",
                           "Converting argument %d from %s to %s",
                           i, argType->typeName, paramType->typeName);

                argValue = convertValueToTargetType(argValue, argType, paramType);
            }
            // Fallback to LLVM type-based conversion using parameter types from function type
            else if (i < funcSymbol->function->getFunctionType()->getNumParams())
            {
                llvm::Type *llvmParamType = funcSymbol->function->getFunctionType()->getParamType(i);

                if (argValue->getType() != llvmParamType)
                {
                    // Handle basic conversions based on LLVM types
                    if (argValue->getType()->isPointerTy() && !llvmParamType->isPointerTy())
                    {
                        // Need to load from pointer
                        if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(argValue))
                        {
                            llvm::Type *allocatedType = allocaInst->getAllocatedType();
                            argValue = builder.CreateLoad(allocatedType, argValue, "arg.load");
                        }
                        else
                        {
                            argValue = builder.CreateLoad(llvmParamType, argValue, "arg.load");
                        }
                    }
                }
            }

            args.push_back(argValue);
        }

        // Create the function call
        std::string funcName = call->name;
        logMessage(LMI, "INFO", "CodeGenVisitor", "Function call: %s", funcName.c_str());
        llvm::Value *result = nullptr;

        // Get the return type data
        DataType *returnDataType = nullptr;
        if (funcSymbol->functionDataType &&
            funcSymbol->functionDataType->container->typeOf == FUNCTION_TYPE)
        {
            returnDataType = funcSymbol->functionDataType->container->type.functionType->returnType;
        }

        if (funcSymbol->returnType->getTypeID() != llvm::Type::VoidTyID)
        {
            // Create the call instruction
            result = builder.CreateCall(funcSymbol->function, args, funcName + ".call");

            // If we have a return data type, process it appropriately
            if (returnDataType)
            {
                // Special handling for str to String if needed
                DataType *resultType = DTM->astInterface->getTypeofASTNode(node);
                if (resultType && resultType->container->typeOf == PRIM_STR)
                {
                    // Convert to string type if needed
                    llvm::Value *strValue = builder.CreateBitCast(result, builder.getInt8Ty(), "str.cast");
                    result = builder.CreateCall(funcSymbol->function, strValue, funcName + ".str.call");
                }
                else if (resultType && resultType->container->typeOf == PRIM_STRING)
                {
                    // Convert to String type if needed
                    llvm::Value *stringValue = builder.CreateBitCast(result, builder.getInt8Ty()->getPointerTo(), "string.cast");
                    result = builder.CreateCall(funcSymbol->function, stringValue, funcName + ".string.call");
                }
                else if (resultType && resultType->container->typeOf == PRIM_INT)
                {
                    // Convert to int type if needed
                    result = performTypeCast(result, builder.getInt32Ty());
                }
                else if (resultType && resultType->container->typeOf == PRIM_FLOAT)
                {
                    // Convert to float type if needed
                    result = performTypeCast(result, builder.getDoubleTy());
                }
                else if (resultType && resultType->container->typeOf == PRIM_BOOLEAN)
                {
                    // Convert to boolean type if needed
                    result = performTypeCast(result, builder.getInt1Ty());
                }
            }

            // Store the result appropriately
            llvm::AllocaInst *returnAlloca = builder.CreateAlloca(
                result->getType(), nullptr, funcName + ".result");
            builder.CreateStore(result, returnAlloca);
            lastValue = returnAlloca;
        }
        else
        {
            // Void return type
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

    llvm::Value *CodeGenVisitor::performTypeCast(llvm::Value *value, llvm::Type *targetType)
    {
        if (!value || !targetType || value->getType() == targetType)
            return value;

        llvm::Type *sourceType = value->getType();

        // Integer -> Integer conversion
        if (sourceType->isIntegerTy() && targetType->isIntegerTy())
        {
            unsigned srcBits = sourceType->getIntegerBitWidth();
            unsigned dstBits = targetType->getIntegerBitWidth();

            if (srcBits < dstBits)
                return builder.CreateZExt(value, targetType, "zext");
            else if (srcBits > dstBits)
                return builder.CreateTrunc(value, targetType, "trunc");
        }

        // Float -> Float conversion
        if (sourceType->isFloatingPointTy() && targetType->isFloatingPointTy())
        {
            if (sourceType->getPrimitiveSizeInBits() < targetType->getPrimitiveSizeInBits())
                return builder.CreateFPExt(value, targetType, "fpext");
            else if (sourceType->getPrimitiveSizeInBits() > targetType->getPrimitiveSizeInBits())
                return builder.CreateFPTrunc(value, targetType, "fptrunc");
        }

        // Integer -> Float
        if (sourceType->isIntegerTy() && targetType->isFloatingPointTy())
            return builder.CreateSIToFP(value, targetType, "sitofp");

        // Float -> Integer
        if (sourceType->isFloatingPointTy() && targetType->isIntegerTy())
            return builder.CreateFPToSI(value, targetType, "fptosi");

        // Pointer conversions
        if (sourceType->isPointerTy() && targetType->isPointerTy())
            return builder.CreateBitCast(value, targetType, "ptrcast");

        // If no conversion was performed, return the original value
        logMessage(LMI, "WARNING", "CodeGenVisitor",
                   "Couldn't cast from %s to %s",
                   sourceType->getTypeID(), targetType->getTypeID());
        return value;
    }
} // namespace Cryo
