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

    Symbol *GlobalSymbolTable::getFrontendSymbol(const char *symbolName, const char *scopeID, TypeOfSymbol symbolType)
    {
        if (!symbolName || symbolName == nullptr)
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
            Symbol *symbol = symbols[i];
            switch (symbolType)
            {
            case VARIABLE_SYMBOL:
            {
                VariableSymbol *varSymbol = symbol->variable;
                if (strcmp(varSymbol->name, symbolName) == 0 && strcmp(varSymbol->scopeId, scopeID) == 0)
                {
                    return symbol;
                }
                else
                {
                    continue;
                }
            }
            case FUNCTION_SYMBOL:
            {
                // Look for function declarations
                Symbol *resolvedSymbol = resolveFunctionSymbol(symbolName, scopeID, symbolType);
                if (resolvedSymbol)
                {
                    return resolvedSymbol;
                }
                else
                {
                    DEBUG_BREAKPOINT;
                }
            }
            case EXTERN_SYMBOL:
            {
                // Look for extern functions
                Symbol *resolvedSymbol = resolveExternSymbol(symbolName);
                if (resolvedSymbol)
                {
                    return resolvedSymbol;
                }
                else
                {
                    DEBUG_BREAKPOINT;
                }
            }
            case TYPE_SYMBOL:
            {
                TypeSymbol *typeSymbol = symbol->type;
                if (strcmp(typeSymbol->name, symbolName) == 0 && strcmp(typeSymbol->scopeId, scopeID) == 0)
                {
                    return symbol;
                }
                else
                {
                    continue;
                }
            }
            case PROPERTY_SYMBOL:
            {
                PropertySymbol *propSymbol = symbol->property;
                if (strcmp(propSymbol->name, symbolName) == 0 && strcmp(propSymbol->scopeId, scopeID) == 0)
                {
                    return symbol;
                }
                else
                {
                    DEBUG_BREAKPOINT;
                }
            }
            case METHOD_SYMBOL:
            {
                MethodSymbol *methodSymbol = symbol->method;
                if (strcmp(methodSymbol->name, symbolName) == 0 && strcmp(methodSymbol->scopeId, scopeID) == 0)
                {
                    return symbol;
                }
                else
                {
                    DEBUG_BREAKPOINT;
                }
            }
            default:
                std::cerr << "Symbol Type not recognized!" << std::endl;
                return nullptr;
            }
        }
    }

    Symbol *GlobalSymbolTable::resolveFunctionSymbol(const char *symbolName, const char *scopeID, TypeOfSymbol symbolType)
    {
        if (!symbolName || symbolName == nullptr)
        {
            return nullptr;
        }

        if (!scopeID || scopeID == nullptr)
        {
            return nullptr;
        }

        // Check for the extern functions first
        Symbol *externSymbol = resolveExternSymbol(symbolName);
        if (externSymbol != nullptr)
        {
            std::cout << "Extern Symbol Resolved!" << std::endl;
            return externSymbol;
        }

        // Check for the global functions (REMOVE THIS LATER)
        // SET FUNCTIONS SCOPE ID TO THE NAMESPACE SCOPE ID
        int globalCount = globalFunctions.size();
        for (int i = 0; i < globalCount; i++)
        {
            FunctionSymbol *functionSymbol = globalFunctions[i];
            if (strcmp(functionSymbol->name, symbolName) == 0)
            {
                return createSymbol(FUNCTION_SYMBOL, functionSymbol);
            }
            else
            {
                continue;
            }
        }
    }

    Symbol *GlobalSymbolTable::resolveExternSymbol(const char *symbolName)
    {
        // Look at the extern functions in `externFunctions` and resolve the symbol
        // if it exists.

        if (!symbolName || symbolName == nullptr)
        {
            return nullptr;
        }

        int externCount = externFunctions.size();
        for (int i = 0; i < externCount; i++)
        {
            ExternSymbol *externSymbol = externFunctions[i];
            if (strcmp(externSymbol->name, symbolName) == 0)
            {
                std::cout << "Extern Symbol Resolved!" << std::endl;
                return createSymbol(EXTERN_SYMBOL, externSymbol);
            }
            else
            {
                std::cout << "Checked Extern Symbol: " << externSymbol->name << " Expected: " << symbolName << std::endl;
                continue;
            }
        }

        std::cout << "<!> Extern Symbol not found <!>" << std::endl;
        return nullptr;
    }

} // namespace Cryo
