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
#ifndef SYNTAX_HIGHLIGHTER_H
#define SYNTAX_HIGHLIGHTER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include "tools/macros/consoleColors.h"

// Extended keywords list including preprocessor directives and more C keywords
const char *syntaxHighlightkeywords[] = {
    // Control flow
    "if", "else", "while", "for", "do", "switch", "case", "break",
    "continue", "return", "goto", "default",
    // Type modifiers
    "static", "extern", "const", "mut", "register", "inline",
    "sizeof", "typedef", "enum", "struct", "union", "auto",
    // Function specifiers
    "inline",
    // Other keywords
    "try", "catch", "throw", "new", "delete",
    NULL};

// Extended types list including more C/C++ types
const char *syntaxHighlighttypes[] = {
    // Basic types
    "int", "string", "Int", "string", "void", "boolean",
    "null",
    // Extended types
    "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
    "int64_t", "uint64_t", "intptr_t", "uintptr_t", "ptrdiff_t",
    // Your custom types
    "string", "String", "Int", "boolean", "null",
    NULL};

// Preprocessor directives
const char *preprocessorDirectives[] = {
    "#include", "#define", "#undef", "#if", "#ifdef", "#ifndef",
    "#else", "#elif", "#endif", "#error", "#pragma", "#line",
    NULL};

// Common macros and special identifiers
const char *specialIdentifiers[] = {
    "public", "true", "false", "private", "FALSE", "__FILE__", "__LINE__",
    "__func__", "__DATE__", "__TIME__", "__STDC__", "__cplusplus",
    NULL};

// Helper to check if character is valid in an identifier
bool isIdentifierChar(char c)
{
    return isalnum(c) || c == '_';
}

// Helper function to check if a string is in an array
bool stringInArray(const char *str, const char **array)
{
    for (int i = 0; array[i] != NULL; i++)
    {
        if (strcmp(str, array[i]) == 0)
            return true;
    }
    return false;
}

void highlightSyntax(const char *line)
{
    char word[256] = {0};
    int wordLen = 0;
    bool inString = false;
    bool inChar = false;
    bool inLineComment = false;
    bool inBlockComment = false;
    bool inPreprocessor = false;

    for (int i = 0; line[i] != '\0' && line[i] != '\n'; i++)
    {
        char c = line[i];
        char next = line[i + 1];

        // Handle preprocessor directives at start of line
        if (c == '#' && !inString && !inChar && !inLineComment && !inBlockComment &&
            (i == 0 || isspace(line[i - 1])))
        {
            printf("%s#", BRIGHT_CYAN);
            inPreprocessor = true;
            continue;
        }

        // Handle strings
        if (c == '"' && !inChar && !inLineComment && !inBlockComment)
        {
            if (!inString)
            {
                printf("%s\"", YELLOW); // Start string
            }
            else
            {
                printf("\"%s", COLOR_RESET); // End string
            }
            inString = !inString;
            continue;
        }

        // Handle escape sequences in strings
        if (inString && c == '\\' && next != '\0')
        {
            printf("%s\\%c%s", LIGHT_YELLOW, next, YELLOW);
            i++; // Skip next character
            continue;
        }

        // Handle character literals
        if (c == '\'' && !inString && !inLineComment && !inBlockComment)
        {
            if (!inChar)
            {
                printf("%s'", YELLOW); // Start char
            }
            else
            {
                printf("'%s", COLOR_RESET); // End char
            }
            inChar = !inChar;
            continue;
        }

        // Handle line comments
        if (c == '/' && next == '/' && !inString && !inChar && !inBlockComment)
        {
            printf("%s//%s", DARK_GRAY, GRAY);
            inLineComment = true;
            i++;
            continue;
        }

        // Handle block comments
        if (c == '/' && next == '*' && !inString && !inChar && !inLineComment)
        {
            printf("%s/*", DARK_GRAY);
            inBlockComment = true;
            i++;
            continue;
        }
        if (c == '*' && next == '/' && inBlockComment)
        {
            printf("*/%s", COLOR_RESET);
            inBlockComment = false;
            i++;
            continue;
        }

        // Print comments in gray
        if (inLineComment || inBlockComment)
        {
            putchar(c);
            continue;
        }

        if (inString || inChar)
        {
            putchar(c);
            continue;
        }

        // Collect word for keyword/type checking
        if (isIdentifierChar(c))
        {
            word[wordLen++] = c;
            word[wordLen] = '\0';
        }
        else
        {
            if (wordLen > 0)
            {
                // Check word against all our categories
                if (stringInArray(word, preprocessorDirectives))
                {
                    printf("%s%s%s", BRIGHT_CYAN, word, COLOR_RESET);
                }
                else if (stringInArray(word, syntaxHighlightkeywords))
                {
                    printf("%s%s%s", MAGENTA, word, COLOR_RESET);
                }
                else if (stringInArray(word, syntaxHighlighttypes))
                {
                    printf("%s%s%s", CYAN, word, COLOR_RESET);
                }
                else if (stringInArray(word, specialIdentifiers))
                {
                    printf("%s%s%s", LIGHT_MAGENTA, word, COLOR_RESET);
                }
                else if (inPreprocessor)
                {
                    printf("%s%s%s", BRIGHT_CYAN, word, COLOR_RESET);
                }
                else
                {
                    // Function call detection (word followed by opening parenthesis)
                    const char *ahead = line + i;
                    while (*ahead && isspace(*ahead))
                        ahead++;
                    if (*ahead == '(')
                    {
                        printf("%s%s%s", LIGHT_YELLOW, word, COLOR_RESET);
                    }
                    else
                    {
                        printf("%s", word);
                    }
                }
                wordLen = 0;
                word[0] = '\0';
            }

            // Handle numeric literals with more precision
            if (isdigit(c) || (c == '.' && isdigit(next)))
            {
                printf("%s", LIGHT_GREEN);
                // Output the first character
                putchar(c);

                // Keep outputting until we hit a non-number character
                i++;
                while (line[i] != '\0' &&
                       (isalnum(line[i]) || line[i] == '.' ||
                        line[i] == 'x' || line[i] == 'X' ||
                        line[i] == 'f' || line[i] == 'F' ||
                        line[i] == 'u' || line[i] == 'U' ||
                        line[i] == 'l' || line[i] == 'L'))
                {
                    putchar(line[i]);
                    i++;
                }
                printf("%s", COLOR_RESET);
                i--; // Back up one since the loop will increment
                continue;
            }

            // Highlight operators and punctuation
            if (strchr("+-*/%=<>!&|^~?:;.,()[]{}", c))
            {
                // Special handling for arrow operator
                if (c == '-' && next == '>')
                {
                    printf("%s->%s", LIGHT_GRAY, COLOR_RESET);
                    i++;
                }
                else
                {
                    printf("%s%c%s", LIGHT_GRAY, c, COLOR_RESET);
                }
            }
            else
            {
                putchar(c);
            }
        }
    }

    // Handle any remaining word
    if (wordLen > 0)
    {
        if (stringInArray(word, preprocessorDirectives))
        {
            printf("%s%s%s", BRIGHT_CYAN, word, COLOR_RESET);
        }
        else if (stringInArray(word, syntaxHighlightkeywords))
        {
            printf("%s%s%s", MAGENTA, word, COLOR_RESET);
        }
        else if (stringInArray(word, syntaxHighlighttypes))
        {
            printf("%s%s%s", CYAN, word, COLOR_RESET);
        }
        else if (stringInArray(word, specialIdentifiers))
        {
            printf("%s%s%s", LIGHT_BLUE, word, COLOR_RESET);
        }
        else if (inPreprocessor)
        {
            printf("%s%s%s", BRIGHT_CYAN, word, COLOR_RESET);
        }
        else
        {
            printf("%s", word);
        }
    }
}

#endif // SYNTAX_HIGHLIGHTER_H