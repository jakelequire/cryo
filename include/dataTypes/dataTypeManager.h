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
#ifndef DATA_TYPE_MANAGER_H
#define DATA_TYPE_MANAGER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>

#include "frontend/tokens.h"
#include "frontend/AST.h"
#include "dataTypes/dataTypeDefs.h"

#define SYMBOL_TABLE_INITIAL_CAPACITY 32

// ---------------------- Forward Declarations ---------------------- //

typedef struct DataTypeManager_t DataTypeManager;
typedef struct DataType DataType;
typedef struct CompilerState CompilerState;
typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;

extern DataTypeManager *globalDataTypeManager;
// The Global Data Type Manager
#define DTM globalDataTypeManager
#define INIT_DTM() initGlobalDataTypeManagerInstance();

// ----------------------------- Data Type Helpers ----------------------------- //

typedef struct DTMDynamicTypeArray_t
{
    DataType **data;
    int count;
    int capacity;

    void (*add)(struct DTMDynamicTypeArray_t *array, DataType *type);
    void (*remove)(struct DTMDynamicTypeArray_t *array, DataType *type);
    void (*resize)(struct DTMDynamicTypeArray_t *array);
    void (*reset)(struct DTMDynamicTypeArray_t *array);
    void (*free)(struct DTMDynamicTypeArray_t *array);
    void (*freeData)(struct DTMDynamicTypeArray_t *array);

    void (*printArray)(struct DTMDynamicTypeArray_t *array);
} DTMDynamicTypeArray;

typedef struct DTMDynamicTuple_t
{
    DataType **values;
    const char **keys;
    int count;
    int capacity;

    void (*add)(struct DTMDynamicTuple_t *tuple, const char *key, DataType *value);
    void (*remove)(struct DTMDynamicTuple_t *tuple, const char *key);
    void (*resize)(struct DTMDynamicTuple_t *tuple);
    void (*reset)(struct DTMDynamicTuple_t *tuple);
    void (*free)(struct DTMDynamicTuple_t *tuple);

    void (*printTuple)(struct DTMDynamicTuple_t *tuple);
} DTMDynamicTuple;

typedef struct DTMHelpers_t
{
    DTMDynamicTypeArray *dynTypeArray;
    DTMDynamicTuple *dynTuple;
} DTMHelpers;

// ----------------------- Primitive Data Type Interface ----------------------- //

// This structure is an interface for creating primitive data types in the compiler.
// This is used to create primitive data types such as `int`, `float`, `string`, `boolean`, `void`, etc.
typedef struct DTMPrimitives_t
{
    DataType *(*createI8)(void);        // type `i8` is an 8-bit integer
    DataType *(*createI16)(void);       // type `i16` is a 16-bit integer
    DataType *(*createI32)(void);       // type `i32` is a 32-bit integer
    DataType *(*createI64)(void);       // type `i64` is a 64-bit integer
    DataType *(*createI128)(void);      // type `i128` is a 128-bit integer
    DataType *(*createInt)(void);       // type `int` is a 32-bit integer
    DataType *(*createFloat)(void);     // type `float` is a 32-bit floating-point number
    DataType *(*createString)(void);    // type `string` is a string
    DataType *(*createBoolean)(void);   // type `boolean` is a boolean
    DataType *(*createVoid)(void);      // type `void` is a void type
    DataType *(*createNull)(void);      // type `null` is a null type
    DataType *(*createAny)(void);       // type `any` is equivalent to `void *`
    DataType *(*createUndefined)(void); // type `undefined` is an undefined type
    DataType *(*createAutoTy)(void);    // type `auto` is an auto type
} DTMPrimitives;

// ----------------------- Struct Data Type Interface ------------------------ //

// This structure is an interface for creating struct data types in the compiler.
// This is used to create struct data types such as `struct ... { ... }`.
typedef struct DTMStructTypes_t
{
    // TODO
} DTMStructTypes;

// ------------------------ Class Data Type Interface ------------------------ //

// This structure is an interface for creating class data types in the compiler.
// This is used to create class data types such as `class ... { ... }`.
typedef struct DTMClassTypes_t
{
    // TODO
} DTMClassTypes;

// ---------------------- Function Data Type Interface ---------------------- //

// This structure is an interface for creating function data types in the compiler.
// This is used to create function data types such as `function (...) -> ...`.
typedef struct DTMFunctionTypes_t
{
    DataType *(*createFunctionTemplate)(void);
    DataType *(*createFunctionType)(DataType **paramTypes, int paramCount, DataType *returnType);
} DTMFunctionTypes;

// ---------------------- Generics Data Type Interface ---------------------- //

// This structure is an interface for creating array data types in the compiler.
// This is used to create array data types such as `int[]`, `float[]`, `string[]`, `boolean[]`, etc.
typedef struct DTMGenerics_t
{
    // TODO
} DTMGenerics;

// ------------------------ Enum Data Type Interface ----------------------- //

// This structure is an interface for creating enum data types in the compiler.
// This is used to create enum data types such as `enum ... { ... }`.
typedef struct DTMEnums_t
{
    // TODO
} DTMEnums;

// ------------------------- Debugging Interfaces ------------------------- //

// This structure is an interface to debug data types in the compiler.
// This is used to print, inspect, and debug data types in the compiler.
typedef struct DTMDebug_t
{
    void (*printDataType)(DataType *type);
} DTMDebug;

// ------------------------ Symbol Table Interfaces ------------------------ //

typedef struct DTMSymbolTableEntry_t
{
    const char *name;
    DataType *type;
} DTMSymbolTableEntry;

typedef struct DTMSymbolTable_t
{
    DTMSymbolTableEntry **entries;
    int entryCount;
    int entryCapacity;

    DataType *(*getEntry)(struct DTMSymbolTable_t *table, const char *name);
    void (*addEntry)(struct DTMSymbolTable_t *table, const char *name, DataType *type);
    void (*removeEntry)(struct DTMSymbolTable_t *table, const char *name);
    void (*updateEntry)(struct DTMSymbolTable_t *table, const char *name, DataType *type);
    void (*resizeTable)(struct DTMSymbolTable_t *table);
    void (*printTable)(struct DTMSymbolTable_t *table);
} DTMSymbolTable;

// -------------------------- Data Types Interface -------------------------- //

typedef struct DTMDataTypes_t
{
    TypeContainer *(*createTypeContainer)(void);
    DataType *(*wrapTypeContainer)(TypeContainer *container);
} DTMDataTypes;

// -------------------------- Data Type Manager -------------------------- //

// A `static` Data Type Manager for managing data types in the compilation process.
// This manager is responsible for creating, accessing, and modifying data types in the compiler.
typedef struct DataTypeManager_t
{
    bool initialized;
    bool defsInitialized;

    // The symbol table for the Data Type Manager.
    DTMSymbolTable *symbolTable;
    // The data types for the Data Type Manager.
    DTMDataTypes *dataTypes;

    // Handles all Cryo primitive data types.
    DTMPrimitives *primitives;
    // Handles Struct data types in the compiler.
    DTMStructTypes *structTypes;
    // Handles Class data types in the compiler.
    DTMClassTypes *classTypes;
    // Handles Function data types in the compiler.
    DTMFunctionTypes *functionTypes;
    // Handles Generics data types in the compiler.
    DTMGenerics *generics;
    // Handles Enum data types in the compiler.
    DTMEnums *enums;

    // Data Type Helpers
    DTMHelpers *helpers;

    // Handles all debugging functions for the Data Type Manager.
    DTMDebug *debug;

    // -----------------------------
    // Function Prototypes

    void (*initDefinitions)(const char *compilerRootPath, CompilerState *state, CryoGlobalSymbolTable *globalTable);

    DataType *(*getTypeofASTNode)(ASTNode *node);
} DataTypeManager;

// ------------------------ Function Prototypes ------------------------- //

// Initialize the global Data Type Manager instance.
void initGlobalDataTypeManagerInstance(void);

// ----------------------- Constructor Prototypes ----------------------- //

// Create a new Data Type Manager instance.
DataTypeManager *createDataTypeManager(void);

DTMDebug *createDTMDebug(void);
DTMPrimitives *createDTMPrimitives(void);
DTMStructTypes *createDTMStructTypes(void);
DTMClassTypes *createDTMClassTypes(void);
DTMFunctionTypes *createDTMFunctionTypes(void);
DTMGenerics *createDTMGenerics(void);
DTMEnums *createDTMEnums(void);

DTMSymbolTable *createDTMSymbolTable(void);
DTMSymbolTableEntry *createDTMSymbolTableEntry(const char *name, DataType *type);

DTMDynamicTypeArray *createDTMDynamicTypeArray(void);
DTMDynamicTuple *createDTMDynamicTuple(void);
DTMHelpers *createDTMHelpers(void);

DTMDataTypes *createDTMDataTypes(void);

#endif // DATA_TYPE_MANAGER_H
