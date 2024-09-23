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
#include "cpp/codegen.h"

namespace Cryo
{

    void BinaryExpressions::handleBinaryExpression(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "BinExp", "Handling Binary Expression");

        DEBUG_BREAKPOINT;
    }

    /// @brief
    /// This function should replace the `createBinaryExpression` function.
    /// It should be able to handle multiple expressions within the binary expression.
    llvm::Value *BinaryExpressions::handleComplexBinOp(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "BinExp", "Handling Complex Binary Operation");

        if (node->metaData->type != NODE_BINARY_EXPR)
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Not a binary expression");
            // If it's not a binary expression, just return the value
            return compiler.getGenerator().getInitilizerValue(node);
        }

        llvm::Value *result = nullptr;
        ASTNode *currentNode = node;

        compiler.getContext().builder.SetInsertPoint(compiler.getContext().builder.GetInsertBlock());

        while (currentNode->metaData->type == NODE_BINARY_EXPR)
        {
            ASTNode *leftNode = currentNode->data.bin_op->left;
            ASTNode *rightNode = currentNode->data.bin_op->right;

            llvm::Value *leftValue;
            if (leftNode->metaData->type == NODE_BINARY_EXPR)
            {
                // If left node is a binary expression, recursively handle it
                debugger.logMessage("INFO", __LINE__, "BinExp", "Handling left binary expression");
                leftValue = handleComplexBinOp(leftNode);
            }
            else
            {
                // Otherwise, get the value
                debugger.logMessage("INFO", __LINE__, "BinExp", "Getting left value");
                leftValue = compiler.getGenerator().getInitilizerValue(leftNode);
            }

            llvm::Value *rightValue = compiler.getGenerator().getInitilizerValue(rightNode);

            if (!result)
            {
                // First iteration
                debugger.logMessage("INFO", __LINE__, "BinExp", "Creating binary expression");
                result = createBinaryExpression(currentNode, leftValue, rightValue);
            }
            else
            {
                // Subsequent iterations
                debugger.logMessage("INFO", __LINE__, "BinExp", "Creating binary expression");
                result = createBinaryExpression(currentNode, result, rightValue);
            }

            if (!result)
            {
                debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to create binary expression");
                return nullptr;
            }

            // Move to the next node (if any)
            if (rightNode->metaData->type == NODE_BINARY_EXPR)
            {
                debugger.logMessage("INFO", __LINE__, "BinExp", "Moving to next binary expression");
                currentNode = rightNode;
            }
            else
            {
                break;
            }
        }

        assert(result != nullptr);

        debugger.logMessage("INFO", __LINE__, "BinExp", "Binary expression handled successfully");
        return result;
    }

    llvm::Value *BinaryExpressions::createBinaryExpression(ASTNode *node, llvm::Value *leftValue, llvm::Value *rightValue)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Binary Expression");

        if (!leftValue || !rightValue)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Invalid operands for binary expression");
            return nullptr;
        }

        CryoOperatorType operatorType = node->data.bin_op->op;
        switch (operatorType)
        {
        case OPERATOR_ADD:
            return compiler.getContext().builder.CreateAdd(leftValue, rightValue, "addtmp");
        case OPERATOR_SUB:
            return compiler.getContext().builder.CreateSub(leftValue, rightValue, "subtmp");
        case OPERATOR_MUL:
            return compiler.getContext().builder.CreateMul(leftValue, rightValue, "multmp");
        case OPERATOR_DIV:
            return compiler.getContext().builder.CreateSDiv(leftValue, rightValue, "divtmp");
        case OPERATOR_MOD:
            return compiler.getContext().builder.CreateSRem(leftValue, rightValue, "modtmp");
        case OPERATOR_AND:
            return compiler.getContext().builder.CreateAnd(leftValue, rightValue, "andtmp");
        case OPERATOR_OR:
            return compiler.getContext().builder.CreateOr(leftValue, rightValue, "ortmp");
        case OPERATOR_LT:
            return compiler.getContext().builder.CreateICmpSLT(leftValue, rightValue, "ltcmp");
        case OPERATOR_LTE:
            return compiler.getContext().builder.CreateICmpSLE(leftValue, rightValue, "ltecmp");
        case OPERATOR_GT:
            return compiler.getContext().builder.CreateICmpSGT(leftValue, rightValue, "gtcmp");
        case OPERATOR_GTE:
            return compiler.getContext().builder.CreateICmpSGE(leftValue, rightValue, "gtecmp");
        case OPERATOR_EQ:
            return compiler.getContext().builder.CreateICmpEQ(leftValue, rightValue, "eqcmp");
        case OPERATOR_NEQ:
            return compiler.getContext().builder.CreateICmpNE(leftValue, rightValue, "neqcmp");
        default:
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Unknown operator type");
            return nullptr;
        }
    }

    llvm::Value *BinaryExpressions::createComparisonExpression(ASTNode *left, ASTNode *right, CryoOperatorType op)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        llvm::IRBuilder<> &builder = compiler.getContext().builder;
        debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Comparison Expression");

        llvm::Value *leftValue = compiler.getGenerator().getInitilizerValue(left);
        llvm::Value *rightValue = compiler.getGenerator().getInitilizerValue(right);

        if (!leftValue || !rightValue)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to generate values for comparison expression");
            return nullptr;
        }

        // // Load value if right is a pointer (a variable)
        if (rightValue->getType()->isPointerTy())
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Loading right operand");
            rightValue = builder.CreateLoad(rightValue->getType()->getPointerTo(), rightValue, "rightLoad");
        }
        if (leftValue->getType()->isPointerTy() && rightValue->getType()->isPointerTy())
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Loading left operand");
            leftValue = builder.CreateLoad(leftValue->getType()->getPointerTo(), leftValue, "leftLoad");
        }

        // Ensure both operands have the same type
        if (leftValue->getType() != rightValue->getType())
        {
            debugger.logMessage("WARNING", __LINE__, "BinExp", "Operand types don't match, attempting to cast");

            // If left is a pointer and right is an integer, we need to load the value from the pointer
            if (leftValue->getType()->isPointerTy() && rightValue->getType()->isIntegerTy())
            {
                debugger.logMessage("INFO", __LINE__, "BinExp", "Casting left operand to match right operand");
                leftValue = compiler.getContext().builder.CreateLoad(rightValue->getType(), leftValue, "leftLoad");
            }
            // If left is an integer and right is a pointer, we need to load the value from the pointer
            else if (leftValue->getType()->isIntegerTy() && rightValue->getType()->isPointerTy())
            {
                debugger.logMessage("INFO", __LINE__, "BinExp", "Casting right operand to match left operand");
                rightValue = compiler.getContext().builder.CreateLoad(leftValue->getType(), rightValue, "rightLoad");
            }
            // If both are integers but of different sizes, cast to the larger size
            else if (leftValue->getType()->isIntegerTy() && rightValue->getType()->isIntegerTy())
            {
                if (leftValue->getType()->getIntegerBitWidth() < rightValue->getType()->getIntegerBitWidth())
                {
                    debugger.logMessage("INFO", __LINE__, "BinExp", "Casting left operand to match right operand");
                    leftValue = compiler.getContext().builder.CreateIntCast(leftValue, rightValue->getType(), true, "leftCast");
                }
                else
                {
                    debugger.logMessage("INFO", __LINE__, "BinExp", "Casting right operand to match left operand");
                    rightValue = compiler.getContext().builder.CreateIntCast(rightValue, leftValue->getType(), true, "rightCast");
                }
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "BinExp", "Unable to cast operands to matching types");
                return nullptr;
            }
        }

        llvm::CmpInst::Predicate predicate;
        switch (op)
        {
        case OPERATOR_EQ:
            predicate = llvm::CmpInst::ICMP_EQ;
            break;
        case OPERATOR_NEQ:
            predicate = llvm::CmpInst::ICMP_NE;
            break;
        case OPERATOR_LT:
            predicate = llvm::CmpInst::ICMP_SLT;
            break;
        case OPERATOR_GT:
            predicate = llvm::CmpInst::ICMP_SGT;
            break;
        case OPERATOR_LTE:
            predicate = llvm::CmpInst::ICMP_SLE;
            break;
        case OPERATOR_GTE:
            predicate = llvm::CmpInst::ICMP_SGE;
            break;
        default:
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Unknown comparison operator");
            return nullptr;
        }

        return compiler.getContext().builder.CreateICmp(predicate, leftValue, rightValue, "compareResult");
    }

} // namespace Cryo
