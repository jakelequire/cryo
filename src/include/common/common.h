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
#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <stdlib.h>

#include "compiler/ast.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/symtable.h"
#include "utils/utility.h"
#include "utils/arena.h"
#include "utils/fs.h"

/*================================*/
// Macros

/// @brief A macro to break the program at a specific point for debugging.
#define DEBUG_BREAKPOINT                                                                      \
    printf("\n#========================================================================#\n"); \
    printf("\n<!> Debug Breakpoint! Exiting...");                                             \
    printf("\n<!> Line: %i, Function: %s", __LINE__, __FUNCTION__);                           \
    printf("\n<!> File: %s\n", __FILE__);                                                     \
    printf("\n#========================================================================#\n"); \
    exit(0)

#define CONDITION_FAILED                                                                      \
    printf("\n#========================================================================#\n"); \
    printf("\n<!> Condition Failed! Exiting...");                                             \
    printf("\n<!> Line: %i, Function: %s", __LINE__, __FUNCTION__);                           \
    printf("\n<!> File: %s\n", __FILE__);                                                     \
    printf("\n#========================================================================#\n"); \
    exit(1)

/*================================*/
// Function Prototypes

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct ASTNode ASTNode;

    int globalCompiler(const char *source);
    ASTNode *getProgramNode(const char *source);

#ifdef __cplusplus
}
#endif

typedef struct ASTNode ASTNode;

int globalCompiler(const char *source);
ASTNode *getProgramNode(const char *source);

#endif // COMMON_H
