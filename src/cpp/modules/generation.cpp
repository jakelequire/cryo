

#include "cpp/codegen.h"


namespace Cryo {


void CryoModules::generateProgram(ASTNode* node) {
    std::cout << "[Generation] Generating code for program\n";
    if (node->data.program.stmtCount == 0) {
        std::cerr << "[Generation] Error generating code for program: No statements found\n";
        return;
    }
    for (int i = 0; i < node->data.program.stmtCount; ++i) {
        std::cout << "[Generation] Generating code for program statement " << i << "\n";
        identifyNodeExpression(node->data.program.statements[i]);
        std::cout << "[Generation] Moving to next statement\n";
    }
    return;
}



void CryoModules::generateBlock(ASTNode* node) {
    if(!node) {
        std::cerr << "[Generation] Error: Failed to generate Block Node. \n";
    }
    std::cout << "[Generation] Generating code for Block Node.\n";

    for(int i = 0; i < node->data.block.stmtCount; i++) {
        std::cout << "[Generation] Generating Code for Block Statement...\n";
        identifyNodeExpression(node->data.block.statements[i]);
    }

    return;
}



void CryoModules::generateFunctionBlock(ASTNode* node) {
    if(!node || !node->data.functionBlock.block) {
        std::cerr << "[Generation] Error: Failed to generate Function Block. @<generateFunctionBlock>\n";
        return;
    }
    std::cout << "[Generation] Generating code for Function Block Node.\n";
    
    for(int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; ++i) {
        std::cout << "[Generation] Generating code for Block Statement. Count: " << i << "\n";
        identifyNodeExpression(node->data.functionBlock.block->data.block.statements[i]);
        std::cout << "[Generation] Moving to next statement...\n";
    }

    llvm::BasicBlock* BB = builder.GetInsertBlock();
    if(!BB->getTerminator()) {
        builder.CreateRetVoid();
    }

    std::cout << "[Generation] Function Block code generation complete!\n";
}



void CryoModules::generateExternalDeclaration(ASTNode* node) {
    if(!node) {
        std::cerr << "[Generation] Error: Extern Declaration Node is null.\n";
        return;
    }

    char* functionName = node->data.externNode.decl.function->name;
    CryoDataType returnTypeData = node->data.externNode.decl.function->returnType;

    llvm::Type *returnType = cryoTypesInstance->getLLVMType(returnTypeData);
    std::vector<llvm::Type*> paramTypes;

    for (int i = 0; i < node->data.externNode.decl.function->paramCount; ++i) {
        ASTNode* parameter = node->data.externNode.decl.function->params[i];
        llvm::Type* paramType;
        if (parameter == nullptr) {
            std::cerr << "[Generation] Error: Extern Parameter is null\n";
            break;
        }
        if(parameter->data.varDecl.dataType == DATA_TYPE_INT) {
            paramType = llvm::Type::getInt32Ty(module->getContext());
            paramTypes.push_back(paramType);
            break;
        }
        if(parameter->data.varDecl.dataType == DATA_TYPE_STRING) {
            paramType = cryoTypesInstance->getLLVMType(DATA_TYPE_STRING);
            paramTypes.push_back(paramType);
            break;
        }
        if (!paramType) {
            std::cerr << "[Generation] Error: Extern Unknown parameter type\n";
            return;
        }
    }
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes[0], false);

    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, module.get());
}



} // namespace Cryo
