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

// =============================================================================

FrontendState *newFrontendState(void)
{
    FrontendState *state = (FrontendState *)malloc(sizeof(FrontendState));
    state->lexer = NULL;
    state->isLexerSet = false;
    return state;
}

CryoError *newCryoError(CryoErrorType type, CryoErrorSeverity severity, CryoErrorCode code)
{
    CryoError *error = (CryoError *)malloc(sizeof(CryoError));
    error->type = type;
    error->severity = severity;
    error->code = code;

    return error;
}

DiagnosticEntry *newDiagnosticEntry(CryoErrorCode *err, CompilerInternalError *internalErr, CryoErrorInfo *cryoErrInfo)
{
    DiagnosticEntry *entry = (DiagnosticEntry *)malloc(sizeof(DiagnosticEntry));
    entry->err = err;
    entry->internalErr = internalErr;
    entry->cryoErrInfo = cryoErrInfo;
    entry->isInternalError = internalErr != NULL;
    entry->isCryoError = cryoErrInfo != NULL;

    return entry;
}

CompilerInternalError *newCompilerInternalError(char *function, char *filename, int line, int column, char *message)
{
    CompilerInternalError *error = (CompilerInternalError *)malloc(sizeof(CompilerInternalError));
    error->filename = filename;
    error->function = function;
    error->line = line;
    error->message = message;

    return error;
}

CryoErrorInfo *newCryoErrorInfo(char *filename, int line, int column, char *message)
{
    CryoErrorInfo *info = (CryoErrorInfo *)malloc(sizeof(CryoErrorInfo));
    info->filename = filename;
    info->line = line;
    info->column = column;
    info->message = message;

    return info;
}

StackFrame *newStackFrame(char *functionName, char *filename, int line)
{
    StackFrame *frame = (StackFrame *)malloc(sizeof(StackFrame));
    frame->functionName = functionName;
    frame->filename = filename;
    frame->line = line;

    return frame;
}

StackTrace *newStackTrace(void)
{
    StackTrace *trace = (StackTrace *)malloc(sizeof(StackTrace));
    trace->frames = (StackFrame **)malloc(sizeof(StackFrame *) * STACK_TRACE_CAPACITY);
    trace->frameCount = 0;
    trace->frameCapacity = STACK_TRACE_CAPACITY;

    trace->push = dyn_stackframe_push;

    return trace;
}
