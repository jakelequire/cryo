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

// C API
CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create(void);
// void CryoGlobalSymbolTable_Destroy(CryoGlobalSymbolTable symTable);

void CryoGlobalSymbolTable_PrintGlobalTable(CryoGlobalSymbolTable *symTable);
#define printGlobalSymbolTable(symTable) CryoGlobalSymbolTable_PrintGlobalTable(symTable)

bool CryoGlobalSymbolTable_GetIsPrimaryTable(CryoGlobalSymbolTable *symTable);
bool CryoGlobalSymbolTable_GetIsDependencyTable(CryoGlobalSymbolTable *symTable);
#define isPrimaryTable(symTable) CryoGlobalSymbolTable_GetIsPrimaryTable(symTable)
#define isDependencyTable(symTable) CryoGlobalSymbolTable_GetIsDependencyTable(symTable)

void CryoGlobalSymbolTable_SetPrimaryTableStatus(CryoGlobalSymbolTable *symTable, bool isPrimary);
void CryoGlobalSymbolTable_SetDependencyTableStatus(CryoGlobalSymbolTable *symTable, bool isDependency);
#define setPrimaryTableStatus(symTable, isPrimary) CryoGlobalSymbolTable_SetPrimaryTableStatus(symTable, isPrimary)
#define setDependencyTableStatus(symTable, isDependency) CryoGlobalSymbolTable_SetDependencyTableStatus(symTable, isDependency)

void CryoGlobalSymbolTable_InitDependencyTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);
#define initDependencySymbolTable(symTable, namespaceName) CryoGlobalSymbolTable_InitDependencyTable(symTable, namespaceName)
void CryoGlobalSymbolTable_CreatePrimaryTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);
#define createPrimarySymbolTable(symTable, namespaceName) CryoGlobalSymbolTable_CreatePrimaryTable(symTable, namespaceName)

void CryoGlobalSymbolTable_AddNodeToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node);
#define addNodeToSymbolTable(symTable, node) CryoGlobalSymbolTable_AddNodeToSymbolTable(symTable, node)

#endif // C_INTERFACE_TABLE_H
