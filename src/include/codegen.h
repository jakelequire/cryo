#ifndef CODEGEN_H
#define CODEGEN_H
/*------ <includes> ------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*-------- <LLVM> --------*/
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
/*---<custom_includes>---*/
#include "ast.h"
#include "ir.h"
#include "token.h"
/*---------<end>---------*/



/*-----<function_prototypes>-----*/
void initializeLLVM();
void finalizeLLVM();
LLVMValueRef generateCodeFromAST(ASTNode* node);
LLVMValueRef generateCode(ASTNode* node);
LLVMValueRef generateFunction(ASTNode* node);
LLVMValueRef generateVariableDeclaration(ASTNode* node);
LLVMValueRef generateBinaryExpression(ASTNode* node);
LLVMValueRef generateUnaryExpression(ASTNode* node);
LLVMValueRef generateReturnStatement(ASTNode* node);
LLVMValueRef generateBlock(ASTNode* node);
/*-----<end_prototypes>-----*/

#endif // CODEGEN_H
