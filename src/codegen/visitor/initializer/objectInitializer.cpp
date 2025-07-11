/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "codegen/codegen.hpp"

namespace Cryo
{

    llvm::Value *Initializer::generateObjectInst(llvm::Value *varVal, ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating object instance...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_OBJECT_INST)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not an object instance");
            return nullptr;
        }

        // Get the object type name
        std::string objectTypeName = node->data.objectNode->name;
        logMessage(LMI, "INFO", "Initializer", "Object type name: %s", objectTypeName.c_str());
        bool isNewObject = node->data.objectNode->isNewInstance;
        // Get the object type
        DataType *objectType = node->data.objectNode->objType;
        if (!objectType)
        {
            logMessage(LMI, "ERROR", "Visitor", "Object type is null");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        // Get the LLVM type
        IRTypeSymbol *irTypeSymbol = context.getInstance().symbolTable->findType("struct." + objectTypeName);
        if (!irTypeSymbol)
        {
            logMessage(LMI, "ERROR", "Visitor", "IR type symbol is null");
            DEBUG_BREAKPOINT;
            return nullptr;
        }
        if (isNewObject)
        {
            logMessage(LMI, "INFO", "Visitor", "Creating new object instance: %s", objectTypeName.c_str());
            llvm::StructType *llvmStructType = llvm::dyn_cast<llvm::StructType>(irTypeSymbol->getType());
            if (!llvmStructType)
            {
                logMessage(LMI, "ERROR", "Visitor", "LLVM struct type is null");
                DEBUG_BREAKPOINT;
                return nullptr;
            }
            logMessage(LMI, "INFO", "Visitor", "LLVM Struct Type: %s", llvmStructType->getName().str().c_str());

            IRFunctionSymbol *ctorFuncSymbol = context.getInstance().symbolTable->findFunction("struct." + objectTypeName + ".ctor");
            if (!ctorFuncSymbol)
            {
                logMessage(LMI, "ERROR", "Visitor", "Constructor function symbol is null");
                DEBUG_BREAKPOINT;
                return nullptr;
            }

            // Prepare the arguments for the constructor
            std::vector<llvm::Value *> ctorArgs;
            // First argument is always 'this' pointer
            int argCount = node->data.objectNode->argCount;
            for (int i = 0; i < argCount; i++)
            {
                ASTNode *argNode = node->data.objectNode->args[i];
                llvm::Value *argVal = getInitializerValue(argNode);
                std::string argValName = argVal->getName().str();
                if (argValName.find("g_str") != std::string::npos)
                {
                    // Allocate the string
                    llvm::AllocaInst *strAlloc = context.getInstance().builder.CreateAlloca(
                        argVal->getType(), nullptr, argValName + ".alloc");
                    context.getInstance().builder.CreateStore(argVal, strAlloc);
                    argVal = context.getInstance().builder.CreateLoad(argVal->getType(), strAlloc, argValName + ".load");
                    logMessage(LMI, "INFO", "Visitor", "Loaded string argument %d: %s", i, argVal->getName().str().c_str());
                }
                else if (argVal->getType()->isPointerTy())
                {
                    // Load the value if it's a pointer
                    argVal = context.getInstance().builder.CreateLoad(argVal->getType(), argVal, argValName + ".load");
                }
                else if (argVal->getType()->isPointerTy())
                {
                    argVal = context.getInstance().builder.CreateLoad(argVal->getType(), argVal, argValName + ".load");
                }

                ctorArgs.push_back(argVal);
            }

            // Create the object instance
            llvm::Value *objectInstance = context.getInstance().builder.CreateCall(
                ctorFuncSymbol->function, ctorArgs, objectTypeName + ".ctor");
            if (!objectInstance)
            {
                logMessage(LMI, "ERROR", "Visitor", "Object instance is null");
                DEBUG_BREAKPOINT;
                return nullptr;
            }

            return objectInstance;
        }

        DEBUG_BREAKPOINT;

        return nullptr;
    }

    llvm::Value *Initializer::generateObjectInst(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating object instance...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_OBJECT_INST)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not an object instance");
            CONDITION_FAILED;
            return nullptr;
        }

        // Get the object type name
        std::string objectTypeName = node->data.objectNode->name;
        logMessage(LMI, "INFO", "Initializer", "Object type name: %s", objectTypeName.c_str());
        bool isNewObject = node->data.objectNode->isNewInstance;
        // Get the object type
        DataType *objectType = node->data.objectNode->objType;
        if (!objectType)
        {
            logMessage(LMI, "ERROR", "Visitor", "Object type is null");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        // Get the LLVM type
        IRTypeSymbol *irTypeSymbol = context.getInstance().symbolTable->findType("struct." + objectTypeName);
        if (!irTypeSymbol)
        {
            logMessage(LMI, "ERROR", "Visitor", "IR type symbol is null");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        if (isNewObject)
        {
            logMessage(LMI, "INFO", "Visitor", "Creating new object instance: %s", objectTypeName.c_str());
            llvm::StructType *llvmStructType = llvm::dyn_cast<llvm::StructType>(irTypeSymbol->getType());
            if (!llvmStructType)
            {
                logMessage(LMI, "ERROR", "Visitor", "LLVM struct type is null");
                DEBUG_BREAKPOINT;
                return nullptr;
            }
            logMessage(LMI, "INFO", "Visitor", "LLVM Struct Type: %s", llvmStructType->getName().str().c_str());

            IRFunctionSymbol *ctorFuncSymbol = context.getInstance().symbolTable->findFunction("struct." + objectTypeName + ".ctor");
            if (!ctorFuncSymbol)
            {
                logMessage(LMI, "ERROR", "Visitor", "Constructor function symbol is null");
                DEBUG_BREAKPOINT;
                return nullptr;
            }

            // Create the object instance allocation
            llvm::AllocaInst *objectAlloca = context.getInstance().builder.CreateAlloca(
                llvmStructType, nullptr, objectTypeName + ".instance");

            // Prepare the arguments for the constructor
            std::vector<llvm::Value *> ctorArgs;
            int argCount = node->data.objectNode->argCount;

            // First argument is still the pointer where constructor will initialize the object
            ctorArgs.push_back(objectAlloca);

            // Add remaining constructor arguments
            for (int i = 0; i < argCount; i++)
            {
                ASTNode *argNode = node->data.functionCall->args[i];
                DataType *argDataType = DTM->astInterface->getTypeofASTNode(argNode);
                if (!argDataType)
                {
                    logMessage(LMI, "ERROR", "Initializer", "Argument %d has no data type", i);
                    CONDITION_FAILED;
                    return nullptr;
                }
                llvm::Type *argType = context.getInstance().symbolTable->getLLVMType(argDataType);
                llvm::Value *argVal = getInitializerValue(argNode);
                if (argDataType->container->primitive == PRIM_STR && argVal->getType()->isPointerTy())
                {
                    // Load the string
                    std::string argValName = argVal->getName().str();
                    llvm::Value *strValue = context.getInstance().builder.CreateLoad(argVal->getType(), argVal, argValName + ".load");
                    argVal = context.getInstance().builder.CreateBitCast(strValue, context.getInstance().builder.getInt8Ty()->getPointerTo(), "str.cast");
                    logMessage(LMI, "INFO", "Visitor", "Loaded string argument %d: %s", i, argVal->getName().str().c_str());
                }
                else if (argVal->getType()->isPointerTy() && argDataType->container->primitive != PRIM_STR)
                {
                    // Load the value if it's a pointer
                    argVal = context.getInstance().builder.CreateLoad(argType, argVal);
                }
                else if (argVal->getType()->isPointerTy())
                {
                    argVal = context.getInstance().builder.CreateLoad(argType, argVal);
                }

                ctorArgs.push_back(argVal);
                logMessage(LMI, "INFO", "Visitor", "Constructor argument %d: %s", i, argVal->getName().str().c_str());
            }

            // Call constructor and get struct by value
            llvm::Value *structValue = context.getInstance().builder.CreateCall(
                ctorFuncSymbol->function, ctorArgs, objectTypeName + ".value");

            // If the constructor returns by value, we need to store it in our allocation
            // NOTE: Only do this if the constructor was modified to return a struct by value
            // If we've updated the constructor to return structs by value:
            context.getInstance().builder.CreateStore(structValue, objectAlloca);

            // Return the pointer to the allocated struct
            return objectAlloca;
        }

        DEBUG_BREAKPOINT;
        return nullptr;
    }

    IRMethodSymbol *Initializer::createClassMethod(const std::string &className, ASTNode *method, DataType *methodDataType)
    {
        ASSERT_NODE_NULLPTR_RET(method);
        if (method->metaData->type != NODE_METHOD)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Node is not a method");
            CONDITION_FAILED;
            return nullptr;
        }
        logMessage(LMI, "INFO", "Visitor", "Creating method: %s", method->data.method->name);
        // Create the function prototype
        std::vector<llvm::Type *> argTypes;
        bool isVarArg = false;
        for (size_t i = 0; i < method->data.method->paramCount; i++)
        {
            logMessage(LMI, "INFO", "Visitor", "Creating method parameter %d: %s", i, method->data.method->params[i]->data.param->name);
            ASTNode *param = method->data.method->params[i];
            DataType *paramDataType = param->data.param->type;
            if (paramDataType->container->objectType == VA_ARGS_OBJ)
            {
                isVarArg = true;
                break;
            }
            llvm::Type *paramType = context.getInstance().symbolTable->getLLVMType(paramDataType);
            argTypes.push_back(paramType);
        }

        logMessage(LMI, "INFO", "Visitor", "Creating method function prototype...");
        DataType *functionDataType = method->data.method->type;
        if (!functionDataType)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Function data type is null");
            CONDITION_FAILED;
            return nullptr;
        }

        logMessage(LMI, "INFO", "Visitor", "Function data type: %s", functionDataType->typeName);
        DataType *returnType = functionDataType->container->type.functionType->returnType;
        if (!returnType)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Return type is null");
            CONDITION_FAILED;
            return nullptr;
        }

        logMessage(LMI, "INFO", "Visitor", "Method return type: %s", returnType->typeName);
        llvm::Type *methodReturnType = context.getInstance().symbolTable->getLLVMType(returnType);
        if (!methodReturnType)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Method return type is null");
            CONDITION_FAILED;
            return nullptr;
        }

        llvm::FunctionType *methodFuncType = llvm::FunctionType::get(
            methodReturnType, argTypes, isVarArg);
        if (isVarArg)
        {
            methodFuncType = llvm::FunctionType::get(
                methodReturnType, argTypes, true);
        }

        logMessage(LMI, "INFO", "Visitor", "Creating method function prototype...");
        // The function signature
        llvm::Function *methodFunction = llvm::Function::Create(
            methodFuncType,
            llvm::Function::ExternalLinkage,
            className + "." + method->data.method->name,
            context.getInstance().module.get());

        methodFunction->setCallingConv(llvm::CallingConv::C);
        methodFunction->setDoesNotThrow();
        methodFunction->setName(className + "." + method->data.method->name);

        logMessage(LMI, "INFO", "Visitor", "Method function prototype created: %s", methodFunction->getName().str().c_str());
        // Add the method to the symbol table
        IRFunctionSymbol methodFnSymbol = IRSymbolManager::createFunctionSymbol(
            methodFunction,
            className + "." + method->data.method->name,
            methodReturnType,
            methodFuncType,
            nullptr, false, false);

        context.getInstance().symbolTable->addFunction(methodFnSymbol);
        logMessage(LMI, "INFO", "Visitor", "Adding method to symbol table: %s", methodFunction->getName().str().c_str());
        IRMethodSymbol methodSymbol = IRSymbolManager::createMethodSymbol(
            methodFnSymbol,
            false, false, false, false, 0, nullptr);

        logMessage(LMI, "INFO", "Visitor", "Setting method params...");
        for (size_t i = 0; i < method->data.method->paramCount; i++)
        {
            ASTNode *param = method->data.method->params[i];
            llvm::Type *paramType = context.getInstance().symbolTable->getLLVMType(param->data.param->type);

            // Create a symbol for each parameter for the symbol table
            std::string paramName = param->data.param->name;
            logMessage(LMI, "INFO", "Visitor", "Parameter Name: %s", paramName.c_str());
            AllocaType allocaType = AllocaTypeInference::inferFromNode(param, false);
            IRVariableSymbol paramSymbol = IRSymbolManager::createVariableSymbol(
                methodFunction, nullptr, paramType, paramName, allocaType);
            paramSymbol.dataType = param->data.param->type;

            // Create the parameter in the function
            llvm::Function::arg_iterator argIt = methodFunction->arg_begin();
            llvm::Value *arg = argIt++;
            arg->setName(paramName);
            paramSymbol.value = arg;
            context.getInstance().symbolTable->addVariable(paramSymbol);
        }

        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context.getInstance().context, "entry", methodFunction);
        context.getInstance().builder.SetInsertPoint(entryBlock);
        logMessage(LMI, "INFO", "Visitor", "Creating method function: %s", methodFunction->getName().str().c_str());
        context.getInstance().symbolTable->addFunction(methodFnSymbol);

        context.getInstance().visitor->visit(method->data.method->body);

        // Clear the current function
        context.getInstance().builder.ClearInsertionPoint();

        logMessage(LMI, "INFO", "Visitor", "Method function created: %s", methodFunction->getName().str().c_str());
        return &(methodSymbol);
    }

} // namespace Cryo
