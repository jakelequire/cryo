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
#include "tools/utils/fs.h"

// This is being hard coded for now, but will be replaced with a config file later
CryoSrcLocations srcLocations[] = {
    {"/workspaces/cryo/"},
    {"/home/Phock/Programming/apps/cryo/"},
};

// <readFile>
/// @brief Takes in a file path and reads the contents of the file into a buffer
char *readFile(const char *path)
{
    printf("[FS] Reading file: %s\n", path);
    FILE *file = fopen(path, "rb"); // Open the file in binary mode to avoid transformations
    if (file == NULL)
    {
        perror("{FS} Could not open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    // handle fseek error
    fseek(file, 0, SEEK_SET);

    if (length == 0)
    {
        perror("{FS} File is empty");
        fclose(file);
        return NULL;
    }

    char *buffer = (char *)malloc(length + 1);
    if (buffer == NULL)
    {
        perror("{FS} Not enough memory to read file");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, length, file);
    if (bytesRead < length)
    {
        perror("{FS} Failed to read the full file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = '\0'; // Null-terminate the buffer
    fclose(file);
    return buffer;
}
// </readFile>

// <fileExists>
/// @brief Checks if a file exists at the given path
bool fileExists(const char *path)
{
    // Make sure the string isn't empty
    if (path == NULL)
    {
        return false;
    }

// Check if the file exists on Linux
// Using `access` seems to be a safer way to check if a file exists
#ifdef __linux__
    if (access(path, F_OK) != -1)
    {
        return true;
    }
    else
    {
        logMessage(LMI, "WARN", "FS", "File does not exist");
    }
    return false;
#endif

    // Open the file
    FILE *file = fopen(path, "r");
    if (file)
    {
        fclose(file);
        return true;
    }
    else
    {
        logMessage(LMI, "WARN", "FS", "File does not exist");
    }

    fclose(file);
    return false;
}
// </fileExists>

// <dirExists>
// @brief Checks if a directory exists at the given path
bool dirExists(const char *path)
{
    // Make sure the string isn't empty
    if (path == NULL)
    {
        return false;
    }

    // Open the file
    DIR *dir = opendir(path);
    if (dir)
    {
        closedir(dir);
        return true;
    }

    closedir(dir);
    return false;
}
// </dirExists>

// <createDir>
/// @brief Creates a directory at the given path
void createDir(const char *path)
{
    if (!dirExists(path))
    {
        logMessage(LMI, "INFO", "FS", "Directory doesn't exist, creating...");
        int status = mkdir(path, 0777);
        if (status == -1)
        {
            logMessage(LMI, "ERROR", "FS", "Failed to create directory");
            DEBUG_BREAKPOINT;
        }
    }

    return;
}
// </createDir>

// <removeFile>
/// @brief Removes a file at the given path
void removeFile(const char *filePath)
{
    if (fileExists(filePath))
    {
        int status = remove(filePath);
        if (status == -1)
        {
            logMessage(LMI, "ERROR", "FS", "Failed to remove file");
            DEBUG_BREAKPOINT;
        }
    }

    return;
}
// </removeFile>

// <getSTDFilePath>
const char *getSTDFilePath(const char *subModule)
{
    // Find the std library path from the environment (CRYO_PATH)
    const char *cryoPath = getenv("CRYO_PATH");
    if (!cryoPath)
    {
        logMessage(LMI, "ERROR", "TypeDefs", "CRYO_PATH environment variable not set.");
        DEBUG_BREAKPOINT;
        return NULL;
    }

    // The std library path will be $CRYO_PATH/cryo/std/{subModule}.cryo
    char *stdPath = (char *)malloc(strlen(cryoPath) + 16 + strlen(subModule) + 6);
    sprintf(stdPath, "%s/cryo/std/%s.cryo", cryoPath, subModule);

    return stdPath;
}
// </getSTDFilePath>

// <trimFilePath>
const char *trimFilePath(const char *filePath)
{
    // Trim the file path
    const char *fileName = strrchr(filePath, '/');
    return fileName;
}
// </trimFilePath>

// <getCurRootDir>
const char *getCurRootDir(void)
{
    // Get the current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to get current working directory");
        CONDITION_FAILED;
    }

    return cwd;
}
// </getCurRootDir>

// <getCryoSrcLocation>
const char *getCryoSrcLocation(void)
{
    int i = 0;
    while (srcLocations->rootDir[i] != NULL)
    {
        if (dirExists(srcLocations->rootDir))
        {
            logMessage(LMI, "INFO", "FS", "Found Cryo source location");
            return srcLocations->rootDir;
        }
        i++;
    }

    return NULL;
}
// </getCryoSrcLocation>

// <getCRuntimePath>
char *getCRuntimePath(void)
{
    // Find the path to the Cryo Compiler
    char *srcPath = (char *)getCryoSrcLocation();
    if (!srcPath)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to find Cryo source location");
        DEBUG_BREAKPOINT;
        return NULL;
    }

    // The runtime path will be $CRYO_PATH/src/runtime/
    char *runtimePath = (char *)malloc(strlen(srcPath) + 32);
    sprintf(runtimePath, "%s/src/runtime/", srcPath);

    printf("Runtime Path: %s\n", runtimePath);

    return runtimePath;
}
// </getCRuntimePath>

const char *appendStrings(const char *str1, const char *str2)
{
    char *result = (char *)malloc(strlen(str1) + strlen(str2) + 1);
    if (!result)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to allocate memory for string concatenation");
        return NULL;
    }

    strcpy(result, str1);
    strcat(result, str2);

    return result;
}
