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
    SymbolTable *GlobalSymbolTable::mergeAllSymbols(void)
    {
        std::vector<Symbol *> primarySymbols = mergePrimaryTable();
        std::vector<Symbol *> dependencySymbols = mergeAllDependencyTables();
        std::vector<Symbol *> allSymbols = mergeTwoSymbolVectors(primarySymbols, dependencySymbols);
        size_t symbolCount = allSymbols.size();
        SymbolTable *table = createNewSymbolTableFromSymbols(allSymbols);

        if (isForReaping)
        {
            reapedTable = table;
            reapedTable->count = symbolCount;
        }

        return table;
    }

    std::vector<Symbol *> GlobalSymbolTable::mergePrimaryTable(void)
    {
        SymbolTable *table = getPrimaryTable();
        if (!table)
        {
            std::cout << "Primary Table is null" << std::endl;
            return std::vector<Symbol *>();
        }
        int count = table->count;
        Symbol **symbols = table->symbols;
        std::vector<Symbol *> mergedSymbols;
        for (int i = 0; i < count; i++)
        {
            mergedSymbols.push_back(symbols[i]);
        }
        return mergedSymbols;
    }

    std::vector<Symbol *> GlobalSymbolTable::mergeAllDependencyTables(void)
    {
        std::cout << "Merging All Dependency Tables" << std::endl;
        std::vector<Symbol *> mergedSymbols;

        for (auto &table : dependencyTableVector)
        {
            int count = table->count;
            Symbol **symbols = table->symbols;
            for (int i = 0; i < count; i++)
            {
                mergedSymbols.push_back(symbols[i]);
            }
        }
        return mergedSymbols;
    }

    std::vector<Symbol *> GlobalSymbolTable::mergeTwoSymbolVectors(std::vector<Symbol *> symbols1, std::vector<Symbol *> symbols2)
    {
        std::vector<Symbol *> mergedSymbols;
        for (auto &symbol : symbols1)
        {
            mergedSymbols.push_back(symbol);
        }
        for (auto &symbol : symbols2)
        {
            mergedSymbols.push_back(symbol);
        }
        return mergedSymbols;
    }

    std::vector<Symbol *> GlobalSymbolTable::mergeTwoTables(SymbolTable *table1, SymbolTable *table2)
    {
        std::vector<Symbol *> mergedSymbols;
        if (!table1 || !table2)
        {
            return mergedSymbols;
        }
        int count1 = table1->count;
        Symbol **symbols1 = table1->symbols;
        for (int i = 0; i < count1; i++)
        {
            mergedSymbols.push_back(symbols1[i]);
        }
        int count2 = table2->count;
        Symbol **symbols2 = table2->symbols;
        for (int i = 0; i < count2; i++)
        {
            mergedSymbols.push_back(symbols2[i]);
        }
        return mergedSymbols;
    }

    SymbolTable *GlobalSymbolTable::createNewSymbolTableFromSymbols(std::vector<Symbol *> symbols)
    {
        const char *namespaceName = this->currentNamespace;
        SymbolTable *table = createSymbolTable(namespaceName);
        for (auto &symbol : symbols)
        {
            addSymbolToTable(symbol, table);
        }
        return table;
    }

} // namespace Cryo
