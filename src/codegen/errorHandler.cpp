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
#include "codegen/oldCodeGen.hpp"

namespace Cryo
{
    void ErrorHandler::IsOutOfBoundsException(llvm::Value *index, std::string arrayName, llvm::Type *arrayType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "ErrorHandler", "Checking for out of bounds exception");

        if (!arrayType->isArrayTy())
        {
            DevDebugger::logMessage("ERROR", __LINE__, "ErrorHandler", "Array type not found");
            CONDITION_FAILED;
        }

        CryoContext &context = compiler.getContext();
        llvm::Function *currentFunction = context.currentFunction;

        if (!currentFunction)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "ErrorHandler", "No current function context");
            CONDITION_FAILED;
        }

        // Get or create the error block for this function
        llvm::BasicBlock *errorBlock = getOrCreateErrorBlock(currentFunction);

        // Get the array size
        llvm::Value *arraySize = llvm::ConstantInt::get(
            context.context,
            llvm::APInt(32, arrayType->getArrayNumElements()));

        // Create continue block
        llvm::BasicBlock *continueBlock = llvm::BasicBlock::Create(
            context.context,
            "continue",
            currentFunction);

        // Create the bounds check
        llvm::Value *isOutOfBounds = context.builder.CreateICmpUGE(
            index,
            arraySize,
            "isOutOfBounds");

        // Branch based on the check
        context.builder.CreateCondBr(isOutOfBounds, errorBlock, continueBlock);

        // Continue with normal execution
        context.builder.SetInsertPoint(continueBlock);
    }

    llvm::BasicBlock *ErrorHandler::getOrCreateErrorBlock(llvm::Function *function)
    {
        // Check if we already have an error block for this function
        auto it = functionErrorBlocks.find(function);
        if (it != functionErrorBlocks.end())
        {
            return it->second;
        }

        // Create new error block for this function
        CryoContext &context = compiler.getContext();
        llvm::BasicBlock *errorBlock = llvm::BasicBlock::Create(
            context.context,
            "bounds.error",
            function);

        // Save current insertion point
        llvm::BasicBlock *savedBlock = context.builder.GetInsertBlock();

        // Setup the error block
        context.builder.SetInsertPoint(errorBlock);
        llvm::Function *errorFunc = context.module->getFunction("outOfBoundsError");
        if (!errorFunc)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "ErrorHandler", "Error handler function not found: @outOfBoundsError");
            CONDITION_FAILED;
        }
        context.builder.CreateCall(errorFunc);

        // Add exit call
        llvm::Function *exitFunc = context.module->getFunction("sys_exit");
        if (exitFunc)
        {
            context.builder.CreateCall(exitFunc, llvm::ConstantInt::get(context.context, llvm::APInt(32, 1)));
        }
        context.builder.CreateUnreachable();

        // Restore previous insertion point
        context.builder.SetInsertPoint(savedBlock);

        // Cache the error block
        functionErrorBlocks[function] = errorBlock;

        return errorBlock;
    }

    void ErrorHandler::initializeErrorBlocks()
    {
        if (isInitialized)
            return;

        CryoContext &context = compiler.getContext();
        if (!context.module)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "ErrorHandler", "Module not initialized");
            CONDITION_FAILED;
        }

        // Create a dedicated function for error handling
        llvm::FunctionType *errorFuncType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(context.context),
            false);

        sharedErrorFunc = llvm::Function::Create(
            errorFuncType,
            llvm::Function::InternalLinkage,
            "array_bounds_error_handler",
            *context.module);

        // Create the shared out of bounds block
        sharedOutOfBoundsBlock = llvm::BasicBlock::Create(
            context.context,
            "shared.out.of.bounds",
            sharedErrorFunc);

        // Store current insertion point
        llvm::BasicBlock *savedBlock = context.builder.GetInsertBlock();
        llvm::Function *savedFunction = savedBlock ? savedBlock->getParent() : nullptr;

        // Set up the out of bounds block
        context.builder.SetInsertPoint(sharedOutOfBoundsBlock);
        llvm::Function *errorHandler = context.module->getFunction("outOfBoundsError");
        if (!errorHandler)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "ErrorHandler", "Error handler function not found");
            CONDITION_FAILED;
        }

        // Call error function and mark as unreachable
        context.builder.CreateCall(errorHandler);
        context.builder.CreateUnreachable();

        // Restore previous insertion point if it existed
        if (savedBlock)
        {
            context.builder.SetInsertPoint(savedBlock);
        }

        isInitialized = true;
        DevDebugger::logMessage("INFO", __LINE__, "ErrorHandler", "Error blocks initialized");
    }
} // namespace Cryo
