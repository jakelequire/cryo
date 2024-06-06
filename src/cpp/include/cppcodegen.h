#ifndef CPP_CODEGEN_H
#define CPP_CODEGEN_H

#include "cppASTNode.h"
#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

class CodeGen {
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;

    public:
        CodeGen();
        llvm::Module* getModule() { return module.get(); };
        llvm::Value* generateCode(const ASTNode* node);
    private:
        llvm::Value* generateCode(const IntLiteral* node);
        llvm::Value* generateCode(const BinaryOp* node);
};

#endif