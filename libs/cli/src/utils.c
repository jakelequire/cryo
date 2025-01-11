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
#include "./include/utils.h"

char *trimLastDir(char *path)
{
    char *last = strrchr(path, '/');
    if (last != NULL)
    {
        *last = '\0';
    }
    return path;
}

char *getAbsolutePath(char *path)
{
    char *absolute = realpath(path, NULL);
    return absolute;
}

char *concatStrings(char *str1, char *str2)
{
    char *result = (char *)malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}

bool stringCompare(char *str1, char *str2)
{
    return strcmp(str1, str2) == 0;
}

bool runSystemCommand(const char *command)
{
    int result = system(command);
    return result == 0;
}

char *getCompilerExePath(void)
{
    char *bin_path = getCompilerBinPath();
    if (bin_path == NULL)
    {
        return NULL;
    }

    char *compiler_path = concatStrings(bin_path, "compiler");
    free(bin_path);

    return compiler_path;
}

char *getCompilerBinPath(void)
{
    const char *command = "cryo-path";
    FILE *fp;
    char *path = NULL;
    size_t buffer_size = 256; // Start with reasonable buffer size
    size_t len = 0;

    // Allocate initial buffer
    path = (char *)malloc(buffer_size * sizeof(char));
    if (path == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    path[0] = '\0'; // Ensure null termination

    // Open the command for reading
    fp = popen(command, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to run command: %s\n", command);
        free(path);
        return NULL;
    }

    // Read the output, handling potential buffer resizing
    char *fgets_result = fgets(path, buffer_size, fp);
    if (fgets_result != NULL)
    {
        // Remove trailing newline if present
        len = strlen(path);
        if (len > 0 && path[len - 1] == '\n')
        {
            path[len - 1] = '\0';
            len--;
        }
    }
    else
    {
        fprintf(stderr, "Failed to read command output\n");
        free(path);
        pclose(fp);
        return NULL;
    }

    // Debug the output
    printf("Command output: %s\n", path);

    // Close the pipe
    int close_result = pclose(fp);
    if (close_result == -1)
    {
        fprintf(stderr, "Failed to close command pipe\n");
        free(path);
        return NULL;
    }

    // Check if command executed successfully
    if (WEXITSTATUS(close_result) != 0)
    {
        fprintf(stderr, "ERROR: Command Failed!\n");
        fprintf(stderr, "Command returned non-zero exit status: %d\n", WEXITSTATUS(close_result));
        free(path);
        return NULL;
    }

    // If path is empty after all this, something went wrong
    if (strlen(path) == 0)
    {
        fprintf(stderr, "Command returned empty path\n");
        free(path);
        return NULL;
    }

    return path;
}
