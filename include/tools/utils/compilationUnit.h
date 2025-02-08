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
#ifndef COMPILATION_UNIT_H
#define COMPILATION_UNIT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "frontend/AST.h"
#include "tools/utils/cTypes.h"
#include "tools/utils/cWrappers.h"

typedef struct ASTNode ASTNode;

typedef enum
{
    COF_UNKNOWN,
    COF_BUILD,   // `/build`
    COF_OUT,     // `/build/out`
    COF_DEPS,    // `/build/out/deps`
    COF_RUNTIME, // `/build/out/runtime`
} CompilationOutputFolder;

typedef enum
{
    CU_TYPE_NONE,
} CompilationUnitType;

typedef struct CompilationFile
{
    char *srcFileName;
    char *srcFilePath;
    char *srcFullPath;

    char *outFileName;
    char *outFilePath;
    char *outFullPath;
} CompilationFile;

typedef struct CompilationUnit
{
    CompilationUnitType type;
    ASTNode *AST;
    CompilationFile *file;
} CompilationUnit;

#endif // COMPILATION_UNIT_H
