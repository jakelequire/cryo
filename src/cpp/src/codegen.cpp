#include "cpp/codegen.h"

/*
To compile the generated LLVM IR code to an object file, run the following commands:
llvm-as ./output.ll -o ./output.bc
llc -filetype=obj ./output.bc -o ./output.o
*/


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
    std::cout << "[CPP] Generating code for AST node\n" << std::endl;
    if (!node) {
        std::cerr << "[CPP] Error generating code: AST node is null\n";
        return;
    };
    std::cout << "[CPP] Generating code for node: " << std::endl;
    logNode(node);

    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            std::cout << "[CPP] Starting code generation for program...\n" << std::endl;
            generateProgram(node, builder, module);
            break;
        case CryoNodeType::NODE_STATEMENT:
            std::cout << "[CPP] Starting code generation for <statement>\n" << std::endl;
            generateStatement(node, builder, module);
            break;
        case CryoNodeType::NODE_EXPRESSION:
            std::cout << "[CPP] Starting code generation for <expression>\n" << std::endl;
            generateExpression(node, builder, module);
            break;
        case CryoNodeType::NODE_VAR_DECLARATION:
            std::cout << "[CPP] Starting code generation for <var declaration>\n" << std::endl;
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
    ASTNode* stmt = *node->data.program.statements;
    if(!stmt) {
        std::cerr << "[CPP] Error generating code for program: No statements found\n";
        return;
    }
    std::cout << "[CPP] Generating code for program statements\n";
    while (stmt) {
        std::cout << "[CPP] Starting process for generating code...\n";
        generateCode(stmt, builder, module);
        std::cout << "[CPP] Moving to next statement\n" << std::endl;
        stmt = stmt->nextSibling;
    }
}


void generateStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for statement\n";
    switch (node->type) {
        case CryoNodeType::NODE_EXPRESSION_STATEMENT:
            std::cout << "[CPP] Generating code for expression statement!\n" << std::endl;
            generateExpression(node->data.stmt.stmt, builder, module);
            break;
        case CryoNodeType::NODE_VAR_DECLARATION:
            std::cout << "[CPP] Generating code for variable declaration!\n" << std::endl;
            generateVarDeclaration(node, builder, module);
            break;
        default:
            std::cerr << "[CPP] Unknown statement type\n";
            break;
    }
}


void generateVarDeclaration(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for variable declaration\n";
    llvm::Type* int32Type = llvm::Type::getInt32Ty(module.getContext());
    llvm::Constant* initializer = llvm::ConstantInt::get(int32Type, node->data.varDecl.initializer->data.literalExpression.value);

    // Register the variable in the module's global scope
    llvm::GlobalVariable* gVar = new llvm::GlobalVariable(
        module,
        int32Type,
        false,
        llvm::GlobalValue::ExternalLinkage,
        initializer,
        node->data.varDecl.name
    );

    std::cout << "[CPP] Variable registered in module's global scope\n" << std::endl;
    std::cout << "[CPP] Variable " << node->data.varDecl.name << " declared with initializer " << node->data.varDecl.initializer->data.literalExpression.value << std::endl;
}

llvm::Value* generateExpression(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for expression\n";
    switch (node->type) {
        case CryoNodeType::NODE_LITERAL_EXPR:
            std::cout << "[CPP] Generating code for literal expression\n";
            return builder.getInt32(node->data.literalExpression.value);
        case CryoNodeType::NODE_BINARY_EXPR:
            std::cout << "[CPP] Generating code for binary operation\n";
            return generateBinaryOperation(node, builder, module);
        // Add cases for other expression types
        default:
            std::cerr << "[CPP] Unknown expression type\n";
            return nullptr;
    }
}


llvm::Value* generateBinaryOperation(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::cout << "[CPP] Generating code for binary operation\n";
    llvm::Value* left = generateExpression(node->data.bin_op.left, builder, module);
    llvm::Value* right = generateExpression(node->data.bin_op.right, builder, module);

    if (!left || !right) {
        std::cerr << "[CPP] Error generating binary operation\n";
        return nullptr;
    }

    std::cout << "[CPP] Binary operation operands generated\n";

    switch (node->data.bin_op.op) {
        case CryoTokenType::TOKEN_PLUS:
            std::cout << "[CPP] Generating code for binary operation: ADD\n";
            return builder.CreateAdd(left, right);
        case CryoTokenType::TOKEN_MINUS:
            std::cout << "[CPP] Generating code for binary operation: SUB\n";
            return builder.CreateSub(left, right);
        case CryoTokenType::TOKEN_STAR:
            std::cout << "[CPP] Generating code for binary operation: MUL\n";
            return builder.CreateMul(left, right);
        case CryoTokenType::TOKEN_SLASH:
            std::cout << "[CPP] Generating code for binary operation: DIV\n";
            return builder.CreateSDiv(left, right);
        // Add cases for other binary operators
        default:
            std::cerr << "[CPP] Unknown binary operator\n";
            return nullptr;
    }
}

