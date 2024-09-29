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
#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "utils/utility.h"
#include "utils/arena.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

typedef struct ASTNode ASTNode;

typedef struct
{
    ASTNode *node;
    char *name;
    CryoNodeType nodeType;
    CryoDataType valueType;
    int scopeLevel;
    bool isConstant;
    int argCount;
    int line;
    int column;
} CryoSymbol;

typedef struct CryoSymbolTable
{
    CryoSymbol **symbols;
    int count;
    int capacity;
    int scopeDepth;
    void (*printSymbolTable)(struct CryoSymbolTable *table);
} CryoSymbolTable;

CryoSymbolTable *createSymbolTable(Arena *arena);
void freeSymbolTable(CryoSymbolTable *table, Arena *arena);
void printSymbolTable(CryoSymbolTable *table);
void enterScope(CryoSymbolTable *table, Arena *arena);
void jumpScope(CryoSymbolTable *table, Arena *arena);
void exitScope(CryoSymbolTable *table, Arena *arena);
void enterBlockScope(CryoSymbolTable *table, Arena *arena);
void exitBlockScope(CryoSymbolTable *table, Arena *arena);
CryoSymbol *findSymbol(CryoSymbolTable *table, const char *name, Arena *arena);
bool isSymbolInTable(CryoSymbolTable *table, char *name);
char *getNameOfNode(ASTNode *node);
CryoSymbol *createCryoSymbol(CryoSymbolTable *table, ASTNode *node, Arena *arena);
void addASTNodeSymbol(CryoSymbolTable *table, ASTNode *node, Arena *arena);

void addDefinitionToSymbolTable(CryoSymbolTable *table, ASTNode *node, Arena *arena);
void updateExistingSymbol(CryoSymbolTable *table, ASTNode *node, Arena *arena);

bool analyzeNode(ASTNode *node, CryoSymbolTable *table, Arena *arena);

// C++ Accessable Functions
#ifdef __cplusplus
extern "C"
{
    void printSymbolTableCXX(CryoSymbolTable *table);
}
#endif

#endif // SYMTABLE_H