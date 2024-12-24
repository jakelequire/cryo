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
#ifndef C_INTERFACE_TABLE_H
#define C_INTERFACE_TABLE_H
// This header file is to give C access to the C++ symbol table
// For easier access to the symbol table.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "frontend/dataTypes.h"
#include "frontend/AST.h"
#include "symbolTable/globalSymtable.hpp"
#include "symbolTable/symdefs.h"

typedef struct ASTNode ASTNode;
typedef struct SymbolTable SymbolTable;
typedef struct VariableSymbol VariableSymbol;
typedef struct FunctionSymbol FunctionSymbol;
typedef struct ExternSymbol ExternSymbol;
typedef struct TypeSymbol TypeSymbol;
typedef struct PropertySymbol PropertySymbol;
typedef struct MethodSymbol MethodSymbol;
typedef struct ScopeBlock ScopeBlock;

// C API -------------------------------------------------------
CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create(void);
void CryoGlobalSymbolTable_PrintGlobalTable(CryoGlobalSymbolTable *symTable);

// Class State Functions ---------------------------------------

bool CryoGlobalSymbolTable_GetIsPrimaryTable(CryoGlobalSymbolTable *symTable);
bool CryoGlobalSymbolTable_GetIsDependencyTable(CryoGlobalSymbolTable *symTable);

void CryoGlobalSymbolTable_SetPrimaryTableStatus(CryoGlobalSymbolTable *symTable, bool isPrimary);
void CryoGlobalSymbolTable_SetDependencyTableStatus(CryoGlobalSymbolTable *symTable, bool isDependency);

// Symbol Table Functions ---------------------------------------

void CryoGlobalSymbolTable_InitDependencyTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);
void CryoGlobalSymbolTable_CreatePrimaryTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);

void CryoGlobalSymbolTable_AddNodeToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node);
void CryoGlobalSymbolTable_AddVariableToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node, const char *scopeID);

SymbolTable *CryoGlobalSymbolTable_GetCurrentSymbolTable(CryoGlobalSymbolTable *symTable);

// Scope Functions ---------------------------------------

void CryoGlobalSymbolTable_EnterScope(CryoGlobalSymbolTable *symTable, const char *name);
void CryoGlobalSymbolTable_ExitScope(CryoGlobalSymbolTable *symTable);
const char *CryoGlobalSymbolTable_GetScopeID(CryoGlobalSymbolTable *symTable, const char *name);

// Class State Functions
#define isPrimaryTable(symTable) \
    CryoGlobalSymbolTable_GetIsPrimaryTable(symTable)
#define isDependencyTable(symTable) \
    CryoGlobalSymbolTable_GetIsDependencyTable(symTable)
#define setPrimaryTableStatus(symTable, isPrimary) \
    CryoGlobalSymbolTable_SetPrimaryTableStatus(symTable, isPrimary)
#define setDependencyTableStatus(symTable, isDependency) \
    CryoGlobalSymbolTable_SetDependencyTableStatus(symTable, isDependency)
#define TableFinished(symTable) \
    CryoGlobalSymbolTable_TableFinished(symTable)

// Symbol Table Functions
#define initDependencySymbolTable(symTable, namespaceName) \
    CryoGlobalSymbolTable_InitDependencyTable(symTable, namespaceName)
#define createPrimarySymbolTable(symTable, namespaceName) \
    CryoGlobalSymbolTable_CreatePrimaryTable(symTable, namespaceName)
#define addNodeToSymbolTable(symTable, node) \
    CryoGlobalSymbolTable_AddNodeToSymbolTable(symTable, node)

#define AddVariableToSymbolTable(symTable, node, scopeID) \
    CryoGlobalSymbolTable_AddVariableToSymbolTable(symTable, node, scopeID)
#define GetCurrentSymbolTable(symTable) \
    CryoGlobalSymbolTable_GetCurrentSymbolTable(symTable)

// Scope Functions
#define EnterScope(symTable, name) \
    CryoGlobalSymbolTable_EnterScope(symTable, name)
#define ExitScope(symTable) \
    CryoGlobalSymbolTable_ExitScope(symTable)
#define GetScopeID(symTable, name) \
    CryoGlobalSymbolTable_GetScopeID(symTable, name)

// Symbol Resolution Functions
#define GetFrontendVariableSymbol(symTable, name, scopeID) \
    CryoGlobalSymbolTable_GetFrontendVariableSymbol(symTable, name, scopeID)

// Debug Functions
#define printGlobalSymbolTable(symTable) \
    CryoGlobalSymbolTable_PrintGlobalTable(symTable)
#define MergeDBChunks(symTable) \
    CryoGlobalSymbolTable_MergeDBChunks(symTable)

// Declaration Functions
#define InitClassDeclaration(symTable, className) \
    CryoGlobalSymbolTable_InitClassDeclaration(symTable, className)
#define AddPropertyToClass(symTable, className, property) \
    CryoGlobalSymbolTable_AddPropertyToClass(symTable, className, property)
#define AddMethodToClass(symTable, className, method) \
    CryoGlobalSymbolTable_AddMethodToClass(symTable, className, method)

// Symbol Resolution Functions
#define GetFrontendSymbol(symTable, name, scopeID, symbolType) \
    CryoGlobalSymbolTable_GetFrontendSymbol(symTable, name, scopeID, symbolType)

#endif // C_INTERFACE_TABLE_H
