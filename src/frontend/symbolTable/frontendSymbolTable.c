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

void initFrontendSymbolTable(void)
{
    g_frontendSymbolTable = CreateSymbolTable();
    if (!g_frontendSymbolTable)
    {
        fprintf(stderr, "Error: Failed to create frontend symbol table\n");
        exit(EXIT_FAILURE);
    }
}

FrontendScope *FrontendSymbolTable_getCurrentScope(FrontendSymbolTable *self)
{
    if (self->scopeStackSize < 0)
    {
        return NULL;
    }
    return self->scopeStack[self->scopeStackSize];
}

void FrontendSymbolTable_addSymbol(FrontendSymbolTable *self, ASTNode *node)
{
    if (!self || self == NULL)
    {
        fprintf(stderr, "Error: FrontendSymbolTable is NULL\n");
        return;
    }

    if (!node || node == NULL)
    {
        fprintf(stderr, "Error: ASTNode is NULL\n");
        return;
    }

    FrontendSymbol *symbol = astNodeToSymbol(node);
    if (!symbol)
    {
        fprintf(stderr, "Error: Failed to create symbol from ASTNode\n");
        return;
    }

    // Add the symbol to the current scope
    int result = FrontendSymbolTable_addSymbolToScope(self, symbol);
    if (result != 0)
    {
        fprintf(stderr, "Error: Failed to add symbol to scope\n");
        return;
    }

    return;
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
    if (self->currentNamespace)
    {
        self->currentNamespace = NULL;
    }
    self->currentNamespace = strdup(namespaceName);
    if (!self->currentNamespace)
    {
        fprintf(stderr, "Error: Failed to allocate memory for namespace\n");
        return;
    }
}

void FrontendSymbolTable_exitNamespace(FrontendSymbolTable *self)
{
    if (self->currentNamespace)
    {
        self->currentNamespace = NULL;
    }
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

    // Initialize pointers to NULL first
    table->symbols = NULL;
    table->scopes = NULL;
    table->scopeStack = NULL;
    table->currentScope = NULL;
    table->currentNamespace = NULL; // Initialize namespace to NULL

    // Allocate memory for arrays
    table->symbols = (FrontendSymbol **)malloc(sizeof(FrontendSymbol *) * MAX_SYMBOLS);
    if (!table->symbols)
    {
        fprintf(stderr, "Error: Failed to allocate memory for symbols array\n");
        free(table);
        return NULL;
    }

    table->scopes = (FrontendScope **)malloc(sizeof(FrontendScope *) * MAX_SCOPES);
    if (!table->scopes)
    {
        fprintf(stderr, "Error: Failed to allocate memory for scopes array\n");
        free(table->symbols);
        free(table);
        return NULL;
    }

    table->scopeStack = (FrontendScope **)malloc(sizeof(FrontendScope *) * SCOPE_STACK_SIZE);
    if (!table->scopeStack)
    {
        fprintf(stderr, "Error: Failed to allocate memory for scope stack\n");
        free(table->scopes);
        free(table->symbols);
        free(table);
        return NULL;
    }

    // Initialize arrays with NULL pointers
    for (size_t i = 0; i < MAX_SYMBOLS; i++)
    {
        table->symbols[i] = NULL;
    }

    for (size_t i = 0; i < MAX_SCOPES; i++)
    {
        table->scopes[i] = NULL;
    }

    for (size_t i = 0; i < SCOPE_STACK_SIZE; i++)
    {
        table->scopeStack[i] = NULL;
    }

    // Initialize other fields
    table->symbolCount = 0;
    table->scopeCount = 0;
    table->scopeStackSize = 0; // Initialize to 0, not -1
    table->symbolCapacity = MAX_SYMBOLS;
    table->scopeCapacity = MAX_SCOPES;

    // Initialize function pointers
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
    table->printTable = FrontendSymbolTable_printTable;

    // Initialize the scope stack with a global scope
    FrontendScope *globalScope = FrontendSymbolTable_createScope(table, "global", SCOPE_GLOBAL);
    if (!globalScope)
    {
        fprintf(stderr, "Error: Failed to create global scope\n");
        free(table->scopeStack);
        free(table->scopes);
        free(table->symbols);
        free(table);
        return NULL;
    }

    table->currentScope = globalScope;

    return table;
}