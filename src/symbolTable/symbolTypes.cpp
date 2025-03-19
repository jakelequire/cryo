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
    DataType *GlobalSymbolTable::resolveDataType(const char *name)
    {
        // Check if it's an array type
        std::string typeName(name);
        bool isArray = false;
        std::string baseTypeName = typeName;

        std::cout << "=================== [ Resolve Data Type ] ===================" << std::endl;
        std::cout << "Type Name: " << typeName << std::endl;
        std::cout << "============================================================" << std::endl;

        // Check for array notation
        if (typeName.length() >= 2 && typeName.substr(typeName.length() - 2) == "[]")
        {
            logMessage(LMI, "INFO", "Symbol Table", "Resolving Array Type", "Type Name: %s", typeName.c_str());
            isArray = true;
            baseTypeName = typeName.substr(0, typeName.length() - 2);
        }

        // First try to find the base type
        DataType *baseType = nullptr;

        // Check in the type table
        for (size_t i = 0; i < typeTable->count; i++)
        {
            if (strcmp(typeTable->typeSymbols[i]->name, baseTypeName.c_str()) == 0)
            {
                logMessage(LMI, "INFO", "Symbol Table", "Found Base Type, Type Name: %s", baseTypeName.c_str());
                baseType = typeTable->typeSymbols[i]->type;
                break;
            }
        }

        if (!baseType)
        {
            // Could not find the base type
            logMessage(LMI, "ERROR", "Symbol Table", "Failed to resolve base type", "Type Name: %s", baseTypeName.c_str());
            return nullptr;
        }

        return baseType;
    }

    TypeSymbol *GlobalSymbolTable::findTypeSymbol(std::string typeName)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Symbol Table", "Resolving Data Type, Type Name: %s", typeName.c_str());

        // Print the type table
        SymbolTableDebugger::logTypeTable(this->typeTable);

        if (typeName.empty())
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
            const char *typeSymbolName = typeTable->typeSymbols[i]->name;
            TypeSymbol *typeSymbol = typeTable->typeSymbols[i];
            if (!typeSymbol || typeSymbol == nullptr)
            {
                continue;
            }
            if (strcmp(typeSymbolName, typeName.c_str()) == 0)
            {
                logMessage(LMI, "INFO", "Symbol Table", "Resolved Data Type", "Type Name %s", typeSymbolName);
                return typeSymbol;
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
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
        if (!typeSymbol || typeSymbol == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Symbol is null");
            return;
        }

        const char *typeName = typeSymbol->name;
        if (!typeName)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Symbol has null name");
            return;
        }

        std::cout << "=================== [ Add Type To Table ] ===================" << std::endl;
        std::cout << "Type Name: " << typeName << std::endl;
        std::cout << "============================================================" << std::endl;

        // Check if type already exists
        if (doesTypeExist(typeName))
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type already exists in table", "Type Name", typeName);
            return;
        }

        // Check if we need to resize the type table
        if (typeTable->count >= typeTable->capacity)
        {
            size_t newCapacity = typeTable->capacity * 2;
            if (newCapacity == 0)
                newCapacity = MAX_TYPE_SYMBOLS; // Initial capacity if 0

            TypeSymbol **newSymbols = (TypeSymbol **)realloc(typeTable->typeSymbols,
                                                             newCapacity * sizeof(TypeSymbol *));
            if (!newSymbols)
            {
                logMessage(LMI, "ERROR", "Symbol Table", "Failed to resize type table");
                return;
            }

            typeTable->typeSymbols = newSymbols;
            typeTable->capacity = newCapacity;
            logMessage(LMI, "INFO", "Symbol Table", "Resized type table", "New Capacity",
                       std::to_string(newCapacity).c_str());
        }

        // Add the type symbol to the table
        typeTable->typeSymbols[typeTable->count++] = typeSymbol;

        logMessage(LMI, "INFO", "Symbol Table", "Added Type to Table", "Type Name", typeName,
                   "Count", std::to_string(typeTable->count).c_str());
        printTypeTable();
    }

    // -------------------------------------------------------

    void GlobalSymbolTable::initTypeDefinition(Symbol *typeSymbol)
    {
        __STACK_FRAME__
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

    void GlobalSymbolTable::addOrUpdateType(TypeSymbol *typeSymbol)
    {
        __STACK_FRAME__
        if (!typeSymbol || typeSymbol == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Symbol is null");
            return;
        }

        const char *typeName = typeSymbol->name;
        if (!typeName || typeName == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type Name is null");
            return;
        }

        if (doesTypeExist(typeName))
        {
            // If the type already exists, we will replace the existing type with the new type
            logMessage(LMI, "INFO", "Symbol Table", "Type already exists in table, updating type", "Type Name", typeName);
            typeTable->typeSymbols[typeTable->count] = typeSymbol;
            return;
        }

        typeTable->typeSymbols[typeTable->count] = typeSymbol;
        typeTable->count++;

        logMessage(LMI, "INFO", "Symbol Table", "Added Type to Table", "Type Name", typeName);
        return;
    }

    // After the type def is complete.d We need to update the type symbol in the table with the new typeSymbol
    void GlobalSymbolTable::completeTypeDefinition(Symbol *typeSymbol, const char *typeName)
    {
        __STACK_FRAME__
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

    DataType *GlobalSymbolTable::resolveGenericDataType(const char *baseName, DataType **typeArgs, int argCount)
    {
        // First find the base generic type
        DataType *baseType = resolveDataType(baseName);
        if (!baseType)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Type %s is not a generic type", baseName);
            return nullptr;
        }

        // Create a new instantiated type
        return nullptr;
    }

    void GlobalSymbolTable::registerGenericType(const char *functionName, DTGenericTy **params, int paramCount, DataType *type)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Symbol Table", "Registering Generic Type, Function Name: ", functionName);

        for (int i = 0; i < paramCount; i++)
        {
            DTGenericTy *param = params[i];
            logMessage(LMI, "INFO", "Symbol Table", "Generic Type Parameter", "Type Name", param->name);

            // Create a new type symbol for the generic parameter
            std::string genericName = generateGenericName(functionName, param->name);
            TypeSymbol *typeSymbol = createTypeSymbol(genericName.c_str(), nullptr, type, GENERIC_TYPE, false, true, nullptr);
            addTypeToTable(typeSymbol);
        }
    }

    std::string GlobalSymbolTable::generateGenericName(const char *functionName, const char *genericName)
    {
        __STACK_FRAME__
        // Validate inputs
        if (!functionName || !genericName)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Invalid parameters for generateGenericName");
            return "";
        }

        // Create a name in the format "functionName<genericName>"
        std::string result = functionName;

        // Check if the generic name already includes angle brackets (might be a nested generic)
        bool hasAngleBrackets = (strchr(genericName, '<') != nullptr && strchr(genericName, '>') != nullptr);

        // Only add angle brackets if they're not already present in the generic name
        if (!hasAngleBrackets)
        {
            result += "<";
            result += genericName;
            result += ">";
        }
        else
        {
            // If the generic name already has angle brackets, just append it
            result += genericName;
        }

        logMessage(LMI, "INFO", "Symbol Table", "Generated generic name: %s", result.c_str());

        return result;
    }

    void GlobalSymbolTable::registerGenericInstantiation(const char *baseName, DataType **typeArgs, int argCount, DataType *resultType)
    {
        // Create a unique name for this instantiation
        std::string instantiationName = createGenericInstantiationName(baseName, typeArgs, argCount);

        TypeSymbol *symbol = createIncompleteTypeSymbol(instantiationName.c_str(), GENERIC_TYPE);
        symbol->isGenericType = false; // It's a concrete instantiation
        symbol->type = resultType;

        // Reference the base generic type
        TypeSymbol *baseSymbol = findTypeSymbol(baseName);
        symbol->generics.baseGenericType = baseSymbol;

        // Store the type arguments
        symbol->generics.typeArguments = (TypeSymbol **)malloc(argCount * sizeof(TypeSymbol *));
        symbol->generics.argCount = argCount;

        // Add to type table
        addTypeToTable(symbol);
    }

    std::string GlobalSymbolTable::createGenericInstantiationName(const char *baseName, DataType **typeArgs, int argCount)
    {
        // Start with the base name
        std::string name = baseName;
        name += "<";

        // Add each type argument
        for (int i = 0; i < argCount; i++)
        {
            // Add the type name
            DataType *type = typeArgs[i];
            name += DTM->debug->dataTypeToString(type);

            // Add comma if not the last argument
            if (i < argCount - 1)
            {
                name += ", ";
            }
        }

        name += ">";

        // Add a hash to ensure uniqueness
        std::hash<std::string> hasher;
        size_t hash = hasher(name);
        std::stringstream hashStr;
        hashStr << "_" << std::hex << hash;

        return name + hashStr.str();
    }

} // namespace Cryo
