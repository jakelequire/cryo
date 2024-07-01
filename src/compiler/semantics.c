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

char* logCryoDataType(CryoDataType type) {
    switch (type) {
        case DATA_TYPE_INT:
            return "int";
        case DATA_TYPE_FLOAT:
            return "float";
        case DATA_TYPE_BOOLEAN:
            return "bool";
        case DATA_TYPE_STRING:
            return "string";
        case DATA_TYPE_FUNCTION:
            return "function";
        default:
            return "unknown";
    }
}

CryoSymbolTable* createSymbolTable() {
    CryoSymbolTable* table = (CryoSymbolTable*)malloc(sizeof(CryoSymbolTable));
    table->count = 0;
    table->capacity = 10;
    table->symbols = (CryoSymbol**)malloc(table->capacity * sizeof(CryoSymbol*));
    table->scopeDepth = 0;
    return table;
}

void freeSymbolTable(CryoSymbolTable* table) {
    for (int i = 0; i < table->count; i++) {
        free(table->symbols[i]);
    }
    free(table->symbols);
    free(table);
}

void printSymbolTable(CryoSymbolTable* table) {
    printf("[Debug] Symbol count: %d\n", table->count);
    printf("[Debug] Scope depth: %d\n", table->scopeDepth);
    printf("[Debug] Table capacity: %d\n", table->capacity);
    printf("\n-----------------------------------------------------\n");
    printf("Symbol Table:\n\n");
    printf("Name               Type    Scope   Const   ArgCount");
    printf("\n-----------------------------------------------------\n");
    for (int i = 0; i < table->count; i++) {
        CryoSymbol* symbol = table->symbols[i];
        printf("%-18s %-7s %-7d %-7s %-8d\n",
               symbol->name,
               logCryoDataType(symbol->type),
               symbol->scopeLevel,
               symbol->isConstant ? "true" : "false",
               symbol->argCount);
    }
    printf("-----------------------------------------------------\n");
}




// >===------------------------------------===< //
// >===-------- Symbol Management ---------===< //
// >===------------------------------------===< //


void enterScope(CryoSymbolTable* table) {
    table->scopeDepth++;
}

void exitScope(CryoSymbolTable* table) {
    int originalCount = table->count;
    for (int i = originalCount - 1; i >= 0; i--) {
        if (table->symbols[i]->scopeLevel >= table->scopeDepth) {
            free(table->symbols[i]->name);
            free(table->symbols[i]);
            table->count--;
        } else {
            break;
        }
    }
    table->scopeDepth--;
}

void enterBlockScope(CryoSymbolTable* table) {
    enterScope(table);
}

void exitBlockScope(CryoSymbolTable* table) {
    exitScope(table);
}


void addSymbol(CryoSymbolTable* table, const char* name, CryoDataType type, bool isConstant) {
    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->symbols = (CryoSymbol**)realloc(table->symbols, table->capacity * sizeof(CryoSymbol*));
    }
    CryoSymbol* symbol = (CryoSymbol*)malloc(sizeof(CryoSymbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->isConstant = isConstant;
    symbol->scopeLevel = table->scopeDepth;
    symbol->argCount = 0;
    table->symbols[table->count++] = symbol;

    printf("[Debug] Added symbol: %s, Type: %d, Scope: %d\n", symbol->name, symbol->type, symbol->scopeLevel);
}




CryoSymbol* findSymbol(CryoSymbolTable* table, const char* name) {
    for (int i = table->count - 1; i >= 0; i--) {
        if (strcmp(table->symbols[i]->name, name) == 0) {
            return table->symbols[i];
        }
    }
    return NULL;
}


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
    if (symbol->type != node->data.expr.expr->data.literalExpression.dataType) {
        fprintf(stderr, "[Sema] Error: Type mismatch in assignment to '%s'\n", node->data.varName.varName);
        exit(1);
    }
}

// Check a function call
void checkFunctionCall(ASTNode* node, CryoSymbolTable* table) {
    CryoSymbol* symbol = findSymbol(table, node->data.functionCall.name);
    if (!symbol || symbol->type != DATA_TYPE_FUNCTION) {
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
    for (int i = 0; i < node->data.functionDecl.params; i++) {
        ASTNode* param = node->data.functionDecl.params[i];
        addSymbol(table, param->data.varDecl.name, param->data.varDecl.dataType, false);
    }

    // Traverse the function body
    traverseAST(node->data.functionDecl.body, table);

    // Ensure return type matches
    // (Assuming a function body can only have one return type, extend if necessary)
    if (node->data.functionDecl.returnType != node->data.functionDecl.body->data.expr.expr->type) {
        fprintf(stderr, "[Sema] Error: Return type mismatch in function '%s'.\n", node->data.functionDecl.name);
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
    if (left->data.expr.expr->type != right->data.expr.expr->type) {
        fprintf(stderr, "[Sema] Error: Type mismatch in binary expression.\n");
        exit(1);
    }

    // Assign result type to the expression node
    node->data.expr.expr->type = left->data.expr.expr->type;
}



// Recursive traversal of AST
void traverseAST(ASTNode* node, CryoSymbolTable* table) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            enterScope(table);
            for (int i = 0; i < node->data.program.stmtCount; i++) {
                traverseAST(node->data.program.statements[i], table);
            }
            printf("\n#------- Symbol Table -------#\n");
            printSymbolTable(table);
            printf("\n#--------- End Table --------#\n");
            exitScope(table);
            break;

        case NODE_VAR_DECLARATION:
            addSymbol(table, node->data.varDecl.name, node->data.varDecl.dataType, false);
            break;


        case NODE_ASSIGN:
            printf("[Semantics] Checking assignment to '%s'\n", node->data.varName.varName);
            checkAssignment(node, table);
            break;

        case NODE_VAR_NAME:
            printf("[Semantics] Checking variable '%s'\n", node->data.varName.varName);
            checkVariable(node, table);
            break;

        case NODE_FUNCTION_CALL:
            printf("[Semantics] Checking function call to '%s'\n", node->data.functionCall.name);
            checkFunctionCall(node, table);
            break;

        case NODE_BLOCK:
            printf("[Semantics] Entering block scope\n");
            enterScope(table);
            for (int i = 0; i < node->data.block.stmtCount; i++) {
                traverseAST(node->data.block.statements[i], table);
            }
            exitScope(table);
            break;

        default:
            printf("[Semantics] Skipping node type %d\n", node->type);
            break;
    }

    traverseAST(node->nextSibling, table);
}


// Main Entry Point
// <analyzeNode>
bool analyzeNode(ASTNode* node) {
    CryoSymbolTable* table = createSymbolTable();
    traverseAST(node, table);

    // Cleanup
    for (int i = 0; i < table->count; i++) {
        free(table->symbols[i]->name);
        free(table->symbols[i]);
    }
    free(table->symbols);
    free(table);
    return true;
}
// </analyzeNode>
