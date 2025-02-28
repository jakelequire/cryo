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
#include "tools/logger/logger_config.h"

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
    const char *HEAVY_SEPARATOR = BOLD WHITE "===================================================================" COLOR_RESET;
    const char *CHILD_SEPARATOR = ">>===--------------";

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
        logger->debugNode(HEAVY_SEPARATOR);
    }

    void SymbolTableDebugger::logVariableSymbol(VariableSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL variable symbol");
            return;
        }

        bool isParam = symbol->isParam;
        const char *trueStr = BOLD GREEN "true" COLOR_RESET;
        const char *falseStr = BOLD RED "false" COLOR_RESET;
        const char *paramStr = isParam ? trueStr : falseStr;

        logger->debugNode("%s | " BOLD WHITE "%s" COLOR_RESET ": %s",
                          getColoredSymbolType(VARIABLE_SYMBOL).c_str(),
                          symbol->name,
                          symbol->type ? DataTypeToString(symbol->type) : "unknown");
        logger->debugNode("ID: %s | Param: %s",
                          symbol->scopeId,
                          paramStr);
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
        logger->debugNode("Parent ID: %s | Function ID: %s",
                          symbol->functionScopeId,
                          symbol->functionScopeId);
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

        logger->debugNode("%s | " BOLD WHITE "%s" COLOR_RESET ": %s",
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

        logger->debugNode("%s | " BOLD WHITE "%s" COLOR_RESET ":" BOLD MAGENTA " %s" COLOR_RESET,
                          getColoredSymbolType(TYPE_SYMBOL).c_str(),
                          symbol->name,
                          TypeofDataTypeToString(symbol->typeOf));
        const char *dataTypeStr = symbol->type ? DataTypeToString(symbol->type) : "TYPE_UNDEFINED";
        logger->debugNode("DataType: %s", dataTypeStr);
        logger->debugNode("ID: %s | Static: %s Generic: %s",
                          symbol->scopeId,
                          symbol->isStatic ? "true" : "false",
                          symbol->isGeneric ? "true" : "false");

        //
        // Struct type symbol
        //
        if (symbol->typeOf == STRUCT_TYPE)
        {
            logger->debugNode("Properties: %d | Methods: %d",
                              symbol->propertyCount,
                              symbol->methodCount);
            // Log the properties
            int propCount = symbol->propertyCount;
            if (propCount > 0)
            {
                logger->debugNode(CHILD_SEPARATOR);
            }
            for (int i = 0; i < propCount; i++)
            {
                PropertySymbol *propSymbol = symbol->properties[i]->property;
                if (!propSymbol)
                {
                    logger->debugNode("NULL PROPERTY");
                }

                logPropertySymbolChild(propSymbol);
            }
            // Log the methods
            int methodCount = symbol->methodCount;
            for (int i = 0; i < methodCount; i++)
            {
                MethodSymbol *methodSymbol = symbol->methods[i]->method;
                if (!methodSymbol)
                {
                    logger->debugNode("NULL METHOD");
                }

                logMethodSymbolChild(methodSymbol);
            }
        }

        //
        // Class type symbol
        //
        if (symbol->typeOf == CLASS_TYPE)
        {
            logger->debugNode("Properties: %d | Methods: %d",
                              symbol->propertyCount,
                              symbol->methodCount);
            // Log the properties
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
            int methodCount = symbol->methodCount;
            for (int i = 0; i < methodCount; i++)
            {
                MethodSymbol *methodSymbol = symbol->methods[i]->method;
                if (!methodSymbol)
                {
                    logger->debugNode("NULL METHOD");
                }

                logMethodSymbolChild(methodSymbol);
            }
        }

        logger->debugNode(SEPARATOR);
    }

    void SymbolTableDebugger::logClassTypeSymbol(TypeSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL class type symbol");
            return;
        }

        const char *dataTypeStr = symbol->type ? DataTypeToString(symbol->type) : "unknown";

        logger->debugNode("%s | " BOLD WHITE "%s" COLOR_RESET ":" BOLD MAGENTA " %s" COLOR_RESET,
                          getColoredSymbolType(TYPE_SYMBOL).c_str(),
                          symbol->name,
                          TypeofDataTypeToString(symbol->typeOf));
        logger->debugNode("DataType: %s", dataTypeStr);
        logger->debugNode("ID: %s | Static: %s Generic: %s",
                          symbol->scopeId,
                          symbol->isStatic ? "true" : "false",
                          symbol->isGeneric ? "true" : "false");
        logger->debugNode("Properties: %d | Methods: %d",
                          symbol->propertyCount,
                          symbol->methodCount);

        // log the properties
        int propCount = symbol->propertyCount;
        if (propCount > 0)
        {
            logger->debugNode(CHILD_SEPARATOR);
        }
        for (int i = 0; i < propCount; i++)
        {
            PropertySymbol *propSymbol = symbol->properties[i]->property;
            if (!propSymbol)
            {
                logger->debugNode("NULL PROPERTY");
            }

            logPropertySymbolChild(propSymbol);
        }
        // log the methods
        int methodCount = symbol->methodCount;
        for (int i = 0; i < methodCount; i++)
        {
            MethodSymbol *methodSymbol = symbol->methods[i]->method;
            if (!methodSymbol)
            {
                logger->debugNode("NULL METHOD");
            }

            logMethodSymbolChild(methodSymbol);
        }
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

        const char *symbolTypeStr = getColoredSymbolType(PROPERTY_SYMBOL).c_str();
        const char *nameStr = symbol->name ? symbol->name : "<anonymous>";
        const char *typeStr = symbol->type ? DataTypeToString(symbol->type) : "unknown";

        logger->debugNode("%s | " BOLD WHITE "%s" COLOR_RESET ": Type: %s",
                          symbolTypeStr,
                          nameStr,
                          typeStr);
        logger->debugNode("ID: %s | Static: %s HasDefault: %s",
                          symbol->scopeId,
                          symbol->isStatic ? "true" : "false",
                          symbol->hasDefaultExpr ? "true" : "false");
        logger->debugNode(CHILD_SEPARATOR);
    }

    void SymbolTableDebugger::logMethodSymbolChild(MethodSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL method symbol");
            return;
        }

        const char *symbolTypeStr = getColoredSymbolType(METHOD_SYMBOL).c_str();
        const char *nameStr = symbol->name ? symbol->name : "<anonymous>";
        const char *returnTypeStr = symbol->returnType ? DataTypeToString(symbol->returnType) : "void";

        logger->debugNode("%s | " BOLD WHITE "%s" COLOR_RESET ": ReturnType: %s",
                          symbolTypeStr,
                          nameStr,
                          returnTypeStr);

        const char *scopeIdStr = symbol->scopeId ? symbol->scopeId : "<anonymous>";
        const char *staticStr = symbol->isStatic ? "true" : "false";
        const char *visibilityStr = symbol->visibility ? "true" : "false";
        logger->debugNode("ID: %s | Static: %s Visibility: %s",
                          scopeIdStr,
                          staticStr,
                          visibilityStr);

        size_t paramCount = symbol->paramCount;
        logger->debugNode("Parameters: %zu", paramCount);

        // Only log parameters if they exist
        // if (symbol->paramCount > 0)
        // {
        //     for (size_t i = 0; i < symbol->paramCount; i++)
        //     {
        //         const char *paramTypeStr = symbol->paramTypes[i] ? DataTypeToString(symbol->paramTypes[i]) : "unknown";
        //         logger->debugNode("  Param %zu: %s", i, paramTypeStr);
        //     }
        // }
        logger->debugNode(CHILD_SEPARATOR);
    }

    void SymbolTableDebugger::logPropertySymbol(PropertySymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL property symbol");
            return;
        }

        logger->debugNode("%s | " BOLD WHITE "%s" COLOR_RESET ": Type: %s",
                          getColoredSymbolType(PROPERTY_SYMBOL).c_str(),
                          symbol->name,
                          symbol->type ? DataTypeToString(symbol->type) : "unknown");
        logger->debugNode("ID: %s | Static: %s HasDefault: %s",
                          symbol->scopeId,
                          symbol->isStatic ? "true" : "false",
                          symbol->hasDefaultExpr ? "true" : "false");
        logger->debugNode(HEAVY_SEPARATOR);
    }

    void SymbolTableDebugger::logMethodSymbol(MethodSymbol *symbol)
    {
        if (!symbol)
        {
            logger->debugNode("NULL method symbol");
            return;
        }

        logger->debugNode("%s | " BOLD WHITE "%s" COLOR_RESET ": ReturnType: %s",
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
        logger->debugNode(HEAVY_SEPARATOR);
    }

    void SymbolTableDebugger::logSymbolTable(SymbolTable *table)
    {
        DEBUG_PRINT_FILTER({
            if (!table)
            {
                logger->debugNode("NULL symbol table");
                return;
            }

            logger->debugNode("SymbolTable | " BOLD "Namespace: " BLUE "%s" COLOR_RESET " Count: %zu",
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
                logger->debugNode(HEAVY_SEPARATOR);
                logger->debugNode(BOLD UNDERLINE YELLOW "\nSymbols:" COLOR_RESET);
                logger->debugNode(SEPARATOR);
                for (size_t i = 0; i < table->count; i++)
                {
                    logSymbol(table->symbols[i]);
                }
            }
            if (table->count == 0)
            {
                logger->debugNode(HEAVY_SEPARATOR);
                logger->debugNode(BOLD UNDERLINE YELLOW "\nSymbols:" COLOR_RESET);
                logger->debugNode(SEPARATOR);
                logger->debugNode("\n " BOLD RED "[!] No symbols in table" COLOR_RESET);
            }
            printf("\n");
            logger->debugNode(HEAVY_SEPARATOR);
        });
    }

    void SymbolTableDebugger::logTypeTable(TypesTable *table)
    {
        DEBUG_PRINT_FILTER({
            if (!table)
            {
                logger->debugNode("NULL type table");
                return;
            }
            logger->debugNode(HEAVY_SEPARATOR);

            logger->debugNode("TypesTable | " BOLD "Namespace: " CYAN "%s" COLOR_RESET " Count: %zu",
                              table->namespaceName ? table->namespaceName : "global",
                              table->count);
            logger->debugNode("ID: %s Depth: %zu", table->scopeId, table->scopeDepth);

            if (table->count > 0)
            {
                logger->debugNode(HEAVY_SEPARATOR);
                logger->debugNode(BOLD UNDERLINE YELLOW "\nSymbol Types:" COLOR_RESET);
                logger->debugNode(SEPARATOR);
                for (size_t i = 0; i < table->count; i++)
                {
                    logTypeSymbol(table->typeSymbols[i]);
                }
            }
            if (table->count == 0)
            {
                logger->debugNode(HEAVY_SEPARATOR);
                logger->debugNode(BOLD UNDERLINE YELLOW "\nSymbol Types:" COLOR_RESET);
                logger->debugNode(SEPARATOR);
                logger->debugNode("\n " BOLD RED "[!] No Types in TypesTable" COLOR_RESET);
            }

            printf("\n");
            logger->debugNode(HEAVY_SEPARATOR);
        });
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