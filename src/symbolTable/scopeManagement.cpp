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
#include "diagnostics/diagnostics.h"

namespace Cryo
{
    void GlobalSymbolTable::enterScope(const char *name)
    {
        __STACK_FRAME__
        ScopeBlock *newScope = createScopeBlock(name, currentScope ? currentScope->depth + 1 : 0);
        newScope->parent = currentScope;
        if (currentScope)
        {
            if (currentScope->childCount >= currentScope->childCapacity)
            {
                currentScope->childCapacity = currentScope->childCapacity ? currentScope->childCapacity * 2 : 1;
                currentScope->children = (ScopeBlock **)realloc(currentScope->children, sizeof(ScopeBlock *) * currentScope->childCapacity);
            }
            currentScope->children[currentScope->childCount++] = newScope;
        }
        currentScope = newScope;
        scopeDepth++;
    }

    void GlobalSymbolTable::exitScope()
    {
        __STACK_FRAME__
        if (currentScope)
        {
            currentScope = currentScope->parent;
            scopeDepth--;
        }
    }

    void GlobalSymbolTable::initNamepsaceScope(const char *namespaceName)
    {
        __STACK_FRAME__
        ScopeBlock *newScope = createScopeBlock(namespaceName, 0);
        currentScope = newScope;
        scopeDepth = 0;
    }

    const char *GlobalSymbolTable::getScopeID(const char *name)
    {
        __STACK_FRAME__
        return IDGen::generate64BitHashID(name);
    }

} // namespace Cryo
