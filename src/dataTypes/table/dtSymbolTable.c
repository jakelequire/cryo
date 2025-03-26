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
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Symbol Table Implementation --------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMSymbolTable_getEntry(DTMSymbolTable *table, const char *scopeName, const char *name)
{
    for (int i = 0; i < table->entryCount; i++)
    {
        if (strcmp(table->entries[i]->name, name) == 0)
        {
            logMessage(LMI, "INFO", "DTM", "Found entry in symbol table: %s", name);
            return table->entries[i]->type;
        }
    }

    return NULL;
}

void DTMSymbolTable_addEntry(DTMSymbolTable *table, const char *scopeName, const char *name, DataType *type)
{
    logMessage(LMI, "INFO", "DTM", "Adding entry to symbol table: %s", name);
    if (table->entryCount >= table->entryCapacity)
    {
        table->resizeTable(table);
    }

    DTMSymbolTableEntry *entry = createDTMSymbolTableEntry(scopeName, name, type);
    table->entries[table->entryCount++] = entry;
}

void DTMSymbolTable_removeEntry(DTMSymbolTable *table, const char *scopeName, const char *name)
{
    for (int i = 0; i < table->entryCount; i++)
    {
        if (strcmp(table->entries[i]->name, name) == 0)
        {
            free(table->entries[i]);
            table->entries[i] = NULL;
            table->entryCount--;
            break;
        }
    }
}

void DTMSymbolTable_updateEntry(DTMSymbolTable *table, const char *scopeName, const char *name, DataType *type)
{
    for (int i = 0; i < table->entryCount; i++)
    {
        if (strcmp(table->entries[i]->name, name) == 0)
        {
            table->entries[i]->type = type;
            break;
        }
    }
}

void DTMSymbolTable_resizeTable(DTMSymbolTable *table)
{
    logMessage(LMI, "INFO", "DTM", "Resizing symbol table...");
    table->entryCapacity *= 2;
    table->entries = (DTMSymbolTableEntry **)realloc(table->entries, sizeof(DTMSymbolTableEntry *) * table->entryCapacity);
    if (!table->entries)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTM Symbol Table\n");
        CONDITION_FAILED;
    }
}

void DTMSymbolTable_printTable(DTMSymbolTable *table)
{
    printf("\n");
    printf("========================================================================================================================\n");
    printf("|                                              DTM Symbol Table Entries                                                |\n");
    printf("========================================================================================================================\n");
    printf("| %-20s | %-33s | %-18s | %-15s | %-8s | %-8s|\n", "Name", "Type", "Scope", "TypeOf", "Const", "Pointer");
    printf("========================================================================================================================\n");

    for (int i = 0; i < table->entryCount; i++)
    {
        DataType *type = table->entries[i]->type;
        bool isConst = type->isConst;
        bool isReference = type->isReference;
        bool isPointer = type->isPointer;

        const char *name = table->entries[i]->name;
        const char *scopeName = table->entries[i]->scopeName;
        const char *typeOfStr = DTM->debug->typeofDataTypeToString(type->container->typeOf);

        if (!name)
        {
            fprintf(stderr, "[Data Type Manager] Error: Symbol table entry has no name\n");
            continue;
        }

        const char *typeStr = DTM->debug->dataTypeToString(type);
        if (!typeStr)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to get string representation of data type\n");
            continue;
        }

        printf("| %-20s | %-33s | %-18s | %-15s | %-8s | %-8s|\n",
               name,
               typeStr,
               scopeName,
               typeOfStr,
               isConst ? "true" : "false",
               isPointer ? "true" : "false");
    }

    printf("========================================================================================================================\n");
    printf("\n");
}

void DTMSymbolTable_importASTNode(DTMSymbolTable *table, ASTNode *rootNode)
{
    if (!rootNode)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to import NULL root node\n");
        CONDITION_FAILED;
    }

    void (*self)(DTMSymbolTable *, ASTNode *) = DTMSymbolTable_importASTNode;

    CryoNodeType nodeType = rootNode->metaData->type;
    switch (nodeType)
    {
    case NODE_PROGRAM:
    {
        CryoProgram *program = rootNode->data.program;
        for (int i = 0; i < program->statementCount; i++)
        {
            ASTNode *statement = program->statements[i];
            self(table, statement);
        }
        break;
    }
    case NODE_FUNCTION_DECLARATION:
    {
        FunctionDeclNode *function = rootNode->data.functionDecl;
        DataType *returnType = DTM->getTypeofASTNode(rootNode);
        table->addEntry(table, rootNode->metaData->moduleName, function->name, returnType);

        self(table, function->body);
        break;
    }
    case NODE_BLOCK:
    {
        CryoBlockNode *block = rootNode->data.block;
        for (int i = 0; i < block->statementCount; i++)
        {
            ASTNode *statement = block->statements[i];
            self(table, statement);
        }
        break;
    }
    case NODE_FUNCTION_BLOCK:
    {
        CryoFunctionBlock *functionBlock = rootNode->data.functionBlock;
        for (int i = 0; i < functionBlock->statementCount; i++)
        {
            ASTNode *statement = functionBlock->statements[i];
            self(table, statement);
        }
        break;
    }
    case NODE_TYPE:
    {
        TypeDecl *typeDecl = rootNode->data.typeDecl;
        DataType *typeOfDecl = rootNode->data.typeDecl->type;
        table->addEntry(table, rootNode->metaData->moduleName, typeDecl->name, typeOfDecl);
        break;
    }
    case NODE_STRUCT_DECLARATION:
    {
        StructNode *structDecl = rootNode->data.structNode;
        DataType *structType = DTM->getTypeofASTNode(rootNode);
        table->addEntry(table, rootNode->metaData->moduleName, structDecl->name, structType);
        break;
    }
    case NODE_CLASS:
    {
        ClassNode *classDecl = rootNode->data.classNode;
        DataType *classType = DTM->getTypeofASTNode(rootNode);
        table->addEntry(table, rootNode->metaData->moduleName, classDecl->name, classType);
        break;
    }
    case NODE_VAR_DECLARATION:
        break;
    case NODE_STATEMENT:
        break;
    case NODE_EXPRESSION:
        break;
    case NODE_BINARY_EXPR:
        break;
    case NODE_UNARY_EXPR:
        break;
    case NODE_LITERAL_EXPR:
        break;
    case NODE_VAR_NAME:
        break;
    case NODE_FUNCTION_CALL:
        break;
    case NODE_IF_STATEMENT:
        break;
    case NODE_WHILE_STATEMENT:
        break;
    case NODE_FOR_STATEMENT:
        break;
    case NODE_RETURN_STATEMENT:
        break;
    case NODE_EXPRESSION_STATEMENT:
        break;
    case NODE_ASSIGN:
        break;
    case NODE_PARAM_LIST:
        break;
    case NODE_PARAM:
        break;
    case NODE_STRING_LITERAL:
        break;
    case NODE_STRING_EXPRESSION:
        break;
    case NODE_BOOLEAN_LITERAL:
        break;
    case NODE_ARRAY_LITERAL:
        break;
    case NODE_IMPORT_STATEMENT:
        break;
    case NODE_EXTERN_STATEMENT:
        break;
    case NODE_EXTERN_FUNCTION:
        break;
    case NODE_ARG_LIST:
        break;
    case NODE_NAMESPACE:
        break;
    case NODE_INDEX_EXPR:
        break;
    case NODE_VAR_REASSIGN:
        break;
    case NODE_PROPERTY:
        break;
    case NODE_CUSTOM_TYPE:
        break;
    case NODE_SCOPED_FUNCTION_CALL:
        break;
    case NODE_EXTERNAL_SYMBOL:
        break;
    case NODE_STRUCT_CONSTRUCTOR:
        break;
    case NODE_PROPERTY_ACCESS:
        break;
    case NODE_THIS:
        break;
    case NODE_THIS_ASSIGNMENT:
        break;
    case NODE_PROPERTY_REASSIGN:
        break;
    case NODE_METHOD:
        break;
    case NODE_IDENTIFIER:
        break;
    case NODE_METHOD_CALL:
        break;
    case NODE_ENUM:
        break;
    case NODE_GENERIC_DECL:
        break;
    case NODE_GENERIC_INST:
        break;
    case NODE_CLASS_CONSTRUCTOR:
        break;
    case NODE_OBJECT_INST:
        break;
    case NODE_NULL_LITERAL:
        break;
    case NODE_TYPEOF:
        break;
    case NODE_USING:
        break;
    case NODE_MODULE:
        break;
    case NODE_ANNOTATION:
        break;
    case NODE_TYPE_CAST:
        break;
    case NODE_UNKNOWN:
        break;
    default:
    {
        fprintf(stderr, "[Data Type Manager] Error: Unknown node type: %d\n", nodeType);
        CONDITION_FAILED;
    }
    }
}

DTMSymbolTableEntry *createDTMSymbolTableEntry(const char *scopeName, const char *name, DataType *type)
{
    DTMSymbolTableEntry *entry = (DTMSymbolTableEntry *)malloc(sizeof(DTMSymbolTableEntry));
    if (!entry)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Symbol Table Entry\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    entry->name = name;
    entry->type = type;
    entry->scopeName = scopeName;

    return entry;
}

DTMSymbolTable *createDTMSymbolTable(void)
{
    DTMSymbolTable *symbolTable = (DTMSymbolTable *)malloc(sizeof(DTMSymbolTable));
    if (!symbolTable)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Symbol Table\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    symbolTable->entries = (DTMSymbolTableEntry **)malloc(sizeof(DTMSymbolTableEntry *) * SYMBOL_TABLE_INITIAL_CAPACITY);
    if (!symbolTable->entries)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Symbol Table Entries\n");
        CONDITION_FAILED;
    }

    symbolTable->entryCount = 0;
    symbolTable->entryCapacity = SYMBOL_TABLE_INITIAL_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    symbolTable->getEntry = DTMSymbolTable_getEntry;
    symbolTable->addEntry = DTMSymbolTable_addEntry;
    symbolTable->removeEntry = DTMSymbolTable_removeEntry;
    symbolTable->updateEntry = DTMSymbolTable_updateEntry;
    symbolTable->resizeTable = DTMSymbolTable_resizeTable;
    symbolTable->printTable = DTMSymbolTable_printTable;

    symbolTable->createEntry = createDTMSymbolTableEntry;
    symbolTable->importASTnode = DTMSymbolTable_importASTNode;

    return symbolTable;
}