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

LLVMValueRef codegen_expr(ASTNode* node);

LLVMValueRef codegen_stmt(ASTNode* node) {
    switch (node->type) {
        case NODE_RETURN_STATEMENT:
            return LLVMBuildRet(builder, codegen_expr(node->data.returnStmt.returnValue));
        case NODE_BLOCK: { // Block statement
            for (int i = 0; i < node->data.block.stmtCount; ++i) {
                codegen_stmt(node->data.block.statements[i]);
            }
            return NULL;
        }
        case NODE_VAR_DECLARATION: // Variable declaration
            return codegen_expr(node->data.varDecl.initializer);
        default:
            return codegen_expr(node);
    }
}

LLVMValueRef codegen_expr(ASTNode* node) {
    switch (node->type) {
        case NODE_LITERAL:
            return LLVMConstInt(LLVMInt32Type(), node->data.value, 0);
        case NODE_VAR_NAME: { // Variable usage
            LLVMValueRef var = LLVMGetNamedGlobal(module, node->data.varName.varName);
            if (!var) {
                fprintf(stderr, "Error: Undefined variable %s\n", node->data.varName.varName);
                return NULL;
            }
            return LLVMBuildLoad2(builder, LLVMInt32Type(), var, node->data.varName.varName);
        }
        case NODE_BINARY_EXPR: { // Binary operations
            LLVMValueRef left = codegen_expr(node->data.bin_op.left);
            LLVMValueRef right = codegen_expr(node->data.bin_op.right);
            if (node->data.bin_op.operator == TOKEN_OP_PLUS) {
                return LLVMBuildAdd(builder, left, right, "addtmp");
            } else if (node->data.bin_op.operator == TOKEN_OP_MINUS) {
                return LLVMBuildSub(builder, left, right, "subtmp");
            }
            break;
        }
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

    codegen_stmt(node->data.functionDecl.body);

    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0)); // Add a default return 0 for simplicity
}

void finalize_codegen() {
    printf("Finalizing Compilation...\n");
    LLVMVerifyModule(module, LLVMAbortProcessAction, NULL);
    printf("------------- <Module Dump> -------------\n");
    LLVMDumpModule(module);
    printf("------------- <Module Dump End> -------------\n");
    //                    The output is here.
    LLVMPrintModuleToFile(module, "output.ll", NULL);
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    printf("Compilation Finished.\n");
}
