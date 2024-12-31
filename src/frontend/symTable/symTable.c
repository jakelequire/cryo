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
#include "frontend/symTable.h"

// <createSymbolTable>
CryoSymbolTable *createSymbolTable(Arena *arena)
{
    CryoSymbolTable *table = (CryoSymbolTable *)ARENA_ALLOC(arena, sizeof(CryoSymbolTable));
    table->count = 0;
    table->capacity = 512;
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
void printSymbolTable(CryoSymbolTable *table)
{
    printf("\n\n-----------------------------------------------------------------------------------------\n");
    printf("[SymTable] Symbol count: %d\n", table->count);
    printf("[SymTable] Scope depth: %d\n", table->scopeDepth);
    printf("[SymTable] Table capacity: %d\n", table->capacity);
    printf("\n\n");
    printf("Namespace: %s\n", table->namespaceName ? table->namespaceName : "Unnamed");
    printf("\n");
    printf("Name                 Node Type               Data Type             Args     Module\n");
    printf("----------------------------------------------------------------------------------------\n");
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
        if (table->symbols[i]->nodeType == NODE_PARAM)
        {
            // Skip printing parameters, comment out the `continue` to print them
            continue;
        }

        char locationStr[16];

        // Create the "L:C" string
        snprintf(locationStr, sizeof(locationStr), "%d:%d",
                 table->symbols[i]->line,
                 table->symbols[i]->column);

        printf("%-20s %-24s %-21s %-7d %-15s\n",
               table->symbols[i]->name ? table->symbols[i]->name : "Unnamed",
               CryoNodeTypeToString(table->symbols[i]->nodeType),
               DataTypeToStringUnformatted(table->symbols[i]->type),
               table->symbols[i]->argCount,
               table->symbols[i]->module ? table->symbols[i]->module : "Unnamed");
    }
    printf("----------------------------------------------------------------------------------------\n");
}
// </printSymbolTable>

void printSymbolTableCXX(CryoSymbolTable *table)
{
    printSymbolTable(table);
}

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

    char *name = getNameOfNode(node);
    if (!name || strlen(name) == 0)
    {
        fprintf(stderr, "Error: name is null in addASTNodeSymbol\n");
        return;
    }

    if (isSymbolInTable(table, name))
    {
        updateExistingSymbol(table, node, arena);
        return;
    }

    CryoSymbol *symbolNode = createCryoSymbol(table, node, arena);
    if (!symbolNode)
    {
        logMessage(LMI, "ERROR", "SymTable", "Failed to create symbol node");
        return;
    }

    if (table->count >= table->capacity)
    {
        table->capacity *= 2;
        table->symbols = (CryoSymbol **)realloc(table->symbols, table->capacity * sizeof(CryoSymbol *));
        if (!table->symbols)
        {
            logMessage(LMI, "ERROR", "SymTable", "Failed to reallocate memory for symbol table");
            return;
        }
    }

    table->symbols[table->count++] = symbolNode;
    logMessage(LMI, "INFO", "SymTable", "Symbol added: %s", strdup(symbolNode->name));
}
// </addASTNodeSymbol>

// <isSymbolInTable>
bool isSymbolInTable(CryoSymbolTable *table, char *name)
{
    if (!name)
    {
        logMessage(LMI, "ERROR", "SymTable", "Name is null in isSymbolInTable");
        return false;
    }

    for (int i = 0; i < table->count; i++)
    {
        if (strcmp(table->symbols[i]->name, name) == 0)
        {
            return true;
        }
    }
    return false;
}
// </isSymbolInTable>

// <updateExistingSymbol>
void updateExistingSymbol(CryoSymbolTable *table, ASTNode *node, Arena *arena)
{
    if (!node)
    {
        logMessage(LMI, "ERROR", "SymTable", "Node is null in updateExistingSymbol");
        return;
    }

    char *name = getNameOfNode(node);
    if (!name || strlen(name) == 0)
    {
        logMessage(LMI, "ERROR", "SymTable", "Name is null in updateExistingSymbol");
        return;
    }

    for (int i = table->count - 1; i >= 0; i--)
    {
        if (strcmp(table->symbols[i]->name, name) == 0)
        {
            logMessage(LMI, "INFO", "SymTable", "Updating existing symbol: %s", name);
            table->symbols[i] = createCryoSymbol(table, node, arena);
            return;
        }
    }
}
// </updateExistingSymbol>

// <addDefinitionToSymbol>
void addDefinitionToSymbolTable(CryoSymbolTable *table, ASTNode *node, Arena *arena)
{
    if (!node)
    {
        logMessage(LMI, "ERROR", "SymTable", "Node is null in addDefinitionToSymbolTable");
        return;
    }

    char *name = getNameOfNode(node);
    if (!name || strlen(name) == 0)
    {
        logMessage(LMI, "ERROR", "SymTable", "Name is null in addDefinitionToSymbolTable");
        return;
    }

    for (int i = table->count - 1; i >= 0; i--)
    {
        if (strcmp(table->symbols[i]->name, name) == 0)
        {
            table->symbols[i] = createCryoSymbol(table, node, arena);
            return;
        }
    }
}
// </addDefinitionToSymbol>

// <getNameOfNode> *KEEP*
char *getNameOfNode(ASTNode *node)
{
    if (!node)
    {
        logMessage(LMI, "ERROR", "SymTable", "Node is null in getNameOfNode");
        return NULL;
    }

    switch (node->metaData->type)
    {
    case NODE_NAMESPACE:
        return strdup(node->data.cryoNamespace->name);
    case NODE_VAR_DECLARATION:
        return strdup(node->data.varDecl->name);
    case NODE_FUNCTION_DECLARATION:
        return strdup(node->data.functionDecl->name);
    case NODE_EXTERN_FUNCTION:
        return strdup(node->data.externFunction->name);
    case NODE_PARAM_LIST:
        return strdup(node->data.varDecl->name);
    case NODE_PARAM:
        return strdup(node->data.param->name);
    case NODE_VAR_NAME:
        return strdup(node->data.varName->varName);
    case NODE_FUNCTION_CALL:
        return strdup(node->data.functionCall->name);
    case NODE_ARRAY_LITERAL:
        return strdup(node->data.varDecl->name);
    case NODE_VAR_REASSIGN:
        return strdup(node->data.varReassignment->existingVarName);
    case NODE_RETURN_STATEMENT:
        break;
    case NODE_STRUCT_DECLARATION:
        return strdup(node->data.structNode->name);
    case NODE_IMPORT_STATEMENT:
        return strdup(node->data.import->moduleName);
    case NODE_PROPERTY:
        return strdup(node->data.property->name);
    case NODE_PROPERTY_ACCESS:
        return strdup(node->data.propertyAccess->propertyName);
    case NODE_METHOD:
        return strdup(node->data.method->name);
    case NODE_CLASS:
        return strdup(node->data.classNode->name);

    default:
        logMessage(LMI, "ERROR", "SymTable", "Unsupported node type %s", CryoNodeTypeToString(node->metaData->type));
        return NULL;
    }
    return NULL;
}
// </getNameOfNode>

// <resolveNodeSymbol>
CryoSymbol *createCryoSymbol(CryoSymbolTable *table, ASTNode *node, Arena *arena)
{
    if (!node)
    {
        logMessage(LMI, "ERROR", "SymTable", "Node is null in createCryoSymbol");
        return NULL;
    }

    CryoSymbol *symbolNode = (CryoSymbol *)ARENA_ALLOC(arena, sizeof(CryoSymbol));
    if (!symbolNode)
    {
        logMessage(LMI, "ERROR", "SymTable", "Failed to allocate memory for symbolNode");
        return NULL;
    }

    symbolNode->node = node;
    symbolNode->name = NULL;
    symbolNode->nodeType = NODE_UNKNOWN;
    symbolNode->type = createUnknownType();
    symbolNode->scopeLevel = table->scopeDepth;
    symbolNode->isConstant = false;
    symbolNode->argCount = 0;
    symbolNode->line = node->metaData->position.line;
    symbolNode->column = node->metaData->position.column;

    switch (node->metaData->type)
    {
    case NODE_NAMESPACE:
        symbolNode->name = strdup(node->data.cryoNamespace->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_VAR_DECLARATION:
    {
        bool isMutable = node->data.varDecl->isMutable;
        symbolNode->name = strdup(node->data.varDecl->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->type = node->data.varDecl->type;
        symbolNode->isConstant = !isMutable;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;
    }

    case NODE_FUNCTION_DECLARATION:
        symbolNode->name = strdup(node->data.functionDecl->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->type = node->data.functionDecl->type;
        symbolNode->argCount = node->data.functionDecl->paramCount;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_EXTERN_FUNCTION:
        symbolNode->name = strdup(node->data.externFunction->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->type = node->data.externFunction->type;
        symbolNode->argCount = node->data.externFunction->paramCount;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_PARAM_LIST:
        symbolNode->name = strdup(node->data.varDecl->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->type = node->data.varDecl->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_VAR_NAME:
        symbolNode->name = strdup(node->data.varName->varName);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->type = node->data.varName->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_FUNCTION_CALL:
        symbolNode->name = strdup(node->data.functionCall->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->argCount = node->data.functionCall->argCount;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_ARRAY_LITERAL:
        symbolNode->name = strdup(node->data.varDecl->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->type = node->data.varDecl->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_VAR_REASSIGN:
        symbolNode->name = strdup(node->data.varReassignment->existingVarName);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->type = node->data.varReassignment->existingVarType;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_PARAM:
        symbolNode->name = strdup(node->data.param->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->type = node->data.param->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_STRUCT_DECLARATION:
        symbolNode->name = strdup(node->data.structNode->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->type = node->data.structNode->type;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_PROPERTY:
        symbolNode->name = strdup(node->data.property->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->type = node->data.property->type;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_RETURN_STATEMENT:
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_IMPORT_STATEMENT:
        symbolNode->name = strdup(node->data.import->moduleName);
        if (node->data.import->subModuleName)
        {
            symbolNode->module = strdup(node->data.import->subModuleName);
            symbolNode->name = (char *)realloc(symbolNode->name, strlen(symbolNode->name) + strlen(strdup(node->data.import->subModuleName)) + 2);
            strcat(symbolNode->name, "::");
            strcat(symbolNode->name, strdup(node->data.import->subModuleName));
        }
        symbolNode->nodeType = node->metaData->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;

    case NODE_PROPERTY_ACCESS:
    {
        symbolNode->name = strdup(node->data.propertyAccess->propertyName);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;
    }

    case NODE_METHOD:
    {
        symbolNode->name = strdup(node->data.method->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->type = node->data.method->type;
        symbolNode->argCount = node->data.method->paramCount;
        symbolNode->module = strdup(node->data.method->parentName);
        break;
    }

    case NODE_ENUM:
    {
        symbolNode->name = strdup(node->data.enumNode->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->module = strdup(node->metaData->moduleName);
        break;
    }

    case NODE_CLASS:
    {
        symbolNode->name = strdup(node->data.classNode->name);
        symbolNode->nodeType = node->metaData->type;
        symbolNode->line = node->metaData->position.line;
        symbolNode->column = node->metaData->position.column;
        symbolNode->type = node->data.classNode->type;
        symbolNode->module = strdup(node->metaData->moduleName);

        break;
    }

    default:
        logMessage(LMI, "ERROR", "SymTable", "Unsupported node type %d", node->metaData->type);
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

// <findImportedSymbol>
CryoSymbol *findImportedSymbol(CryoSymbolTable *table, const char *name, const char *module, Arena *arena)
{
    for (int i = table->count - 1; i >= 0; i--)
    {
        if (strcmp(table->symbols[i]->name, name) == 0 && strcmp(table->symbols[i]->module, module) == 0)
        {
            return table->symbols[i];
        }
    }
    return NULL;
}
// </findImportedSymbol>

// <getCurrentNamespace>
char *getCurrentNamespace(CryoSymbolTable *table)
{
    for (int i = table->count - 1; i >= 0; i--)
    {
        if (table->symbols[i]->node->metaData->type == NODE_NAMESPACE)
        {
            return table->symbols[i]->name;
        }
    }
    return NULL;
}
// </getCurrentNamespace>

void importAstTreeDefs(ASTNode *root, CryoSymbolTable *table, Arena *arena, CompilerState *state)
{
    if (!root)
    {
        logMessage(LMI, "ERROR", "SymTable", "Root is null in importAstTree");
        return;
    }

    logMessage(LMI, "INFO", "SymTable", "Importing AST tree definitions...");
    printAST(root, 0, arena);

    for (int i = 0; i < root->data.program->statementCount; i++)
    {
        if (root->data.program->statements[i]->metaData->type == NODE_NAMESPACE)
        {
        }
        else
        {
            logMessage(LMI, "INFO", "SymTable", "Adding import statement to symbol table");
            addASTNodeSymbol(table, root->data.program->statements[i], arena);
        }
    }

    return;
}

void setNamespace(CryoSymbolTable *table, const char *name)
{
    if (table->namespaceName == NULL)
        table->namespaceName = strdup(name);
}

void importRuntimeDefinitionsToSymTable(CryoSymbolTable *table, ASTNode *runtimeNode, Arena *arena)
{
    for (int i = 0; i < runtimeNode->data.program->statementCount; i++)
    {
        ASTNode *node = runtimeNode->data.program->statements[i];
        if (node->metaData->type == NODE_CLASS)
        {
            addClassMethodsToTable(node, table, arena);

            logMessage(LMI, "INFO", "SymTable", "Adding runtime class definition to symbol table %s%s%s%s",
                       BOLD, GREEN, node->data.classNode->name, COLOR_RESET);

            continue;
        }
        addASTNodeSymbol(table, node, arena);
        logMessage(LMI, "INFO", "SymTable", "Adding runtime definition to symbol table %s%s%s%s",
                   BOLD, GREEN, CryoNodeTypeToString(node->metaData->type), COLOR_RESET);
    }
}

void addClassMethodsToTable(ASTNode *classNode, CryoSymbolTable *table, Arena *arena)
{
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "SymTable", "Expected class declaration node.");
        return;
    }

    // Add the class itself to the symbol table
    addASTNodeSymbol(table, classNode, arena);

    // Iterate over the public/private/protected methods and add them to the symbol table
    ClassNode *classData = classNode->data.classNode;
    for (int i = 0; i < classData->publicMembers->methodCount; i++)
    {
        addASTNodeSymbol(table, classData->publicMembers->methods[i], arena);
    }

    for (int i = 0; i < classData->privateMembers->methodCount; i++)
    {
        addASTNodeSymbol(table, classData->privateMembers->methods[i], arena);
    }

    for (int i = 0; i < classData->protectedMembers->methodCount; i++)
    {
        addASTNodeSymbol(table, classData->protectedMembers->methods[i], arena);
    }

    return;
}

CryoSymbol *resolveModuleSymbol(const char *moduleName, const char *symbolName, CryoSymbolTable *table, Arena *arena)
{
    for (int i = 0; i < table->count; i++)
    {
        if (strcmp(table->symbols[i]->name, symbolName) == 0 && strcmp(table->symbols[i]->module, moduleName) == 0)
        {
            return table->symbols[i];
        }
    }

    printSymbolTable(table);
    logMessage(LMI, "ERROR", "SymTable", "Failed to resolve module symbol %s%s%s%s in module %s%s%s%s",
               BOLD, RED, symbolName, COLOR_RESET, BOLD, RED, moduleName, COLOR_RESET);
    return NULL;
}
