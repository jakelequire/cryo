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
#include "codegen/codegen.hpp"

namespace Cryo
{
    void Initializer::generateStructConstructor(ASTNode *node, llvm::StructType *structType)
    {
        logMessage(LMI, "INFO", "Visitor", "Generating struct constructor...");
        std::string structName = structType->getName().str();

        // Get constructor arguments
        int cTorArgs = node->data.structConstructor->argCount;
        ASTNode **ctorArgs = node->data.structConstructor->args;
        std::vector<llvm::Type *> ctorArgTypes;

        // Get argument types
        for (int i = 0; i < cTorArgs; i++)
        {
            DataType *argType = ctorArgs[i]->data.param->type;
            llvm::Type *llvmArgType = context.getInstance().symbolTable->getLLVMType(argType);
            ctorArgTypes.push_back(llvmArgType);
        }

        // Add self pointer as first argument
        ctorArgTypes.insert(ctorArgTypes.begin(), structType->getPointerTo());

        // Create function type
        llvm::FunctionType *ctorFuncType = llvm::FunctionType::get(
            structType->getPointerTo(), // Return type is pointer to struct
            ctorArgTypes,
            false);

        // Create the constructor function
        llvm::Function *ctorFunction = llvm::Function::Create(
            ctorFuncType,
            llvm::Function::ExternalLinkage,
            structName + ".ctor",
            context.getInstance().module.get());

        ctorFunction->setCallingConv(llvm::CallingConv::C);
        ctorFunction->setDoesNotThrow();
        ctorFunction->addFnAttr(llvm::Attribute::NoUnwind);

        // Create entry block
        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(
            context.getInstance().context,
            "entry",
            ctorFunction);
        context.getInstance().builder.SetInsertPoint(entryBlock);

        // Get function arguments
        llvm::Function::arg_iterator args = ctorFunction->arg_begin();
        llvm::Value *selfArg = args++;
        selfArg->setName("self");

        // Store each constructor argument into the corresponding struct field
        for (int i = 0; i < cTorArgs; i++)
        {
            llvm::Value *arg = args++;
            arg->setName(ctorArgs[i]->data.param->name);

            // Create GEP to get pointer to struct field
            llvm::Value *fieldPtr = context.getInstance().builder.CreateStructGEP(
                structType,
                selfArg,
                i,
                "field_ptr");

            // Store argument into field
            context.getInstance().builder.CreateStore(arg, fieldPtr);
        }

        // Return the self pointer
        context.getInstance().builder.CreateRet(selfArg);

        // Verify function
        llvm::verifyFunction(*ctorFunction);

        // Add to symbol table
        IRFunctionSymbol ctorFuncSymbol = IRSymbolManager::createFunctionSymbol(
            ctorFunction,
            structName + ".ctor",
            structType->getPointerTo(),
            ctorFuncType,
            nullptr,
            false,
            false);
        context.getInstance().symbolTable->addFunction(ctorFuncSymbol);
    }

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
            ctorArgs.push_back(varVal);
            int argCount = node->data.objectNode->argCount;
            for (int i = 0; i < argCount; i++)
            {
                ctorArgs.push_back(getInitializerValue(node->data.objectNode->args[i]));
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

    llvm::Type *Initializer::derefValueForType(llvm::Value *value)
    {
        if (!value)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Value is null");
            return nullptr;
        }

        llvm::Type *type = value->getType();
        if (type->isPointerTy())
        {
            logMessage(LMI, "INFO", "CodeGen", "Dereferencing pointer: %s", value->getName().str().c_str());
            // Dereference the pointer
            llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(value);
            if (inst)
            {
                CodeGenDebug::printLLVMInstruction(inst);
                /*
                >>===-----------<LLVM Instruction Node>-----------===<<
                Inst:   %self.alloc = alloca %struct.Int, align 8
                Address: 0x555557f2d7b0
                >>===---------------------------------------------===<<
                */
                // If we print out something like the above, then we need to get the type of the alloca used
                // to create the pointer. This is because the alloca is not a pointer type, but a struct type.
                // So we need to get the type of the alloca and use that instead of the pointer type.
                inst->printAsOperand(llvm::errs(), false);
                llvm::Type *allocaType = nullptr;

                if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(inst))
                {
                    allocaType = allocaInst->getAllocatedType();
                    CodeGenDebug::printLLVMAllocaInst(allocaInst);
                }
                else
                {
                    logMessage(LMI, "ERROR", "CodeGen", "Value is not an alloca instruction");
                    return nullptr;
                }

                CodeGenDebug::printLLVMType(allocaType);
                logMessage(LMI, "INFO", "CodeGen", "Dereferencing pointer: %s", allocaType->getStructName().str().c_str());
                return allocaType;
            }
        }
        return type;
    }

} // namespace Cryo
