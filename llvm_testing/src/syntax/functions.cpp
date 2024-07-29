// functions.cpp 

#include "cpp/codegen.h"

namespace Cryo {

llvm::Function* CodeGen::generateFunctionPrototype(ASTNode* node) {
    if (node->type != NODE_FUNCTION_DECLARATION && node->type != NODE_EXTERN_FUNCTION) {
        std::cerr << "[CPP] Error: Invalid function node\n";
        return nullptr;
    }

    FunctionDeclNode* funcDecl = (node->type == NODE_FUNCTION_DECLARATION) 
        ? node->data.functionDecl.function 
        : node->data.externNode.decl.function;

    std::string functionName = funcDecl->name;
    llvm::Type* returnType = getLLVMType(funcDecl->returnType);

    std::vector<llvm::Type*> paramTypes;
    for (int i = 0; i < funcDecl->paramCount; ++i) {
        ASTNode* param = funcDecl->params[i];
        llvm::Type* paramType = getLLVMType(param->data.varDecl.dataType);
        paramTypes.push_back(paramType);
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function* function = llvm::Function::Create(
        funcType, 
        (node->type == NODE_EXTERN_FUNCTION) ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage,
        functionName, 
        module.get()
    );

    // Set parameter names
    unsigned idx = 0;
    for (auto &Arg : function->args()) {
        Arg.setName(funcDecl->params[idx]->data.varDecl.name);
        idx++;
    }

    return function;
}


void CodeGen::generateFunction(ASTNode* node) {
    llvm::Function* function = generateFunctionPrototype(node);
    if (!function) return;

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(context, "entry", function);
    builder.SetInsertPoint(BB);

    // Create a new scope for function parameters
    beginScope();

    // Add parameters to the symbol table
    for (auto &Arg : function->args()) {
        llvm::AllocaInst* Alloca = createEntryBlockAlloca(function, Arg.getName());
        builder.CreateStore(&Arg, Alloca);
        symbolTable[std::string(Arg.getName())] = Alloca;
    }

    // Generate code for function body
    if (node->data.functionDecl.function->body) {
        generateCode(node->data.functionDecl.function->body);
    }

    // End the function scope
    endScope();

    // Validate the generated function
    verifyFunction(*function);
}

void CodeGen::generateFunctionCall(ASTNode* node) {
    llvm::Function* callee = module->getFunction(node->data.functionCall.name);
    if (!callee) {
        std::cerr << "Unknown function referenced: " << node->data.functionCall.name << "\n";
        return;
    }

    if (callee->arg_size() != node->data.functionCall.argCount) {
        std::cerr << "Incorrect number of arguments passed\n";
        return;
    }

    std::vector<llvm::Value*> argsV;
    for (unsigned i = 0; i < node->data.functionCall.argCount; ++i) {
        argsV.push_back(generateExpression(node->data.functionCall.args[i]));
        if (!argsV.back()) return;
    }

    return builder.CreateCall(callee, argsV, "calltmp");
}

llvm::AllocaInst* CodeGen::createEntryBlockAlloca(llvm::Function* function, const std::string& varName) {
    llvm::IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    return TmpB.CreateAlloca(llvm::Type::getDoubleTy(context), 0, varName.c_str());
}

} // namespace Cryo
