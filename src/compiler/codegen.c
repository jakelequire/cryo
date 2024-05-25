#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <stdio.h>
#include "ast.h"
#include "codegen.h"

LLVMModuleRef module;
LLVMBuilderRef builder;

void init_codegen() {
    printf("\n-----------------------------");
    printf("\nCryo Compiler - Codegen\n\n");
    printf("\nInitializing Compilation...\n");
    LLVMInitializeAllTargetInfos();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();
    module = LLVMModuleCreateWithName("cryo_module");
    builder = LLVMCreateBuilder();

    printf("Compilation Initialized.\n");
    printf("-----------------------------\n");
}

LLVMValueRef codegen_expr(ASTNode* node) {
    switch (node->type) {
        case TOKEN_INT:
            return LLVMConstInt(LLVMInt32Type(), node->data.value, 0);
        case TOKEN_KW_RETURN:
            return LLVMBuildRet(builder, codegen_expr(&node->data.returnStmt));
        // Handle other kinds of nodes...
        default:
            return NULL;
    }
}

void codegen_function(ASTNode* node) {
    LLVMTypeRef return_type = LLVMInt32Type(); // Assuming return type is always int for simplicity
    LLVMTypeRef function_type = LLVMFunctionType(return_type, NULL, 0, 0);
    LLVMValueRef function = LLVMAddFunction(module, node->data.functionDecl.name, function_type);

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    codegen_expr(node->data.functionDecl.body);

    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0)); // Add a default return 0 for simplicity
}

void finalize_codegen() {
    printf("Finalizing Compilation...\n");
    LLVMVerifyModule(module, LLVMAbortProcessAction, NULL);
    LLVMDumpModule(module);
    LLVMPrintModuleToFile(module, "output.ll", NULL);
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    printf("Compilation Finished.\n");;
}

