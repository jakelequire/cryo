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
#include "diagnostics/diagnostics.h"
#include "tools/logger/logger_config.h"

const char *CryoErrorCodeToString(CryoErrorCode errorCode)
{
    switch (errorCode)
    {
    case CRYO_SUCCESS:
        return "SUCCESS";
    case CRYO_ERROR_UNKNOWN:
        return "UNKNOWN ERROR";
    case CRYO_ERROR_INVALID_ARGUMENT:
        return "INVALID ARGUMENT";
    case CRYO_ERROR_OUT_OF_MEMORY:
        return "OUT OF MEMORY";
    case CRYO_ERROR_SYNTAX:
        return "SYNTAX ERROR";
    case CRYO_ERROR_UNEXPECTED_TOKEN:
        return "UNEXPECTED TOKEN";
    case CRYO_ERROR_UNTERMINATED_STRING:
        return "UNTERMINATED STRING";
    case CRYO_ERROR_INVALID_CHARACTER:
        return "INVALID CHARACTER";

    // Semantic Errors (200-299)
    case CRYO_ERROR_UNDEFINED_SYMBOL:
        return "UNDEFINED SYMBOL";
    case CRYO_ERROR_TYPE_MISMATCH:
        return "TYPE MISMATCH";
    case CRYO_ERROR_REDEFINITION:
        return "REDEFINITION";
    case CRYO_ERROR_UNKNOWN_DATA_TYPE:
        return "UNKNOWN DATA TYPE";
    case CRYO_ERROR_NOT_A_FUNCTION:
        return "NOT A FUNCTION";
    case CRYO_ERROR_INVALID_FUNCTION_CALL:
        return "INVALID FUNCTION CALL";
    case CRYO_ERROR_INVALID_ARGUMENT_COUNT:
        return "INVALID ARGUMENT COUNT";
    case CRYO_ERROR_INVALID_RETURN_TYPE:
        return "INVALID RETURN TYPE";
    case CRYO_ERROR_INVALID_ACCESS:
        return "INVALID ACCESS";
    case CRYO_ERROR_INVALID_ASSIGNMENT:
        return "INVALID ASSIGNMENT";
    case CRYO_ERROR_INVALID_INDEX:
        return "INVALID INDEX";
    case CRYO_ERROR_INVALID_MEMBER_ACCESS:
        return "INVALID MEMBER ACCESS";
    case CRYO_ERROR_INVALID_CAST:
        return "INVALID CAST";
    case CRYO_ERROR_INVALID_SCOPE:
        return "INVALID SCOPE";
    case CRYO_ERROR_INVALID_NAMESPACE:
        return "INVALID NAMESPACE";
    case CRYO_ERROR_NULL_DATA_TYPE:
        return "NULL DATA TYPE";
    case CRYO_ERROR_NULL_POINTER_DEREFERENCE:
        return "NULL POINTER DEREFERENCE";
    case CRYO_ERROR_TOO_MANY_TYPE_ARGUMENTS:
        return "TOO MANY TYPE ARGUMENTS";
    case CRYO_ERROR_INCORRECT_TYPE_ARGUMENT_COUNT:
        return "INCORRECT TYPE ARGUMENT COUNT";
    case CRYO_ERROR_INVALID_OPERATOR:
        return "INVALID OPERATOR";
    case CRYO_ERROR_INVALID_OPERATOR_PRECEDENCE:
        return "INVALID OPERATOR PRECEDENCE";
    case CRYO_ERROR_INVALID_THIS_CONTEXT:
        return "INVALID THIS CONTEXT";
    case CRYO_ERROR_UNDEFINED_PROPERTY:
        return "UNDEFINED PROPERTY";
    case CRYO_ERROR_UNDEFINED_METHOD:
        return "UNDEFINED METHOD";
    case CRYO_ERROR_INVALID_NODE_TYPE:
        return "INVALID NODE TYPE";
    case CRYO_ERROR_INVALID_MUTABILITY:
        return "INVALID MUTABILITY";
    case CRYO_ERROR_INVALID_TYPE:
        return "INVALID TYPE";
    case CRYO_ERROR_UNDEFINED_TYPE:
        return "UNDEFINED TYPE";
    case CRYO_ERROR_INVALID_TYPE_DECLARATION:
        return "INVALID TYPE DECLARATION";

    // Linker Errors (300-399)
    case CRYO_ERROR_LINK_FAILED:
        return "LINK FAILED";
    case CRYO_ERROR_UNDEFINED_REFERENCE:
        return "UNDEFINED REFERENCE";
    case CRYO_ERROR_DUPLICATE_SYMBOL:
        return "DUPLICATE SYMBOL";

    // Internal Errors (900-999)
    case CRYO_ERROR_INTERNAL:
        return "INTERNAL ERROR";
    case CRYO_ERROR_NOT_IMPLEMENTED:
        return "NOT IMPLEMENTED";
    case CRYO_ERROR_PANIC:
        return "PANIC";
    case CRYO_ERROR_NULL_AST_NODE:
        return "NULL AST NODE";
    case CRYO_ERROR_INTEGER_OVERFLOW:
        return "INTEGER OVERFLOW";

    default:
        return "UNKNOWN ERROR CODE";
    }
}

const char *CryoErrorSeverityToString(CryoErrorSeverity severity)
{
    switch (severity)
    {
    case CRYO_SEVERITY_NOTE:
        return "NOTE";
    case CRYO_SEVERITY_WARNING:
        return "WARNING";
    case CRYO_SEVERITY_ERROR:
        return "ERROR";
    case CRYO_SEVERITY_FATAL:
        return "FATAL";
    case CRYO_SEVERITY_INTERNAL:
        return "INTERNAL";
    default:
        return "UNKNOWN SEVERITY";
    }
}