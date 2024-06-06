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
// LLVM Global Variables
LLVMModuleRef module;
LLVMBuilderRef builder;
LLVMContextRef context;

// <initializeLLVM>
void initializeLLVM() {
    context = LLVMContextCreate();
    module = LLVMModuleCreateWithNameInContext("CryoModule", context);
    builder = LLVMCreateBuilderInContext(context);
    printf("LLVM context, module, and builder initialized.\n");
}

// </initializeLLVM>


// <finalizeLLVM>
void finalizeLLVM() {
    // Write LLVM IR to a file
    char *error = NULL;
    if (LLVMPrintModuleToFile(module, "output.ll", &error) != 0) {
        fprintf(stderr, "Error writing LLVM IR to file: %s\n", error);
        LLVMDisposeMessage(error);
    } else {
        LLVMDisposeMessage(error);
        printf("LLVM IR successfully written to output.ll\n");
    }

    // Clean up LLVM
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    LLVMContextDispose(context);
    printf("LLVM context, module, and builder finalized.\n");
}
// </finalizeLLVM>


// Function to generate LLVM IR for a binary expression
LLVMValueRef generateBinaryExprLLVM(LLVMBuilderRef builder, ASTNode* node) {
    LLVMValueRef left = generateLLVM(builder, node->data.bin_op.left);
    LLVMValueRef right = generateLLVM(builder, node->data.bin_op.right);

    if (!left || !right) {
        fprintf(stderr, "Error generating binary operands.\n");
        return NULL;
    }

    LLVMValueRef result = NULL;
    switch (node->data.bin_op.operator) {
        case TOKEN_OP_PLUS:
            printf("Generating add operation for %p and %p.\n", left, right);
            result = LLVMBuildAdd(builder, left, right, "addtmp");
            printf("Generated add result: %p\n", result);
            break;
        // Handle other binary operators...
        default:
            fprintf(stderr, "Unknown binary operator: %d\n", node->data.bin_op.operator);
            return NULL;
    }

    if (result) {
        printf("Binary operation result in generateBinaryExprLLVM: %p\n", result);
    } else {
        fprintf(stderr, "Error generating binary operation result.\n");
    }

    return result;
}


// Recursive function to generate LLVM IR from AST
LLVMValueRef generateLLVM(LLVMBuilderRef builder, ASTNode* node) {
    switch (node->type) {
        case NODE_LITERAL:
            printf("Generating literal: %d\n", node->data.value);
            return LLVMConstInt(LLVMInt32TypeInContext(context), node->data.value, 0);
        case NODE_BINARY_EXPR:
            printf("Generating binary expression\n");
            LLVMValueRef result = generateBinaryExprLLVM(builder, node);
            printf("Binary expression result in generateLLVM: %p\n", result); // Add debug print
            return result;
        default:
            printf("Unknown node type: %d\n", node->type);
            return LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0); // Default return value
    }
}


// Function to generate LLVM IR for the entire program
void generateProgramLLVM(ASTNode* root, const char* filename) {
    initializeLLVM();

    LLVMTypeRef mainType = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);
    LLVMValueRef mainFunc = LLVMAddFunction(module, "main", mainType);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context, mainFunc, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    LLVMValueRef result = generateLLVM(builder, root);

    if (!result || LLVMGetTypeKind(LLVMTypeOf(result)) != LLVMIntegerTypeKind) {
        printf("Invalid result type or value. Defaulting to 0.\n");
        result = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
    } else {
        printf("Valid result generated: %p\n", result);
    }

    LLVMBuildRet(builder, result);

    char *error = NULL;
    if (LLVMVerifyModule(module, LLVMAbortProcessAction, &error) != 0) {
        fprintf(stderr, "Error verifying module: %s\n", error);
        LLVMDisposeMessage(error);
    } else {
        LLVMDisposeMessage(error);
    }

    if (LLVMPrintModuleToFile(module, filename, &error) != 0) {
        fprintf(stderr, "Error writing LLVM IR to file: %s\n", error);
        LLVMDisposeMessage(error);
    } else {
        LLVMDisposeMessage(error);
        printf("LLVM IR successfully written to %s\n", filename);
    }

    finalizeLLVM();

    printf("LLVM IR generated and written to %s\n", filename);
}

