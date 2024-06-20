/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          *
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#include "compiler/codegen.h"

LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMValueRef function;
LLVMValueRef named_values[256]; // Array to hold named values, assuming a small number of variables for simplicity

// Helper function to get the current insert block with error checking
LLVMBasicBlockRef getInsertBlockWithCheck(LLVMBuilderRef builder) {
    LLVMBasicBlockRef block = LLVMGetInsertBlock(builder);
    if (!block) {
        fprintf(stderr, "[CodeGen] {ERROR} No current block for builder\n");
        exit(1);
    }
    return block;
}

// Helper function to check LLVM errors and exit on failure
void checkLLVMError(const char* errorMessage) {
    char* error = NULL;
    LLVMVerifyModule(module, LLVMReturnStatusAction, &error);
    if (error) {
        fprintf(stderr, "%s: %s\n", errorMessage, error);
        LLVMDisposeMessage(error);
        exit(1);
    }
}

// Initialization of LLVM
void initializeLLVM() {
    LLVMInitializeCore(LLVMGetGlobalPassRegistry());
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();

    module = LLVMModuleCreateWithName("my_module");
    if (!module) {
        fprintf(stderr, "[Init] {ERROR} Failed to create module\n");
        exit(1);
    }

    builder = LLVMCreateBuilder();
    if (!builder) {
        fprintf(stderr, "[Init] {ERROR} Failed to create builder\n");
        exit(1);
    }

    LLVMTypeRef param_types[] = { LLVMInt32Type() };
    LLVMTypeRef ret_type = LLVMFunctionType(LLVMInt32Type(), param_types, 1, 0);
    function = LLVMAddFunction(module, "main", ret_type);
    if (!function) {
        fprintf(stderr, "[Init] {ERROR} Failed to add function\n");
        exit(1);
    }

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
    if (!entry) {
        fprintf(stderr, "[Init] {ERROR} Failed to append basic block\n");
        exit(1);
    }
    LLVMPositionBuilderAtEnd(builder, entry);
}

// Code generation function
LLVMValueRef generateCode(ASTNode* node) {
    if (!node) {
        fprintf(stderr, "[CodeGen] {ERROR} Node is NULL\n");
        return NULL;
    }

    printf("[DEBUG] Generating code for node type: %d\n", node->type);

    switch (node->type) {
        case NODE_PROGRAM: {
            printf("[CodeGen] Generating code for program\n");
            LLVMValueRef last_value = NULL;
            for (int i = 0; i < node->data.program.stmtCount; ++i) {
                last_value = generateCode(node->data.program.statements[i]);
            }
            return last_value;
        }

        case NODE_LITERAL_EXPR:
            printf("[CodeGen] Generating code for literal expression: %d\n", node->data.value);
            return LLVMConstInt(LLVMInt32Type(), node->data.value, 0);

        case NODE_VAR_DECLARATION: {
            printf("[CodeGen] Generating code for variable declaration: %s\n", node->data.varDecl.name);
            LLVMValueRef init_val = generateCode(node->data.varDecl.initializer);
            if (!init_val) {
                fprintf(stderr, "[CodeGen] {ERROR} Failed to generate code for initializer\n");
                return NULL;
            }

            LLVMTypeRef var_type = LLVMTypeOf(init_val);
            printf("[CodeGen] Variable type created: %p\n", var_type);

            LLVMBasicBlockRef current_block = getInsertBlockWithCheck(builder);

            LLVMValueRef parent_func = LLVMGetBasicBlockParent(current_block);
            if (!parent_func) {
                fprintf(stderr, "[CodeGen] {ERROR} No parent function for the current block\n");
                return NULL;
            }

            printf("[CodeGen] Inserting variable into function %p\n", parent_func);

            LLVMValueRef var = LLVMBuildAlloca(builder, var_type, node->data.varDecl.name);
            if (!var) {
                fprintf(stderr, "[CodeGen] {ERROR} LLVMBuildAlloca failed\n");
                return NULL;
            }
            printf("[CodeGen] Variable allocated: %p\n", var);

            LLVMBuildStore(builder, init_val, var);
            printf("[CodeGen] Variable initialized\n");
            named_values[node->data.varDecl.name[0] % 256] = var;
            printf("[CodeGen] Variable stored in named_values\n");
            return var;
        }

        case NODE_BINARY_EXPR: {
            printf("[CodeGen] Generating code for binary expression\n");
            LLVMValueRef left = generateCode(node->data.bin_op.left);
            LLVMValueRef right = generateCode(node->data.bin_op.right);
            switch (node->data.bin_op.operator) {
                case '+':
                    return LLVMBuildAdd(builder, left, right, "addtmp");
                case '-':
                    return LLVMBuildSub(builder, left, right, "subtmp");
                default:
                    return NULL;
            }
        }

        default:
            fprintf(stderr, "[CodeGen] {ERROR} Unknown node type: %d\n", node->type);
            return NULL;
    }
}

// Finalize LLVM function
void finalizeLLVM(LLVMValueRef result) {
    if (result) {
        LLVMBuildRet(builder, result);

        char *error = NULL;
        LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
        if (error) {
            fprintf(stderr, "LLVMVerifyModule: %s\n", error);
            LLVMDisposeMessage(error);
        } else {
            printf("[CodeGen] Module verified successfully\n");
        }

        char *llvm_ir = LLVMPrintModuleToString(module);
        printf("%s\n", llvm_ir);
        LLVMDisposeMessage(llvm_ir);
        printf("[CodeGen] IR code generated!\n");
    } else {
        fprintf(stderr, "[CodeGen] {ERROR} No result to finalize.\n");
    }

    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    printf("[CodeGen] LLVM resources freed.\n");
}


// ---------------------------------------------------------------------------------------------------------------------
// DEBUGGING FUNCTIONS

int codegen(int argc, char* argv[]) {
    initializeLLVM();

    // Generate some code
    ASTNode* left = createASTNode(NODE_LITERAL_EXPR);
    left->data.value = 5;

    ASTNode* right = createASTNode(NODE_LITERAL_EXPR);
    right->data.value = 7;

    ASTNode* add = createASTNode(NODE_BINARY_EXPR);
    add->data.bin_op.left = left;
    add->data.bin_op.right = right;
    add->data.bin_op.operator = '+';

    LLVMValueRef result = generateCode(add);
    if (result) {
        finalizeLLVM(result);
    } else {
        fprintf(stderr, "Error: Code generation failed.\n");
    }
    
    return 0;
}
