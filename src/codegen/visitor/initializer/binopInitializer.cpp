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

        // First generate the left and right operands
        llvm::Value *lhs = getInitializerValue(node->data.bin_op->left);
        llvm::Value *rhs = getInitializerValue(node->data.bin_op->right);
        if (!lhs || !rhs)
        {
            logMessage(LMI, "ERROR", "Initializer", "Left or right operand is null");
            return nullptr;
        }

        // Special handling for pointer null comparisons
        if ((opType == OPERATOR_EQ || opType == OPERATOR_NEQ) &&
            (lhs->getType()->isPointerTy() || rhs->getType()->isPointerTy()))
        {
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
                    logMessage(LMI, "INFO", "Initializer", "Loading pointer value for null comparison");
                    ptrValue = context.getInstance().builder.CreateLoad(
                        allocaInst->getAllocatedType(),
                        ptrValue,
                        "ptr_value");
                }

                // Create null pointer constant of the appropriate type
                llvm::PointerType *ptrType = nullptr;
                if (ptrValue->getType()->isPointerTy())
                {
                    ptrType = llvm::cast<llvm::PointerType>(ptrValue->getType());
                }
                else
                {
                    // Fallback to generic pointer
                    ptrType = llvm::PointerType::get(context.getInstance().context, 0);
                }

                llvm::Value *nullPtr = llvm::ConstantPointerNull::get(ptrType);

                // Create the comparison
                if (opType == OPERATOR_EQ)
                {
                    return context.getInstance().builder.CreateICmpEQ(ptrValue, nullPtr, "ptr_eq_null");
                }
                else
                {
                    return context.getInstance().builder.CreateICmpNE(ptrValue, nullPtr, "ptr_ne_null");
                }
            }
        }

        // Handle standard operators
        logMessage(LMI, "INFO", "Initializer", "Binary operator: %s", CryoOperatorTypeToString(opType));
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
        case OPERATOR_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
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
