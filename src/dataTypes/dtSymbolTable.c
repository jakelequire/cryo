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
            return table->entries[i]->type;
        }
    }

    return NULL;
}

void DTMSymbolTable_addEntry(DTMSymbolTable *table, const char *scopeName, const char *name, DataType *type)
{
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
    printf("\n\n");
    printf("=================================================================\n");
    for (int i = 0; i < table->entryCount; i++)
    {
        DataType *type = table->entries[i]->type;
        printf("Name: %s, Type: %s\n", table->entries[i]->name, type->debug->toString(type));
    }
    printf("=================================================================\n");
}

void DTMSymbolTable_importRootNode(DTMSymbolTable *table, ASTNode *rootNode)
{
    if (!rootNode)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to import NULL root node\n");
        CONDITION_FAILED;
    }

    if (rootNode->metaData->type != NODE_PROGRAM)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to import non-program root node\n");
        CONDITION_FAILED;
    }

    void (*self)(DTMSymbolTable *, ASTNode *) = DTMSymbolTable_importRootNode;

    for (int i = 0; i < rootNode->data.program->statementCount; i++)
    {
        ASTNode *statement = rootNode->data.program->statements[i];
        // Iterate through the AST Tree (self)
        self(table, statement);
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
    symbolTable->importRootNode = DTMSymbolTable_importRootNode;

    return symbolTable;
}