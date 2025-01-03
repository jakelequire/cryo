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
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "frontend/AST.h"
#include "frontend/tokens.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "tools/utils/utility.h"
#include "tools/arena/arena.h"
#include "tools/utils/fs.h"
#include "common/common.h"
#include "settings/compilerSettings.h"
#include "frontend/dataTypes.h"

typedef struct ASTNode ASTNode;
typedef struct CompilerState CompilerState;
typedef struct DataType DataType;

typedef struct CryoSymbol
{
    ASTNode *node;
    char *name;
    CryoNodeType nodeType;
    DataType *type;
    int scopeLevel;
    bool isConstant;
    int argCount;
    int line;
    int column;
    char *module;
} CryoSymbol;

typedef struct CryoSymbolTable
{
    CryoSymbol **symbols;
    int count;
    int capacity;
    int scopeDepth;
    char *namespaceName;
    void (*printSymbolTable)(struct CryoSymbolTable *table);
} CryoSymbolTable;

CryoSymbolTable *createSymbolTable(Arena *arena);
void setNamespace(CryoSymbolTable *table, const char *name);
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

char *getCurrentNamespace(CryoSymbolTable *table);
CryoSymbol *findImportedSymbol(CryoSymbolTable *table, const char *name, const char *module, Arena *arena);
void importAstTreeDefs(ASTNode *root, CryoSymbolTable *table, Arena *arena, CompilerState *state);
void importRuntimeDefinitionsToSymTable(CryoSymbolTable *table, ASTNode *runtimeNode, Arena *arena);

CryoSymbol *resolveModuleSymbol(const char *moduleName, const char *symbolName, CryoSymbolTable *table, Arena *arena);

// C++ Accessable Functions
#ifdef __cplusplus
extern "C"
{
    void printSymbolTableCXX(CryoSymbolTable *table);
}
#endif

// # =================================================== #
// # SymTable Output (./outputSymTable.c)                #
// # =================================================== #

typedef struct SymTableOutput
{
    FILE *file;
    const char *fileName;
    const char *filePath;
    const char *fileExt;
    const char *cwd;
} SymTableOutput;

SymTableOutput *createSymTableOutput(const char *fileName, const char *filePath, const char *fileExt, const char *cwd);
void outputSymTable(CryoSymbolTable *table, CompilerSettings *settings);
char *symTableOutBuffer(CryoSymbolTable *table);
void createSymTableOutputFile(const char *buffer, SymTableOutput *symTableOutput);
void removePrevSymTableOutput(const char *filePath);

#endif // SYMTABLE_H
