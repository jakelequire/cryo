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

namespace Cryo {

void CryoSyntax::generateVarDeclaration(ASTNode* node) {
    std::cout << "[Variables] Generating variable declaration\n";
    if (node == nullptr) {
        std::cerr << "Error: node is null in generateVarDeclaration." << std::endl;
        return;
    }

    if (node->type != NODE_VAR_DECLARATION) {
        std::cerr << "Error: node is not a variable declaration in generateVarDeclaration." << std::endl;
        return;
    }

    CryoTypes& cryoTypesInstance = compiler.getTypes();
    CryoModules& cryoModulesInstance = compiler.getModules();
    CryoContext& cryoContext = compiler.getContext();

    char* varName = node->data.varDecl.name;
    if (varName == nullptr) {
        std::cerr << "Error: variable name is null in generateVarDeclaration." << std::endl;
        return;
    }

    CryoDataType varType = node->data.varDecl.dataType;
    llvm::Type* llvmType = cryoTypesInstance.getLLVMType(varType);
    if (llvmType == nullptr) {
        std::cerr << "Error: failed to get LLVM type for variable " << varName << std::endl;
        return;
    }

    llvm::Value* initialValue = nullptr;
    if (node->data.varDecl.initializer) {
        initialValue = generateExpression(node->data.varDecl.initializer);
        if (initialValue == nullptr) {
            std::cerr << "Error: failed to generate initial value for variable " << varName << std::endl;
            return;
        }
    }

    llvm::Value* var = createVariableDeclaration(node);
    if (var == nullptr) {
        std::cerr << "Error: failed to create variable declaration for " << varName << std::endl;
        return;
    }

    cryoContext.namedValues[varName] = var;

    if (initialValue) {
        std::cout << "[DEBUG] Initial Value: "
                  << cryoTypesInstance.LLVMTypeIdToString(initialValue->getType()->getTypeID())
                  << "\t\nVariable: "
                  << cryoTypesInstance.LLVMTypeIdToString(var->getType()->getTypeID())
                  << "\t\nLLVM Type: "
                  << cryoTypesInstance.LLVMTypeIdToString(llvmType->getTypeID())
                  << std::endl;

        if (auto *ptrType = llvm::dyn_cast<llvm::PointerType>(var->getType())) {
            if (ptrType == initialValue->getType()) {
                cryoContext.builder.CreateStore(initialValue, var);
            } else {
                std::cerr << "Error: Type mismatch between variable and initial value.\n";
            }
        } else {
            std::cerr << "Error: var is not a pointer type as expected.\n";
        }
    }

    std::cout << "[CPP] Generated variable declaration for " << varName << std::endl;
}



llvm::Value* CryoSyntax::lookupVariable(char* name) {
    CryoContext& cryoContext = compiler.getContext();

    llvm::Value* var = cryoContext.namedValues[name];
    if (!var) {
        std::cerr << "[CPP] Error: Variable " << name << " not found\n";
    }
    return var;
}

llvm::Value* CryoSyntax::createVariableDeclaration(ASTNode* node) {
    CryoTypes& cryoTypesInstance = compiler.getTypes();
    CryoContext& cryoContext = compiler.getContext();

    char* varName = node->data.varDecl.name;
    CryoDataType varType = node->data.varDecl.dataType;
    llvm::Type* llvmType = cryoTypesInstance.getLLVMType(varType);

    llvm::Value* var = nullptr;
    if (node->data.varDecl.isGlobal) {
        llvm::Constant* initialValue = nullptr;
        if (node->data.varDecl.initializer) {
            initialValue = (llvm::Constant*)generateExpression(node->data.varDecl.initializer);
        }
        var = createGlobalVariable(llvmType, initialValue, varName);
    } else {
        var = cryoContext.builder.CreateAlloca(llvmType, nullptr, varName);
    }

    return var;
}

llvm::Value* CryoSyntax::getVariableValue(char* name) {
    CryoContext& cryoContext = compiler.getContext();

    llvm::Value* var = lookupVariable(name);
    if (!var) {
        return nullptr;
    }
    llvm::Type* varType = var->getType();
    return cryoContext.builder.CreateLoad(varType, var, name);
}

llvm::GlobalVariable* CryoSyntax::createGlobalVariable(llvm::Type* varType, llvm::Constant* initialValue, char* varName) {
    CryoContext& cryoContext = compiler.getContext();

    llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(*cryoContext.module, varType, false, llvm::GlobalValue::ExternalLinkage, initialValue, varName);
    return globalVar;
}

llvm::Value* CryoSyntax::loadGlobalVariable(llvm::GlobalVariable* globalVar, char* name) {
    CryoContext& cryoContext = compiler.getContext();

    llvm::Type* varType = globalVar->getType();
    return cryoContext.builder.CreateLoad(varType, globalVar, name);
}

void CryoSyntax::generateArrayLiteral(ASTNode* node) {
    CryoTypes& cryoTypesInstance = compiler.getTypes();
    CryoContext& cryoContext = compiler.getContext();
    
    char* varName = node->data.varDecl.name;
    CryoDataType varType = node->data.varDecl.dataType;
    llvm::Type* llvmType = cryoTypesInstance.getLLVMType(varType);

    llvm::Value* var = nullptr;
    if (node->data.varDecl.isGlobal) {
        llvm::Constant* initialValue = nullptr;
        if (node->data.varDecl.initializer) {
            initialValue = (llvm::Constant*)generateExpression(node->data.varDecl.initializer);
        }
        var = createGlobalVariable(llvmType, initialValue, varName);
    } else {
        var = cryoContext.builder.CreateAlloca(llvmType, nullptr, varName);
    }

    cryoContext.namedValues[varName] = var;
}

std::vector<llvm::Constant*> CryoSyntax::generateArrayElements(ASTNode* arrayLiteral) {
    std::vector<llvm::Constant*> elements;
    for (int i = 0; i < arrayLiteral->data.arrayLiteral.elementCount; i++) {
        ASTNode* element = arrayLiteral->data.arrayLiteral.elements[i];
        llvm::Constant* llvmElement = (llvm::Constant*)generateExpression(element);
        elements.push_back(llvmElement);
    }
    return elements;
}


} // namespace Cryo
