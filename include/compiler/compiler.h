/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "dataTypes/dataTypes.h"
#include "tools/utils/fs.h"
#include "common/common.h"
#include "settings/compilerSettings.h"
#include "tools/utils/cTypes.h"
#include "semantics/semantics.h"
#include "linker/compilationUnit.h"

typedef struct CompilerSettings CompilerSettings;
typedef struct CompilerState CompilerState;
typedef struct ASTNode ASTNode;
typedef struct Bootstrapper Bootstrapper;
typedef struct SymbolTable SymbolTable;
typedef struct TypesTable TypesTable;
typedef struct CompilationUnit_t CompilationUnit;
typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;

typedef struct CCTXPragma_t
{
    bool internal;
} CCTXPragma;

typedef struct CCTXFile_t
{
    char *fileName;
    char *filePath;
} CCTXFile;

typedef struct CompilerContext_t
{
    CCTXPragma *pragma;
    CCTXFile *file;
} CompilerContext;

CompilerContext *initCompilerContext(void);
CCTXPragma *initCCTXPragma(void);
CCTXFile *initCCTXFile(void);

extern CompilerContext *compilerCtx;

// C++ headers
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct CryoLinker_t *CryoLinker;

    int generateIRFromAST(CompilationUnit *unit,
                          CompilerState *state, CryoLinker *cLinker);

    int generateIRFromAST(CompilationUnit *unit, CompilerState *state,
                          CryoLinker *cLinker);

#ifdef __cplusplus
}
#endif

extern int generateIRFromAST(CompilationUnit *unit, CompilerState *state,
                             CryoLinker *cLinker);

// -------------------------------------------------------------
// NEW Compiler Functions

#define INIT_SUBSYSTEMS(buildDir, compilerRootPath, fileName, source, settings, linker, arena, lexer, state) \
    do                                                                                                       \
    {                                                                                                        \
        linker = CreateCryoLinker(buildDir, compilerRootPath);                                               \
        arena = createArena(ARENA_SIZE, ALIGNMENT);                                                          \
        lexer = (Lexer){};                                                                                   \
        state = initCompilerState(arena, &lexer, fileName);                                                  \
        initLexer(&lexer, source, fileName, state);                                                          \
        state->settings = settings;                                                                          \
    } while (0)

int cryoCompile(CompilerSettings *settings);

int exe_single_file_build(CompilerSettings *settings);
int exe_project_build(CompilerSettings *settings);
int exe_lsp_build(CompilerSettings *settings);
int exe_source_build(CompilerSettings *settings);

// Import AST Nodes
ASTNode *compileForASTNode(const char *filePath, CompilerState *state);

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
