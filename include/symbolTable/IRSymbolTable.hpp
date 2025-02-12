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

#include "symbolTable/IRdefs.hpp"

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
            // Initialize with global scope
            scopeStack.push_back({});
        }

        // Core scope operations
        void pushScope()
        {
            scopeStack.push_back({});
        }

        void popScope()
        {
            if (scopeStack.size() > 1)
            { // Keep global scope
                scopeStack.pop_back();
            }
        }

        // Symbol management
        bool addVariable(const IRVariableSymbol &symbol)
        {
            if (scopeStack.empty())
                return false;
            auto &currentScope = scopeStack.back();
            currentScope[symbol.name] = symbol;
            return true;
        }

        bool addFunction(const IRFunctionSymbol &symbol)
        {
            functions[symbol.name] = symbol;
            return true;
        }

        // Symbol lookup
        IRVariableSymbol *findVariable(const std::string &name)
        {
            // Search from current scope up to global
            for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it)
            {
                auto found = it->find(name);
                if (found != it->end())
                {
                    return &found->second;
                }
            }
            return nullptr;
        }

        IRFunctionSymbol *findFunction(const std::string &name)
        {
            auto it = functions.find(name);
            return (it != functions.end()) ? &it->second : nullptr;
        }

        // Module access
        llvm::Module *getModule() { return currentModule; }
    };

} // namespace Cryo

#endif // IR_SYMBOL_TABLE_H
