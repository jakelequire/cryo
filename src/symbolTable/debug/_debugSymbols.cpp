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
───────────────────────────────────────────────────────────────────

*/

namespace Cryo
{
    Logger *SymbolTableDebugger::logger = new Logger(Logger::DEBUG);
    const char *SEPARATOR = "───────────────────────────────────────────────────────────────────";
    const char *CHILD_SEPARATOR = "------------------------------";

    void SymbolTableDebugger::logScopeBlock(ScopeBlock *block)
    {
        if (!block)
        {
            logger->debugNode("NULL scope block");
            return;
        }

        logger->debugNode("%s | Name: %s Namespace: %s",
                          getColoredScopeType().c_str(),
                          block->name ? block->name : "<anonymous>",
                          block->namespaceName ? block->namespaceName : "global");
        logger->debugNode("ID: %s Depth: %zu | Parent: %s",
                          block->id,
                          block->depth,
                          block->parent ? block->parent->name : "none");
        logger->debugNode("Child Count: %zu", block->childCount);
        logger->debugNode(SEPARATOR);
    }

    void SymbolTableDebugger::logVariableSymbol(VariableSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL variable symbol");
            return;
        }

        logger->debugNode("%s | %s: %s",
                          getColoredSymbolType(VARIABLE_SYMBOL).c_str(),
                          symbol->name,
                          symbol->type ? DataTypeToString(symbol->type) : "unknown");
        logger->debugNode("ID: %s",
                          symbol->scopeId);
        logger->debugNode(SEPARATOR);
    }

    void SymbolTableDebugger::logFunctionSymbol(FunctionSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL function symbol");
            return;
        }

        logger->debugNode("%s | %s: %s",
                          getColoredSymbolType(FUNCTION_SYMBOL).c_str(),
                          symbol->name,
                          symbol->returnType ? DataTypeToString(symbol->returnType) : "void");
        logger->debugNode("ID: %s | Visibility: %s",
                          symbol->scopeId,
                          CryoVisibilityTypeToString(symbol->visibility));
        logger->debugNode("Parameters: %zu", symbol->paramCount);
        logger->debugNode(SEPARATOR);
    }

    void SymbolTableDebugger::logExternSymbol(ExternSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL extern symbol");
            return;
        }

        logger->debugNode("%s | %s: %s",
                          getColoredSymbolType(EXTERN_SYMBOL).c_str(),
                          symbol->name,
                          symbol->returnType ? DataTypeToString(symbol->returnType) : "void");
        logger->debugNode("ID: %s | Visibility: %s",
                          symbol->scopeId,
                          CryoVisibilityTypeToString(symbol->visibility));
        logger->debugNode("Parameters: %zu", symbol->paramCount);
        logger->debugNode(SEPARATOR);
    }

    void SymbolTableDebugger::logTypeSymbol(TypeSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL type symbol");
            return;
        }
        if (symbol->typeOf == CLASS_TYPE)
        {
            logClassTypeSymbol(symbol);
            return;
        }

        logger->debugNode("%s | %s TypeOf: %s",
                          getColoredSymbolType(TYPE_SYMBOL).c_str(),
                          symbol->name,
                          TypeofDataTypeToString(symbol->typeOf));
        logger->debugNode("ID: %s | Static: %s Generic: %s",
                          symbol->scopeId,
                          symbol->isStatic ? "true" : "false",
                          symbol->isGeneric ? "true" : "false");
        logger->debugNode(SEPARATOR);
    }

    void SymbolTableDebugger::logClassTypeSymbol(TypeSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL class type symbol");
            return;
        }

        logger->debugNode("%s | %s TypeOf: %s",
                          getColoredSymbolType(TYPE_SYMBOL).c_str(),
                          symbol->name,
                          TypeofDataTypeToString(symbol->typeOf));
        logger->debugNode("ID: %s | Static: %s Generic: %s",
                          symbol->scopeId,
                          symbol->isStatic ? "true" : "false",
                          symbol->isGeneric ? "true" : "false");
        logger->debugNode("Properties: %d | Methods: %d",
                          symbol->propertyCount,
                          symbol->methodCount);

        // log the properties
        int propCount = symbol->propertyCount;
        for (int i = 0; i < propCount; i++)
        {
            PropertySymbol *propSymbol = symbol->properties[i]->property;
            if (!propSymbol)
            {
                logger->debugNode("NULL PROPERTY");
            }

            logPropertySymbolChild(propSymbol);
        }

        logger->debugNode(SEPARATOR);
    }

    // Only difference between this function and `logPropertySymbol` is not having the same
    // seperator at the end. It should be shorter then the main one to show that it's a child
    void SymbolTableDebugger::logPropertySymbolChild(PropertySymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL property symbol");
            return;
        }

        logger->debugNode("%s | %s Type: %s",
                          getColoredSymbolType(PROPERTY_SYMBOL).c_str(),
                          symbol->name,
                          symbol->type ? DataTypeToString(symbol->type) : "unknown");
        logger->debugNode("ID: %s | Static: %s HasDefault: %s",
                          symbol->scopeId,
                          symbol->isStatic ? "true" : "false",
                          symbol->hasDefaultExpr ? "true" : "false");
        logger->debugNode(CHILD_SEPARATOR);
    }

    void SymbolTableDebugger::logPropertySymbol(PropertySymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL property symbol");
            return;
        }

        logger->debugNode("%s | %s Type: %s",
                          getColoredSymbolType(PROPERTY_SYMBOL).c_str(),
                          symbol->name,
                          symbol->type ? DataTypeToString(symbol->type) : "unknown");
        logger->debugNode("ID: %s | Static: %s HasDefault: %s",
                          symbol->scopeId,
                          symbol->isStatic ? "true" : "false",
                          symbol->hasDefaultExpr ? "true" : "false");
        logger->debugNode(SEPARATOR);
    }

    void SymbolTableDebugger::logMethodSymbol(MethodSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL method symbol");
            return;
        }

        logger->debugNode("%s | %s ReturnType: %s",
                          getColoredSymbolType(METHOD_SYMBOL).c_str(),
                          symbol->name,
                          symbol->returnType ? DataTypeToString(symbol->returnType) : "void");
        logger->debugNode("ID: %s | Static: %s Visibility: %d",
                          symbol->scopeId,
                          symbol->isStatic ? "true" : "false",
                          symbol->visibility);
        logger->debugNode("Parameters: %zu", symbol->paramCount);

        // Only log parameters if they exist
        if (symbol->paramCount > 0)
        {
            for (size_t i = 0; i < symbol->paramCount; i++)
            {
                logger->debugNode("  Param %zu: %s", i,
                                  symbol->paramTypes[i] ? DataTypeToString(symbol->paramTypes[i]) : "unknown");
            }
        }
        logger->debugNode(SEPARATOR);
    }

    void SymbolTableDebugger::logSymbolTable(SymbolTable *table)
    {
        if (!table)
        {
            logger->debugNode("NULL symbol table");
            return;
        }

        logger->debugNode("SymbolTable | Namespace: %s Count: %zu",
                          table->namespaceName ? table->namespaceName : "global",
                          table->count);
        logger->debugNode("ID: %s Depth: %zu", table->scopeId, table->scopeDepth);

        if (table->currentScope)
        {
            logger->debugNode("\nCurrent Scope:");
            logScopeBlock(table->currentScope);
        }

        if (table->count > 0)
        {
            logger->debugNode("\nSymbols:\n");
            logger->debugNode(SEPARATOR);
            for (size_t i = 0; i < table->count; i++)
            {
                logSymbol(table->symbols[i]);
            }
        }
        logger->debugNode(SEPARATOR);
    }

    void SymbolTableDebugger::logTypeTable(TypesTable *table)
    {
        if (!table)
        {
            logger->debugNode("NULL type table");
            return;
        }

        logger->debugNode("TypeTable | Namespace: %s Count: %zu",
                          table->namespaceName ? table->namespaceName : "global",
                          table->count);
        logger->debugNode("ID: %s Depth: %zu", table->scopeId, table->scopeDepth);

        if (table->count > 0)
        {
            logger->debugNode("\nTypes:");
            for (size_t i = 0; i < table->count; i++)
            {
                logger->debugNode("  Type %zu: %s", i, DataTypeToString(table->types[i]));
            }
        }
        logger->debugNode(SEPARATOR);
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

    std::string SymbolTableDebugger::getColoredSymbolType(TypeOfSymbol symbolType)
    {
        std::string baseType = getTypeofSymbol(symbolType);
        std::string colorCode;

        switch (symbolType)
        {
        case VARIABLE_SYMBOL:
            colorCode = SymbolColors::VARIABLE();
            break;
        case FUNCTION_SYMBOL:
            colorCode = SymbolColors::FUNCTION();
            break;
        case EXTERN_SYMBOL:
            colorCode = SymbolColors::EXTERN();
            break;
        case TYPE_SYMBOL:
            colorCode = SymbolColors::TYPE();
            break;
        case PROPERTY_SYMBOL:
            colorCode = SymbolColors::PROPERTY();
            break;
        case METHOD_SYMBOL:
            colorCode = SymbolColors::METHOD();
            break;
        default:
            return baseType; // No color for unknown types
        }

        return std::string(colorCode) + std::string(ANSI_BOLD) + baseType + std::string(ANSI_RESET);
    }

    // Helper function for table types
    std::string SymbolTableDebugger::getColoredTableType(const char *tableType)
    {
        return std::string(SymbolColors::TABLE()) + std::string(ANSI_BOLD) + tableType + std::string(ANSI_RESET);
    }

    // Helper function for scope blocks
    std::string SymbolTableDebugger::getColoredScopeType()
    {
        return std::string(SymbolColors::SCOPE()) + std::string(ANSI_BOLD) + "ScopeBlock" + std::string(ANSI_RESET);
    }

} // namespace Cryo