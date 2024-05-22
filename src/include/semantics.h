#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "include/ast.h"
#include <stdbool.h>


void analyze_function(ASTNode* node);
void analyze_return(ASTNode* node);


bool analyze(ASTNode* root);

#endif // SEMANTICS_H


