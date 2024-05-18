#include "include/ir.h"
#include <stdlib.h>


IRInstruction* generate_ir_function(ASTNode* node) {
    if (node == NULL || node->type != NODE_FUNCTION_DECL) return NULL;

    IRInstruction* instr = (IRInstruction*)malloc(sizeof(IRInstruction));
    instr->type = IR_FUNC_DEF;
    instr->func_def.func_name = node->data.functionDecl.name;
    instr->next = generate_ir(node->data.functionDecl.body);
    
    return instr;
}

IRInstruction* generate_ir_return(ASTNode* node) {
    if (node == NULL || node->type != NODE_RETURN_STATEMENT) return NULL;

    IRInstruction* instr = (IRInstruction*)malloc(sizeof(IRInstruction));
    instr->type = IR_RETURN;
    instr->ret.value = node->data.returnStmt.returnValue->data.value;
    instr->next = NULL;

    return instr;
}

IRInstruction* generate_ir_binary_op(ASTNode* node) {
    if (node == NULL || node->type != NODE_BINARY_EXPR) return NULL;

    IRInstruction* instr = (IRInstruction*)malloc(sizeof(IRInstruction));
    instr->type = IR_ADD; // Assume the operation is addition for simplicity
    instr->binary_op.left = generate_ir(node->data.bin_op.left);
    instr->binary_op.right = generate_ir(node->data.bin_op.right);
    instr->next = NULL;

    return instr;
}

IRInstruction* generate_ir(ASTNode* node) {
    if (node == NULL) return NULL;

    switch (node->type) {
        case NODE_FUNCTION_DECL:
            return generate_ir_function(node);
        case NODE_RETURN_STATEMENT:
            return generate_ir_return(node);
        case NODE_BINARY_EXPR:
            return generate_ir_binary_op(node);
        // Handle other AST node types...
        default:
            return NULL;
    }
}
