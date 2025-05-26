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

// This file contains the data type definitions for the compiler.
// These data types are used to represent the types of variables, functions, and other data in the compiler.
// This is a forward declaration file for the data types in the compiler.
// Please see the `dataTypeDefs.h` file for the actual data type definitions.
// For using the Data Type Manager, please make sure to ` #include "dataTypes/dataTypeManager.h" ` in your source files.

#ifndef DATA_TYPES_H
#define DATA_TYPES_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "dataTypes/dataTypeDefs.h"

typedef enum LiteralType
{
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_STRING,
    LITERAL_BOOLEAN,
    LITERAL_NULL,
} LiteralType;

typedef struct DataType_t DataType;
typedef struct TypeContainer_t TypeContainer;

typedef struct DTSimpleTy_t DTSimpleTy;
typedef struct DTArrayTy_t DTArrayTy;
typedef struct DTEnumTy_t DTEnumTy;
typedef struct DTFunctionTy_t DTFunctionTy;
typedef struct DTStructTy_t DTStructTy;
typedef struct DTClassTy_t DTClassTy;
typedef struct DTObjectTy_t DTObjectTy;
typedef struct DTGenericTy_t DTGenericTy;

#endif // DATA_TYPES_H
