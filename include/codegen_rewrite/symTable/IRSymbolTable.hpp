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
#ifndef IR_SYMBOL_TABLE_H
#define IR_SYMBOL_TABLE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdbool.h>

#include "IRdefs.hpp"
#include "tools/macros/consoleColors.h"

namespace Cryo
{

    class IRSymbolTable
    {
    private:
        llvm::Module *currentModule;
        std::vector<std::map<std::string, IRVariableSymbol>> scopeStack;
        std::map<std::string, IRFunctionSymbol> functions;
        std::map<std::string, IRTypeSymbol> types;

    public:
        explicit IRSymbolTable(llvm::Module *module)
            : currentModule(module)
        {
            std::cout << "Creating symbol table for module: " << module->getName().str() << std::endl;
            // Initialize with global scope
            scopeStack.push_back({});
        }

        friend class IRSymbolManager;
        IRSymbolManager *getSymbolManager() { return new IRSymbolManager(); }
        void setCurrentFunction(llvm::Function *function) { currentFunction = function; }
        void clearCurrentFunction() { currentFunction = nullptr; }

        // Module access
        llvm::Module *getModule() { return currentModule; }

        // Core scope operations
        void pushScope();
        void popScope();

        // Symbol management
        bool addVariable(const IRVariableSymbol &symbol);
        bool addFunction(const IRFunctionSymbol &symbol);
        bool addType(const IRTypeSymbol &symbol);

        // Symbol lookup
        IRVariableSymbol *findVariable(const std::string &name);
        // Helper method for creating global variables
        IRVariableSymbol *createGlobalVariable(const std::string &name, llvm::Type *type,
                                               llvm::Value *initialValue = nullptr);

        IRFunctionSymbol *findFunction(const std::string &name);
        IRTypeSymbol *findType(const std::string &name);

        void debugPrint() const;

    private:
        llvm::Function *currentFunction;

        template <typename T>
        llvm::Type *getLLVMType();

        template <typename T>
        llvm::Value *createLLVMValue(const T &value);

    public:
        template <typename T>
        IRVariableSymbol *createLocalVar(const std::string &name, T value)
        {
            auto *type = getLLVMType<T>();
            auto *llvmValue = createLLVMValue<T>(value);
            return createVariable(name, type, llvmValue);
        }
    };

} // namespace Cryo

#endif // IR_SYMBOL_TABLE_H
