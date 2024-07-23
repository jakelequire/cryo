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

llvm::Type* CodeGen::getLLVMType(CryoDataType type) {
    std::cout << "[CPP] Getting LLVM type for: " << CryoDataTypeToString(type) << "\n";
    switch (type) {
        case DATA_TYPE_INT:
            return llvm::Type::getInt32Ty(module->getContext());
        case DATA_TYPE_FLOAT:
            return llvm::Type::getFloatTy(module->getContext());
        case DATA_TYPE_STRING:
            return llvm::Type::getInt8Ty(module->getContext())->getPointerTo();
        case DATA_TYPE_BOOLEAN:
            return llvm::Type::getInt1Ty(module->getContext());
        case DATA_TYPE_VOID:
            return llvm::Type::getVoidTy(module->getContext());
        case DATA_TYPE_NULL:
            return llvm::Type::getInt8Ty(module->getContext())->getPointerTo();
        case DATA_TYPE_INT_ARRAY:
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(module->getContext()), 0);
        case DATA_TYPE_FLOAT_ARRAY:
            return llvm::ArrayType::get(llvm::Type::getFloatTy(module->getContext()), 0);
        case DATA_TYPE_STRING_ARRAY:
            return llvm::ArrayType::get(llvm::Type::getInt8Ty(module->getContext())->getPointerTo(), 0);
        case DATA_TYPE_BOOLEAN_ARRAY:
            return llvm::ArrayType::get(llvm::Type::getInt1Ty(module->getContext()), 0);
        case DATA_TYPE_VOID_ARRAY:
            return llvm::ArrayType::get(llvm::Type::getVoidTy(module->getContext()), 0);
        default:
            std::cerr << "[CPP] Error: Unknown data type\n";
            return nullptr;
    }
}

void CodeGen::generateFunctionPrototype(ASTNode* node) {
    std::cout << "[CPP] Generating function prototype: " << node->data.functionDecl.function->name << "\n";
    std::cout << "[CPP] Function return type: " << node->data.functionDecl.function->returnType << "\n";
    std::cout << "[CPP] Function Type: " << CryoNodeTypeToString(node->data.functionDecl.type) << "\n";

    char* functionName;
    if(node->type == NODE_EXTERN_STATEMENT) {
        functionName = node->data.externNode.decl.function->name;
    } else if (node->type == NODE_FUNCTION_DECLARATION) {
        functionName = node->data.functionDecl.function->name;
    } else {
        std::cerr << "[CPP] Error: Invalid function node\n";
        return;
    }

    llvm::Type* returnType = getLLVMType(node->data.functionDecl.function->returnType);

    std::vector<llvm::Type*> paramTypes;
    if(node->type == NODE_EXTERN_FUNCTION) {
        std::cout << "[CPP] Generating external function prototype\n";
        for (int i = 0; i < node->data.externNode.decl.function->paramCount - 1; ++i) {
            ASTNode* parameter = node->data.externNode.decl.function->params[i];
            if (!parameter) {
                std::cerr << "[CPP] Error: Null parameter node\n";
                return;
            }
            std::cout << "[CPP] Generating parameter: " << CryoNodeTypeToString(parameter->type) << "\n";
            std::cout << "[CPP] Generating parameter type: " << CryoDataTypeToString(parameter->data.varDecl.dataType) << "\n";

            llvm::Type* paramType = getLLVMType(parameter->data.varDecl.dataType);
            if (!paramType) {
                std::cerr << "[CPP] Error: Unknown parameter type\n";
                return;
            }
            paramTypes.push_back(paramType);
            std::cout << "[CPP] Parameter type added to list\n";
        }
    }

    if (node->data.functionDecl.function->params) {
        std::cout << "[CPP] Generating function parameters\n";
        for (int i = 0; i < node->data.functionDecl.function->paramCount; ++i) {
            ASTNode* parameter = node->data.functionDecl.function->params[i];
            if (!parameter) {
                std::cerr << "[CPP] Error: Null parameter node\n";
                return;
            }
            llvm::Type* paramType = getLLVMType(parameter->data.varDecl.dataType);
            if (!paramType) {
                std::cerr << "[CPP] Error: Unknown parameter type\n";
                return;
            }
            paramTypes.push_back(paramType);
        }
    }

    std::cout << "[CPP] Function parameter types generated\n";
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    std::cout << "[CPP] Function type generated\n";

    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, module.get());
    std::cout << "[CPP] Function created\n";

    std::cout << "[CPP] Function prototype generated: " << functionName << "\n";
}


void CodeGen::createDefaultMainFunction() {
    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(module->getContext()), false);
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "_defaulted", module.get());
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(module->getContext(), "entry", function);
    
    std::cout << "[CPP] Created basic block for default main function\n";

    builder.SetInsertPoint(entry);
    builder.CreateRetVoid();
    
    std::cout << "[CPP] Added return statement to default main function\n";
}

void CodeGen::generateFunctionCall(ASTNode* node) {
    std::string functionName = node->data.functionCall.name;
    std::vector<llvm::Value*> args;

    for (int i = 0; i < node->data.functionCall.argCount; ++i) {
        llvm::Value* arg = generateExpression(node->data.functionCall.args[i]);
        if (!arg) {
            std::cerr << "[CPP] Error generating argument for function call\n";
            return;
        }
        args.push_back(arg);
    }

    std::cout << "[CPP] Generating function call for " << functionName << "\n";
    llvm::Function* callee = module->getFunction(functionName);
    if (!callee) {
        std::cerr << "[CPP] Error: Unknown function referenced: " << functionName << "\n";
        return;
    }

    builder.CreateCall(callee, args);
    std::cout << "[CPP] Created function call for " << functionName << "\n";
}


void CodeGen::generateFunction(ASTNode* node) {
    char* functionName;
    if(node->type == NODE_EXTERN_STATEMENT) {
        functionName = node->data.externNode.decl.function->name;
    } else if (node->type == NODE_FUNCTION_DECLARATION) {
        functionName = node->data.functionDecl.function->name;
    } else {
        std::cerr << "[CPP] Error: Invalid function node\n";
        return;
    }
    std::cout << "[CPP] Generating code for function NAME: " << functionName << "\n";

    llvm::Type* returnType = getLLVMType(node->data.functionDecl.function->returnType);

    std::cout << "[CPP] Function return type: " << returnType << "\n";

    std::vector<llvm::Type*> paramTypes;
    if (node->data.functionDecl.function->params) {
        for (int i = 0; i < node->data.functionDecl.function->paramCount; ++i) {
            if(i % 2 == 0) {
                return;
            }
            ASTNode* parameterType = node->data.functionDecl.function->params[i];
            std::cout << "[CPP - DEBUG] Parameter type: " << parameterType->data.varDecl.dataType << "\n";
            CryoDataType parameterTypeData = parameterType->data.varDecl.dataType;
            std:: cout << "[CPP - DEBUG] Parameter type data: " << parameterTypeData << "\n";
            llvm::Type* paramType = getLLVMType(parameterTypeData);
            if (!paramType) {
                std::cerr << "[CPP] Error: Unknown parameter type\n";
                return;
            }
            paramTypes.push_back(paramType);
        }
    }

    std::cout << "[CPP] Function parameter types generated\n";

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function* function = module->getFunction(functionName);
    if (!function) {
        function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, module.get());
    } else {
        std::cerr << "[CPP] Warning: Function " << functionName << " already exists in the module\n";
    }

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(module->getContext(), "entry", function);
    builder.SetInsertPoint(BB);

    // Generate code for the function body
    if (node->data.functionDecl.function->body) {
        generateCode(node->data.functionDecl.function->body);
    }

    // Ensure the function has a return statement or terminator
    if (!BB->getTerminator()) {
        if (returnType->isVoidTy()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(llvm::Constant::getNullValue(returnType));
        }
    }
    std::cout << "[CPP] Generated function: " << functionName << "\n";
}

void CodeGen::generateFunctionBlock(ASTNode* node) {
    std::cout << "[CPP] Generating code for function block\n";
    if (!node || !node->data.functionBlock.block) {
        std::cerr << "[CPP] Error generating code for function block: Invalid function block\n";
        return;
    }
    std::cout << "[CPP] Function block contains " << node->data.functionBlock.block->data.block.stmtCount << " statements\n";

    // Generate code for each statement in the block
    for (int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; ++i) {
        std::cout << "[CPP] Generating code for block statement " << i << "\n";
        generateCode(node->data.functionBlock.block->data.block.statements[i]);
        std::cout << "[CPP] Moving to next statement\n";
    }

    llvm::BasicBlock* BB = builder.GetInsertBlock();
    if (!BB->getTerminator()) {
        builder.CreateRetVoid();
    }

    std::cout << "[CPP] Function block code generation complete\n";
}

void CodeGen::generateReturnStatement(ASTNode* node) {
    llvm::Value* retValue = nullptr;
    if (node->data.returnStmt.returnValue) {
        retValue = generateExpression(node->data.returnStmt.returnValue);
    }

    llvm::BasicBlock* currentBlock = builder.GetInsertBlock();
    llvm::Function* currentFunction = currentBlock->getParent();
    llvm::Type* returnType = currentFunction->getReturnType();

    if (!returnType) {
        returnType = llvm::Type::getVoidTy(module->getContext());
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
        builder.CreateRet(retValue);    // <--- This is the line that causes the error
    } else {
        std::cerr << "Error: Return value expected but not provided\n";
        builder.CreateRet(llvm::UndefValue::get(returnType));
    }
}

void CodeGen::generateExternalDeclaration(ASTNode* node) {
    if (node->type != NODE_EXTERN_STATEMENT) {
        std::cerr << "[CPP] Error: Extern Node is not an extern statement\n";
        return;
    }

    std::cout << "[CPP] Generating external function declaration\n";
    char* functionName = node->data.externNode.decl.function->name;
    std::cout << "[CPP] Extern Function name: " << functionName << "\n";
    CryoDataType returnTypeData = node->data.externNode.decl.function->returnType;
    std::cout << "[CPP] Extern Function return type: " << returnTypeData << "\n";

    llvm::Type* returnType = getLLVMType(returnTypeData);
    std::cout << "[CPP] LLVM Extern Function return type: " << returnType << "\n";
    std::vector<llvm::Type*> paramTypes;
    std::cout << "[CPP] Extern Function parameter count: " << node->data.externNode.decl.function->paramCount << "\n";

    for (int i = 0; i < node->data.externNode.decl.function->paramCount; ++i) {
        ASTNode* parameter = node->data.externNode.decl.function->params[i];
        llvm::Type* paramType;
        if (parameter == nullptr) {
            std::cerr << "[CPP] Error: Extern Parameter is null\n";
            break;
        }
        if(parameter->data.varDecl.dataType == DATA_TYPE_INT) {
            std::cout << "[CPP] Extern <INT> Parameter: " << i << " type: " << parameter->data.varDecl.dataType << "\n";
            paramType = llvm::Type::getInt32Ty(module->getContext());
            paramTypes.push_back(paramType);
            break;
        }
        if(parameter->data.varDecl.dataType == DATA_TYPE_STRING) {
            std::cout << "[CPP] Extern <STRING> Parameter: " << i << " type: " << parameter->data.varDecl.dataType << "\n";
            paramType = getLLVMType(DATA_TYPE_STRING);
            paramTypes.push_back(paramType);
            break;
        }
        if (!paramType) {
            std::cerr << "[CPP] Error: Extern Unknown parameter type\n";
            return;
        }
    }

    std::cout << "[CPP] Extern Function parameter types generated\n";

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes[0], false);
    std::cout << "[CPP] Extern Function type generated\n";
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, module.get());
    std::cout << "[CPP] Extern Function created\n";
    std::cout << "[CPP] External function declaration generated: " << functionName << "\n";
}

llvm::Function* CodeGen::getCryoFunction(char* name, llvm::ArrayRef<llvm::Type*> argTypes) {
    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(module->getContext()), argTypes, false);

    llvm::Function* func = module->getFunction(name);
    if (!func) {
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module.get());
    }
    return func;
}

} // namespace Cryo
