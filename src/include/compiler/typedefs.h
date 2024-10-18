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
#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#include "compiler/token.h"
#include "compiler/ast.h"
#include "utils/fs.h"
#include "utils/utility.h"
#include "common/common.h"

typedef struct ExternalSymbol
{
    CryoNodeType nodeType;
    CryoDataType dataType;
    char *name;
    ASTNode *node;
} ExternalSymbol;

typedef struct ExternalSymbolTable
{
    ExternalSymbol **symbols;
    int count;
    int capacity;
} ExternalSymbolTable;

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
        ASTNode *structType;
        // Unimplemented
        ASTNode *enumType;
        // Unimplemented
        ASTNode *functionType;
        // Built-in type
        CryoDataType builtInType;
    };
} DataType;

char *TypeofDataTypeToString(TypeofDataType type);
CryoDataType parseDataType(const char *typeStr);
CryoDataType getPrimativeTypeFromString(const char *typeStr);

#endif // TYPEDEFS_H
