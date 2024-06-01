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
#include "ast.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
/*---------<end>---------*/

/*-----<function_prototypes>-----*/
bool analyze(ASTNode* root);
bool analyzeVariableDeclaration(ASTNode* node);
bool analyzeFunction(ASTNode* node);
bool analyzeIfStatement(ASTNode* node);
bool analyzeWhileStatement(ASTNode* node);
bool analyzeForStatement(ASTNode* node);
bool analyzeBinaryExpression(ASTNode* node);
bool analyzeUnaryExpression(ASTNode* node);
bool analyzeReturn(ASTNode* node);
bool analyzeBlock(ASTNode* node);
bool analyzeProgram(ASTNode* node);
//Entry Point
bool analyzeNode(ASTNode* node);
/*-----<end_prototypes>-----*/

#endif // SEMANTICS_H

