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
#include "tools/utils/c_logger.h"
#include "symbolTable/globalSymtable.hpp"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

namespace Cryo
{
    void GlobalSymbolTable::cleanupAndDestroy(void)
    {
        __STACK_FRAME__
        if (symbolTable)
        {
            logMessage(LMI, "INFO", "SymbolTable", "Deleting primary symbol table");
            delete symbolTable;
        }
        if (currentDependencyTable)
        {
            logMessage(LMI, "INFO", "SymbolTable", "Deleting current dependency table");
            delete currentDependencyTable;
        }
        if (reapedTable)
        {
            logMessage(LMI, "INFO", "SymbolTable", "Deleting reaped table");
            delete reapedTable;
        }
        dependencyTableVector.clear();
    }

    bool GlobalSymbolTable::getIsPrimaryTable(void)
    {
        __STACK_FRAME__
        return tableContext.isPrimary;
    }

    bool GlobalSymbolTable::getIsDependencyTable(void)
    {
        __STACK_FRAME__
        return tableContext.isDependency;
    }

    void GlobalSymbolTable::setLinker(CryoLinker *linker)
    {
        __STACK_FRAME__
        this->linker = linker;
    }

    CryoLinker *GlobalSymbolTable::getLinker(void)
    {
        __STACK_FRAME__
        return linker;
    }

    void GlobalSymbolTable::setIsPrimaryTable(bool isPrimary)
    {
        __STACK_FRAME__
        tableContext.isPrimary = isPrimary;
        tableContext.isDependency = !isPrimary;
        tableState = TABLE_IN_PROGRESS;
    }

    void GlobalSymbolTable::setIsDependencyTable(bool isDependency)
    {
        __STACK_FRAME__
        tableContext.isDependency = isDependency;
        tableContext.isPrimary = !isDependency;
        tableState = TABLE_IN_PROGRESS;
    }

    void GlobalSymbolTable::resetCurrentDepsTable(void)
    {
        __STACK_FRAME__
        currentDependencyTable = nullptr;
    }

    void GlobalSymbolTable::setCurrentDependencyTable(SymbolTable *table)
    {
        __STACK_FRAME__
        resetCurrentDepsTable();
        currentDependencyTable = table;
    }

    void GlobalSymbolTable::setPrimaryTable(SymbolTable *table)
    {
        __STACK_FRAME__
        symbolTable = table;
    }

    void GlobalSymbolTable::addGlobalFunctionToTable(FunctionSymbol *function)
    {
        __STACK_FRAME__
        globalFunctions.push_back(function);
    }

    void GlobalSymbolTable::addExternFunctionToTable(ExternSymbol *function)
    {
        __STACK_FRAME__
        externFunctions.push_back(function);
    }

    void GlobalSymbolTable::mergeDBChunks(void)
    {
        __STACK_FRAME__
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
        __STACK_FRAME__
        return debugInfo.dependencyDir.c_str();
    }

    const char *GlobalSymbolTable::getNamespace(void)
    {
        __STACK_FRAME__
        return this->currentNamespace;
    }

    void GlobalSymbolTable::addSymbolToTable(Symbol *symbol, SymbolTable *table)
    {
        __STACK_FRAME__
        if (!symbol || symbol == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to add symbol to table, symbol is null");
            return;
        }
        if (!table || table == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to add symbol to table, table is null");
            return;
        }
        addSingleSymbolToTable(symbol, table);
    }

    void GlobalSymbolTable::pushNewDependencyTable(SymbolTable *table)
    {
        __STACK_FRAME__
        if (!table || table == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to push new dependency table, table is null");
            return;
        }

        // We should check if the table is already in the vector and pointer array
        if (std::find(dependencyTableVector.begin(), dependencyTableVector.end(), table) != dependencyTableVector.end())
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to push new dependency table, table is already in the vector");
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

    void GlobalSymbolTable::pushTypeSymbols(TypesTable *importedTypesTable)
    {
        __STACK_FRAME__
        if (!importedTypesTable || importedTypesTable == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to push type symbols, imported types table is null");
            return;
        }

        // Get the type symbols from the imported types table
        TypeSymbol **typeSymbols = importedTypesTable->typeSymbols;
        size_t typeCount = importedTypesTable->count;

        // Push the type symbols to the global symbol table
        for (size_t i = 0; i < typeCount; i++)
        {
            TypeSymbol *typeSymbol = typeSymbols[i];
            addTypeToTable(typeSymbol);
        }

        return;
    }

    void GlobalSymbolTable::pushNewScopePair(const char *name, const char *id)
    {
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
        if (!symbolName || symbolName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to get scope symbol type from name, symbol name is null!");
            return UNKNOWN_SYMBOL;
        }

        logMessage(LMI, "INFO", "SymbolTable", "Checking for symbol: %s", symbolName);

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
                    logMessage(LMI, "INFO", "SymbolTable", "Checking Function Symbol: %s | Against: %s", funcSymbol->name, symbolName);
                    if (strcmp(funcSymbol->name, symbolName) == 0)
                    {
                        return FUNCTION_SYMBOL;
                    }
                    break;
                }
                case TYPE_SYMBOL:
                {
                    TypeSymbol *typeSymbol = symbol->type;
                    logMessage(LMI, "INFO", "SymbolTable", "Checking Type Symbol: %s | Against: %s", typeSymbol->name, symbolName);
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
        __STACK_FRAME__
        if (!symbolName || symbolName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to get type of data type from name, symbol name is null!");
            return UNKNOWN_TYPE;
        }

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
        __STACK_FRAME__
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
        __STACK_FRAME__
        if (!table || table == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to import reaped table, table is null");
            return;
        }

        pushNewDependencyTable(table);
        return;
    }

    void GlobalSymbolTable::importReapedTypesTable(TypesTable *reapedTable)
    {
        __STACK_FRAME__
        if (!reapedTable || reapedTable == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to import reaped types table, table is null");
            return;
        }

        pushTypeSymbols(reapedTable);
        return;
    }

    SymbolTable *GlobalSymbolTable::getSpecificSymbolTable(const char *name)
    {
        __STACK_FRAME__
        if (!name || name == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to get specific symbol table, name is null");
            return nullptr;
        }

        for (size_t i = 0; i < dependencyTableVector.size(); i++)
        {
            SymbolTable *table = dependencyTableVector[i];
            if (table->namespaceName == name)
            {
                logMessage(LMI, "INFO", "SymbolTable", "Found specific symbol table: %s", name);
                return table;
            }
        }
        return nullptr;
    }

    void GlobalSymbolTable::importRuntimeSymbols(SymbolTable *table)
    {
        __STACK_FRAME__
        if (!table || table == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to import runtime symbols, table is null");
            return;
        }
        if (table->namespaceName != "runtime")
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to import runtime symbols, table is not runtime");
            return;
        }
    }

    ASTNode **GlobalSymbolTable::mergeAllSymbolsToASTArray()
    {
        __STACK_FRAME__
        std::vector<Symbol *> primarySymbols = mergePrimaryTable();
        std::vector<Symbol *> dependencySymbols = mergeAllDependencyTables();

        std::vector<Symbol *> mergedSymbols = mergeTwoSymbolVectors(primarySymbols, dependencySymbols);

        SymbolTable *mergedTable = createNewSymbolTableFromSymbols(mergedSymbols);

        ASTNode **astArray = new ASTNode *[mergedTable->count];

        for (size_t i = 0; i < mergedTable->count; i++)
        {
            Symbol *symbol = mergedTable->symbols[i];
            ASTNode *node = getASTNodeFromSymbol(symbol);
            astArray[i] = node;
        }

        return astArray;
    }

    // ========================================================================

    void GlobalSymbolTable::createPrimaryTable(const char *namespaceName)
    {
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
        if (isForReaping)
        {
            completeDependencyTable();
            SymbolTable *fullTable = mergeAllSymbols();
        }
    }

    void GlobalSymbolTable::addNodeToTable(ASTNode *node)
    {
        __STACK_FRAME__
        if (!node || node == nullptr)
        {
            return;
        }

        // Check wether we are in the primary table or a dependency table
        if (tableContext.isPrimary)
        {
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
        __STACK_FRAME__
        if (!symbol || symbol == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol is null");
            return;
        }
        if (!table || table == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
            return;
        }

        // Check if the symbol exists in the table
        if (doesSymbolExist(symbol, table))
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol already exists in table");
            return;
        }

        table->symbols[table->count] = symbol;
        table->count++;

        const char *symbolName = getSymbolName(symbol);

        logMessage(LMI, "INFO", "SymbolTable", "Added symbol to table: %s", symbolName);

        return;
    }

    void GlobalSymbolTable::addSymbolsToSymbolTable(Symbol **symbols, SymbolTable *table)
    {
        __STACK_FRAME__
        if (!symbols || symbols == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbols are null");
            return;
        }
        if (!table || table == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
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
        __STACK_FRAME__
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
        __STACK_FRAME__
        if (!node || node == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Node is null");
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
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
        if (tableContext.isPrimary)
        {
            addSingleSymbolToTable(symbol, symbolTable);
        }
        else if (tableContext.isDependency)
        {
            addSingleSymbolToTable(symbol, currentDependencyTable);
        }
    }

    TypesTable *GlobalSymbolTable::initTypeTable(const char *namespaceName)
    {
        __STACK_FRAME__
        TypesTable *typeTable = createTypeTable(namespaceName);
        if (typeTable)
        {
            logMessage(LMI, "INFO", "SymbolTable", "Created Type Table");
            return typeTable;
        }

        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create Type Table");
        return nullptr;
    }

    Symbol *GlobalSymbolTable::wrapSubSymbol(TypeOfSymbol symbolType, void *symbol)
    {
        __STACK_FRAME__
        if (!symbol || symbol == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol is null");
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
        __STACK_FRAME__
        if (!symbol || symbol == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol is null");
            return false;
        }
        if (!table || table == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
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
        __STACK_FRAME__
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
