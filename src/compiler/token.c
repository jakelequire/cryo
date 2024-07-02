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
#include "compiler/token.h"




/*
typedef enum {
    OPERATOR_UNKNOWN = -1,      // `<UNKNOWN>`       -1
    OPERATOR_ADD,               // `+`               0
    OPERATOR_SUB,               // `-`               1
    OPERATOR_MUL,               // `*`               2
    OPERATOR_DIV,               // `/`               3
    OPERATOR_MOD,               // `%`               4
    OPERATOR_AND,               // `&`               5
    OPERATOR_OR,                // `|`               6
    OPERATOR_XOR,               // `^`               7
    OPERATOR_NOT,               // `!`               8
    OPERATOR_LSHIFT,            // `<<`              9
    OPERATOR_RSHIFT,            // `>>`              10
    OPERATOR_LT,                // `<`               11
    OPERATOR_GT,                // `>`               12
    OPERATOR_LTE,               // `<=`              13
    OPERATOR_GTE,               // `>=`              14
    OPERATOR_EQ,                // `==`              15
    OPERATOR_NEQ,               // `!=`              16
    OPERATOR_ASSIGN,            // `=`               17
    OPERATOR_ADD_ASSIGN,        // `+=`              18
    OPERATOR_SUB_ASSIGN,        // `-=`              19
    OPERATOR_MUL_ASSIGN,        // `*=`              20
    OPERATOR_DIV_ASSIGN,        // `/=`              21
    OPERATOR_MOD_ASSIGN,        // `%=`              22
    OPERATOR_AND_ASSIGN,        // `&=`              23
    OPERATOR_OR_ASSIGN,         // `|=`              24
    OPERATOR_XOR_ASSIGN,        // `^=`              25
    OPERATOR_NA,                // `<UNKNOWN>`       26
} CryoOperatorType;
*/

char* operatorToString(CryoOperatorType op) {
    switch (op) {
        case OPERATOR_ADD: return "+";
        case OPERATOR_SUB: return "-";
        case OPERATOR_MUL: return "*";
        case OPERATOR_DIV: return "/";
        case OPERATOR_MOD: return "%";
        case OPERATOR_AND: return "&";
        case OPERATOR_OR: return "|";
        case OPERATOR_XOR: return "^";
        case OPERATOR_NOT: return "!";
        case OPERATOR_LSHIFT: return "<<";
        case OPERATOR_RSHIFT: return ">>";
        case OPERATOR_LT: return "<";
        case OPERATOR_GT: return ">";
        case OPERATOR_LTE: return "<=";
        case OPERATOR_GTE: return ">=";
        case OPERATOR_EQ: return "==";
        case OPERATOR_NEQ: return "!=";
        case OPERATOR_ASSIGN: return "=";
        case OPERATOR_ADD_ASSIGN: return "+=";
        case OPERATOR_SUB_ASSIGN: return "-=";
        case OPERATOR_MUL_ASSIGN: return "*=";
        case OPERATOR_DIV_ASSIGN: return "/=";
        case OPERATOR_MOD_ASSIGN: return "%=";
        case OPERATOR_AND_ASSIGN: return "&=";
        case OPERATOR_OR_ASSIGN: return "|=";
        case OPERATOR_XOR_ASSIGN: return "^=";
        case OPERATOR_NA: return "<N/A>";
        default: return "<UNKNOWN>";
    }
}


