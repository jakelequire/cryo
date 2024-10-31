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
#ifndef TYPE_TABLE_H
#define TYPE_TABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include "frontend/tokens.h"
#include "frontend/AST.h"
#include "settings/compilerSettings.h"
#include "common/common.h"

typedef struct CompilerState CompilerState;
typedef struct ASTNode ASTNode;
typedef struct Arena Arena;

typedef enum PrimitiveDataType
{
    PRIM_INT,     // `int`
    PRIM_FLOAT,   // `float`
    PRIM_STRING,  // `string`
    PRIM_BOOLEAN, // `boolean`
    PRIM_VOID,    // `void`
    PRIM_NULL,    // `null`
    PRIM_UNKNOWN  // `<UNKNOWN>`
} PrimitiveDataType;

typedef enum TypeofDataType
{
    PRIMITIVE_TYPE, // `int`, `float`, `string`, `boolean`, `void`
    STRUCT_TYPE,    // `struct ... { ... }`
    ENUM_TYPE,      // `enum ... { ... }`
    FUNCTION_TYPE,  // `function (...) -> ...`
    UNKNOWN_TYPE    // `<UNKNOWN>`
} TypeofDataType;

typedef struct StructType
{
    const char *name;
    ASTNode **properties;
    int propertyCount;
    int propertyCapacity;
    ASTNode **methods;
    int methodCount;
    int methodCapacity;
    bool hasDefaultValue;
    bool hasConstructor;
    int size;
} StructType;

typedef struct DataType
{
    TypeofDataType typeOf;
    union
    {
        PrimitiveDataType primitiveType;
        // Points to a struct node (StructNode)
        StructType *structType;
        // Unimplemented
        struct ASTNode *enumType;
        // Unimplemented
        struct ASTNode *functionType;
        // Built-in type (Needs to be removed for )
        CryoDataType builtInType;
    };
} DataType;

// This is the global symbol table specifically for types.
// This is how we will handle type checking and type inference.
typedef struct TypeTable
{
    DataType **types;
    int count;
    int capacity;
    char *namespaceName;
} TypeTable;

// # =========================================================================== #

// Type Table Management
TypeTable *initTypeTable(void);

// Data Type Creation from AST Nodes / Primitives
DataType *createDataTypeFromPrimitive(PrimitiveDataType type);
DataType *createDataTypeFromStruct(ASTNode *structNode, CompilerState *state, TypeTable *typeTable);
DataType *createDataTypeFromEnum(Arena *arena, CompilerState *state, ASTNode *enumNode);
DataType *createDataTypeFromFunction(Arena *arena, CompilerState *state, ASTNode *functionNode);
DataType *createDataTypeFromUnknown(void);

// Node to type conversion
StructType *createStructDataType(const char *name);

// Add Type to Type Table
void addTypeToTypeTable(TypeTable *table, DataType *type);

// Utility Functions
char *TypeofDataTypeToString(TypeofDataType type);
char *PrimitiveDataTypeToString(PrimitiveDataType type);

void printFormattedStructType(StructType *type);
void printFormattedType(DataType *type);

void printTypeTable(TypeTable *table);

#endif // TYPE_TABLE_H
