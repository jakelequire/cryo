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

        if (node->metaData->type != NODE_BINARY_EXPR)
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Node is not a binary expression");
            return;
        }

        llvm::Value *result = context.getInstance().initializer->generateBinaryExpr(node);
        if (result)
        {
            logMessage(LMI, "DEBUG", "CodeGenVisitor", "Binary expression result: %p", result);
            // Store the result properly so it can be retrieved
            lastValue = result;
        }
        else
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Failed to generate binary expression");
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

            // Get expected parameter type if available
            DataType *paramType = nullptr;
            if (funcSymbol->functionDataType &&
                funcSymbol->functionDataType->container->typeOf == FUNCTION_TYPE &&
                i < funcSymbol->functionDataType->container->type.functionType->paramCount)
            {

                paramType = funcSymbol->functionDataType->container->type.functionType->paramTypes[i];
            }

            std::cout << "--------------------------------------------------------\n";
            argType->debug->printVerbosType(argType);
            std::cout << "--------------------------------------------------------\n";
            if (paramType)
            {
                paramType->debug->printVerbosType(paramType);
            }
            else
            {
                logMessage(LMI, "ERROR", "CodeGenVisitor", "Parameter type is null");
            }
            std::cout << "--------------------------------------------------------\n";

            // Check if the argument type is a struct or object type
            bool isStructOrObjectType = false;
            if (argType && argType->container)
            {
                // Check if it's an object type (struct, class, etc.)
                if (argType->container->typeOf == OBJECT_TYPE)
                {
                    isStructOrObjectType = true;
                }
                // Check if it's the special String primitive
                else if (argType->container->primitive == PRIM_STRING)
                {
                    isStructOrObjectType = true;
                }
            }

            // Check if the parameter type expects a struct or object pointer
            bool paramExpectsStructPtr = true;
            if (paramType && paramType->container)
            {
                // Check if parameter expects an object type
                if (paramType->container->typeOf == OBJECT_TYPE)
                {
                    paramExpectsStructPtr = true;
                }
                // Check if parameter expects the special String primitive
                else if (paramType->container->primitive == PRIM_STRING)
                {
                    paramExpectsStructPtr = true;
                }
                else
                {
                    paramExpectsStructPtr = false;
                }
            }

            // Get the LLVM value for the argument
            llvm::Value *argValue = getLLVMValue(argNode);
            if (!argValue)
            {
                logMessage(LMI, "ERROR", "CodeGenVisitor", "Argument %d value is null", i);
                continue;
            }

            std::string argValName = argValue->getName().str();

            if (argValName.find("g_str") != std::string::npos)
            {
                // Allocate the string
                llvm::AllocaInst *strAlloc = context.getInstance().builder.CreateAlloca(
                    argValue->getType(), nullptr, argValName + ".alloc");
                context.getInstance().builder.CreateStore(argValue, strAlloc);
                argValue = context.getInstance().builder.CreateLoad(argValue->getType(), strAlloc, argValName + ".load");
                logMessage(LMI, "INFO", "Visitor", "Loaded string argument %d: %s", i, argValue->getName().str().c_str());
            }
            // Check if the argument is a global string
            else if (argType->container->primitive == PRIM_STR)
            {
                // Load the string
                std::string argValName = argValue->getName().str();
                argValue = builder.CreateLoad(argValue->getType(), argValue, argValName + ".load");
                logMessage(LMI, "INFO", "CodeGenVisitor",
                           "Loaded string argument %d: %s", i, argValue->getName().str().c_str());
            }

            if (argValue->getType()->isPointerTy() &&
                !(isStructOrObjectType && paramExpectsStructPtr) &&
                argType->container->primitive != PRIM_STR)
            {
                // Load the value if it's a pointers
                std::string argValName = argValue->getName().str();
                if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(argValue))
                {
                    argValue = context.getInstance().builder.CreateLoad(allocaInst->getAllocatedType(), argValue, argValName + ".load");
                }
                else
                {
                    logMessage(LMI, "ERROR", "CodeGenVisitor", "Argument %d is not an alloca", i);
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
        if (!node || !node->data.unary_op)
            return;

        llvm::Value *unaryExpr = context.getInstance().initializer->getInitializerValue(node);
        if (!unaryExpr)
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Unary expression value is null");
            CONDITION_FAILED;
            return;
        }

        return;
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
