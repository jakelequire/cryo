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
#include "frontend/parser.h"

void parsingError(
    char *message,
    char *functionName,
    CryoSymbolTable *table,
    Arena *arena,
    CompilerState *state,
    Lexer *lexer,
    const char *source, 
    TypeTable *typeTable)
{
    int line = lexer->currentToken.line;
    int column = lexer->currentToken.column;

    char *curModule = getCurrentNamespace(table);
    if (!curModule)
    {
        curModule = "Unnamed";
    }

    printSymbolTable(table);
    dumpCompilerState(*state);

    printf("\n\n------------------------------------------------------------------------\n");
    printf("<!> [Parser] Module: %s\n", curModule);
    printf("<!> [Parser] Error: %s at line %d, column %d\n", message, line, column);
    printf("@Function: <%s>\n", functionName);
    printf("------------------------------------------------------------------------\n\n");
    // Print the line containing the error
    printf("%d ", line - 1);
    printLine(source, line - 1, arena, state);
    printf("%d ", line);
    printLine(source, line, arena, state);

    for (int i = line; i < column + line; i++)
    {
        printf(" ");
    }

    printf(" ^ %s\n", message);
    printf("\n------------------------------------------------------------------------\n\n");
    exit(1);
}

void printLine(const char *source, int line, Arena *arena, CompilerState *state)
{
    const char *lineStart = source;
    while (*lineStart && line > 1)
    {
        if (*lineStart == '\n')
            line--;
        lineStart++;
    }

    const char *lineEnd = lineStart;
    while (*lineEnd && *lineEnd != '\n')
    {
        lineEnd++;
    }

    printf("%.*s\n", (int)(lineEnd - lineStart), lineStart);
}

Position getPosition(Lexer *lexer)
{
    // Set the position of the lexer
    Position position;
    position.line = lexer->line;
    position.column = lexer->column;
    return position;
}

void printPosition(Position position)
{
    printf("Line: %d, Column: %d\n", position.line, position.column);
}

void debugCurrentToken(Lexer *lexer, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    // printf("[Parser DEBUG] Current Token: %s, Lexeme: %.*s\n",
    //        CryoTokenToString(lexer->currentToken.type), lexer->currentToken.length, lexer->currentToken.start);
    logMessage("DEBUG", __LINE__, "Parser", "Current Token: %s, Lexeme: %.*s",
               CryoTokenToString(lexer->currentToken.type), lexer->currentToken.length, lexer->currentToken.start);
}
