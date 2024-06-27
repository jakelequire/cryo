#ifndef CPP_LOGGER_H
#define CPP_LOGGER_H
#include <iostream>
#include <string>
#include <fstream>

#include "compiler/ast.h"


void printIndentation(int level);
void logNode(ASTNode* node, int indentLevel);




#endif // CPP_LOGGER_H