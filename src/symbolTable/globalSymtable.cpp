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
    bool GlobalSymbolTable::getIsPrimaryTable(void)
    {
        return tableContext.isPrimary;
    }
    bool GlobalSymbolTable::getIsDependencyTable(void)
    {
        return tableContext.isDependency;
    }
    void GlobalSymbolTable::setIsPrimaryTable(bool isPrimary)
    {
        tableContext.isPrimary = isPrimary;
        tableContext.isDependency = !isPrimary;
        tableState = TABLE_IN_PROGRESS;
    }
    void GlobalSymbolTable::setIsDependencyTable(bool isDependency)
    {
        tableContext.isDependency = isDependency;
        tableContext.isPrimary = !isDependency;
        tableState = TABLE_IN_PROGRESS;
    }
    void GlobalSymbolTable::resetCurrentDepsTable(void)
    {
        currentDependencyTable = nullptr;
    }
    void GlobalSymbolTable::setCurrentDependencyTable(SymbolTable *table)
    {
        resetCurrentDepsTable();
        currentDependencyTable = table;
    }
    void GlobalSymbolTable::setPrimaryTable(SymbolTable *table)
    {
        symbolTable = table;
    }
    void GlobalSymbolTable::addGlobalFunctionToTable(FunctionSymbol *function)
    {
        globalFunctions.push_back(function);
    }
    void GlobalSymbolTable::addExternFunctionToTable(ExternSymbol *function)
    {
        externFunctions.push_back(function);
    }
    void GlobalSymbolTable::mergeDBChunks(void)
    {
        db->createScopedDB();
    }

    // ========================================================================

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

    void GlobalSymbolTable::initNamespace(const char *namespaceName)
    {
        if (!namespaceName || namespaceName == nullptr)
        {
            return;
        }

        if (tableState == TABLE_COMPLETE)
        {
            tableState = TABLE_IN_PROGRESS;
        }

        const char *scopeID = IDGen::generate64BitHashID(namespaceName);
        setScopeID(scopeID);
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

    void GlobalSymbolTable::addSingleSymbolToTable(Symbol *symbol, SymbolTable *table)
    {
        if (!symbol || symbol == nullptr)
        {
            std::cout << "addSingleSymbolToTable: Symbol is null" << std::endl;
            return;
        }
        if (!table || table == nullptr)
        {
            std::cout << "addSingleSymbolToTable: Symbol Table is null" << std::endl;
            return;
        }

        // Check if the symbol exists in the table
        if (doesSymbolExist(symbol, table))
        {
            std::cout << "addSingleSymbolToTable: Symbol already exists in table" << std::endl;
            return;
        }

        table->symbols[table->count] = symbol;
        table->count++;

        const char *symbolName = getSymbolName(symbol);
        std::cout << "GST: Symbol Added: " << symbolName << std::endl;

        return;
    }

    void GlobalSymbolTable::addSymbolsToSymbolTable(Symbol **symbols, SymbolTable *table)
    {
        if (!symbols || symbols == nullptr)
        {
            std::cout << "addSymbolsToSymbolTable: Symbols are null" << std::endl;
            return;
        }
        if (!table || table == nullptr)
        {
            std::cout << "addSymbolsToSymbolTable: Symbol Table is null" << std::endl;
            return;
        }

        if (table->count + 1 >= table->capacity)
        {
            table->capacity *= 2;
            table->symbols = (Symbol **)realloc(table->symbols, sizeof(Symbol *) * table->capacity);
        }

        for (size_t i = 0; i < table->count; i++)
        {
            table->symbols[table->count] = symbols[i];
            table->count++;
        }

        return;
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

        const char *varName = node->data.varDecl->name;

        VariableSymbol *variableSymbol = createVariableSymbol(varName,
                                                              node->data.varDecl->type,
                                                              node,
                                                              scopeID);
        Symbol *symbol = createSymbol(VARIABLE_SYMBOL, variableSymbol);
        addSingleSymbolToTable(symbol, getCurrentSymbolTable());
    }

    void GlobalSymbolTable::addParamToSymbolTable(ASTNode *node, const char *functionScopeID)
    {
        if (!node || node == nullptr)
        {
            std::cout << "addParamToSymbolTable: Node is null" << std::endl;
            return;
        }

        const char *paramName = node->data.param->name;

        VariableSymbol *paramSymbol = createVariableSymbol(paramName,
                                                           node->data.param->type,
                                                           node,
                                                           functionScopeID);
        Symbol *symbol = createSymbol(VARIABLE_SYMBOL, paramSymbol);
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
                    if (strcmp(funcSymbol->name, name) == 0 && strcmp(funcSymbol->functionScopeId, scopeID) == 0)
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

    Symbol *GlobalSymbolTable::querySpecifiedTable(const char *symbolName, TypeOfSymbol symbolType, SymbolTable *table)
    {
        if (!symbolName || symbolName == nullptr)
        {
            return nullptr;
        }

        if (!table || table == nullptr)
        {
            return nullptr;
        }

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
                    if (strcmp(varSymbol->name, symbolName) == 0)
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
                    if (strcmp(funcSymbol->name, symbolName) == 0)
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
                    if (strcmp(typeSymbol->name, symbolName) == 0)
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
                    if (strcmp(propSymbol->name, symbolName) == 0)
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
                    if (strcmp(methodSymbol->name, symbolName) == 0)
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

    // ========================================================================

    bool GlobalSymbolTable::doesSymbolExist(Symbol *symbol, SymbolTable *table)
    {
        if (!symbol || symbol == nullptr)
        {
            std::cout << "doesSymbolExist: Symbol is null" << std::endl;
            return false;
        }
        if (!table || table == nullptr)
        {
            std::cout << "doesSymbolExist: Symbol Table is null" << std::endl;
            return false;
        }

        for (size_t i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == symbol)
            {
                return true;
            }
        }

        return false;
    }

    const char *GlobalSymbolTable::getSymbolName(Symbol *symbol)
    {
        TypeOfSymbol symbolType = symbol->symbolType;
        switch (symbolType)
        {
        case VARIABLE_SYMBOL:
            return symbol->variable->name;
        case FUNCTION_SYMBOL:
            return symbol->function->name;
        case EXTERN_SYMBOL:
            return symbol->externSymbol->name;
        case TYPE_SYMBOL:
            return symbol->type->name;
        case PROPERTY_SYMBOL:
            return symbol->property->name;
        case METHOD_SYMBOL:
            return symbol->method->name;
        default:
            return "<unknown_symbol_type>";
        }
    }

} // namespace Cryo
