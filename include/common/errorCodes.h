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
#ifndef ERROR_CODES_H
#define ERROR_CODES_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Error Code Structure
// [Error Code] {SHORT HAND ENUM} [Description]

// Semantic Errors
// All Semantic Error Codes start with 1000 and increment by 1
typedef enum SemanticError
{
    UNKNOWN_ERROR = -1,    // Unknown Error
    SEMANTIC_ERROR = 1000, // General Semantic Error Code
    SEMANTIC_WARNING,      // General Semantic Warning Code

    MISSING_SEMICOLON, // Missing Semicolon
    MISSING_COLON,     // Missing Colon
    MISSING_COMMA,     // Missing Comma
    MISSING_PAREN,     // Missing Parenthesis
    MISSING_BRACE,     // Missing Brace
    MISSING_BRACKET,   // Missing Bracket

    INVALID_TYPE,        // Invalid Type
    INVALID_VALUE,       // Invalid Value
    INVALID_EXPRESSION,  // Invalid Expression
    INVALID_STATEMENT,   // Invalid Statement
    INVALID_DECLARATION, // Invalid Declaration
    INVALID_ASSIGNMENT,  // Invalid Assignment
    INVALID_OPERATION,   // Invalid Operation
    INVALID_ARGUMENT,    // Invalid Argument
    INVALID_RETURN,      // Invalid Return
    INVALID_BREAK,       // Invalid Break
    INVALID_CONTINUE,    // Invalid Continue
    INVALID_LOOP,        // Invalid Loop
    INVALID_CONDITION,   // Invalid Condition

    // More to be added...
};

typedef struct Error
{
    int code;
    const char *message;
    const char *file;
    const char *function;
    int line;
    int column;
} Error;

#endif // ERROR_CODES_H
