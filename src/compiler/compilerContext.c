/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "linker/linker.hpp"
#include "compiler/compiler.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

CompilerContext *compilerCtx = NULL;

CCTXPragma *initCCTXPragma(void)
{
    CCTXPragma *pragma = (CCTXPragma *)malloc(sizeof(CCTXPragma));
    if (!pragma)
    {
        fprintf(stderr, "Error: Failed to allocate CCTXPragma\n");
        return NULL;
    }

    pragma->internal = false;

    return pragma;
}

CCTXFile *initCCTXFile(void)
{
    CCTXFile *file = (CCTXFile *)malloc(sizeof(CCTXFile));
    if (!file)
    {
        fprintf(stderr, "Error: Failed to allocate CCTXFile\n");
        return NULL;
    }

    file->fileName = NULL;
    file->filePath = NULL;

    return file;
}

CompilerContext *initCompilerContext(void)
{
    CompilerContext *ctx = (CompilerContext *)malloc(sizeof(CompilerContext));
    if (!ctx)
    {
        fprintf(stderr, "Error: Failed to allocate CompilerContext\n");
        return NULL;
    }

    ctx->pragma = initCCTXPragma();
    ctx->file = initCCTXFile();

    return ctx;
}
