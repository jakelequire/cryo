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

typedef enum PrimitiveDataType
{
    PRIM_INT,
    PRIM_FLOAT,
    PRIM_STRING,
    PRIM_BOOLEAN,
    PRIM_VOID,
    PRIM_NULL,
    PRIM_UNKNOWN
} PrimitiveDataType;

// Unimplemented
typedef enum TypeofDataType
{
    PRIMITIVE_TYPE, // `int`, `float`, `string`, `boolean`, `void`
    STRUCT_TYPE,    // `struct ... { ... }`
    ENUM_TYPE,      // `enum ... { ... }`
    FUNCTION_TYPE,  // `function (...) -> ...`
    UNKNOWN_TYPE    // `<UNKNOWN>`
} TypeofDataType;

// Unimplemented
typedef struct DataType
{
    TypeofDataType typeOf;
    union
    {
        CryoDataType primitiveType;
        // Points to a struct node (StructNode)
        struct ASTNode *structType;
        // Unimplemented
        struct ASTNode *enumType;
        // Unimplemented
        struct ASTNode *functionType;
        // Built-in type
        CryoDataType builtInType;
    };
} DataType;

typedef struct TypeTable
{

} TypeTable;

// # =========================================================================== #

char *TypeofDataTypeToString(TypeofDataType type);

#endif // TYPE_TABLE_H
