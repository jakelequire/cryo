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






// <generateReturnStatement>
void generateReturnStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    if (!node) {
        std::cerr << "[CPP] Error: Return statement node is null\n";
        return;
    }

    llvm::Value* retValue = nullptr;
    if (node->data.returnStmt.returnValue) {
        std::cout << "[CPP] Return statement node has return value of type: " << node->data.returnStmt.returnValue->type << "\n";
        retValue = generateExpression(node->data.returnStmt.returnValue, builder, module);
    }

    if (!retValue) {
        std::cerr << "[CPP] Error: Return statement value is null\n";
        builder.CreateRetVoid();
        return;
    }

    // If the return value is a variable, we need to load its value
    if (node->data.returnStmt.returnValue->type == CryoNodeType::NODE_VAR_NAME) {
        llvm::Type* retType = retValue->getType();
        llvm::PointerType* retPtr = PointerType::get(retType, 0);
        retValue = builder.CreateLoad(retPtr, retValue);
    }

    std::cout << "[CPP] Return Statement Node with return value generated\n";

    if (retValue) {
        std::cout << "[CPP] Return value: " << retValue << "\n"; // Print the value for debugging
        builder.CreateRet(retValue);
    } else {
        std::cerr << "[CPP] Error: Failed to generate return value\n";
        builder.CreateRetVoid();
    }
}
// </generateReturnStatement>


// <generateFunction>
void generateFunction(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for function: " << node->data.functionDecl.name << "\n";

    llvm::Type* returnType = nullptr;
    switch (node->data.functionDecl.returnType) {
        case DATA_TYPE_INT:
            returnType = llvm::Type::getInt32Ty(module.getContext());
            break;
        case DATA_TYPE_FLOAT:
            returnType = llvm::Type::getFloatTy(module.getContext());
            break;
        case DATA_TYPE_BOOLEAN:
            returnType = llvm::Type::getInt1Ty(module.getContext());
            break;
        case DATA_TYPE_VOID:
            returnType = builder.getVoidTy();
            break;
        default:
            std::cerr << "[CPP] Error: Unknown function return type: " << node->data.functionDecl.returnType << "\n";
            return;
    }
    std::cout << "[CPP] Function return type: " << node->data.functionDecl.returnType << " mapped to LLVM type: " << returnType << "\n";

    std::vector<llvm::Type*> paramTypes;
    if (node->data.functionDecl.params) {
        for (int i = 0; i < node->data.functionDecl.paramCount; ++i) {
            ASTNode* paramNode = node->data.functionDecl.params->data.paramList.params[i];
            switch (paramNode->data.varDecl.dataType) {
                case DATA_TYPE_INT:
                    paramTypes.push_back(llvm::Type::getInt32Ty(module.getContext()));
                    break;
                case DATA_TYPE_FLOAT:
                    paramTypes.push_back(llvm::Type::getFloatTy(module.getContext()));
                    break;
                case DATA_TYPE_BOOLEAN:
                    paramTypes.push_back(llvm::Type::getInt1Ty(module.getContext()));
                    break;
                default:
                    std::cerr << "[CPP] Error: Unknown parameter type\n";
                    return;
            }
        }
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function::LinkageTypes linkageType = node->data.functionDecl.visibility == CryoVisibilityType::VISIBILITY_PRIVATE ? llvm::Function::PrivateLinkage : llvm::Function::ExternalLinkage;
    llvm::Function* function = llvm::Function::Create(funcType, linkageType, node->data.functionDecl.name, module);

    auto paramIter = function->arg_begin();
    if (node->data.functionDecl.params) {
        for (int i = 0; i < node->data.functionDecl.paramCount; ++i, ++paramIter) {
            ASTNode* paramNode = node->data.functionDecl.params->data.paramList.params[i];
            paramIter->setName(paramNode->data.varDecl.name);
        }
    }

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(module.getContext(), "entry", function);
    builder.SetInsertPoint(BB);

    if (node->data.functionDecl.body) {
        generateCode(node->data.functionDecl.body, builder, module);
    } else {
        std::cerr << "[CPP] Error: Function body is null\n";
        return;
    }

    if (returnType->isVoidTy()) {
        builder.CreateRetVoid();
    } else {
        if (BB->getTerminator() == nullptr) {
            std::cerr << "[CPP] Error: Missing return statement\n";
            builder.CreateRet(llvm::Constant::getNullValue(returnType));
        }
    }
}
// </generateFunction>



// <generateFunctionBlock>
void generateFunctionBlock(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for function block\n";
    if (!node || !node->data.functionBlock.block) {
        std::cerr << "[CPP] Error generating code for function block: Invalid function block\n";
        return;
    }
    std::cout << "[CPP] Function block contains " << node->data.functionBlock.block->data.block.stmtCount << " statements\n";

    // Generate code for each statement in the block
    for (int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; ++i) {
        std::cout << "[CPP] Generating code for block statement " << i << "\n";
        generateCode(node->data.functionBlock.block->data.block.statements[i], builder, module);
        std::cout << "[CPP] Moving to next statement\n";
    }
}
// </generateFunctionBlock>