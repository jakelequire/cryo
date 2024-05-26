#ifndef MAIN_H
#define MAIN_H

#include "include/lexer.h"
#include "include/parser.h"
#include "include/ast.h"
#include "include/semantics.h"
#include "include/ir.h"
#include "include/codegen.h"

char* readFile(const char* path);
int main(int argc, char* argv[]);


#endif // MAIN_H