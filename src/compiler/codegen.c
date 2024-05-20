#include "include/codegen.h"
#include "ir.h"
#include <stdio.h>

void generate_code_function(IRInstruction* ir);
void generate_code_return(IRInstruction* ir);
void generate_code_binary_op(IRInstruction* ir);

LLVMModuleRef generate_code(Node *root) {
    LLVMModuleRef module = LLVMModuleCreateWithName("cryo_module");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    
    // Implement code generation logic here, converting AST nodes to LLVM IR
    
    LLVMDisposeBuilder(builder);
    return module;
}

int generate_code(IRInstruction* ir) {
    if (ir == NULL) return 0;

    switch (ir->type) {
        case IR_FUNC_DEF:
            generate_code_function(ir);
            break;
        case IR_RETURN:
            generate_code_return(ir);
            break;
        case IR_ADD:
            generate_code_binary_op(ir);
            break;
        // Handle other IR instructions...
        default:
            fprintf(stderr, "Unknown IR instruction type\n");
            return 0;
    }

    return 1;
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
