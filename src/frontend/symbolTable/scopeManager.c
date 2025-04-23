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

FrontendScope *FrontendSymbolTable_createScope(FrontendSymbolTable *self, const char *name, ScopeType type)
{
    if (self->scopeCount >= MAX_SCOPES)
    {
        fprintf(stderr, "Error: Maximum number of scopes reached\n");
        return NULL;
    }

    FrontendScope *scope = (FrontendScope *)malloc(sizeof(FrontendScope));
    if (!scope)
    {
        fprintf(stderr, "Error: Failed to allocate memory for scope\n");
        return NULL;
    }

    scope->name = strdup(name) ? strdup(name) : NULL;
    scope->id = strdup(name) ? strdup(name) : NULL; // TODO: Generate a unique ID
    scope->symbols = (FrontendSymbol **)malloc(sizeof(FrontendSymbol *) * MAX_SYMBOLS);
    if (!scope->symbols)
    {
        fprintf(stderr, "Error: Failed to allocate memory for scope symbols\n");
        free(scope);
        return NULL;
    }
    scope->symbolCount = 0;
    scope->symbolCapacity = MAX_SYMBOLS;
    scope->type = type;
    scope->depth = self->scopeCount;
    scope->symbolCount = 0;
    scope->symbolCapacity = MAX_SYMBOLS;

    scope->parent = self->scopeStackSize >= 0 ? self->scopeStack[self->scopeStackSize] : NULL;

    self->scopes[self->scopeCount++] = scope;
    self->scopeStack[++self->scopeStackSize] = scope;

    return scope;
}

int FrontendSymbolTable_addSymbolToScope(FrontendSymbolTable *self, FrontendSymbol *symbol)
{
    if (self->scopeStackSize < 0)
    {
        fprintf(stderr, "Error: No active scope to add symbol to\n");
        return -1;
    }

    FrontendScope *currentScope = self->currentScope;
    if (!currentScope)
    {
        fprintf(stderr, "Error: Current scope is NULL\n");
        return -1;
    }
    if (currentScope->symbolCount >= MAX_SYMBOLS)
    {
        fprintf(stderr, "Error: Maximum number of symbols reached in current scope\n");
        return -1;
    }

    currentScope->symbols[currentScope->symbolCount++] = symbol;
    return 0;
}

void FrontendSymbolTable_pushScope(FrontendSymbolTable *self, FrontendScope *scope)
{
    if (self->scopeStackSize >= SCOPE_STACK_SIZE - 1)
    {
        fprintf(stderr, "Error: Scope stack overflow\n");
        return;
    }
    self->scopeStack[++self->scopeStackSize] = scope;
}

void FrontendSymbolTable_enterScope(FrontendSymbolTable *self, const char *name, ScopeType type)
{
    // Validate input parameters
    if (!self)
    {
        fprintf(stderr, "Error: Cannot enter scope on NULL symbol table\n");
        DEBUG_BREAKPOINT;
        return;
    }

    // Check if scope stack has room for another scope
    if (self->scopeStackSize >= SCOPE_STACK_SIZE)
    {
        fprintf(stderr, "Error: Scope stack overflow, cannot enter new scope\n");
        DEBUG_BREAKPOINT;
        return;
    }

    // Create a new scope object
    FrontendScope *newScope = FrontendSymbolTable_createScope(self, name, type);
    if (!newScope)
    {
        fprintf(stderr, "Error: Failed to create new scope for '%s'\n",
                name ? name : "unnamed");
        DEBUG_BREAKPOINT;
        return;
    }

    // Update current scope pointer to the newly created scope
    self->currentScope = newScope;

    // Log the scope entry for debugging
    logMessage(LMI, "INFO", "SymbolTable",
               "Entered scope '%s' (Type: %s, Depth: %zu)",
               newScope->name,
               getScopeTypeString(newScope->type),
               newScope->depth);
}

void FrontendSymbolTable_exitScope(FrontendSymbolTable *self)
{
    // Validate input parameter
    if (!self)
    {
        fprintf(stderr, "Error: Cannot exit scope on NULL symbol table\n");
        DEBUG_BREAKPOINT;
        return;
    }

    // Check if there's a scope to exit
    if (self->scopeStackSize <= 1)
    { // Keep at least global scope
        fprintf(stderr, "Error: Cannot exit global scope\n");
        DEBUG_BREAKPOINT;
        return;
    }

    // Log before popping for debugging
    if (self->currentScope)
    {
        logMessage(LMI, "INFO", "SymbolTable",
                   "Exiting scope '%s' (Type: %s, Depth: %zu)",
                   self->currentScope->name,
                   getScopeTypeString(self->currentScope->type),
                   self->currentScope->depth);
    }

    // Pop the top scope from the stack
    self->scopeStackSize--;

    // Update current scope to the new top of stack
    if (self->scopeStackSize > 0)
    {
        self->currentScope = self->scopeStack[self->scopeStackSize - 1];
    }
    else
    {
        // This should never happen if we check for scopeStackSize <= 1 above
        // But as a safeguard, set to NULL if somehow we ended up with an empty stack
        self->currentScope = NULL;
        fprintf(stderr, "Warning: Scope stack is now empty\n");
    }

    // Note: We don't free the scope memory here since we keep all scopes
    // in the self->scopes array for the lifetime of the symbol table
    // This makes lookups easier and allows referring to parent scopes
}
