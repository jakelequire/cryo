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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "frontend/dataTypes.h"
#include "frontend/AST.h"
#include "frontend/tokens.h"
#include "tools/utils/cWrappers.h"
#include "tools/utils/cTypes.h"

typedef struct SemanticAnalyzer
{
    // --------------------------
    // Properties

    bool passedAnalysis;

    // --------------------------
    // Methods

    // Main method to analyze the AST, the AST Node passed here should be the root program node.
    _NEW_METHOD(void, analyzeAST, struct SemanticAnalyzer *self, ASTNode *root);

    // Specific methods for analyzing different types of nodes

    // Analyze a program node
    _NEW_METHOD(void, analyzeProgramNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze a function declaration node
    _NEW_METHOD(void, analyzeFunctionDeclarationNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze a variable declaration node
    _NEW_METHOD(void, analyzeVariableDeclarationNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze a struct declaration node
    _NEW_METHOD(void, analyzeStructDeclarationNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze a class declaration node
    _NEW_METHOD(void, analyzeClassDeclarationNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze a method declaration node
    _NEW_METHOD(void, analyzeMethodDeclarationNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze a property declaration node
    _NEW_METHOD(void, analyzePropertyDeclarationNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze literal nodes
    _NEW_METHOD(void, analyzeLiteralNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze binary operation nodes
    _NEW_METHOD(void, analyzeBinaryOperationNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze unary operation nodes
    _NEW_METHOD(void, analyzeUnaryOperationNode, struct SemanticAnalyzer *self, ASTNode *node);
    // Analyze assignment nodes
    _NEW_METHOD(void, analyzeAssignmentNode, struct SemanticAnalyzer *self, ASTNode *node);

} SemanticAnalyzer;

//===================================================================
// Function Prototypes
//===================================================================

/// @brief Initialize the semantic analysis process.
/// @param root The root node of the AST.
/// @return int Returns 0 if the analysis was successful, otherwise returns 1.
int initSemanticAnalysis(ASTNode *root);

SemanticAnalyzer *initSemanticAnalyzer(void);

void analyzeAST(struct SemanticAnalyzer *self, ASTNode *root);
void analyzeProgramNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzeFunctionDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzeVariableDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzeStructDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzeClassDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzeMethodDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzePropertyDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzeLiteralNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzeBinaryOperationNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzeUnaryOperationNode(struct SemanticAnalyzer *self, ASTNode *node);
void analyzeAssignmentNode(struct SemanticAnalyzer *self, ASTNode *node);

#endif // SEMANTICS_H
