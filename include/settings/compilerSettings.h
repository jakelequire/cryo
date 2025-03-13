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
#include "tools/cryoconfig/cryoconfig.h"
#include "../../version.h"

#define COMPILER_VERSION _COMPILER_VERSION_
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL
#define SET_DEBUG_LEVEL(level) DEBUG_LEVEL = level

typedef struct CompiledFile CompiledFile;
typedef struct ProjectSettings ProjectSettings;

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

typedef enum
{
    OPT_AST_DUMP = 1000, // `--ast-dump`
    OPT_IR_DUMP,         // `--ir-dump`
    OPT_LSP_SYMBOLS,     // `--lsp-symbols`
    OPT_ENABLE_LOGS,     // `--enable-logs`
    OPT_DISABLE_LOGS,    // `--disable-logs`
    OPT_PROJECT          // `--project`
} LongOnlyOptions;

typedef struct CompilerSettings
{
    // Version
    const char *version;

    // Paths
    const char *projectDir;
    const char *rootDir;
    const char *customOutputPath;
    const char *compilerRootPath;
    const char *inputFile;
    const char *inputFilePath;
    const char *lspOutputPath; // {WORKSPACE}/build/lsp
    const char *runtimePath;
    const char *buildDir;
    char *sourceText;
    int totalFiles;

    // Flags
    bool astDump;
    bool irDump;
    bool isSource;
    bool activeBuild;
    bool isLSP;
    bool enableLogs;
    bool isProject;
    bool isSingleFile;
    bool verbose;

    // Debugging
    DebugLevel debugLevel;
    BuildType buildType;
    CompiledFile **compiledFiles;

    // Project Settings
    ProjectSettings *projectSettings;
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
CompiledFile createCompiledFile(void);
CompilerSettings createCompilerSettings();

void parseCommandLineArguments(int argc, char **argv, CompilerSettings *settings);

bool isASTDumpEnabled(CompilerSettings *settings);
bool isSourceText(CompilerSettings *settings);

#endif // COMPILER_SETTINGS_H
