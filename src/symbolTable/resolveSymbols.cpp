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
                    printGlobalTable(this);
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

    Symbol *GlobalSymbolTable::findSymbol(const char *symbolName, const char *scopeID)
    {
        if (!symbolName || symbolName == nullptr)
        {
            std::cerr << "Error: Failed to find symbol, symbol name is null!" << std::endl;
            return nullptr;
        }
        if (!scopeID || scopeID == nullptr)
        {
            std::cerr << "Error: Failed to find symbol, scope ID is null!" << std::endl;
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            std::cerr << "Error: Failed to find symbol, table is null!" << std::endl;
            return nullptr;
        }

        int symbolCount = table->count;
        Symbol **symbols = table->symbols;

        for (int i = 0; i < symbolCount; i++)
        {
            Symbol *symbol = symbols[i];
            switch (symbol->symbolType)
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
                FunctionSymbol *funcSymbol = symbol->function;
                if (strcmp(funcSymbol->name, symbolName) == 0 && strcmp(funcSymbol->parentScopeID, scopeID) == 0)
                {
                    return symbol;
                }
                else
                {
                    continue;
                }
            }
            case EXTERN_SYMBOL:
            {
                ExternSymbol *externSymbol = symbol->externSymbol;
                if (strcmp(externSymbol->name, symbolName) == 0)
                {
                    return symbol;
                }
                else
                {
                    continue;
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
                    continue;
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
                    continue;
                }
            }
            default:
                std::cerr << "Error: Failed to find symbol, symbol type not recognized!" << std::endl;
                return nullptr;
            }
        }

        std::cerr << "Error: Failed to find symbol, symbol not found!" << std::endl;
        return nullptr;
    }

    Symbol *GlobalSymbolTable::findMethodSymbol(const char *methodName, const char *className, TypeofDataType typeOfNode)
    {
        if (!methodName || methodName == nullptr)
        {
            return nullptr;
        }

        if (!className || className == nullptr)
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

        // Look for the method symbol in the current table
        for (int i = 0; i < symbolCount; i++)
        {
            if (symbols[i]->symbolType == TYPE_SYMBOL)
            {
                TypeSymbol *typeSymbol = symbols[i]->type;
                if (strcmp(typeSymbol->name, className) == 0)
                {
                    // Check the methods
                    for (int j = 0; j < typeSymbol->methodCount; j++)
                    {
                        MethodSymbol *methodSymbol = typeSymbol->methods[j]->method;
                        if (strcmp(methodSymbol->name, methodName) == 0)
                        {
                            return typeSymbol->methods[j];
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
                else
                {
                    continue;
                }
            }
        }

        // Check the dependency tables
        Symbol *fallbackSymbol = seekMethodSymbolInAllTables(methodName, className, typeOfNode);
        if (fallbackSymbol != nullptr)
        {
            return fallbackSymbol;
        }

        std::cerr << "Error: Failed to find method symbol, symbol not found!" << std::endl;
        return nullptr;
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

        const char *currentNamespaceScopeID = getScopeID();

        // Check the namespace scope functions
        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            return nullptr;
        }

        int symbolCount = table->count;
        Symbol **symbols = table->symbols;

        // {NamespaceID}::{FunctionName}
        // The namespace ID is the scope ID of the namespace
        // This is the key to resolving the function symbol

        std::cout << "Resolving Function Symbol: " << symbolName << " in Scope: " << currentNamespaceScopeID << std::endl;

        for (int i = 0; i < symbolCount; i++)
        {
            if (symbols[i]->symbolType == FUNCTION_SYMBOL)
            {
                Symbol *symbol = symbols[i];
                FunctionSymbol *funcSymbol = symbol->function;
                const char *funcParentScopeID = funcSymbol->parentScopeID;
                std::cout << "Checking Function Symbol: " << funcSymbol->name << std::endl;
                if (strcmp(funcSymbol->name, symbolName) == 0 && strcmp(funcParentScopeID, currentNamespaceScopeID) == 0)
                {
                    std::cout << "Function Symbol Resolved!" << std::endl;
                    return symbol;
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

        std::cout << "Checking all tables for function symbol: " << symbolName << std::endl;

        Symbol *fallbackSymbol = seekFunctionSymbolInAllTables(symbolName);
        if (fallbackSymbol != nullptr)
        {
            return fallbackSymbol;
        }

        std::cout << "<!> Function Symbol not found: " << symbolName << " <!>" << std::endl;
        return nullptr;
    }

    Symbol *GlobalSymbolTable::seekFunctionSymbolInAllTables(const char *symbolName)
    {
        if (!symbolName || symbolName == nullptr)
        {
            return nullptr;
        }

        Symbol *symbol = nullptr;

        // Check the dependency tables
        int depCount = dependencyTableVector.size();
        for (int i = 0; i < depCount; i++)
        {
            SymbolTable *depTable = dependencyTableVector[i];
            symbol = querySpecifiedTable(symbolName, FUNCTION_SYMBOL, depTable);
            if (symbol != nullptr)
            {
                std::cout << "Function Symbol Resolved in Dependency Table!" << std::endl;
                return symbol;
            }
            else
            {
                continue;
            }
        }

        std::cout << "<!> Function Symbol not found in any table: " << symbolName << " <!>" << std::endl;
        return nullptr;
    }

    Symbol *GlobalSymbolTable::seekMethodSymbolInAllTables(const char *methodName, const char *className, TypeofDataType typeOfNode)
    {
        if (!methodName || methodName == nullptr)
        {
            return nullptr;
        }

        if (!className || className == nullptr)
        {
            return nullptr;
        }

        Symbol *symbol = nullptr;
        std::cout << "Seeking Method Symbol: " << methodName << " in Class: " << className << std::endl;

        // Check the dependency tables
        int depCount = dependencyTableVector.size();
        for (int i = 0; i < depCount; i++)
        {
            SymbolTable *depTable = dependencyTableVector[i];
            symbol = querySpecifiedTable(methodName, TYPE_SYMBOL, depTable);
            if (symbol != nullptr)
            {
                std::cout << "Type Symbol Found in Dependency Table! Checking Methods..." << std::endl;
                TypeSymbol *typeSymbol = symbol->type;
                for (int j = 0; j < typeSymbol->methodCount; j++)
                {
                    MethodSymbol *methodSymbol = typeSymbol->methods[j]->method;
                    if (strcmp(methodSymbol->name, methodName) == 0)
                    {
                        std::cout << "Method Symbol Resolved in Dependency Table!" << std::endl;
                        return typeSymbol->methods[j];
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else
            {
                continue;
            }
        }

        std::cout << "<!> Method Symbol not found in any table: " << methodName << " <!>" << std::endl;
        return nullptr;
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

        std::cout << "<!> Extern Symbol not found: " << symbolName << " <!>" << std::endl;
        return nullptr;
    }

    FunctionSymbol *GlobalSymbolTable::resolveScopedFunctionCall(const char *scopeID, const char *functionName)
    {
        if (!scopeID || scopeID == nullptr)
        {
            return nullptr;
        }

        if (!functionName || functionName == nullptr)
        {
            return nullptr;
        }

        SymbolTable *table = findSymbolTable(scopeID);
        if (!table)
        {
            std::cerr << "Error: Failed to resolve scoped function call, table not found!" << std::endl;
            return nullptr;
        }

        int symbolCount = table->count;
        Symbol **symbols = table->symbols;

        std::cout << "Resolving Scoped Function Symbol: " << functionName << " in Scope: " << scopeID << std::endl;

        for (int i = 0; i < symbolCount; i++)
        {
            if (symbols[i]->symbolType == FUNCTION_SYMBOL)
            {
                Symbol *symbol = symbols[i];
                FunctionSymbol *funcSymbol = symbol->function;
                if (strcmp(funcSymbol->name, functionName) == 0 && strcmp(funcSymbol->parentScopeID, scopeID) == 0)
                {
                    std::cout << "Scoped Function Symbol Resolved!" << std::endl;
                    return funcSymbol;
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
    }

    SymbolTable *GlobalSymbolTable::findSymbolTable(const char *scopeID)
    {
        if (!scopeID || scopeID == nullptr)
        {
            std::cerr << "Error: Failed to find symbol table, scope ID is null!" << std::endl;
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            std::cerr << "Error: Failed to find symbol table, current table is null!" << std::endl;
            return nullptr;
        }

        std::cout << "Checking for Symbol Table: " << scopeID << std::endl;

        if (strcmp(table->scopeId, scopeID) == 0)
        {
            std::cout << "Found Current Table!" << std::endl;
            return table;
        }

        std::cout << "Checking Dependency Tables..." << std::endl;

        // If it is not the current table, check the dependency tables
        int depCount = dependencyTableVector.size();
        for (int i = 0; i < depCount; i++)
        {
            std::cout << "Checking Dependency Table: " << i << std::endl;
            SymbolTable *depTable = dependencyTableVector[i];
            if (strcmp(depTable->scopeId, scopeID) == 0)
            {
                std::cout << "Found Dependency Table!" << std::endl;
                return depTable;
            }
            else
            {
                std::cout << "Dependency Table: " << i << " not found! Expected: " << scopeID << " Found: " << depTable->scopeId << std::endl;
                continue;
            }
        }

        std::cerr << "Error: Failed to find symbol table!" << std::endl;
        return nullptr;
    }

    DataType *GlobalSymbolTable::getDataTypeFromSymbol(Symbol *symbol)
    {
        if (!symbol || symbol == nullptr)
        {
            std::cerr << "Error: Failed to get data type from symbol, symbol is null!" << std::endl;
            return nullptr;
        }

        switch (symbol->symbolType)
        {
        case VARIABLE_SYMBOL:
        {
            VariableSymbol *varSymbol = symbol->variable;
            return varSymbol->type;
        }
        case FUNCTION_SYMBOL:
        {
            FunctionSymbol *funcSymbol = symbol->function;
            return funcSymbol->returnType;
        }
        case TYPE_SYMBOL:
        {
            TypeSymbol *typeSymbol = symbol->type;
            return typeSymbol->type;
        }
        case PROPERTY_SYMBOL:
        {
            PropertySymbol *propSymbol = symbol->property;
            return propSymbol->type;
        }
        case METHOD_SYMBOL:
        {
            MethodSymbol *methodSymbol = symbol->method;
            return methodSymbol->returnType;
        }
        default:
            std::cerr << "Error: Failed to get data type from symbol, symbol type not recognized!" << std::endl;
            return nullptr;
        }

        std::cerr << "Error: Failed to get data type from symbol, symbol type not recognized!" << std::endl;
        return nullptr;
    }

    ASTNode *GlobalSymbolTable::getASTNodeFromSymbol(Symbol *symbol)
    {
        if (!symbol || symbol == nullptr)
        {
            std::cerr << "Error: Failed to get AST node from symbol, symbol is null!" << std::endl;
            return nullptr;
        }

        switch (symbol->symbolType)
        {
        case VARIABLE_SYMBOL:
        {
            VariableSymbol *varSymbol = symbol->variable;
            return varSymbol->node;
        }
        case FUNCTION_SYMBOL:
        {
            FunctionSymbol *funcSymbol = symbol->function;
            return funcSymbol->node;
        }
        case TYPE_SYMBOL:
        {
            TypeSymbol *typeSymbol = symbol->type;
            return typeSymbol->node;
        }
        case PROPERTY_SYMBOL:
        {
            PropertySymbol *propSymbol = symbol->property;
            return propSymbol->node;
        }
        case METHOD_SYMBOL:
        {
            MethodSymbol *methodSymbol = symbol->method;
            return methodSymbol->node;
        }
        default:
            std::cerr << "Error: Failed to get AST node from symbol, symbol type not recognized!" << std::endl;
            return nullptr;
        }

        std::cerr << "Error: Failed to get AST node from symbol, symbol type not recognized!" << std::endl;
        return nullptr;
    }

} // namespace Cryo
