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
#include "symbolTable/globalSymtable.hpp"

namespace Cryo
{

    VariableSymbol *GlobalSymbolTable::getFrontendVariableSymbol(const char *name, const char *scopeID)
    {
        if (!name || name == nullptr)
        {
            return nullptr;
        }

        if (!scopeID || scopeID == nullptr)
        {
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            return nullptr;
        }

        int symbolCount = table->count;
        Symbol **symbols = table->symbols;

        for (int i = 0; i < symbolCount; i++)
        {
            if (symbols[i]->symbolType == VARIABLE_SYMBOL)
            {
                Symbol *symbol = symbols[i];
                VariableSymbol *varSymbol = symbol->variable;
                if (strcmp(varSymbol->name, name) == 0 && strcmp(varSymbol->scopeId, scopeID) == 0)
                {
                    std::cout << "Variable Symbol Resolved!" << std::endl;
                    return varSymbol;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }

        std::cout << "<!> Variable Symbol not found <!>" << std::endl;
        return nullptr;
    }

    MethodSymbol *GlobalSymbolTable::getFrontendMethodSymbol(const char *methodName, const char *className, const char *scopeID)
    {
        if (!methodName || methodName == nullptr)
        {
            return nullptr;
        }

        if (!className || className == nullptr)
        {
            return nullptr;
        }

        if (!scopeID || scopeID == nullptr)
        {
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            return nullptr;
        }

        int symbolCount = table->count;
        Symbol **symbols = table->symbols;

        for (int i = 0; i < symbolCount; i++)
        {
            if (symbols[i]->symbolType == METHOD_SYMBOL)
            {
                Symbol *symbol = symbols[i];
                MethodSymbol *methodSymbol = symbol->method;
                if (strcmp(methodSymbol->name, methodName) == 0 && strcmp(methodSymbol->scopeId, scopeID) == 0)
                {
                    std::cout << "Method Symbol Resolved!" << std::endl;
                    return methodSymbol;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }

        std::cout << "<!> Method Symbol not found <!>" << std::endl;
        return nullptr;
    }

} // namespace Cryo
