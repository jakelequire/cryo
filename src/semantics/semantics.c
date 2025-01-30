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
#include "frontend/semantics.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

int initSemanticAnalysis(ASTNode *root)
{
    __STACK_FRAME__
    if (!root)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Root node is NULL");
        return 1;
    }

    SemanticAnalyzer *analyzer = initSemanticAnalyzer();
    if (!analyzer)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Failed to initialize semantic analyzer");
        return 1;
    }

    analyzeAST(analyzer, root);

    return 0;
}

SemanticAnalyzer *initSemanticAnalyzer(void)
{
    __STACK_FRAME__
    SemanticAnalyzer *analyzer = (SemanticAnalyzer *)malloc(sizeof(SemanticAnalyzer));
    if (!analyzer)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Failed to allocate memory for semantic analyzer");
        return NULL;
    }

    analyzer->passedAnalysis = false;

    analyzer->analyzeAST = analyzeAST;
    analyzer->analyzeProgramNode = analyzeProgramNode;
    analyzer->analyzeFunctionDeclarationNode = analyzeFunctionDeclarationNode;
    analyzer->analyzeVariableDeclarationNode = analyzeVariableDeclarationNode;
    analyzer->analyzeStructDeclarationNode = analyzeStructDeclarationNode;
    analyzer->analyzeClassDeclarationNode = analyzeClassDeclarationNode;
    analyzer->analyzeMethodDeclarationNode = analyzeMethodDeclarationNode;
    analyzer->analyzePropertyDeclarationNode = analyzePropertyDeclarationNode;
    analyzer->analyzeLiteralNode = analyzeLiteralNode;
    analyzer->analyzeBinaryOperationNode = analyzeBinaryOperationNode;
    analyzer->analyzeUnaryOperationNode = analyzeUnaryOperationNode;
    analyzer->analyzeAssignmentNode = analyzeAssignmentNode;

    return analyzer;
}

void analyzeAST(struct SemanticAnalyzer *self, ASTNode *root)
{
}
void analyzeProgramNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzeFunctionDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzeVariableDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzeStructDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzeClassDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzeMethodDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzePropertyDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzeLiteralNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzeBinaryOperationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzeUnaryOperationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
void analyzeAssignmentNode(struct SemanticAnalyzer *self, ASTNode *node)
{
}
