/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "diagnostics/diagnostics.h"

#define PATH_MAX 4096

jFS *fs = NULL;

jFS *initFS(void)
{
    __STACK_FRAME__
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
    fs->createDirectory = createDirectory;
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
    fs->createNewEmptyFileWpath = createNewEmptyFileWpath;
    fs->cleanFilePath = cleanFilePath;
    fs->getFileName = getFileName;
    fs->getFileExt = getFileExt;
    fs->getRealPath = getRealPath;
    fs->trimStringQuotes = trimStringQuotes;
    fs->getDirectoryPath = getDirectoryPath;
    fs->listDir = listDir;
    return fs;
}

void initGlobalFS(void)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
    logMessage(LMI, "INFO", "FS", "Reading file: %s", path);
    FILE *file = fopen(path, "rb"); // Open the file in binary mode to avoid transformations
    if (file == NULL)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to open file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    // handle fseek error
    fseek(file, 0, SEEK_SET);

    if (length == 0)
    {
        logMessage(LMI, "ERROR", "FS", "File is empty: %s\n", path);
        fclose(file);
        return NULL;
    }

    char *buffer = (char *)malloc(length + 1);
    if (buffer == NULL)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to allocate memory for file buffer");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, length, file);
    if (bytesRead < length)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to read file: %s\n", path);
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
                    logMessage(LMI, "ERROR", "FS", "Failed to create directory: %s\n", temp);
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
            logMessage(LMI, "ERROR", "FS", "Failed to create directory: %s\n", temp);
            return -1;
        }
        logMessage(LMI, "INFO", "FS", "Directory already exists: %s\n", temp);
    }
    else
    {
        logMessage(LMI, "INFO", "FS", "Created directory: %s\n", temp);
    }

    return 0;
}
// </createDir>

int createDirectory(const char *path)
{
    return createDir(path);
}

// <removeFile>
/// @brief Removes a file at the given path
void removeFile(const char *filePath)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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

    logMessage(LMI, "INFO", "FS", "Found Cryo runtime path: %s", runtimePath);

    return runtimePath;
}
// </getCRuntimePath>

const char *appendStrings(const char *str1, const char *str2)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
    const char *command = "cryo-path";
    FILE *fp;
    char *path = NULL;
    size_t buffer_size = 256; // Start with reasonable buffer size
    size_t len = 0;

    // Allocate initial buffer
    path = (char *)malloc(buffer_size * sizeof(char));
    if (path == NULL)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to allocate memory for path");
        return NULL;
    }
    path[0] = '\0'; // Ensure null termination

    // Open the command for reading
    fp = popen(command, "r");
    if (fp == NULL)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to run command: %s\n", command);
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
        logMessage(LMI, "ERROR", "FS", "Failed to read command output");
        free(path);
        pclose(fp);
        return NULL;
    }

    // Debug the output
    logMessage(LMI, "INFO", "FS", "Command output: %s\n", path);

    // Close the pipe
    int close_result = pclose(fp);
    if (close_result == -1)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to close command pipe");
        free(path);
        return NULL;
    }

    // Check if command executed successfully
    if (WEXITSTATUS(close_result) != 0)
    {
        logMessage(LMI, "ERROR", "FS", "Command failed with exit status: %d\n", WEXITSTATUS(close_result));
        free(path);
        return NULL;
    }

    // If path is empty after all this, something went wrong
    if (strlen(path) == 0)
    {
        logMessage(LMI, "ERROR", "FS", "Command output is empty");
        free(path);
        return NULL;
    }

    return path;
}

// This function will see if the `CRYO_COMPILER` environment variable is set
char *getPathFromEnvVar(void)
{
    __STACK_FRAME__
    const char *env_var = getenv("CRYO_COMPILER");
    if (env_var == NULL)
    {
        logMessage(LMI, "ERROR", "FS", "CRYO_COMPILER environment variable not set");
        return NULL;
    }

    return strdup(env_var);
}

// This function should return `{LOCATION}/cryo/bin/` where `LOCATION` is the root directory of the Cryo compiler
char *getCompilerBinPath(void)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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

int createNewEmptyFileWpath(const char *fileWithPath)
{
    __STACK_FRAME__
    // Validate input
    if (!fileWithPath || strlen(fileWithPath) == 0)
    {
        logMessage(LMI, "ERROR", "FS", "Invalid file path");
        return -1;
    }

    // Create mutable copies of the path for manipulation
    char dir_path[PATH_MAX];
    strncpy(dir_path, fileWithPath, PATH_MAX - 1);
    dir_path[PATH_MAX - 1] = '\0';

    // Extract directory path by finding last '/'
    char *last_slash = strrchr(dir_path, '/');
    if (!last_slash)
    {
        // If no directory specified, use current directory
        logMessage(LMI, "INFO", "FS", "No directory specified, using current directory");
        strcpy(dir_path, ".");
    }
    else
    {
        *last_slash = '\0'; // Terminate string at last slash
    }

    // Create directory structure
    char *p = dir_path;
    if (*p == '/')
        p++; // Skip leading slash

    while (*p != '\0')
    {
        if (*p == '/')
        {
            *p = '\0'; // Temporarily terminate
            if (mkdir(dir_path, 0755) == -1 && errno != EEXIST)
            {
                logMessage(LMI, "ERROR", "FS", "Failed to create directory %s: %s\n",
                           dir_path, strerror(errno));
                return -1;
            }
            *p = '/'; // Restore slash
        }
        p++;
    }

    // Create final directory level
    if (strlen(dir_path) > 0 && strcmp(dir_path, ".") != 0)
    {
        if (mkdir(dir_path, 0755) == -1 && errno != EEXIST)
        {
            logMessage(LMI, "ERROR", "FS", "Failed to create directory %s: %s\n",
                       dir_path, strerror(errno));
            return -1;
        }
    }

    // Create the empty file
    FILE *file = fopen(fileWithPath, "w");
    if (!file)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to create file: %s\n", fileWithPath);
        return -1;
    }

    fclose(file);
    return 0;
}

// This function will clean up a file path that removes double slashes like: `/path/to//file`
const char *cleanFilePath(char *filePath)
{
    __STACK_FRAME__
    char *cleanedPath = (char *)malloc(strlen(filePath) + 1);
    if (!cleanedPath)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to allocate memory for cleaned path");
        return NULL;
    }

    char *p = filePath;
    char *q = cleanedPath;
    while (*p)
    {
        *q++ = *p++;
        if (*p == '/' && *(p + 1) == '/')
        {
            p++;
        }
    }
    *q = '\0';

    return cleanedPath;
}

const char *getFileName(const char *file)
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

const char *getFileExt(const char *file)
{
    const char *lastDot = strrchr(file, '.');
    return lastDot ? lastDot + 1 : NULL;
}

const char *getRealPath(const char *path)
{
    logMessage(LMI, "INFO", "FS", "Getting real path for: %s", path);
    char *realPath = realpath(path, NULL);
    if (!realPath)
    {
        switch (errno)
        {
        case EACCES:
            logMessage(LMI, "ERROR", "FS", "Permission denied while getting real path for: %s", path);
            break;
        case EINVAL:
            logMessage(LMI, "ERROR", "FS", "Invalid path: %s", path);
            break;
        case EIO:
            logMessage(LMI, "ERROR", "FS", "I/O error occurred while getting real path for: %s", path);
            break;
        case ELOOP:
            logMessage(LMI, "ERROR", "FS", "Too many symbolic links encountered while getting real path for: %s", path);
            break;
        case ENAMETOOLONG:
            logMessage(LMI, "ERROR", "FS", "Path name too long: %s", path);
            break;
        case ENOENT:
            logMessage(LMI, "ERROR", "FS", "Path does not exist: %s", path);
            break;
        case ENOMEM:
            logMessage(LMI, "ERROR", "FS", "Out of memory while getting real path for: %s", path);
            break;
        case ENOTDIR:
            logMessage(LMI, "ERROR", "FS", "A component of the path is not a directory: %s", path);
            break;
        default:
            logMessage(LMI, "ERROR", "FS", "Unknown error occurred while getting real path for: %s", path);
            break;
        }
        return NULL;
    }

    return realPath;
}

// Removes " from the beginning and end of a string
const char *trimStringQuotes(const char *str)
{
    if (!str)
    {
        return NULL;
    }

    size_t len = strlen(str);
    if (len < 2)
    {
        return str;
    }

    if (str[0] == '"' && str[len - 1] == '"')
    {
        char *trimmed = (char *)malloc(len - 1);
        if (!trimmed)
        {
            return NULL;
        }

        strncpy(trimmed, str + 1, len - 2);
        trimmed[len - 2] = '\0';
        return trimmed;
    }

    return str;
}

const char *getDirectoryPath(const char *path)
{
    const char *lastSlash = strrchr(path, '/');
    if (!lastSlash)
    {
        return NULL;
    }

    size_t length = (size_t)(lastSlash - path);
    char *dirPath = (char *)malloc(length + 1);
    if (!dirPath)
    {
        return NULL;
    }

    strncpy(dirPath, path, length);
    dirPath[length] = '\0';

    return dirPath;
}

// <listDir>
char **listDir(const char *path)
{
    __STACK_FRAME__
    DIR *dir;
    struct dirent *entry;
    char **fileList = NULL;
    size_t count = 0;

    dir = opendir(path);
    if (!dir)
    {
        logMessage(LMI, "ERROR", "FS", "Failed to open directory: %s", path);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG) // Regular file
        {
            fileList = (char **)realloc(fileList, sizeof(char *) * (count + 1));
            fileList[count] = strdup(entry->d_name);
            count++;
        }
    }

    closedir(dir);
    return fileList;
}
// </listDir>