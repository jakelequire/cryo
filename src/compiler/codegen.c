#include "codegen.h"
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <stdio.h>

// LLVM Global Variables
LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMContextRef context;

void initializeLLVM() {
    context = LLVMContextCreate();
    module = LLVMModuleCreateWithNameInContext("CryoModule", context);
    builder = LLVMCreateBuilderInContext(context);
}

void finalizeLLVM() {
    // Write LLVM IR to a file
    char *error = NULL;
    if (LLVMPrintModuleToFile(module, "output.ll", &error) != 0) {
        fprintf(stderr, "Error writing LLVM IR to file: %s\n", error);
        LLVMDisposeMessage(error);
    }

    // Clean up LLVM
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    LLVMContextDispose(context);
}

LLVMValueRef generateCode(ASTNode* node);
LLVMValueRef generateFunction(ASTNode* node);
LLVMValueRef generateVariableDeclaration(ASTNode* node);
LLVMValueRef generateBinaryExpression(ASTNode* node);
LLVMValueRef generateUnaryExpression(ASTNode* node);
LLVMValueRef generateReturnStatement(ASTNode* node);
LLVMValueRef generateBlock(ASTNode* node);

LLVMValueRef generateCodeFromAST(ASTNode* node) {
    // Create the main function
    LLVMTypeRef returnType = LLVMInt32TypeInContext(context); // int return type for main
    LLVMTypeRef funcType = LLVMFunctionType(returnType, NULL, 0, 0);
    LLVMValueRef mainFunction = LLVMAddFunction(module, "main", funcType);

    // Create a new basic block to start insertion into
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context, mainFunction, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    // Generate code for the root block (the body of the main function)
    generateBlock(node);

    // Finish off the main function with a return statement
    LLVMValueRef retValue = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    LLVMBuildRet(builder, retValue);

    return mainFunction;
}

LLVMValueRef generateCode(ASTNode* node) {
    switch (node->type) {
        case NODE_FUNCTION_DECLARATION:
            return generateFunction(node);
        case NODE_VAR_DECLARATION:
            return generateVariableDeclaration(node);
        case NODE_BINARY_EXPR:
            return generateBinaryExpression(node);
        case NODE_UNARY_EXPR:
            return generateUnaryExpression(node);
        case NODE_RETURN_STATEMENT:
            return generateReturnStatement(node);
        case NODE_BLOCK:
            return generateBlock(node);
        // Handle other node types...
        default:
            fprintf(stderr, "Unknown node type in code generation: %d\n", node->type);
            return NULL;
    }
}

LLVMValueRef generateFunction(ASTNode* node) {
    // Create LLVM function
    LLVMTypeRef returnType = LLVMVoidTypeInContext(context); // Assume void return type for simplicity
    LLVMTypeRef funcType = LLVMFunctionType(returnType, NULL, 0, 0);
    LLVMValueRef function = LLVMAddFunction(module, node->data.functionDecl.name, funcType);

    // Create a new basic block to start insertion into
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context, function, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    // Generate code for the function body
    generateCode(node->data.functionDecl.body);

    // Finish off the function
    LLVMBuildRetVoid(builder);

    return function;
}

LLVMValueRef generateVariableDeclaration(ASTNode* node) {
    // For simplicity, assume all variables are integers
    LLVMTypeRef intType = LLVMInt32TypeInContext(context);
    LLVMValueRef var = LLVMBuildAlloca(builder, intType, node->data.varDecl.name);

    // Initialize the variable if there is an initializer
    if (node->data.varDecl.initializer) {
        LLVMValueRef initVal = generateCode(node->data.varDecl.initializer);
        LLVMBuildStore(builder, initVal, var);
    }

    return var;
}

LLVMValueRef generateBinaryExpression(ASTNode* node) {
    LLVMValueRef left = generateCode(node->data.bin_op.left);
    LLVMValueRef right = generateCode(node->data.bin_op.right);

    switch (node->data.bin_op.operator) {
        case TOKEN_PLUS:
            return LLVMBuildAdd(builder, left, right, "addtmp");
        case TOKEN_MINUS:
            return LLVMBuildSub(builder, left, right, "subtmp");
        case TOKEN_STAR:
            return LLVMBuildMul(builder, left, right, "multmp");
        case TOKEN_SLASH:
            return LLVMBuildSDiv(builder, left, right, "divtmp");
        default:
            fprintf(stderr, "Unknown binary operator\n");
            return NULL;
    }
}

LLVMValueRef generateUnaryExpression(ASTNode* node) {
    LLVMValueRef operand = generateCode(node->data.unary_op.operand);

    switch (node->data.unary_op.operator) {
        case TOKEN_MINUS:
            return LLVMBuildNeg(builder, operand, "negtmp");
        case TOKEN_OP_NOT:
            return LLVMBuildNot(builder, operand, "nottmp");
        default:
            fprintf(stderr, "Unknown unary operator\n");
            return NULL;
    }
}

LLVMValueRef generateReturnStatement(ASTNode* node) {
    if (node->data.returnStmt.returnValue) {
        LLVMValueRef returnValue = generateCode(node->data.returnStmt.returnValue);
        return LLVMBuildRet(builder, returnValue);
    } else {
        return LLVMBuildRetVoid(builder);
    }
}

LLVMValueRef generateBlock(ASTNode* node) {
    LLVMValueRef lastValue = NULL;
    for (int i = 0; i < node->data.block.stmtCount; ++i) {
        lastValue = generateCode(node->data.block.statements[i]);
    }
    return lastValue;
}
