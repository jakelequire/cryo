
// variables.cpp 

#include "cpp/codegen.h"

namespace Cryo {

llvm::Value* CodeGen::generateVariableDeclaration(ASTNode* node) {
    std::string varName = node->data.varDecl.name;
    llvm::Type* varType = getLLVMType(node->data.varDecl.dataType);

    llvm::AllocaInst* alloca;
    if (node->data.varDecl.isGlobal) {
        llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(
            *module,
            varType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            llvm::Constant::getNullValue(varType),
            varName
        );
        symbolTable[varName] = globalVar;
        alloca = globalVar;
    } else {
        alloca = builder.CreateAlloca(varType, 0, varName);
        symbolTable[varName] = alloca;
    }

    if (node->data.varDecl.initializer) {
        llvm::Value* initValue = generateExpression(node->data.varDecl.initializer);
        builder.CreateStore(initValue, alloca);
    }

    return alloca;
}

llvm::Value* CodeGen::generateVariableReference(ASTNode* node) {
    std::string varName = node->data.varName.varName;
    llvm::Value* value = symbolTable[varName];
    if (!value) {
        std::cerr << "Unknown variable name: " << varName << "\n";
        return nullptr;
    }

    return builder.CreateLoad(value->getType(), value, varName.c_str());
}

llvm::Value* CodeGen::generateAssignment(ASTNode* node) {
    llvm::Value* variable = symbolTable[node->data.bin_op.left->data.varName.varName];
    if (!variable) {
        std::cerr << "Unknown variable name: " << node->data.bin_op.left->data.varName.varName << "\n";
        return nullptr;
    }

    llvm::Value* value = generateExpression(node->data.bin_op.right);
    if (!value) return nullptr;

    builder.CreateStore(value, variable);
    return value;
}

void CodeGen::beginScope() {
    symbolTable.push();
}

void CodeGen::endScope() {
    symbolTable.pop();
}

llvm::Value* CodeGen::getVariable(const std::string& name) {
    return symbolTable[name];
}

} // namespace Cryo
