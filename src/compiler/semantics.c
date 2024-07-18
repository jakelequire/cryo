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
#include "compiler/semantics.h"



// >===------------------------------------===< //
// >===--------- Semantic Checks ----------===< //
// >===------------------------------------===< //
void checkVariable(ASTNode* node, CryoSymbolTable* table) {
    CryoSymbol* symbol = findSymbol(table, node->data.varDecl.name);
    if (!symbol) {
        fprintf(stderr, "[Semantics] Error: Undefined variable '%s'\n", node->data.varDecl.name);
        exit(1);
    }
}

// Check if an assignment is valid
void checkAssignment(ASTNode* node, CryoSymbolTable* table) {
    CryoSymbol* symbol = findSymbol(table, node->data.varName.varName);
    if (!symbol) {
        fprintf(stderr, "[Sema] Error: Undefined variable '%s' in assignment\n", node->data.varName.varName);
        exit(1);
    }
    if (symbol->isConstant) {
        fprintf(stderr, "[Sema] Error: Cannot assign to constant variable '%s'\n", node->data.varName.varName);
        exit(1);
    }
    if (symbol->valueType != node->data.expr.expression->data.literalExpression.dataType) {
        fprintf(stderr, "[Sema] Error: Type mismatch in assignment to '%s'\n", node->data.varName.varName);
        exit(1);
    }
}

// Check a function call
void checkFunctionCall(ASTNode* node, CryoSymbolTable* table) {
    CryoSymbol* symbol = findSymbol(table, node->data.functionCall.name);
    if (!symbol || symbol->valueType != DATA_TYPE_FUNCTION) {
        fprintf(stderr, "[Sema] Error: Undefined function '%s'\n", node->data.functionCall.name);
        exit(1);
    }
    if (node->data.functionCall.argCount != symbol->argCount) {
        fprintf(stderr, "[Sema] Error: Argument count mismatch in function call to '%s'\n", node->data.functionCall.name);
        exit(1);
    }
}

void checkFunctionDeclaration(ASTNode* node, CryoSymbolTable* table) {
    enterScope(table);

    // Add function parameters to the symbol table
    for (int i = 0; i < node->data.functionDecl.function->paramCount; i++) {
        ASTNode* param = node->data.functionDecl.function->params[i];
        addSymbol(table, param->data.varDecl.name, param->data.varDecl.dataType, false, false, i, param->data.varDecl.dataType);
    }

    // Traverse the function body
    traverseAST(node->data.functionDecl.function->body, table);

    // Ensure return type matches
    // (Assuming a function body can only have one return type, extend if necessary)
    if (node->data.functionDecl.function->returnType != node->data.functionDecl.function->body->data.expr.expression->type) {
        fprintf(stderr, "[Sema] Error: Return type mismatch in function '%s'.\n", node->data.functionDecl.function->name);
        exit(1);
    }

    exitScope(table);
}


void checkBinaryExpression(ASTNode* node, CryoSymbolTable* table) {
    if (!node) return;
    
    ASTNode* left = node->data.bin_op.left;
    ASTNode* right = node->data.bin_op.right;

    // Recursively check both operands
    traverseAST(left, table);
    traverseAST(right, table);

    // Check if types of left and right operands are compatible
    if (left->data.expr.expression->type != right->data.expr.expression->type) {
        fprintf(stderr, "[Sema] Error: Type mismatch in binary expression.\n");
        exit(1);
    }

    // Assign result type to the expression node
    node->data.expr.expression->type = left->data.expr.expression->type;
}
