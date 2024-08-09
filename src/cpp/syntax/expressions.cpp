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
            {
                std::cout << "[Expressions] Generating code for integer literal\n";
                llvm::ConstantInt *intVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(cryoContext.context),
                                                                   node->data.literalExpression.intValue);
                if (!intVal || intVal->getType() == nullptr)
                {
                    std::cerr << "[CPP] Error: Failed to create integer literal\n";
                    return nullptr;
                }
                return intVal;
            }
            case DATA_TYPE_FLOAT:
                std::cout << "[Expressions] Generating code for float literal\n";
                return llvm::ConstantFP::get(llvm::Type::getFloatTy(cryoContext.context),
                                             node->data.literalExpression.floatValue);
            case DATA_TYPE_STRING:
            {
                std::cout << "[Expressions] Generating code for string literal\n";
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
            case DATA_TYPE_BOOLEAN:
                std::cout << "[Expressions] Generating code for boolean literal\n";
                return llvm::ConstantInt::get(llvm::Type::getInt1Ty(cryoContext.context),
                                              node->data.boolean.value);

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
            llvm::Value *var = lookupVariable(node->data.varName.varName);
            if (!var)
            {
                std::cerr << "[CPP] Error: Variable not found: " << node->data.varName.varName << "\n";
                return nullptr;
            }

            if (llvm::isa<llvm::GlobalVariable>(var))
            {
                llvm::GlobalVariable *global = llvm::cast<llvm::GlobalVariable>(var);
                return cryoContext.builder.CreateLoad(global->getValueType(), global, node->data.varName.varName);
            }
            else if (var->getType()->isPointerTy())
            {
                return cryoContext.builder.CreateLoad(var->getType(), var, node->data.varName.varName);
            }
            return var;
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
