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
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        llvm::Value *initValue = generateExpression(initializer);
        if (!initValue)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Failed to generate initial value for variable");
            return;
        }

        if (var->getType()->isPointerTy() && !initValue->getType()->isPointerTy())
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Casting initial value to pointer type");
            initValue = context.builder.CreateBitCast(initValue, var->getType());
        }
        else if (llvm::isa<llvm::GlobalVariable>(var))
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Storing initial value in global variable");
            llvm::cast<llvm::GlobalVariable>(var)->setInitializer(llvm::cast<llvm::Constant>(initValue));
        }
        else
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Storing initial value in local variable");
            context.builder.CreateStore(initValue, var);
        }
    }

    llvm::Constant *CryoSyntax::getVariableValue(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        if (!node || node->metaData->type != NODE_VAR_DECLARATION)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Invalid variable declaration node");
            return nullptr;
        }

        llvm::Value *var = lookupVariable(node->data.varDecl->name);
        if (!var)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found: " + std::string(node->data.varDecl->name));
            return nullptr;
        }

        llvm::Type *varType = var->getType();
        cryoContext.builder.CreateLoad(varType, var, node->data.varDecl->name);

        cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Loaded variable: " + std::string(node->data.varDecl->name));
        return (llvm::Constant *)var;
    }

    llvm::Value *CryoSyntax::createLocalVariable(CryoContext &context, llvm::Type *type, llvm::StringRef name)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Allocating local variable");

        if (!context.builder.GetInsertBlock())
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "No valid insertion point for local variable allocation");
            return nullptr;
        }

        try
        {
            llvm::AllocaInst *allocaInst = context.builder.CreateAlloca(type, nullptr, name);
            if (!allocaInst)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Failed to allocate variable");
                return nullptr;
            }
            return allocaInst;
        }
        catch (const std::exception &e)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Exception during variable allocation: " + std::string(e.what()));
            return nullptr;
        }
    }

    llvm::Value *CryoSyntax::allocateVariable(CryoContext &context, llvm::Type *type, const char *name)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        if (!context.builder.GetInsertBlock())
        {
            llvm::Function *currentFunction = context.builder.GetInsertBlock() ? context.builder.GetInsertBlock()->getParent() : nullptr;
            std::string functionName = currentFunction ? currentFunction->getName().str() : "Unknown";
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "No insert block found in function: " + functionName);
            return nullptr;
        }

        try
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Allocating variable");
            return context.builder.CreateAlloca(type, nullptr, name);
        }
        catch (const std::exception &e)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Failed to allocate variable");
            return nullptr;
        }
    }

    bool CryoSyntax::validateVarDeclarationNode(const ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        if (!node || node->metaData->type != NODE_VAR_DECLARATION)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Invalid variable declaration node");
            return false;
        }
        if (!node->data.varDecl->name)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Variable name is null");
            return false;
        }
        return true;
    }

    void CryoSyntax::generateVarDeclaration(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (!validateVarDeclarationNode(node))
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Invalid variable declaration node");
            return;
        }

        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypes = compiler.getTypes();
        CryoDebugger &debugger = compiler.getDebugger();

        llvm::Constant *initialValue = nullptr;
        if (node->data.varDecl->initializer->data.literal)
        {
            initialValue = (llvm::Constant *)generateExpression(node->data.varDecl->initializer);
            if (!initialValue)
            {
                debugger.logMessage("ERROR", __LINE__, "Variables", "Failed to generate initial value for variable");
                return;
            }
        }

        llvm::StringRef varName(node->data.varDecl->name);
        llvm::Type *llvmType = cryoTypes.getLLVMType(node->data.varDecl->type);
        if (!llvmType)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Unsupported variable type");
            return;
        }

        llvm::Value *var = nullptr;
        if (node->data.varDecl->isGlobal)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Creating Global Variable");
            var = createGlobalVariable(cryoContext, llvmType, varName, node->data.varDecl->isReference, initialValue);
        }
        else
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Creating Local Variable");
            std::string varNameStr = varName.str();
            var = createLocalVariable(cryoContext, llvmType, varNameStr);
        }

        if (!var)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Failed to create variable");
            return;
        }

        cryoContext.namedValues[varName.str()] = var;

        debugger.logMessage("INFO", __LINE__, "Variables", "Generated variable declaration for " + varName.str());
    }

    llvm::Value *CryoSyntax::lookupVariable(char *name)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        // First, check if it's a global variable
        llvm::GlobalVariable *global = cryoContext.module->getGlobalVariable(name);
        if (global)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Found global variable: " + std::string(name));
            return global;
        }

        // Then check local variables
        auto it = cryoContext.namedValues.find(name);
        if (it != cryoContext.namedValues.end())
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Found local variable: " + std::string(name));
            return it->second;
        }

        cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found: " + std::string(name));
        return nullptr;
    }

    llvm::Value *CryoSyntax::createVariableDeclaration(ASTNode *node)
    {
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        char *varName = node->data.varDecl->name;
        CryoDataType varType = node->data.varDecl->type;
        llvm::Type *llvmType = cryoTypesInstance.getLLVMType(varType);

        if (!llvmType)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Unsupported variable type");
            return nullptr;
        }

        llvm::Value *var = nullptr;
        llvm::Constant *initialValue = nullptr;

        CryoNodeType initType = node->metaData->type;
        std::string initTypeName = (std::string)CryoNodeTypeToString(initType);
        cryoDebugger.logMessage("DEBUG", __LINE__, "Variables", "Variable Initialization Type: " + initTypeName);

        CryoDataType initDataType = node->data.varDecl->type;
        std::string initDataTypeName = (std::string)CryoDataTypeToString(initDataType);
        cryoDebugger.logMessage("DEBUG", __LINE__, "Variables", "Variable Initialization Data Type: " + initDataTypeName);

        std::string varNameStr = varName;
        cryoDebugger.logMessage("DEBUG", __LINE__, "Variables", "Variable Name: " + varNameStr);
        // --------------------------------------------------------------------------------------------

        if (node->data.varDecl->initializer->data.literal)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Creating Variable with Literal Initializer");
            initialValue = (llvm::Constant *)generateExpression(node->data.varDecl->initializer);
        }
        else if (node->data.varDecl->initializer->metaData->type == NODE_ARRAY_LITERAL)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Creating Variable with Array Initializer");
            generateArrayLiteral(node->data.varDecl->initializer);
        }
        else
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Creating Variable without Initializer");
        }

        // --------------------------------------------------------------------------------------------
        cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Variable created: " + std::string(varName));
        cryoContext.namedValues[strdup(varName)] = var;

        if (initialValue)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Initializing Variable");
            cryoContext.builder.CreateStore(initialValue, var);
        }
        else
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Variable not initialized");
        }

        cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Generated variable declaration for " + std::string(varName));
        return var;
    }

    llvm::Value *CryoSyntax::getVariableValue(char *name)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        llvm::Value *var = lookupVariable(name);
        if (!var)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found: " + std::string(name));
            return nullptr;
        }
        llvm::Type *varType = var->getType();
        cryoContext.builder.CreateLoad(varType, var, name);

        cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Loaded variable: " + std::string(name));
        return var;
    }

    llvm::Value *CryoSyntax::createGlobalVariable(CryoContext &context, llvm::Type *type, llvm::StringRef name, bool isConstant, llvm::Constant *initialValue)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        llvm::GlobalVariable *global = new llvm::GlobalVariable(
            *compiler.getContext().module, type, isConstant,
            llvm::GlobalValue::ExternalLinkage,
            initialValue,
            name);

        cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Created global variable: " + name.str());
        return global;
    }

    llvm::Value *CryoSyntax::loadGlobalVariable(llvm::GlobalVariable *globalVar, char *name)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        llvm::Type *varType = globalVar->getType();
        cryoContext.builder.CreateLoad(varType, globalVar, name);
        cryoContext.namedValues[name] = globalVar;

        cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Loaded global variable: " + std::string(name));
        return globalVar;
    }

    void CryoSyntax::generateArrayLiteral(ASTNode *node)
    {
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        char *varName = node->data.varDecl->name;
        CryoDataType varType = node->data.varDecl->type;
        llvm::Type *llvmType = cryoTypesInstance.getLLVMType(varType);

        llvm::Value *var = nullptr;
        if (node->data.varDecl->isGlobal)
        {
            llvm::Constant *initialValue = nullptr;
            if (node->data.varDecl->initializer->data.literal)
            {
                cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Creating Global Variable with Literal Initializer");
                initialValue = (llvm::Constant *)generateExpression(node);
            }
            var = new llvm::GlobalVariable(*cryoContext.module, llvmType, false,
                                           llvm::GlobalValue::ExternalLinkage,
                                           initialValue, strdup(varName));
        }
        else
        {
            var = cryoContext.builder.CreateAlloca(llvmType, nullptr, strdup(varName));
            if (!var)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Variables", "Failed to create variable");
                return;
            }
        }

        cryoContext.namedValues[strdup(varName)] = var;
        cryoDebugger.logMessage("INFO", __LINE__, "Variables", "Generated variable declaration for " + std::string(varName));
    }

    std::vector<llvm::Constant *> CryoSyntax::generateArrayElements(ASTNode *arrayLiteral)
    {
        std::vector<llvm::Constant *> elements;
        for (int i = 0; i < arrayLiteral->data.array->elementCount - 1; i++)
        {
            ASTNode *element = arrayLiteral->data.array->elements[i];
            llvm::Constant *llvmElement = (llvm::Constant *)generateExpression(element);
            elements.push_back(llvmElement);
        }
        return elements;
    }

} // namespace Cryo
