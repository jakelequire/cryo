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
#include "tools/utils/c_logger.h"
#include <malloc.h>
#include <stdarg.h>
#include <stdbool.h>

#define FILE_NAME_LENGTH 18
#define MODULE_NAME_LENGTH 15
#define FUNC_NAME_LENGTH 15
#define META_INFO_LENGTH 50
#define META_INFO_PADDING 50
#define TYPE_BUFFER_PADDING 10

CompilerSettings *compilerSettings;

typedef struct EnabledLogs
{
    bool all;
    bool none;
    bool info;
    bool error;
    bool warn;
    bool debug;
} EnabledLogs;

typedef struct LoggerMessage
{
    int line;
    const char *file;
    const char *func;
    const char *type;
    const char *module;
    const char *message;
} LoggerMessage;

LoggerMessage *createLoggerMessage(int line, const char *file, const char *func, const char *type, const char *module, const char *message)
{
    LoggerMessage *loggerMessage = (LoggerMessage *)malloc(sizeof(LoggerMessage));
    if (!loggerMessage)
    {
        return NULL;
    }

    loggerMessage->line = line;
    loggerMessage->file = file;
    loggerMessage->func = func;
    loggerMessage->type = type;
    loggerMessage->module = module;
    loggerMessage->message = message;

    return loggerMessage;
}

void outputNewLogBuffer(int line,
                        const char *file,
                        const char *func,
                        const char *type,
                        const char *module,
                        const char *message, ...)
{
    if (!compilerSettings)
    {
        fprintf(stderr, "Compiler settings not found\n");
        return;
    }
    // const char *rootDir = compilerSettings->rootDir;
    // const char *logDir = "build/debug/logs";
    // const char *fullLogPath = appendPathToFileName(rootDir, logDir, true);
    // const char *logFileName = "compiler.log";
    // const char *logFilePath = appendPathToFileName(fullLogPath, logFileName, true);

    // if (!dirExists(fullLogPath))
    // {
    //     createDir(fullLogPath);
    // }
    //
    // FILE *logFile = fopen(logFilePath, "a");
    // if (!logFile)
    // {
    //     fprintf(stderr, "Failed to open log file: %s (errno: %d: %s)\n",
    //             logFilePath, errno, strerror(errno));
    //     return;
    // }
    //
    // char *shortFile = stringShortener(getFileName(file), FILE_NAME_LENGTH, 0);
    // char *shortModule = stringShortener(module, MODULE_NAME_LENGTH, 0);
    // char *shortFunc = stringShortener(func, FUNC_NAME_LENGTH, 0);
    //
    // if (!shortFile || !shortModule || !shortFunc)
    // {
    //     fprintf(stderr, "Memory allocation failed\n");
    //     fclose(logFile);
    //     free(shortFile);
    //     free(shortModule);
    //     free(shortFunc);
    //     return;
    // }
    //
    // char metaInfo[META_INFO_LENGTH];
    // char *numberFormatBuffer = (char *)malloc(10);
    // sprintf(numberFormatBuffer, "%d", line);
    //
    // snprintf(metaInfo, META_INFO_LENGTH, "<%-3s|%s:%s:%s",
    //          "", numberFormatBuffer, shortModule, shortFile, shortFunc);
    //
    // // Format the message with variable arguments
    // char formattedMessage[4096]; // Choose an appropriate size
    // va_list args;
    // va_start(args, message);
    // vsnprintf(formattedMessage, sizeof(formattedMessage), message, args);
    // va_end(args);
    //
    // // Write to file
    // fprintf(logFile, "%-*s | %s\n", META_INFO_PADDING, metaInfo, formattedMessage);
    //
    // free(shortFile);
    // free(shortModule);
    // free(shortFunc);
    // free(numberFormatBuffer);
    //
    // fclose(logFile);
}

void initCompilerSettings(CompilerSettings *settings)
{
    compilerSettings = settings;
    printf("Found compiler settings\n");
    logCompilerSettings(compilerSettings);
}

char *stringShortener(const char *string, int length, int addDots)
{
    char *shortened = (char *)malloc(length + 1 + (addDots ? 3 : 0));
    if (!shortened)
    {
        return NULL;
    }

    if (addDots && strlen(string) > length)
    {
        strncpy(shortened, string, length - 3);
        strcat(shortened, "...");
    }
    else
    {
        strncpy(shortened, string, length);
        shortened[length] = '\0';
    }

    return shortened;
}

char *getFileName(const char *file)
{
    const char *lastSlash = strrchr(file, '/');
    const char *filename = lastSlash ? lastSlash + 1 : file;

    char *trimmed = strdup(filename);
    char *dot = strrchr(trimmed, '.');
    if (dot)
    {
        *dot = '\0';
    }
    return trimmed;
}

const char *getParentDirOfFile(const char *file)
{
    const char *lastSlash = strrchr(file, '/');
    if (!lastSlash)
    {
        return "";
    }

    static char parentDir[256];
    strncpy(parentDir, file, lastSlash - file);
    parentDir[lastSlash - file] = '\0';

    return parentDir;
}

const char *typeBufferFormatter(const char *type)
{
    char *buffer = (char *)malloc(sizeof(char) * 1024);

    if (strcmp(type, "INFO") == 0)
    {
        snprintf(buffer, 1024, "%s%s%s", GREEN, type, COLOR_RESET);
        return buffer;
    }
    else if (strcmp(type, "ERROR") == 0)
    {
        snprintf(buffer, 1024, "%s%s%s", RED, type, COLOR_RESET);
        return buffer;
    }
    else if (strcmp(type, "WARN") == 0)
    {
        snprintf(buffer, 1024, "%s%s%s", YELLOW, type, COLOR_RESET);
        return buffer;
    }
    else if (strcmp(type, "DEBUG") == 0)
    {
        snprintf(buffer, 1024, "%s%s%s", CYAN, type, COLOR_RESET);
        return buffer;
    }
    else if (strcmp(type, "CRITICAL") == 0)
    {
        snprintf(buffer, 1024, "%s%s%s", LIGHT_RED, type, COLOR_RESET);
        return buffer;
    }
    else
    {
        snprintf(buffer, 1024, "%s%s%s", WHITE, type, COLOR_RESET);
        return buffer;
    }

    return buffer;
}

void logMessage(
    int line,
    const char *file,
    const char *func,
    const char *type,
    const char *module,
    const char *message, ...)
{

    char *shortFile = stringShortener(getFileName(file), FILE_NAME_LENGTH, 0);
    char *shortModule = stringShortener(module, MODULE_NAME_LENGTH, 0);
    char *shortFunc = stringShortener(func, FUNC_NAME_LENGTH, 0);

    if (!shortFile || !shortModule || !shortFunc)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(shortFile);
        free(shortModule);
        free(shortFunc);
        return;
    }

    char metaInfo[META_INFO_LENGTH];
    const char *typeFormatBuffer = typeBufferFormatter(type);
    char *numberFormatBuffer = (char *)malloc(10);
    // Number format should be light cyan
    sprintf(numberFormatBuffer, "%s%d%s", LIGHT_CYAN, line, COLOR_RESET);

    snprintf(metaInfo, META_INFO_LENGTH, "<%s>%-3s|%s:%s:%s",
             typeFormatBuffer, "", numberFormatBuffer, shortModule, shortFile, shortFunc);

    va_list args;
    va_start(args, message);
    printf("%-*s | ", META_INFO_PADDING, metaInfo);
    vprintf(message, args);
    printf("\n");
    va_end(args);

    // va_start(args, message);
    // outputNewLogBuffer(line, file, func, type, module, message, args);
    // va_end(args);

    free(shortFile);
    free(shortModule);
    free(shortFunc);
    free(numberFormatBuffer);
}

// logMessage(LMI, "INFO", "Parser", "Parsing struct instance...");
