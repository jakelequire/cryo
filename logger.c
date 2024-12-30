#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define FILE_NAME_LENGTH 18
#define MODULE_NAME_LENGTH 15
#define FUNC_NAME_LENGTH 12
#define META_INFO_LENGTH 40
#define META_INFO_PADDING 40

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

#define GET_LOGGER_META_INFO \
    __LINE__, __FILE__, __func__, getParentDirOfFile(__FILE__)

void logMessage(
    int line,
    const char *file,
    const char *func,
    const char *parentDir,
    const char *type,
    const char *module,
    const char *message, ...)
{
    char *shortFile = stringShortener(getFileName(file), FILE_NAME_LENGTH, 0);
    char *shortModule = stringShortener(module, MODULE_NAME_LENGTH, 0);
    char *shortFunc = stringShortener(func, FUNC_NAME_LENGTH, 1);

    if (!shortFile || !shortModule || !shortFunc)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(shortFile);
        free(shortModule);
        free(shortFunc);
        return;
    }

    char metaInfo[META_INFO_LENGTH];
    snprintf(metaInfo, META_INFO_LENGTH, "<%s> [%d|%s:%s:%s", type, line, shortModule, shortFile, shortFunc);

    va_list args;
    va_start(args, message);
    printf("%-*s] | ", META_INFO_PADDING, metaInfo);
    vprintf(message, args);
    printf("\n");
    va_end(args);

    free(shortFile);
    free(shortModule);
    free(shortFunc);
}

void longFunctionNameThatIsVeryLongAndShouldBeShortened()
{
    logMessage(GET_LOGGER_META_INFO, "INFO", "Parser", "Parsing struct instance...");
}

int main()
{
    logMessage(GET_LOGGER_META_INFO, "INFO", "Parser", "Parsing struct instance...");

    longFunctionNameThatIsVeryLongAndShouldBeShortened();
    return 0;
}

/*
Clang Command to single compile and run:
clang-18 -o logger logger.c && ./logger
*/
