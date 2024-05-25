#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include "ir.h"
#include "token.h"


void init_codegen();
LLVMValueRef codegen_expr(ASTNode* node);
void finalize_codegen();
void codegen_function(ASTNode* node);

#endif // CODEGEN_H