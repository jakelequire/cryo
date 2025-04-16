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
#ifndef _CODEGEN_DEBUG_H
#define _CODEGEN_DEBUG_H

#include <iostream>
#include <string>
#include <vector>

#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"

namespace Cryo
{

    class CodeGenDebug
    {
    public:
        static void printLLVMValue(llvm::Value *value);
        static void printLLVMType(llvm::Type *type);
        static void printLLVMFunction(llvm::Function *function);
        static void printLLVMStruct(llvm::StructType *structType);
        static void printLLVMInstruction(llvm::Instruction *inst);
        static void printLLVMAllocaInst(llvm::AllocaInst *allocaInst);
        static std::string LLVMTypeIDToString(llvm::Type *type);
    };

} // namespace Cryo
#endif // _CODEGEN_DEBUG_H
