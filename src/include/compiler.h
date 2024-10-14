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

#include "compiler/ast.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/token.h"
#include "compiler/symtable.h"
#include "compiler/typedefs.h"
#include "utils/fs.h"
#include "common/common.h"
#include "settings.h"

typedef struct CompilerSettings CompilerSettings;
typedef struct CompilerState CompilerState;

// C++ headers
#ifdef __cplusplus
extern "C"
{
#endif

    int generateCodeWrapper(ASTNode *node, CompilerState *state);

#ifdef __cplusplus
}
#endif

int cryoCompiler(const char *filePath, CompilerSettings *settings);
ASTNode *compileForProgramNode(const char *filePath);
int compileImportFile(const char *filePath, CompilerSettings *settings);

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
