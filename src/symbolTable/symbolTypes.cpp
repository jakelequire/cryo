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

namespace Cryo
{
    DataType *GlobalSymbolTable::resolveDataType(const char *name)
    {
        // No need for strdup here - std::string will make its own copy
        std::string cxxTypeNameStr(name); // Direct construction from const char*

        logMessage(LMI, "INFO", "Symbol Table", "Resolving Data Type", "Type Name", name);

        if (cxxTypeNameStr.empty())
        {
            std::cerr << "Error: Failed to resolve data type, name is empty!" << std::endl;
            return nullptr;
        }

        if (!typeTable || typeTable == nullptr)
        {
            std::cerr << "Error: Failed to resolve data type, types table is null!" << std::endl;
            return nullptr;
        }

        size_t count = typeTable->count;
        for (size_t i = 0; i < count; i++)
        {
            const char *typeName = typeTable->typeSymbols[i]->name;
            DataType *dataType = typeTable->typeSymbols[i]->type;
            if (!dataType || dataType == nullptr)
            {
                continue;
            }
            if (strcmp(typeName, cxxTypeNameStr.c_str()) == 0)
            {
                logMessage(LMI, "INFO", "Symbol Table", "Resolved Data Type", "Type Name", typeName);
                return dataType;
            }
            else
            {
                continue;
            }
        }

        return nullptr;
    }

    void GlobalSymbolTable::parseSymbolTableForTypes(SymbolTable *symbolTable)
    {
        if (!symbolTable || symbolTable == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Symbol Table is null");
            return;
        }

        Symbol **symbols = symbolTable->symbols;
        for (size_t i = 0; i < symbolTable->count; i++)
        {
            Symbol *symbol = symbols[i];
            switch (symbol->symbolType)
            {
            case TYPE_SYMBOL:
            {
                TypeSymbol *typeSymbol = symbol->type;
                addTypeToTable(typeSymbol);
                break;
            }
            default:
            {
                break;
            }
            }
        }
    }

    int GlobalSymbolTable::addDataTypeSymbol(Symbol *symbol)
    {
        if (!symbol || symbol == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Symbol is null");
            return -1;
        }
        TypeOfSymbol symbolType = symbol->symbolType;
        if (symbolType != TYPE_SYMBOL)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Symbol is not a Type Symbol");
            const char *symbolTypeStr = typeOfSymbolToString(symbolType);
            logMessage(LMI, "ERROR", "Symbol Table", "Symbol Type", symbolTypeStr);
            return -1;
        }
        TypeSymbol *typeSymbol = symbol->type;
        addTypeToTable(typeSymbol);

        const char *typeName = typeSymbol->name;
        logMessage(LMI, "INFO", "Symbol Table", "Added Type Symbol to Table", "Type Name", typeName);

        return 0;
    }

    bool GlobalSymbolTable::doesTypeExist(const char *name)
    {
        DEBUG_PRINT_FILTER({
            std::cout << "Checking if Type Exists: " << name << std::endl;
        });
        if (!typeTable || typeTable == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Table is null");
            return false;
        }
        if (!name || name == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Name is null");
            return false;
        }

        size_t count = typeTable->count;
        for (size_t i = 0; i < count; i++)
        {
            TypeSymbol *typeSymbol = typeTable->typeSymbols[i];
            if (!typeSymbol || typeSymbol == nullptr)
            {
                logMessage(LMI, "ERROR", "Symbol Table", "Type Symbol is null");
                continue;
            }
            const char *typeName = typeTable->typeSymbols[i]->name;
            if (strcmp(typeName, name) == 0)
            {
                logMessage(LMI, "INFO", "Symbol Table", "Type Exists in Table", "Type Name", typeName);
                return true;
            }
        }

        return false;
    }

    void GlobalSymbolTable::addTypeToTable(TypeSymbol *typeSymbol)
    {
        if (!typeSymbol || typeSymbol == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Symbol is null");
            return;
        }

        const char *typeName = typeSymbol->name;
        if (doesTypeExist(typeName))
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type already exists in table", "Type Name", typeName);
            return;
        }

        typeTable->typeSymbols[typeTable->count] = typeSymbol;
        typeTable->count++;

        logMessage(LMI, "INFO", "Symbol Table", "Added Type to Table", "Type Name", typeName);
        return;
    }

    // -------------------------------------------------------

    void GlobalSymbolTable::initTypeDefinition(Symbol *typeSymbol)
    {
        if (!typeSymbol || typeSymbol == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Symbol is null");
            return;
        }
        if (typeSymbol->symbolType != TYPE_SYMBOL)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Symbol is not a Type Symbol");
            return;
        }

        TypeSymbol *type = typeSymbol->type;
        const char *typeName = type->name;
        if (!typeName || typeName == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Name is null");
            return;
        }

        if (doesTypeExist(typeName))
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type already exists in table", "Type Name", typeName);
            return;
        }
        size_t tableCount = typeTable->count;
        typeTable->typeSymbols[tableCount] = type;
        typeTable->count++;

        return;
    }

    // After the type def is complete. We need to update the type symbol in the table with the new typeSymbol
    void GlobalSymbolTable::completeTypeDefinition(Symbol *typeSymbol, const char *typeName)
    {
        if (!typeSymbol || typeSymbol == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Symbol is null");
            return;
        }
        if (typeSymbol->symbolType != TYPE_SYMBOL)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Symbol is not a Type Symbol");
            return;
        }

        size_t count = typeTable->count;
        for (size_t i = 0; i < count; i++)
        {
            TypeSymbol *type = typeTable->typeSymbols[i];
            if (!type || type == nullptr)
            {
                continue;
            }
            if (strcmp(type->name, typeName) == 0)
            {
                typeTable->typeSymbols[i] = typeSymbol->type;
                return;
            }
        }
        return;
    }

} // namespace Cryo
