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
#ifndef FRONTEND_SYMBOL_TABLE_H
#define FRONTEND_SYMBOL_TABLE_H

#include "frontend/AST.h"
#include "frontend/tokens.h"
#include "dataTypes/dataTypes.h"
#include "tools/logger/logger_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>

#define MAX_SYMBOLS 1024
#define MAX_SCOPES 256
#define SCOPE_STACK_SIZE 32

// Forward declarations
typedef struct FrontendSymbol_t FrontendSymbol;
typedef struct FrontendScope_t FrontendScope;
typedef struct FrontendSymbolTable_t FrontendSymbolTable;

extern FrontendSymbolTable *g_frontendSymbolTable;

typedef enum
{
    SCOPE_GLOBAL,
    SCOPE_NAMESPACE,
    SCOPE_FUNCTION,
    SCOPE_BLOCK,
    SCOPE_CLASS,
    SCOPE_STRUCT,
    SCOPE_UNKNOWN
} ScopeType;

typedef struct FrontendScope_t
{
    const char *name;         // Scope name (function name, class name, etc)
    const char *id;           // Unique scope ID
    ScopeType type;           // Type of scope
    size_t depth;             // Nesting depth
    struct Scope_t *parent;   // Parent scope
    FrontendSymbol **symbols; // Symbols in this scope
    size_t symbolCount;       // Number of symbols in scope
    size_t symbolCapacity;    // Maximum symbols in scope
} FrontendScope;

typedef struct FrontendSymbolTable_t
{
    // Global storage
    FrontendSymbol **symbols; // All symbols
    size_t symbolCount;       // Total number of symbols
    size_t symbolCapacity;    // Maximum total symbols

    // Scope management
    FrontendScope **scopes; // All scopes
    size_t scopeCount;      // Number of scopes
    size_t scopeCapacity;   // Maximum number of scopes

    // Current scope tracking
    FrontendScope *currentScope; // Current active scope
    FrontendScope **scopeStack;  // Stack of nested scopes
    size_t scopeStackSize;       // Current stack depth

    // Namespace tracking
    const char *currentNamespace; // Current namespace name

    // -----------------------------------------------------------
    // Symbol Table Methods

    // Scope management
    void (*enterScope)(FrontendSymbolTable *self, const char *name, ScopeType type);
    void (*exitScope)(FrontendSymbolTable *self);
    FrontendScope *(*getCurrentScope)(FrontendSymbolTable *self);

    // Symbol management
    void (*addSymbol)(FrontendSymbolTable *self, FrontendSymbol *symbol);
    FrontendSymbol *(*lookup)(FrontendSymbolTable *self, const char *name);
    FrontendSymbol *(*lookupInGlobalScope)(FrontendSymbolTable *self, const char *name);
    FrontendSymbol *(*lookupInNamespaceScope)(FrontendSymbolTable *self, const char *name, const char *namespaceName);

    // Namespace management
    void (*enterNamespace)(FrontendSymbolTable *self, const char *namespaceName);
    void (*exitNamespace)(FrontendSymbolTable *self);
    const char *(*getCurrentNamespace)(FrontendSymbolTable *self);
} FrontendSymbolTable;

// Table lifecycle
FrontendSymbolTable *CreateSymbolTable(void);

#endif // FRONTEND_SYMBOL_TABLE_H
