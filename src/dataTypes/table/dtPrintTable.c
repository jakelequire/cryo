/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "tools/logger/logger_config.h"
#include "dataTypes/dataTypeManager.h"

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

    // Check if there is any entries in the snapshot
    int snapshotEntryCount = 0;
    if (table->snapshot && table->snapshot->entryCount > 0)
    {
        // Count valid snapshot entries
        for (int i = 0; i < table->snapshot->entryCount; i++)
        {
            if (table->snapshot->entries[i] && table->snapshot->entries[i]->type &&
                table->snapshot->entries[i]->type->container)
            {
                snapshotEntryCount++;
            }
        }
        entryCount += snapshotEntryCount;
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

    // Process main table entries
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

    // Process snapshot entries
    if (table->snapshot)
    {
        for (int i = 0; i < table->snapshot->entryCount; i++)
        {
            if (!table->snapshot->entries[i] || !table->snapshot->entries[i]->type ||
                !table->snapshot->entries[i]->type->container)
            {
                continue;
            }

            DataType *type = table->snapshot->entries[i]->type;

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

    // ===== PRINT MAIN TABLE ENTRIES =====
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
        if (type->isPrototype)
            strcat(flags, "T");
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
                nameColor = LIGHT_GREEN;
                typeColor = GREEN;
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

        if (type->container->typeOf == OBJECT_TYPE && type->container->objectType == CLASS_OBJ)
        {
            int propCount = 0;
            int methodCount = 0;

            // Safe property extraction
            if (type->container->type.classType)
            {
                propCount = type->container->type.classType->propertyCount;
                methodCount = type->container->type.classType->methodCount;
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

    // ===== PRINT SNAPSHOT ENTRIES =====
    if (table->snapshot && table->snapshot->entryCount > 0)
    {
        // Optional: Add a section header for snapshot entries
        printf("%s%s╟", BOLD, CYAN);
        for (int i = 0; i < tableWidth - 2; i++)
            printf("─");
        printf("╢%s\n", COLOR_RESET);

        printf("%s%s║ %s%-*s %s║%s\n",
               BOLD, CYAN, YELLOW,
               tableWidth - 4, "SNAPSHOT ENTRIES",
               CYAN, COLOR_RESET);

        printf("%s%s╟", BOLD, CYAN);
        for (int i = 0; i < tableWidth - 2; i++)
            printf("─");
        printf("╢%s\n", COLOR_RESET);

        for (int i = 0; i < table->snapshot->entryCount; i++)
        {
            // Skip invalid entries entirely
            if (!table->snapshot->entries[i] || !table->snapshot->entries[i]->type ||
                !table->snapshot->entries[i]->type->container)
            {
                continue;
            }

            // Safe extraction of entry data with defaults
            DataType *type = table->snapshot->entries[i]->type;
            const char *name = table->snapshot->entries[i]->name ? table->snapshot->entries[i]->name : "<unnamed>";
            const char *scope = table->snapshot->entries[i]->scopeName ? table->snapshot->entries[i]->scopeName : "global";

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

            if (type->container->typeOf == OBJECT_TYPE && type->container->objectType == CLASS_OBJ)
            {
                int propCount = 0;
                int methodCount = 0;

                // Safe property extraction
                if (type->container->type.classType)
                {
                    propCount = type->container->type.classType->propertyCount;
                    methodCount = type->container->type.classType->methodCount;
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
    }

    // ===== BOTTOM BORDER =====
    printf("%s%s╚", BOLD, CYAN);
    for (int i = 0; i < tableWidth - 2; i++)
        printf("═");
    printf("╝%s\n\n", COLOR_RESET);
}
