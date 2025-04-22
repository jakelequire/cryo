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

/*
Error Code Template:
    X0-000000
    X = Error Type
        E = General Error
        F = Frontend Error
        S = Semantic Error
        L = Linker Error
        I = Internal Error
    0 = Error Severity
        0 = Note
        1 = Warning
        2 = Error
        3 = Fatal
    000000 = Error Number

The format of `X0-000000` is the template of the error codes used in Cryo.
The first digit after the Error Type is the Error Severity.
*/

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
        CRYO_SEVERITY_FATAL,
        CRYO_SEVERITY_INTERNAL
    } CryoErrorSeverity;

    typedef enum CryoErrorType
    {
        CRYO_ERROR_TYPE_GENERAL = 0x45,  // 'E' (General Error)
        CRYO_ERROR_TYPE_FRONTEND = 0x46, // 'F' (Frontend Error)
        CRYO_ERROR_TYPE_SEMANTIC = 0x53, // 'S' (Semantic Error)
        CRYO_ERROR_TYPE_LINKER = 0x4C,   // 'L' (Linker Error)
        CRYO_ERROR_TYPE_INTERNAL = 0x49  // 'I' (Internal Error)
    } CryoErrorType;

    typedef struct CryoError
    {
        CryoErrorType type;
        CryoErrorSeverity severity;
        CryoErrorCode code;
    } CryoError;

#ifdef __cplusplus
}
#endif

#endif // CRYO_ERROR_CODES_H
