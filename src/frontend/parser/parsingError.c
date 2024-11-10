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
#include "tools/misc/syntaxHighlighter.h"

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

    // Top border with module info
    printf("\n\n%s╔════════════════════════════════ PARSER ERROR ════════════════════════════════╗%s\n", LIGHT_RED, COLOR_RESET);
    printf("\n");

    printf("%s%s  %sModule:%s %s%-50s%s %s%s\n",
           LIGHT_RED, COLOR_RESET,
           BOLD, COLOR_RESET,
           CYAN, curModule, COLOR_RESET,
           LIGHT_RED, COLOR_RESET);

    // Error message
    printf("%s%s  %sError:%s %s%-60s%s %s%s\n",
           LIGHT_RED, COLOR_RESET,
           BOLD, COLOR_RESET,
           BRIGHT_RED, message, COLOR_RESET,
           LIGHT_RED, COLOR_RESET);

    // Location info
    printf("%s%s  Location: %sLine %d, Column %d%s %s%s\n",
           LIGHT_RED, COLOR_RESET,
           YELLOW, line, column, COLOR_RESET,
           LIGHT_RED, COLOR_RESET);

    // Function name
    printf("%s%s  Function: %s%s%s %s%s\n",
           LIGHT_RED, COLOR_RESET,
           MAGENTA, functionName, COLOR_RESET,
           LIGHT_RED, COLOR_RESET);

    // Middle border
    printf("\n");
    printf("%s╠══════════════════════════════════════════════════════════════════════════════╣%s\n", LIGHT_RED, COLOR_RESET);

    printf("\n");
    // Show 3 lines before the error line
    for (int i = 3; i > 0; i--)
    {
        int contextLine = line - i;
        if (contextLine > 0)
        {
            printf("%s%s %s%3d │%s ",
                   LIGHT_RED, COLOR_RESET,
                   DARK_GRAY, contextLine, COLOR_RESET);
            printLine(source, contextLine, arena, state);
        }
    }

    // Error line (highlighted in bright red)
    printf("%s%s %s%3d │%s ",
           LIGHT_RED, COLOR_RESET,
           BRIGHT_RED, line, COLOR_RESET);
    printLine(source, line, arena, state);

    // Error pointer with message
    printf("%s%s     │ ", LIGHT_RED, COLOR_RESET);
    for (int i = 0; i < column; i++)
    {
        printf(" ");
    }
    printf("%s^%s %s%s%s\n", BRIGHT_RED, COLOR_RESET, BRIGHT_RED, message, COLOR_RESET);

    // Show 3 lines after the error line
    for (int i = 1; i <= 3; i++)
    {
        int contextLine = line + i;
        printf("%s%s %s%3d │%s ",
               LIGHT_RED, COLOR_RESET,
               DARK_GRAY, contextLine, COLOR_RESET);
        printLine(source, contextLine, arena, state);
    }

    printf("\n");

    // Bottom border
    printf("%s╚══════════════════════════════════════════════════════════════════════════════╝%s\n\n", LIGHT_RED, COLOR_RESET);

    printSymbolTable(table);

    exit(1);
}

void printLine(const char *source, int line, Arena *arena, CompilerState *state)
{
    const char *start = source;
    for (int i = 1; i < line; i++)
    {
        start = strchr(start, '\n');
        if (!start)
            return;
        start++;
    }

    // Find the end of the line
    const char *end = strchr(start, '\n');
    if (!end)
        end = start + strlen(start);

    // Create a temporary buffer for the line
    int lineLength = end - start;
    char *lineBuf = (char *)malloc(lineLength + 1);
    strncpy(lineBuf, start, lineLength);
    lineBuf[lineLength] = '\0';

    // Apply syntax highlighting
    highlightSyntax(lineBuf);
    printf("\n");

    free(lineBuf);
}

void printHorizontalLine(int width)
{
    for (int i = 0; i < width; i++)
    {
        printf(GRAY "%s" COLOR_RESET, HORIZONTAL);
    }
    printf("\n");
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
