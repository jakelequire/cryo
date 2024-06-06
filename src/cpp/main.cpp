#include "cppASTNode.h"
#include "CodeGen.h"
#include <llvm/Support/raw_ostream.h>


int main() {
    // Create some AST Nodes
    auto lhs = std::make_unique<IntLiteral>(10);
    auto rhs = std::make_unique<IntLiteral>(20);
    auto root = std::make_unique<BinaryOp>('+', std::move(lhs), std::move(rhs));

    // Print the AST
    root->print();
    std::cout << std::endl;

    // Generate LLVM IR
    CodeGen codgen;
    llvm::Value* result = codegen.generateCode(root.get());

    // Print the IR
    codegen.getModule()->print(llvm::errs(), nullptr);

    return 0;
}
