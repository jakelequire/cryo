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
#ifndef SEMANTICS_H
#define SEMANTICS_H
/*------ <includes> ------*/
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "compiler/ast.h"
#include "compiler/token.h"
#include "compiler/symtable.h"
/*---------<end>---------*/
//

/*-----<function_prototypes>-----*/

// Symbol Management

void checkVariable(ASTNode *node, CryoSymbolTable *table, Arena *arena);
void checkAssignment(ASTNode *node, CryoSymbolTable *table, Arena *arena);
void checkFunctionCall(ASTNode *node, CryoSymbolTable *table, Arena *arena);
void traverseAST(ASTNode *node, CryoSymbolTable *table, Arena *arena);
CryoDataType getExpressionType(ASTNode *node, Arena *arena);
/*-----<end_prototypes>-----*/

#endif // SEMANTICS_H