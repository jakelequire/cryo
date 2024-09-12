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

// <createSymbolTable>
CryoSymbolTable *createSymbolTable(Arena *arena)
{
    CryoSymbolTable *table = (CryoSymbolTable *)ARENA_ALLOC(arena, sizeof(CryoSymbolTable));
    table->count = 0;
    table->capacity = 32;
    table->symbols = (CryoSymbol **)ARENA_ALLOC(arena, table->capacity * sizeof(CryoSymbol *));
    table->scopeDepth = 0;
    return table;
}
// </createSymbolTable>

// <freeSymbolTable>
void freeSymbolTable(CryoSymbolTable *table, Arena *arena)
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
void printSymbolTable(CryoSymbolTable *table, Arena *arena)
{
    printf("\n\n-------------------------------------------------------------------------------------------------\n");
    printf("[SymTable] Symbol count: %d\n", table->count);
    printf("[SymTable] Scope depth: %d\n", table->scopeDepth);
    printf("[SymTable] Table capacity: %d\n", table->capacity);
    printf("\n-------------------------------------------------------------------------------------------------\n");
    printf("Symbol Table:\n\n");
    printf("Name                 Type                 Val/RetType          Scope        Const       ArgCount\n");
    printf("-------------------------------------------------------------------------------------------------\n");
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
void enterScope(CryoSymbolTable *table, Arena *arena)
{
    table->scopeDepth++;
}
// </enterScope>

// <jumpScope>
void jumpScope(CryoSymbolTable *table, Arena *arena)
{
    while (table->count > 0 && table->symbols[table->count - 1]->scopeLevel >= table->scopeDepth)
    {
        table->count--;
    }
    table->scopeDepth--;
}
// </jumpScope>

// <exitScope>
void exitScope(CryoSymbolTable *table, Arena *arena)
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
void enterBlockScope(CryoSymbolTable *table, Arena *arena)
{
    enterScope(table, arena);
}
// </enterBlockScope>

// <exitBlockScope>
void exitBlockScope(CryoSymbolTable *table, Arena *arena)
{
    exitScope(table, arena);
}
// </exitBlockScope>

// <addASTNodeSymbol>
void addASTNodeSymbol(CryoSymbolTable *table, ASTNode *node, Arena *arena)
{
    if (!node)
    {
        fprintf(stderr, "Error: node is null in addASTNodeSymbol\n");
        return;
    }
    // printf("DEBUG [SymTable] Adding symbol Type: %s\n", CryoNodeTypeToString(node->metaData->type));

    CryoSymbol *symbolNode = createCryoSymbol(table, node, arena);
    if (!symbolNode)
    {
        logMessage("ERROR", __LINE__, "SymTable", "Failed to create symbol node");
        return;
    }

    if (table->count >= table->capacity)
    {
        table->capacity *= 2;
        table->symbols = (CryoSymbol **)realloc(table->symbols, table->capacity * sizeof(CryoSymbol *));
        if (!table->symbols)
        {
            logMessage("ERROR", __LINE__, "SymTable", "Failed to reallocate memory for symbol table");
            return;
        }
    }

    table->symbols[table->count++] = symbolNode;
    logMessage("INFO", __LINE__, "SymTable", "Symbol added: %s", strdup(symbolNode->name));
}
// </addASTNodeSymbol>

// <resolveNodeSymbol>
CryoSymbol *createCryoSymbol(CryoSymbolTable *table, ASTNode *node, Arena *arena)
{
    if (!node)
    {
        logMessage("ERROR", __LINE__, "SymTable", "Node is null in createCryoSymbol");
        return NULL;
    }

    CryoSymbol *symbolNode = (CryoSymbol *)ARENA_ALLOC(arena, sizeof(CryoSymbol));
    if (!symbolNode)
    {
        logMessage("ERROR", __LINE__, "SymTable", "Failed to allocate memory for symbolNode");
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
        symbolNode->name = strdup(node->data.cryoNamespace->name);
        symbolNode->nodeType = node->metaData->type;
        break;

    case NODE_VAR_DECLARATION:
    {
        symbolNode->name = strdup(node->data.varDecl->name);
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
        logMessage("ERROR", __LINE__, "SymTable", "Unsupported node type %d", node->metaData->type);
        error("Unsupported node type", "createCryoSymbol", table, arena);
        return NULL;
    }

    return symbolNode;
}

// <findSymbol>
CryoSymbol *findSymbol(CryoSymbolTable *table, const char *name, Arena *arena)
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
bool analyzeNode(ASTNode *node, CryoSymbolTable *table, Arena *arena)
{
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
