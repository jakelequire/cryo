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

    llvm::Value *CryoSyntax::generateBinaryOperation(ASTNode *node)
    {
        std::cout << "[CPP] Generating code for binary operation\n";
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();

        llvm::Value *left = generateExpression(node->data.bin_op->left);
        llvm::Value *right = generateExpression(node->data.bin_op->right);

        if (!left || !right)
        {
            std::cerr << "[CPP] Error generating binary operation: operands are null\n";
            return nullptr;
        }

        std::cout << "[CPP] Binary operation operands generated\n";

        switch (node->data.bin_op->op)
        {
        case OPERATOR_ADD:
            return cryoContext.builder.CreateAdd(left, right, "addtmp");
        case OPERATOR_SUB:
            return cryoContext.builder.CreateSub(left, right, "subtmp");
        case OPERATOR_MUL:
            return cryoContext.builder.CreateMul(left, right, "multmp");
        case OPERATOR_DIV:
            return cryoContext.builder.CreateSDiv(left, right, "divtmp");
        case OPERATOR_EQ:
            return cryoContext.builder.CreateICmpEQ(left, right, "eqtmp");
        case OPERATOR_NEQ:
            return cryoContext.builder.CreateICmpNE(left, right, "neqtmp");
        case OPERATOR_LT:
            return cryoContext.builder.CreateICmpSLT(left, right, "ltcmp");
        case OPERATOR_GT:
            return cryoContext.builder.CreateICmpSGT(left, right, "gttmp");
        case OPERATOR_LTE:
            return cryoContext.builder.CreateICmpSLE(left, right, "letmp");
        case OPERATOR_GTE:
            return cryoContext.builder.CreateICmpSGE(left, right, "getmp");
        default:
            std::cerr << "[CPP] Unknown binary operator\n";
            return nullptr;
        }

        std::cout << "[CPP] DEBUG Binary operation generated.\n";
    }

} // namespace Cryo
