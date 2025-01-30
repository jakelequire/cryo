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
    VariableSymbol *GlobalSymbolTable::getFrontendVariableSymbol(const char *name, const char *scopeID)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
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
                    logMessage(LMI, "INFO", "SymbolTable", "Variable Symbol found");
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

        logMessage(LMI, "ERROR", "SymbolTable", "Variable Symbol not found");
        return nullptr;
    }

    MethodSymbol *GlobalSymbolTable::getFrontendMethodSymbol(const char *methodName, const char *className, const char *scopeID)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        if (!methodName || methodName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Method Name is null");
            return nullptr;
        }

        if (!className || className == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Class Name is null");
            return nullptr;
        }

        if (!scopeID || scopeID == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Scope ID is null");
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
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
                    logMessage(LMI, "INFO", "SymbolTable", "Method Symbol found");
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

        logMessage(LMI, "ERROR", "SymbolTable", "Method Symbol not found");
        return nullptr;
    }

    Symbol *GlobalSymbolTable::getFrontendSymbol(const char *symbolName, const char *scopeID, TypeOfSymbol symbolType)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        if (!symbolName || symbolName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Name is null");
            return nullptr;
        }

        if (!scopeID || scopeID == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Scope ID is null");
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
                    logMessage(LMI, "INFO", "SymbolTable", "Variable Symbol found: %s", symbolName);
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
                    logMessage(LMI, "INFO", "SymbolTable", "Function Symbol found: %s", symbolName);
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
                    logMessage(LMI, "INFO", "SymbolTable", "Extern Symbol found: %s", symbolName);
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
                    logMessage(LMI, "INFO", "SymbolTable", "Type Symbol found: %s", symbolName);
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
                    logMessage(LMI, "INFO", "SymbolTable", "Property Symbol found: %s", symbolName);
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
                    logMessage(LMI, "INFO", "SymbolTable", "Method Symbol found: %s", symbolName);
                    return symbol;
                }
                else
                {
                    DEBUG_BREAKPOINT;
                }
            }
            default:
                logMessage(LMI, "ERROR", "SymbolTable", "Symbol Type not recognized");
                return nullptr;
            }
        }
    }

    Symbol *GlobalSymbolTable::findSymbol(const char *symbolName, const char *scopeID)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        if (!symbolName || symbolName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Name is null");
            return nullptr;
        }
        if (!scopeID || scopeID == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Scope ID is null");
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
            return nullptr;
        }

        int symbolCount = table->count;
        Symbol **symbols = table->symbols;

        this->printScopeLookup();

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
                    logMessage(LMI, "INFO", "SymbolTable", "Variable Symbol found");
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
                    logMessage(LMI, "INFO", "SymbolTable", "Function Symbol found");
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
                    logMessage(LMI, "INFO", "SymbolTable", "Extern Symbol found: %s", symbolName);
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
                    logMessage(LMI, "INFO", "SymbolTable", "Type Symbol found: %s", symbolName);
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
                    logMessage(LMI, "INFO", "SymbolTable", "Property Symbol found: %s", symbolName);
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
                    logMessage(LMI, "INFO", "SymbolTable", "Method Symbol found: %s", symbolName);
                    return symbol;
                }
                else
                {
                    continue;
                }
            }
            default:
                logMessage(LMI, "ERROR", "SymbolTable", "Symbol Type not recognized");
                return nullptr;
            }
        }

        logMessage(LMI, "ERROR", "SymbolTable", "Symbol not found: %s", symbolName);
        return nullptr;
    }

    Symbol *GlobalSymbolTable::findMethodSymbol(const char *methodName, const char *className, TypeofDataType typeOfNode)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        if (!methodName || methodName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Method Name is null");
            return nullptr;
        }

        if (!className || className == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Class Name is null");
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
            return nullptr;
        }

        int symbolCount = table->count;
        Symbol **symbols = table->symbols;

        size_t typeCount = typeTable->count;
        if (typeCount == 0)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Type Table is empty");
            return nullptr;
        }

        for (int i = 0; i < typeCount; i++)
        {
            TypeSymbol *typeSymbol = typeTable->typeSymbols[i];
            if (strcmp(typeSymbol->name, className) == 0)
            {
                int methodCount = typeSymbol->methodCount;
                for (int j = 0; j < methodCount; j++)
                {
                    MethodSymbol *methodSymbol = typeSymbol->methods[j]->method;
                    if (strcmp(methodSymbol->name, methodName) == 0)
                    {
                        logMessage(LMI, "INFO", "SymbolTable", "Method Symbol found: %s", methodName);
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

        logMessage(LMI, "ERROR", "SymbolTable", "Method Symbol not found: %s", methodName);
        return nullptr;
    }

    Symbol *GlobalSymbolTable::seekMethodSymbolInAllTables(const char *methodName, const char *className, TypeofDataType typeOfNode)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        if (!methodName || methodName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Method Name is null");
            return nullptr;
        }

        if (!className || className == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Class Name is null");
            return nullptr;
        }

        Symbol *symbol = nullptr;

        // Check the dependency tables
        int depCount = dependencyTableVector.size();
        for (int i = 0; i < depCount; i++)
        {
            SymbolTable *depTable = dependencyTableVector[i];
            symbol = querySpecifiedTable(methodName, TYPE_SYMBOL, depTable);
            if (symbol != nullptr)
            {
                TypeSymbol *typeSymbol = symbol->type;
                for (int j = 0; j < typeSymbol->methodCount; j++)
                {
                    MethodSymbol *methodSymbol = typeSymbol->methods[j]->method;
                    if (strcmp(methodSymbol->name, methodName) == 0)
                    {
                        logMessage(LMI, "INFO", "SymbolTable", "Method Symbol found: %s", methodName);
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

        logMessage(LMI, "ERROR", "SymbolTable", "Method Symbol not found: %s", methodName);
        return nullptr;
    }

    Symbol *GlobalSymbolTable::resolveExternSymbol(const char *symbolName)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        // Look at the extern functions in `externFunctions` and resolve the symbol
        // if it exists.

        if (!symbolName || symbolName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Name is null");
            return nullptr;
        }

        int externCount = externFunctions.size();
        for (int i = 0; i < externCount; i++)
        {
            ExternSymbol *externSymbol = externFunctions[i];
            if (strcmp(externSymbol->name, symbolName) == 0)
            {
                logMessage(LMI, "INFO", "SymbolTable", "Extern Symbol found: %s", symbolName);
                return createSymbol(EXTERN_SYMBOL, externSymbol);
            }
            else
            {
                continue;
            }
        }

        logMessage(LMI, "ERROR", "SymbolTable", "Extern Symbol not found: %s", symbolName);
        return nullptr;
    }

    SymbolTable *GlobalSymbolTable::findSymbolTable(const char *scopeID)
    {
        if (!scopeID || scopeID == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Scope ID is null");
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
            return nullptr;
        }

        if (strcmp(table->scopeId, scopeID) == 0)
        {
            logMessage(LMI, "INFO", "SymbolTable", "Symbol Table found");
            return table;
        }

        // If it is not the current table, check the dependency tables
        int depCount = dependencyTableVector.size();
        for (int i = 0; i < depCount; i++)
        {
            SymbolTable *depTable = dependencyTableVector[i];
            if (strcmp(depTable->scopeId, scopeID) == 0)
            {
                logMessage(LMI, "INFO", "SymbolTable", "Dependency Table found");
                return depTable;
            }
            else
            {
                continue;
            }
        }

        logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table not found");
        return nullptr;
    }

    DataType *GlobalSymbolTable::getDataTypeFromSymbol(Symbol *symbol)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        if (!symbol || symbol == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol is null");
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
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Type not recognized");
            return nullptr;
        }

        logMessage(LMI, "ERROR", "SymbolTable", "Symbol Type not recognized");
        return nullptr;
    }

    ASTNode *GlobalSymbolTable::getASTNodeFromSymbol(Symbol *symbol)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        if (!symbol || symbol == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol is null");
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
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Type not recognized");
            return nullptr;
        }

        logMessage(LMI, "ERROR", "SymbolTable", "Symbol Type not recognized");
        return nullptr;
    }

    Symbol *GlobalSymbolTable::queryCurrentTable(const char *scopeID, const char *name, TypeOfSymbol symbolType)
    {
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        if (!scopeID || scopeID == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Scope ID is null");
            return nullptr;
        }

        if (!name || name == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Name is null");
            return nullptr;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
            return nullptr;
        }

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
                        logMessage(LMI, "INFO", "SymbolTable", "Variable Symbol found: %s", name);
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
                        logMessage(LMI, "INFO", "SymbolTable", "Function Symbol found: %s", name);
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
                        logMessage(LMI, "INFO", "SymbolTable", "Extern Symbol found: %s", name);
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
                    if (strcmp(typeSymbol->name, name) == 0 && strcmp(typeSymbol->scopeId, scopeID) == 0)
                    {
                        logMessage(LMI, "INFO", "SymbolTable", "Type Symbol found: %s", name);
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
                    if (strcmp(propSymbol->name, name) == 0 && strcmp(propSymbol->scopeId, scopeID) == 0)
                    {
                        logMessage(LMI, "INFO", "SymbolTable", "Property Symbol found: %s", name);
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
                        logMessage(LMI, "INFO", "SymbolTable", "Method Symbol found: %s", name);
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
        GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
        if (!symbolName || symbolName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Name is null");
            return nullptr;
        }

        if (!table || table == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
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
                        logMessage(LMI, "INFO", "SymbolTable", "Variable Symbol found");
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
                        logMessage(LMI, "INFO", "SymbolTable", "Function Symbol found");
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
                        logMessage(LMI, "INFO", "SymbolTable", "Extern Symbol found");
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
                        logMessage(LMI, "INFO", "SymbolTable", "Type Symbol found");
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
                        logMessage(LMI, "INFO", "SymbolTable", "Property Symbol found");
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
                        logMessage(LMI, "INFO", "SymbolTable", "Method Symbol found");
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
