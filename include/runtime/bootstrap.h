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
#ifndef CRYO_BOOTSTRAP_H
#define CRYO_BOOTSTRAP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "frontend/AST.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/dataTypes.h"

#include "tools/utils/fs.h"

typedef struct TypeTable TypeTable;
typedef struct CompilerSettings CompilerSettings;
typedef struct CompilerState CompilerState;
typedef struct Lexer Lexer;
typedef struct Arena Arena;
typedef struct ASTNode ASTNode;

typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;
typedef struct CryoLinker_t *CryoLinker;

// C++ headers
#ifdef __cplusplus
extern "C"
{
#endif

    int preprocessRuntimeIR(ASTNode *runtimeNode, CompilerState *state, const char *outputPath, CryoLinker *cLinker);
    int generateImportCode(ASTNode *importNode, CompilerState *state, CryoLinker *cLinker, const char *outputPath);

#ifdef __cplusplus
}
#endif

enum BootstrapStatus
{
    BOOTSTRAP_IDLE = -1,
    BOOTSTRAP_SUCCESS,
    BOOTSTRAP_FAILED,
    BOOTSTRAP_IN_PROGRESS
};

typedef struct Bootstrapper
{
    Arena *arena;
    TypeTable *typeTable;
    Lexer *lexer;
    CompilerState *state;
    ASTNode *programNode;
    enum BootstrapStatus status;
} Bootstrapper;

void boostrapRuntimeDefinitions(TypeTable *typeTable, CryoGlobalSymbolTable *globalTable, CryoLinker *cLinker);
Bootstrapper *initBootstrapper(const char *filePath);
void updateBootstrapStatus(Bootstrapper *bootstrapper, enum BootstrapStatus status);

ASTNode *compileForRuntimeNode(Bootstrapper *bootstrap, const char *filePath, CryoGlobalSymbolTable *globalTable);

#endif // CRYO_BOOTSTRAP_H
