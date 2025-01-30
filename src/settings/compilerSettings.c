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
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"
int DEBUG_LEVEL = 0;

#define MAX_SOURCE_BUFFER 1024 * 1024 // 1MB

void printUsage(const char *programName)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    printf("\n");
    fprintf(stderr, "Usage: %s -f <file> [options]\n", programName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -f, --file <path>    Specify input file path (required)\n");
    fprintf(stderr, "  -o, --output <path>  Specify output file path\n");
    fprintf(stderr, "  -s, --source <text>  Specify source text directly\n");
    fprintf(stderr, "  -a, --active-build  Flag that indicates the build is active\n");
    fprintf(stderr, "  -v, --verbose        Enable verbose output\n");
    fprintf(stderr, "  -d, --debug-level    Set the debug level (0-3)\n");
    fprintf(stderr, "  -L, --logs           Enable logs\n");
    fprintf(stderr, "  -p, --project        Specify project directory\n");
    fprintf(stderr, "  -h, --help           Display this help message\n");
    printf("\n");
    printf("Advanced options:\n");
    printf("      --ast-dump         Dump AST to stdout\n");
    printf("      --ir-dump          Dump IR to stdout (UNIMPLEMENTED)\n");
    printf("      --lsp-symbols      Compile and connect to LSP server\n");
    printf("      --enable-logs      Enable logs\n");
    printf("      --disable-logs     Disable logs\n");
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
    {"project", required_argument, 0, 'p'},
    {"help", no_argument, 0, 'h'},

    // Long-only options (no short equivalent)
    {"ast-dump", no_argument, 0, OPT_AST_DUMP},
    {"ir-dump", no_argument, 0, OPT_IR_DUMP},
    {"lsp-symbols", no_argument, 0, OPT_LSP_SYMBOLS},
    // Enable Logs: --enable-logs
    {"enable-logs", no_argument, 0, OPT_ENABLE_LOGS},
    // Disable Logs: --disable-logs
    {"disable-logs", no_argument, 0, OPT_DISABLE_LOGS},
    {"project", required_argument, 0, OPT_PROJECT},

    {0, 0, 0, 0} // Required terminator
};

CompilerSettings getCompilerSettings(int argc, char *argv[])
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    CompilerSettings settings = createCompilerSettings();
    parseCommandLineArguments(argc, argv, &settings);
    return settings;
}

void parseCommandLineArguments(int argc, char **argv, CompilerSettings *settings)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    settings->verbose = false;
    settings->isSource = false;
    settings->customOutputPath = NULL;
    settings->debugLevel = DEBUG_NONE;
    settings->buildType = BUILD_DEV;
    settings->compilerRootPath = getCompilerRootPath();
    settings->isProject = false;
    settings->version = COMPILER_VERSION;

    const char *runtimePath = appendStrings(getCompilerRootPath(), "/Std/Runtime/");
    printf("Runtime Path: %s\n", runtimePath);
    settings->runtimePath = runtimePath;

    const char *inputFilePath = (const char *)malloc(sizeof(char) * 256);

    printf("Getting compiler settings...\n");
    int c;
    int option_index = 0;
    char *optstring = "f:s:o:avd:L:p:h";
    while ((c = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1)
    {
        switch (c)
        {
        // The `-f` option is used to specify the input file (Single file mode)
        case 'f':
        {
            settings->isSingleFile = true;
            settings->inputFile = optarg;
            inputFilePath = realpath(optarg, NULL);
            break;
        }
        // The `-s` option is used to specify source text directly
        case 's':
        {
            settings->isSource = true;
            settings->sourceText = optarg;
            break;
        }
        // The `-o` option is used to specify a custom output path
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
        // The `-a` option is used to indicate that the build is active (deprecated)
        case 'a':
        {
            settings->activeBuild = true;
            break;
        }
        // The `-v` option is used to enable verbose output (deprecated)
        case 'v':
        {
            settings->verbose = true;
            break;
        }
        // The `-d` option is used to set the debug level (unimplemented)
        case 'd':
        {
            settings->debugLevel = getDebugLevel(atoi(optarg));
            break;
        }
            // The `-p` option is used to indicate that we are in project mode
        case 'p':
        {
            logMessage(LMI, "INFO", "Compiler Settings", "Initializing as project...");
            settings->isProject = true;
            settings->isSingleFile = false;
            settings->inputFile = NULL;
            char *projectDir = realpath(optarg, NULL);
            if (projectDir == NULL)
            {
                fprintf(stderr, "Error: Invalid project directory\n");
                exit(1);
            }
            settings->projectDir = projectDir;

            break;
        }
        // Usage and help
        case 'h':
        {
            printUsage(argv[0]);
            exit(0);
            break;
        }
        case '?':
        {
            printUsage(argv[0]);
            exit(1);
            break;
        }

        // Long-only options
        // The `--ast-dump` option is used to dump the AST to stdout
        case OPT_AST_DUMP:
        {
            settings->astDump = true;
            if (!inputFilePath)
            {
                fprintf(stderr, "Error: No input file specified\n");
                printUsage(argv[0]);
                exit(1);
            }
            executeASTDump(inputFilePath);
            break;
        }
        // The `--ir-dump` option is used to dump the IR to stdout
        case OPT_IR_DUMP:
        {
            settings->irDump = true;
            if (settings->verbose)
            {
                printf("IR dump enabled\n");
            }
            break;
        }
        // The `--lsp-symbols` option is used to compile and connect to the LSP server
        case OPT_LSP_SYMBOLS:
        {
            settings->isLSP = true;
            if (settings->verbose)
            {
                printf("LSP Symbols enabled\n");
            }
            if (optind >= argc)
            {
                fprintf(stderr, "Error: No input file specified for LSP symbols\n");
                printUsage(argv[0]);
                exit(1);
            }
            settings->lspOutputPath = argv[optind];
            break;
        }
        // The `--enable-logs` option is used to enable logs
        case OPT_ENABLE_LOGS:
        {
            settings->enableLogs = true;
            break;
        }
        // The `--disable-logs` option is used to disable logs
        case OPT_DISABLE_LOGS:
        {
            settings->enableLogs = false;
            DISABLE_ALL_LOGS();
            break;
        }

        default:
        {
            printf("Error: Unknown option: %c\n", c);
            printUsage(argv[0]);
            exit(1);
            break;
        }
        }
    }

    if (settings->inputFile == NULL && settings->isSingleFile == true)
    {
        fprintf(stderr, "Error: No input file specified\n");
        printUsage(argv[0]);
        exit(1);
    }

    printf("Finished parsing command line arguments.. initializing compiler settings...\n");

    if (!settings->isSingleFile && settings->isProject)
    {
        printf("Completed project initialization, initializing as project...\n");
        // Set null values for settings we don't need to use
        settings->inputFile = NULL;
        settings->inputFilePath = NULL;
        settings->isSingleFile = false;
        settings->isProject = true;
        char *projectBuildDir = concatStrings(settings->projectDir, "/build");
        settings->buildDir = projectBuildDir;

        return;
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

    if (settings->isSingleFile)
    {
        settings->projectDir = NULL;
        const char *inputFile = removeFileFromPath(settings->inputFilePath);
        const char *buildDir = appendStrings(inputFile, "/build");
        settings->buildDir = buildDir;
        printf("Initialized build directory: %s\n", buildDir);
    }
}

// ==============================
// Utility Functions

void logCompilerSettings(CompilerSettings *settings)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    const char *trueFlag = BOLD GREEN "true" COLOR_RESET;
    const char *falseFlag = BOLD RED "false" COLOR_RESET;
    DEBUG_PRINT_FILTER({
        printf("Printing compiler settings...\n");
        printf("\n");
        printf("# ============ Compiler Settings ============ #\n");
        printf("  Root Directory: %s\n", settings->rootDir);
        printf("  Project Directory: %s\n", settings->projectDir);
        printf("  Build Directory: %s\n", settings->buildDir);
        printf("  Runtime Path: %s\n", settings->runtimePath);
        printf("  Compiler Root Path: %s\n", settings->compilerRootPath);
        printf("  Input File: %s\n", settings->inputFile);
        printf("  File Path: %s\n", settings->inputFilePath);
        printf("  Debug Level: %s\n", DebugLevelToString(settings->debugLevel));
        printf("  Custom Output Path: %s\n", settings->customOutputPath);
        printf(" ----------------------\n");
        printf(" Flags:\n");
        printf("  AST Dump: %s\n", settings->astDump ? trueFlag : falseFlag);
        printf("  IR Dump: %s\n", settings->irDump ? trueFlag : falseFlag);
        printf("  LSP Symbols: %s\n", settings->isLSP ? trueFlag : falseFlag);
        printf("  Enable Logs: %s\n", settings->enableLogs ? trueFlag : falseFlag);
        printf("  Disable Logs: %s\n", !settings->enableLogs ? trueFlag : falseFlag);
        printf("  Is Single File: %s\n", settings->isSingleFile ? trueFlag : falseFlag);
        printf("  Is Project: %s\n", settings->isProject ? trueFlag : falseFlag);
        printf("  Source Text: %s\n", settings->isSource ? trueFlag : falseFlag);
        printf("  Active Build: %s\n", settings->activeBuild ? trueFlag : falseFlag);
        printf("# =========================================== #\n");
        printf("\n");
    });
}

const char *DebugLevelToString(DebugLevel level)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
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
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
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
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
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
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    CompiledFile file;
    file.fileName = NULL;
    file.filePath = NULL;
    file.outputPath = NULL;
    return file;
}

CompilerSettings createCompilerSettings(void)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    CompilerSettings settings;
    settings.projectDir = (char *)malloc(sizeof(char) * 1024);
    settings.runtimePath = (char *)malloc(sizeof(char) * 1024);
    settings.inputFile = (char *)malloc(sizeof(char) * 1024);
    settings.inputFilePath = (char *)malloc(sizeof(char) * 1024);
    settings.lspOutputPath = (char *)malloc(sizeof(char) * 1024);
    settings.buildDir = (char *)malloc(sizeof(char) * 1024);
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
    settings.isLSP = false;
    settings.isSingleFile = false;
    settings.isProject = false;

    return settings;
}

void addCompiledFileToSettings(CompilerSettings *settings, CompiledFile *file)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
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
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    return settings->astDump;
}

bool isSourceText(CompilerSettings *settings)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    return settings->isSource;
}
