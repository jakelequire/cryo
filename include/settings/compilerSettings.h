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
#ifndef COMPILER_SETTINGS_H
#define COMPILER_SETTINGS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>

#include "common/common.h"
#include "settings/compilerArgs.h"

#define COMPILER_VERSION "0.0.1"

typedef struct CompiledFile CompiledFile;

#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL
#define SET_DEBUG_LEVEL(level) DEBUG_LEVEL = level

typedef enum DebugLevel
{
    DEBUG_NONE = 0,
    DEBUG_LOW = 1,
    DEBUG_MEDIUM = 2,
    DEBUG_HIGH = 3
} DebugLevel;

typedef enum BuildType
{
    BUILD_DEV = 0,
    BUILD_DEBUG = 1,
    BUILD_RELEASE = 2
} BuildType;

typedef struct EnabledLogs
{
    bool logLexer;
    bool logParser;
    bool logAST;
    bool logSymtable;
    bool logCompiler;
    bool logUtility;
    bool logArena;
    bool logCommon;
    bool logSettings;
} EnabledLogs;

typedef enum
{
    OPT_AST_DUMP = 1000, // Start after ASCII range to avoid conflicts
    OPT_IR_DUMP,
} LongOnlyOptions;

typedef struct CompilerSettings
{
    const char *rootDir;
    const char *customOutputPath;
    const char *inputFile;
    const char *inputFilePath;
    bool activeBuild;
    bool isSource;
    bool verbose;
    EnabledLogs enabledLogs;
    DebugLevel debugLevel;
    /**
     * typedef enum BuildType
     * {
     *     BUILD_DEV = 0, // Default
     *     BUILD_DEBUG = 1,
     *     BUILD_RELEASE = 2
     * } BuildType;
     */
    BuildType buildType;
    CompiledFile **compiledFiles;
    int totalFiles;

    // Flags
    bool astDump;
    bool irDump;

    // Version
    const char *version;
} CompilerSettings;

// ==============================
// Function Prototypes
void printUsage(const char *programName);
CompilerSettings getCompilerSettings(int argc, char *argv[]);

// ==============================
// Utility Functions
void logCompilerSettings(CompilerSettings *settings);
const char *DebugLevelToString(DebugLevel level);
DebugLevel getDebugLevel(int level);
const char *BuildTypeToString(BuildType type);
void addCompiledFileToSettings(CompilerSettings *settings, CompiledFile *file);
EnabledLogs createEnabledLogs(void);
CompiledFile createCompiledFile(void);
EnabledLogs parseEnabledLogsArgs(const char *logArgs, EnabledLogs *logs);
CompilerSettings createCompilerSettings();

void parseCommandLineArguments(int argc, char **argv, CompilerSettings *settings);

bool isASTDumpEnabled(CompilerSettings *settings);

#endif // COMPILER_SETTINGS_H
