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
#ifndef CONFIG_TOKENS_H
#define CONFIG_TOKENS_H

#include <stdio.h>
#include <stdlib.h>

typedef enum
{
    CONFIG_TOK_UNKNOWN = -1,
    CONFIG_TOK_PROJECT = 0,  // [project]
    CONFIG_TOK_COMPILER,     // [compiler]
    CONFIG_TOK_DEPENDENCIES, // [dependencies]
    CONFIG_TOK_BUILD,        // [build]
    CONFIG_TOK_RUN,          // [run]

    // Primitive tokens
    CONFIG_TOK_IDENTIFIER, // project_name
    CONFIG_TOK_STRING,     // "string"
    CONFIG_TOK_NUMBER,     // 1234
    CONFIG_TOK_BOOL,       // true/false
    CONFIG_TOK_NULL,       // null
} ConfigToken;

#endif // CONFIG_TOKENS_H
