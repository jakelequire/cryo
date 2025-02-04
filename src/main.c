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
#include "diagnostics/diagnostics.h"
#include "tools/utils/compileTimer.h"
#include "tools/utils/buildStats.h"
#include "tools/utils/env.h"
#include "tools/utils/c_logger.h"
#include "tools/logger/logger_config.h"
#include "tools/cryoconfig/cryoconfig.h"
#include "tools/utils/fs.h"

#define MAX_PATH_SIZE 1024 * 10

#define INIT_LIBS()                                   \
    /* Initialize the logging system */               \
    INIT_LOGS();                                      \
    /* Initialize the global diagnostics manager */   \
    INIT_GDM();                                       \
    /* Initialize the global file system utilities */ \
    INIT_FS();

#define CLEANUP_COMPILATION() \
    /* Logs Destructor */     \
    CLEANUP_LOGS();

int main(int argc, char *argv[])
{
    // Initialize the libraries and tools
    INIT_LIBS();
    __STACK_FRAME__

    // Initialize the compile timer
    START_COMPILER_TIMER

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
    if (!&settings)
    {
        fprintf(stderr, "Error: Failed to initialize compiler settings\n");
        return 1;
    }

    initLoggerCompilerSettings(&settings); // Initialize the compiler settings for the logger
    INIT_LOGS_WITH_SETTINGS(&settings);    // Initialize the logger with the compiler settings
    INIT_PROJECT_CONFIG(&settings);        // Initialize the project configuration

    // Initialize the build stats
    BuildStats *buildStats = createBuildStats();
    addCompilerSettings(buildStats, &settings);

    // Entry point for compilation
    int compileResult = cryoCompile(&settings);
    if (compileResult != 0)
    {
        CONDITION_FAILED;
        return 1;
    }

    // Stop the compile timer and print the build stats
    STOP_COMPILER_TIMER

    // Cleanup the compilation
    CLEANUP_COMPILATION();

    exit(EXIT_SUCCESS);
}
