#include "cppCodeGen.h"

CodeGen::CodeGen() : builder(context) {
    module = std::make_unique<llvm::Module>("main", context);
}

llvm::Value* CodeGen::generateCode(const ASTNode* node) {
    if (const IntLiteral* intLiteral = dynamic_cast<const IntLiteral*>(node)) {
        return generateCode(intLiteral);
    } else if (const BinaryOp* binaryOp = dynamic_cast<const BinaryOp*>(node)) {
        return generateCode(binaryOp);
    }
    return nullptr;
}

llvm::Value* CodeGen::generateCode(const IntLiteral* intLiteral) {
    return llvm::ConstantInt::get(context, llvm::APInt(32, intLiteral->getValue()));
}

llvm::Value* CodeGen::generateCode(const BinaryOp* binaryOp) {
    llvm::Value* lhs = generateCode(binaryOp->getLHS());
    llvm::Value* rhs = generateCode(binaryOp->getRHS());
    switch (binaryOp->getOp()) {
        case '+':
            return builder.CreateAdd(lhs, rhs, "addtmp");
        case '-':
            return builder.CreateSub(lhs, rhs, "subtmp");
        case '*':
            return builder.CreateMul(lhs, rhs, "multmp");
        case '/':
            return builder.CreateSDiv(lhs, rhs, "divtmp");
        default:
            return nullptr;
    }
}