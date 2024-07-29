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
    if(node->type == NODE_EXTERN_FUNCTION) {
        functionName = node->data.externNode.decl.function->name;
    } else if (node->type == NODE_FUNCTION_DECLARATION) {
        functionName = node->data.functionDecl.function->name;
    } else {
        std::cerr << "[CPP] Error: Invalid function node\n";
        return;
    }
    
    CryoDataType ASTDataType = node->data.functionDecl.function->returnType;
    llvm::Type* returnType = CryoTypes->getLLVMType(ASTDataType);
    std::vector<llvm::Type*> paramTypes;

    // TODO
}

void CryoSyntax::generateFunction(ASTNode* node) {

}

void CryoSyntax::generateExternalDeclaration(ASTNode* node) {

}

void CryoSyntax::generateReturnStatement(ASTNode* node) {

}

void CryoSyntax::generateFunctionCall(ASTNode* node) {

}

void CryoSyntax::generateFunctionBlock(ASTNode* node) {

}




} // namespace Cryo
