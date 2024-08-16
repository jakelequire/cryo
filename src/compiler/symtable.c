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
#include <string.h>

// <logSymCryoDataType>
char *logSymCryoDataType(CryoDataType type)
{
    switch (type)
    {
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
CryoSymbolTable *createSymbolTable()
{
    CryoSymbolTable *table = (CryoSymbolTable *)malloc(sizeof(CryoSymbolTable));
    table->count = 0;
    table->capacity = 10;
    table->symbols = (CryoSymbol **)malloc(table->capacity * sizeof(CryoSymbol *));
    table->scopeDepth = 0;
    return table;
}
// </createSymbolTable>

// <freeSymbolTable>
void freeSymbolTable(CryoSymbolTable *table)
{
    for (int i = 0; i < table->count; i++)
    {
        free(table->symbols[i]);
    }
    free(table->symbols);
    free(table);
}
// </freeSymbolTable>

// <printSymbolTable>
void printSymbolTable(CryoSymbolTable *table)
{
    printf("[SymTable - Debug] Symbol count: %d\n", table->count);
    printf("[SymTable - Debug] Scope depth: %d\n", table->scopeDepth);
    printf("[SymTable - Debug] Table capacity: %d\n", table->capacity);
    printf("\n-------------------------------------------------------------------------------------------------\n");
    printf("Symbol Table:\n\n");
    printf("Name                 Type                 Val/RetType          Scope        Const       ArgCount\n");
    printf("\n-------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < table->count; i++)
    {
        if (table->symbols[i] == NULL)
        {
            printf("Error: symbol at index %d is null\n", i);
            continue;
        }
        if (table->symbols[i]->node == NULL)
        {
            printf("Error: node in symbol at index %d is null\n", i);
            continue;
        }
        printf("%-20s %-24s %-18s %-10d %-14s %-10d\n",
               table->symbols[i]->name ? table->symbols[i]->name : "Unnamed",
               CryoNodeTypeToString(table->symbols[i]->nodeType),
               CryoDataTypeToString(table->symbols[i]->valueType),
               table->symbols[i]->scopeLevel,
               table->symbols[i]->isConstant ? "true" : "false",
               table->symbols[i]->argCount);
    }
    printf("-------------------------------------------------------------------------------------------------\n");
}
// </printSymbolTable>

// <enterScope>
void enterScope(CryoSymbolTable *table)
{
    table->scopeDepth++;
}
// </enterScope>

// <jumpScope>
void jumpScope(CryoSymbolTable *table)
{
    while (table->count > 0 && table->symbols[table->count - 1]->scopeLevel >= table->scopeDepth)
    {
        table->count--;
    }
    table->scopeDepth--;
}
// </jumpScope>

// <exitScope>
void exitScope(CryoSymbolTable *table)
{
    while (table->count > 0 && table->symbols[table->count - 1]->scopeLevel == table->scopeDepth)
    {
        free(table->symbols[table->count - 1]->name);
        free(table->symbols[table->count - 1]);
        table->count--;
    }
    table->scopeDepth--;
}
// </exitScope>

// <enterBlockScope>
void enterBlockScope(CryoSymbolTable *table)
{
    enterScope(table);
}
// </enterBlockScope>

// <exitBlockScope>
void exitBlockScope(CryoSymbolTable *table)
{
    exitScope(table);
}
// </exitBlockScope>

// <addASTNodeSymbol>
void addASTNodeSymbol(CryoSymbolTable *table, ASTNode *node)
{
    if (!node)
    {
        fprintf(stderr, "Error: node is null in addASTNodeSymbol\n");
        return;
    }
    // printf("DEBUG [SymTable] Adding symbol Type: %s\n", CryoNodeTypeToString(node->metaData->type));

    CryoSymbol *symbolNode = createCryoSymbol(table, node);
    if (!symbolNode)
    {
        fprintf(stderr, "Error: Failed to create symbolNode in addASTNodeSymbol\n");
        return;
    }

    if (table->count >= table->capacity)
    {
        table->capacity *= 2;
        table->symbols = (CryoSymbol **)realloc(table->symbols, table->capacity * sizeof(CryoSymbol *));
        if (!table->symbols)
        {
            fprintf(stderr, "Error: Failed to reallocate memory for symbols table\n");
            return;
        }
    }

    table->symbols[table->count++] = symbolNode;
    fprintf(stdout, "Symbol added: %s\n", symbolNode->name ? symbolNode->name : "Unnamed Symbol");
}
// </addASTNodeSymbol>

// <resolveNodeSymbol>
CryoSymbol *createCryoSymbol(CryoSymbolTable *table, ASTNode *node)
{
    if (!node)
    {
        fprintf(stderr, "Error: node is null in createCryoSymbol\n");
        return NULL;
    }

    CryoSymbol *symbolNode = (CryoSymbol *)malloc(sizeof(CryoSymbol) * 2);
    if (!symbolNode)
    {
        fprintf(stderr, "Error: Failed to allocate memory for symbolNode\n");
        return NULL;
    }

    symbolNode->node = node;
    symbolNode->name = NULL;
    symbolNode->nodeType = DATA_TYPE_UNKNOWN;
    symbolNode->valueType = DATA_TYPE_UNKNOWN;
    symbolNode->scopeLevel = table->scopeDepth;
    symbolNode->isConstant = false;
    symbolNode->argCount = 0;

    switch (node->metaData->type)
    {
    case NODE_NAMESPACE:
        symbolNode->name = strdup(node->metaData->moduleName);
        symbolNode->nodeType = node->metaData->type;
        break;

    case NODE_VAR_DECLARATION:
    {
        printf("[Symtable] Variable Name: %s\n", node->data.varDecl->name);
        char *var_name = strdup(node->data.varDecl->name);
        symbolNode->name = var_name;
        symbolNode->nodeType = node->metaData->type;
        symbolNode->valueType = node->data.varDecl->type;
        break;
    }

    case NODE_FUNCTION_DECLARATION:
        symbolNode->name = strdup(node->data.functionDecl->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->valueType = node->data.functionDecl->returnType;
        symbolNode->argCount = node->data.functionDecl->paramCount;
        break;

    case NODE_EXTERN_FUNCTION:
        symbolNode->name = strdup(node->data.externFunction->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->valueType = node->data.externFunction->returnType;
        symbolNode->argCount = node->data.externFunction->paramCount;
        break;

    case NODE_PARAM_LIST:
        symbolNode->name = strdup(node->data.varDecl->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->valueType = node->data.varDecl->type;
        break;

    case NODE_VAR_NAME:
        symbolNode->name = strdup(node->data.varName->varName);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->valueType = node->data.varName->refType;
        break;

    case NODE_FUNCTION_CALL:
        symbolNode->name = strdup(node->data.functionCall->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->argCount = node->data.functionCall->argCount;
        break;

    case NODE_ARRAY_LITERAL:
        symbolNode->name = strdup(node->data.varDecl->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->valueType = node->data.varDecl->type;
        break;

    default:
        fprintf(stderr, "Error: Unsupported node type %d\n", node->metaData->type);
        error("Unsupported node type", "createCryoSymbol", table);
        free(symbolNode);
        return NULL;
    }

    return symbolNode;
}

// <findSymbol>
CryoSymbol *findSymbol(CryoSymbolTable *table, const char *name)
{
    for (int i = table->count - 1; i >= 0; i--)
    {
        if (strcmp(table->symbols[i]->name, name) == 0)
        {
            return table->symbols[i];
        }
    }
    return NULL;
}
// </findSymbol>

// Main Entry Point
// <analyzeNode>
bool analyzeNode(ASTNode *node, CryoSymbolTable *table)
{
    // traverseAST(node, table);

    // Cleanup
    for (int i = 0; i < table->count; i++)
    {
        free(table->symbols[i]->name);
        free(table->symbols[i]);
    }
    free(table->symbols);
    free(table);
    return true;
}
// </analyzeNode>
