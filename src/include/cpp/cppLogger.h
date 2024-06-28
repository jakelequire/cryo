#ifndef CPP_LOGGER_H
#define CPP_LOGGER_H
#include <iostream>
#include <string>
#include <fstream>

#include "compiler/ast.h"


void printIndentation(int level);
void logASTNode(ASTNode* node, int indentLevel);
void logCryoDataType(CryoDataType dataType);

char* dataTypeToString(CryoDataType dataType);
char* visibilityToString(CryoVisibilityType visibility);
char* nodeTypeToString(CryoNodeType type);
#endif // CPP_LOGGER_H