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


void CryoSyntax::createDefaultMainFunction() {
    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(module->getContext()), false);
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "_defaulted", module.get());
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(module->getContext(), "entry", function);

    builder.SetInsertPoint(entry);
    builder.CreateRetVoid();

    std::cout << "[CPP] Created basic block for default main function\n";
}

void CryoSyntax::generateFunctionPrototype(ASTNode* node) {
    char* functionName;
    FunctionDeclNode* functionNode = nullptr;

    if(node->type == NODE_EXTERN_FUNCTION) {
        functionName = node->data.externNode.decl.function->name;
        functionNode = node->data.externNode.decl.function;
    } else if (node->type == NODE_FUNCTION_DECLARATION) {
        functionName = node->data.functionDecl.function->name;
        functionNode = node->data.functionDecl.function;
    } else {
        std::cerr << "[CPP] Error: Invalid function node\n";
        return;
    }

    std::cout << "[CPP] Generating function prototype for " << functionName << std::endl;

    CryoDataType ASTDataType = functionNode->returnType;
    std::cout << "[CPP] Function return type: " << CryoDataTypeToString(ASTDataType) << std::endl;
    llvm::Type* returnType = CryoTypes->getLLVMType(ASTDataType);
    std::cout << "[CPP] LLVM return type: " << returnType->getTypeID() << std::endl;
    std::vector<llvm::Type*> paramTypes;

    std::cout << "[CPP] Function has " << functionNode->paramCount << " parameters\n";

    for (int i = 0; i < functionNode->paramCount; i++) {
        CryoDataType paramType = functionNode->params[i]->data.varDecl.dataType;
        llvm::Type* llvmType = CryoTypes->getLLVMType(paramType);
        paramTypes.push_back(llvmType);
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, module.get());

    if (node->type == NODE_EXTERN_FUNCTION) {
        namedValues[functionName] = function;
    }

    std::cout << "[CPP] Created function prototype for " << functionName << std::endl;
}

void CryoSyntax::generateFunction(ASTNode* node) {
    char* functionName;
    FunctionDeclNode* functionNode = nullptr;

    if (node->type == NODE_EXTERN_STATEMENT) {
        functionName = node->data.externNode.decl.function->name;
        functionNode = node->data.externNode.decl.function;
    } else if (node->type == NODE_FUNCTION_DECLARATION) {
        functionName = node->data.functionDecl.function->name;
        functionNode = node->data.functionDecl.function;
    } else {
        std::cerr << "[CPP] Error: Invalid function node\n";
        return;
    }

    llvm::Function* function = module->getFunction(functionName);
    if (!function) {
        std::cerr << "[CPP] Error: Function not found in module\n";
        return;
    }

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(module->getContext(), "entry", function);
    builder.SetInsertPoint(entry);

    // Create a new scope for the function
    namedValues.clear();

    for (int i = 0; i < functionNode->paramCount; i++) {
        llvm::Argument* arg = function->arg_begin() + i;
        arg->setName(functionNode->params[i]->data.varDecl.name);
        namedValues[functionNode->params[i]->data.varDecl.name] = arg;
    }

    generateFunctionBlock(functionNode->body);

    llvm::verifyFunction(*function);
    std::cout << "[CPP] Generated function " << functionName << std::endl;

    if (node->type == NODE_EXTERN_STATEMENT) {
        namedValues.erase(functionName);
    }

    return;
}

void CryoSyntax::generateExternalDeclaration(ASTNode* node) {
    if (node->type == NODE_EXTERN_FUNCTION) {
        generateFunctionPrototype(node);
    } else {
        std::cerr << "[CPP] Error: Invalid external declaration node\n";
    }

    return;
}

void CryoSyntax::generateReturnStatement(ASTNode* node) {
    if (!node->data.returnStmt.expression) {
        builder.CreateRetVoid();
        return;
    }

    llvm::Value* returnValue = generateExpression(node->data.returnStmt.expression);
    builder.CreateRet(returnValue);

    return;
}

void CryoSyntax::generateFunctionCall(ASTNode* node) {
    llvm::Function* function = module->getFunction(node->data.functionCall.name);
    if (!function) {
        std::cerr << "[CPP] Error: Function not found in module\n";
        return;
    }

    std::vector<llvm::Value*> args;
    for (int i = 0; i < node->data.functionCall.argCount; i++) {
        args.push_back(generateExpression(node->data.functionCall.args[i]));
    }

    llvm::Value* call = builder.CreateCall(function, args);
    std::cout << "[CPP] Generated function call to " << node->data.functionCall.name << std::endl;

    return;
}

void CryoSyntax::generateFunctionBlock(ASTNode* node) {
    for (int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; i++) {
        generateStatement(node->data.functionBlock.block->data.block.statements[i]);
    }

    return;
}




} // namespace Cryo
