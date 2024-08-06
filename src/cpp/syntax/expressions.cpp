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

    llvm::Value *CryoSyntax::generateExpression(ASTNode *node)
    {
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoModules &cryoModulesInstance = compiler.getModules();
        CryoContext &cryoContext = compiler.getContext();

        std::cout << "[Expressions] Generating code for expression\n"
                  << "Type: " << CryoNodeTypeToString(node->type) << "\n";

        switch (node->type)
        {
        case CryoNodeType::NODE_LITERAL_EXPR:
        {
            std::cout << "[Expressions] Generating code for literal expression\n";
            switch (node->data.literalExpression.dataType)
            {
            case DATA_TYPE_INT:
                return llvm::ConstantInt::get(llvm::Type::getInt32Ty(cryoContext.context),
                                              node->data.literalExpression.intValue);
            case DATA_TYPE_FLOAT:
                return llvm::ConstantFP::get(llvm::Type::getFloatTy(cryoContext.context),
                                             node->data.literalExpression.floatValue);
            case DATA_TYPE_STRING:
            {
                llvm::Constant *strConstant = llvm::ConstantDataArray::getString(cryoContext.context, node->data.literalExpression.stringValue);
                llvm::GlobalVariable *strGlobal = new llvm::GlobalVariable(
                    *cryoContext.module,
                    strConstant->getType(),
                    true,
                    llvm::GlobalValue::PrivateLinkage,
                    strConstant,
                    ".str");
                return llvm::ConstantExpr::getBitCast(strGlobal, llvm::Type::getInt8Ty(cryoContext.context));
            }
            case DATA_TYPE_VOID:
            case DATA_TYPE_UNKNOWN:
                std::cerr << "[CPP] Error: Unknown data type in generateExpression\n";
                return nullptr;
            default:
                std::cerr << "[CPP] Error: Unknown data type in generateExpression\n";
                return nullptr;
            }
        }

        case CryoNodeType::NODE_BINARY_EXPR:
        {
            std::cout << "[CPP] Generating code for binary expression!\n";
            return generateBinaryOperation(node);
        }

        case CryoNodeType::NODE_VAR_NAME:
        {
            std::cout << "[CPP] Generating code for variable\n";
            return lookupVariable(node->data.varName.varName);
        }

        case CryoNodeType::NODE_VAR_DECLARATION:
        {
            std::cout << "[CPP] Generating code for variable declaration\n";
            return createVariableDeclaration(node);
        }

        case CryoNodeType::NODE_STRING_LITERAL:
        {
            std::cout << "[CPP] Generating code for string literal\n";
            return cryoTypesInstance.createString(node->data.literalExpression.stringValue);
        }

        default:
        {
            std::cout << "[CPP - Error] Unknown expression type: " << node->type << "\n";
            std::cerr << "[CPP] Unknown expression type\n";
            return nullptr;
        }
        }
    }

    void CryoSyntax::generateStatement(ASTNode *node)
    {
    }

} // namespace Cryo
