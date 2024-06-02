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
#include "compiler/ir.h"



// <freeIR>
void freeIR(IRInstruction* instr) {
    // TODO: Implement
}
// </freeIR>


// <generate_ir_function>
IRInstruction* generate_ir_function(ASTNode* node) {
    if (node == NULL || node->type != NODE_FUNCTION_DECLARATION) return NULL;

    IRInstruction* instr = (IRInstruction*)malloc(sizeof(IRInstruction));
    instr->type = IR_FUNC_DEF;
    instr->func_def.func_name = node->data.functionDecl.name;
    instr->next = generate_ir(node->data.functionDecl.body);
    
    return instr;
}
// </generate_ir_function>


// <generate_ir_return>
IRInstruction* generate_ir_return(ASTNode* node) {
    if (node == NULL || node->type != NODE_RETURN_STATEMENT) return NULL;

    IRInstruction* instr = (IRInstruction*)malloc(sizeof(IRInstruction));
    instr->type = IR_RETURN;
    instr->ret.value = node->data.returnStmt.returnValue->data.value;
    instr->next = NULL;

    return instr;
}
// </generate_ir_return>


// <generate_ir_binary_op>
IRInstruction* generate_ir_binary_op(ASTNode* node) {
    if (node == NULL || node->type != NODE_BINARY_EXPR) return NULL;

    IRInstruction* instr = (IRInstruction*)malloc(sizeof(IRInstruction));
    instr->type = IR_ADD; // Assume the operation is addition for simplicity
    instr->binary_op.left = generate_ir(node->data.bin_op.left);
    instr->binary_op.right = generate_ir(node->data.bin_op.right);
    instr->next = NULL;

    return instr;
}
// </generate_ir_binary_op>


// <generate_ir>
IRInstruction* generate_ir(ASTNode* node) {
    if (node == NULL) return NULL;

    switch (node->type) {
        case NODE_FUNCTION_DECLARATION:
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
// </generate_ir>
