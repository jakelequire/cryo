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
#include "tools/utils/env.h"
#include "tools/utils/c_logger.h"
#include "tools/logger/logger_config.h"
#include "tools/utils/cryoProject.h"
#include "tools/utils/fs.h"

int main(int argc, char *argv[])
{
    // Initialize the logging system
    INIT_LOGS();

    // Initialize the global file system utilities
    initGlobalFS();

    // Get the parent directory of the compiler executable
    char *parent = getCompilerRootPath(argv[0]);
    if (parent)
    {
        logMessage(LMI, "INFO", "MAIN", "Parent directory: %s", parent);
    }

    // Initialize environment variables
    int envResult = initEnvVars(parent);
    if (envResult != 0)
    {
        fprintf(stderr, "Error: Failed to initialize environment variables\n");
        return 1;
    }
    free(parent); // Free the parent directory string

    // Initialize the compiler settings
    CompilerSettings settings = getCompilerSettings(argc, argv);
    initLoggerCompilerSettings(&settings); // Initialize the compiler settings for the logger
    updateEnabledLogs(g_enabledLogs, settings.enableLogs);
    if (settings.isSingleFile)
    {
        printf("Initializing as single file...\n");
        const char *dir = removeFileFromPath(settings.inputFilePath);
        INIT_PROJECT_CONFIG(dir, &settings);
    }
    else
    {
        printf("Initializing as project...\n");
        INIT_PROJECT_CONFIG(settings.rootDir, &settings);
    }
    printf("Compiler settings initialized\n");
    logCompilerSettings(&settings);

    // Initialize the build stats
    BuildStats *buildStats = createBuildStats();
    addCompilerSettings(buildStats, &settings);

    // Initialize the compile timer
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

    // Wrap up everything
    double elapsed = stopTimer(compileTimer);
    getSystemInfo(buildStats);
    addElapsedTime(buildStats, elapsed);
    printBuildStats(buildStats);

    CLEANUP_LOGS();

    exit(EXIT_SUCCESS);
}
