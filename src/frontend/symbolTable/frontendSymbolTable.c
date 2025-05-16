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
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create FrontendSymbolTable");
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
        logMessage(LMI, "ERROR", "SymbolTable", "SymbolTable is NULL");
        return;
    }

    if (!node || node == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "ASTNode is NULL");
        return;
    }

    logMessage(LMI, "INFO", "SymbolTable", "Adding symbol to symbol table");
    node->print(node);

    FrontendSymbol *symbol = astNodeToSymbol(node);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to convert ASTNode to FrontendSymbol");
        return;
    }

    // Add the symbol to the current scope
    int result = FrontendSymbolTable_addSymbolToScope(self, symbol);
    if (result != 0)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to add symbol to scope");
        return;
    }

    return;
}

FrontendSymbol *FrontendSymbolTable_lookup(FrontendSymbolTable *self, const char *name)
{
    // Only look up in the current scope
    if (!self || self == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "SymbolTable is NULL");
        return NULL;
    }

    if (!name || name == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Name is NULL");
        return NULL;
    }

    FrontendScope *scope = self->currentScope;
    if (!scope || scope == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Current scope is NULL");
        return NULL;
    }

    for (size_t i = 0; i < scope->symbolCount; i++)
    {
        if (strcmp(scope->symbols[i]->name, name) == 0)
        {
            return scope->symbols[i];
        }
    }
    return NULL;
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

FrontendSymbol *FrontendSymbolTable_lookupInScope(FrontendSymbolTable *self, const char *scopeName, const char *symbolName)
{
    // The `scopeName` is the key to the scope, and the name will be the key to the symbol
    logMessage(LMI, "INFO", "SymbolTable", "Resolving symbol '%s' in scope '%s'", symbolName, scopeName);

    size_t i;
    for (i = 0; i < self->scopeCount; i++)
    {
        if (strcmp(self->scopes[i]->name, scopeName) == 0)
        {
            logMessage(LMI, "INFO", "SymbolTable", "Found scope '%s'", scopeName);
            break;
        }
    }
    if (i == self->scopeCount)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Scope '%s' not found", scopeName);
        return NULL;
    }

    FrontendScope *scope = self->scopes[i];
    for (size_t j = 0; j < scope->symbolCount; j++)
    {
        if (strcmp(scope->symbols[j]->name, symbolName) == 0)
        {
            return scope->symbols[j];
        }
    }

    logMessage(LMI, "INFO", "SymbolTable", "Symbol '%s' not found in scope '%s'", symbolName, scopeName);
    return NULL;
}

FrontendSymbol *FrontendSymbolTable_lookupType(FrontendSymbolTable *self, const char *typeName)
{
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
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to allocate memory for namespace name");
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

void FrontendSymbolTable_addProtoType(FrontendSymbolTable *self, const char *name, DataType *protoType)
{
    if (!self || self == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "SymbolTable is NULL");
        return;
    }

    if (!name || name == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Name is NULL");
        return;
    }

    FrontendSymbol *symbol = (FrontendSymbol *)malloc(sizeof(FrontendSymbol));
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to allocate memory for FrontendSymbol");
        return;
    }
    symbol->name = strdup(name);
    if (!symbol->name)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to allocate memory for symbol name");
        free(symbol);
        return;
    }
    symbol->id = "UNDEF";                               // Set to "UNDEF" for now
    symbol->node = NULL;                                // Set to NULL for now
    symbol->type = protoType;                           // Set to prototype type
    symbol->scopeType = SCOPE_GLOBAL;                   // Set to global scope for now
    symbol->lineNumber = 0;                             // Set to 0 for now
    symbol->columnNumber = 0;                           // Set to 0 for now
    symbol->isDefined = false;                          // Set to false for now
    symbol->isPrototype = true;                         // Set to true for prototype
    symbol->print = FrontendSymbol_printFrontendSymbol; // Set to NULL for now

    // Add the symbol to the current scope
    int result = FrontendSymbolTable_addSymbolToScope(self, symbol);
    if (result != 0)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to add symbol to scope");
        return;
    }
    logMessage(LMI, "INFO", "SymbolTable", "Added prototype symbol '%s'", name);
    return;
}

void FrontendSymbolTable_completePrototypeSymbol(FrontendSymbolTable *self, const char *name, ASTNode *node, DataType *type)
{
    if (!self || self == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "SymbolTable is NULL");
        return;
    }

    if (!name || name == NULL)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Name is NULL");
        return;
    }

    FrontendSymbol *symbol = FrontendSymbolTable_lookup(self, name);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to find symbol '%s'", name);
        return;
    }

    symbol->node = node;
    symbol->type = type;
}

FrontendSymbolTable *CreateSymbolTable(void)
{
    __STACK_FRAME__
    FrontendSymbolTable *table = (FrontendSymbolTable *)malloc(sizeof(FrontendSymbolTable));
    if (!table)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to allocate memory for FrontendSymbolTable");
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
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to allocate memory for symbols array");
        free(table);
        return NULL;
    }

    table->scopes = (FrontendScope **)malloc(sizeof(FrontendScope *) * MAX_SCOPES);
    if (!table->scopes)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to allocate memory for scopes array");
        return NULL;
    }

    table->scopeStack = (FrontendScope **)malloc(sizeof(FrontendScope *) * SCOPE_STACK_SIZE);
    if (!table->scopeStack)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to allocate memory for scope stack");
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
    table->scopeStackSize = 0;
    table->symbolCapacity = MAX_SYMBOLS;
    table->scopeCapacity = MAX_SCOPES;

    // Initialize function pointers
    table->enterScope = FrontendSymbolTable_enterScope;
    table->exitScope = FrontendSymbolTable_exitScope;
    table->getCurrentScope = FrontendSymbolTable_getCurrentScope;

    table->addSymbol = FrontendSymbolTable_addSymbol;
    table->addProtoType = FrontendSymbolTable_addProtoType;
    table->completePrototypeSymbol = FrontendSymbolTable_completePrototypeSymbol;

    table->lookup = FrontendSymbolTable_lookup;
    table->lookupInGlobalScope = FrontendSymbolTable_lookupInGlobalScope;
    table->lookupInNamespaceScope = FrontendSymbolTable_lookupInNamespaceScope;
    table->lookupInScope = FrontendSymbolTable_lookupInScope;
    table->lookupType = FrontendSymbolTable_lookupType;

    table->enterNamespace = FrontendSymbolTable_enterNamespace;
    table->exitNamespace = FrontendSymbolTable_exitNamespace;
    table->getCurrentNamespace = FrontendSymbolTable_getCurrentNamespace;
    table->printTable = FrontendSymbolTable_printTable;

    // Initialize the scope stack with a global scope
    FrontendScope *globalScope = FrontendSymbolTable_createScope(table, "global", SCOPE_GLOBAL);
    if (!globalScope)
    {
        logMessage(LMI, "ERROR", "SymbolTable", "Failed to create global scope");
        return NULL;
    }

    table->currentScope = globalScope;

    return table;
}