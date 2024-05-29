#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "include/ast.h"
#include <stdbool.h>


void analyze_function(ASTNode* node);
void analyze_return(ASTNode* node);
bool analyzeNode(ASTNode* node);
bool analyzeProgram(ASTNode* node);
bool analyzeFunction(ASTNode* node);
bool analyzeVariableDeclaration(ASTNode* node);
bool analyzeIfStatement(ASTNode* node);
bool analyzeWhileStatement(ASTNode* node);
bool analyzeForStatement(ASTNode* node);
bool analyzeBinaryExpression(ASTNode* node);
bool analyzeUnaryExpression(ASTNode* node);
bool analyzeReturn(ASTNode* node);
bool analyzeBlock(ASTNode* node);

bool analyze(ASTNode* root);

#endif // SEMANTICS_H


