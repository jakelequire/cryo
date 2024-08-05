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

    void CryoSyntax::generateVarDeclaration(ASTNode *node)
    {
        std::cout << "[Variables] Generating variable declaration\n";
        if (node == nullptr)
        {
            std::cerr << "Error: node is null in generateVarDeclaration." << std::endl;
            return;
        }

        if (node->type != NODE_VAR_DECLARATION)
        {
            std::cerr << "Error: node is not a variable declaration in generateVarDeclaration." << std::endl;
            return;
        }

        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoModules &cryoModulesInstance = compiler.getModules();
        CryoContext &cryoContext = compiler.getContext();

        char *varName = node->data.varDecl.name;
        if (varName == nullptr)
        {
            std::cerr << "Error: variable name is null in generateVarDeclaration." << std::endl;
            return;
        }

        CryoDataType varType = node->data.varDecl.dataType;
        llvm::Type *llvmType = cryoTypesInstance.getLLVMType(varType);
        if (llvmType == nullptr)
        {
            std::cerr << "Error: failed to get LLVM type for variable " << varName << std::endl;
            return;
        }

        llvm::Value *initialValue = nullptr;
        if (node->data.varDecl.initializer)
        {
            initialValue = generateExpression(node->data.varDecl.initializer);
            if (initialValue == nullptr)
            {
                std::cerr << "Error: failed to generate initial value for variable " << varName << std::endl;
                return;
            }
        }
        else
        {
            initialValue = llvm::Constant::getNullValue(llvmType);
        }

        llvm::Value *var = createVariableDeclaration(node);
        if (var == nullptr)
        {
            std::cerr << "Error: failed to create variable declaration for " << varName << std::endl;
            return;
        }
        else
        {
            std::cout << "[Variables] Created variable declaration for " << varName << std::endl;
        }

        cryoContext.namedValues[varName] = var;

        std::cout << "[CPP] Generated variable declaration for " << varName << std::endl;
    }

    llvm::Value *CryoSyntax::lookupVariable(char *name)
    {
        CryoContext &cryoContext = compiler.getContext();

        llvm::Value *var = cryoContext.namedValues[name];
        if (!var)
        {
            std::cerr << "[CPP] Error: Variable " << name << " not found\n";
        }
        return var;
    }

    llvm::Value *CryoSyntax::createVariableDeclaration(ASTNode *node)
    {
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();

        char *varName = node->data.varDecl.name;
        CryoDataType varType = node->data.varDecl.dataType;
        llvm::Type *llvmType = cryoTypesInstance.getLLVMType(varType);

        if (!llvmType)
        {
            std::cerr << "Error: Failed to get LLVM type for " << CryoDataTypeToString(varType) << std::endl;
            return nullptr;
        }

        llvm::Value *var = nullptr;
        llvm::Constant *initialValue = nullptr;

        if (node->data.varDecl.initializer)
        {
            ASTNode *initNode = node->data.varDecl.initializer;
            switch (varType)
            {
            case DATA_TYPE_INT:
                if (initNode->type == NODE_LITERAL_EXPR && initNode->data.literalExpression.dataType == DATA_TYPE_INT)
                {
                    int intValue = initNode->data.literalExpression.intValue;
                    initialValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(cryoContext.context), intValue);
                }
                break;
            case DATA_TYPE_FLOAT:
                if (initNode->type == NODE_LITERAL_EXPR && initNode->data.literalExpression.dataType == DATA_TYPE_FLOAT)
                {
                    float floatValue = initNode->data.literalExpression.floatValue;
                    initialValue = llvm::ConstantFP::get(llvm::Type::getFloatTy(cryoContext.context), floatValue);
                }
                break;
            case DATA_TYPE_BOOLEAN:
                if (initNode->type == NODE_LITERAL_EXPR && initNode->data.literalExpression.dataType == DATA_TYPE_BOOLEAN)
                {
                    bool boolValue = initNode->data.literalExpression.booleanValue;
                    initialValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(cryoContext.context), boolValue);
                }
                break;
            case DATA_TYPE_STRING:
                if (initNode->type == NODE_LITERAL_EXPR && initNode->data.literalExpression.dataType == DATA_TYPE_STRING)
                {
                    char *strValue = initNode->data.literalExpression.stringValue;
                    std::cout << "[Variables] Creating string constant: " << strValue << std::endl;

                    llvm::Constant *stringConstant = llvm::ConstantDataArray::getString(cryoContext.context, strValue);
                    llvm::GlobalVariable *globalString = new llvm::GlobalVariable(
                        *cryoContext.module,
                        stringConstant->getType(),
                        true,
                        llvm::GlobalValue::PrivateLinkage,
                        stringConstant,
                        ".str");

                    initialValue = llvm::ConstantExpr::getBitCast(globalString, llvm::Type::getInt8Ty(cryoContext.context));
                    llvmType = llvm::Type::getInt8Ty(cryoContext.context);

                    std::cout << "[Variables] Created string constant with type: " << cryoTypesInstance.LLVMTypeToString(llvmType) << std::endl;
                }
                else
                {
                    std::cerr << "[Variables] Error: Invalid string initializer" << std::endl;
                }
                break;
            case DATA_TYPE_ARRAY:
                if (initNode->type == NODE_ARRAY_LITERAL)
                {
                    // Handle array initialization
                    // You'll need to implement this based on your array structure
                }
                break;
            default:
                std::cerr << "Unsupported variable type for initialization: " << CryoDataTypeToString(varType) << std::endl;
                break;
            }
        }

        if (node->data.varDecl.isGlobal)
        {
            if (initialValue)
            {
                var = new llvm::GlobalVariable(*cryoContext.module, llvmType, false,
                                               llvm::GlobalValue::ExternalLinkage,
                                               initialValue, varName);
            }
            else
            {
                var = new llvm::GlobalVariable(*cryoContext.module, llvmType, false,
                                               llvm::GlobalValue::ExternalLinkage,
                                               llvm::Constant::getNullValue(llvmType), varName);
            }
        }
        else
        {
            var = cryoContext.builder.CreateAlloca(llvmType, nullptr, varName);
            if (initialValue)
            {
                // Check if initialValue needs to be loaded
                if (initialValue->getType()->isPointerTy())
                {
                    cryoContext.builder.CreateLoad(initialValue->getType(), initialValue);
                }

                // Check if we need to bitcast the initialValue to match the variable type
                if (initialValue->getType() != llvmType)
                {
                    llvm::Value *val = cryoContext.builder.CreateBitCast(initialValue, llvmType);
                    cryoContext.builder.CreateStore(val, var);
                }

                cryoContext.builder.CreateStore(initialValue, var);
            }
        }

        return var;
    }

    llvm::Value *CryoSyntax::getVariableValue(char *name)
    {
        CryoContext &cryoContext = compiler.getContext();

        llvm::Value *var = lookupVariable(name);
        if (!var)
        {
            return nullptr;
        }
        llvm::Type *varType = var->getType();
        return cryoContext.builder.CreateLoad(varType, var, name);
    }

    llvm::GlobalVariable *CryoSyntax::createGlobalVariable(llvm::Type *varType, llvm::Constant *initialValue, char *varName)
    {
        CryoContext &cryoContext = compiler.getContext();

        // Check if global variable already exists
        if (cryoContext.module->getGlobalVariable(varName))
        {
            std::cout << "[Variables] Global variable " << varName << " already declared. Skipping." << std::endl;
            return nullptr;
        }

        // Create global variable
        llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(*cryoContext.module, varType, false, llvm::GlobalValue::ExternalLinkage, initialValue, varName);
        return globalVar;
    }

    llvm::Value *CryoSyntax::loadGlobalVariable(llvm::GlobalVariable *globalVar, char *name)
    {
        CryoContext &cryoContext = compiler.getContext();

        llvm::Type *varType = globalVar->getType();
        return cryoContext.builder.CreateLoad(varType, globalVar, name);
    }

    void CryoSyntax::generateArrayLiteral(ASTNode *node)
    {
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();

        char *varName = node->data.varDecl.name;
        CryoDataType varType = node->data.varDecl.dataType;
        llvm::Type *llvmType = cryoTypesInstance.getLLVMType(varType);

        llvm::Value *var = nullptr;
        if (node->data.varDecl.isGlobal)
        {
            llvm::Constant *initialValue = nullptr;
            if (node->data.varDecl.initializer)
            {
                initialValue = (llvm::Constant *)generateExpression(node->data.varDecl.initializer);
            }
            var = createGlobalVariable(llvmType, initialValue, varName);
        }
        else
        {
            var = cryoContext.builder.CreateAlloca(llvmType, nullptr, varName);
        }

        cryoContext.namedValues[varName] = var;
    }

    std::vector<llvm::Constant *> CryoSyntax::generateArrayElements(ASTNode *arrayLiteral)
    {
        std::vector<llvm::Constant *> elements;
        for (int i = 0; i < arrayLiteral->data.arrayLiteral.elementCount; i++)
        {
            ASTNode *element = arrayLiteral->data.arrayLiteral.elements[i];
            llvm::Constant *llvmElement = (llvm::Constant *)generateExpression(element);
            elements.push_back(llvmElement);
        }
        return elements;
    }

} // namespace Cryo
