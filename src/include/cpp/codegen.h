#ifndef CODEGEN_H
#define CODEGEN_H
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

extern "C" {
    #include "compiler/ast.h"
}


void generateCode(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);

void generateFunction(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateProgram(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);

void codegen(ASTNode* root);




#endif // CODEGEN_H