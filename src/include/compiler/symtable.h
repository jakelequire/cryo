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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

typedef struct ASTNode ASTNode;



typedef struct {
    ASTNode* node;
    char* name;
    CryoNodeType nodeType;
    CryoDataType valueType;
    int scopeLevel;
    bool isConstant;
    int argCount;
} CryoSymbol;

typedef struct CryoSymbolTable {
    CryoSymbol** symbols;
    int count;
    int capacity;
    int scopeDepth;
} CryoSymbolTable;


char* logSymCryoDataType(CryoDataType type);


CryoSymbolTable* createSymbolTable      (void);
void freeSymbolTable                    (CryoSymbolTable* table);
void printSymbolTable                   (CryoSymbolTable* table);
void enterScope                         (CryoSymbolTable* table);
void jumpScope                          (CryoSymbolTable* table);
void exitScope                          (CryoSymbolTable* table);
void enterBlockScope                    (CryoSymbolTable* table);
void exitBlockScope                     (CryoSymbolTable* table);
void addSymbol                          (CryoSymbolTable* table, CryoSymbol* symbol);
CryoSymbol* findSymbol                  (CryoSymbolTable* table, const char* name);

CryoSymbol* createCryoSymbol            (CryoSymbolTable* table, ASTNode* node);
void addASTNodeSymbol                   (CryoSymbolTable* table, ASTNode* node);

void traverseAST(ASTNode* node, CryoSymbolTable* table);
bool analyzeNode(ASTNode* node, CryoSymbolTable* table);


#endif // SYMTABLE_H