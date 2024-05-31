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
#ifndef IR_H
#define IR_H
/*------ <includes> ------*/
#include <stdbool.h>
#include <stdlib.h>
/*---<custom_includes>---*/
#include "ast.h"
/*---------<end>---------*/
/*-------<structure_defs>-------*/
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
/*-------<end_defs>-------*/
/*-----<function_prototypes>-----*/
void freeIR(IRInstruction* instr);
IRInstruction* generate_ir_function(ASTNode* node);
IRInstruction* generate_ir_return(ASTNode* node);
IRInstruction* generate_ir_binary_op(ASTNode* node);
IRInstruction* generate_ir(ASTNode* node);
/*-----<end_prototypes>-----*/




#endif // IR_H