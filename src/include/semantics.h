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

