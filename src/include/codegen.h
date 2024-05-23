#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include "ir.h"

// LLVMModuleRef generate_code();
//LLVMModuleRef generate_llvm_module(Node *root);
int generate_code();
void generate_simple_function();

#endif // CODEGEN_H