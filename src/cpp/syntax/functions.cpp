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

namespace Cryo
{

    void CryoSyntax::createDefaultMainFunction()
    {
        CryoContext &cryoContext = compiler.getContext();

        // Check if _defaulted function already exists
        if (cryoContext.module->getFunction("main"))
        {
            std::cout << "[Functions] Main function already exists. Not creating default.\n";
            return;
        }
        if (cryoContext.module->getFunction("_defaulted"))
        {
            std::cout << "[Functions] _defaulted function already exists. Skipping creation.\n";
            return;
        }

        llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(cryoContext.module->getContext()), false);
        llvm::Function *function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "_defaulted", cryoContext.module.get());
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(cryoContext.module->getContext(), "entry", function);

        cryoContext.builder.SetInsertPoint(entry);
        cryoContext.builder.CreateRetVoid();

        std::cout << "[Functions] Created basic block for default main function\n";
    }

    void CryoSyntax::generateFunctionPrototype(ASTNode *node)
    {
        if (node == nullptr)
        {
            std::cerr << "Error: node is null in generateFunctionPrototype." << std::endl;
            return;
        }

        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();

        char *functionName;
        FunctionDeclNode *functionNode = nullptr;
        if (node->type == NODE_EXTERN_FUNCTION)
        {
            functionName = node->data.externNode.decl.function->name;
            functionNode = (node->data.externNode.decl.function);
        }
        else if (node->type == NODE_FUNCTION_DECLARATION)
        {
            functionName = node->data.functionDecl.function->name;
            functionNode = (node->data.functionDecl.function);
        }

        // Check if the function already exists in the module
        if (cryoContext.module->getFunction(functionName))
        {
            std::cerr << "[Functions] Warning: Function " << functionName << " already exists in the module\n";
            return;
        }

        std::vector<llvm::Type *> paramTypes;
        for (int i = 0; i < functionNode->paramCount; i++)
        {
            auto param = functionNode->params[i];
            llvm::Type *llvmType = cryoTypesInstance.getLLVMType(param->data.varDecl.dataType);
            paramTypes.push_back(llvmType);
        }

        llvm::FunctionType *funcType = llvm::FunctionType::get(cryoTypesInstance.getLLVMType(functionNode->returnType), paramTypes, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, cryoContext.module.get());
    }

    void CryoSyntax::generateFunction(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypesInstance = compiler.getTypes();

        char *functionName;
        FunctionDeclNode *functionNode = nullptr;

        if (node->type == NODE_EXTERN_STATEMENT)
        {
            functionName = node->data.externNode.decl.function->name;
            functionNode = node->data.externNode.decl.function;
        }
        else if (node->type == NODE_FUNCTION_DECLARATION)
        {
            functionName = node->data.functionDecl.function->name;
            functionNode = node->data.functionDecl.function;
        }
        else
        {
            std::cerr << "[Functions] Error: Invalid function node\n";
            return;
        }

        llvm::Function *function = cryoContext.module->getFunction(functionName);
        if (!function)
        {
            // Generate the function prototype if it doesn't exist
            generateFunctionPrototype(node);
            auto function = cryoContext.module->getFunction(functionName);
            if (!function)
            {
                std::cerr << "[Functions] Error: Failed to generate function prototype\n";
                return;
            }
        }

        llvm::BasicBlock *entry = llvm::BasicBlock::Create(cryoContext.module->getContext(), "entry", function);
        std::cout << "[Functions] Generating function " << functionName << std::endl;
        cryoContext.builder.SetInsertPoint(entry);
        std::cout << "[Functions] Set insert point for function " << functionName << std::endl;

        // Create a new scope for the function
        // cryoContext.namedValues.clear();
        std::vector<llvm::Type *> paramTypes;
        for (int i = 0; i < functionNode->paramCount; i++)
        {
            llvm::Argument *arg = function->arg_begin() + i;
            arg->setName(functionNode->params[i]->data.varDecl.name);
            cryoContext.namedValues[functionNode->params[i]->data.varDecl.name] = arg;
            paramTypes.push_back(arg->getType());
        }

        llvm::Type *returnType = cryoTypesInstance.getLLVMType(node->data.functionDecl.function->returnType);
        llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes, false);
        if (!function)
        {
            function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, cryoContext.module.get());
        }
        else
        {
            std::cerr << "[CPP] Warning: Function " << functionName << " already exists in the module\n";
        }

        cryoContext.builder.SetInsertPoint(entry);

        // Generate code for the function body
        if (node->data.functionDecl.function->body)
        {
            generateFunctionBlock(node->data.functionDecl.function->body);
        }

        // Ensure the function has a return statement or terminator
        if (!entry->getTerminator())
        {
            if (returnType->isVoidTy())
            {
                cryoContext.builder.CreateRetVoid();
            }
            else
            {
                cryoContext.builder.CreateRet(llvm::Constant::getNullValue(returnType));
            }
        }
        return;
    }

    void CryoSyntax::generateExternalPrototype(ASTNode *node)
    {
        if (node == nullptr)
        {
            std::cerr << "[Functions] Error: Node is null in generateExternalPrototype\n";
            return;
        }

        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();

        char *functionName = node->data.externNode.decl.function->name;
        FunctionDeclNode *functionNode = node->data.externNode.decl.function;

        // Check if the function already exists in the module
        if (cryoContext.module->getFunction(functionName))
        {
            std::cerr << "[Functions] Warning: Function " << functionName << " already exists in the module\n";
            return;
        }

        std::vector<llvm::Type *> paramTypes;

        for (int i = 0; i < functionNode->paramCount; i++)
        {
            llvm::Type *paramType;
            switch (functionNode->params[i]->data.varDecl.dataType)
            {
            case DATA_TYPE_INT:
                if (strstr(functionNode->name, "Ptr") != NULL)
                    paramType = llvm::Type::getInt32Ty(cryoContext.context)->getPointerTo();
                else
                    paramType = llvm::Type::getInt32Ty(cryoContext.context);
                break;
            case DATA_TYPE_STRING:
            {
                paramType = llvm::Type::getInt8Ty(cryoContext.context)->getPointerTo();
                break;
            }
            // Add other types as needed
            default:
                std::cerr << "Unsupported parameter type in function " << functionNode->name << std::endl;
                return;
            }
            paramTypes.push_back(paramType);
        }

        llvm::FunctionType *funcType = llvm::FunctionType::get(cryoTypesInstance.getLLVMType(functionNode->returnType), paramTypes, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, cryoContext.module.get());
    }

    void CryoSyntax::generateExternalDeclaration(ASTNode *node)
    {
        if (node == nullptr)
        {
            std::cerr << "[Functions] Error: Node is null in generateExternalDeclaration\n";
            return;
        }

        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();

        char *functionName = node->data.externNode.decl.function->name;
        FunctionDeclNode *functionNode = node->data.externNode.decl.function;

        // Check if the function already exists in the module
        if (cryoContext.module->getFunction(functionName))
        {
            std::cerr << "[Functions] Warning: Function " << functionName << " already exists in the module\n";
            return;
        }

        std::vector<llvm::Type *> paramTypes;

        for (int i = 0; i < functionNode->paramCount; i++)
        {
            llvm::Type *paramType;
            switch (functionNode->params[i]->data.varDecl.dataType)
            {
            case DATA_TYPE_INT:
                if (strstr(functionNode->name, "Ptr") != NULL)
                    paramType = llvm::Type::getInt32Ty(cryoContext.context)->getPointerTo();
                else
                    paramType = llvm::Type::getInt32Ty(cryoContext.context);
                break;
            case DATA_TYPE_STRING:
                paramType = llvm::Type::getInt8Ty(cryoContext.context)->getPointerTo();
                break;
            // Add other types as needed
            default:
                std::cerr << "Unsupported parameter type in function " << functionNode->name << std::endl;
                return;
            }
            paramTypes.push_back(paramType);
        }

        return;
    }

    void CryoSyntax::generateReturnStatement(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        if (!node->data.returnStmt.expression)
        {
            cryoContext.builder.CreateRetVoid();
            return;
        }

        llvm::Value *returnValue = generateExpression(node->data.returnStmt.expression);
        cryoContext.builder.CreateRet(returnValue);

        return;
    }

    void CryoSyntax::generateFunctionCall(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypesInstance = compiler.getTypes();

        std::cout << "[Functions] Generating call to function: " << node->data.functionCall.name << std::endl;

        llvm::Function *function = cryoContext.module->getFunction(node->data.functionCall.name);
        if (!function)
        {
            std::cerr << "[Functions] Error: Function " << node->data.functionCall.name << " not found in module" << std::endl;
            return;
        }

        std::vector<llvm::Value *> args;
        for (int i = 0; i < node->data.functionCall.argCount; i++)
        {
            llvm::Value *arg = generateExpression(node->data.functionCall.args[i]);
            if (arg == nullptr)
            {
                std::cerr << "Error: Failed to generate argument " << i << " for function call" << std::endl;
                return;
            }

            llvm::Type *paramType = function->getArg(i)->getType();
            if (arg->getType() != paramType)
            {
                if (paramType->isPointerTy() && !arg->getType()->isPointerTy())
                {
                    // If the function expects a pointer but we have a value, pass the address
                    llvm::AllocaInst *temp = cryoContext.builder.CreateAlloca(arg->getType());
                    cryoContext.builder.CreateStore(arg, temp);
                    arg = temp;
                }
                else if (!paramType->isPointerTy() && arg->getType()->isPointerTy())
                {
                    // If the function expects a value but we have a pointer, load it
                    arg = cryoContext.builder.CreateLoad(paramType, arg);
                }
            }

            args.push_back(arg);
        }
        llvm::Value *call = cryoContext.builder.CreateCall(function, args);
        std::cout << "[Functions] Generated function call to " << node->data.functionCall.name << std::endl;
    }

    llvm::Value *CryoSyntax::createFunctionCall(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypesInstance = compiler.getTypes();

        std::cout << "[Functions] Generating call to function: " << node->data.functionCall.name << std::endl;

        llvm::Function *function = cryoContext.module->getFunction(node->data.functionCall.name);
        if (!function)
        {
            std::cerr << "[Functions] Error: Function " << node->data.functionCall.name << " not found in module" << std::endl;
            return nullptr;
        }

        std::vector<llvm::Value *> args;
        for (int i = 0; i < node->data.functionCall.argCount; i++)
        {
            llvm::Value *arg = generateExpression(node->data.functionCall.args[i]);
            if (arg == nullptr)
            {
                std::cerr << "Error: Failed to generate argument " << i << " for function call" << std::endl;
                return nullptr;
            }

            llvm::Type *paramType = function->getArg(i)->getType();
            if (arg->getType() != paramType)
            {
                if (paramType->isPointerTy() && !arg->getType()->isPointerTy())
                {
                    // If the function expects a pointer but we have a value, pass the address
                    llvm::AllocaInst *temp = cryoContext.builder.CreateAlloca(arg->getType());
                    cryoContext.builder.CreateStore(arg, temp);
                    arg = temp;
                }
                else if (!paramType->isPointerTy() && arg->getType()->isPointerTy())
                {
                    // If the function expects a value but we have a pointer, load it
                    arg = cryoContext.builder.CreateLoad(paramType, arg);
                }
            }

            args.push_back(arg);
        }
        llvm::Value *call = cryoContext.builder.CreateCall(function, args);
        std::cout << "[Functions] Generated function call to " << node->data.functionCall.name << std::endl;
        return call;
    }

    void CryoSyntax::generateFunctionBlock(ASTNode *node)
    {
        if (!node)
        {
            std::cerr << "[Functions] Error: Node is null in generateFunctionBlock\n";
            return;
        }

        CryoContext &cryoContext = compiler.getContext();

        ASTNode **statements = node->data.block.statements;
        for (int i = 0; i < node->data.block.stmtCount; i++)
        {
            identifyNodeExpression(statements[i]);
        }

        return;
    }

} // namespace Cryo
