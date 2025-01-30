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
#ifndef GLOBAL_COMPILER_H
#define GLOBAL_COMPILER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "frontend/AST.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/tokens.h"
#include "frontend/dataTypes.h"
#include "tools/utils/fs.h"
#include "common/common.h"
#include "settings/compilerSettings.h"
#include "runtime/bootstrap.h"
#include "tools/utils/cTypes.h"
#include "semantics/semantics.h"

typedef struct CompilerSettings CompilerSettings;
typedef struct CompilerState CompilerState;
typedef struct ASTNode ASTNode;
typedef struct Bootstrapper Bootstrapper;
typedef struct SymbolTable SymbolTable;
typedef struct TypesTable TypesTable;

// C++ headers
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct CryoLinker_t *CryoLinker;
    int generateCodeWrapper(ASTNode *node, CompilerState *state, CryoLinker *linker);
    int preprocessRuntimeIR(ASTNode *runtimeNode, CompilerState *state, const char *outputPath, CryoLinker *cLinker);
    int generateImportCode(ASTNode *importNode, CompilerState *state, CryoLinker *cLinker, const char *outputPath);

#ifdef __cplusplus
}
#endif

// Compiler Functions
int cryoCompiler(const char *filePath, CompilerSettings *settings);
ASTNode *compileForProgramNode(const char *filePath);
int compileImportFile(const char *filePath, CompilerSettings *settings);

// Source Text Compiler
int sourceTextCompiler(char *filePath, CompilerSettings *settings);
const char *handleTextBuffer(char *source);

// Module Compiler
ASTNode *compileModuleFileToProgramNode(const char *filePath, const char *outputPath, CompilerState *state, CryoGlobalSymbolTable *globalTable);
SymbolTable *compileToReapSymbols(const char *filePath, const char *outputPath, CompilerState *state, Arena *arena, CryoGlobalSymbolTable *globalTable);
int processNodeToIRObject(ASTNode *node, CompilerState *state, const char *outputPath, CryoLinker *cLinker);

// LSP Compiler
int lspCompiler(const char *filePath, CompilerSettings *settings);

// Project Compiler
int compileProject(CompilerSettings *settings);

// C++ Accessable Functions
#ifdef __cplusplus
extern "C"
{
    typedef struct CompilerSettings CompilerSettings;
    typedef struct CompilerState CompilerState;
    typedef struct ASTNode ASTNode;

    int compileImportFileCXX(const char *filePath, CompilerSettings *settings);
}
#endif

#endif // GLOBAL_COMPILER_H
