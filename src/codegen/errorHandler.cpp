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
        llvm::Value *arraySize = nullptr;

        if (arrayType->isArrayTy())
        {
            arraySize = llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(32, arrayType->getArrayNumElements()));
        }
        else
        {
            DevDebugger::logMessage("ERROR", __LINE__, "ErrorHandler", "Array type not found");
            CONDITION_FAILED;
        }

        llvm::Value *isOutOfBounds = compiler.getContext().builder.CreateICmpUGE(index, arraySize, "isOutOfBounds");
        llvm::BasicBlock *currentBlock = compiler.getContext().builder.GetInsertBlock();
        llvm::Function *currentFunction = currentBlock->getParent();
        llvm::BasicBlock *outOfBoundsBlock = llvm::BasicBlock::Create(compiler.getContext().context, "outOfBounds", currentFunction);
        llvm::BasicBlock *inBoundsBlock = llvm::BasicBlock::Create(compiler.getContext().context, "inBounds", currentFunction);
        llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(compiler.getContext().context, "merge", currentFunction);

        compiler.getContext().builder.CreateCondBr(isOutOfBounds, outOfBoundsBlock, inBoundsBlock);

        // Out of bounds block
        compiler.getContext().builder.SetInsertPoint(outOfBoundsBlock);
        compiler.getContext().builder.CreateCall(compiler.getFunctions().getFunction("outOfBoundsError"), {});
        compiler.getContext().builder.CreateBr(mergeBlock);

        // In bounds block
        compiler.getContext().builder.SetInsertPoint(inBoundsBlock);
        compiler.getContext().builder.CreateBr(mergeBlock);

        // Merge block
        compiler.getContext().builder.SetInsertPoint(mergeBlock);

        return;
    }

} // namespace Cryo
