#include "cpp/codegen.h"
#include <iostream>


void generateCode(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    switch (node->type) {
        case CryoNodeType::NODE_FUNCTION_DECLARATION:
            generateFunction(node, builder, module);
            break;
        // Add cases for other node types as needed
        default:
            std::cerr << "Unknown AST node type\n";
            break;
    }
}

void generateFunction(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    llvm::FunctionType* funcType = llvm::FunctionType::get(builder.getVoidTy(), false);
    llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->data.functionDecl.name, module);

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(module.getContext(), "entry", func);
    builder.SetInsertPoint(entry);

    // Generate code for the function body (implement as needed)
    // For example, iterate over the statements in the body and generate IR for each
    ASTNode* stmt = node->data.functionDecl.body->firstChild;
    while (stmt) {
        generateCode(stmt, builder, module);
        stmt = stmt->nextSibling;
    }

    builder.CreateRetVoid();
}

void codegen(ASTNode* root) {
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder(context);
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("ASTModule", context);

    generateCode(root, builder, *module);

    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "Error: LLVM module verification failed\n";
    } else {
        module->print(llvm::outs(), nullptr);
    }
}
