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
