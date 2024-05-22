#include "include/codegen.h"
#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


void generate_code_function(IRInstruction* ir);
void generate_code_return(IRInstruction* ir);
void generate_code_binary_op(IRInstruction* ir);

LLVMModuleRef generate_llvm_module(Node *root) {
    // TODO: Implement this function
    return NULL;
}

bool generate_code(IRInstruction* ir) {
    LLVMModuleRef module = LLVMModuleCreateWithName("cryo_module");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    
    // Implement code generation logic here, converting AST nodes to LLVM IR
    
    LLVMDisposeBuilder(builder);

    free(module);
    return true;
}


void generate_code_function(IRInstruction* ir) {
    printf("FUNC %s\n", ir->func_def.func_name);
    IRInstruction* body = ir->next;
    while (body) {
        generate_code(body);
        body = body->next;
    }
    printf("END_FUNC\n");
}

void generate_code_return(IRInstruction* ir) {
    printf("RET %d\n", ir->ret.value);
}

void generate_code_binary_op(IRInstruction* ir) {
    if (ir->type == IR_ADD) {
        printf("ADD ");
    } else if (ir->type == IR_SUB) {
        printf("SUB ");
    } // Handle other binary operations...

    if (ir->binary_op.left && ir->binary_op.right) {
        generate_code(ir->binary_op.left);
        generate_code(ir->binary_op.right);
    }
    printf("\n");
}
