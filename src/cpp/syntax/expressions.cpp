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
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for expression");

        switch (node->metaData->type)
        {
        case CryoNodeType::NODE_LITERAL_EXPR:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for literal expression");
            switch (node->data.literal->dataType)
            {
            case DATA_TYPE_INT:
            {
                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for integer literal");
                llvm::ConstantInt *intVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(cryoContext.context),
                                                                   node->data.literal->value.intValue);
                if (!intVal || intVal->getType() == nullptr)
                {
                    cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Failed to generate code for integer literal");
                    return nullptr;
                }
                return intVal;
            }
            case DATA_TYPE_FLOAT:
            {
                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for float literal");
                return llvm::ConstantFP::get(llvm::Type::getFloatTy(cryoContext.context),
                                             node->data.literal->value.floatValue);
            }
            case DATA_TYPE_STRING:
            {
                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for string literal");
                llvm::Constant *strConstant = llvm::ConstantDataArray::getString(cryoContext.context, node->data.literal->value.stringValue);
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
                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for boolean literal");
                return llvm::ConstantInt::get(llvm::Type::getInt1Ty(cryoContext.context),
                                              node->data.literal->value.booleanValue);

            case DATA_TYPE_VOID:
            case DATA_TYPE_UNKNOWN:
                cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Unknown data type in generateExpression");
                return nullptr;
            default:
                cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Unknown data type in generateExpression");
                return nullptr;
            }
        }

        case CryoNodeType::NODE_BINARY_EXPR:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for binary expression");
            return generateBinaryOperation(node);
        }

        case CryoNodeType::NODE_VAR_NAME:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for variable name");
            llvm::Value *var = lookupVariable(node->data.varName->varName);
            if (!var)
            {
                std::cerr << "[ERROR] \t@" << __LINE__ << "\t{Generation}\t\t Variable not found: " << node->data.varName->varName << std::endl;
                return nullptr;
            }

            if (llvm::isa<llvm::GlobalVariable>(var))
            {
                llvm::GlobalVariable *global = llvm::cast<llvm::GlobalVariable>(var);

                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for global variable");
                return cryoContext.builder.CreateLoad(global->getValueType(), global, node->data.varName->varName);
            }
            else if (var->getType()->isPointerTy())
            {
                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for pointer variable");
                return cryoContext.builder.CreateLoad(var->getType(), var, node->data.varName->varName);
            }
            return var;
        }

        case CryoNodeType::NODE_VAR_DECLARATION:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for variable declaration");
            return createVariableDeclaration(node);
        }

        case CryoNodeType::NODE_STRING_LITERAL:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for string literal");
            return cryoTypesInstance.createString(node->data.literal->value.stringValue);
        }

        default:
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Unsupported node type in generateExpression");
            return nullptr;
        }
        }
    }

    void CryoSyntax::generateStatement(ASTNode *node)
    {
    }

} // namespace Cryo
