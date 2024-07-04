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



// <declareFunctions>
bool declareFunctions(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    bool mainFunctionExists = false;

    if (!node) {
        std::cerr << "[CPP] Error in declareFunctions: AST node is null\n";
        return mainFunctionExists;
    }

    if (node->type == CryoNodeType::NODE_FUNCTION_DECLARATION) {
        if (std::string(node->data.functionDecl.name) == "main") {
            mainFunctionExists = true;
        }
        generateFunctionPrototype(node, builder, module);
    }

    // Recursively declare functions for all child nodes
    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            for (int i = 0; i < node->data.program.stmtCount; ++i) {
                if (declareFunctions(node->data.program.statements[i], builder, module)) {
                    mainFunctionExists = true;
                }
            }
            break;

        case CryoNodeType::NODE_BLOCK:
            for (int i = 0; i < node->data.block.stmtCount; ++i) {
                if (declareFunctions(node->data.block.statements[i], builder, module)) {
                    mainFunctionExists = true;
                }
            }
            break;

        default:
            break;
    }
    return mainFunctionExists;
}
// </declareFunctions>

// <generateFunctionPrototype>
void generateFunctionPrototype(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating function prototype: " << node->data.functionDecl.name << "\n";

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
        case DATA_TYPE_STRING:
            returnType = llvm::Type::getInt8Ty(module.getContext())->getPointerTo();
            break;
        case DATA_TYPE_VOID:
            returnType = llvm::Type::getVoidTy(module.getContext());
            break;
        default:
            std::cerr << "[CPP] Error: Unknown function return type: " << node->data.functionDecl.returnType << "\n";
            return;
    }

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
    llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->data.functionDecl.name, &module);
}
// </generateFunctionPrototype>


// <createDefaultMainFunction>
void createDefaultMainFunction(llvm::IRBuilder<>& builder, llvm::Module& module) {
    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(module.getContext()), false);
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "_defaulted", &module);
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(module.getContext(), "entry", function);
    
    std::cout << "[CPP] Created basic block for default main function\n";

    builder.SetInsertPoint(entry);
    // builder.CreateRetVoid();
    
    std::cout << "[CPP] Added return statement to default main function\n";
}
// </createDefaultMainFunction>


// <generateFunctionCall>
void generateFunctionCall(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::string functionName = node->data.functionCall.name;
    std::vector<llvm::Value*> args;

    for (int i = 0; i < node->data.functionCall.argCount; ++i) {
        args.push_back(generateExpression(node->data.functionCall.args[i], builder, module));
    }

    llvm::Function* callee = module.getFunction(functionName);
    if (!callee) {
        std::cerr << "Unknown function referenced: " << functionName << "\n";
        return;
    }

    builder.CreateCall(callee, args);
}
// </generateFunctionCall>


// <generateReturnStatement>
void generateReturnStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    llvm::Value* retValue = nullptr;
    if (node->data.returnStmt.returnValue) {
        retValue = generateExpression(node->data.returnStmt.returnValue, builder, module);
    }

    llvm::BasicBlock* currentBlock = builder.GetInsertBlock();
    llvm::Function* currentFunction = currentBlock->getParent();
    llvm::Type* returnType = currentFunction->getReturnType();

    if (!returnType) {
        returnType = llvm::Type::getVoidTy(module.getContext());
    }

    if (returnType->isVoidTy()) {
        builder.CreateRetVoid();
    } else if (retValue) {
        if (returnType != retValue->getType()) {
            // Perform necessary type casting if types do not match
            if (retValue->getType()->isPointerTy() && returnType->isIntegerTy()) {
                retValue = builder.CreatePtrToInt(retValue, returnType);
            } else if (retValue->getType()->isIntegerTy() && returnType->isPointerTy()) {
                retValue = builder.CreateIntToPtr(retValue, returnType);
            } else {
                retValue = builder.CreateBitCast(retValue, returnType);
            }
        }
        builder.CreateRet(retValue);
    } else {
        std::cerr << "Error: Return value expected but not provided\n";
        builder.CreateRet(llvm::UndefValue::get(returnType));
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
        case DATA_TYPE_STRING:
            returnType = llvm::Type::getInt8Ty(module.getContext())->getPointerTo();
            break;
        case DATA_TYPE_VOID:
            returnType = llvm::Type::getVoidTy(module.getContext());
            break;
        default:
            std::cerr << "[CPP] Error: Unknown function return type: " << node->data.functionDecl.returnType << "\n";
            return;
    }

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
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->data.functionDecl.name, &module);

    // Generate a unique basic block name
    std::string basicBlockName = node->data.functionDecl.name;
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(module.getContext(), basicBlockName, function);
    builder.SetInsertPoint(BB);

    if (node->data.functionDecl.body) {
        generateCode(node->data.functionDecl.body, builder, module);
    }

    if (!BB->getTerminator()) {
        if (returnType->isVoidTy()) {
            //builder.CreateRetVoid();
        } else {
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



// <getCryoFunction>
llvm::Function* getCryoFunction(llvm::Module& module, const std::string& name, llvm::ArrayRef<llvm::Type*> argTypes) {
    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(module.getContext()), argTypes, false);

    llvm::Function* func = module.getFunction(name);
    if (!func) {
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    }
    return func;
}
// </getCryoFunction>
