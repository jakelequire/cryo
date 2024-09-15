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

    llvm::Value *BinaryExpressions::createBinaryExpression(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Binary Expression");

        ASTNode *leftASTNode = node->data.bin_op->left;
        ASTNode *rightASTNode = node->data.bin_op->right;

        if (!leftASTNode || !rightASTNode)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to generate operands for binary expression");
            return nullptr;
        }

        llvm::Value *leftValue = compiler.getGenerator().getInitilizerValue(leftASTNode);
        debugger.logMessage("INFO", __LINE__, "BinExp", "Left Value Generated");

        llvm::Value *rightValue = compiler.getGenerator().getInitilizerValue(rightASTNode);
        debugger.logMessage("INFO", __LINE__, "BinExp", "Right Value Generated");

        if (!leftValue || !rightValue)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to generate values for binary expression");
            return nullptr;
        }

        CryoOperatorType operatorType = node->data.bin_op->op;
        switch (operatorType)
        {
        case OPERATOR_ADD:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Add Expression");
            return compiler.getContext().builder.CreateAdd(leftValue, rightValue, "addtmp");
        }
        case OPERATOR_SUB:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Sub Expression");
            return compiler.getContext().builder.CreateSub(leftValue, rightValue, "subtmp");
        }
        case OPERATOR_MUL:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Mul Expression");
            return compiler.getContext().builder.CreateMul(leftValue, rightValue, "multmp");
        }
        case OPERATOR_DIV:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Div Expression");
            return compiler.getContext().builder.CreateSDiv(leftValue, rightValue, "divtmp");
        }
        case OPERATOR_MOD:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Mod Expression");
            return compiler.getContext().builder.CreateSRem(leftValue, rightValue, "modtmp");
        }
        case OPERATOR_AND:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating And Expression");
            return compiler.getContext().builder.CreateAnd(leftValue, rightValue, "andtmp");
        }
        case OPERATOR_OR:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Or Expression");
            return compiler.getContext().builder.CreateOr(leftValue, rightValue, "ortmp");
        }
        case OPERATOR_LT:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Less Than Expression");
            return compiler.getContext().builder.CreateICmpSLT(leftValue, rightValue, "ltcmp");
        }
        case OPERATOR_LTE:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Less Than or Equal Expression");
            return compiler.getContext().builder.CreateICmpSLE(leftValue, rightValue, "ltecmp");
        }
        case OPERATOR_GT:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Greater Than Expression");
            return compiler.getContext().builder.CreateICmpSGT(leftValue, rightValue, "gtcmp");
        }
        case OPERATOR_GTE:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Greater Than or Equal Expression");
            return compiler.getContext().builder.CreateICmpSGE(leftValue, rightValue, "gtecmp");
        }
        case OPERATOR_EQ:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Equal Expression");
            return compiler.getContext().builder.CreateICmpEQ(leftValue, rightValue, "eqcmp");
        }
        case OPERATOR_NEQ:
        {
            debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Not Equal Expression");
            return compiler.getContext().builder.CreateICmpNE(leftValue, rightValue, "neqcmp");
        }
        default:
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Unknown operator type");
            return nullptr;
        }
        }
        debugger.logMessage("INFO", __LINE__, "BinExp", "Binary Expression Created");

        return nullptr;
    }

    llvm::Value *BinaryExpressions::createComparisonExpression(ASTNode *left, ASTNode *right, CryoOperatorType op)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        std::string currentModule = compiler.getContext().currentNamespace;
        debugger.logMessage("INFO", __LINE__, "BinExp", "Creating Comparison Expression");

        llvm::Value *leftValue = compiler.getGenerator().getInitilizerValue(left);
        llvm::Value *rightValue = compiler.getGenerator().getInitilizerValue(right);

        if (!leftValue || !rightValue)
        {
            debugger.logMessage("ERROR", __LINE__, "BinExp", "Failed to generate values for comparison expression");
            return nullptr;
        }

        CryoDataType leftType = left->data.varDecl->type;
        CryoDataType rightType = right->data.varDecl->type;
        CryoNodeType leftNodeType = left->metaData->type;
        CryoNodeType rightNodeType = right->metaData->type;
        llvm::Type *llvmLeftType = nullptr;
        llvm::Type *llvmRightType = nullptr;

        if (leftNodeType == NODE_VAR_NAME)
        {
            // Find the variable in the symbol table
            std::string argName = std::string(left->data.varName->varName);
            std::cout << "Argument Name: " << argName << std::endl;
            std::cout << "Current Module: " << currentModule << std::endl;
            CryoVariableNode *retreivedNode = compiler.getSymTable().getVariableNode(currentModule, argName);
            if (!retreivedNode)
            {
                debugger.logMessage("ERROR", __LINE__, "Functions", "Argument not found");
                compiler.dumpModule();
                exit(1);
            }
            std::cout << "--------<OUTPUT>--------" << std::endl;
            std::cout << retreivedNode->name << std::endl;
            debugger.logNode(retreivedNode->initializer);
            std::cout << "--------</OUTPUT>--------" << std::endl;

            // DEBUG_BREAKPOINT;
        }

        std::cout << "Left Node Type: " << CryoNodeTypeToString(leftNodeType) << std::endl;
        std::cout << "Left Type: " << CryoDataTypeToString(leftType) << std::endl;
        std::cout << "Right Node Type: " << CryoNodeTypeToString(rightNodeType) << std::endl;
        std::cout << "Right Type: " << CryoDataTypeToString(rightType) << std::endl;

        // DEBUG_BREAKPOINT;

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

        llvm::Value *compareExpr = compiler.getContext().builder.CreateICmp(predicate, leftValue, rightValue, "compareResult");
        debugger.logMessage("INFO", __LINE__, "BinExp", "Comparison Expression Created");

        return compareExpr;
    }

} // namespace Cryo
