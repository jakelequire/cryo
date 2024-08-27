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
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        // Check if _defaulted function already exists
        if (cryoContext.module->getFunction("main"))
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Functions", "main function already exists. Skipping creation.");
            return;
        }
        if (cryoContext.module->getFunction("_defaulted"))
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Functions", "_defaulted function already exists. Skipping creation.");
            return;
        }

        llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(cryoContext.context), false);
        llvm::Function *function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "_defaulted", cryoContext.module.get());
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(cryoContext.context, "entry", function);

        cryoContext.builder.SetInsertPoint(entry);
        cryoContext.builder.CreateRetVoid();

        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Created default main function");

        return;
    }

    void CryoSyntax::generateFunctionPrototype(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (node == nullptr)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Node is null in generateFunctionPrototype");
            return;
        }

        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();

        char *functionName;
        FunctionDeclNode *functionNode = nullptr;
        if (node->metaData->type == NODE_EXTERN_FUNCTION)
        {
            functionName = node->data.externNode->externNode->data.functionDecl->name;
            functionNode = (node->data.externNode->externNode->data.functionDecl);
        }
        else if (node->metaData->type == NODE_FUNCTION_DECLARATION)
        {
            functionName = node->data.functionDecl->name;
            functionNode = (node->data.functionDecl);
        }

        // Check if the function already exists in the module
        if (cryoContext.module->getFunction(functionName))
        {
            cryoDebugger.logMessage("WARNING", __LINE__, "Functions", "Function already exists in module");
            return;
        }

        std::vector<llvm::Type *> paramTypes;
        for (int i = 0; i < functionNode->paramCount; i++)
        {
            auto param = functionNode->params[i];
            llvm::Type *llvmType = cryoTypesInstance.getLLVMType(param->data.varDecl->type);
            paramTypes.push_back(llvmType);
        }

        llvm::FunctionType *funcType = llvm::FunctionType::get(cryoTypesInstance.getLLVMType(functionNode->returnType), paramTypes, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, cryoContext.module.get());
    }

    void CryoSyntax::generateFunction(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        char *functionName = nullptr;
        FunctionDeclNode *functionNode = nullptr;

        if (node->metaData->type == NODE_FUNCTION_DECLARATION)
        {
            functionName = strdup(node->data.functionDecl->name);
            functionNode = node->data.functionDecl;
        }
        else if (node->metaData->type == NODE_EXTERN_STATEMENT)
        {
            functionName = strdup(node->data.externNode->externNode->data.functionDecl->name);
            functionNode = node->data.externNode->externNode->data.functionDecl;
        }
        else
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Invalid node type for function generation");
            return;
        }

        llvm::Function *function = cryoContext.module->getFunction(functionName);
        if (!function)
        {
            // Generate the function prototype if it doesn't exist
            generateFunctionPrototype(node);
            function = cryoContext.module->getFunction(functionName);
            if (!function)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Failed to generate function prototype");
                return;
            }
        }

        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Generating code for function");
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(cryoContext.context, "entry", function);
        cryoContext.builder.SetInsertPoint(entry);

        // Create a new scope for the function
        // cryoContext.namedValues.clear();
        std::vector<llvm::Type *> paramTypes;
        for (int i = 0; i < functionNode->paramCount; i++)
        {
            llvm::Argument *arg = function->arg_begin() + i;
            arg->setName(functionNode->params[i]->data.varDecl->name);
            cryoContext.namedValues[functionNode->params[i]->data.varDecl->name] = arg;
            paramTypes.push_back(arg->getType());
        }

        llvm::Type *returnType = cryoTypesInstance.getLLVMType(node->data.functionDecl->returnType);
        llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes, false);
        if (!function)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Creating function");
            function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, cryoContext.module.get());
        }
        else
        {
            if (function->getFunctionType() != funcType)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Function type mismatch");
                return;
            }
        }

        cryoContext.builder.SetInsertPoint(entry);

        // Generate code for the function body
        if (node->data.functionDecl->body)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Generating function block");
            generateFunctionBlock(node->data.functionDecl->body);
        }

        // Ensure the function has a return statement or terminator
        if (!entry->getTerminator())
        {
            if (returnType->isVoidTy())
            {
                cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Function Block has no terminator");
                cryoContext.builder.CreateRetVoid();
            }
            else
            {
                cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Function Block has no terminator");
                cryoContext.builder.CreateRet(llvm::Constant::getNullValue(returnType));
            }
        }
        return;
    }

    void CryoSyntax::generateExternalPrototype(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (node == nullptr)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Node is null in generateExternalPrototype");
            return;
        }

        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();

        char *functionName = node->data.externFunction->name;
        ExternFunctionNode *functionNode = node->data.externFunction;

        // Check if the function already exists in the module
        if (cryoContext.module->getFunction(functionName))
        {
            cryoDebugger.logMessage("WARNING", __LINE__, "Functions", "Function already exists in module");
            return;
        }

        std::vector<llvm::Type *> paramTypes;

        for (int i = 0; i < functionNode->paramCount; i++)
        {
            llvm::Type *paramType;
            switch (functionNode->params[i]->data.varDecl->type)
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
                cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Unsupported parameter type in function");
                return;
            }
            paramTypes.push_back(paramType);
        }

        llvm::FunctionType *funcType = llvm::FunctionType::get(cryoTypesInstance.getLLVMType(functionNode->returnType), paramTypes, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, strdup(functionName), cryoContext.module.get());
        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Generated external function prototype");
    }

    void CryoSyntax::generateExternalDeclaration(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (node == nullptr)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "External Declaration Node is null");
            return;
        }

        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoContext &cryoContext = compiler.getContext();

        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Generating code for External Declaration Node");
        char *functionName = strdup(node->data.externFunction->name);
        cryoDebugger.logMessage("DEBUG", __LINE__, "Functions", "Function Name: " + std::string(functionName));
        ExternFunctionNode *functionNode = node->data.externFunction;
        CryoDataType returnTypeData = functionNode->returnType;
        cryoDebugger.logMessage("DEBUG", __LINE__, "Functions", "Return Type: " + std::to_string(returnTypeData));

        // Check if the function already exists in the module
        if (cryoContext.module->getFunction(functionName))
        {
            cryoDebugger.logMessage("WARNING", __LINE__, "Functions", "Function already exists in module");
            return;
        }

        std::vector<llvm::Type *> paramTypes;

        for (int i = 0; i < functionNode->paramCount; i++)
        {
            llvm::Type *paramType;
            switch (functionNode->params[i]->data.varDecl->type)
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
                cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Unsupported parameter type in function");
                return;
            }
            paramTypes.push_back(paramType);
        }

        return;
    }

    void CryoSyntax::generateReturnStatement(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        if (!node->data.returnStatement->expression)
        {
            cryoContext.builder.CreateRetVoid();
            return;
        }

        llvm::Value *returnValue = generateExpression(node->data.returnStatement->expression);
        cryoContext.builder.CreateRet(returnValue);

        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Generated return statement");
        return;
    }

    void CryoSyntax::generateFunctionCall(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Generating call to function");

        llvm::Function *function = cryoContext.module->getFunction(node->data.functionCall->name);
        if (!function)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Function not found in module");
            return;
        }

        std::vector<llvm::Value *> args;
        for (int i = 0; i < node->data.functionCall->argCount; i++)
        {
            std::cout << "[DEBUG] Getting Arguments..." << std::endl;
            llvm::Value *arg = generateArguments(node->data.functionCall->args[i]);
            std::cout << "[DEBUG]"
                      << CryoNodeTypeToString(node->data.functionCall->args[i]->metaData->type)
                      << std::endl;

            if (arg == nullptr)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Failed to generate argument for function call");
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
                    cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Passing pointer argument");
                    arg = temp;
                }
                else if (!paramType->isPointerTy() && arg->getType()->isPointerTy())
                {
                    // If the function expects a value but we have a pointer, load it
                    cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Loading pointer argument");
                    arg = cryoContext.builder.CreateLoad(paramType, arg);
                }
            }

            cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Adding argument to function call");
            args.push_back(arg);
        }
        llvm::Value *call = cryoContext.builder.CreateCall(function, args);
        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Generated function call");
    }

    llvm::Value *CryoSyntax::createFunctionCall(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Generating call to function");

        llvm::Function *function = cryoContext.module->getFunction(node->data.functionCall->name);
        if (!function)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Function not found in module");
            return nullptr;
        }

        std::vector<llvm::Value *> args;
        for (int i = 0; i < node->data.functionCall->argCount; i++)
        {
            llvm::Value *arg = generateArguments(node->data.functionCall->args[i]);
            if (arg == nullptr)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Failed to generate argument for function call");
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

                    cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Passing pointer argument");
                    arg = temp;
                }
                else if (!paramType->isPointerTy() && arg->getType()->isPointerTy())
                {
                    // If the function expects a value but we have a pointer, load it
                    cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Loading pointer argument");
                    arg = cryoContext.builder.CreateLoad(paramType, arg);
                }
            }

            cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Adding argument to function call");
            args.push_back(arg);
        }
        llvm::Value *call = cryoContext.builder.CreateCall(function, args);

        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Generated function call");
        return call;
    }

    void CryoSyntax::generateFunctionBlock(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (!node)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Function block node is null");
            return;
        }

        CryoContext &cryoContext = compiler.getContext();

        ASTNode **statements = node->data.functionBlock->statements;
        for (int i = 0; i < node->data.functionBlock->statementCount; i++)
        {
            identifyNodeExpression(statements[i]);
        }

        cryoDebugger.logMessage("INFO", __LINE__, "Functions", "Function block generation complete");
        return;
    }

    llvm::Value *CryoSyntax::generateArguments(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (!node)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Arguments node is null");
            return nullptr;
        }

        std::cout << "[DEBUG] Generating Arguments for node type: "
                  << CryoNodeTypeToString(node->metaData->type) << std::endl;

        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypesInstance = compiler.getTypes();

        llvm::Value *value = nullptr;
        switch (node->metaData->type)
        {
        case NODE_LITERAL_EXPR:
        {
            CryoDataType dt = node->data.literal->dataType;
            switch (dt)
            {
            case DATA_TYPE_INT:
                value = llvm::ConstantInt::get(cryoContext.context, llvm::APInt(32, node->data.literal->value.intValue, true));
                break;

            case DATA_TYPE_STRING:
                value = cryoContext.builder.CreateGlobalStringPtr(node->data.literal->value.stringValue);
                break;
            }

            break;
        }
        case NODE_VAR_DECLARATION: {
            llvm::Type *type = cryoTypesInstance.getLLVMType(node->data.varDecl->type);
            llvm::Value *initValue = nullptr;
            if (node->data.varDecl->initializer)
            {
                initValue = generateExpression(node->data.varDecl->initializer);
                if (!initValue)
                {
                    cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Failed to generate init value for variable declaration");
                    return nullptr;
                }
            }

            llvm::AllocaInst *alloca = cryoContext.builder.CreateAlloca(type, nullptr, node->data.varDecl->name);
            if (initValue)
            {
                cryoContext.builder.CreateStore(initValue, alloca);
            }

            value = alloca;
            break;
        }
        case NODE_VAR_NAME:
        {
            value = cryoContext.namedValues[node->data.varName->varName];
            if (!value)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Unknown variable name");
                return nullptr;
            }
            break;
        }
        default:
            cryoDebugger.logMessage("ERROR", __LINE__, "Functions", "Unsupported argument type");
            return nullptr;
        }

        return value;
    }

} // namespace Cryo
