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

    llvm::Value *Initializer::generateUnaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating unary expression...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_UNARY_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a unary expression");
            return nullptr;
        }

        CryoUnaryOpNode *unaryOp = node->data.unary_op;
        if (!unaryOp)
        {
            logMessage(LMI, "ERROR", "Initializer", "Unary op is null");
            CONDITION_FAILED;
        }

        // Get the operand
        ASTNode *operandNode = unaryOp->expression;
        llvm::Value *operand = getInitializerValue(operandNode);
        if (!operand)
        {
            logMessage(LMI, "ERROR", "Initializer", "Failed to generate operand");
            CONDITION_FAILED;
        }

        // Get result type
        DataType *resultType = unaryOp->resultType;
        if (!resultType)
        {
            logMessage(LMI, "ERROR", "Initializer", "Result type is null");
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Initializer", "Unary operation: %d", unaryOp->op);

        // Dispatch to appropriate handler based on operator type
        switch (unaryOp->op)
        {
        case TOKEN_ADDRESS_OF:
            return handleAddressOf(operand);
        case TOKEN_DEREFERENCE:
            return handleDereference(operand, operandNode);
        case TOKEN_MINUS:
            return handleUnaryMinus(operand, operandNode);
        case TOKEN_BANG:
            return handleLogicalNot(operand, operandNode);
        case TOKEN_INCREMENT:
            return handleIncrement(operand, operandNode);
        case TOKEN_DECREMENT:
            return handleDecrement(operand, operandNode);
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unsupported unary operator: %d", unaryOp->op);
            return nullptr;
        }
    }

    llvm::Value *Initializer::handleAddressOf(llvm::Value *operand)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating address-of expression");

        // Check if operand is already an appropriate type
        if (llvm::isa<llvm::AllocaInst>(operand) ||
            llvm::isa<llvm::GetElementPtrInst>(operand) ||
            operand->getType()->isPointerTy())
        {
            if (llvm::isa<llvm::LoadInst>(operand))
            {
                // If it's a load instruction, get the pointer operand
                llvm::LoadInst *loadInst = llvm::cast<llvm::LoadInst>(operand);
                operand = loadInst->getPointerOperand();
            }
            else if (llvm::isa<llvm::GetElementPtrInst>(operand))
            {
                // If it's a GEP instruction, return it as is
                return operand;
            }
            return operand;
        }

        // If all else fails, allocate and store the value
        llvm::AllocaInst *allocaInst = context.getInstance().builder.CreateAlloca(
            operand->getType(), nullptr, "addr_temp");
        context.getInstance().builder.CreateStore(operand, allocaInst);
        return allocaInst;
    }

    llvm::Value *Initializer::handleDereference(llvm::Value *operand, ASTNode *operandNode)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating dereference expression");

        // Operand must be a pointer type
        if (!operand->getType()->isPointerTy())
        {
            logMessage(LMI, "ERROR", "Initializer", "Dereference operand is not a pointer");
            return nullptr;
        }

        // Determine the type to load
        llvm::Type *pointeeType = determinePointeeType(operand, operandNode);

        // Load the value from the pointer
        return context.getInstance().builder.CreateLoad(pointeeType, operand, "deref");
    }

    llvm::Type *Initializer::determinePointeeType(llvm::Value *operand, ASTNode *operandNode)
    {
        // Check for common pointer types
        if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(operand))
        {
            return allocaInst->getAllocatedType();
        }
        else if (llvm::GetElementPtrInst *gepInst = llvm::dyn_cast<llvm::GetElementPtrInst>(operand))
        {
            return gepInst->getResultElementType();
        }
        else if (llvm::LoadInst *loadInst = llvm::dyn_cast<llvm::LoadInst>(operand))
        {
            return loadInst->getPointerOperandType();
        }

        // Use AST type information as fallback
        llvm::Type *pointeeType = context.getInstance().symbolTable->getLLVMType(
            DTM->astInterface->getTypeofASTNode(operandNode));

        if (!pointeeType)
        {
            logMessage(LMI, "WARNING", "Initializer", "Could not determine pointee type, using i8");
            pointeeType = context.getInstance().builder.getInt8Ty();
        }

        return pointeeType;
    }

    llvm::Value *Initializer::loadPointerOperandIfNeeded(llvm::Value *operand, ASTNode *operandNode)
    {
        if (!operand->getType()->isPointerTy())
        {
            return operand;
        }

        llvm::Type *pointeeType = determinePointeeType(operand, operandNode);
        return context.getInstance().builder.CreateLoad(pointeeType, operand, "load_for_op");
    }

    llvm::Value *Initializer::handleUnaryMinus(llvm::Value *operand, ASTNode *operandNode)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating unary minus expression");

        // Load from pointer if needed
        operand = loadPointerOperandIfNeeded(operand, operandNode);

        // Handle different types
        if (operand->getType()->isIntegerTy())
        {
            return context.getInstance().builder.CreateNeg(operand, "neg");
        }
        else if (operand->getType()->isFloatingPointTy())
        {
            return context.getInstance().builder.CreateFNeg(operand, "fneg");
        }

        logMessage(LMI, "ERROR", "Initializer", "Unary minus not supported for this type");
        return nullptr;
    }

    llvm::Value *Initializer::handleLogicalNot(llvm::Value *operand, ASTNode *operandNode)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating logical not expression");

        // Load from pointer if needed
        operand = loadPointerOperandIfNeeded(operand, operandNode);

        // Convert to boolean
        llvm::Value *boolValue = convertToBool(operand);
        if (!boolValue)
        {
            return nullptr;
        }

        // Negate the boolean value
        return context.getInstance().builder.CreateXor(
            boolValue,
            llvm::ConstantInt::get(context.getInstance().builder.getInt1Ty(), 1),
            "lnot");
    }

    llvm::Value *Initializer::convertToBool(llvm::Value *value)
    {
        if (value->getType()->isIntegerTy(1))
        {
            // Already a boolean
            return value;
        }
        else if (value->getType()->isIntegerTy())
        {
            // Compare with zero for integers
            return context.getInstance().builder.CreateICmpNE(
                value,
                llvm::ConstantInt::get(value->getType(), 0),
                "tobool");
        }
        else if (value->getType()->isFloatingPointTy())
        {
            // Compare with zero for floating point
            return context.getInstance().builder.CreateFCmpONE(
                value,
                llvm::ConstantFP::get(value->getType(), 0.0),
                "tobool");
        }
        else if (value->getType()->isPointerTy())
        {
            // Compare with null for pointers
            return context.getInstance().builder.CreateICmpNE(
                value,
                llvm::ConstantPointerNull::get(
                    llvm::cast<llvm::PointerType>(value->getType())),
                "tobool");
        }

        logMessage(LMI, "ERROR", "Initializer", "Type cannot be converted to boolean");
        return nullptr;
    }

    llvm::Value *Initializer::handleIncrement(llvm::Value *operand, ASTNode *operandNode)
    {
        return handleIncrementOrDecrement(operand, operandNode, true);
    }

    llvm::Value *Initializer::handleDecrement(llvm::Value *operand, ASTNode *operandNode)
    {
        return handleIncrementOrDecrement(operand, operandNode, false);
    }

    llvm::Value *Initializer::handleIncrementOrDecrement(llvm::Value *operand, ASTNode *operandNode, bool isIncrement)
    {
        const char *opName = isIncrement ? "increment" : "decrement";
        logMessage(LMI, "INFO", "Initializer", "Generating %s expression", opName);

        // Ensure operand is a pointer to a value
        if (!operand->getType()->isPointerTy())
        {
            logMessage(LMI, "ERROR", "Initializer", "%s operand must be an lvalue", opName);
            CONDITION_FAILED;
        }

        // Determine the type
        llvm::Type *pointeeType = determinePointeeType(operand, operandNode);

        // Load the current value
        llvm::Value *currentVal = context.getInstance().builder.CreateLoad(
            pointeeType, operand, isIncrement ? "inc.old" : "dec.old");

        // Modify the value
        llvm::Value *result = nullptr;
        if (currentVal->getType()->isIntegerTy())
        {
            result = isIncrement
                         ? context.getInstance().builder.CreateAdd(
                               currentVal,
                               llvm::ConstantInt::get(currentVal->getType(), 1),
                               isIncrement ? "inc.new" : "dec.new")
                         : context.getInstance().builder.CreateSub(
                               currentVal,
                               llvm::ConstantInt::get(currentVal->getType(), 1),
                               isIncrement ? "inc.new" : "dec.new");
        }
        else if (currentVal->getType()->isFloatingPointTy())
        {
            result = isIncrement
                         ? context.getInstance().builder.CreateFAdd(
                               currentVal,
                               llvm::ConstantFP::get(currentVal->getType(), 1.0),
                               isIncrement ? "inc.new" : "dec.new")
                         : context.getInstance().builder.CreateFSub(
                               currentVal,
                               llvm::ConstantFP::get(currentVal->getType(), 1.0),
                               isIncrement ? "inc.new" : "dec.new");
        }
        else
        {
            logMessage(LMI, "ERROR", "Initializer", "%s not supported for this type", opName);
            CONDITION_FAILED;
        }

        // Store the modified value
        context.getInstance().builder.CreateStore(result, operand);

        // Return the new value
        return result;
    }

} // namespace Cryo