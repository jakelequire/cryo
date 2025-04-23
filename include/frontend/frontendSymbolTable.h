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

// ---------------------------------------------------------------------------------
// Forward declarations

/// ```
/// {
///     const char *name;    // Symbol name
///     const char *id;      // Unique symbol ID
///     ASTNode *node;       // AST node associated with the symbol
///     DataType *type;      // Data type of the symbol
///     ScopeType scopeType; // Scope type where the symbol is defined
///     size_t lineNumber;   // Line number in source code
///     size_t columnNumber; // Column number in source code
///     bool isDefined;      // Whether the symbol is defined or not
///
///     void (*print)(FrontendSymbol *self); // Print function for debugging
/// } FrontendSymbol;
/// ```
typedef struct FrontendSymbol_t FrontendSymbol;

/// ```
/// {
///     const char *name;         // Scope name (function name, class name, etc)
///     const char *id;           // Unique scope ID
///     ScopeType type;           // Type of scope
///     size_t depth;             // Nesting depth
///     struct Scope_t *parent;   // Parent scope
///     FrontendSymbol **symbols; // Symbols in this scope
///     size_t symbolCount;       // Number of symbols in scope
///     size_t symbolCapacity;    // Maximum symbols in scope
/// } FrontendScope;
/// ```
typedef struct FrontendScope_t FrontendScope;

///
typedef struct FrontendSymbolTable_t FrontendSymbolTable;

/// @brief The global instance of the frontend symbol table
/// @note This is a singleton instance that is used throughout the frontend
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

typedef struct FrontendSymbol_t
{
    const char *name;    // Symbol name
    const char *id;      // Unique symbol ID
    ASTNode *node;       // AST node associated with the symbol
    DataType *type;      // Data type of the symbol
    ScopeType scopeType; // Scope type where the symbol is defined
    size_t lineNumber;   // Line number in source code
    size_t columnNumber; // Column number in source code
    bool isDefined;      // Whether the symbol is defined or not

    void (*print)(FrontendSymbol *self); // Print function for debugging
} FrontendSymbol;

typedef struct FrontendScope_t
{
    const char *name;               // Scope name (function name, class name, etc)
    const char *id;                 // Unique scope ID
    ScopeType type;                 // Type of scope
    size_t depth;                   // Nesting depth
    struct FrontendScope_t *parent; // Parent scope
    FrontendSymbol **symbols;       // Symbols in this scope
    size_t symbolCount;             // Number of symbols in scope
    size_t symbolCapacity;          // Maximum symbols in scope
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
    void (*addSymbol)(FrontendSymbolTable *self, ASTNode *node);
    FrontendSymbol *(*lookup)(FrontendSymbolTable *self, const char *name);
    FrontendSymbol *(*lookupInGlobalScope)(FrontendSymbolTable *self, const char *name);
    FrontendSymbol *(*lookupInNamespaceScope)(FrontendSymbolTable *self, const char *name, const char *namespaceName);
    FrontendSymbol *(*lookupInScope)(FrontendSymbolTable *self, const char *scopeName, const char *symbolName);

    // Namespace management
    void (*enterNamespace)(FrontendSymbolTable *self, const char *namespaceName);
    void (*exitNamespace)(FrontendSymbolTable *self);
    const char *(*getCurrentNamespace)(FrontendSymbolTable *self);

    // Debugging
    void (*printTable)(FrontendSymbolTable *self);
} FrontendSymbolTable;

// Table lifecycle
void initFrontendSymbolTable(void);
FrontendSymbolTable *CreateSymbolTable(void);
FrontendSymbol *astNodeToSymbol(ASTNode *node);

// Scope Management
FrontendScope *FrontendSymbolTable_createScope(FrontendSymbolTable *self, const char *name, ScopeType type);
int FrontendSymbolTable_addSymbolToScope(FrontendSymbolTable *self, FrontendSymbol *symbol);
void FrontendSymbolTable_pushScope(FrontendSymbolTable *self, FrontendScope *scope);
void FrontendSymbolTable_enterScope(FrontendSymbolTable *self, const char *name, ScopeType type);
void FrontendSymbolTable_exitScope(FrontendSymbolTable *self);
const char *getScopeTypeString(ScopeType type);
// Debugging
void FrontendSymbolTable_printTable(FrontendSymbolTable *self);

// ----------------------------------------------

/// FEST - Frontend Symbol Table
/// @brief Macro to access the global frontend symbol table
#define FEST g_frontendSymbolTable

#endif // FRONTEND_SYMBOL_TABLE_H
