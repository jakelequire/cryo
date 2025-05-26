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
#ifndef COMPILER_DEFS_H
#define COMPILER_DEFS_H
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
#include "dataTypes/dataTypeManager.h"

typedef struct DTMCompilerDefs_t
{
    DataType *(*create_VA_ARGS)(void); // type `va_args` is a variable argument list
} DTMCompilerDefs;

DTMCompilerDefs *createDTMCompilerDefs(void);

void register_VA_ARGS_type(void);
DataType *create_VA_ARGS(void);

#endif // COMPILER_DEFS_H