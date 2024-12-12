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

    // -------------------------------------------------------

    void GlobalSymbolTable::createPrimaryTable(const char *namespaceName)
    {
        SymbolTable *symbolTable = createSymbolTable(namespaceName);
        if (symbolTable)
        {
            setIsPrimaryTable(symbolTable);
            return;
        }
        return;
    }

    void GlobalSymbolTable::initDependencyTable(const char *namespaceName)
    {
        SymbolTable *table = createSymbolTable(namespaceName);
        if (table)
        {
            setCurrentDependencyTable(table);
            dependencyTableVector.push_back(table);
            dependencyCount++;
            return;
        }

        return;
    }

    void GlobalSymbolTable::addNodeToTable(ASTNode *node)
    {
        if (!node || node == nullptr)
        {
            return;
        }

        // Check wether we are in the primary table or a dependency table
        if (tableContext.isPrimary)
        {
            std::cout << "Adding to Primary Table" << std::endl;
            // Add to primary table
            if (symbolTable && symbolTable != nullptr)
            {
                // Add the node to the primary table
                Symbol *symbol = ASTNodeToSymbol(node);
                if (symbol)
                {
                    // Add the symbol to the symbol table
                    addSingleSymbolToTable(symbol, symbolTable);
                    return;
                }
                return;
            }
        }
        else if (tableContext.isDependency)
        {
            std::cout << "Adding to Dependency Table" << std::endl;
            // Add to dependency table
            if (currentDependencyTable && currentDependencyTable != nullptr)
            {
                // Add the node to the dependency table
                Symbol *symbol = ASTNodeToSymbol(node);
                if (symbol)
                {
                    // Add the symbol to the dependency table
                    addSingleSymbolToTable(symbol, currentDependencyTable);
                    return;
                }
                return;
            }
        }
    }

    void GlobalSymbolTable::completeDependencyTable()
    {
        if (currentDependencyTable)
        {
            // Clear the current dependency table, add to the dependency table vector
            currentDependencyTable = nullptr;
            return;
        }
    }

    // ========================================================
    // Table Debug View

    void GlobalSymbolTable::printGlobalTable(GlobalSymbolTable *table)
    {
        std::cout << "\n"
                  << std::endl;
        std::cout << "\n============ Global Symbol Table State ============\n"
                  << std::endl;

        // Print Debug Info
        std::cout << "Debug Information:" << std::endl;
        std::cout << "├── Build Dir: " << debugInfo.buildDir << std::endl;
        std::cout << "├── Dependency Dir: " << debugInfo.dependencyDir << std::endl;
        std::cout << "├── Debug Dir: " << debugInfo.debugDir << std::endl;
        std::cout << "└── DB Dir: " << debugInfo.DBdir << std::endl;

        // Print Main Symbol Table
        std::cout << "\nMain Symbol Table:" << std::endl;
        if (symbolTable && symbolTable->count > 0)
        {
            debugger->logSymbolTable(symbolTable);
        }
        else
        {
            std::cout << "└── [Empty]" << std::endl;
        }

        // Print Types Table
        std::cout << "\nTypes Table:" << std::endl;
        if (typeTable)
        {
            debugger->logTypeTable(typeTable);
        }
        else
        {
            std::cout << "└── [Empty]" << std::endl;
        }

        // Print Dependencies
        std::cout << "\nDependency Tables:" << std::endl;
        std::cout << "├── Count: " << dependencyCount << std::endl;
        std::cout << "├── Capacity: " << dependencyCapacity << std::endl;
        std::cout << "└── Dependencies:" << std::endl;

        for (size_t i = 0; i < dependencyTableVector.size(); i++)
        {
            SymbolTable *depTable = dependencyTableVector[i];
            if (depTable)
            {
                std::cout << "    ├── [" << i << "] Dependency Table:" << std::endl;
                std::cout << "    │   ";
                debugger->logSymbolTable(depTable);
            }
        }

        if (dependencyTableVector.empty())
        {
            std::cout << "    └── [Empty]" << std::endl;
        }

        std::cout << "\n============ End Global Symbol Table State ============\n"
                  << std::endl;
        std::cout << "\n"
                  << std::endl;
    }

} // namespace Cryo
