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
    CryoContext& cryoContext = compiler.getContext();

    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(cryoContext.module->getContext()), false);
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "_defaulted", cryoContext.module.get());
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(cryoContext.module->getContext(), "entry", function);

    cryoContext.builder.SetInsertPoint(entry);
    cryoContext.builder.CreateRetVoid();

    std::cout << "[Functions] Created basic block for default main function\n";
}


void CryoSyntax::generateFunctionPrototype(ASTNode* node) {
    if (node == nullptr) {
        std::cerr << "Error: node is null in generateFunctionPrototype." << std::endl;
        return;
    }

    CryoTypes& cryoTypesInstance = compiler.getTypes();
    CryoContext& cryoContext = compiler.getContext();

    char* functionName;
    FunctionDeclNode* functionNode = nullptr;
    if(node->type == NODE_EXTERN_FUNCTION) {
        functionName = node->data.externNode.decl.function->name;
        functionNode = (FunctionDeclNode*)node->data.externNode.decl.function;
    } else if (node->type == NODE_FUNCTION_DECLARATION) {
        functionName = node->data.functionDecl.function->name;
        functionNode = (FunctionDeclNode*)node->data.functionDecl.function;
    } else {
        std::cerr << "[Functions] Error: Invalid function node\n";
        return;
    }

    if (!functionName || !functionNode) {
        std::cerr << "[Functions] Error: Function name or function node is null\n";
        return;
    }

    std::cout << "[Functions] Generating function prototype for " << functionName << std::endl;
    CryoDataType ASTDataType = functionNode->returnType;
    std::cout << "[Functions] Function return type: " << CryoDataTypeToString(ASTDataType) << std::endl;
    llvm::Type* returnType = cryoTypesInstance.getLLVMType(ASTDataType);
    if (!returnType) {
        std::cerr << "[Functions] Error: Failed to get LLVM return type\n";
        return;
    }
    if (!returnType) {
        std::cerr << "[Functions] Error: returnType is null\n";
        return;
    }
    std::cout << "[Functions] LLVM return type: " << returnType << std::endl;

    std::vector<llvm::Type*> paramTypes;
    if (!functionNode || !functionNode->params) {
        std::cerr << "[Functions] Error: functionNode or functionNode->params is null\n";
        return;
    }
    for (int i = 0; i < functionNode->paramCount; i++) {
        std::cout << "[Functions] Getting param " << i << std::endl;
        if(functionNode->params[i]->data.varDecl.dataType) {
            std::cout << "[Functions] Getting params...\n";
            CryoDataType paramType = functionNode->params[i]->data.varDecl.dataType;
            llvm::Type* llvmType = cryoTypesInstance.getLLVMType(paramType);
            if (!llvmType) {
                std::cerr << "[Functions] Error: Failed to get LLVM type for parameter " << i << std::endl;
                return;
            }
            paramTypes.push_back(llvmType);
        } else {
            std::cerr << "[Functions] Error: Parameter data type is null for parameter " << i << std::endl;
            return;
        }
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    if (!funcType) {
        std::cerr << "[Functions] Error: Failed to create LLVM function type\n";
        return;
    }

    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, cryoContext.module.get());
    if (!function) {
        std::cerr << "[Functions] Error: Failed to create LLVM function\n";
        return;
    }
    std::cout << "[Functions] Successfully generated function prototype for " << functionName << std::endl;
}

void CryoSyntax::generateFunction(ASTNode* node) {
    CryoContext& cryoContext = compiler.getContext();
    CryoTypes& cryoTypesInstance = compiler.getTypes();

    char* functionName;
    FunctionDeclNode* functionNode = nullptr;

    if (node->type == NODE_EXTERN_STATEMENT) {
        functionName = node->data.externNode.decl.function->name;
        functionNode = node->data.externNode.decl.function;
    } else if (node->type == NODE_FUNCTION_DECLARATION) {
        functionName = node->data.functionDecl.function->name;
        functionNode = node->data.functionDecl.function;
    } else {
        std::cerr << "[Functions] Error: Invalid function node\n";
        return;
    }

    llvm::Function* function = cryoContext.module->getFunction(functionName);
    if (!function) {
        // Generate the function prototype if it doesn't exist
        generateFunctionPrototype(node);
        auto function = cryoContext.module->getFunction(functionName);
        if (!function) {
            std::cerr << "[Functions] Error: Failed to generate function prototype\n";
            return;
        }
    }

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(cryoContext.module->getContext(), "entry", function);
    std::cout << "[Functions] Generating function " << functionName << std::endl;
    cryoContext.builder.SetInsertPoint(entry);
    std::cout << "[Functions] Set insert point for function " << functionName << std::endl;

    // Create a new scope for the function
    // cryoContext.namedValues.clear();
    std::vector<llvm::Type*> paramTypes;
    for (int i = 0; i < functionNode->paramCount; i++) {
        llvm::Argument* arg = function->arg_begin() + i;
        arg->setName(functionNode->params[i]->data.varDecl.name);
        cryoContext.namedValues[functionNode->params[i]->data.varDecl.name] = arg;
        paramTypes.push_back(arg->getType());
    }

    llvm::Type* returnType = cryoTypesInstance.getLLVMType(node->data.functionDecl.function->returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    if (!function) {
        function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, cryoContext.module.get());
    } else {
        std::cerr << "[CPP] Warning: Function " << functionName << " already exists in the module\n";
    }

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(cryoContext.module->getContext(), "entry", function);
    cryoContext.builder.SetInsertPoint(BB);

    // Generate code for the function body
    if (node->data.functionDecl.function->body) {
        generateFunctionBlock(node->data.functionDecl.function->body);
    }

    // Ensure the function has a return statement or terminator
    if (!BB->getTerminator()) {
        if (returnType->isVoidTy()) {
            cryoContext.builder.CreateRetVoid();
        } else {
            cryoContext.builder.CreateRet(llvm::Constant::getNullValue(returnType));
        }
    }
    return;
}

void CryoSyntax::generateExternalDeclaration(ASTNode* node) {
    if (node->type == NODE_EXTERN_FUNCTION) {
        generateFunctionPrototype(node);
    } else {
        std::cerr << "[Functions] Error: Invalid external declaration node\n";
    }

    generateFunctionPrototype(node);

    return;
}

void CryoSyntax::generateReturnStatement(ASTNode* node) {
    CryoContext& cryoContext = compiler.getContext();
    if (!node->data.returnStmt.expression) {
        cryoContext.builder.CreateRetVoid();
        return;
    }

    llvm::Value* returnValue = generateExpression(node->data.returnStmt.expression);
    cryoContext.builder.CreateRet(returnValue);

    return;
}

void CryoSyntax::generateFunctionCall(ASTNode* node) {
    CryoContext& cryoContext = compiler.getContext();
    
    llvm::Function* function = cryoContext.module->getFunction(node->data.functionCall.name);
    if (!function) {
        std::cerr << "[Functions] Error: Function not found in module @generateFunctionCall\n";
        return;
    }

    std::vector<llvm::Value*> args;
    for (int i = 0; i < node->data.functionCall.argCount; i++) {
        args.push_back(generateExpression(node->data.functionCall.args[i]));
    }

    llvm::Value* call = cryoContext.builder.CreateCall(function, args);
    std::cout << "[Functions] Generated function call to " << node->data.functionCall.name << std::endl;

    return;
}

void CryoSyntax::generateFunctionBlock(ASTNode* node) {
    if (!node) {
        std::cerr << "[Functions] Error: Node is null in generateFunctionBlock\n";
        return;
    }

    CryoContext& cryoContext = compiler.getContext();

    ASTNode** statements = node->data.block.statements;
    for (int i = 0; i < node->data.block.stmtCount; i++) {
        identifyNodeExpression(statements[i]);
    }

    return;
}




} // namespace Cryo
