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
#include "tools/logger/logger_config.h"

#define PATH_MAX 4096

jFS *fs = NULL;

jFS *initFS(void)
{
    jFS *fs = (jFS *)malloc(sizeof(jFS));
    if (!fs)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to allocate memory for FS");
        return NULL;
    }

    fs->readFile = readFile;
    fs->fileExists = fileExists;
    fs->dirExists = dirExists;
    fs->createDir = createDir;
    fs->removeFile = removeFile;
    fs->getSTDFilePath = getSTDFilePath;
    fs->trimFilePath = trimFilePath;
    fs->getFileNameFromPathNoExt = getFileNameFromPathNoExt;
    fs->getFileNameFromPath = getFileNameFromPath;
    fs->getCurRootDir = getCurRootDir;
    fs->getCryoSrcLocation = getCryoSrcLocation;
    fs->getCRuntimePath = getCRuntimePath;
    fs->appendStrings = appendStrings;
    fs->appendExtensionToFileName = appendExtensionToFileName;
    fs->appendPathToFileName = appendPathToFileName;
    fs->removeFileFromPath = removeFileFromPath;
    fs->changeFileExtension = changeFileExtension;
    fs->getPathFromCryoPath = getPathFromCryoPath;
    fs->getPathFromEnvVar = getPathFromEnvVar;
    fs->getCompilerBinPath = getCompilerBinPath;
    fs->getCompilerRootPath = getCompilerRootPath;
    fs->createNewEmptyFile = createNewEmptyFile;
    return fs;
}

void initGlobalFS(void)
{
    fs = initFS();
}

// This is being hard coded for now, but will be replaced with a config file later
CryoSrcLocations srcLocations[] = {
    {"/workspaces/cryo/"},
    {"/home/Phock/Programming/apps/cryo/"},
};

// <readFile>
/// @brief Takes in a file path and reads the contents of the file into a buffer
char *readFile(const char *path)
{
    logMessage(LMI, "INFO", "FS", "Reading file: %s", path);
    FILE *file = fopen(path, "rb"); // Open the file in binary mode to avoid transformations
    if (file == NULL)
    {
        fprintf(stderr, "{FS} Failed to open file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    // handle fseek error
    fseek(file, 0, SEEK_SET);

    if (length == 0)
    {
        fprintf(stderr, "{FS} File is empty: %s\n", path);
        fclose(file);
        return NULL;
    }

    char *buffer = (char *)malloc(length + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "{FS} Failed to allocate memory for file: %s\n", path);
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, length, file);
    if (bytesRead < length)
    {
        fprintf(stderr, "{FS} Failed to read file: %s\n", path);
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
int dirExists(const char *path)
{
    if (path == NULL || strlen(path) >= PATH_MAX)
    {
        return -1;
    }

    struct stat st;
    if (stat(path, &st) == 0)
    {
        return S_ISDIR(st.st_mode) ? 1 : 0;
    }

    return (errno == ENOENT) ? 0 : -1;
}
// </dirExists>

// <createDir>
/// @brief Creates a directory at the given path
int createDir(const char *path)
{
    if (path == NULL)
    {
        return -1;
    }

    // Create a modifiable copy of the path
    char temp[PATH_MAX];
    strncpy(temp, path, PATH_MAX - 1);
    temp[PATH_MAX - 1] = '\0';

    // Convert forward slashes to system separator if needed
    for (char *p = temp; *p; p++)
    {
        if (*p == '/')
        {
            *p = '/'; // Or use PATH_SEPARATOR if defined
        }
    }

    // Create each directory in the path
    for (char *p = temp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0'; // Temporarily terminate the string

            // Try to create the directory
            if (mkdir(temp, 0700) != 0)
            {
                // Ignore if directory already exists
                if (errno != EEXIST)
                {
                    return -1;
                }
            }

            *p = '/'; // Restore the slash
        }
    }

    // Create the final directory
    if (mkdir(temp, 0700) != 0)
    {
        // Ignore if directory already exists
        if (errno != EEXIST)
        {
            return -1;
        }
    }

    return 0;
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
    logMessage(LMI, "INFO", "TypeDefs", "Found Cryo path: %s", cryoPath);

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

// <getFileNameFromPathNoExt>
/// @brief Extracts the filename from a file path without the extension
/// i.e. `/path/to/file.txt` -> `file`
char *getFileNameFromPathNoExt(const char *filePath)
{
    if (!filePath)
    {
        return NULL;
    }

    // Find last occurrence of forward or backward slash
    const char *lastForwardSlash = strrchr(filePath, '/');
    const char *lastBackSlash = strrchr(filePath, '\\');
    const char *fileName = lastForwardSlash > lastBackSlash ? lastForwardSlash : lastBackSlash;

    // If no slash found, use the entire path as filename
    if (!fileName)
    {
        fileName = filePath;
    }
    else
    {
        // Move past the slash
        fileName++;
    }

    // Find the last dot for extension
    const char *lastDot = strrchr(fileName, '.');

    // Calculate length (either to dot or end of string)
    size_t nameLength = lastDot ? (size_t)(lastDot - fileName) : strlen(fileName);

    // Allocate memory (+1 for null terminator)
    char *fileNameNoExt = (char *)malloc(nameLength + 1);
    if (!fileNameNoExt)
    {
        return NULL;
    }

    // Copy the filename without extension
    strncpy(fileNameNoExt, fileName, nameLength);
    fileNameNoExt[nameLength] = '\0';

    return fileNameNoExt;
}
// </getFileNameFromPathNoExt>

// <getFileNameFromPath>
/// @brief Extracts the filename from a file path
/// i.e. `/path/to/file.txt` -> `file.txt`
char *getFileNameFromPath(const char *filePath)
{
    if (!filePath)
    {
        return NULL;
    }

    // Find last occurrence of forward or backward slash
    const char *lastForwardSlash = strrchr(filePath, '/');
    const char *lastBackSlash = strrchr(filePath, '\\');
    const char *fileName = lastForwardSlash > lastBackSlash ? lastForwardSlash : lastBackSlash;

    // If no slash found, use the entire path as filename
    if (!fileName)
    {
        fileName = filePath;
    }
    else
    {
        // Move past the slash
        fileName++;
    }

    // Calculate length of filename including extension
    size_t nameLength = strlen(fileName);

    // Allocate memory (+1 for null terminator)
    char *fileNameCopy = (char *)malloc(nameLength + 1);
    if (!fileNameCopy)
    {
        return NULL;
    }

    // Copy the complete filename
    strncpy(fileNameCopy, fileName, nameLength);
    fileNameCopy[nameLength] = '\0';

    return fileNameCopy;
}
// <getFileNameFromPath>

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

const char *appendExtensionToFileName(const char *fileName, const char *extension)
{
    char *result = (char *)malloc(strlen(fileName) + strlen(extension) + 1);
    if (!result)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to allocate memory for string concatenation");
        return NULL;
    }

    strcpy(result, fileName);
    strcat(result, extension);

    return result;
}

/// @brief Appends a path to a file name.
/// @param path
/// @param fileName
/// @param endingSlash True if the path should end with a slash, false otherwise
/// @return
const char *appendPathToFileName(const char *path, const char *fileName, bool endingSlash)
{
    char *result = (char *)malloc(strlen(path) + strlen(fileName) + 2);
    if (!result)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to allocate memory for string concatenation");
        return NULL;
    }

    strcpy(result, path);
    if (endingSlash)
    {
        strcat(result, "/");
    }
    strcat(result, fileName);

    return result;
}

const char *removeFileFromPath(const char *path)
{
    const char *lastSlash = strrchr(path, '/');
    if (!lastSlash)
    {
        return path;
    }

    size_t length = (size_t)(lastSlash - path);
    char *result = (char *)malloc(length + 1);
    if (!result)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to allocate memory for string concatenation");
        return NULL;
    }

    strncpy(result, path, length);
    result[length] = '\0';

    return result;
}

const char *changeFileExtension(const char *fileName, const char *newExtension)
{
    const char *lastDot = strrchr(fileName, '.');
    if (!lastDot)
    {
        return fileName;
    }

    size_t length = (size_t)(lastDot - fileName);
    char *result = (char *)malloc(length + strlen(newExtension) + 1);
    if (!result)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to allocate memory for string concatenation");
        return NULL;
    }

    strncpy(result, fileName, length);
    result[length] = '\0';
    strcat(result, newExtension);

    return result;
}

char *getPathFromCryoPath(void)
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

// This function will see if the `CRYO_COMPILER` environment variable is set
char *getPathFromEnvVar(void)
{
    const char *env_var = getenv("CRYO_COMPILER");
    if (env_var == NULL)
    {
        fprintf(stderr, "CRYO_COMPILER environment variable not set\n");
        return NULL;
    }

    return strdup(env_var);
}

// This function should return `{LOCATION}/cryo/bin/` where `LOCATION` is the root directory of the Cryo compiler
char *getCompilerBinPath(void)
{
    // Try getting path from cryo-path binary first
    char *path = getPathFromCryoPath();
    if (path != NULL)
    {
        return path;
    }

    // Try getting path from CRYO_COMPILER environment variable
    path = getPathFromEnvVar();
    if (path != NULL)
    {
        return path;
    }

    // Fallback paths to check
    const char *fallback_paths[] = {
        "/usr/local/bin/cryo/bin/",
        "/usr/bin/cryo/bin/",
        getenv("HOME") ? strcat(getenv("HOME"), "/cryo/bin/") : NULL};

    for (size_t i = 0; i < sizeof(fallback_paths) / sizeof(fallback_paths[0]); i++)
    {
        if (fallback_paths[i] && access(fallback_paths[i], F_OK) == 0)
        {
            return strdup(fallback_paths[i]);
        }
    }

    return NULL;
}

// This function should return `{LOCATION}/cryo/` where `LOCATION` is the root directory of the Cryo compiler
char *getCompilerRootPath(void)
{
    // Try getting path from cryo-path binary first
    char *path = getPathFromCryoPath();
    if (path != NULL)
    {
        return (char *)removeFileFromPath(removeFileFromPath(path));
    }

    // Try getting path from CRYO_COMPILER environment variable
    path = getPathFromEnvVar();
    if (path != NULL)
    {
        return (char *)removeFileFromPath(removeFileFromPath(path));
    }

    // Fallback paths to check
    const char *fallback_paths[] = {
        "/usr/local/bin/cryo/",
        "/usr/bin/cryo/",
        getenv("HOME") ? strcat(getenv("HOME"), "/cryo/") : NULL};

    for (size_t i = 0; i < sizeof(fallback_paths) / sizeof(fallback_paths[0]); i++)
    {
        if (fallback_paths[i] && access(fallback_paths[i], F_OK) == 0)
        {
            return strdup(fallback_paths[i]);
        }
    }

    return NULL;
}
void createNewEmptyFile(const char *fileName, const char *ext, const char *path)
{
    // Create the file name
    char *newFileName = (char *)malloc(strlen(fileName) + strlen(ext) + 1);
    strcpy(newFileName, fileName);
    strcat(newFileName, ext);

    // Create the file path
    char *filePath = (char *)malloc(strlen(path) + strlen(newFileName) + 2);
    strcpy(filePath, path);
    strcat(filePath, "/");
    strcat(filePath, newFileName);

    // Create directories recursively
    char *tempPath = strdup(path);
    char *p = tempPath;

    // Skip leading slashes
    while (*p == '/')
        p++;

    while (*p != '\0')
    {
        if (*p == '/')
        {
            *p = '\0'; // Temporarily terminate the string
            if (mkdir(tempPath, 0755) != 0 && errno != EEXIST)
            {
                logMessage(LMI, "ERROR", "FS", "Failed to create directory: %s", tempPath);
                perror("mkdir");
                DEBUG_BREAKPOINT;
                free(tempPath);
                free(newFileName);
                free(filePath);
                return;
            }
            *p = '/'; // Restore the slash
        }
        p++;
    }

    // Create final directory
    if (mkdir(tempPath, 0755) != 0 && errno != EEXIST)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to create directory: %s", tempPath);
        perror("mkdir");
        DEBUG_BREAKPOINT;
        free(tempPath);
        free(newFileName);
        free(filePath);
        return;
    }

    free(tempPath);

    // Create the file
    FILE *file = fopen(filePath, "w");
    if (!file)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to create file: %s", filePath);
        perror("fopen");
        DEBUG_BREAKPOINT;
        free(newFileName);
        free(filePath);
        return;
    }

    fclose(file);
    free(newFileName);
    free(filePath);
}
