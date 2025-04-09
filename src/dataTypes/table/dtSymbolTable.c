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
#include "diagnostics/diagnostics.h"
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Symbol Table Implementation --------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTMSymbolTable_printEntry(DTMSymbolTableEntry *entry)
{
    if (!entry)
    {
        fprintf(stderr, "[Data Type Manager] Error: Symbol table entry is NULL\n");
        return;
    }

    printf("| %-20s | %-33s | %-18s | %-15s | %-8s | %-8s|\n", entry->name, entry->type->typeName, entry->scopeName,
           DTM->debug->typeofDataTypeToString(entry->type->container->typeOf),
           entry->type->isConst ? "true" : "false", entry->type->isPointer ? "true" : "false");
}

void DTMSymbolTable_addPrototype(
    DTMSymbolTable *table, const char *scopeName, const char *name,
    PrimitiveDataType primitive, TypeofDataType typeOf, TypeofObjectType objectType)
{
    DataType *protoType = DTM->dataTypes->createProtoType(name, primitive, typeOf, objectType);
    DTM->symbolTable->addEntry(DTM->symbolTable, scopeName, name, protoType);

    logMessage(LMI, "INFO", "DTM", "Added prototype to symbol table: %s", name);
}

DataType *DTMSymbolTable_getEntry(DTMSymbolTable *table, const char *scopeName, const char *name)
{
    logMessage(LMI, "INFO", "DTM", "Looking up entry in symbol table: %s", name);
    table->printTable(table);
    for (int i = 0; i < table->entryCount; i++)
    {
        if (strcmp(table->entries[i]->name, name) == 0)
        {
            logMessage(LMI, "INFO", "DTM", "Found entry in symbol table: %s", name);
            return table->entries[i]->type;
        }
    }

    // Check if the entry is in the snapshot cache
    for (int i = 0; i < table->snapshot->entryCount; i++)
    {
        if (strcmp(table->snapshot->entries[i]->name, name) == 0)
        {
            logMessage(LMI, "INFO", "DTM", "Found entry in symbol table snapshot: %s", name);
            return table->snapshot->entries[i]->type;
        }
    }

    logMessage(LMI, "INFO", "DTM", "Failed to find entry in symbol table: %s", name);
    DTM->symbolTable->printTable(DTM->symbolTable);
    return NULL;
}

void DTMSymbolTable_addEntry(DTMSymbolTable *table, const char *scopeName, const char *name, DataType *type)
{
    logMessage(LMI, "INFO", "DTM", "Adding entry to symbol table: %s", name);
    if (!table || !name || !type)
    {
        logMessage(LMI, "ERROR", "DTM", "Invalid arguments for adding entry to symbol table");
        return;
    }

    if (table->entryCount >= table->entryCapacity)
    {
        table->resizeTable(table);
    }

    // Check to see if the entry already exists
    for (int i = 0; i < table->entryCount; i++)
    {
        if (table->entries[i] && table->entries[i]->name &&
            strcmp(table->entries[i]->name, name) == 0)
        {

            // Check if types are compatible (same typeOf and typeOfObject)
            DataType *existingType = table->entries[i]->type;
            if (existingType && existingType->container && type->container)
            {

                // Check if the basic type categories match
                if (existingType->container->typeOf == type->container->typeOf)
                {

                    // For object types, we need to check objectType as well
                    if (existingType->container->typeOf != OBJECT_TYPE ||
                        existingType->container->objectType == type->container->objectType)
                    {

                        logMessage(LMI, "INFO", "DTM", "Updating existing entry in symbol table: %s", name);

                        // Create and set the new entry
                        DTMSymbolTableEntry *entry = createDTMSymbolTableEntry(scopeName, name, type);

                        // Free the old entry to prevent memory leaks
                        table->entries[i] = entry;
                        return;
                    }
                    else
                    {
                        logMessage(LMI, "WARN", "DTM",
                                   "Cannot update entry '%s': Object types differ (%s vs %s)",
                                   name,
                                   DTM->debug->typeofObjectTypeToString(existingType->container->objectType),
                                   DTM->debug->typeofObjectTypeToString(type->container->objectType));
                        DTM->symbolTable->printTable(DTM->symbolTable);
                        CONDITION_FAILED;
                    }
                }
                else
                {
                    logMessage(LMI, "WARN", "DTM",
                               "Cannot update entry '%s': Type categories differ (%s vs %s)",
                               name,
                               DTM->debug->typeofDataTypeToString(existingType->container->typeOf),
                               DTM->debug->typeofDataTypeToString(type->container->typeOf));
                    DTM->symbolTable->printTable(DTM->symbolTable);

                    CONDITION_FAILED;
                }
            }

            // If we reach here, types are incompatible, so we don't update
            logMessage(LMI, "WARN", "DTM", "Skipping add for '%s': An entry with this name exists but has incompatible type", name);
            CONDITION_FAILED;
            return;
        }
    }

    // If we reach here, it's a new entry
    DTMSymbolTableEntry *entry = createDTMSymbolTableEntry(scopeName, name, type);
    table->entries[table->entryCount] = entry;
    table->entryCount++;

    logMessage(LMI, "INFO", "DTM", "Added new entry to symbol table: %s", name);
}

void DTMSymbolTable_resetEntries(DTMSymbolTable *table)
{
    for (int i = 0; i < table->entryCount; i++)
    {
        free(table->entries[i]);
        table->entries[i] = NULL;
    }
    free(table->entries);
    table->entries = NULL;
    table->entryCount = 0;
    table->entries = (DTMSymbolTableEntry **)malloc(sizeof(DTMSymbolTableEntry *) * table->entryCapacity);
    if (!table->entries)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to reset DTM Symbol Table\n");
        CONDITION_FAILED;
    }

    logMessage(LMI, "INFO", "DTM", "Reset symbol table entries");
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
    logMessage(LMI, "INFO", "DTM", "Removed entry from symbol table: %s", name);
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
    if (table->entryCount >= table->entryCapacity)
    {
        table->entryCapacity *= 2;
        table->entries = (DTMSymbolTableEntry **)realloc(table->entries, sizeof(DTMSymbolTableEntry *) * table->entryCapacity);
        if (!table->entries)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTM Symbol Table\n");
            CONDITION_FAILED;
        }
    }
    else
    {
        logMessage(LMI, "INFO", "DTM", "No need to resize symbol table. Current capacity: %d, current count: %d", table->entryCapacity, table->entryCount);
        return;
    }
}

void DTMSymbolTable_importASTNode(DTMSymbolTable *table, ASTNode *rootNode)
{
    if (!rootNode)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to import NULL root node\n");
        CONDITION_FAILED;
    }

    CryoNodeType nodeType = rootNode->metaData->type;
    char *nodeTypeStr = CryoNodeTypeToString(nodeType);
    printf("%s%s[Data Type Manager]%s Importing AST node: %s\n", BOLD, CYAN, COLOR_RESET, nodeTypeStr);
    switch (nodeType)
    {
    case NODE_PROGRAM:
    {
        CryoProgram *program = rootNode->data.program;
        for (int i = 0; i < program->statementCount; i++)
        {
            ASTNode *statement = program->statements[i];
            DTMSymbolTable_importASTNode(table, statement);
        }
        break;
    }
    case NODE_FUNCTION_DECLARATION:
    {
        FunctionDeclNode *function = rootNode->data.functionDecl;
        DataType *returnType = DTM->getTypeofASTNode(rootNode);
        DTM->symbolTable->addEntry(table, rootNode->metaData->moduleName, function->name, returnType);

        DTMSymbolTable_importASTNode(table, function->body);
        break;
    }
    case NODE_BLOCK:
    {
        CryoBlockNode *block = rootNode->data.block;
        for (int i = 0; i < block->statementCount; i++)
        {
            ASTNode *statement = block->statements[i];
            DTMSymbolTable_importASTNode(table, statement);
        }
        break;
    }
    case NODE_FUNCTION_BLOCK:
    {
        CryoFunctionBlock *functionBlock = rootNode->data.functionBlock;
        for (int i = 0; i < functionBlock->statementCount; i++)
        {
            if (functionBlock->statementCount == 0)
            {
                return;
            }
            ASTNode *statement = functionBlock->statements[i];
            DTMSymbolTable_importASTNode(table, statement);
        }
        break;
    }
    case NODE_TYPE:
    {
        TypeDecl *typeDecl = rootNode->data.typeDecl;
        DataType *typeOfDecl = rootNode->data.typeDecl->type;
        DTM->symbolTable->addEntry(table, rootNode->metaData->moduleName, typeDecl->name, typeOfDecl);
        break;
    }
    case NODE_STRUCT_DECLARATION:
    {
        StructNode *structDecl = rootNode->data.structNode;
        DataType *structType = DTM->getTypeofASTNode(rootNode);
        DTM->symbolTable->addEntry(table, rootNode->metaData->moduleName, structDecl->name, structType);
        break;
    }
    case NODE_CLASS:
    {
        ClassNode *classDecl = rootNode->data.classNode;
        DataType *classType = DTM->getTypeofASTNode(rootNode);
        DTM->symbolTable->addEntry(table, rootNode->metaData->moduleName, classDecl->name, classType);
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
        const char *nodeTypeStr = CryoNodeTypeToString(nodeType);
        fprintf(stderr, "[Data Type Manager] Error: Unknown node type: %s\n", nodeTypeStr);
        CONDITION_FAILED;
    }
    }

    return;
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

DataType *DTMSymbolTable_lookup(DTMSymbolTable *table, const char *name)
{
    // check if the name is a primitive type

    // Check if the name is a primitive type
    DataType *primitive = DTMParsePrimitive(name);
    if (primitive)
    {
        return primitive;
    }

    for (int i = 0; i < table->entryCount; i++)
    {
        if (strcmp(table->entries[i]->name, name) == 0)
        {
            return table->entries[i]->type;
        }
    }

    // Check if the name is in the snapshot
    for (int i = 0; i < table->snapshot->entryCount; i++)
    {
        if (strcmp(table->snapshot->entries[i]->name, name) == 0)
        {
            return table->snapshot->entries[i]->type;
        }
    }

    // If we reach here, the name is not found
    logMessage(LMI, "ERROR", "DTM", "Failed to find entry in symbol table: %s", name);
    return NULL;
}

/**
 * @brief Create a snapshot of the current symbol table state
 *
 * Creates a deep copy of all entries in the symbol table for later comparison.
 *
 * @param symbolTable The symbol table to snapshot
 * @return EntrySnapshot* Pointer to the snapshot structure
 */
EntrySnapshot *createEntrySnapshot(DTMSymbolTable *symbolTable)
{
    __STACK_FRAME__

    if (symbolTable->snapshot->entryCount > 0)
    {
        return symbolTable->snapshot;
    }

    // Allocate the snapshot structure
    EntrySnapshot *snapshot = (EntrySnapshot *)malloc(sizeof(EntrySnapshot));
    if (!snapshot)
    {
        logMessage(LMI, "ERROR", "DTM", "Failed to allocate memory for symbol table snapshot");
        CONDITION_FAILED;
        return NULL;
    }

    // Set basic properties
    snapshot->entryCount = symbolTable->entryCount;
    snapshot->entryCapacity = symbolTable->entryCount; // Only allocate what we need

    // No entries to copy
    if (symbolTable->entryCount == 0)
    {
        snapshot->entries = NULL;
        return snapshot;
    }

    // Allocate memory for entry pointers
    snapshot->entries = (DTMSymbolTableEntry **)malloc(
        sizeof(DTMSymbolTableEntry *) * snapshot->entryCapacity);

    if (!snapshot->entries)
    {
        logMessage(LMI, "ERROR", "DTM", "Failed to allocate memory for snapshot entries");
        free(snapshot);
        CONDITION_FAILED;
        return NULL;
    }

    // Copy each entry
    for (int i = 0; i < symbolTable->entryCount; i++)
    {
        // Allocate a new entry
        DTMSymbolTableEntry *newEntry = createDTMSymbolTableEntry(
            symbolTable->entries[i]->scopeName,
            symbolTable->entries[i]->name,
            symbolTable->entries[i]->type);
        if (!newEntry)
        {
            logMessage(LMI, "ERROR", "DTM", "Failed to allocate memory for snapshot entry");
            freeEntrySnapshot(snapshot);
            CONDITION_FAILED;
            return NULL;
        }

        // Assign the new entry to the snapshot
        snapshot->entries[i] = newEntry;
    }

    symbolTable->snapshotInitialized = true;

    return snapshot;
}

/**
 * @brief Free memory used by a symbol table snapshot
 *
 * @param snapshot The snapshot to free
 */
void freeEntrySnapshot(EntrySnapshot *snapshot)
{
    __STACK_FRAME__

    if (!snapshot)
        return;

    if (snapshot->entries)
    {
        // Free each entry
        for (int i = 0; i < snapshot->entryCount; i++)
        {
            if (snapshot->entries[i])
            {
                // Free any other dynamically allocated fields
                free(snapshot->entries[i]);
            }
        }
        free(snapshot->entries);
    }

    free(snapshot);
}

/**
 * @brief Start a snapshot of the symbol table
 *
 * Captures the current state of the symbol table before changes are made.
 *
 * @param table The symbol table to snapshot
 */
void DTMSymbolTable_startSnapshot(DTMSymbolTable *table)
{
    __STACK_FRAME__
    logMessage(LMI, "WARN", "DTM", "%s%s<!> Starting symbol table snapshot <!>%s", BOLD, YELLOW, COLOR_RESET);
    // Clean up any existing snapshot first
    if (table->snapshot)
    {
        logMessage(LMI, "WARN", "DTM", "Overwriting existing symbol table snapshot");
        freeEntrySnapshot(table->snapshot);
        table->snapshot = NULL;
    }

    // Create the new snapshot

    table->snapshot = createEntrySnapshot(table);
    if (table->snapshot)
    {
        logMessage(LMI, "INFO", "DTM", "Created symbol table snapshot with %d entries",
                   table->snapshot->entryCount);
    }

    // Clear the `DTM->symbolTable->entries` array
    for (int i = 0; i < table->entryCount; i++)
    {
        if (table->entries[i])
        {
            free(table->entries[i]);
            table->entries[i] = NULL;
        }
    }
    table->entryCount = 0;
    table->entryCapacity = SYMBOL_TABLE_INITIAL_CAPACITY;
    table->entries = (DTMSymbolTableEntry **)malloc(sizeof(DTMSymbolTableEntry *) * SYMBOL_TABLE_INITIAL_CAPACITY);
    if (!table->entries)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Symbol Table\n");
        CONDITION_FAILED;
    }

    logMessage(LMI, "INFO", "DTM", "Cleared symbol table entries after snapshot");
    // Reset the entry count
}

/**
 * @brief Find new entries that were added since the snapshot was taken
 *
 * @param table The current symbol table
 * @return int Number of new entries found
 */
int findNewEntries(DTMSymbolTable *table)
{
    __STACK_FRAME__

    // Safety checks
    if (!table)
    {
        fprintf(stderr, "%s%s[Data Type Manager]%s Error: NULL symbol table pointer\n", BOLD, RED, COLOR_RESET);
        return 0;
    }

    // If there's no snapshot, we can't find new entries
    if (!table->snapshot)
    {
        fprintf(stderr, "%s%s[Data Type Manager]%s Error: No active snapshot\n", BOLD, RED, COLOR_RESET);
        return 0;
    }

    int newEntriesCount = 0;

    // Iterate through each entry in the main table
    for (int i = 0; i < table->entryCount; i++)
    {
        // Skip invalid entries
        if (!table->entries[i] || !table->entries[i]->type || !table->entries[i]->type->container)
        {
            continue;
        }

        const char *name = table->entries[i]->name ? table->entries[i]->name : "<unnamed>";
        const char *scope = table->entries[i]->scopeName ? table->entries[i]->scopeName : "global";

        // Flag to track if this entry is found in the snapshot
        bool foundInSnapshot = false;

        // Check if this entry exists in the snapshot
        for (int j = 0; j < table->snapshot->entryCount; j++)
        {
            // Skip invalid snapshot entries
            if (!table->snapshot->entries[j] || !table->snapshot->entries[j]->name || !table->snapshot->entries[j]->scopeName)
            {
                continue;
            }

            const char *snapshotName = table->snapshot->entries[j]->name;
            const char *snapshotScope = table->snapshot->entries[j]->scopeName;

            // Compare by name and scope
            if (strcmp(name, snapshotName) == 0 && strcmp(scope, snapshotScope) == 0)
            {
                foundInSnapshot = true;
                logMessage(LMI, "INFO", "DTM", "Found entry in snapshot: %s", name);
                break;
            }
            else
            {
                logMessage(LMI, "INFO", "DTM", "Entry not found in snapshot: %s", name);
                // Create a new entry in the snapshot
                DTMSymbolTableEntry *newEntry = table->createEntry(scope, name, table->entries[i]->type);
                if (newEntry)
                {
                    logMessage(LMI, "INFO", "DTM", "Adding new entry to snapshot: %s", name);
                    table->snapshot->entries[table->snapshot->entryCount++] = newEntry;
                    newEntriesCount++;
                }
            }
        }

        // If not found in snapshot, add it
        if (!foundInSnapshot)
        {
            // Check if we need to resize the snapshot
            if (table->snapshot->entryCount >= table->snapshot->entryCapacity)
            {
                // Calculate new capacity (e.g., double the current capacity)
                int newCapacity = table->snapshot->entryCapacity * 2;
                if (newCapacity == 0)
                {
                    newCapacity = 8; // Start with at least 8 entries
                }

                logMessage(LMI, "INFO", "DTM", "Resizing snapshot entries array from %d to %d", table->snapshot->entryCapacity, newCapacity);
                // Allocate new memory
                DTMSymbolTableEntry **newEntries = (DTMSymbolTableEntry **)realloc(
                    table->snapshot->entries,
                    newCapacity * sizeof(DTMSymbolTableEntry *));

                // Check if reallocation was successful
                if (!newEntries)
                {
                    fprintf(stderr, "%s%s[Data Type Manager]%s Error: Failed to resize snapshot entries array\n",
                            BOLD, RED, COLOR_RESET);
                    return newEntriesCount;
                }

                // Update snapshot with new array and capacity
                table->snapshot->entries = newEntries;
                table->snapshot->entryCapacity = newCapacity;
            }

            DTMSymbolTableEntry *newEntry = table->createEntry(scope, name, table->entries[i]->type);

            // Add the entry to the snapshot if creation was successful
            if (newEntry)
            {
                logMessage(LMI, "INFO", "DTM", "Adding new entry to snapshot: %s", name);
                table->snapshot->entries[table->snapshot->entryCount++] = newEntry;
                newEntriesCount++;
            }
        }
    }

    return newEntriesCount;
}

/**
 * @brief End the snapshot and process new entries
 *
 * Identifies entries that were added since the snapshot was taken,
 * processes them as needed, and cleans up the snapshot.
 *
 * @param table The symbol table
 */
void DTMSymbolTable_endSnapshot(DTMSymbolTable *table)
{
    __STACK_FRAME__
    logMessage(LMI, "WARN", "DTM", "%s%s<!> Ending symbol table snapshot <!>%s", BOLD, YELLOW, COLOR_RESET);
    if (!table->snapshot)
    {
        logMessage(LMI, "WARN", "DTM", "No snapshot to end");
        return;
    }

    // Find new entries since the snapshot
    int newEntryCount = findNewEntries(table);

    // `findNewEntries` will add any new types to the snapshot. We just need to reset the `entries` array.
    logMessage(LMI, "INFO", "DTM", "Found %d new entries since snapshot", newEntryCount);
    logMessage(LMI, "INFO", "DTM", "Symbol table snapshot processing complete");
}

/**
 * @brief Process a newly added symbol table entry
 *
 * Performs any necessary operations on a new entry identified after import.
 *
 * @param table The symbol table
 * @param entry The new entry to process
 */
void processNewEntry(DTMSymbolTable *table, DTMSymbolTableEntry *entry)
{
    __STACK_FRAME__

    // Depending on your needs, you might:
    // 1. Add the entry to an export list
    // 2. Mark it as available for external use
    // 3. Create references in parent scopes
    // 4. Add it to additional lookup tables
    // 5. Register it with a package manager

    // For now, just add it to the main imports table if it's a type or class
    if (entry->type->container->typeOf == OBJECT_TYPE)
    {
        // Example: Add to an imports registry
        addToImportsRegistry(entry->name, entry->scopeName);

        logMessage(LMI, "INFO", "DTM", "Registered import: %s", entry->name);
    }
}

/**
 * @brief Add an imported symbol to the imports registry
 *
 * @param name Symbol name
 * @param scope Symbol scope
 */
void addToImportsRegistry(const char *name, const char *scope)
{
    __STACK_FRAME__

    // This function would implement your registry logic
    // For example, it could add the import to a global imports table

    // Note: This is a placeholder - implement based on your needs
    logMessage(LMI, "INFO", "DTM", "Added %s from %s to imports registry", name, scope);
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

    symbolTable->snapshot = NULL;
    symbolTable->snapshotInitialized = false;

    // ==================== [ Function Assignments ] ==================== //

    symbolTable->getEntry = DTMSymbolTable_getEntry;
    symbolTable->addEntry = DTMSymbolTable_addEntry;
    symbolTable->resetEntries = DTMSymbolTable_resetEntries;
    symbolTable->addProtoType = DTMSymbolTable_addPrototype;
    symbolTable->removeEntry = DTMSymbolTable_removeEntry;
    symbolTable->updateEntry = DTMSymbolTable_updateEntry;
    symbolTable->resizeTable = DTMSymbolTable_resizeTable;
    symbolTable->printTable = DTMSymbolTable_printTable;

    symbolTable->createEntry = createDTMSymbolTableEntry;
    symbolTable->importASTnode = DTMSymbolTable_importASTNode;
    symbolTable->lookup = DTMSymbolTable_lookup;

    symbolTable->startSnapshot = DTMSymbolTable_startSnapshot;
    symbolTable->endSnapshot = DTMSymbolTable_endSnapshot;

    return symbolTable;
}
