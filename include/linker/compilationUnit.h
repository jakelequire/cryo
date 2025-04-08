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
#include <stdbool.h>
#include <string.h>

#include "frontend/AST.h"

typedef enum CompilationUnitType_t
{
    CRYO_MODULE,
    CRYO_MAIN,
    CRYO_RUNTIME,
    CRYO_DEPENDENCY,
    CRYO_STDLIB,
} CompilationUnitType;

char *compilationUnitTypeToString(CompilationUnitType type);

typedef struct CompilationUnitDir_t
{
    CompilationUnitType type;
    // The input file. (e.g. `path/to/file.cryo`)
    const char *src_fileName;     // `fileName` is the name of the file without the extension
    const char *src_fullFileName; // `fullFileName` is the full name of the file with the extension
    const char *src_fileDir;      // `fileDir` `path/to` is the directory of the file
    const char *src_filePath;     // `filePath` `path/to/file.*` is the full path to the file
    const char *src_fileExt;      // `fileExt` `.*` is the file extension

    const char *out_fileName;     // `fileName` is the name of the file without the extension
    const char *out_fullFileName; // `fullFileName` is the full name of the file with the extension
    const char *out_fileDir;      // `fileDir`  `path/to` is the directory of the file
    const char *out_filePath;     // `filePath` `path/to/file.*` is the full path to the file
    const char *out_fileExt;      // `fileExt`  `.*` is the file extension

    void (*print)(struct CompilationUnitDir_t dir);
} CompilationUnitDir;

void CompilationUnitDir_print(CompilationUnitDir dir);

typedef struct CompilationUnit_t
{
    CompilationUnitType type;
    CompilationUnitDir dir;
    ASTNode *ast;
    bool isASTSet;
    bool isVerified;
    bool hasCompiledToIR;

    void (*setAST)(struct CompilationUnit_t *unit, ASTNode *ast);
    int (*verify)(struct CompilationUnit_t *unit);
} CompilationUnit;

// ------------------------
// Compilation Unit Method Prototypes

CompilationUnit *CompilationUnit_Create(CompilationUnitType type, CompilationUnitDir dir);
void CompilationUnit_SetAST(CompilationUnit *unit, ASTNode *ast);
int CompilationUnit_Verify(CompilationUnit *self);

CompilationUnitDir createCompilationUnitDir(const char *inputFile, const char *buildDir, CompilationUnitType type);
CompilationUnit *createNewCompilationUnit(ASTNode *root, CompilationUnitDir dir);
CompilationUnitDir createCompilerCompilationUnitDir(
    const char *inputFile, const char *buildDir, const char *compilerDir, CompilationUnitType type);
// ------------------------
// Macros

#define CreateCompilationUnitDir(inputFile, buildDir, type) createCompilationUnitDir(inputFile, buildDir, type)
#define CreateCompilationUnit(type, dir) CompilationUnit_Create(type, dir)

#endif // COMPILATION_UNIT_H
