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

        // Create function type - return struct by value
        llvm::FunctionType *ctorFuncType = llvm::FunctionType::get(
            structType, // Return type is the struct itself
            ctorArgTypes,
            false);

        // Create the constructor function
        llvm::Function *ctorFunction = llvm::Function::Create(
            ctorFuncType,
            llvm::Function::ExternalLinkage,
            structName + ".ctor",
            context.getInstance().module.get());

        logMessage(LMI, "INFO", "Visitor", "Constructor function created: %s", ctorFunction->getName().str().c_str());

        // Setup function attributes
        ctorFunction->setCallingConv(llvm::CallingConv::C);
        ctorFunction->setDoesNotThrow();
        ctorFunction->addFnAttr(llvm::Attribute::NoUnwind);

        // Create entry block
        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(
            context.getInstance().context,
            "entry",
            ctorFunction);
        context.getInstance().builder.SetInsertPoint(entryBlock);

        // Allocate the struct locally in the function
        llvm::Value *structAlloca = context.getInstance().builder.CreateAlloca(
            structType, nullptr, "instance");

        // Process arguments
        llvm::Function::arg_iterator args = ctorFunction->arg_begin();

        // Enter constructor context
        context.getInstance().symbolTable->enterConstructorInstance();

        // Create a new scope for constructor body
        context.getInstance().symbolTable->pushScope();

        // Register 'self.alloc' for constructor body access
        IRVariableSymbol selfSymbol = IRSymbolManager::createVariableSymbol(
            ctorFunction,
            structAlloca,
            structType,
            "self.alloc",
            AllocaType::AllocaOnly);
        context.getInstance().symbolTable->addVariable(selfSymbol);

        // Register constructor parameters as local variables
        for (int i = 0; i < cTorArgs; i++)
        {
            llvm::Value *arg = &(*args++);
            std::string paramName = ctorArgs[i]->data.param->name;
            arg->setName(paramName);

            // Create alloca for parameter
            llvm::Value *argAlloca = context.getInstance().builder.CreateAlloca(
                arg->getType(), nullptr, paramName + ".addr");
            context.getInstance().builder.CreateStore(arg, argAlloca);

            // Register in symbol table
            IRVariableSymbol paramSymbol = IRSymbolManager::createVariableSymbol(
                ctorFunction,
                argAlloca,
                arg->getType(),
                paramName,
                AllocaType::AllocaLoadStore);
            context.getInstance().symbolTable->addVariable(paramSymbol);
        }
        logMessage(LMI, "INFO", "Visitor", "Constructor parameters registered.");

        // Initialize struct fields with default values if needed
        // ... (add code here if your language has default field initialization)

        // Process constructor body
        if (node->data.structConstructor->constructorBody)
        {
            logMessage(LMI, "INFO", "Visitor", "Generating constructor body...");
            context.getInstance().visitor->visit(node->data.structConstructor->constructorBody);
        }
        else
        {
            logMessage(LMI, "INFO", "Visitor", "No constructor body found.");
        }

        // Exit constructor context
        context.getInstance().symbolTable->popScope();
        context.getInstance().symbolTable->exitConstructorInstance();

        // Load the entire struct to return it by value
        llvm::Value *resultStruct = context.getInstance().builder.CreateLoad(
            structType, structAlloca, "complete_struct");

        // Return the struct by value
        context.getInstance().builder.CreateRet(resultStruct);

        // Verify function
        llvm::verifyFunction(*ctorFunction);

        logMessage(LMI, "INFO", "Visitor", "Constructor function verified.");

        // Add to symbol table
        IRFunctionSymbol ctorFuncSymbol = IRSymbolManager::createFunctionSymbol(
            ctorFunction,
            structName + ".ctor",
            structType,
            ctorFuncType,
            nullptr,
            false,
            false);
        context.getInstance().symbolTable->addFunction(ctorFuncSymbol);
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
