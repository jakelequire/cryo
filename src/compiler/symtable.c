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
        case DATA_TYPE_UNKNOWN:
            return "unknown";
        case DATA_TYPE_INT:
            return "int";
        case DATA_TYPE_FLOAT:
            return "float";
        case DATA_TYPE_STRING:
            return "string";
        case DATA_TYPE_BOOLEAN:
            return "boolean";
        case DATA_TYPE_FUNCTION:
            return "function";
        case DATA_TYPE_EXTERN_FUNCTION:
            return "extern_func";
        case DATA_TYPE_VOID:
            return "void";
        case DATA_TYPE_NULL:
            return "null";
        case DATA_TYPE_ARRAY:
            return "array";
        case DATA_TYPE_INT_ARRAY:
            return "int[]";
        case DATA_TYPE_FLOAT_ARRAY:
            return "float[]";
        case DATA_TYPE_STRING_ARRAY:
            return "string[]";
        case DATA_TYPE_BOOLEAN_ARRAY:
            return "boolean[]";
        case DATA_TYPE_VOID_ARRAY:
            return "void[]";
        case INTERNAL_DATA_TYPE_EXPRESSION:
            return "i_expression";
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
    printf("\n----------------------------------------------------------------------------------------------\n");
    printf("Symbol Table:\n\n");
    printf("Name                   Type               Val/RetType        Scope            Const    ArgCount\n");
    printf("\n----------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < table->count; i++) {
        CryoSymbol* symbol = table->symbols[i];
        printf("%-20s %-20s %-20s %-15d %-10s %-10d\n",
                symbol->name,
                CryoNodeTypeToString(symbol->nodeType),
                CryoDataTypeToString(symbol->valueType),
                symbol->scopeLevel,
                symbol->isConstant ? "true" : "false",
                symbol->argCount
            );
    }
    printf("----------------------------------------------------------------------------------------------\n");
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
void addSymbol(CryoSymbolTable* table, const char* name, CryoNodeType nodeType, CryoDataType valueType, bool isConstant, int argCount, CryoDataType paramTypes) {
    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->symbols = (CryoSymbol**)realloc(table->symbols, table->capacity * sizeof(CryoSymbol*));
    }
    CryoSymbol* symbol = (CryoSymbol*)malloc(sizeof(CryoSymbol));
    symbol->name = strdup(name);
    symbol->nodeType = nodeType;
    symbol->valueType = valueType;
    symbol->isConstant = isConstant;
    symbol->scopeLevel = table->scopeDepth;
    symbol->argCount = argCount;
    symbol->paramTypes = (CryoDataType*)malloc(argCount * sizeof(CryoDataType));
    for (int i = 0; i < argCount; ++i) {
        symbol->paramTypes[i] = &paramTypes;
    }
    table->symbols[table->count++] = symbol;

    printf("\n[SymTable] Added symbol '%s' of type '%s' and val/returnType of '%s' to scope %d with %d args\n\n",
        name, CryoNodeTypeToString(nodeType), CryoDataTypeToString(valueType), table->scopeDepth, argCount);
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



// <resolveType>
CryoDataType resolveType(CryoSymbolTable* table, ASTNode* node) {
    switch (node->type) {
        case NODE_LITERAL_EXPR:
            return node->data.literalExpression.dataType;

        case NODE_VAR_NAME: {
            CryoSymbol* symbol = findSymbol(table, node->data.varName.varName);
            return symbol ? symbol->nodeType : DATA_TYPE_UNKNOWN;
        }

        case NODE_BINARY_EXPR: {
            CryoDataType leftType = resolveType(table, node->data.bin_op.left);
            CryoDataType rightType = resolveType(table, node->data.bin_op.right);
            if (leftType == rightType) {
                return leftType;
            } else {
                // Add logic to handle type coercion if needed
                return DATA_TYPE_UNKNOWN;
            }
        }

        case NODE_UNARY_EXPR:
            return resolveType(table, node->data.unary_op.operand);

        case NODE_FUNCTION_CALL: {
            CryoSymbol* funcSymbol = findSymbol(table, node->data.functionCall.name);
            if (!funcSymbol || funcSymbol->nodeType != NODE_FUNCTION_CALL) {
                return DATA_TYPE_UNKNOWN;
            }
            // Resolve types of arguments
            for (int i = 0; i < node->data.functionCall.argCount; ++i) {
                CryoDataType argType = resolveType(table, node->data.functionCall.args[i]);
                if (argType == DATA_TYPE_UNKNOWN) {
                    return DATA_TYPE_UNKNOWN;
                }
            }
            return funcSymbol->valueType; // or return function return type if available
        }

        case NODE_ASSIGN:
            // Assignment statements do not have a direct data type

        case NODE_STRING_LITERAL:
            return DATA_TYPE_STRING;

        case NODE_BOOLEAN_LITERAL:
            return DATA_TYPE_BOOLEAN;

        case NODE_ARRAY_LITERAL:
            if (node->data.arrayLiteral.elementCount > 0) {
                return resolveType(table, node->data.arrayLiteral.elements[0]); // Assuming all elements are of the same type
            } else {
                return DATA_TYPE_UNKNOWN;
            }

        case NODE_IF_STATEMENT:
        case NODE_WHILE_STATEMENT:
        case NODE_FOR_STATEMENT:
        case NODE_RETURN_STATEMENT:
            // These nodes do not have a direct data type
            return DATA_TYPE_VOID;

        case NODE_FUNCTION_DECLARATION:
            return node->data.functionDecl.function->returnType;

        case NODE_IMPORT_STATEMENT:
            // Import statements typically do not have a type
            return DATA_TYPE_UNKNOWN;

        case NODE_EXTERN_STATEMENT:
            // Handle extern statements by resolving the type of the extern declaration
            if (node->data.externNode.decl.function) {
                return node->data.externNode.decl.function->returnType;
            }
            return DATA_TYPE_UNKNOWN;

        case NODE_EXTERN_FUNCTION:
            // Extern function declarations should resolve to the function's return type
            return node->data.externNode.decl.function->returnType;

        case NODE_ARG_LIST:
            // Argument lists do not have a single data type
            return DATA_TYPE_UNKNOWN;

        case NODE_PARAM_LIST:
            // Parameter lists do not have a single data type
            return DATA_TYPE_UNKNOWN;

        default:
            return DATA_TYPE_UNKNOWN;
    }
}
// </resolveType>



// Recursive traversal of AST
// <traverseAST>
void traverseAST(ASTNode* node, CryoSymbolTable* table) {
    while (node) {
        switch (node->type) {
            case NODE_PROGRAM:
                for (int i = 0; i < node->data.program.stmtCount; i++) {
                    traverseAST(node->data.program.statements[i], table);
                }
                printf("\n#------- Symbol Table -------#\n");
                printSymbolTable(table);
                printf("\n#--------- End Table --------#\n");
                //exitScope(table);
                break;

            case NODE_VAR_DECLARATION:
                addSymbol(
                    table, 
                    node->data.varDecl.name, 
                    node->type, 
                    resolveType(table, node->data.varDecl.initializer),
                    false, 
                    0, 
                    node->data.varDecl.dataType
                );
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
                CryoSymbol* funcSymbol = findSymbol(table, node->data.functionCall.name);
                if (!funcSymbol) {
                    printf("[SymTable - Error] Function '%s' not declared\n", node->data.functionCall.name);
                    break;
                }

                // Check argument types
                for (int i = 0; i < node->data.functionCall.argCount; i++) {
                    CryoDataType* expectedType = funcSymbol->paramTypes[i];
                    CryoDataType actualType = resolveType(table, node->data.functionCall.args[i]);
                    if (expectedType != actualType) {
                        printf("[SymTable - Error] Argument %d to function '%s' expected type '%s', got '%s'\n",
                               i + 1, node->data.functionCall.name, CryoDataTypeToString((CryoDataType)expectedType), CryoDataTypeToString(actualType));
                    }
                }
                break;

            case NODE_FUNCTION_DECLARATION:
                printf("[SymTable] Adding function '%s'\n", node->data.functionDecl.function->name);
                addSymbol(
                    table, 
                    node->data.functionDecl.function->name, 
                    NODE_FUNCTION_DECLARATION,
                    node->data.functionDecl.function->returnType,
                    false, 
                    node->data.functionDecl.function->paramCount, 
                    DATA_TYPE_UNKNOWN
                );

                // Enter function scope and add parameters
                enterScope(table);
                for (int i = 0; i < node->data.functionDecl.function->paramCount; ++i) {
                    ASTNode* param = node->data.functionDecl.function->params[i];
                    printf("[SymTable] Adding parameter '%s'\n", param->data.varDecl.name);
                    addSymbol(
                        table, 
                        param->data.varDecl.name, 
                        NODE_PARAM_LIST,
                        param->data.varDecl.dataType,
                        false, 
                        0, 
                        DATA_TYPE_UNKNOWN
                    );
                }

                // Traverse function body
                traverseAST(node->data.functionDecl.function->body, table);
                exitScope(table);
                break;

            case NODE_FUNCTION_BLOCK:
                printf("[SymTable] Entering function block scope\n");
                enterScope(table);
                for (int i = 0; i < node->data.functionBlock.block->data.block.stmtCount; i++) {
                    traverseAST(node->data.functionBlock.block->data.block.statements[i], table);
                }
                //exitScope(table);
                break;

            case NODE_BLOCK:
                printf("[SymTable] Entering block scope\n");
                enterScope(table);
                for (int i = 0; i < node->data.block.stmtCount; i++) {
                    traverseAST(node->data.block.statements[i], table);
                }
                //exitScope(table);
                break;

            case NODE_IF_STATEMENT:
                printf("[SymTable] Checking if statement\n");
                traverseAST(node->data.ifStmt.condition, table);
                traverseAST(node->data.ifStmt.thenBranch, table);
                if (node->data.ifStmt.elseBranch) {
                    traverseAST(node->data.ifStmt.elseBranch, table);
                }
                break;

            case NODE_WHILE_STATEMENT:
                printf("[SymTable] Checking while statement\n");
                traverseAST(node->data.whileStmt.condition, table);
                traverseAST(node->data.whileStmt.body, table);
                break;

            case NODE_FOR_STATEMENT:
                printf("[SymTable] Checking for statement\n");
                traverseAST(node->data.forStmt.initializer, table);
                traverseAST(node->data.forStmt.condition, table);
                traverseAST(node->data.forStmt.increment, table);
                traverseAST(node->data.forStmt.body, table);
                break;

            case NODE_RETURN_STATEMENT:
                printf("[SymTable] Checking return statement\n");
                traverseAST(node->data.returnStmt.expression, table);
                break;

            case NODE_LITERAL_EXPR:
                // No action needed for literals
                break;

            case NODE_BINARY_EXPR:
                traverseAST(node->data.bin_op.left, table);
                traverseAST(node->data.bin_op.right, table);
                break;

            case NODE_UNARY_EXPR:
                traverseAST(node->data.unary_op.operand, table);
                break;

            case NODE_STRING_LITERAL:
                // No action needed for string literals
                break;

            case NODE_BOOLEAN_LITERAL:
                // No action needed for boolean literals
                break;

            case NODE_ARRAY_LITERAL:
                for (int i = 0; i < node->data.arrayLiteral.elementCount; i++) {
                    traverseAST(node->data.arrayLiteral.elements[i], table);
                }
                break;

            case NODE_IMPORT_STATEMENT:
                printf("[SymTable] Handling import statement for module '%s'\n", node->data.importStatementNode.modulePath);
                // Add logic to handle the import statement if necessary
                break;

            case NODE_EXTERN_STATEMENT:
                printf("[SymTable] Handling extern statement\n");
                // Traverse the extern node's declaration if it's a function
                if (node->data.externNode.decl.function) {
                    traverseAST((ASTNode*)node->data.externNode.decl.function, table);
                }
                break;

        case NODE_EXTERN_FUNCTION:
            printf("[SymTable] Adding extern function '%s'\n", node->data.externNode.decl.function->name);
            addSymbol(
                table, 
                node->data.externNode.decl.function->name, 
                NODE_EXTERN_FUNCTION,
                node->data.externNode.decl.function->returnType,
                false, 
                node->data.externNode.decl.function->paramCount, 
                DATA_TYPE_UNKNOWN
            );

            // Add parameters to the symbol table
            for (int i = 0; i < node->data.externNode.decl.function->paramCount; ++i) {
                ASTNode* param = node->data.externNode.decl.function->params[i];
                printf("[SymTable] Adding parameter '%s'\n", param->data.varDecl.name);
                addSymbol(
                    table, 
                    param->data.varDecl.name, 
                    NODE_PARAM_LIST,
                    param->data.varDecl.dataType,
                    false, 
                    0, 
                    DATA_TYPE_UNKNOWN
                );
            }
            break;

            case NODE_ARG_LIST:
                for (int i = 0; i < node->data.argList.argCount; i++) {
                    traverseAST(node->data.argList.args[i], table);
                }
                break;

            case NODE_UNKNOWN:
                printf("[SymTable] Unknown node type\n");
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
bool analyzeNode(ASTNode* node, CryoSymbolTable *table) {
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