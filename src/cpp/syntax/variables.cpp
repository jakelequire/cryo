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
    void CryoSyntax::initializeVariable(CryoContext &context, llvm::Value *var, ASTNode *initializer)
    {
        llvm::Value *initValue = generateExpression(initializer);
        if (!initValue)
        {
            compiler.getDebugger().logError("Failed to generate initializer");
            return;
        }

        if (var->getType()->isPointerTy() && !initValue->getType()->isPointerTy())
        {
            initValue = context.builder.CreateBitCast(initValue, var->getType());
        }
        else if (llvm::isa<llvm::GlobalVariable>(var))
        {
            llvm::cast<llvm::GlobalVariable>(var)->setInitializer(llvm::cast<llvm::Constant>(initValue));
        }
        else
        {
            context.builder.CreateStore(initValue, var);
        }
    }

    llvm::Value *CryoSyntax::createLocalVariable(CryoContext &context, llvm::Type *type, llvm::StringRef name)
    {
        return context.builder.CreateAlloca(type, nullptr, name);
    }

    llvm::Value *CryoSyntax::allocateVariable(CryoContext &context, llvm::Type *type, const char *name)
    {
        CryoDebugger &debugger = compiler.getDebugger();

        if (!context.builder.GetInsertBlock())
        {
            llvm::Function *currentFunction = context.builder.GetInsertBlock() ? context.builder.GetInsertBlock()->getParent() : nullptr;
            std::string functionName = currentFunction ? currentFunction->getName().str() : "Unknown";
            debugger.logError("No current basic block for insertion in function: " + functionName);
            return nullptr;
        }

        try
        {
            return context.builder.CreateAlloca(type, nullptr, name);
        }
        catch (const std::exception &e)
        {
            debugger.logError("Exception during CreateAlloca for variable: " + std::string(name), e.what());
            return nullptr;
        }
    }

    bool CryoSyntax::validateVarDeclarationNode(const ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();

        if (!node || node->type != NODE_VAR_DECLARATION)
        {
            debugger.logError("Invalid node in validateVarDeclarationNode");
            return false;
        }
        if (!node->data.varDecl.name)
        {
            debugger.logError("Variable name is null in validateVarDeclarationNode");
            return false;
        }
        return true;
    }

    void CryoSyntax::generateVarDeclaration(ASTNode *node)
    {
        if (!validateVarDeclarationNode(node))
            return;

        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypes = compiler.getTypes();
        CryoDebugger &debugger = compiler.getDebugger();

        llvm::StringRef varName(node->data.varDecl.name);
        llvm::Type *llvmType = cryoTypes.getLLVMType(node->data.varDecl.dataType);
        if (!llvmType)
        {
            debugger.logError("Unsupported variable type", CryoDataTypeToString(node->data.varDecl.dataType));
            return;
        }

        llvm::Value *var = nullptr;
        if (node->data.varDecl.isGlobal)
        {
            var = createGlobalVariable(cryoContext, llvmType, varName, node->data.varDecl.isReference);
        }
        else
        {
            var = createLocalVariable(cryoContext, llvmType, varName);
        }

        if (!var)
        {
            debugger.logError("Failed to create variable", varName.str());
            return;
        }

        cryoContext.namedValues[varName.str()] = var;

        if (node->data.varDecl.initializer)
        {
            initializeVariable(cryoContext, var, node->data.varDecl.initializer);
        }

        debugger.logSuccess("Generated variable declaration for", varName.str());
    }

    llvm::Value *CryoSyntax::lookupVariable(char *name)
    {
        CryoContext &cryoContext = compiler.getContext();

        // First, check if it's a global variable
        llvm::GlobalVariable *global = cryoContext.module->getGlobalVariable(name);
        if (global)
        {
            return global;
        }

        // Then check local variables
        auto it = cryoContext.namedValues.find(name);
        if (it != cryoContext.namedValues.end())
        {
            return it->second;
        }

        std::cerr << "[CPP] Error: Unknown variable name: " << name << "\n";
        return nullptr;
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

                    initialValue = globalString;
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
                                               nullptr, varName);
            }
        }
        else
        {
            var = cryoContext.builder.CreateAlloca(llvmType, nullptr, varName);
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
        cryoContext.builder.CreateLoad(varType, var, name);
        return var;
    }

    llvm::Value *CryoSyntax::createGlobalVariable(CryoContext &context, llvm::Type *type, llvm::StringRef name, bool isConstant)
    {
        CryoContext &cryoContext = compiler.getContext();
        llvm::GlobalVariable *global = new llvm::GlobalVariable(
            *compiler.getContext().module, type, isConstant,
            llvm::GlobalValue::ExternalLinkage,
            llvm::Constant::getNullValue(type), name);
        return global;
    }

    llvm::Value *CryoSyntax::loadGlobalVariable(llvm::GlobalVariable *globalVar, char *name)
    {
        CryoContext &cryoContext = compiler.getContext();

        llvm::Type *varType = globalVar->getType();
        cryoContext.builder.CreateLoad(varType, globalVar, name);
        cryoContext.namedValues[name] = globalVar;
        return globalVar;
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
            var = new llvm::GlobalVariable(*cryoContext.module, llvmType, false,
                                           llvm::GlobalValue::ExternalLinkage,
                                           initialValue, varName);
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
