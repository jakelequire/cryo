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
#include "settings/compilerSettings.h"
int DEBUG_LEVEL = 0;

#define MAX_SOURCE_BUFFER 1024 * 1024 // 1MB

void printUsage(const char *programName)
{
    printf("\n");
    fprintf(stderr, "Usage: %s -f <file> [options]\n", programName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -f, --file <path>    Specify input file path (required)\n");
    fprintf(stderr, "  -o, --output <path>  Specify output file path\n");
    fprintf(stderr, "  -s, --source <text>  Specify source text directly\n");
    fprintf(stderr, "  -a, --active-build  Flag that indicates the build is active\n");
    fprintf(stderr, "  -v, --verbose        Enable verbose output\n");
    fprintf(stderr, "  -d, --debug-level    Set the debug level (0-3)\n");
    fprintf(stderr, "  -h, --help           Display this help message\n");
    printf("\n");
    printf("Advanced options:\n");
    printf("      --ast-dump         Dump AST to stdout\n");
    printf("      --ir-dump          Dump IR to stdout (UNIMPLEMENTED)\n");
    printf("\n");
}

static const struct option long_options[] = {
    // Existing options with their short equivalents
    {"file", required_argument, 0, 'f'},
    {"source", required_argument, 0, 's'},
    {"output", required_argument, 0, 'o'},
    {"active", no_argument, 0, 'a'},
    {"verbose", no_argument, 0, 'v'},
    {"debug", required_argument, 0, 'd'},
    {"logs", required_argument, 0, 'L'},
    {"help", no_argument, 0, 'h'},

    // Long-only options (no short equivalent)
    {"ast-dump", no_argument, 0, OPT_AST_DUMP},
    {"ir-dump", no_argument, 0, OPT_IR_DUMP},
    {0, 0, 0, 0} // Required terminator
};

CompilerSettings getCompilerSettings(int argc, char *argv[])
{
    CompilerSettings settings = createCompilerSettings();
    parseCommandLineArguments(argc, argv, &settings);
    return settings;
}

void parseCommandLineArguments(int argc, char **argv, CompilerSettings *settings)
{
    settings->verbose = false;
    settings->isSource = false;
    settings->customOutputPath = NULL;
    settings->debugLevel = DEBUG_NONE;
    settings->buildType = BUILD_DEV;

    printf("Parsing Command Line Arguments\n");
    printf("Argc: %i\n", argc);
    printf("Argv: %s\n", argv[0]);

    const char *inputFilePath = (const char *)malloc(sizeof(char) * 256);

    int c;
    int option_index = 0;
    char *optstring = "f:s:o:avd:L:h";
    while ((c = getopt_long(argc, argv, "f:s:o:avd:L:h", long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'f':
            settings->inputFile = optarg;
            inputFilePath = optarg;
            break;
        case 's':
            settings->isSource = true;
            settings->sourceText = optarg;
            break;
        case 'o':
        {
            // Get the current working directory that the program was run from
            char *cwd = getcwd(NULL, 0);
            // Get the length of the current working directory
            size_t cwdLen = strlen(cwd);
            // Get the length of the custom output path
            size_t outLen = strlen(optarg);
            // Allocate memory for the full output path
            char *outPath = (char *)malloc(cwdLen + outLen + 2);
            // Copy the current working directory to the full output path
            strcpy(outPath, cwd);
            // Concatenate the output path to the full output path
            char *providedPath = optarg;
            if (providedPath[0] == '.')
            {
                providedPath += 2;
            }
            strcat(outPath, "/");
            strcat(outPath, providedPath);
            // Set the custom output path
            printf("<!> Output Path: %s\n", strdup(outPath));
            settings->customOutputPath = outPath;
            break;
        }
        case 'a':
            settings->activeBuild = true;
            break;
        case 'v':
            settings->verbose = true;
            break;
        case 'd':
            settings->debugLevel = getDebugLevel(atoi(optarg));
            break;

        // Long-only options
        case OPT_AST_DUMP:
            settings->astDump = true;
            if (!inputFilePath)
            {
                fprintf(stderr, "Error: No input file specified\n");
                printUsage(argv[0]);
                exit(1);
            }
            executeASTDump(inputFilePath);
            break;

        case OPT_IR_DUMP:
            settings->irDump = true;
            if (settings->verbose)
            {
                printf("IR dump enabled\n");
            }
            break;

        case 'h':
            printUsage(argv[0]);
            exit(0);
            break;
        case '?':
            printUsage(argv[0]);
            exit(1);
            break;
        default:
            printUsage(argv[0]);
            exit(1);
            break;
        }
    }

    if (settings->inputFile == NULL)
    {
        fprintf(stderr, "Error: No input file specified\n");
        printUsage(argv[0]);
        exit(1);
    }

    char *fullFilePath = (char *)malloc(strlen(settings->rootDir) + strlen(settings->inputFile) + 2);
    strcpy(fullFilePath, settings->rootDir);
    strcat(fullFilePath, "/");
    // Trim the `.` from the input file
    if (settings->inputFile[0] == '.')
    {
        strcat(fullFilePath, settings->inputFile + 2);
    }
    settings->inputFilePath = fullFilePath;

    if (settings->debugLevel > 3)
    {
        fprintf(stderr, "Error: Debug level must be between 0 and 3\n");
        printUsage(argv[0]);
        exit(1);
    }
}
// ==============================
// Utility Functions

void logCompilerSettings(CompilerSettings *settings)
{
    printf("\n");
    printf("# ============ Compiler Settings ============ #\n");
    printf("  Root Directory: %s\n", settings->rootDir);
    printf("  Input File: %s\n", settings->inputFile);
    printf("  File Path: %s\n", settings->inputFilePath);
    printf("  Active Build: %s\n", settings->activeBuild ? "true" : "false");
    printf("  Debug Level: %s\n", DebugLevelToString(settings->debugLevel));
    printf("  Verbose: %s\n", settings->verbose ? "true" : "false");
    printf("  Custom Output Path: %s\n", settings->customOutputPath);
    printf("  Source Text: %s\n", settings->isSource ? "true" : "false");
    printf("# =========================================== #\n");
    printf("\n");
}

const char *DebugLevelToString(DebugLevel level)
{
    switch (level)
    {
    case DEBUG_NONE:
        return "NONE";
    case DEBUG_LOW:
        return "LOW";
    case DEBUG_MEDIUM:
        return "MEDIUM";
    case DEBUG_HIGH:
        return "HIGH";
    default:
        return "UNKNOWN";
    }
}

DebugLevel getDebugLevel(int level)
{
    switch (level)
    {
    case 0:
        return DEBUG_NONE;
    case 1:
        return DEBUG_LOW;
    case 2:
        return DEBUG_MEDIUM;
    case 3:
        return DEBUG_HIGH;
    default:
        return DEBUG_NONE;
    }
}

const char *BuildTypeToString(BuildType type)
{
    switch (type)
    {
    case BUILD_DEV:
        return "Development";
    case BUILD_DEBUG:
        return "Debug";
    case BUILD_RELEASE:
        return "Release";
    default:
        return "Unknown";
    }
}

CompiledFile createCompiledFile(void)
{
    CompiledFile file;
    file.fileName = NULL;
    file.filePath = NULL;
    file.outputPath = NULL;
    return file;
}

CompilerSettings createCompilerSettings(void)
{
    CompilerSettings settings;
    settings.rootDir = getcwd(NULL, 0);
    settings.customOutputPath = NULL;
    settings.activeBuild = false;
    settings.verbose = false;
    settings.isSource = false;
    settings.sourceText = (char *)malloc(sizeof(char) * MAX_SOURCE_BUFFER);
    settings.customOutputPath = NULL;
    settings.debugLevel = DEBUG_NONE;
    settings.buildType = BUILD_DEV;
    settings.compiledFiles = (CompiledFile **)malloc(sizeof(CompiledFile *) * 64);
    settings.version = COMPILER_VERSION;
    settings.totalFiles = 0;
    settings.astDump = false;
    settings.irDump = false;
    return settings;
}

void addCompiledFileToSettings(CompilerSettings *settings, CompiledFile *file)
{
    for (int i = 0; i < 64; ++i)
    {
        if (settings->compiledFiles[i] == NULL)
        {
            settings->compiledFiles[i] = file;
            break;
        }
    }
}

bool isASTDumpEnabled(CompilerSettings *settings)
{
    return settings->astDump;
}

bool isSourceText(CompilerSettings *settings)
{
    return settings->isSource;
}
