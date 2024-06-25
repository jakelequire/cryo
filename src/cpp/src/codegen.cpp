#include "cpp/codegen.h"
#include <iostream>


void generateCode(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            std::cout << "Generating code for program\n";
            generateProgram(node, builder, module);
            break;
        case CryoNodeType::NODE_STATEMENT:
            std::cout << "Generating code for statement\n";
            break;
        case CryoNodeType::NODE_EXPRESSION:
            std::cout << "Generating code for expression\n";
            break;
        case CryoNodeType::NODE_LITERAL_EXPR:
            std::cout << "Generating code for literal\n";
            break;
        case CryoNodeType::NODE_FUNCTION_DECLARATION:
            generateFunction(node, builder, module);
            break;
        case CryoNodeType::NODE_VAR_DECLARATION:
            std::cout << "Generating code for variable declaration\n";
        // Add cases for other node types as needed
        default:
            std::cerr << "Unknown AST node type\n";
            break;
    }
}


void generateProgram(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    // Generate code for the program body (implement as needed)
    // For example, iterate over the statements in the body and generate IR for each
    ASTNode* stmt = node->firstChild;
    while (stmt) {
        generateCode(stmt, builder, module);
        stmt = stmt->nextSibling;
    }
}

void generateFunction(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    llvm::FunctionType* funcType = llvm::FunctionType::get(builder.getVoidTy(), false);
    /*
        static Function *Create(
                            FunctionType *Ty,           // Function type
                            LinkageTypes Linkage,       // Linkage type
                            unsigned AddrSpace,         // Default is 0
                            const Twine &N = "",        // Name of the function
                            Module *M = nullptr         // Module to insert the function to
                        ) {
        return new Function(Ty, Linkage, AddrSpace, N, M);
        }
    */
    llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, 0, node->data.functionDecl.name, &module);
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
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("CryoModule", context);

    generateCode(root, builder, *module);

    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "Error: LLVM module verification failed\n";
    } else {
        // Print the generated LLVM IR code to a .ll file
        module->print(llvm::outs(), nullptr);
    }
}
