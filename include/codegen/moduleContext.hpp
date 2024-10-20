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
#ifndef MODULECONTEXT_HPP
#define MODULECONTEXT_HPP
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

#include "common/common.h"

namespace Cryo
{
    class ModuleContext
    {
    public:
        static ModuleContext &getInstance();

        ModuleContext(const ModuleContext &) = delete;
        ModuleContext &operator=(const ModuleContext &) = delete;

        CompilerState *state;

        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        std::unique_ptr<llvm::Module> module;
        std::unique_ptr<std::vector<llvm::Module *>> modules;

        std::unordered_map<std::string, llvm::Value *> namedValues;
        std::unordered_map<std::string, llvm::StructType *> structTypes;

        std::string currentNamespace;
        llvm::Function *currentFunction;
        std::vector<CompiledFile> compiledFiles;

        std::vector<llvm::Module *> *getModules() { return modules.get(); }

        bool inGlobalScope = true;

        void initializeContext();
        void setModuleIdentifier(const std::string &name);
        void addCompiledFileInfo(CompiledFile file);

    private:
        ModuleContext();
    };
}

#endif // MODULECONTEXT_HPP
