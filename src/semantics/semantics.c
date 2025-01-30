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
#include "semantics/semantics.h"
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

    analyzer->analyzeAST(analyzer, root);

    bool passedAnalysis = analyzer->passedAnalysis;
    if (!passedAnalysis)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Failed to pass semantic analysis");
        analyzer->printAnalysisReport(analyzer);
        return 1;
    }

    analyzer->printAnalysisReport(analyzer);
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

    // --------------------------
    // Properties

    analyzer->passedAnalysis = false;
    analyzer->errorMessage = NULL;

    analyzer->nodesAnalyzed = 0;
    analyzer->nodesPassed = 0;
    analyzer->nodesFailed = 0;

    // --------------------------
    // Methods

    analyzer->treeAnalysis = treeAnalysis;
    analyzer->reportSemanticError = reportSemanticError;
    analyzer->unimplementedAnalysis = unimplementedAnalysis;
    analyzer->incrementNodesAnalyzed = incrementNodesAnalyzed;
    analyzer->printAnalysisReport = printAnalysisReport;
    analyzer->setPassedAnalysis = setPassedAnalysis;

    // Specific Analysis Methods
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

void reportSemanticError(struct SemanticAnalyzer *self, const char *message)
{
    __STACK_FRAME__
    logMessage(LMI, "ERROR", "Semantic Analysis", message);
}

void unimplementedAnalysis(struct SemanticAnalyzer *self)
{
    __STACK_FRAME__
    logMessage(LMI, "WARN", "Semantic Analysis", "Unimplemented node analysis");
}

void incrementNodesAnalyzed(struct SemanticAnalyzer *self, enum NodeAnalysisStatus status)
{
    __STACK_FRAME__
    self->nodesAnalyzed++;
    if (status == NAS_PASSED)
    {
        self->nodesPassed++;
    }
    else if (status == NAS_FAILED)
    {
        self->nodesFailed++;
    }
}

void setPassedAnalysis(struct SemanticAnalyzer *self, bool passed)
{
    __STACK_FRAME__
    self->passedAnalysis = passed;
}

void printAnalysisReport(struct SemanticAnalyzer *self)
{
    __STACK_FRAME__
    printf("\n");
    printf("+------------------- Semantic Analysis Report -------------------+\n");
    fprintf(stdout, "Nodes Analyzed: %zu\n", self->nodesAnalyzed);
    fprintf(stdout, "Nodes Passed: %zu\n", self->nodesPassed);
    fprintf(stdout, "Nodes Failed: %zu\n", self->nodesFailed);
    fprintf(stdout, "Analysis Passed: %s\n", self->passedAnalysis ? "true" : "false");
    printf("+---------------------------------------------------------------+\n");
    printf("\n");
}

// ======================================================================================== //
//                              Semantic Analysis Functions                                 //
// ======================================================================================== //

void analyzeAST(struct SemanticAnalyzer *self, ASTNode *root)
{
    __STACK_FRAME__
    if (!root)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Root node is NULL");
        return;
    }

    self->analyzeProgramNode(self, root);
}

void analyzeProgramNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
    if (!node)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Program node is NULL");
        return;
    }

    logMessage(LMI, "INFO", "Semantic Analysis", "Analyzing program node...");
    for (size_t i = 0; i < node->data.program->statementCount; i++)
    {
        ASTNode *statement = node->data.program->statements[i];
        self->treeAnalysis(self, statement);
    }

    self->setPassedAnalysis(self, true);
    return;
}

void analyzeFunctionDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
}

void analyzeVariableDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
    if (!node)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Variable declaration node is NULL");
        self->reportSemanticError(self, "Variable declaration node is NULL");
        self->incrementNodesAnalyzed(self, NAS_FAILED);
        return;
    }
    if (node->metaData->type != NODE_VAR_DECLARATION)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Node is not a variable declaration node");
        self->reportSemanticError(self, "Node is not a variable declaration node");
        self->incrementNodesAnalyzed(self, NAS_FAILED);
        return;
    }

    logMessage(LMI, "INFO", "Semantic Analysis", "Analyzing variable declaration node...");

    DataType *varType = node->data.varDecl->type;               // The declared type of the variable
    ASTNode *varInitilizer = node->data.varDecl->initializer;   // The initializer for the variable
    DataType *initType = getDataTypeFromASTNode(varInitilizer); // The type of the initializer
    if (!varType || !varInitilizer || !initType)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Variable declaration node is missing type or initializer");
        self->reportSemanticError(self, "Variable declaration node is missing type or initializer");
        self->incrementNodesAnalyzed(self, NAS_FAILED);
        return;
    }

    // Check if the initializer type matches the declared type
    bool typesMatch = isSameType(varType, initType);
    if (!typesMatch)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Variable type does not match initializer type");
        self->reportSemanticError(self, "Variable type does not match initializer type");
        self->incrementNodesAnalyzed(self, NAS_FAILED);
        return;
    }

    self->incrementNodesAnalyzed(self, NAS_PASSED);
    return;
}

void analyzeStructDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
}

void analyzeClassDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
}

void analyzeMethodDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
}

void analyzePropertyDeclarationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
}

void analyzeLiteralNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
}

void analyzeBinaryOperationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
}

void analyzeUnaryOperationNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
}

void analyzeAssignmentNode(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
}
