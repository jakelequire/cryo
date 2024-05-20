#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "include/ast.h"


void analyze_function(ASTNode* node);
void analyze_return(ASTNode* node);


bool analyze(Node *root);

#endif // SEMANTICS_H


