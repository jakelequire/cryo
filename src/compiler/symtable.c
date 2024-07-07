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
#include "compiler/symtable.h"


// <logSymCryoDataType>
char* logSymCryoDataType(CryoDataType type) {
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
        case DATA_TYPE_VOID:
            return "void";
        case DATA_TYPE_NULL:
            return "null";
        case DATA_TYPE_UNKNOWN:
            return "unknown";
        case NODE_LITERAL_INT:
            return "lit_int";
        default:
            return "unknown";
    }
}
// </logSymCryoDataType>


// <createSymbolTable>
CryoSymbolTable* createSymbolTable() {
    CryoSymbolTable* table = (CryoSymbolTable*)malloc(sizeof(CryoSymbolTable));
    table->count = 0;
    table->capacity = 10;
    table->symbols = (CryoSymbol**)malloc(table->capacity * sizeof(CryoSymbol*));
    table->scopeDepth = 0;
    return table;
}
// </createSymbolTable>


// <freeSymbolTable>
void freeSymbolTable(CryoSymbolTable* table) {
    for (int i = 0; i < table->count; i++) {
        free(table->symbols[i]);
    }
    free(table->symbols);
    free(table);
}
// </freeSymbolTable>


// <printSymbolTable>
void printSymbolTable(CryoSymbolTable* table) {
    printf("[SymTable - Debug] Symbol count: %d\n", table->count);
    printf("[SymTable - Debug] Scope depth: %d\n", table->scopeDepth);
    printf("[SymTable - Debug] Table capacity: %d\n", table->capacity);
    printf("\n---------------------------------------------------------------------\n");
    printf("Symbol Table:\n\n");
    printf("Name                       Type         Scope        Const   ArgCount");
    printf("\n---------------------------------------------------------------------\n");
    for (int i = 0; i < table->count; i++) {
        CryoSymbol* symbol = table->symbols[i];
        printf("%-25s %-15s %-10d %-10s %-10d\n",
               symbol->name,
               logSymCryoDataType(symbol->type),
               symbol->scopeLevel,
               symbol->isConstant ? "true" : "false",
               symbol->argCount);
    }
    printf("---------------------------------------------------------------------\n");
}
// </printSymbolTable>


// <enterScope>
void enterScope(CryoSymbolTable* table) {
    table->scopeDepth++;
}
// </enterScope>


// <jumpScope>
void jumpScope(CryoSymbolTable* table) {
    while (table->count > 0 && table->symbols[table->count - 1]->scopeLevel >= table->scopeDepth) {
        table->count--;
    }
    table->scopeDepth--;
}
// </jumpScope>


// <exitScope>
void exitScope(CryoSymbolTable* table) {
    while (table->count > 0 && table->symbols[table->count - 1]->scopeLevel == table->scopeDepth) {
        free(table->symbols[table->count - 1]->name);
        free(table->symbols[table->count - 1]);
        table->count--;
    }
    table->scopeDepth--;
}
// </exitScope>


// <enterBlockScope>
void enterBlockScope(CryoSymbolTable* table) {
    enterScope(table);
}
// </enterBlockScope>


// <exitBlockScope>
void exitBlockScope(CryoSymbolTable* table) {
    exitScope(table);
}
// </exitBlockScope>


// <addSymbol>
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

    printf("[SymTable - Debug] Added symbol: %s, Type: %d, Scope: %d\n", symbol->name, symbol->type, symbol->scopeLevel);
}
// </addSymbol>


// <findSymbol>
CryoSymbol* findSymbol(CryoSymbolTable* table, const char* name) {
    for (int i = table->count - 1; i >= 0; i--) {
        if (strcmp(table->symbols[i]->name, name) == 0) {
            return table->symbols[i];
        }
    }
    return NULL;
}
// </findSymbol>


// Recursive traversal of AST
// <traverseAST>
void traverseAST(ASTNode* node, CryoSymbolTable* table) {
while (node) {
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
                printf("[SymTable] Checking assignment to '%s'\n", node->data.varName.varName);
                if (!findSymbol(table, node->data.varName.varName)) {
                    printf("[Error] Variable '%s' not declared\n", node->data.varName.varName);
                    break;
                }
                break;

            case NODE_VAR_NAME:
                printf("[SymTable] Checking variable '%s'\n", node->data.varName.varName);
                if (!findSymbol(table, node->data.varName.varName)) {
                    printf("[SymTable - Error] Variable '%s' not declared\n", node->data.varName.varName);
                }
                break;

            case NODE_FUNCTION_CALL:
                printf("[SymTable] Checking function call to '%s'\n", node->data.functionCall.name);
                if (!findSymbol(table, node->data.functionCall.name)) {
                    printf("[SymTable - Error] Function '%s' not declared\n", node->data.functionCall.name);
                }
                break;

            case NODE_FUNCTION_DECLARATION:
                printf("[SymTable] Checking function '%s'\n", node->data.functionDecl.function->name);
                addSymbol(table, node->data.functionDecl.function->name, DATA_TYPE_FUNCTION, false);
                enterScope(table); // Enter function scope
                // Add function parameters to symbol table
                for (int i = 0; i < node->data.functionDecl.function->paramCount; i++) {
                    addSymbol(table, node->data.functionDecl.function->params[i]->data.varDecl.name, node->data.functionDecl.function->params[i]->data.varDecl.dataType, false);
                }
                traverseAST(node->data.functionDecl.function->body, table);
                break;

            case NODE_FUNCTION_BLOCK:
                printf("[SymTable] Entering function block scope\n");
                enterScope(table);
                for (int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; i++) {
                    traverseAST(node->data.functionBlock.block->data.block.statements[i], table);
                }
                break;

            case NODE_BLOCK:
                printf("[SymTable] Entering block scope\n");
                enterScope(table);
                for (int i = 0; i < node->data.block.stmtCount; i++) {
                    traverseAST(node->data.block.statements[i], table);
                }
                break;

            default:
                printf("[SymTable] Skipping node type %d\n", node->type);
                break;
        }

        node = node->nextSibling;
    }
}
// </traverseAST>


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