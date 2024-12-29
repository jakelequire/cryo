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
    // ========================================================================
    // Function Declaration Functions

    void GlobalSymbolTable::initFunctionDeclaration(const char *functionName, const char *parentScopeID, ASTNode **params, size_t paramCount, DataType *returnType)
    {
        if (!functionName || functionName == nullptr)
        {
            std::cout << "initFunctionDeclaration: Function Name is null" << std::endl;
            return;
        }

        if (!parentScopeID || parentScopeID == nullptr)
        {
            std::cout << "initFunctionDeclaration: Parent Scope ID is null" << std::endl;
            return;
        }

        DataType **paramTypes = (DataType **)malloc(sizeof(DataType *) * paramCount);
        int i = 0;
        for (i = 0; i < paramCount; i++)
        {
            paramTypes[i] = getDataTypeFromASTNode(params[i]);
            addParamToSymbolTable(params[i], parentScopeID);
        }

        FunctionSymbol *functionSymbol = createIncompleteFunctionSymbol(functionName, parentScopeID, returnType, paramTypes, paramCount);
        Symbol *symbol = createSymbol(FUNCTION_SYMBOL, functionSymbol);

        // Add the symbol to the current table
        addSingleSymbolToTable(symbol, getCurrentSymbolTable());

        return;
    }

    void GlobalSymbolTable::completeFunctionDeclaration(ASTNode *functionNode, const char *scopeID, const char *parentScopeID)
    {
        if (!functionNode || functionNode == nullptr)
        {
            std::cout << "completeFunctionDeclaration: Function Node is null" << std::endl;
            return;
        }

        if (!scopeID || scopeID == nullptr)
        {
            std::cout << "completeFunctionDeclaration: Scope ID is null" << std::endl;
            return;
        }

        if (!parentScopeID || parentScopeID == nullptr)
        {
            std::cout << "completeFunctionDeclaration: Parent Scope ID is null" << std::endl;
            return;
        }

        CryoVisibilityType visibility = functionNode->data.functionDecl->visibility;

        const char *functionName = functionNode->data.functionDecl->name;

        // Find the existing function symbol and finish it
        Symbol *symbol = resolveFunctionSymbol(functionName, parentScopeID, FUNCTION_SYMBOL);
        if (!symbol)
        {
            std::cout << "completeFunctionDeclaration: Function Symbol not found" << std::endl;
            return;
        }

        FunctionSymbol *functionSymbol = symbol->function;
        functionSymbol->node = functionNode;

        // Update the function symbol in the table
        updateFunctionSymbol(symbol, getCurrentSymbolTable());

        return;
    }

    void GlobalSymbolTable::addExternFunctionToTable(ASTNode *externNode, const char *namespaceScopeID)
    {
        if (!externNode || externNode == nullptr)
        {
            std::cout << "addExternFunctionToTable: Extern Node is null" << std::endl;
            return;
        }

        ASTNode **params = externNode->data.externFunction->params;
        int paramCount = externNode->data.externFunction->paramCount;
        DataType **paramTypes = (DataType **)malloc(sizeof(DataType *) * paramCount);
        int i = 0;
        for (i = 0; i < paramCount; i++)
        {
            paramTypes[i] = getDataTypeFromASTNode(params[i]);
        }

        const char *externName = externNode->data.externFunction->name;
        const char *scopeID = getScopeID();

        ExternSymbol *externSymbol = createExternSymbol(externName,
                                                        externNode->data.externFunction->type,
                                                        paramTypes,
                                                        externNode->data.externFunction->paramCount,
                                                        externNode->metaData->type,
                                                        VISIBILITY_PUBLIC,
                                                        namespaceScopeID);

        Symbol *symbol = createSymbol(EXTERN_SYMBOL, externSymbol);
        addSingleSymbolToTable(symbol, getCurrentSymbolTable());

        return;
    }

    // ========================================================================
    // Function Resolution Functions

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
            symbol = querySpecifiedTable(symbolName, EXTERN_SYMBOL, depTable);
            if (symbol != nullptr)
            {
                std::cout << "Extern Function Symbol Resolved in Dependency Table!" << std::endl;
                return symbol;
            }
            else
            {
                continue;
            }
        }

        // Check the primary tables
        SymbolTable *primaryTable = getPrimaryTable();
        symbol = querySpecifiedTable(symbolName, FUNCTION_SYMBOL, primaryTable);
        if (symbol != nullptr)
        {
            std::cout << "Function Symbol Resolved in Primary Table!" << std::endl;
            return symbol;
        }
        symbol = querySpecifiedTable(symbolName, EXTERN_SYMBOL, primaryTable);
        if (symbol != nullptr)
        {
            std::cout << "Extern Function Symbol Resolved in Primary Table!" << std::endl;
            return symbol;
        }

        std::cout << "<!> Function Symbol not found in any table: " << symbolName << " <!>" << std::endl;
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

    // ========================================================================

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

    void GlobalSymbolTable::updateFunctionSymbol(Symbol *functionSymbol, SymbolTable *table)
    {
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == functionSymbol)
            {
                table->symbols[i] = functionSymbol;
                return;
            }
        }
    }

} // namespace Cryo