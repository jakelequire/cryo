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
// ------------------------------------- DTM Debug Functions ----------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTMDebug_printDataType(DataType *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type\n");
        CONDITION_FAILED;
    }

    printf("Data Type: %s\n", type->typeName);
}

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

DataType *DTMFunctionTypes_createFunctionTemplate(void)
{
    DTFunctionTy *function = createDTFunctionTy();
    return DTM->dataTypes->wrapFunctionType(function);
}

DataType *DTMFunctionTypes_createFunctionType(DataType **paramTypes, int paramCount, DataType *returnType)
{
    DTFunctionTy *function = createDTFunctionTy();
    function->setParams(function, paramTypes, paramCount);
    function->setReturnType(function, returnType);
    return DTM->dataTypes->wrapFunctionType(function);
}

DTMFunctionTypes *createDTMFunctionTypes(void)
{
    DTMFunctionTypes *functionTypes = (DTMFunctionTypes *)malloc(sizeof(DTMFunctionTypes));
    if (!functionTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Function Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    functionTypes->createFunctionTemplate = DTMFunctionTypes_createFunctionTemplate;
    functionTypes->createFunctionType = DTMFunctionTypes_createFunctionType;

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
        DataType *type = table->entries[i]->type;
        printf("Name: %s, Type: %s\n", table->entries[i]->name, type->debug->toString(type));
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
// ---------------------------------- Data Types Implementation -------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

TypeContainer *DTMTypeContainerWrappers_createTypeContainer(void)
{
    TypeContainer *container = (TypeContainer *)malloc(sizeof(TypeContainer));
    if (!container)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to create Type Container\n");
        CONDITION_FAILED;
    }

    return container;
}

DataType *DTMTypeContainerWrappers_wrapTypeContainer(TypeContainer *container)
{
    DataType *type = (DataType *)malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to wrap Type Container\n");
        CONDITION_FAILED;
    }

    type->container = container;

    return type;
}

DataType *DTMTypeContainerWrappers_wrapSimpleType(struct DTSimpleTy_t *simpleTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->type.simpleType = simpleTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapArrayType(struct DTArrayTy_t *arrayTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = ARRAY_TYPE;
    container->type.arrayType = arrayTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapEnumType(struct DTEnumTy_t *enumTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = ENUM_TYPE;
    container->type.enumType = enumTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapFunctionType(struct DTFunctionTy_t *functionTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = FUNCTION_TYPE;
    container->type.functionType = functionTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapStructType(struct DTStructTy_t *structTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = OBJECT_TYPE;
    container->type.structType = structTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapClassType(struct DTClassTy_t *classTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = OBJECT_TYPE;
    container->type.classType = classTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapObjectType(struct DTObjectTy_t *objectTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = OBJECT_TYPE;
    container->type.objectType = objectTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapGenericType(struct DTGenericTy_t *genericTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = GENERIC_TYPE;
    container->type.genericType = genericTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DTMDataTypes *createDTMDataTypes(void)
{
    DTMDataTypes *dataTypes = (DTMDataTypes *)malloc(sizeof(DTMDataTypes));
    if (!dataTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Data Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    dataTypes->createTypeContainer = DTMTypeContainerWrappers_createTypeContainer;
    dataTypes->wrapTypeContainer = DTMTypeContainerWrappers_wrapTypeContainer;

    dataTypes->wrapSimpleType = DTMTypeContainerWrappers_wrapSimpleType;
    dataTypes->wrapArrayType = DTMTypeContainerWrappers_wrapArrayType;
    dataTypes->wrapEnumType = DTMTypeContainerWrappers_wrapEnumType;
    dataTypes->wrapFunctionType = DTMTypeContainerWrappers_wrapFunctionType;
    dataTypes->wrapStructType = DTMTypeContainerWrappers_wrapStructType;
    dataTypes->wrapClassType = DTMTypeContainerWrappers_wrapClassType;
    dataTypes->wrapObjectType = DTMTypeContainerWrappers_wrapObjectType;
    dataTypes->wrapGenericType = DTMTypeContainerWrappers_wrapGenericType;

    return dataTypes;
}

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- AST Interface Implementation ----------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMastInterface_getTypeofASTNode(ASTNode *node)
{

    return NULL;
}

DTMastInterface *createDTMAstInterface(void)
{
    DTMastInterface *astInterface = (DTMastInterface *)malloc(sizeof(DTMastInterface));
    if (!astInterface)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM AST Interface\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    astInterface->getTypeofASTNode = DTMastInterface_getTypeofASTNode;

    return astInterface;
}

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- Data Type Manager Implementation ------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void initGlobalDataTypeManagerInstance(void)
{
    globalDataTypeManager = createDataTypeManager();
    globalDataTypeManager->initialized = true;
}

void DataTypeManager_initDefinitions(const char *compilerRootPath, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    if (DTM->defsInitialized)
    {
        fprintf(stderr, "[Data Type Manager] Error: Definitions already initialized\n");
        CONDITION_FAILED;
        return;
    }

    const char *defsPath = fs->appendStrings(compilerRootPath, "/Std/Runtime/defs.cryo");
    if (!defsPath)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for definitions path\n");
        CONDITION_FAILED;
    }

    ASTNode *defsNode = compileForASTNode(defsPath, state, globalTable);
    if (!defsNode)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to compile definitions\n");
        CONDITION_FAILED;
    }

    defsNode->print(defsNode);

    DEBUG_BREAKPOINT;
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

    manager->initialized = false;
    manager->defsInitialized = false;

    manager->symbolTable = createDTMSymbolTable();
    manager->dataTypes = createDTMDataTypes();

    manager->primitives = createDTMPrimitives();
    manager->debug = createDTMDebug();
    manager->structTypes = createDTMStructTypes();
    manager->classTypes = createDTMClassTypes();
    manager->functionTypes = createDTMFunctionTypes();
    manager->generics = createDTMGenerics();
    manager->enums = createDTMEnums();

    manager->helpers = createDTMHelpers();

    // ===================== [ Function Assignments ] ===================== //

    manager->initDefinitions = DataTypeManager_initDefinitions;
    manager->parseType = DTMParseType;

    return manager;
}
