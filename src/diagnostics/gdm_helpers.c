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

#define MAX_NUM_FRAMES 16

void addLexer(GlobalDiagnosticsManager *self, Lexer *lexer)
{
    self->frontendState->lexer = lexer;
    self->frontendState->isLexerSet = true;
}

void create_stack_frame(GlobalDiagnosticsManager *self, char *functionName, char *filename, int line)
{
    if (!functionName || !filename)
    {
        fprintf(stderr, "Error: Invalid arguments for newStackFrame\n");
        return;
    }

    StackFrame *frame = (StackFrame *)malloc(sizeof(StackFrame));
    if (!frame)
    {
        fprintf(stderr, "Error: Failed to allocate memory for stack frame\n");
        return;
    }
    frame->functionName = functionName;
    frame->filename = filename;
    frame->line = line;

    dyn_stackframe_push(frame);
}

void dyn_stackframe_push(StackFrame *frame)
{
    StackTrace *stackTrace = GDM->stackTrace;
    if (stackTrace->frameCount == stackTrace->frameCapacity)
    {
        printf("Resizing stack trace\n");
        stackTrace->frameCapacity *= 2;
        stackTrace->frames = (StackFrame **)realloc(stackTrace->frames, sizeof(StackFrame *) * stackTrace->frameCapacity);
    }

    stackTrace->frames[stackTrace->frameCount++] = frame;
}

void print_stack_trace(GlobalDiagnosticsManager *self)
{
    size_t numFrames = MAX_NUM_FRAMES;
    StackTrace *stackTrace = self->stackTrace;
    size_t start = stackTrace->frameCount > numFrames ? stackTrace->frameCount - numFrames : 0;
    size_t last = stackTrace->frameCount > numFrames ? numFrames : stackTrace->frameCount;

    // First pass: find the maximum length of filename+line combination
    size_t maxLength = 0;
    for (size_t i = start; i < stackTrace->frameCount; i++)
    {
        StackFrame *frame = stackTrace->frames[i];
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s[%d]", frame->filename, frame->line);
        size_t len = strlen(buffer);
        if (len > maxLength)
        {
            maxLength = len;
        }
    }

    // Calculate total visible frames for reverse numbering
    size_t totalFrames = last;

    // Second pass: print with padding and reverse numbering
    size_t forward_index = 0;
    for (size_t i = start; i < stackTrace->frameCount; i++)
    {
        StackFrame *frame = stackTrace->frames[i];
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s[%d]", frame->filename, frame->line);

        // Calculate padding needed
        size_t padding = maxLength - strlen(buffer);
        char padStr[256] = {0};
        memset(padStr, ' ', padding);

        // Use reverse index for display
        size_t display_index = totalFrames - forward_index - 1;

        // Check for the last frame and color it red
        if (forward_index == last - 1)
        {
            printf(BOLD RED "+-------------------------------------------------------------------------------------------------+\n");
            printf("| (%lu) %s%s - " BOLD CYAN "%s\n" COLOR_RESET,
                   display_index, buffer, padStr, frame->functionName);
            printf(BOLD RED "+-------------------------------------------------------------------------------------------------+\n" COLOR_RESET);
            break;
        }
        printf("+-------------------------------------------------------------------------------------------------+\n");
        printf("| (%lu) %s%s - " BOLD CYAN "%s\n" COLOR_RESET,
               display_index, buffer, padStr, frame->functionName);
        forward_index++;
    }
    printf("Total frames: %lu\n", stackTrace->frameCount);
}
