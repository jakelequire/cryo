#include "cppCodeGen.h"

CodeGen::CodeGen() : builder(context) {
    module = std::make_unique<llvm::Module>("main", context);
}

llvm::Value* CodeGen::generateCode(const ASTNode* node) {
    if(const IntLiteral* intLiteral = dynamic_cast<const IntLteral*>(node)) {
        return generateCode(intLiteral);
    }
    else if (const BinaryOp* binaryOp = dynamic_cast<const BinaryOp*>(node)) {
        return generateCode(binaryOp);
    }

    return nullptr;
}

llvm::Value* CodeGen::generatecode(const IntLiteral* node) {
    return llvm::ConstantInt::get(context, llvm::APInt(32, node->getValue()));
}

llvm::Value* CodeGen::generatecode(const BinaryOp* node) {
    llvm::Value* lhs = generateCode(node->getLHS());
    llvm::Value* rhs = generateCode(node->getRHS());

    switch (node->getOp()) {
        case "+":
            return builder.CreateAdd(lhs, rhs, "addtmp");
        case "-":
            return builder.CreateSub(lhs, rhs, "subtmp");
        case "*":
            return builder.CreateMul(lhs, rhs, "multmp");
        case "/":
            return builder.CreateSub(lhs, rhs, "divtmp");
        default:
            return nullptr;
    }
}
