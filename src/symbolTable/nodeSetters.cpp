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

    void GlobalSymbolTable::addSymbolsToSymbolTable(Symbol **symbols, SymbolTable *table)
    {
        if (!symbols || symbols == nullptr)
        {
            std::cout << "addSymbolsToSymbolTable: Symbols are null" << std::endl;
            return;
        }
        if (!table || table == nullptr)
        {
            std::cout << "addSymbolsToSymbolTable: Symbol Table is null" << std::endl;
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

    void GlobalSymbolTable::addSingleSymbolToTable(Symbol *symbol, SymbolTable *table)
    {
        if (!symbol || symbol == nullptr)
        {
            std::cout << "addSingleSymbolToTable: Symbol is null" << std::endl;
            return;
        }
        if (!table || table == nullptr)
        {
            std::cout << "addSingleSymbolToTable: Symbol Table is null" << std::endl;
            return;
        }

        table->symbols[table->count] = symbol;
        table->count++;

        return;
    }

} // namespace Cryo
