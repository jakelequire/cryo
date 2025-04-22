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
#include "frontend/frontendSymbolTable.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

FrontendSymbolTable *g_frontendSymbolTable = NULL;

void FrontendSymbolTable_enterScope(FrontendSymbolTable *self, const char *name, ScopeType type)
{
    // TODO: Implement scope entering logic
    DEBUG_BREAKPOINT;
}

void FrontendSymbolTable_exitScope(FrontendSymbolTable *self)
{
    // TODO: Implement scope exiting logic
    DEBUG_BREAKPOINT;
}

FrontendScope *FrontendSymbolTable_getCurrentScope(FrontendSymbolTable *self)
{
    if (self->scopeStackSize < 0)
    {
        return NULL;
    }
    return self->scopeStack[self->scopeStackSize];
}

void FrontendSymbolTable_addSymbol(FrontendSymbolTable *self, FrontendSymbol *symbol)
{
    if (self->symbolCount >= MAX_SYMBOLS)
    {
        fprintf(stderr, "Error: Symbol table is full\n");
        return;
    }
    self->symbols[self->symbolCount++] = symbol;
}

FrontendSymbol *FrontendSymbolTable_lookup(FrontendSymbolTable *self, const char *name)
{
    // TODO: Implement symbol lookup logic
    DEBUG_BREAKPOINT;
}

FrontendSymbol *FrontendSymbolTable_lookupInGlobalScope(FrontendSymbolTable *self, const char *name)
{
    // TODO: Implement global scope lookup logic
    DEBUG_BREAKPOINT;
}

FrontendSymbol *FrontendSymbolTable_lookupInNamespaceScope(FrontendSymbolTable *self, const char *name, const char *namespaceName)
{
    // TODO: Implement namespace scope lookup logic
    DEBUG_BREAKPOINT;
}

void FrontendSymbolTable_enterNamespace(FrontendSymbolTable *self, const char *namespaceName)
{
    // TODO: Implement namespace entering logic
    DEBUG_BREAKPOINT;
}

void FrontendSymbolTable_exitNamespace(FrontendSymbolTable *self)
{
    // TODO: Implement namespace exiting logic
    DEBUG_BREAKPOINT;
}

const char *FrontendSymbolTable_getCurrentNamespace(FrontendSymbolTable *self)
{
    return self->currentNamespace;
}

FrontendSymbolTable *CreateSymbolTable(void)
{
    __STACK_FRAME__
    FrontendSymbolTable *table = (FrontendSymbolTable *)malloc(sizeof(FrontendSymbolTable));
    if (!table)
    {
        fprintf(stderr, "Error: Failed to allocate memory for symbol table\n");
        return NULL;
    }

    table->symbols = (FrontendSymbol **)malloc(sizeof(FrontendSymbol *) * MAX_SYMBOLS);
    table->scopes = (FrontendScope **)malloc(sizeof(FrontendScope *) * MAX_SCOPES);
    table->scopeStack = (FrontendScope **)malloc(sizeof(FrontendScope *) * SCOPE_STACK_SIZE);
    table->symbolCount = 0;
    table->scopeCount = 0;
    table->scopeStackSize = -1;

    table->enterScope = FrontendSymbolTable_enterScope;
    table->exitScope = FrontendSymbolTable_exitScope;
    table->getCurrentScope = FrontendSymbolTable_getCurrentScope;

    table->addSymbol = FrontendSymbolTable_addSymbol;
    table->lookup = FrontendSymbolTable_lookup;
    table->lookupInGlobalScope = FrontendSymbolTable_lookupInGlobalScope;
    table->lookupInNamespaceScope = FrontendSymbolTable_lookupInNamespaceScope;

    table->enterNamespace = FrontendSymbolTable_enterNamespace;
    table->exitNamespace = FrontendSymbolTable_exitNamespace;
    table->getCurrentNamespace = FrontendSymbolTable_getCurrentNamespace;

    return table;
}
