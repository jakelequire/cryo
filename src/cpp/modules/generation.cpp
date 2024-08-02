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


void CryoModules::generateProgram(ASTNode* node) {
    CodeGen& codeGenInstance = compiler.getCodeGen();
    CryoSyntax& cryoSyntaxInstance = compiler.getSyntax();

    std::cout << "[Generation] Generating code for program\n";
    if (node->data.program.stmtCount == 0) {
        std::cerr << "[Generation] Error generating code for program: No statements found\n";
        return;
    }
    for (int i = 0; i < node->data.program.stmtCount; ++i) {
        std::cout << "[Generation] Generating code for program statement " << i << "\n";
        cryoSyntaxInstance.identifyNodeExpression(node->data.program.statements[i]);
        std::cout << "[Generation] Moving to next statement\n";
    }
    return;
}



void CryoModules::generateBlock(ASTNode* node) {
    if(!node) {
        std::cerr << "[Generation] Error: Failed to generate Block Node. \n";
    }
    std::cout << "[Generation] Generating code for Block Node.\n";
    CodeGen& codeGenInstance = compiler.getCodeGen();
    CryoSyntax& cryoSyntaxInstance = compiler.getSyntax();


    for(int i = 0; i < node->data.block.stmtCount; i++) {
        std::cout << "[Generation] Generating Code for Block Statement...\n";
        cryoSyntaxInstance.identifyNodeExpression(node->data.block.statements[i]);
    }

    return;
}



void CryoModules::generateFunctionBlock(ASTNode* node) {
    if(!node || !node->data.functionBlock.block) {
        std::cerr << "[Generation] Error: Failed to generate Function Block. @<generateFunctionBlock>\n";
        return;
    }
    std::cout << "[Generation] Generating code for Function Block Node.\n";
    CodeGen& codeGenInstance = compiler.getCodeGen();
    CryoContext& cryoContext = compiler.getContext();
    CryoTypes& cryoTypesInstance = compiler.getTypes();
    CryoSyntax& cryoSyntaxInstance = compiler.getSyntax();

    
    for(int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; ++i) {
        std::cout << "[Generation] Generating code for Block Statement. Count: " << i << "\n";
        cryoSyntaxInstance.identifyNodeExpression(node->data.functionBlock.block->data.block.statements[i]);
        std::cout << "[Generation] Moving to next statement...\n";
    }

    llvm::BasicBlock* BB = cryoContext.builder.GetInsertBlock();
    if(!BB->getTerminator()) {
        cryoContext.builder.CreateRetVoid();
    }

    std::cout << "[Generation] Function Block code generation complete!\n";
}



void CryoModules::generateExternalDeclaration(ASTNode* node) {
    if(!node) {
        std::cerr << "[Generation] Error: Extern Declaration Node is null.\n";
        return;
    }

    CryoContext& cryoContext = compiler.getContext();
    CryoTypes& cryoTypesInstance = compiler.getTypes();

    char* functionName = node->data.externNode.decl.function->name;
    CryoDataType returnTypeData = node->data.externNode.decl.function->returnType;

    llvm::Type *returnType = cryoTypesInstance.getLLVMType(returnTypeData);
    std::vector<llvm::Type*> paramTypes;

    for (int i = 0; i < node->data.externNode.decl.function->paramCount; ++i) {
        ASTNode* parameter = node->data.externNode.decl.function->params[i];
        llvm::Type* paramType;
        if (parameter == nullptr) {
            std::cerr << "[Generation] Error: Extern Parameter is null\n";
            break;
        }
        if(parameter->data.varDecl.dataType == DATA_TYPE_INT) {
            paramType = cryoTypesInstance.getLLVMType(DATA_TYPE_INT);
            paramTypes.push_back(paramType);
            break;
        }
        if(parameter->data.varDecl.dataType == DATA_TYPE_STRING) {
            paramType = cryoTypesInstance.getLLVMType(DATA_TYPE_STRING);
            paramTypes.push_back(paramType);
            break;
        }
        if (!paramType) {
            std::cerr << "[Generation] Error: Extern Unknown parameter type\n";
            return;
        }
    }
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes[0], false);

    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, cryoContext.module.get());
}


} // namespace Cryo
