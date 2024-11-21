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

        llvm::Function *function = compiler.getContext().module->getFunction("CryoArrayOutOfBounds");
        if (!function)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "ErrorHandler", "Function not found: CryoArrayOutOfBounds");
            CONDITION_FAILED;
        }

        // Get the array length
        llvm::Value *arrayLength = compiler.getArrays().getArrayLength(arrayName);
        if (!arrayLength)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "ErrorHandler", "Array length not found");
            CONDITION_FAILED;
        }

        // Create the comparison
        llvm::Value *cmp = compiler.getContext().builder.CreateICmpSGE(index, arrayLength, "cmp");

        // Create the conditional branch
        llvm::BasicBlock *currentBlock = compiler.getContext().builder.GetInsertBlock();
        llvm::BasicBlock *outOfBoundsBlock = llvm::BasicBlock::Create(compiler.getContext().context, "outOfBounds", currentBlock->getParent());
        llvm::BasicBlock *continueBlock = llvm::BasicBlock::Create(compiler.getContext().context, "continue", currentBlock->getParent());

        compiler.getContext().builder.CreateCondBr(cmp, outOfBoundsBlock, continueBlock);

        // Set the insert point to the out of bounds block
        compiler.getContext().builder.SetInsertPoint(outOfBoundsBlock);

        // Call the out of bounds function
        compiler.getContext().builder.CreateCall(function, {}, "call");

        // Create the unconditional branch
        compiler.getContext().builder.CreateBr(continueBlock);

        // Set the insert point to the continue block
        compiler.getContext().builder.SetInsertPoint(continueBlock);
    }

} // namespace Cryo
