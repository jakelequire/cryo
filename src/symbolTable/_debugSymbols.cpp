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

namespace Cryo
{

    // Define the static member
    Logger *SymbolTableDebugger::logger = new Logger(Logger::DEBUG);

    void SymbolTableDebugger::logScopeBlock(ScopeBlock *block)
    {
        if (!block)
        {
            logger->log("NULL scope block");
            return;
        }

        logger->log("Scope Block:");
        logger->log("  Namespace: %s", block->namespaceName ? block->namespaceName : "global");
        logger->log("  Name: %s", block->name ? block->name : "<anonymous>");
        logger->log("  ID: %zu", block->id);
        logger->log("  Depth: %zu", block->depth);
        logger->log("  Child Count: %zu", block->childCount);

        if (block->parent)
        {
            logger->log("  Parent Scope: %s (ID: %zu)",
                        block->parent->name ? block->parent->name : "<anonymous>",
                        block->parent->id);
        }
    }

    void SymbolTableDebugger::logVariableSymbol(VariableSymbol *symbol)
    {
        if (!symbol)
        {
            logger->log("NULL variable symbol");
            return;
        }

        logger->log("Variable Symbol:");
        logger->log("  Name: %s", symbol->name);
        logger->log("  Scope ID: %zu", symbol->scopeId);
        logger->log("  Type: %s", symbol->type ? DataTypeToString(symbol->type) : "unknown");
        logger->log("  AST Node: %p", (void *)symbol->node);
    }

    void SymbolTableDebugger::logFunctionSymbol(FunctionSymbol *symbol)
    {
        if (!symbol)
        {
            logger->log("NULL function symbol");
            return;
        }

        logger->log("Function Symbol:");
        logger->log("  Name: %s", symbol->name);
        logger->log("  Scope ID: %zu", symbol->scopeId);
        logger->log("  Return Type: %s",
                    symbol->returnType ? DataTypeToString(symbol->returnType) : "void");
        logger->log("  Parameters: %zu", symbol->paramCount);

        for (size_t i = 0; i < symbol->paramCount; i++)
        {
            logger->log("    Param %zu: %s", i,
                        symbol->paramTypes[i] ? DataTypeToString(symbol->paramTypes[i]) : "unknown");
        }

        logger->log("  Visibility: %d", symbol->visibility);
    }

    void SymbolTableDebugger::logTypeSymbol(TypeSymbol *symbol)
    {
        if (!symbol)
        {
            logger->log("NULL type symbol");
            return;
        }

        logger->log("Type Symbol:");
        logger->log("  Name: %s", symbol->name);
        logger->log("  Scope ID: %zu", symbol->scopeId);
        logger->log("  Type Of: %d", symbol->typeOf);
        logger->log("  Is Static: %s", symbol->isStatic ? "true" : "false");
        logger->log("  Is Generic: %s", symbol->isGeneric ? "true" : "false");
    }

    void SymbolTableDebugger::logPropertySymbol(PropertySymbol *symbol)
    {
        if (!symbol)
        {
            logger->log("NULL property symbol");
            return;
        }

        logger->log("Property Symbol:");
        logger->log("  Name: %s", symbol->name);
        logger->log("  Scope ID: %zu", symbol->scopeId);
        logger->log("  Type: %s", symbol->type ? DataTypeToString(symbol->type) : "unknown");
        logger->log("  Is Static: %s", symbol->isStatic ? "true" : "false");
        logger->log("  Has Default: %s", symbol->hasDefaultExpr ? "true" : "false");
    }

    void SymbolTableDebugger::logMethodSymbol(MethodSymbol *symbol)
    {
        if (!symbol)
        {
            logger->log("NULL method symbol");
            return;
        }

        logger->log("Method Symbol:");
        logger->log("  Name: %s", symbol->name);
        logger->log("  Scope ID: %zu", symbol->scopeId);
        logger->log("  Return Type: %s",
                    symbol->returnType ? DataTypeToString(symbol->returnType) : "void");
        logger->log("  Parameters: %zu", symbol->paramCount);

        for (size_t i = 0; i < symbol->paramCount; i++)
        {
            logger->log("    Param %zu: %s", i,
                        symbol->paramTypes[i] ? DataTypeToString(symbol->paramTypes[i]) : "unknown");
        }

        logger->log("  Visibility: %d", symbol->visibility);
        logger->log("  Is Static: %s", symbol->isStatic ? "true" : "false");
    }

    void SymbolTableDebugger::logSymbol(Symbol *symbol)
    {
        if (!symbol)
        {
            logger->log("NULL symbol");
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
            logger->log("Unknown symbol type: %d", symbol->symbolType);
        }
    }

    void SymbolTableDebugger::logSymbolTable(SymbolTable *table)
    {
        if (!table)
        {
            logger->log("NULL symbol table");
            return;
        }

        logger->log("Symbol Table:");
        logger->log("  Namespace: %s", table->namespaceName ? table->namespaceName : "global");
        logger->log("  Symbol Count: %zu", table->count);
        logger->log("  Scope Depth: %zu", table->scopeDepth);
        logger->log("  Scope ID: %zu", table->scopeId);

        if (table->currentScope)
        {
            logger->log("Current Scope:");
            logScopeBlock(table->currentScope);
        }

        logger->log("Symbols:");
        for (size_t i = 0; i < table->count; i++)
        {
            logSymbol(table->symbols[i]);
        }
    }

    void SymbolTableDebugger::logTypeTable(TypesTable *table)
    {
        if (!table)
        {
            logger->log("NULL type table");
            return;
        }

        logger->log("Type Table:");
        logger->log("  Namespace: %s", table->namespaceName ? table->namespaceName : "global");
        logger->log("  Type Count: %zu", table->count);
        logger->log("  Scope Depth: %zu", table->scopeDepth);
        logger->log("  Scope ID: %zu", table->scopeId);

        logger->log("Types:");
        for (size_t i = 0; i < table->count; i++)
        {
            logger->log("Type %zu: %s", i, DataTypeToString(table->types[i]));
        }
    }

} // namespace Cryo