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
#include "settings.h"

int DEBUG_LEVEL = 0;

void printUsage(const char *programName)
{
    fprintf(stderr, "Usage: %s -f <file> [options]\n", programName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -f, --file <path>    Specify input file path (required)\n");
    fprintf(stderr, "  -o, --output <path>  Specify output file path\n");
    fprintf(stderr, "  -s, --source <text>  Specify source text directly\n");
    fprintf(stderr, "  -a, --active-build  Flag that indicates the build is active\n");
    fprintf(stderr, "  -v, --verbose        Enable verbose output\n");
    fprintf(stderr, "  -d, --debug-level    Set the debug level (0-3)\n");
    fprintf(stderr, "  -h, --help           Display this help message\n");
}

static struct option long_options[] = {
    {"file", required_argument, 0, 'f'},
    {"source", required_argument, 0, 's'},
    {"active-build", no_argument, 0, 'a'},
    {"output", required_argument, 0, 'o'},
    {"verbose", no_argument, 0, 'v'},
    {"debug-level", required_argument, 0, 'd'},
    {"enable-logs", required_argument, 0, 'L'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}};

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
    settings->buildType = BUILD_NONE;
    settings->enabledLogs = createEnabledLogs();

    printf("Parsing Command Line Arguments\n");
    printf("Argc: %i\n", argc);
    printf("Argv: %s\n", argv[0]);

    int c;
    int option_index = 0;
    char *optstring = "f:s:o:avd:L:h";
    while ((c = getopt_long(argc, argv, "f:s:o:avd:L:h", long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'f':
            settings->inputFile = optarg;
            break;
        case 's':
            settings->isSource = true;
            settings->inputFile = optarg;
            break;
        case 'o':
            settings->customOutputPath = optarg;
            break;
        case 'a':
            settings->activeBuild = true;
            break;
        case 'v':
            settings->verbose = true;
            break;
        case 'd':
            settings->debugLevel = atoi(optarg);
            break;
        case 'L':
            parseEnabledLogsArgs(optarg, &settings->enabledLogs);
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
    printf("  Enabled Logs:\n");
    printf("    Lexer: %s\n", settings->enabledLogs.logLexer ? "true" : "false");
    printf("    Parser: %s\n", settings->enabledLogs.logParser ? "true" : "false");
    printf("    AST: %s\n", settings->enabledLogs.logAST ? "true" : "false");
    printf("    Symtable: %s\n", settings->enabledLogs.logSymtable ? "true" : "false");
    printf("    Compiler: %s\n", settings->enabledLogs.logCompiler ? "true" : "false");
    printf("    Utility: %s\n", settings->enabledLogs.logUtility ? "true" : "false");
    printf("    Arena: %s\n", settings->enabledLogs.logArena ? "true" : "false");
    printf("    Common: %s\n", settings->enabledLogs.logCommon ? "true" : "false");
    printf("    Settings: %s\n", settings->enabledLogs.logSettings ? "true" : "false");
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

const char *BuildTypeToString(BuildType type)
{
    switch (type)
    {
    case BUILD_NONE:
        return "NONE";
    case BUILD_DEBUG:
        return "DEBUG";
    case BUILD_RELEASE:
        return "RELEASE";
    default:
        return "UNKNOWN";
    }
}

EnabledLogs createEnabledLogs()
{
    EnabledLogs logs;
    logs.logLexer = false;
    logs.logParser = false;
    logs.logAST = false;
    logs.logSymtable = false;
    logs.logCompiler = false;
    logs.logUtility = false;
    logs.logArena = false;
    logs.logCommon = false;
    logs.logSettings = false;
    return logs;
}

CompilerSettings createCompilerSettings()
{
    CompilerSettings settings;
    settings.rootDir = getcwd(NULL, 0);
    settings.customOutputPath = NULL;
    settings.activeBuild = false;
    settings.verbose = false;
    settings.isSource = false;
    settings.customOutputPath = NULL;
    settings.debugLevel = DEBUG_NONE;
    settings.buildType = BUILD_NONE;
    settings.enabledLogs = createEnabledLogs();
    return settings;
}

EnabledLogs parseEnabledLogsArgs(const char *logArgs, EnabledLogs *logs)
{
    char *logStr = strdup(logArgs);
    char *token = strtok(logStr, ",");
    while (token != NULL)
    {
        if (strcmp(token, "LEXER") == 0)
        {
            printf("Lexer Enabled\n");
            logs->logLexer = true;
        }
        else if (strcmp(token, "PARSER") == 0)
        {
            printf("Parser Enabled\n");
            logs->logParser = true;
        }
        else if (strcmp(token, "AST") == 0)
        {
            printf("AST Enabled\n");
            logs->logAST = true;
        }
        else if (strcmp(token, "SYMTABLE") == 0)
        {
            printf("Symtable Enabled\n");
            logs->logSymtable = true;
        }
        else if (strcmp(token, "COMPILER") == 0)
        {
            printf("Compiler Enabled\n");
            logs->logCompiler = true;
        }
        else if (strcmp(token, "UTILITY") == 0)
        {
            printf("Utility Enabled\n");
            logs->logUtility = true;
        }
        else if (strcmp(token, "ARENA") == 0)
        {
            printf("Arena Enabled\n");
            logs->logArena = true;
        }
        else if (strcmp(token, "COMMON") == 0)
        {
            printf("Common Enabled\n");
            logs->logCommon = true;
        }
        else if (strcmp(token, "SETTINGS") == 0)
        {
            printf("Settings Enabled\n");
            logs->logSettings = true;
        }
        else
        {
            fprintf(stderr, "Unknown log type: %s\n", token);
        }
        token = strtok(NULL, ",");
    }
    free(logStr);

    return *logs;
}