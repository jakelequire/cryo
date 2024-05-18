#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "include/ast.h"


void analyze_function(ASTNode* node);
void analyze_return(ASTNode* node);


int analyze(ASTNode* node);

#endif // SEMANTICS_H


