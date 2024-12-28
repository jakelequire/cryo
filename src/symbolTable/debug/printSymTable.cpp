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
    void GlobalSymbolTable::printGlobalTable(GlobalSymbolTable *table)
    {
        std::cout << "\n"
                  << std::endl;
        std::cout << "\n============ Global Symbol Table State ============\n"
                  << std::endl;

        // Print Debug Info
        std::cout << "Debug Information:" << std::endl;
        std::cout << "├── Build Dir: \t    " << debugInfo.buildDir << std::endl;
        std::cout << "├── Dependency Dir: " << debugInfo.dependencyDir << std::endl;
        std::cout << "├── Debug Dir: \t    " << debugInfo.debugDir << std::endl;
        std::cout << "└── DB Dir: \t    " << debugInfo.DBdir << std::endl;

        // Scope Information
        std::cout << "\nScope Information:" << std::endl;
        std::cout << "├── Scope ID:    " << scopeId << std::endl;
        std::cout << "└── Scope Depth: " << scopeDepth << std::endl;

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

    void GlobalSymbolTable::logSymbol(Symbol *symbol)
    {
        if (!symbol)
        {
            std::cerr << "Error: Symbol is null" << std::endl;
        }

        debugger->logSymbol(symbol);

        return;
    }

} // namespace Cryo
