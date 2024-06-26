#include "cpp/codegen.h"



void codegen(ASTNode* root) {
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder(context);
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("CryoModule", context);

    generateCode(root, builder, *module);

    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "Error: LLVM module verification failed\n";
    } else {
        // Output the generated LLVM IR code to a .ll file
        std::error_code EC;
        llvm::raw_fd_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);

        if (EC) {
            std::cerr << "Could not open file: " << EC.message() << std::endl;
        } else {
            std::cout << "\n>===------- LLVM IR Code -------===<\n" << std::endl;
            module->print(dest, nullptr);
            module->print(llvm::outs(), nullptr);
            std::cout << "\n>===------- End IR Code ------===<\n" << std::endl;
            std::cout << "LLVM IR written to output.ll" << std::endl;
        }
    }
}

void generateCode(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    if (!node) return;
    std::cout << "[CPP] Generating code for node: " << logNode(node) << std::endl;
    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            generateProgram(node, builder, module);
            break;
        case CryoNodeType::NODE_STATEMENT:
            generateStatement(node, builder, module);
            break;
        case CryoNodeType::NODE_EXPRESSION:
            generateExpression(node, builder, module);
            break;
        case CryoNodeType::NODE_VAR_DECLARATION:
            generateVarDeclaration(node, builder, module);
            break;
        // Add cases for other node types
        default:
            std::cerr << "[CPP] Unknown AST node type\n";
            break;
    }
}

void generateProgram(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for program\n";
    ASTNode* stmt = node->firstChild;
    while (stmt) {
        generateCode(stmt, builder, module);
        stmt = stmt->nextSibling;
    }
}

void generateStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for statement\n";
    switch (node->type) {
        case CryoNodeType::NODE_EXPRESSION_STATEMENT:
            generateExpression(node->data.stmt.stmt, builder, module);
            break;
        // Add cases for other statement types
        default:
            std::cerr << "[CPP] Unknown statement type\n";
            break;
    }
}

llvm::Value* generateExpression(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for expression\n";
    switch (node->type) {
        case CryoNodeType::NODE_LITERAL_EXPR:
            return builder.getInt32(node->data.literalExpression.value);
        case CryoNodeType::NODE_BINARY_EXPR:
            return generateBinaryOperation(node, builder, module);
        // Add cases for other expression types
        default:
            std::cerr << "[CPP] Unknown expression type\n";
            return nullptr;
    }
}

void generateVarDeclaration(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for variable declaration\n";
    llvm::Type* int32Type = llvm::Type::getInt32Ty(module.getContext());
    llvm::Value* initializer = builder.getInt32(node->data.varDecl.initializer->data.literalExpression.value);
    llvm::AllocaInst* alloca = builder.CreateAlloca(int32Type, nullptr, node->data.varDecl.name);

    if (!initializer) {
        std::cerr << "[CPP] Error generating variable initializer\n";
        return;
    }

    builder.CreateStore(initializer, alloca);
}

llvm::Value* generateBinaryOperation(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for binary operation\n";
    llvm::Value* left = generateExpression(node->data.bin_op.left, builder, module);
    llvm::Value* right = generateExpression(node->data.bin_op.right, builder, module);

    if (!left || !right) {
        std::cerr << "[CPP] Error generating binary operation\n";
        return nullptr;
    }

    switch (node->data.bin_op.op) {
        case CryoTokenType::TOKEN_PLUS:
            return builder.CreateAdd(left, right);
        case CryoTokenType::TOKEN_MINUS:
            return builder.CreateSub(left, right);
        case CryoTokenType::TOKEN_STAR:
            return builder.CreateMul(left, right);
        case CryoTokenType::TOKEN_SLASH:
            return builder.CreateSDiv(left, right);
        // Add cases for other binary operators
        default:
            std::cerr << "[CPP] Unknown binary operator\n";
            return nullptr;
    }
}

