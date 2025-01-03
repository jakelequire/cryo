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
    void GlobalSymbolTable::parseSymbolTableForTypes(SymbolTable *symbolTable)
    {
        if (!symbolTable || symbolTable == nullptr)
        {
            std::cerr << "parseSymbolTableForTypes: Symbol Table is null" << std::endl;
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
            std::cerr << "addDataTypeSymbol: Symbol is null" << std::endl;
            return -1;
        }
        TypeOfSymbol symbolType = symbol->symbolType;
        if (symbolType != TYPE_SYMBOL)
        {
            std::cerr << "addDataTypeSymbol: Symbol is not a Type Symbol" << std::endl;
            const char *symbolTypeStr = typeOfSymbolToString(symbolType);
            std::cerr << "addDataTypeSymbol: Symbol Type: " << symbolTypeStr << std::endl;
            return -1;
        }
        TypeSymbol *typeSymbol = symbol->type;
        addTypeToTable(typeSymbol);

        const char *typeName = typeSymbol->name;
        std::cout << "Type Added to Type Table: " << typeName << std::endl;

        return 0;
    }

    bool GlobalSymbolTable::doesTypeExist(const char *name)
    {
        std::cout << "Checking if Type Exists: " << name << std::endl;
        if (!typeTable || typeTable == nullptr)
        {
            std::cerr << "doesTypeExist: Type Table is null" << std::endl;
            return false;
        }
        if (!name || name == nullptr)
        {
            std::cerr << "doesTypeExist: Name is null" << std::endl;
            return false;
        }

        std::cout << "<!> Looking for Type: " << name << std::endl;
        size_t count = typeTable->count;
        std::cout << "Type Table Count: " << count << std::endl;
        for (size_t i = 0; i < count; i++)
        {
            TypeSymbol *typeSymbol = typeTable->typeSymbols[i];
            if (!typeSymbol || typeSymbol == nullptr)
            {
                std::cerr << "doesTypeExist: Type Symbol is null" << std::endl;
                continue;
            }
            const char *typeName = typeTable->typeSymbols[i]->name;
            std::cout << "Checking Type: " << typeName << std::endl;
            if (strcmp(typeName, name) == 0)
            {
                std::cout << "Type Exists in Type Table: " << name << std::endl;
                return true;
            }
        }

        return false;
    }

    void GlobalSymbolTable::addTypeToTable(TypeSymbol *typeSymbol)
    {
        if (!typeSymbol || typeSymbol == nullptr)
        {
            std::cerr << "addTypeToTable: Type Symbol is null" << std::endl;
            return;
        }

        const char *typeName = typeSymbol->name;
        if (doesTypeExist(typeName))
        {
            std::cerr << "addTypeToTable: Type already exists in table" << std::endl;
            return;
        }

        typeTable->typeSymbols[typeTable->count] = typeSymbol;
        typeTable->count++;

        std::cout << "Type Added to Type Table: " << typeName << std::endl;
        return;
    }

    // -------------------------------------------------------

    void GlobalSymbolTable::initTypeDefinition(Symbol *typeSymbol)
    {
        std::cout << "Initializing Type Definition" << std::endl;
        if (!typeSymbol || typeSymbol == nullptr)
        {
            std::cerr << "initTypeDefinition: Type Symbol is null" << std::endl;
            return;
        }
        if (typeSymbol->symbolType != TYPE_SYMBOL)
        {
            std::cerr << "initTypeDefinition: Symbol is not a Type Symbol" << std::endl;
            return;
        }

        TypeSymbol *type = typeSymbol->type;
        const char *typeName = type->name;
        std::cout << "Creating Type Definition for: " << typeName << std::endl;
        if (!typeName || typeName == nullptr)
        {
            std::cerr << "initTypeDefinition: Type Name is null" << std::endl;
            return;
        }

        if (doesTypeExist(typeName))
        {
            std::cerr << "initTypeDefinition: Type already exists in table" << std::endl;
            return;
        }
        std::cout << "Type DOES NOT EXIST. Adding Type to Table" << std::endl;
        size_t tableCount = typeTable->count;
        std::cout << "Type Table Count: " << tableCount << std::endl;
        typeTable->typeSymbols[tableCount] = type;
        typeTable->count++;

        std::cout << "Type Added to Type Table: " << typeName << std::endl;
        return;
    }

    // After the type def is complete. We need to update the type symbol in the table with the new typeSymbol
    void GlobalSymbolTable::completeTypeDefinition(Symbol *typeSymbol, const char *typeName)
    {
    }

} // namespace Cryo
