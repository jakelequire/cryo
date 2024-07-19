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
#ifndef CLI_COMPILER_H
#define CLI_COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/ast.h"
#include "compiler/symtable.h"
#include "compiler/error.h"
#include "utils/fs.h"

#ifdef __cplusplus
extern "C" {
#endif

void generateCodeWrapper(ASTNode* node);

#ifdef __cplusplus
}
#endif


int cryoCompiler(const char* source);


#endif // CLI_COMPILER_H
