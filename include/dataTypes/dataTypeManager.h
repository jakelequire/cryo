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
#define DYN_GROWTH_FACTOR 2
#define MAX_PARAM_CAPACITY 16
#define MAX_FIELD_CAPACITY 16
#define MAX_METHOD_CAPACITY 16
#define MAX_ENUM_CAPACITY 16
#define MAX_GENERIC_CAPACITY 16
#define MAX_TYPE_CAPACITY 16

// Define each token mapping
#define DECLARE_TOKEN(str, type) {str, type}

// Define the data types table with X-Macros
#define DATA_TYPE_TABLE                      \
    X("int", TOKEN_TYPE_INT)                 \
    X("string", TOKEN_TYPE_STRING)           \
    X("boolean", TOKEN_TYPE_BOOLEAN)         \
    X("void", TOKEN_TYPE_VOID)               \
    X("null", TOKEN_TYPE_NULL)               \
    X("any", TOKEN_TYPE_ANY)                 \
    X("int[]", TOKEN_TYPE_INT_ARRAY)         \
    X("string[]", TOKEN_TYPE_STRING_ARRAY)   \
    X("boolean[]", TOKEN_TYPE_BOOLEAN_ARRAY) \
    X("i8", TOKEN_TYPE_I8)                   \
    X("i16", TOKEN_TYPE_I16)                 \
    X("i32", TOKEN_TYPE_I32)                 \
    X("i64", TOKEN_TYPE_I64)                 \
    X("i128", TOKEN_TYPE_I128)

// ---------------------- Forward Declarations ---------------------- //

typedef struct DataTypeManager_t DataTypeManager;
typedef struct DataType_t DataType;
typedef struct CompilerState CompilerState;
typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;

typedef struct DTGenericTy_t DTGenericTy;
typedef struct DTArrayTy_t DTArrayTy;
typedef struct DTEnumTy_t DTEnumTy;
typedef struct DTFunctionTy_t DTFunctionTy;
typedef struct DTStructTy_t DTStructTy;
typedef struct DTClassTy_t DTClassTy;
typedef struct DTObjectTy_t DTObjectTy;
typedef struct DTSimpleTy_t DTSimpleTy;
typedef struct DTPropertyTy_t DTPropertyTy;

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

    DataType *(*createPrimString)(const char *str);
    DataType *(*createPrimBoolean)(bool value);

    PrimitiveDataType (*getPrimitiveType)(const char *typeStr);
} DTMPrimitives;

typedef struct DTMPropertyTypes_t
{
    DTPropertyTy *(*createPropertyTemplate)(void);
    DTPropertyTy *(*createPropertyType)(const char *propertyName, DataType *propertyType, ASTNode *node, bool isStatic, bool isConst, bool isPublic, bool isPrivate, bool isProtected);
    ASTNode *(*findStructPropertyNode)(DTStructTy *structNode, const char *propertyName);
} DTMPropertyTypes;

// ----------------------- Struct Data Type Interface ------------------------ //

// This structure is an interface for creating struct data types in the compiler.
// This is used to create struct data types such as `struct ... { ... }`.
typedef struct DTMStructTypes_t
{
    DataType *(*createStructTemplate)(void);
    DataType *(*createCompleteStructType)(const char *structName, DTPropertyTy **properties, int propertyCount, DataType **methods, int methodCount, bool hasConstructor, DataType **ctorArgs, int *ctorArgCount);
    DataType *(*createStructType)(const char *structName, DTPropertyTy **properties, int propertyCount, DataType **methods, int methodCount);
} DTMStructTypes;

// ------------------------ Class Data Type Interface ------------------------ //

// This structure is an interface for creating class data types in the compiler.
// This is used to create class data types such as `class ... { ... }`.
typedef struct DTMClassTypes_t
{
    DataType *(*createClassTemplate)(void);
} DTMClassTypes;

// ---------------------- Function Data Type Interface ---------------------- //

// This structure is an interface for creating function data types in the compiler.
// This is used to create function data types such as `function (...) -> ...`.
typedef struct DTMFunctionTypes_t
{
    DataType *(*createFunctionTemplate)(void);
    DataType *(*createFunctionType)(DataType **paramTypes, int paramCount, DataType *returnType);
    DataType *(*createMethodType)(const char *methodName, DataType *returnType, DataType **paramTypes, int paramCount);
} DTMFunctionTypes;

// ---------------------- Generics Data Type Interface ---------------------- //

// This structure is an interface for creating Generics data types in the compiler.
// This is used to create Generics data types such as `T`, `U`, `V`, etc.
typedef struct DTMGenerics_t
{
    DataType *(*createGenericTypeInstance)(DataType *genericType, DataType **paramTypes, int paramCount);
    DTGenericTy *(*createEmptyGenericType)();
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
    const char *(*dataTypeToString)(DataType *type);

    const char *(*typeofDataTypeToString)(TypeofDataType type);
    const char *(*typeofObjectTypeToString)(TypeofObjectType type);
    const char *(*primitiveDataTypeToString)(PrimitiveDataType type);
    const char *(*primitiveDataTypeToCType)(PrimitiveDataType type);
} DTMDebug;

// ------------------------ Symbol Table Interfaces ------------------------ //

typedef struct DTMSymbolTableEntry_t
{
    const char *name;
    const char *scopeName;
    DataType *type;
} DTMSymbolTableEntry;

typedef struct DTMSymbolTable_t
{
    DTMSymbolTableEntry **entries;
    int entryCount;
    int entryCapacity;

    DataType *(*getEntry)(struct DTMSymbolTable_t *table, const char *scopeName, const char *name);
    void (*addEntry)(struct DTMSymbolTable_t *table, const char *scopeName, const char *name, DataType *type);
    void (*addProtoType)(struct DTMSymbolTable_t *table, const char *scopeName, const char *name, PrimitiveDataType primitive, TypeofDataType typeOf, TypeofObjectType objectType);
    void (*removeEntry)(struct DTMSymbolTable_t *table, const char *scopeName, const char *name);
    void (*updateEntry)(struct DTMSymbolTable_t *table, const char *scopeName, const char *name, DataType *type);
    void (*resizeTable)(struct DTMSymbolTable_t *table);
    void (*printTable)(struct DTMSymbolTable_t *table);

    DTMSymbolTableEntry *(*createEntry)(const char *scopeName, const char *name, DataType *type);
    void (*importASTnode)(struct DTMSymbolTable_t *table, ASTNode *rootNode);
} DTMSymbolTable;

// -------------------------- Data Types Interface -------------------------- //

typedef struct DTMDataTypes_t
{
    TypeContainer *(*createTypeContainer)(void);
    DataType *(*wrapTypeContainer)(struct TypeContainer_t *container);
    DataType *(*wrapFunctionType)(struct DTFunctionTy_t *functionType);
    DataType *(*wrapArrayType)(struct DTArrayTy_t *arrayType);
    DataType *(*wrapEnumType)(struct DTEnumTy_t *enumType);
    DataType *(*wrapSimpleType)(struct DTSimpleTy_t *simpleType);
    DataType *(*wrapObjectType)(struct DTObjectTy_t *objectType);
    DataType *(*wrapGenericType)(struct DTGenericTy_t *genericType);
    DataType *(*wrapStructType)(struct DTStructTy_t *structType);
    DataType *(*wrapClassType)(struct DTClassTy_t *classType);

    DataType *(*createProtoType)(const char *name, PrimitiveDataType primitive, TypeofDataType typeOf, TypeofObjectType objectType);
} DTMDataTypes;

typedef struct DTMTypeValidation_t
{
    bool (*isSameType)(DataType *type1, DataType *type2);
    bool (*isCompatibleType)(DataType *type1, DataType *type2);
    bool (*isStringType)(DataType *type);
} DTMTypeValidation;

typedef struct DTMastInterface_t
{
    DataType *(*getTypeofASTNode)(ASTNode *node);
    DataType **(*createTypeArrayFromAST)(ASTNode *node);
    DataType **(*createTypeArrayFromASTArray)(ASTNode **nodes, int count);
    DTPropertyTy **(*createPropertyArrayFromAST)(ASTNode **nodes, int count);
} DTMastInterface;

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
    // The AST interface for the Data Type Manager.
    DTMastInterface *astInterface;
    // The type validation for the Data Type Manager.
    DTMTypeValidation *validation;

    // Handles all Cryo primitive data types.
    DTMPrimitives *primitives;
    // Handles Property data types in the compiler.
    DTMPropertyTypes *propertyTypes;
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

    // Initialize the type definitions for the Data Type Manager.
    void (*initDefinitions)(const char *compilerRootPath, const char *buildDir,
                            CryoLinker *linker, CompilerState *state, CryoGlobalSymbolTable *globalTable);

    DataType *(*getTypeofASTNode)(ASTNode *node);
    DataType *(*parseType)(const char *typeStr);
    DataType *(*resolveType)(DataTypeManager *self, const char *typeStr);
} DataTypeManager;

// ------------------------ Function Prototypes ------------------------- //

// Initialize the global Data Type Manager instance.
void initGlobalDataTypeManagerInstance(void);

DataType *DTMParseType(const char *typeStr);
DataType *DTMResolveType(DataTypeManager *self, const char *typeStr);

// ----------------------- Constructor Prototypes ----------------------- //

DTMDebug *createDTMDebug(void);

// Create a new Data Type Manager instance.
DataTypeManager *createDataTypeManager(void);

DTMPrimitives *createDTMPrimitives(void);
DTMPropertyTypes *createDTMPropertyTypes(void);
DTMStructTypes *createDTMStructTypes(void);
DTMClassTypes *createDTMClassTypes(void);
DTMFunctionTypes *createDTMFunctionTypes(void);
DTMGenerics *createDTMGenerics(void);
DTMEnums *createDTMEnums(void);

DTMSymbolTable *createDTMSymbolTable(void);
DTMSymbolTableEntry *createDTMSymbolTableEntry(const char *scopeName, const char *name, DataType *type);

DTMDynamicTypeArray *createDTMDynamicTypeArray(void);
DTMDynamicTuple *createDTMDynamicTuple(void);
DTMHelpers *createDTMHelpers(void);

DTMDataTypes *createDTMDataTypes(void);
DTMastInterface *createDTMAstInterface(void);
DTMTypeValidation *createDTMTypeValidation(void);

#endif // DATA_TYPE_MANAGER_H
