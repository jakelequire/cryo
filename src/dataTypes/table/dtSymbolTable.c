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
    for (int i = 0; i < table->entryCount; i++)
    {
        if (strcmp(table->entries[i]->name, name) == 0)
        {
            logMessage(LMI, "INFO", "DTM", "Found entry in symbol table: %s", name);
            return table->entries[i]->type;
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
                        free(table->entries[i]);
                        table->entries[i] = entry;
                        return;
                    }
                    else
                    {
                        logMessage(LMI, "WARNING", "DTM",
                                   "Cannot update entry '%s': Object types differ (%s vs %s)",
                                   name,
                                   DTM->debug->typeofObjectTypeToString(existingType->container->objectType),
                                   DTM->debug->typeofObjectTypeToString(type->container->objectType));
                        CONDITION_FAILED;
                    }
                }
                else
                {
                    logMessage(LMI, "WARNING", "DTM",
                               "Cannot update entry '%s': Type categories differ (%s vs %s)",
                               name,
                               DTM->debug->typeofDataTypeToString(existingType->container->typeOf),
                               DTM->debug->typeofDataTypeToString(type->container->typeOf));
                    CONDITION_FAILED;
                }
            }

            // If we reach here, types are incompatible, so we don't update
            logMessage(LMI, "WARNING", "DTM", "Skipping add for '%s': An entry with this name exists but has incompatible type", name);
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
    // ===== Safety checks =====
    if (!table)
    {
        fprintf(stderr, "%s%s[Data Type Manager]%s Error: NULL symbol table pointer\n", BOLD, RED, COLOR_RESET);
        return;
    }

    // ===== Prepare data with extreme caution =====
    int entryCount = 0;
    // Count valid entries first
    for (int i = 0; i < table->entryCount; i++)
    {
        if (table->entries[i] && table->entries[i]->type && table->entries[i]->type->container)
        {
            entryCount++;
        }
    }

    if (entryCount == 0)
    {
        printf("\n%s%s[Data Type Manager]%s Symbol table contains no valid entries\n\n", BOLD, YELLOW, COLOR_RESET);
        return;
    }

    // ===== FIXED COLUMN WIDTHS - CRITICAL FOR ALIGNMENT =====
    const int tableWidth = 95;

    // Stats section
    const int statsColWidth = 22; // Each stats column width

    // Data section - must add up to tableWidth - 2 (borders) - 4 (separators)
    const int nameWidth = 16;   // Name column
    const int typeWidth = 28;   // Type column
    const int typeofWidth = 12; // TypeOf column
    const int scopeWidth = 16;  // Scope column
    const int flagsWidth = 7;   // Flags column

    // ===== Pre-compute statistics safely =====
    int primCount = 0;
    int structCount = 0;
    int funcCount = 0;
    int constCount = 0;
    int ptrCount = 0;
    int refCount = 0;

    for (int i = 0; i < table->entryCount; i++)
    {
        if (!table->entries[i] || !table->entries[i]->type || !table->entries[i]->type->container)
        {
            continue;
        }

        DataType *type = table->entries[i]->type;

        if (type->container->typeOf == PRIM_TYPE)
            primCount++;
        else if (type->container->typeOf == OBJECT_TYPE)
            structCount++;
        else if (type->container->typeOf == FUNCTION_TYPE)
            funcCount++;

        if (type->isConst)
            constCount++;
        if (type->isPointer)
            ptrCount++;
        if (type->isReference)
            refCount++;
    }

    // ===== Begin printing - TOP BORDER =====
    printf("\n");
    printf("%s%s╔", BOLD, CYAN);
    for (int i = 0; i < tableWidth - 2; i++)
        printf("═");
    printf("╗%s\n", COLOR_RESET);

    // ===== TITLE ROW - Fixed width title =====
    char titleStr[80];
    snprintf(titleStr, sizeof(titleStr), "DTM SYMBOL TABLE - %d %s",
             entryCount, (entryCount == 1 ? "entry" : "entries"));

    int tablePrintWidth = entryCount > 10 ? tableWidth - 4 : tableWidth - 3;

    printf("%s%s║ %s%s%-*s%s%s║%s\n",
           BOLD, CYAN, YELLOW, BOLD,
           tablePrintWidth, titleStr,
           COLOR_RESET, CYAN, COLOR_RESET);

    // ===== STATS SECTION DIVIDER =====
    printf("%s%s╠", BOLD, CYAN);
    for (int i = 0; i < statsColWidth; i++)
        printf("═");
    printf("╦");
    for (int i = 0; i < statsColWidth; i++)
        printf("═");
    printf("╦");
    for (int i = 0; i < statsColWidth; i++)
        printf("═");
    printf("╦");
    for (int i = 0; i < statsColWidth + 2; i++)
        printf("═");
    printf("╣%s\n", COLOR_RESET);

    // ===== STATS HEADERS - FIXED WIDTH =====
    printf("%s%s║%s %-*s%s%s║%s %-*s%s%s║%s %-*s%s%s║%s %-*s%s%s║%s\n",
           BOLD, CYAN, LIGHT_YELLOW,
           statsColWidth - 1, "Types:", BOLD, CYAN, LIGHT_CYAN,
           statsColWidth - 1, "Modifiers:", BOLD, CYAN, LIGHT_GREEN,
           statsColWidth - 1, "Scopes:", BOLD, CYAN, LIGHT_MAGENTA,
           statsColWidth + 1, "Stats:", BOLD, CYAN, COLOR_RESET);

    // ===== STATS ROWS - FIXED WIDTH =====
    char primStr[16], constStr[16], moduleStr[16], totalStr[16];
    snprintf(primStr, sizeof(primStr), "Primitive: %d", primCount);
    snprintf(constStr, sizeof(constStr), "Const: %d", constCount);
    snprintf(moduleStr, sizeof(moduleStr), "Module: %d", 0);
    snprintf(totalStr, sizeof(totalStr), "Total: %d", entryCount);

    printf("%s%s║%s %-*s%s%s║%s %-*s%s%s║%s %-*s%s%s║%s %-*s%s%s║%s\n",
           BOLD, CYAN, GREEN,
           statsColWidth - 1, primStr, BOLD, CYAN, LIGHT_YELLOW,
           statsColWidth - 1, constStr, BOLD, CYAN, LIGHT_GREEN,
           statsColWidth - 1, moduleStr, BOLD, CYAN, LIGHT_MAGENTA,
           statsColWidth + 1, totalStr, BOLD, CYAN, COLOR_RESET);

    char structStr[16], ptrStr[16], funcStr[16];
    snprintf(structStr, sizeof(structStr), "Struct: %d", structCount);
    snprintf(ptrStr, sizeof(ptrStr), "Pointer: %d", ptrCount);
    snprintf(funcStr, sizeof(funcStr), "Function: %d", 0);

    printf("%s%s║%s %-*s%s%s║%s %-*s%s%s║%s %-*s%s%s║%s %-*s%s%s║%s\n",
           BOLD, CYAN, YELLOW,
           statsColWidth - 1, structStr, BOLD, CYAN, LIGHT_YELLOW,
           statsColWidth - 1, ptrStr, BOLD, CYAN, LIGHT_GREEN,
           statsColWidth - 1, funcStr, BOLD, CYAN, WHITE,
           statsColWidth + 1, "", BOLD, CYAN, COLOR_RESET);

    char funcCountStr[16], refStr[16], globalStr[16];
    snprintf(funcCountStr, sizeof(funcCountStr), "Function: %d", funcCount);
    snprintf(refStr, sizeof(refStr), "Reference: %d", refCount);
    snprintf(globalStr, sizeof(globalStr), "Global: %d", 0);

    printf("%s%s║%s %-*s%s%s║%s %-*s%s%s║%s %-*s%s%s║%s %-*s%s%s║%s\n",
           BOLD, CYAN, BLUE,
           statsColWidth - 1, funcCountStr, BOLD, CYAN, LIGHT_YELLOW,
           statsColWidth - 1, refStr, BOLD, CYAN, LIGHT_GREEN,
           statsColWidth - 1, globalStr, BOLD, CYAN, WHITE,
           statsColWidth + 1, "", BOLD, CYAN, COLOR_RESET);

    // ===== DATA SECTION DIVIDER =====
    printf("%s%s╠", BOLD, CYAN);
    for (int i = 0; i < tableWidth - 2; i++)
        printf("═");
    printf("╣%s\n", COLOR_RESET);

    // ===== COLUMN HEADERS - FIXED WIDTH =====
    // Calculate vertical separators positions precisely
    int pos1 = nameWidth + 1;
    int pos2 = pos1 + typeWidth + 2;
    int pos3 = pos2 + typeofWidth + 2;
    int pos4 = pos3 + scopeWidth + 2;

    printf("%s%s║ %s%-*s %s| %s%-*s %s| %s%-*s %s| %s%-*s %s| %s%-*s %s║%s\n",
           BOLD, CYAN, LIGHT_GREEN,
           nameWidth, "Name", CYAN, YELLOW,
           typeWidth, "Type", CYAN, LIGHT_MAGENTA,
           typeofWidth, "TypeOf", CYAN, BLUE,
           scopeWidth, "Scope", CYAN, LIGHT_CYAN,
           flagsWidth, "Flags", CYAN, COLOR_RESET);

    // ===== DATA SECTION DIVIDER =====
    printf("%s%s╠", BOLD, CYAN);
    for (int i = 0; i < tableWidth - 2; i++)
        printf("═");
    printf("╣%s\n", COLOR_RESET);

    // ===== DATA ROWS =====
    int validEntries = 0;

    for (int i = 0; i < table->entryCount && validEntries < entryCount; i++)
    {
        // Skip invalid entries entirely
        if (!table->entries[i] || !table->entries[i]->type || !table->entries[i]->type->container)
        {
            continue;
        }

        validEntries++;

        // Safe extraction of entry data with defaults
        DataType *type = table->entries[i]->type;
        const char *name = table->entries[i]->name ? table->entries[i]->name : "<unnamed>";
        const char *scope = table->entries[i]->scopeName ? table->entries[i]->scopeName : "global";

        // Get type information safely
        const char *typeStr = "<unknown>";
        if (DTM && DTM->debug && DTM->debug->dataTypeToString)
        {
            const char *tmp = DTM->debug->dataTypeToString(type);
            if (tmp)
                typeStr = tmp;
        }

        // Get typeof information safely
        const char *typeofStr = "<unknown>";
        if (DTM && DTM->debug && DTM->debug->typeofDataTypeToString)
        {
            if (type->container->objectType == NON_OBJECT)
            {
                typeofStr = DTM->debug->typeofDataTypeToString(type->container->typeOf);
            }
            else
            {

                const char *tmp = DTM->debug->typeofObjectTypeToString(type->container->objectType);
                if (tmp)
                    typeofStr = tmp;
            }
        }

        // Build flags string
        char flags[16] = "";
        if (type->isConst)
            strcat(flags, "C");
        if (type->isPointer)
            strcat(flags, "P");
        if (type->isReference)
            strcat(flags, "R");
        if (flags[0] == '\0')
            strcpy(flags, "-");

        // Select colors based on type
        const char *nameColor = WHITE;
        const char *typeColor = WHITE;

        switch (type->container->typeOf)
        {
        case PRIM_TYPE:
            nameColor = LIGHT_GREEN;
            typeColor = GREEN;
            break;
        case FUNCTION_TYPE:
            nameColor = LIGHT_BLUE;
            typeColor = BLUE;
            break;
        case OBJECT_TYPE:
            if (type->container->objectType == STRUCT_OBJ)
            {
                nameColor = LIGHT_CYAN;
                typeColor = CYAN;
            }
            else
            {
                nameColor = LIGHT_MAGENTA;
                typeColor = MAGENTA;
            }
            break;
        default:
            nameColor = WHITE;
            typeColor = WHITE;
        }

        // Truncate each field to ensure alignment
        char nameField[nameWidth + 1];
        char typeField[typeWidth + 1];
        char typeofField[typeofWidth + 1];
        char scopeField[scopeWidth + 1];

        snprintf(nameField, sizeof(nameField), "%-*.*s", nameWidth, nameWidth, name);
        snprintf(typeField, sizeof(typeField), "%-*.*s", typeWidth, typeWidth, typeStr);
        snprintf(typeofField, sizeof(typeofField), "%-*.*s", typeofWidth, typeofWidth, typeofStr);
        snprintf(scopeField, sizeof(scopeField), "%-*.*s", scopeWidth, scopeWidth, scope);

        // Print a single row with fixed width fields and explicit column positions
        printf("%s%s║ %s%-*s %s| %s%-*s %s| %s%-*s %s| %s%-*s %s| %s%-*s %s║%s\n",
               BOLD, CYAN,
               nameColor, nameWidth, nameField, CYAN,
               typeColor, typeWidth, typeField, CYAN,
               LIGHT_MAGENTA, typeofWidth, typeofField, CYAN,
               BLUE, scopeWidth, scopeField, CYAN,
               LIGHT_CYAN, flagsWidth, flags, CYAN, COLOR_RESET);

        // For struct/class types, add an informational second row
        if (type->container->typeOf == OBJECT_TYPE && type->container->objectType == STRUCT_OBJ)
        {
            int propCount = 0;
            int methodCount = 0;

            // Safe property extraction
            if (type->container->type.structType)
            {
                propCount = type->container->type.structType->propertyCount;
                methodCount = type->container->type.structType->methodCount;
            }

            if (propCount > 0 || methodCount > 0)
            {
                char infoStr[typeWidth + 1];
                snprintf(infoStr, sizeof(infoStr), "↳ Props: %d, Methods: %d",
                         propCount, methodCount);
                // Truncate infoStr to fit within typeWidth
                infoStr[typeWidth] = '\0';

                // Create a properly aligned info row with empty columns for others
                printf("%s%s║ %s%-*s %s| %s%-*s %s| %s%-*s %s| %s%-*s %s| %s%-*s %s║%s\n",
                       BOLD, CYAN,
                       DARK_GRAY, nameWidth, "", CYAN,
                       DARK_GRAY, typeWidth + (sizeof(infoStr) - typeWidth) + 1, infoStr, CYAN,
                       DARK_GRAY, typeofWidth, "", CYAN,
                       DARK_GRAY, scopeWidth, "", CYAN,
                       DARK_GRAY, flagsWidth, "", CYAN, COLOR_RESET);
            }
        }
    }

    // ===== BOTTOM BORDER =====
    printf("%s%s╚", BOLD, CYAN);
    for (int i = 0; i < tableWidth - 2; i++)
        printf("═");
    printf("╝%s\n\n", COLOR_RESET);
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
        const char *nodeTypeStr = CryoNodeTypeToString(nodeType);
        fprintf(stderr, "[Data Type Manager] Error: Unknown node type: %s\n", nodeTypeStr);
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

DataType *DTMSymbolTable_lookup(DTMSymbolTable *table, const char *name)
{
    for (int i = 0; i < table->entryCount; i++)
    {
        if (strcmp(table->entries[i]->name, name) == 0)
        {
            return table->entries[i]->type;
        }
    }
    logMessage(LMI, "ERROR", "DTM", "Failed to find entry in symbol table: %s", name);
    return NULL;
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
    symbolTable->addProtoType = DTMSymbolTable_addPrototype;
    symbolTable->removeEntry = DTMSymbolTable_removeEntry;
    symbolTable->updateEntry = DTMSymbolTable_updateEntry;
    symbolTable->resizeTable = DTMSymbolTable_resizeTable;
    symbolTable->printTable = DTMSymbolTable_printTable;

    symbolTable->createEntry = createDTMSymbolTableEntry;
    symbolTable->importASTnode = DTMSymbolTable_importASTNode;
    symbolTable->lookup = DTMSymbolTable_lookup;

    return symbolTable;
}