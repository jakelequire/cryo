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
// errorMessage.c
#include "diagnostics/diagnostics.h"
#include "tools/logger/logger_config.h"

extern void highlightSyntax(const char *line);

#ifndef ERROR_CONTEXT_LINES_BEFORE
#define ERROR_CONTEXT_LINES_BEFORE 7 // Default: show 5 lines before error
#endif

#ifndef ERROR_CONTEXT_LINES_AFTER
#define ERROR_CONTEXT_LINES_AFTER 13 // Default: show 10 lines after error
#endif

#define ERROR_CONTEXT_TOTAL_LINES (ERROR_CONTEXT_LINES_BEFORE + 1 + ERROR_CONTEXT_LINES_AFTER)

// Border characters
#define BOX_HORIZONTAL "════════════════════════════════════════════════════════════════════════════════"
#define BOX_VERTICAL "║"
#define BOX_TOP_LEFT "╔"
#define BOX_TOP_RIGHT "╗"
#define BOX_BOTTOM_LEFT "╚"
#define BOX_BOTTOM_RIGHT "╝"
#define BOX_MIDDLE_LEFT "╠"
#define BOX_MIDDLE_RIGHT "╣"

/**
 * Extracts and highlights a single line from source code
 */
static int extractAndHighlightLine(const char *source, size_t targetLine,
                                   char *lineBuf, size_t maxLength)
{
    if (!source || targetLine < 1 || maxLength == 0)
        return 0;

    // Find the beginning of the target line
    const char *start = source;
    size_t currentLine = 1;

    while (*start && currentLine < targetLine)
    {
        if (*start == '\n')
        {
            currentLine++;
        }
        start++;
    }

    // Check if we found the target line
    if (currentLine != targetLine)
        return 0;

    // Find the end of the line
    const char *end = strchr(start, '\n');
    if (!end)
        end = start + strlen(start);

    // Calculate line length
    size_t lineLength = end - start;
    if (lineLength > maxLength - 1)
        lineLength = maxLength - 1;

    // If lineBuf is NULL, allocate a new buffer
    char *tempBuf = lineBuf;
    int needsFree = 0;

    if (!tempBuf)
    {
        tempBuf = (char *)malloc(lineLength + 1);
        if (!tempBuf)
            return 0;
        needsFree = 1;
    }

    // Copy the line to the buffer
    strncpy(tempBuf, start, lineLength);
    tempBuf[lineLength] = '\0';

    // Apply syntax highlighting
    highlightSyntax(tempBuf);

    // Free the buffer if we allocated it
    if (needsFree)
        free(tempBuf);

    return 1;
}

/**
 * Safely prints a specific line from the source code
 */
static int printSourceCodeLine(const char *source, size_t targetLine, size_t maxLineLength)
{
    // Validate input parameters
    if (!source)
        return 0;

    if (targetLine < 1 || maxLineLength == 0)
        return 0;

    // Allocate a buffer for the line
    char *lineBuf = (char *)malloc(maxLineLength + 1);
    if (!lineBuf)
        return 0;

    // Find and extract the line
    const char *start = source;
    size_t currentLine = 1;

    while (*start && currentLine < targetLine)
    {
        if (*start == '\n')
        {
            currentLine++;
        }
        start++;
    }

    // Check if we found the target line
    if (currentLine != targetLine)
    {
        free(lineBuf);
        return 0;
    }

    // Find the end of the line
    const char *end = strchr(start, '\n');
    if (!end)
        end = start + strlen(start);

    // Calculate line length and handle truncation
    size_t lineLength = end - start;
    size_t outputLength = (lineLength < maxLineLength) ? lineLength : maxLineLength;

    // Copy the line to the buffer
    strncpy(lineBuf, start, outputLength);
    lineBuf[outputLength] = '\0';

    // Apply syntax highlighting
    highlightSyntax(lineBuf);

    // If line was truncated, indicate it
    if (lineLength > maxLineLength)
    {
        printf("... (%zu more characters)", lineLength - maxLineLength);
    }

    printf("\n");
    free(lineBuf);
    return 1;
}

/**
 * Prints context lines around the error with highlighting
 */
static void printErrorContext(const char *source, size_t errorLine, size_t errorColumn,
                              const char *errorMessage, size_t maxLineLength)
{
    if (!source || !errorMessage)
        return;

    // Find start of source for line counting
    const char *sourceStart = source;
    size_t currentLine = 1;

    // Use dynamic array sizes based on macros
    const char **lineStarts = (const char **)calloc(ERROR_CONTEXT_TOTAL_LINES, sizeof(const char *));
    size_t *lineLengths = (size_t *)calloc(ERROR_CONTEXT_TOTAL_LINES, sizeof(size_t));

    if (!lineStarts || !lineLengths)
    {
        // Fallback to minimal error reporting if allocation fails
        printf("Error at line %zu, column %zu: %s\n", errorLine, errorColumn, errorMessage);
        if (lineStarts)
            free(lineStarts);
        if (lineLengths)
            free(lineLengths);
        return;
    }

    // First scan: find line starts for context
    while (*source)
    {
        // Mark start of lines we care about (using macro-defined range)
        if (currentLine >= errorLine - ERROR_CONTEXT_LINES_BEFORE &&
            currentLine <= errorLine + ERROR_CONTEXT_LINES_AFTER)
        {
            size_t idx = currentLine - (errorLine - ERROR_CONTEXT_LINES_BEFORE);
            if (idx < ERROR_CONTEXT_TOTAL_LINES)
            { // Safety check
                lineStarts[idx] = source;
            }
        }

        // Scan until end of current line
        const char *lineStart = source;
        while (*source && *source != '\n')
        {
            source++;
        }

        // Calculate line length
        size_t lineLength = source - lineStart;
        if (currentLine >= errorLine - ERROR_CONTEXT_LINES_BEFORE &&
            currentLine <= errorLine + ERROR_CONTEXT_LINES_AFTER)
        {
            size_t idx = currentLine - (errorLine - ERROR_CONTEXT_LINES_BEFORE);
            if (idx < ERROR_CONTEXT_TOTAL_LINES)
            { // Safety check
                lineLengths[idx] = lineLength;
            }
        }

        // Move past newline if not at end
        if (*source)
        {
            source++;
            currentLine++;
        }
    }

    // Buffer for a single line
    char *lineBuf = (char *)malloc(maxLineLength + 1);
    if (!lineBuf)
    {
        free(lineStarts);
        free(lineLengths);
        return;
    }

    // Print context lines - use macro for loop bounds
    for (int i = 0; i < ERROR_CONTEXT_TOTAL_LINES; i++)
    {
        size_t contextLine = errorLine - ERROR_CONTEXT_LINES_BEFORE + i;
        const char *lineStart = lineStarts[i];
        size_t lineLength = lineLengths[i];

        if (!lineStart)
            continue; // Skip if line not found

        // Determine line color and prefix based on if it's the error line
        const char *lineColor = (contextLine == errorLine) ? BRIGHT_RED : DARK_GRAY;

        // Print line number and separator with improved styling
        printf("%s%s%s %s%3zu %s│%s ",
               LIGHT_RED, BOX_VERTICAL, COLOR_RESET,
               lineColor, contextLine,
               lineColor, COLOR_RESET);

        if (lineLength > 0)
        {
            // Extract and highlight the line
            size_t charsToPrint = lineLength < maxLineLength ? lineLength : maxLineLength;

            // Copy line to buffer
            strncpy(lineBuf, lineStart, charsToPrint);
            lineBuf[charsToPrint] = '\0';

            // Apply syntax highlighting
            highlightSyntax(lineBuf);

            // If line was truncated, indicate it
            if (lineLength > maxLineLength)
            {
                printf(" %s... (%zu more)%s", DARK_GRAY, lineLength - maxLineLength, COLOR_RESET);
            }
        }

        printf("\n");

        // Add error pointer on error line
        if (contextLine == errorLine)
        {
            // First line with arrow pointer
            printf("%s%s     │%s", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);

            // Cap the column to avoid going too far right
            size_t safeColumn = errorColumn < maxLineLength ? errorColumn : maxLineLength - 1;

            // Print spaces up to the error column
            for (size_t j = 0; j < safeColumn; j++)
            {
                printf(" ");
            }

            // Print the error pointer
            printf("%s▲%s ", BRIGHT_RED, COLOR_RESET);

            // Get error message length
            const char *msg = errorMessage;
            size_t msgLen = strlen(msg);

            // Calculate available width for first line
            size_t terminalWidth = 80;                               // Adjust based on your box width
            size_t firstLineAvail = terminalWidth - safeColumn - 15; // Account for border + arrow + margin

            // Print first line (up to available width)
            size_t lineLen = msgLen < firstLineAvail ? msgLen : firstLineAvail;
            printf("%s", BRIGHT_RED);
            for (size_t i = 0; i < lineLen; i++)
            {
                printf("%c", msg[i]);
            }
            printf("%s\n", COLOR_RESET);

            // For continuation lines, use more space (only indent 8 characters from the left margin)
            size_t contIndent = 8;                              // Much smaller indent for continuation lines
            size_t contWidth = terminalWidth - contIndent - 10; // More space for text

            // Print additional lines if needed
            size_t pos = lineLen;
            while (pos < msgLen)
            {
                // Print border with minimal indentation for continuation lines
                printf("%s%s     │%s", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);

                // Add a minimal indentation (just enough to show it's a continuation)
                printf("  ");

                // Calculate remaining length for this line with the wider available space
                size_t remainingLen = msgLen - pos;
                lineLen = remainingLen < contWidth ? remainingLen : contWidth;

                // Print this segment of the message
                printf("%s", BRIGHT_RED);
                for (size_t i = 0; i < lineLen; i++)
                {
                    printf("%c", msg[pos + i]);
                }
                printf("%s\n", COLOR_RESET);

                pos += lineLen;
            }
            printf("%s%s     │%s\n", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
        }
    }

    // Clean up
    free(lineBuf);
    free(lineStarts);
    free(lineLengths);
}

static void printCompilerDiagnostic(const char *compiler_file, const char *compiler_function, int compiler_line)
{
    if (compiler_file && compiler_function)
    {
        printf("%s%s%s  ", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
        printf("%sCompiler Error:%s %s%-60s%s\n",
               BOLD, COLOR_RESET,
               BRIGHT_RED, "Internal compiler error", COLOR_RESET);

        printf("%s%s%s  ", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
        printf("%sLocation:%s %sFile:%s %s%-50s%s\n",
               BOLD, COLOR_RESET,
               BOLD, COLOR_RESET,
               CYAN, compiler_file, COLOR_RESET);

        printf("%s%s%s  ", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
        printf("%sLocation:%s %sLine %d, Function%s %s%s%s\n",
               BOLD, COLOR_RESET,
               YELLOW, compiler_line, COLOR_RESET,
               GREEN, compiler_function, COLOR_RESET);
    }
}

static void printErrorDiagnostic(CryoErrorCode errorCode, CryoErrorSeverity severity,
                                 const char *message, const char *currentFile, size_t errorLine, size_t errorColumn)
{
    const char *errorCodeStr = CryoErrorCodeToString(errorCode);
    const char *severityStr = CryoErrorSeverityToString(severity);
    const char *severityColor = BRIGHT_RED;

    // Determine severity string and color
    switch (severity)
    {
    case CRYO_SEVERITY_NOTE:
        severityColor = CYAN;
        break;
    case CRYO_SEVERITY_WARNING:
        severityColor = YELLOW;
        break;
    case CRYO_SEVERITY_ERROR:
        severityColor = BRIGHT_RED;
        break;
    case CRYO_SEVERITY_FATAL:
        severityColor = BRIGHT_RED;
        break;
    default:
        break;
    }

    if (currentFile)
    {
        // Calculate available space for text
        size_t terminalWidth = 80;                          // Adjust based on your box width
        size_t prefixWidth = 15;                            // Space for border, padding, and severity label
        size_t wrapWidth = terminalWidth - prefixWidth - 5; // Safe margin

        // Print severity label
        printf("%s%s%s  ", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
        int labelLen = printf("%s(%s) %s:%s ",
                              severityColor, severityStr, errorCodeStr, COLOR_RESET);

        // Process message for word-wrapping
        const char *msg = message;
        size_t msgLen = strlen(msg);
        size_t pos = 0;
        size_t lineStart = 0;
        size_t lastSpace = 0;
        int isFirstLine = 1;

        // Process message word by word
        while (pos < msgLen)
        {
            // Find the next space or end of string
            while (pos < msgLen && msg[pos] != ' ')
            {
                pos++;
            }

            // If we've gone beyond the wrap width, print what we have so far
            if (pos - lineStart > wrapWidth || (pos < msgLen && msg[pos] == '\n'))
            {
                // If we found a space before reaching wrap width, break at that space
                size_t printEnd = (lastSpace > lineStart) ? lastSpace : pos;

                // Print current line segment
                if (isFirstLine)
                {
                    // First line already has the label printed
                    printf("%s", severityColor);
                    for (size_t i = lineStart; i < printEnd; i++)
                    {
                        printf("%c", msg[i]);
                    }
                    printf("%s\n", COLOR_RESET);
                    isFirstLine = 0;
                }
                else
                {
                    // Continuation line
                    printf("%s%s%s  ", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
                    // Add proper indentation
                    for (int i = 0; i < labelLen; i++)
                    {
                        printf(" ");
                    }
                    printf("%s", severityColor);
                    for (size_t i = lineStart; i < printEnd; i++)
                    {
                        printf("%c", msg[i]);
                    }
                    printf("%s\n", COLOR_RESET);
                }

                // Set the next line start (skip the space we broke at)
                lineStart = (lastSpace > lineStart && lastSpace < msgLen) ? lastSpace + 1 : pos;
                lastSpace = lineStart;
            }

            // Remember this space position
            if (pos < msgLen && msg[pos] == ' ')
            {
                lastSpace = pos;
                pos++;
            }
        }

        // Print any remaining text
        if (lineStart < msgLen)
        {
            if (isFirstLine)
            {
                printf("%s", severityColor);
                for (size_t i = lineStart; i < msgLen; i++)
                {
                    printf("%c", msg[i]);
                }
                printf("%s\n", COLOR_RESET);
            }
            else
            {
                printf("%s%s%s  ", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
                for (int i = 0; i < labelLen; i++)
                {
                    printf(" ");
                }
                printf("%s", severityColor);
                for (size_t i = lineStart; i < msgLen; i++)
                {
                    printf("%c", msg[i]);
                }
                printf("%s\n", COLOR_RESET);
            }
        }

        // File location line
        printf("%s%s%s  ", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
        printf("%sLocation:%s %sFile:%s %s%s%s\n",
               BOLD, COLOR_RESET,
               BOLD, COLOR_RESET,
               CYAN, currentFile, COLOR_RESET);

        // Line and column location
        printf("%s%s%s  ", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
        printf("%sLocation:%s %sLine %zu, Column %zu%s\n",
               BOLD, COLOR_RESET,
               YELLOW, errorLine, errorColumn, COLOR_RESET);
    }
}

void FrontendState_printErrorScreen(struct FrontendState_t *self, CryoErrorCode errorCode, CryoErrorSeverity severity,
                                    const char *message, const char *compiler_file, const char *compiler_function, int compiler_line)
{
    if (!self || !self->sourceCode)
        return;

    // Use provided line/column if specified, otherwise use current position
    size_t errorLine = self->currentLine;
    size_t errorColumn = self->currentColumn;
    const char *currentFile = self->currentFile ? self->currentFile : "Unknown file";
    const char *errorMessage = message ? message : "Unknown error";
    const size_t MAX_LINE_LENGTH = 120;

    // Top border with title - simplified approach
    printf("\n\n");
    printf("%s%s%s%s\n", LIGHT_RED, BOX_TOP_LEFT, BOX_HORIZONTAL, COLOR_RESET);

    // Title line
    printf("%s%s%s", LIGHT_RED, BOX_VERTICAL, COLOR_RESET);
    const char *title = "COMPILER ERROR";
    int title_length = strlen(title);
    int title_padding = 40 - title_length / 2;
    for (int i = 0; i < title_padding; i++)
        printf(" ");
    printf("%s%s%s", BRIGHT_RED, title, COLOR_RESET);
    for (int i = 0; i < title_padding; i++)
        printf(" ");
    printf("\n");

    // Compiler diagnostic section
    printCompilerDiagnostic(compiler_file, compiler_function, compiler_line);

    // Middle border
    printf("%s%s%s%s\n", LIGHT_RED, BOX_MIDDLE_LEFT, BOX_HORIZONTAL, COLOR_RESET);

    // Error diagnostic section
    printErrorDiagnostic(errorCode, severity, errorMessage, currentFile, errorLine, errorColumn);

    // Middle border
    printf("%s%s%s%s\n", LIGHT_RED, BOX_MIDDLE_LEFT, BOX_HORIZONTAL, COLOR_RESET);

    // Show context and error lines
    printErrorContext(self->sourceCode, errorLine, errorColumn, errorMessage, MAX_LINE_LENGTH);

    // Bottom border
    printf("%s%s%s%s\n\n", LIGHT_RED, BOX_BOTTOM_LEFT, BOX_HORIZONTAL, COLOR_RESET);
    printf("\n\n");
}

void GlobalDiagnosticsManager_printASTErrorScreen(GlobalDiagnosticsManager *self,
                                                  CryoErrorCode errorCode, CryoErrorSeverity severity,
                                                  const char *moduleName, ASTNode *failedNode,
                                                  const char *message,
                                                  const char *compiler_file, const char *compiler_function, int compiler_line)
{
}
