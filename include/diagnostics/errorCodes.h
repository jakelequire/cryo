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
#ifndef CRYO_ERROR_CODES_H
#define CRYO_ERROR_CODES_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum CryoErrorCode
    {
        // General (0-99)
        CRYO_SUCCESS = 0,
        CRYO_ERROR_UNKNOWN = 1,
        CRYO_ERROR_INVALID_ARGUMENT = 2,
        CRYO_ERROR_OUT_OF_MEMORY = 3,

        // Lexer/Parser Errors (100-199)
        CRYO_ERROR_SYNTAX = 100,
        CRYO_ERROR_UNEXPECTED_TOKEN = 101,
        CRYO_ERROR_UNTERMINATED_STRING = 102,
        CRYO_ERROR_INVALID_CHARACTER = 103,

        // Semantic Errors (200-299)
        CRYO_ERROR_UNDEFINED_SYMBOL = 200,
        CRYO_ERROR_TYPE_MISMATCH = 201,
        CRYO_ERROR_REDEFINITION = 202,

        // Linker Errors (300-399)
        CRYO_ERROR_LINK_FAILED = 300,
        CRYO_ERROR_UNDEFINED_REFERENCE = 301,
        CRYO_ERROR_DUPLICATE_SYMBOL = 302,

        // Internal Errors (900-999)
        CRYO_ERROR_INTERNAL = 900,
        CRYO_ERROR_NOT_IMPLEMENTED = 901
    } CryoErrorCode;

    // Error severity levels
    typedef enum CryoErrorSeverity
    {
        CRYO_SEVERITY_NOTE,
        CRYO_SEVERITY_WARNING,
        CRYO_SEVERITY_ERROR,
        CRYO_SEVERITY_FATAL
    } CryoErrorSeverity;

#ifdef __cplusplus
}
#endif

#endif // CRYO_ERROR_CODES_H
