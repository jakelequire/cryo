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
#include "symbolTable/debugSymbols.hpp"

/*
New format to implement later for logging:

{Typeof Symbol} | Name: {Symbol Name} Type: {Symbol Type}
ID: {Scope ID} Depth: {INT} | Visibility: {Symbol Visibility}
{SYMBOL_SPECIFIC_DATA}
------------------------------------------------------------

*/

namespace Cryo
{
    // Define the static member
    Logger *SymbolTableDebugger::logger = new Logger(Logger::DEBUG);

    void SymbolTableDebugger::logScopeBlock(ScopeBlock *block)
    {
        if (!block)
        {
            logger->debugNode("NULL scope block");
            return;
        }

        logger->debugNode("Scope Block:");
        logger->debugNode("\tNamespace: %s", block->namespaceName ? block->namespaceName : "global");
        logger->debugNode("\tName: %s", block->name ? block->name : "<anonymous>");
        logger->debugNode("\tID: %s", block->id);
        logger->debugNode("\tDepth: %zu", block->depth);
        logger->debugNode("\tChild Count: %zu", block->childCount);

        if (block->parent)
        {
            logger->debugNode("\tParent Scope: %s (ID: %zu)",
                              block->parent->name ? block->parent->name : "<anonymous>",
                              block->parent->id);
        }
    }

    void SymbolTableDebugger::logVariableSymbol(VariableSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL variable symbol");
            return;
        }

        logger->debugNode("Variable Symbol:");
        logger->debugNode("\tName: %s", symbol->name);
        logger->debugNode("\tScope ID: %s", symbol->scopeId);
        logger->debugNode("\tType: %s", symbol->type ? DataTypeToString(symbol->type) : "unknown");
        logger->debugNode("\tAST Node: %p", (void *)symbol->node);
    }

    void SymbolTableDebugger::logFunctionSymbol(FunctionSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL function symbol");
            return;
        }

        logger->debugNode("Function Symbol:");
        logger->debugNode("\tName: %s", symbol->name);
        logger->debugNode("\tScope ID: %s", symbol->scopeId);
        logger->debugNode("\tFunction Type: %s",
                          symbol->returnType ? DataTypeToString(symbol->returnType) : "void");
        logger->debugNode("\tParameters: %zu", symbol->paramCount);

        // for (size_t i = 0; i < symbol->paramCount; i++)
        // {
        //     DataType *paramType = symbol->paramTypes[i];
        //     logger->debugNode("    Param %zu: %s", i, paramType ? DataTypeToString(paramType) : "unknown");
        // }

        logger->debugNode("\tVisibility: %s", CryoVisibilityTypeToString(symbol->visibility));
    }

    void SymbolTableDebugger::logExternSymbol(ExternSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL extern symbol");
            return;
        }

        logger->debugNode("Extern Symbol:");
        logger->debugNode("\tName: %s", symbol->name);
        logger->debugNode("\tScope ID: %s", symbol->scopeId);
        logger->debugNode("\tFunction Type: %s",
                          symbol->returnType ? DataTypeToString(symbol->returnType) : "void");
        logger->debugNode("\tParameters: %zu", symbol->paramCount);

        // for (size_t i = 0; i < symbol->paramCount; i++)
        // {
        //      logger->debugNode("    Param %zu: %s", i,
        //                      symbol->paramTypes[i] ? DataTypeToString(symbol->paramTypes[i]) : "unknown");
        // }

        logger->debugNode("\tVisibility: %s", CryoVisibilityTypeToString(symbol->visibility));
    }

    void SymbolTableDebugger::logTypeSymbol(TypeSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL type symbol");
            return;
        }

        logger->debugNode("Type Symbol:");
        logger->debugNode("\tName: %s", symbol->name);
        logger->debugNode("\tScope ID: %s", symbol->scopeId);
        logger->debugNode("\tType Of: %s", TypeofDataTypeToString(symbol->typeOf));
        logger->debugNode("\tIs Static: %s", symbol->isStatic ? "true" : "false");
        logger->debugNode("\tIs Generic: %s", symbol->isGeneric ? "true" : "false");
    }

    void SymbolTableDebugger::logPropertySymbol(PropertySymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL property symbol");
            return;
        }

        logger->debugNode("Property Symbol:");
        logger->debugNode("\tName: %s", symbol->name);
        logger->debugNode("\tScope ID: %s", symbol->scopeId);
        logger->debugNode("\tType: %s", symbol->type ? DataTypeToString(symbol->type) : "unknown");
        logger->debugNode("\tIs Static: %s", symbol->isStatic ? "true" : "false");
        logger->debugNode("\tHas Default: %s", symbol->hasDefaultExpr ? "true" : "false");
    }

    void SymbolTableDebugger::logMethodSymbol(MethodSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL method symbol");
            return;
        }

        logger->debugNode("Method Symbol:");
        logger->debugNode("\tName: %s", symbol->name);
        logger->debugNode("\tScope ID: %s", symbol->scopeId);
        logger->debugNode("\tReturn Type: %s",
                          symbol->returnType ? DataTypeToString(symbol->returnType) : "void");
        logger->debugNode("\tParameters: %zu", symbol->paramCount);

        for (size_t i = 0; i < symbol->paramCount; i++)
        {
            logger->debugNode("    Param %zu: %s", i,
                              symbol->paramTypes[i] ? DataTypeToString(symbol->paramTypes[i]) : "unknown");
        }

        logger->debugNode("\tVisibility: %d", symbol->visibility);
        logger->debugNode("\tIs Static: %s", symbol->isStatic ? "true" : "false");
    }

    void SymbolTableDebugger::logSymbol(Symbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL symbol");
            return;
        }

        switch (symbol->symbolType)
        {
        case VARIABLE_SYMBOL:
            logVariableSymbol(symbol->variable);
            break;
        case FUNCTION_SYMBOL:
            logFunctionSymbol(symbol->function);
            break;
        case EXTERN_SYMBOL:
            logExternSymbol(symbol->externSymbol);
            break;
        case TYPE_SYMBOL:
            logTypeSymbol(symbol->type);
            break;
        case PROPERTY_SYMBOL:
            logPropertySymbol(symbol->property);
            break;
        case METHOD_SYMBOL:
            logMethodSymbol(symbol->method);
            break;
        default:
            logger->debugNode("Unknown symbol type: %d", symbol->symbolType);
        }
    }

    void SymbolTableDebugger::logSymbolTable(SymbolTable *table)
    {
        if (!table)
        {
            logger->debugNode("NULL symbol table");
            return;
        }

        logger->debugNode("Symbol Table");
        logger->debugNode("\tNamespace: %s", table->namespaceName ? table->namespaceName : "global");
        logger->debugNode("\tSymbol Count: %zu", table->count);
        logger->debugNode("\tScope Depth: %zu", table->scopeDepth);
        logger->debugNode("\tScope ID: %s", table->scopeId);

        if (table->currentScope)
        {
            logger->debugNode("Current Scope:");
            logScopeBlock(table->currentScope);
        }

        logger->debugNode("\tSymbols:");
        for (size_t i = 0; i < table->count; i++)
        {
            logSymbol(table->symbols[i]);
        }
    }

    void SymbolTableDebugger::logTypeTable(TypesTable *table)
    {
        if (!table)
        {
            logger->debugNode("NULL type table");
            return;
        }

        logger->debugNode("Type Table:");
        logger->debugNode("\tNamespace: %s", table->namespaceName ? table->namespaceName : "global");
        logger->debugNode("\tType Count: %zu", table->count);
        logger->debugNode("\tScope Depth: %zu", table->scopeDepth);
        logger->debugNode("\tScope ID: %s", table->scopeId);

        logger->debugNode("Types:");
        for (size_t i = 0; i < table->count; i++)
        {
            logger->debugNode("Type %zu: %s", i, DataTypeToString(table->types[i]));
        }
    }

    const char *SymbolTableDebugger::getTypeofSymbol(TypeOfSymbol symbolType)
    {
        switch (symbolType)
        {
        case VARIABLE_SYMBOL:
            return "Variable";
        case FUNCTION_SYMBOL:
            return "Function";
        case EXTERN_SYMBOL:
            return "Extern";
        case TYPE_SYMBOL:
            return "Type";
        case PROPERTY_SYMBOL:
            return "Property";
        case METHOD_SYMBOL:
            return "Method";
        default:
            return "Unknown";
        }
    }

    void SymbolTableDebugger::logTypeofSymbol(TypeOfSymbol symbolType)
    {
        logger->debugNode("Type of Symbol: %s", getTypeofSymbol(symbolType));
    }

} // namespace Cryo