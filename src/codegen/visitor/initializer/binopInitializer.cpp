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
    // Helper to load a value if it's a pointer
    llvm::Value *Initializer::loadIfPointerToValue(llvm::Value *val, const char *name)
    {
        if (val->getType()->isPointerTy())
        {
            // If it's a pointer to a non-pointer type
            if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(val))
            {
                llvm::Type *allocaType = allocaInst->getAllocatedType();
                if (!allocaType->isPointerTy())
                {
                    // Load the value from the pointer
                    logMessage(LMI, "INFO", "Initializer", "Loading pointer value: %s", name);
                    val = context.getInstance().builder.CreateLoad(allocaType, val, name);
                }
            }
            else
            {
                // It's not an AllocaInst, just load it
                logMessage(LMI, "INFO", "Initializer", "Loading pointer value: %s", name);
                val = context.getInstance().builder.CreateLoad(val->getType(), val, name);
            }
        }
        return val;
    }

    // Handle pointer arithmetic operations
    std::pair<llvm::Value *, llvm::Value *> Initializer::handlePointerArithmetic(
        llvm::Value *left, llvm::Value *right, CryoOperatorType opType, const char *opName)
    {
        if (left->getType()->isPointerTy() && right->getType()->isPointerTy())
        {
            // Can't add/subtract/etc. two pointers directly
            logMessage(LMI, "ERROR", "Initializer", "Cannot perform %s on two pointers", opName);
            return {nullptr, nullptr};
        }

        // If the right operand is a pointer, swap them
        // (pointer arithmetic typically has the pointer on the left)
        if (!left->getType()->isPointerTy() && right->getType()->isPointerTy())
        {
            if (opType == OPERATOR_ADD)
            {
                // a + ptr is the same as ptr + a
                std::swap(left, right);
            }
            else
            {
                // For other operations, can't have pointer on right
                logMessage(LMI, "ERROR", "Initializer", "Cannot perform %s with pointer as right operand", opName);
                return {nullptr, nullptr};
            }
        }

        // Left is a pointer but right is not, ensure right is an integer
        if (left->getType()->isPointerTy() && !right->getType()->isPointerTy())
        {
            if (!right->getType()->isIntegerTy())
            {
                // Convert to integer
                if (right->getType()->isFloatingPointTy())
                {
                    right = context.getInstance().builder.CreateFPToSI(
                        right,
                        llvm::Type::getInt32Ty(context.getInstance().context),
                        "float_to_int");
                }
                else
                {
                    logMessage(LMI, "ERROR", "Initializer", "Right operand must be an integer for pointer arithmetic");
                    return {nullptr, nullptr};
                }
            }
        }

        return {left, right};
    }

    // Handle null pointer comparisons
    llvm::Value *Initializer::handleNullComparison(llvm::Value *lhs, llvm::Value *rhs,
                                                   CryoOperatorType opType, ASTNode *node)
    {
        if ((opType != OPERATOR_EQ && opType != OPERATOR_NEQ) ||
            (!lhs->getType()->isPointerTy() && !rhs->getType()->isPointerTy()))
        {
            return nullptr; // Not a null comparison
        }

        DataType *leftType = DTM->astInterface->getTypeofASTNode(node->data.bin_op->left);
        DataType *rightType = DTM->astInterface->getTypeofASTNode(node->data.bin_op->right);

        bool isNullComparison = false;
        bool isLeftNull = false;
        bool isRightNull = false;

        // Check if one side is null
        if (rightType && (rightType->container->primitive == PRIM_NULL ||
                          strcmp(rightType->typeName, "null") == 0))
        {
            isNullComparison = true;
            isRightNull = true;
        }
        else if (leftType && (leftType->container->primitive == PRIM_NULL ||
                              strcmp(leftType->typeName, "null") == 0))
        {
            isNullComparison = true;
            isLeftNull = true;
        }

        if (isNullComparison)
        {
            // Determine which is the pointer
            llvm::Value *ptrValue = isLeftNull ? rhs : lhs;

            if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(ptrValue))
            {
                llvm::Type *allocaType = allocaInst->getAllocatedType();
                if (!allocaType->isPointerTy())
                {
                    logMessage(LMI, "INFO", "Initializer", "Loading pointer value for null comparison");
                    // It's not a pointer, get the pointer
                    ptrValue = context.getInstance().builder.CreateLoad(
                        allocaType->getPointerTo(),
                        ptrValue,
                        "ptr_value");
                }
                else
                {
                    logMessage(LMI, "INFO", "Initializer", "Loading pointer value for null comparison");
                    ptrValue = context.getInstance().builder.CreateLoad(
                        allocaInst->getAllocatedType(),
                        ptrValue,
                        "ptr_value");
                }
            }

            // Create null pointer constant of the appropriate type
            llvm::PointerType *ptrType = nullptr;
            if (ptrValue->getType()->isPointerTy())
            {
                logMessage(LMI, "INFO", "Initializer", "Pointer type detected for null comparison");
                ptrType = llvm::cast<llvm::PointerType>(ptrValue->getType());
            }
            else
            {
                // Fallback to generic pointer
                logMessage(LMI, "INFO", "Initializer", "Generic pointer type detected for null comparison");
                ptrType = llvm::PointerType::get(context.getInstance().context, 0);
            }

            llvm::Value *nullPtr = llvm::ConstantPointerNull::get(ptrType);

            // Perform the comparison
            if (opType == OPERATOR_EQ)
            {
                return context.getInstance().builder.CreateICmpEQ(ptrValue, nullPtr, "ptr_eq_null");
            }
            else
            {
                return context.getInstance().builder.CreateICmpNE(ptrValue, nullPtr, "ptr_ne_null");
            }
        }

        return nullptr; // Not a null comparison
    }

    // Handle arithmetic operations
    llvm::Value *Initializer::handleArithmeticOp(llvm::Value *lhs, llvm::Value *rhs, CryoOperatorType opType)
    {
        lhs = loadIfPointerToValue(lhs, "lhs_arith_load");
        rhs = loadIfPointerToValue(rhs, "rhs_arith_load");

        switch (opType)
        {
        case OPERATOR_ADD:
            return context.getInstance().builder.CreateAdd(lhs, rhs, "add");
        case OPERATOR_SUB:
            return context.getInstance().builder.CreateSub(lhs, rhs, "sub");
        case OPERATOR_MUL:
            return context.getInstance().builder.CreateMul(lhs, rhs, "mul");
        case OPERATOR_DIV:
            return context.getInstance().builder.CreateSDiv(lhs, rhs, "div");
        case OPERATOR_MOD:
            return context.getInstance().builder.CreateSRem(lhs, rhs, "mod");
        default:
            return nullptr;
        }
    }

    // Handle bitwise operations
    llvm::Value *Initializer::handleBitwiseOp(llvm::Value *lhs, llvm::Value *rhs, CryoOperatorType opType)
    {
        lhs = loadIfPointerToValue(lhs, "lhs_bitwise_load");
        rhs = loadIfPointerToValue(rhs, "rhs_bitwise_load");

        switch (opType)
        {
        case OPERATOR_AND:
            return context.getInstance().builder.CreateAnd(lhs, rhs, "and");
        case OPERATOR_OR:
            return context.getInstance().builder.CreateOr(lhs, rhs, "or");
        case OPERATOR_XOR:
            return context.getInstance().builder.CreateXor(lhs, rhs, "xor");
        case OPERATOR_LSHIFT:
            return context.getInstance().builder.CreateShl(lhs, rhs, "lshift");
        case OPERATOR_RSHIFT:
            return context.getInstance().builder.CreateLShr(lhs, rhs, "rshift");
        default:
            return nullptr;
        }
    }

    // Handle comparison operations
    llvm::Value *Initializer::handleComparisonOp(llvm::Value *lhs, llvm::Value *rhs, CryoOperatorType opType)
    {
        lhs = loadIfPointerToValue(lhs, "lhs_comp_load");
        rhs = loadIfPointerToValue(rhs, "rhs_comp_load");

        switch (opType)
        {
        case OPERATOR_LT:
            return context.getInstance().builder.CreateICmpSLT(lhs, rhs, "lt");
        case OPERATOR_GT:
            return context.getInstance().builder.CreateICmpSGT(lhs, rhs, "gt");
        case OPERATOR_LTE:
            return context.getInstance().builder.CreateICmpSLE(lhs, rhs, "lte");
        case OPERATOR_GTE:
            return context.getInstance().builder.CreateICmpSGE(lhs, rhs, "gte");
        case OPERATOR_EQ:
            return context.getInstance().builder.CreateICmpEQ(lhs, rhs, "eq");
        case OPERATOR_NEQ:
            return context.getInstance().builder.CreateICmpNE(lhs, rhs, "neq");
        default:
            return nullptr;
        }
    }

    // Handle pointer arithmetic specifically
    llvm::Value *Initializer::handlePointerArithmeticOp(llvm::Value *left, llvm::Value *right,
                                                        CryoOperatorType opType)
    {
        llvm::Type *elementType;
        if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(left))
        {
            elementType = allocaInst->getAllocatedType();
        }
        else
        {
            elementType = left->getType();
        }

        if (opType == OPERATOR_ADD)
        {
            // Create a GEP instruction for addition
            return context.getInstance().builder.CreateGEP(
                elementType,
                left,
                right,
                "ptr_add");
        }
        else if (opType == OPERATOR_SUB)
        {
            // Negate the index for subtraction
            llvm::Value *negIndex = context.getInstance().builder.CreateNeg(right, "neg_idx");

            // Create a GEP instruction for subtraction
            return context.getInstance().builder.CreateGEP(
                elementType,
                left,
                negIndex,
                "ptr_sub");
        }

        return nullptr;
    }

    // Main binary expression generator
    llvm::Value *Initializer::generateBinaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating binary expression...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_BINARY_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a binary expression");
            return nullptr;
        }

        CryoOperatorType opType = node->data.bin_op->op;

        // Generate the left and right operands
        llvm::Value *lhs = getInitializerValue(node->data.bin_op->left);
        llvm::Value *rhs = getInitializerValue(node->data.bin_op->right);
        if (!lhs || !rhs)
        {
            logMessage(LMI, "ERROR", "Initializer", "Left or right operand is null");
            return nullptr;
        }

        // Special handling for pointer null comparisons
        llvm::Value *nullCompResult = handleNullComparison(lhs, rhs, opType, node);
        if (nullCompResult)
        {
            return nullCompResult;
        }

        // Log the operator
        logMessage(LMI, "INFO", "Initializer", "Binary operator: %s", CryoOperatorTypeToString(opType));

        // Process based on operator category
        switch (opType)
        {
        // Arithmetic operations
        case OPERATOR_ADD:
        case OPERATOR_SUB:
        {
            // First check for pointer arithmetic
            auto [left, right] = handlePointerArithmetic(lhs, rhs, opType, CryoOperatorTypeToString(opType));
            if (!left || !right)
                return nullptr;

            // Handle pointer arithmetic specially
            if (left->getType()->isPointerTy())
            {
                return handlePointerArithmeticOp(left, right, opType);
            }
            else
            {
                // Regular arithmetic
                return handleArithmeticOp(left, right, opType);
            }
        }

        case OPERATOR_MUL:
        case OPERATOR_DIV:
        case OPERATOR_MOD:
            return handleArithmeticOp(lhs, rhs, opType);

        // Bitwise operations
        case OPERATOR_AND:
        case OPERATOR_OR:
        case OPERATOR_XOR:
        case OPERATOR_LSHIFT:
        case OPERATOR_RSHIFT:
            return handleBitwiseOp(lhs, rhs, opType);

        // Comparison operations
        case OPERATOR_LT:
        case OPERATOR_GT:
        case OPERATOR_LTE:
        case OPERATOR_GTE:
        case OPERATOR_EQ:
        case OPERATOR_NEQ:
            return handleComparisonOp(lhs, rhs, opType);

        // Assignment operators not supported in expressions
        case OPERATOR_ASSIGN:
        case OPERATOR_ADD_ASSIGN:
        case OPERATOR_SUB_ASSIGN:
        case OPERATOR_MUL_ASSIGN:
        case OPERATOR_DIV_ASSIGN:
        case OPERATOR_MOD_ASSIGN:
        case OPERATOR_AND_ASSIGN:
        case OPERATOR_OR_ASSIGN:
        case OPERATOR_XOR_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;

        // Increment/decrement not supported in expressions
        case OPERATOR_INCREMENT:
        case OPERATOR_DECREMENT:
            logMessage(LMI, "ERROR", "Initializer", "Increment/decrement operator is not supported in expressions");
            return nullptr;

        case OPERATOR_NA:
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled operator type: %s", CryoOperatorTypeToString(opType));
            return nullptr;
        }
    }

} // namespace Cryo