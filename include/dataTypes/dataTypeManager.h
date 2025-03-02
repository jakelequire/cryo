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

// ---------------------- Forward Declarations ---------------------- //

typedef struct DataTypeManager_t DataTypeManager;

extern DataTypeManager *globalDataTypeManager;
#define DTM globalDataTypeManager
#define INIT_DTM() initGlobalDataTypeManagerInstance();

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
    DataType *(*createBoolean)(bool b); // type `boolean` is a boolean
    DataType *(*createVoid)(void);      // type `void` is a void type
    DataType *(*createNull)(void);      // type `null` is a null type
    DataType *(*createAny)(void);       // type `any` is equivalent to `void *`
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
    // TODO
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

typedef struct DTMSymbolTable_t
{
    // TODO
} DTMSymbolTable;

// -------------------------- Data Type Manager -------------------------- //

// A `static` Data Type Manager for managing data types in the compilation process.
// This manager is responsible for creating, accessing, and modifying data types in the compiler.
typedef struct DataTypeManager_t
{
    // The symbol table for the Data Type Manager.
    DTMSymbolTable *symbolTable;

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

    // Handles all debugging functions for the Data Type Manager.
    DTMDebug *debug;
} DataTypeManager;

// ------------------------ Function Prototypes ------------------------- //

// Initialize the global Data Type Manager instance.
void initGlobalDataTypeManagerInstance(void);

// ----------------------- Constructor Prototypes ----------------------- //

// Create a new Data Type Manager instance.
DataTypeManager *createDataTypeManager(void);

DTMSymbolTable *createDTMSymbolTable(void);

DTMDebug *createDTMDebug(void);
DTMPrimitives *createDTMPrimitives(void);
DTMStructTypes *createDTMStructTypes(void);
DTMClassTypes *createDTMClassTypes(void);
DTMFunctionTypes *createDTMFunctionTypes(void);
DTMGenerics *createDTMGenerics(void);
DTMEnums *createDTMEnums(void);

#endif // DATA_TYPE_MANAGER_H
