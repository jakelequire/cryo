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

    void GlobalSymbolTable::createPrimaryTable(const char *namespaceName)
    {
        SymbolTable *symbolTable = createSymbolTable(namespaceName);
        if (symbolTable)
        {
            setIsPrimaryTable(symbolTable);
            setPrimaryTable(symbolTable);
            initNamepsaceScope(namespaceName);
            return;
        }
        return;
    }

    void GlobalSymbolTable::initDependencyTable(const char *namespaceName)
    {
        SymbolTable *table = createSymbolTable(namespaceName);
        if (table)
        {
            if (dependencyCount == 0)
            {
                setIsDependencyTable(table);
                setCurrentDependencyTable(table);
                dependencyTableVector.push_back(table);
                dependencyCount++;
                initNamepsaceScope(namespaceName);
                return;
            }
            else
            {
                dependencyTableVector.push_back(table);
                dependencyCount++;
                setCurrentDependencyTable(table);
                initNamepsaceScope(namespaceName);
                return;
            }
        }

        return;
    }

    void GlobalSymbolTable::addNodeToTable(ASTNode *node)
    {
        if (!node || node == nullptr)
        {
            return;
        }

        // Check wether we are in the primary table or a dependency table
        if (tableContext.isPrimary)
        {
            std::cout << "Adding to Primary Table" << std::endl;
            // Add to primary table
            if (symbolTable && symbolTable != nullptr)
            {
                // Add the node to the primary table
                Symbol *symbol = ASTNodeToSymbol(node);
                if (symbol)
                {
                    // Add the symbol to the symbol table
                    addSingleSymbolToTable(symbol, symbolTable);
                    return;
                }
                return;
            }
        }
        // Check wether we are in the dependency table
        else if (tableContext.isDependency)
        {
            std::cout << "Adding to Dependency Table" << std::endl;
            // Add to dependency table
            if (currentDependencyTable && currentDependencyTable != nullptr)
            {
                // Add the node to the dependency table
                Symbol *symbol = ASTNodeToSymbol(node);
                if (symbol)
                {
                    // Add the symbol to the dependency table
                    addSingleSymbolToTable(symbol, currentDependencyTable);
                    return;
                }
                return;
            }
        }
    }

    void GlobalSymbolTable::completeDependencyTable()
    {
        if (currentDependencyTable)
        {
            // Clear the current dependency table, add to the dependency table vector
            currentDependencyTable = nullptr;
            return;
        }
    }

    void GlobalSymbolTable::addVariableToSymbolTable(ASTNode *node, const char *scopeID)
    {
        if (!node || node == nullptr)
        {
            std::cout << "addVariableToSymbolTable: Node is null" << std::endl;
            return;
        }

        VariableSymbol *variableSymbol = createVariableSymbol(node->data.varDecl->name,
                                                              node->data.varDecl->type,
                                                              node,
                                                              scopeID);
        Symbol *symbol = createSymbol(VARIABLE_SYMBOL, variableSymbol);
        addSingleSymbolToTable(symbol, getCurrentSymbolTable());
    }

    SymbolTable *GlobalSymbolTable::getCurrentSymbolTable(void)
    {
        if (tableContext.isPrimary)
        {
            return symbolTable;
        }
        else if (tableContext.isDependency)
        {
            return currentDependencyTable;
        }
        return nullptr;
    }

    void GlobalSymbolTable::tableFinished(void)
    {
        tableState = TABLE_COMPLETE;
        if (tableContext.isPrimary)
        {
            if (symbolTable)
            {
                db->appendSerializedTable(symbolTable);
            }
        }
        else if (tableContext.isDependency)
        {
            if (currentDependencyTable && dependencyCount == 1)
            {
                db->serializeSymbolTable(currentDependencyTable);
            }
        }
        else
        {
            tableState = TABLE_ERROR;
        }
    }

    void GlobalSymbolTable::addSymbolToCurrentTable(Symbol *symbol)
    {
        if (tableContext.isPrimary)
        {
            addSingleSymbolToTable(symbol, symbolTable);
        }
        else if (tableContext.isDependency)
        {
            addSingleSymbolToTable(symbol, currentDependencyTable);
        }
    }

    Symbol *GlobalSymbolTable::queryCurrentTable(const char *scopeID, const char *name, TypeOfSymbol symbolType)
    {
        if (!scopeID || scopeID == nullptr)
        {
            return nullptr;
        }

        if (!name || name == nullptr)
        {
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            return nullptr;
        }

        // DEBUG
        std::cout << "DEBUG: Querying Table" << std::endl;
        SymbolTableDebugger::logSymbolTable(table);

        int symbolCount = table->count;
        Symbol **symbols = table->symbols;

        for (int i = 0; i < symbolCount; i++)
        {
            if (symbols[i]->symbolType == symbolType)
            {
                Symbol *symbol = symbols[i];
                switch (symbolType)
                {
                case VARIABLE_SYMBOL:
                {
                    VariableSymbol *varSymbol = symbol->variable;
                    if (strcmp(varSymbol->name, name) == 0 && strcmp(varSymbol->scopeId, scopeID) == 0)
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
                    if (strcmp(funcSymbol->name, name) == 0 && strcmp(funcSymbol->scopeId, scopeID) == 0)
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
                    if (strcmp(externSymbol->name, name) == 0 && strcmp(externSymbol->scopeId, scopeID) == 0)
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
                    std::cout << "Querying Type Symbol" << std::endl;
                    TypeSymbol *typeSymbol = symbol->type;
                    if (strcmp(typeSymbol->name, name) == 0 && strcmp(typeSymbol->scopeId, scopeID) == 0)
                    {
                        std::cout << "Type Symbol Resolved! " << typeSymbol->name << std::endl;
                        return symbol;
                    }
                    else
                    {
                        std::cout << "Type Symbol not found: " << name << std::endl;
                        continue;
                    }
                }
                case PROPERTY_SYMBOL:
                {
                    PropertySymbol *propSymbol = symbol->property;
                    if (strcmp(propSymbol->name, name) == 0 && strcmp(propSymbol->scopeId, scopeID) == 0)
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
                    if (strcmp(methodSymbol->name, name) == 0 && strcmp(methodSymbol->scopeId, scopeID) == 0)
                    {
                        return symbol;
                    }
                    else
                    {
                        continue;
                    }
                }
                default:
                    break;
                }
            }
            else
            {
                continue;
            }
        }

        return nullptr;
    }

} // namespace Cryo
