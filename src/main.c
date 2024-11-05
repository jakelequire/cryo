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
#include <stdio.h>
#include <stdlib.h>

#include "settings/compilerSettings.h"
#include "compiler/compiler.h"
#include "tools/utils/compileTimer.h"
#include "tools/utils/buildStats.h"

int main(int argc, char *argv[])
{
    // Initialize the compiler settings
    CompilerSettings settings = getCompilerSettings(argc, argv);
    logCompilerSettings(&settings);

    BuildStats *buildStats = createBuildStats();
    addCompilerSettings(buildStats, &settings);

    CompileTimer *compileTimer = createCompileTimer();
    startTimer(compileTimer);

    // Check if the input file exists
    const char *filePath = settings.inputFilePath;
    if (!fileExists(filePath))
    {
        fprintf(stderr, "Error: File not found: %s\n", filePath);
        return 1;
    }

    // Compile the file
    int compilerResult = cryoCompiler(filePath, &settings);
    if (compilerResult != 0)
    {
        CONDITION_FAILED;
        return 1;
    }

    double elapsed = stopTimer(compileTimer);
    getSystemInfo(buildStats);
    addElapsedTime(buildStats, elapsed);
    printBuildStats(buildStats);

    exit(EXIT_SUCCESS);
}
