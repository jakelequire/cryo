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
#include "tools/cxx/IDGen.hpp"
#include "diagnostics/diagnostics.h"
#include "frontend/parser.h"

// Parse generic type parameters: <T, U, V>
GenericType **parseGenericTypeParams(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, int *paramCount)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing generic type parameters...");

    // consume(__LINE__, lexer, TOKEN_LESS, "Expected '<' to start generic type parameters.", "parseGenericTypeParams", arena, state, context);

    // GenericType **typeParams = (GenericType **)malloc(GENERIC_CAPACITY * sizeof(GenericType *));
    // int count = 0;

    // while (lexer->currentToken.type != TOKEN_GREATER)
    // {
    //     if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    //     {
    //         parsingError("Expected generic type identifier.", "parseGenericTypeParams", arena, state, lexer, lexer->source, globalTable);
    //         return NULL;
    //     }

    //     char *typeName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    //     GenericType *typeParam = createGenericParameter(typeName);
    //     typeParams[count++] = typeParam;

    //     getNextToken(lexer, arena, state);

    //     if (lexer->currentToken.type == TOKEN_COMMA)
    //     {
    //         getNextToken(lexer, arena, state);
    //     }
    //     else if (lexer->currentToken.type != TOKEN_GREATER)
    //     {
    //         parsingError("Expected ',' or '>' in generic type parameter list.", "parseGenericTypeParams", arena, state, lexer, lexer->source, globalTable);
    //         return NULL;
    //     }
    // }

    // consume(__LINE__, lexer, TOKEN_GREATER, "Expected '>' to end generic type parameters.", "parseGenericTypeParams", arena, state, context);

    // *paramCount = count;
    // return typeParams;
    return NULL;
}
