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

            // The left value:
            llvm::Value *leftValue;
            CryoNodeType leftNodeType = leftNode->metaData->type;
            switch (leftNodeType)
            {
            case NODE_VAR_NAME:
            {
                debugger.logMessage("INFO", __LINE__, "BinExp", "Getting left value");
                std::string varName = leftNode->data.varName->varName;
                std::cout << "<!> (left) Variable Name: " << varName << std::endl;
                leftValue = compiler.getVariables().getVariable(varName);
                if (!leftValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to get left value");
                    CONDITION_FAILED;
                }
                if (leftValue->getType()->isPointerTy())
                {
                    debugger.logMessage("INFO", __LINE__, "BinExp", "Creating temporary value for pointer");
                    leftValue = createTempValueForPointer(leftValue, varName);
                    if (!leftValue)
                    {
                        debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to create temporary value for pointer");
                        CONDITION_FAILED;
                    }
                }
                else
                {
                    debugger.logMessage("INFO", __LINE__, "BinExp", "Left value is not a pointer");

                    llvm::Value *tempValue = compiler.getGenerator().getInitilizerValue(leftNode);
                }
                break;
            }
            default:
            {
                debugger.logMessage("INFO", __LINE__, "BinExp", "Getting left value");
                std::cout << "Unknown node type: " << CryoNodeTypeToString(leftNodeType) << std::endl;

                leftValue = compiler.getGenerator().getInitilizerValue(leftNode);
                if (!leftValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to get left value");
                    CONDITION_FAILED;
                }
            }
            }

            // The right value:
            llvm::Value *rightValue;
            CryoNodeType rightNodeType = rightNode->metaData->type;
            switch (rightNodeType)
            {
            case NODE_VAR_NAME:
            {
                debugger.logMessage("INFO", __LINE__, "BinExp", "Getting right value");
                std::string varName = rightNode->data.varName->varName;
                std::cout << "<!> (right) VarName: " << varName << std::endl;
                rightValue = compiler.getVariables().getVariable(varName);
                if (!rightValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to get right value");
                    CONDITION_FAILED;
                }
                if (rightValue->getType()->isPointerTy())
                {
                    debugger.logMessage("INFO", __LINE__, "BinExp", "Creating temporary value for pointer");
                    rightValue = createTempValueForPointer(rightValue, varName);
                    if (!rightValue)
                    {
                        debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to create temporary value for pointer");
                        CONDITION_FAILED;
                    }
                }
                break;
            }
            default:
            {
                debugger.logMessage("INFO", __LINE__, "BinExp", "Getting right value");
                std::cout << "Unknown node type: " << CryoNodeTypeToString(rightNodeType) << std::endl;

                rightValue = compiler.getGenerator().getInitilizerValue(rightNode);
                if (!rightValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to get right value");
                    CONDITION_FAILED;
                }
            }
            }

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
            // -----------------------
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
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Binary Expression");

        if (!leftValue || !rightValue)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Invalid operands for binary expression");
            CONDITION_FAILED;
        }

        CryoDataType leftDataType = node->data.bin_op->left->data.literal->dataType;
        CryoDataType rightDataType = node->data.bin_op->right->data.literal->dataType;
        llvm::Type *leftType = types.getType(leftDataType, 0);
        llvm::Type *rightType = types.getType(rightDataType, 0);
        bool sameType = leftType == rightType;

        if (!leftType || !rightType)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to get types for binary expression");
            CONDITION_FAILED;
        }

        llvm::Value *result = nullptr;
        CryoOperatorType operatorType = node->data.bin_op->op;
        switch (operatorType)
        {
        case OPERATOR_ADD:
            result = compiler.getContext().builder.CreateAdd(leftValue, rightValue, "addtmp");
            break;
        case OPERATOR_SUB:
            result = compiler.getContext().builder.CreateSub(leftValue, rightValue, "subtmp");
            break;
        case OPERATOR_MUL:
            result = compiler.getContext().builder.CreateMul(leftValue, rightValue, "multmp");
            break;
        case OPERATOR_DIV:
            result = compiler.getContext().builder.CreateSDiv(leftValue, rightValue, "divtmp");
            break;
        case OPERATOR_MOD:
            result = compiler.getContext().builder.CreateSRem(leftValue, rightValue, "modtmp");
            break;
        case OPERATOR_AND:
            result = compiler.getContext().builder.CreateAnd(leftValue, rightValue, "andtmp");
            break;
        case OPERATOR_OR:
            result = compiler.getContext().builder.CreateOr(leftValue, rightValue, "ortmp");
            break;
        case OPERATOR_LT:
            result = compiler.getContext().builder.CreateICmpSLT(leftValue, rightValue, "ltcmp");
            break;
        case OPERATOR_LTE:
            result = compiler.getContext().builder.CreateICmpSLE(leftValue, rightValue, "ltecmp");
            break;
        case OPERATOR_GT:
            result = compiler.getContext().builder.CreateICmpSGT(leftValue, rightValue, "gtcmp");
            break;
        case OPERATOR_GTE:
            result = compiler.getContext().builder.CreateICmpSGE(leftValue, rightValue, "gtecmp");
            break;
        case OPERATOR_EQ:
            result = compiler.getContext().builder.CreateICmpEQ(leftValue, rightValue, "eqcmp");
            break;
        case OPERATOR_NEQ:
            result = compiler.getContext().builder.CreateICmpNE(leftValue, rightValue, "neqcmp");
            break;
        default:
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Unknown operator type");
            CONDITION_FAILED;
        }

        if (!result)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to create binary expression");
            CONDITION_FAILED;
        }

        return result;
    }

    llvm::Value *BinaryExpressions::createTempValueForPointer(llvm::Value *value, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BackendSymTable &symTable = compiler.getSymTable();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "BinExp", "Creating temporary value for pointer");

        // For these temporary variables, this is how the naming convention will work:
        // (varName) + ".temp"
        std::string tempVarName = varName + ".temp";
        std::cout << "Temp Var Name: " << tempVarName << std::endl;

        STVariable *symTableNode = symTable.getVariable(compiler.getContext().currentNamespace, varName);
        if (!symTableNode)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to get variable node from symtable");
            CONDITION_FAILED;
        }

        llvm::Type *varType = symTableNode->LLVMType;
        llvm::Value *varValue = symTableNode->LLVMValue;
        CryoDataType varDataType = symTableNode->dataType;
        llvm::Type *varPointerType = types.getType(varDataType, 0);

        llvm::Value *tempValue = compiler.getContext().builder.CreateAlloca(varPointerType, varValue, tempVarName);
        if (!tempValue)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to create temporary value");
            CONDITION_FAILED;
        }

        // Store the value in the temporary variable
        compiler.getContext().builder.CreateStore(varValue, tempValue);

        // Add the temporary variable to the named values
        compiler.getContext().namedValues[tempVarName] = tempValue;

        return tempValue;
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
            CONDITION_FAILED;
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
                CONDITION_FAILED;
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
            CONDITION_FAILED;
        }

        return compiler.getContext().builder.CreateICmp(predicate, leftValue, rightValue, "compareResult");
    }

} // namespace Cryo
