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
    void GlobalSymbolTable::cleanupAndDestroy(void)
    {
        if (symbolTable)
        {
            std::cout << "Deleting primary table" << std::endl;
            delete symbolTable;
        }
        if (currentDependencyTable)
        {
            std::cout << "Deleting current dependency table" << std::endl;
            delete currentDependencyTable;
        }
        if (reapedTable)
        {
            std::cout << "Deleting reaped table" << std::endl;
            delete reapedTable;
        }
        for (size_t i = 0; i < dependencyTableVector.size(); i++)
        {
            std::cout << "Deleting dependency table: " << i << std::endl;
            delete dependencyTableVector[i];
        }
        for (size_t i = 0; i < globalFunctions.size(); i++)
        {
            std::cout << "Deleting global function: " << i << std::endl;
            delete globalFunctions[i];
        }
        for (size_t i = 0; i < externFunctions.size(); i++)
        {
            std::cout << "Deleting extern function: " << i << std::endl;
            delete externFunctions[i];
        }
    }

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
        if (isForReaping)
        {
            return;
        }
        else
        {
            // db->createScopedDB();
        }
    }
    const char *GlobalSymbolTable::getDependencyDirStr(void)
    {
        return debugInfo.dependencyDir.c_str();
    }
    void GlobalSymbolTable::addSymbolToTable(Symbol *symbol, SymbolTable *table)
    {
        if (!symbol || symbol == nullptr)
        {
            std::cerr << "Error: Failed to add symbol to table, symbol is null!" << std::endl;
            return;
        }
        if (!table || table == nullptr)
        {
            std::cerr << "Error: Failed to add symbol to table, table is null!" << std::endl;
            return;
        }
        addSingleSymbolToTable(symbol, table);
    }

    void GlobalSymbolTable::pushNewDependencyTable(SymbolTable *table)
    {
        if (!table || table == nullptr)
        {
            std::cerr << "Error: Failed to push new dependency table, table is null!" << std::endl;
            return;
        }

        // We should check if the table is already in the vector and pointer array
        if (std::find(dependencyTableVector.begin(), dependencyTableVector.end(), table) != dependencyTableVector.end())
        {
            std::cout << "Dependency Table already exists in vector" << std::endl;
            return;
        }

        dependencyTableVector.push_back(table);

        if (dependencyCount == 0)
        {
            dependencyTables[0] = table;
            dependencyCount++;
            return;
        }
        else
        {
            dependencyTables[dependencyCount] = table;
            dependencyCount++;
            return;
        }
    }

    void GlobalSymbolTable::pushNewScopePair(const char *name, const char *id)
    {
        // Make a new pair
        std::pair<std::string, std::string> pair = std::make_pair(name, id);
        // Make sure the pair is not already in the vector
        if (std::find(scopeLookup.begin(), scopeLookup.end(), pair) != scopeLookup.end())
        {
            return;
        }
        // Add the pair to the vector
        scopeLookup.push_back(pair);
    }

    const char *GlobalSymbolTable::getScopeIDFromName(const char *name)
    {
        for (size_t i = 0; i < scopeLookup.size(); i++)
        {
            if (scopeLookup[i].first == name)
            {
                return scopeLookup[i].second.c_str();
            }
        }
        return nullptr;
    }

    const char *GlobalSymbolTable::getScopeIDFromID(const char *id)
    {
        for (size_t i = 0; i < scopeLookup.size(); i++)
        {
            if (scopeLookup[i].second == id)
            {
                return scopeLookup[i].first.c_str();
            }
        }
        return nullptr;
    }

    // Looks through all symbol tables to find the name that matches the symbol and returns it's symbol type
    TypeOfSymbol GlobalSymbolTable::getScopeSymbolTypeFromName(const char *symbolName)
    {
        if (!symbolName || symbolName == nullptr)
        {
            return UNKNOWN_SYMBOL;
        }

        std::cout << "<!> Searching for Symbol: " << symbolName << " <!>" << std::endl;

        // Check the primary table
        if (symbolTable)
        {
            Symbol **symbols = symbolTable->symbols;
            int i = 0;
            for (i = 0; i < symbolTable->count; i++)
            {
                Symbol *symbol = symbols[i];
                switch (symbol->symbolType)
                {
                case FUNCTION_SYMBOL:
                {
                    FunctionSymbol *funcSymbol = symbol->function;
                    std::cout << "Checking Function Symbol: " << funcSymbol->name << " | Against: " << symbolName << std::endl;
                    if (strcmp(funcSymbol->name, symbolName) == 0)
                    {
                        return FUNCTION_SYMBOL;
                    }
                    break;
                }
                case TYPE_SYMBOL:
                {
                    TypeSymbol *typeSymbol = symbol->type;
                    std::cout << "Checking Type Symbol: " << typeSymbol->name << " | Against: " << symbolName << std::endl;
                    if (strcmp(typeSymbol->name, symbolName) == 0)
                    {
                        return TYPE_SYMBOL;
                    }
                    break;
                }
                default:
                {
                    break;
                }
                }
            }
        }

        // Check the dependency tables
        int depCount = dependencyTableVector.size();
        for (int i = 0; i < depCount; i++)
        {
            SymbolTable *depTable = dependencyTableVector[i];
            Symbol **symbols = depTable->symbols;
            int j = 0;
            for (j = 0; j < depTable->count; j++)
            {
                Symbol *symbol = symbols[j];
                switch (symbol->symbolType)
                {
                case FUNCTION_SYMBOL:
                {
                    FunctionSymbol *funcSymbol = symbol->function;
                    if (strcmp(funcSymbol->name, symbolName) == 0)
                    {
                        return FUNCTION_SYMBOL;
                    }
                    break;
                }
                case TYPE_SYMBOL:
                {
                    TypeSymbol *typeSymbol = symbol->type;
                    if (strcmp(typeSymbol->name, symbolName) == 0)
                    {
                        return TYPE_SYMBOL;
                    }
                    break;
                }
                default:
                {
                    break;
                }
                }
            }
        }

        return UNKNOWN_SYMBOL;
    }

    TypeofDataType GlobalSymbolTable::getTypeOfDataTypeFromName(const char *symbolName)
    {
        if (!symbolName || symbolName == nullptr)
        {
            std::cerr << "Error: Failed to get type of data type from name, symbol name is null!" << std::endl;
            return UNKNOWN_TYPE;
        }

        std::cout << "<!> Searching for Data Type: " << symbolName << " <!>" << std::endl;

        // Check the primary table
        if (symbolTable)
        {
            Symbol **symbols = symbolTable->symbols;
            int i = 0;
            for (i = 0; i < symbolTable->count; i++)
            {
                Symbol *symbol = symbols[i];
                switch (symbol->symbolType)
                {
                case TYPE_SYMBOL:
                {
                    TypeSymbol *typeSymbol = symbol->type;
                    if (strcmp(typeSymbol->name, symbolName) == 0)
                    {
                        return typeSymbol->typeOf;
                    }
                    break;
                }
                default:
                {
                    break;
                }
                }
            }
        }

        // Check the dependency tables
        int depCount = dependencyTableVector.size();
        for (int i = 0; i < depCount; i++)
        {
            SymbolTable *depTable = dependencyTableVector[i];
            Symbol **symbols = depTable->symbols;
            int j = 0;
            for (j = 0; j < depTable->count; j++)
            {
                Symbol *symbol = symbols[j];
                switch (symbol->symbolType)
                {
                case TYPE_SYMBOL:
                {
                    TypeSymbol *typeSymbol = symbol->type;
                    if (strcmp(typeSymbol->name, symbolName) == 0)
                    {
                        return typeSymbol->typeOf;
                    }
                    break;
                }
                default:
                {
                    break;
                }
                }
            }
        }

        return UNKNOWN_TYPE;
    }

    const char *GlobalSymbolTable::typeOfSymbolToString(TypeOfSymbol symbolType)
    {
        switch (symbolType)
        {
        case VARIABLE_SYMBOL:
            return "VARIABLE_SYMBOL";
        case FUNCTION_SYMBOL:
            return "FUNCTION_SYMBOL";
        case EXTERN_SYMBOL:
            return "EXTERN_SYMBOL";
        case TYPE_SYMBOL:
            return "TYPE_SYMBOL";
        case PROPERTY_SYMBOL:
            return "PROPERTY_SYMBOL";
        case METHOD_SYMBOL:
            return "METHOD_SYMBOL";
        case UNKNOWN_SYMBOL:
            return "UNKNOWN_SYMBOL";
        default:
            return "DEFAULTED_SYMBOL";
        }
    }

    void GlobalSymbolTable::importReapedTable(SymbolTable *table)
    {
        if (!table || table == nullptr)
        {
            std::cerr << "Error: Failed to import reaped table, table is null!" << std::endl;
            return;
        }

        pushNewDependencyTable(table);

        std::cout << "Imported Reaped Table" << std::endl;
    }

    SymbolTable *GlobalSymbolTable::getSpecificSymbolTable(const char *name)
    {
        if (!name || name == nullptr)
        {
            std::cerr << "Error: Failed to get specific symbol table, name is null!" << std::endl;
            return nullptr;
        }

        for (size_t i = 0; i < dependencyTableVector.size(); i++)
        {
            SymbolTable *table = dependencyTableVector[i];
            if (table->namespaceName == name)
            {
                std::cout << "Found Specific Table: " << name << std::endl;
                return table;
            }
        }

        std::cerr << "Error: Failed to get specific symbol table, table not found!" << std::endl;
        return nullptr;
    }

    void GlobalSymbolTable::importRuntimeSymbols(SymbolTable *table)
    {
        if (!table || table == nullptr)
        {
            std::cerr << "Error: Failed to import runtime symbols, table is null!" << std::endl;
            return;
        }
        if (table->namespaceName != "runtime")
        {
            std::cerr << "Error: Failed to import runtime symbols, table is not runtime!" << std::endl;
            return;
        }

        
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

    void GlobalSymbolTable::addNewDependencyTable(const char *namespaceName, SymbolTable *table)
    {
        if (!table || table == nullptr)
        {
            return;
        }

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

        this->currentNamespace = namespaceName;

        this->pushNewScopePair(namespaceName, scopeID);
    }

    void GlobalSymbolTable::completeFrontend(void)
    {
        if (isForReaping)
        {
            completeDependencyTable();
            std::cout << "Dependency Table Pointer Array Count: " << dependencyCount << std::endl;
            SymbolTable *fullTable = mergeAllSymbols();
            std::cout << "<!> DEBUG: Merged Full Table <!>" << std::endl;
        }
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
            dependencyTableVector.push_back(currentDependencyTable);
            currentDependencyTable = nullptr;
            dependencyCount++;
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
                // db->appendSerializedTable(symbolTable);
            }
        }
        else if (tableContext.isDependency)
        {
            if (currentDependencyTable && dependencyCount == 1)
            {
                // db->serializeSymbolTable(currentDependencyTable);
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

    TypesTable *GlobalSymbolTable::initTypeTable(const char *namespaceName)
    {
        TypesTable *typeTable = createTypeTable(namespaceName);
        if (typeTable)
        {
            return typeTable;
        }

        typeTable->types = (DataType **)malloc(sizeof(DataType *) * typeTable->capacity);

        std::cerr << "initTypeTable: Failed to create Type Table" << std::endl;
        return nullptr;
    }

    Symbol *GlobalSymbolTable::wrapSubSymbol(TypeOfSymbol symbolType, void *symbol)
    {
        if (!symbol || symbol == nullptr)
        {
            std::cerr << "wrapSubSymbol: Symbol is null" << std::endl;
            return nullptr;
        }

        Symbol *subSymbol = createSymbol(symbolType, symbol);
        if (subSymbol)
        {
            return subSymbol;
        }
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
