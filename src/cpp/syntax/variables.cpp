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
    char* varName = node->data.varDecl.name;
    CryoDataType varType = node->data.varDecl.dataType;
    llvm::Type* llvmType = CryoTypes->getLLVMType(varType);

    llvm::Value* initialValue = nullptr;
    if (node->data.varDecl.initializer) {
        initialValue = generateExpression(node->data.varDecl.initializer);
    }

    llvm::Value* var = createVariableDeclaration(node);
    namedValues[varName] = var;

    if (initialValue) {
        builder.CreateStore(initialValue, var);
    }

    std::cout << "[CPP] Generated variable declaration for " << varName << std::endl;
}


llvm::Value* CryoSyntax::lookupVariable(char* name) {
    llvm::Value* var = namedValues[name];
    if (!var) {
        std::cerr << "[CPP] Error: Variable " << name << " not found\n";
    }
    return var;
}


llvm::Value* CryoSyntax::createVariableDeclaration(ASTNode* node) {
    char* varName = node->data.varDecl.name;
    CryoDataType varType = node->data.varDecl.dataType;
    llvm::Type* llvmType = CryoTypes->getLLVMType(varType);

    llvm::Value* var = nullptr;
    if (node->data.varDecl.isGlobal) {
        llvm::Constant* initialValue = nullptr;
        if (node->data.varDecl.initializer) {
            initialValue = (llvm::Constant*)generateExpression(node->data.varDecl.initializer);
        }
        var = createGlobalVariable(llvmType, initialValue, varName);
    } else {
        var = builder.CreateAlloca(llvmType, nullptr, varName);
    }

    return var;
}


llvm::Value* CryoSyntax::getVariableValue(char* name) {
    llvm::Value* var = lookupVariable(name);
    if (!var) {
        return nullptr;
    }
    llvm::Type* varType = var->getType();
    return builder.CreateLoad(varType, var, name);
}


llvm::GlobalVariable* CryoSyntax::createGlobalVariable(llvm::Type* varType, llvm::Constant* initialValue, char* varName) {
    llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(*module, varType, false, llvm::GlobalValue::ExternalLinkage, initialValue, varName);
    return globalVar;
}


llvm::Value* CryoSyntax::loadGlobalVariable(llvm::GlobalVariable* globalVar, char* name) {
    llvm::Type* varType = globalVar->getType();
    return builder.CreateLoad(varType, globalVar, name);
}


void CryoSyntax::generateArrayLiteral(ASTNode* node) {
    char* varName = node->data.varDecl.name;
    CryoDataType varType = node->data.varDecl.dataType;
    llvm::Type* llvmType = CryoTypes->getLLVMType(varType);

    llvm::Value* var = nullptr;
    if (node->data.varDecl.isGlobal) {
        llvm::Constant* initialValue = nullptr;
        if (node->data.varDecl.initializer) {
            initialValue = (llvm::Constant*)generateExpression(node->data.varDecl.initializer);
        }
        var = createGlobalVariable(llvmType, initialValue, varName);
    } else {
        var = builder.CreateAlloca(llvmType, nullptr, varName);
    }

    namedValues[varName] = var;
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
