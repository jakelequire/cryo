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

// The global Data Type Manager instance. This is a static instance that is used to manage data types in the compiler.
// This is initialized in the `main` function of the compiler.
DataTypeManager *globalDataTypeManager = NULL;

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Primitive Data Types ---------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMPrimitives_createI8(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I8;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI16(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I16;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI32(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I32;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI64(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I64;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI128(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I128;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createInt(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_INT;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createFloat(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_FLOAT;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createString(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_STRING;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createBoolean(bool b)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_BOOLEAN;
    container->boolValue = b;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createVoid(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_VOID;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createNull(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_NULL;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createAny(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_ANY;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createUndefined(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_UNDEFINED;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createAutoTy(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_AUTO;

    return wrapTypeContainer(container);
}

DTMPrimitives *createDTMPrimitives(void)
{
    DTMPrimitives *primitives = (DTMPrimitives *)malloc(sizeof(DTMPrimitives));
    if (!primitives)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Primitives\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    primitives->createI8 = DTMPrimitives_createI8;
    primitives->createI16 = DTMPrimitives_createI16;
    primitives->createI32 = DTMPrimitives_createI32;
    primitives->createI64 = DTMPrimitives_createI64;
    primitives->createI128 = DTMPrimitives_createI128;
    primitives->createInt = DTMPrimitives_createInt;
    primitives->createFloat = DTMPrimitives_createFloat;
    primitives->createString = DTMPrimitives_createString;
    primitives->createBoolean = DTMPrimitives_createBoolean;
    primitives->createVoid = DTMPrimitives_createVoid;
    primitives->createNull = DTMPrimitives_createNull;
    primitives->createAny = DTMPrimitives_createAny;
    primitives->createUndefined = DTMPrimitives_createUndefined;
    primitives->createAutoTy = DTMPrimitives_createAutoTy;

    return primitives;
}

// --------------------------------------------------------------------------------------------------- //
// ----------------------------------- Debugging Functions ------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTMDebug_printDataType(DataType *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type\n");
        CONDITION_FAILED;
    }

    printf("Data Type: %s\n", type->container->custom.name);
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Struct Data Types ------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTMDebug *createDTMDebug(void)
{
    DTMDebug *debug = (DTMDebug *)malloc(sizeof(DTMDebug));
    if (!debug)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Debug\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    debug->printDataType = DTMDebug_printDataType;

    return debug;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Struct Data Types ------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTMStructTypes *createDTMStructTypes(void)
{
    DTMStructTypes *structTypes = (DTMStructTypes *)malloc(sizeof(DTMStructTypes));
    if (!structTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Struct Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return structTypes;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Class Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTMClassTypes *createDTMClassTypes(void)
{
    DTMClassTypes *classTypes = (DTMClassTypes *)malloc(sizeof(DTMClassTypes));
    if (!classTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Class Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return classTypes;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Function Data Types ----------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTMFunctionTypes *createDTMFunctionTypes(void)
{
    DTMFunctionTypes *functionTypes = (DTMFunctionTypes *)malloc(sizeof(DTMFunctionTypes));
    if (!functionTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Function Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return functionTypes;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Generic Data Types ------------------------------------------ //
// --------------------------------------------------------------------------------------------------- //

DTMGenerics *createDTMGenerics(void)
{
    DTMGenerics *generics = (DTMGenerics *)malloc(sizeof(DTMGenerics));
    if (!generics)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Generics\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return generics;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Enum Data Types --------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTMEnums *createDTMEnums(void)
{
    DTMEnums *enums = (DTMEnums *)malloc(sizeof(DTMEnums));
    if (!enums)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Enums\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return enums;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Symbol Table Implementation --------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMSymbolTable_getEntry(DTMSymbolTable *table, const char *name)
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

void DTMSymbolTable_addEntry(DTMSymbolTable *table, const char *name, DataType *type)
{
    if (table->entryCount >= table->entryCapacity)
    {
        table->resizeTable(table);
    }

    DTMSymbolTableEntry *entry = createDTMSymbolTableEntry(name, type);
    table->entries[table->entryCount++] = entry;
}

void DTMSymbolTable_removeEntry(DTMSymbolTable *table, const char *name)
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

void DTMSymbolTable_updateEntry(DTMSymbolTable *table, const char *name, DataType *type)
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
        printf("Name: %s, Type: %s\n", table->entries[i]->name, DataTypeToString(table->entries[i]->type));
    }
    printf("=================================================================\n");
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

    return symbolTable;
}

DTMSymbolTableEntry *createDTMSymbolTableEntry(const char *name, DataType *type)
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

    return entry;
}

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- Data Type Manager Implementation ------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void initGlobalDataTypeManagerInstance(void)
{
    globalDataTypeManager = createDataTypeManager();
}

void DataTypeManager_initDefinitions(void)
{
    return;
}

DataTypeManager *createDataTypeManager(void)
{
    DataTypeManager *manager = (DataTypeManager *)malloc(sizeof(DataTypeManager));
    if (!manager)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate Data Type Manager\n");
        CONDITION_FAILED;
    }

    // ===================== [ Property Assignments ] ===================== //

    manager->symbolTable = createDTMSymbolTable();
    manager->primitives = createDTMPrimitives();
    manager->debug = createDTMDebug();
    manager->structTypes = createDTMStructTypes();
    manager->classTypes = createDTMClassTypes();
    manager->functionTypes = createDTMFunctionTypes();
    manager->generics = createDTMGenerics();
    manager->enums = createDTMEnums();

    // ===================== [ Function Assignments ] ===================== //

    manager->initDefinitions = DataTypeManager_initDefinitions;

    return manager;
}
