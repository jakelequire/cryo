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
#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/GlobalVariable.h"

enum class AllocaType
{
    // No alloca type (constants and immediate values such as literals).
    None,

    // `AllocaOnly` is used for variables that are only allocated on the stack.
    // Requires `alloca`.
    AllocaOnly,

    // `AllocaAndLoad` is used for variables that are allocated on the stack and loaded.
    // Requires `alloca` and `load`.
    AllocaAndLoad,

    // `AllocaAndStore` is used for variables that are allocated on the stack and stored.
    // Requires `alloca` and `store`.
    AllocaAndStore,

    // `AllocaLoadStore` is used for variables that are allocated on the stack, loaded, and stored.
    // Requires `alloca`, `load`, and `store`.
    AllocaLoadStore,

    // `Global` is used for variables that are allocated in the global scope.
    Global,

    // `Parameter` is used for function parameters.
    Parameter,

    // `Temporary` is used for temporary variables.
    Temporary,

    // `Aggregate` is used for aggregate types (structs, arrays, etc.).
    Aggregate,

    // `phi` is used for phi nodes in SSA form.
    Phi,

    // `StackSave` is used for saving the stack pointer.
    StackSave,
};

struct Allocation
{
    AllocaType allocaType;

    // Basic Allocation Instructions
    llvm::AllocaInst *allocaInst;
    llvm::StoreInst *storeInst;
    llvm::LoadInst *loadInst;

    // Global Allocation Instructions
    llvm::GlobalVariable *globalVariable;

    // For phi nodes in SSA form
    llvm::PHINode *phiNode;
};

typedef struct IRFunctionSymbol
{
    llvm::Function *function;
    llvm::Type *returnType;
    std::string name;

    llvm::BasicBlock *entryBlock;

    std::vector<llvm::Value *> arguments;
    std::vector<llvm::Value *> localVariables;
    std::vector<llvm::Value *> allocaVariables;
} IRFunctionSymbol;

typedef struct IRVariableSymbol
{
    llvm::Value *value;
    llvm::Type *type;
    std::string name;
    AllocaType allocaType;
} IRVariableSymbol;

typedef struct IRTypeSymbol
{

} IRTypeSymbol;

typedef struct IRPropertySymbol
{

} IRPropertySymbol;

typedef struct IRMethodSymbol
{

} IRMethodSymbol;
