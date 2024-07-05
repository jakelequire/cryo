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

// <getLLVMType>
llvm::Type* getLLVMType(CryoDataType type, llvm::Module& module) {
    switch (type) {
        case DATA_TYPE_INT:
            return llvm::Type::getInt32Ty(module.getContext());
        case DATA_TYPE_FLOAT:
            return llvm::Type::getFloatTy(module.getContext());
        case DATA_TYPE_BOOLEAN:
            return llvm::Type::getInt1Ty(module.getContext());
        case DATA_TYPE_STRING:
            return llvm::Type::getInt8Ty(module.getContext())->getPointerTo();
        case DATA_TYPE_VOID:
            return llvm::Type::getVoidTy(module.getContext());
        default:
            std::cerr << "[CPP] Error: Unknown data type\n";
            return nullptr;
    }
}
// </getLLVMType>


// <generateFunctionPrototype>
void generateFunctionPrototype(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating function prototype: " << node->data.functionDecl.name << "\n";

    llvm::Type* returnType = getLLVMType(node->data.functionDecl.returnType, module);

    std::vector<llvm::Type*> paramTypes;
    if (node->data.functionDecl.params) {
        for (int i = 0; i < node->data.functionDecl.paramCount; ++i) {
            llvm::Type* paramType = getLLVMType(node->data.functionDecl.params->data.paramList.params[i]->data.varDecl.dataType, module);
            if (!paramType) {
                std::cerr << "[CPP] Error: Unknown parameter type\n";
                return;
            }
            paramTypes.push_back(paramType);
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
    builder.CreateRetVoid();
    
    std::cout << "[CPP] Added return statement to default main function\n";
}
// </createDefaultMainFunction>


// <generateFunctionCall>
void generateFunctionCall(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::string functionName = node->data.functionCall.name;
    std::vector<llvm::Value*> args;

    for (int i = 0; i < node->data.functionCall.argCount; ++i) {
        llvm::Value* arg = generateExpression(node->data.functionCall.args[i], builder, module);
        if (!arg) {
            std::cerr << "[CPP] Error generating argument for function call\n";
            return;
        }
        args.push_back(arg);
    }

    llvm::Function* callee = module.getFunction(functionName);
    if (!callee) {
        std::cout << "[CPP] Unknown function referenced: " << functionName << "\n";
        return;
    }

    llvm::CallInst* call = builder.CreateCall(callee, args);
    std::cout << "[CPP] Created function call for " << functionName << "\n";

    llvm::BasicBlock* currentBlock = builder.GetInsertBlock();
    llvm::Function* parentFunction = currentBlock->getParent();
    if (parentFunction == callee) {
        std::cout << "[CPP] Function call is in the same function\n";
    } else {
        std::cout << "[CPP] Function call is in a different function\n";
    }

    // Ensure the current block has a terminator if it is not a void function
    if (!callee->getReturnType()->isVoidTy()) {
        if (!currentBlock->getTerminator()) {
            builder.CreateRet(call);
            std::cout << "[CPP] Added return statement after function call\n";
        }
    }
}
// </generateFunctionCall>


// <generateFunction>
void generateFunction(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for function: " << node->data.functionDecl.name << "\n";

    llvm::Type* returnType = getLLVMType(node->data.functionDecl.returnType, module);

    std::vector<llvm::Type*> paramTypes;
    if (node->data.functionDecl.params) {
        for (int i = 0; i < node->data.functionDecl.paramCount; ++i) {
            llvm::Type* paramType = getLLVMType(node->data.functionDecl.params->data.paramList.params[i]->data.varDecl.dataType, module);
            if (!paramType) {
                std::cerr << "[CPP] Error: Unknown parameter type\n";
                return;
            }
            paramTypes.push_back(paramType);
        }
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->data.functionDecl.name, &module);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(module.getContext(), "entry", function);
    builder.SetInsertPoint(BB);

    // Generate code for the function body
    if (node->data.functionDecl.body) {
        generateCode(node->data.functionDecl.body, builder, module);
    }

    // Ensure the function has a return statement or terminator
    if (!BB->getTerminator()) {
        if (returnType->isVoidTy()) {
            builder.CreateRetVoid();
        } else {
            // builder.CreateRet(llvm::Constant::getNullValue(returnType));
        }
    }
    std::cout << "[CPP] Generated function: " << node->data.functionDecl.name << "\n";
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
        // Notes:
        // Removing this will make function calls work.
        // Keeping it will make regular functions work.
        builder.CreateRet(retValue);    // <--- This is the line that causes the error
    } else {
        std::cerr << "Error: Return value expected but not provided\n";
        builder.CreateRet(llvm::UndefValue::get(returnType));
    }
}
// </generateReturnStatement>



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
