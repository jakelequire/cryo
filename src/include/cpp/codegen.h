#ifndef CODEGEN_H
#define CODEGEN_H
#include <iostream>
#include <fstream>
#include <string>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include <llvm/Support/raw_ostream.h>

#include "cpp/cppLogger.h"

using namespace llvm;

extern "C" {
    #include "compiler/ast.h"
}

void generateCode(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateProgram(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateVarDeclaration(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);

llvm::Value* generateExpression(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
llvm::Value* generateBinaryOperation(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);

void codegen(ASTNode* root);




#endif // CODEGEN_H