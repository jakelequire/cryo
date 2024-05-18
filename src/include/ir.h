#ifndef IR_H
#define IR_H

#include "ast.h"

typedef enum {
    IR_FUNC_DEF,
    IR_FUNC_CALL,
    IR_RETURN,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    // Add other instruction types as needed
} IR_Type;

typedef struct IRInstruction {
    IR_Type type;
    union {
        struct { char* func_name; } func_def;
        struct { char* func_name; } func_call;
        struct { int value; } ret;
        struct {
            struct IRInstruction* left;
            struct IRInstruction* right;
        } binary_op;
    };
    struct IRInstruction* next;
} IRInstruction;


void freeIR(IRInstruction* instr);

IRInstruction* generate_ir_function(ASTNode* node);
IRInstruction* generate_ir_return(ASTNode* node);
IRInstruction* generate_ir_binary_op(ASTNode* node);

IRInstruction* generate_ir(ASTNode* node);


#endif // IR_H