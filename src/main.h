#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/ast.h"
#include "include/semantics.h"
#include "include/ir.h"
#include "include/codegen.h"
#include "include/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#ifdef __cplusplus
}
#endif

//char* readFile(const char* path);
int main(int argc, char* argv[]);


#endif // MAIN_H