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
#include "codegen/devDebugger/devDebugger.hpp"

namespace Cryo
{
    class ModuleContext
    {
    public:
        ModuleContext() : builder(context)
        {
            initializeContext();
        }
        ~ModuleContext() = default;

        static ModuleContext &getInstance()
        {
            return instance;
        }
        CompilerState *state;

        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        std::unique_ptr<llvm::Module> module;

        std::unordered_map<std::string, llvm::Value *> namedValues;
        std::unordered_map<std::string, llvm::StructType *> structTypes;

        std::string currentNamespace;
        llvm::Function *currentFunction;
        std::vector<CompiledFile> compiledFiles;

        bool inGlobalScope = true;

        void initializeContext()
        {
            std::string moduleName = "CryoModuleDefaulted";
            module = std::make_unique<llvm::Module>(moduleName, context);
            DevDebugger::logMessage("INFO", __LINE__, "ModuleContext", "Module Initialized");
        }

        void setModuleIdentifier(const std::string &name);
        void addCompiledFileInfo(CompiledFile file);

    private:
        // Current Instance
        static ModuleContext instance;
    };

}

#endif // MODULECONTEXT_HPP
