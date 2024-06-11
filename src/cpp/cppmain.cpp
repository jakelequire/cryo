#include <iostream>
#include "cppASTNode.h"
#include "cppCodeGen.h"
using namespace std;

int main() {
    std::cout << "[cppmain.cpp]" << std::endl;

    unique_ptr<ASTNode> root = make_unique<BinaryOp>(
        new IntLiteral(1), new IntLiteral(2), '+');

    root->print();

    std::cout << "Hello, World!" << std::endl;

    CodeGen codegen;
    llvm::Value* result = codegen.generateCode(root.get());

    return 0;
}

