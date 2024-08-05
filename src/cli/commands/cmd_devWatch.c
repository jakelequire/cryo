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
#include "cli/devWatch.h"

#define IGNORE_PATTERNS_COUNT 3
#define MAX_CAPACITY 10
#define SLEEP_DURATION 1

const char *ignorePatterns[IGNORE_PATTERNS_COUNT] = {
    ".git",
    // "/bin",
    "/tests"};

DevWatchArgs getDevWatchArg(char *arg)
{
    if (strcmp(arg, "-help") == 0)
        return DEV_WATCH_ARG_HELP;
    if (strcmp(arg, "version") == 0)
        return DEV_WATCH_ARG_START;
    return DEV_WATCH_ARG_UNKNOWN;
}

void executeDevWatchCmd(char *argv[])
{
    char *argument = argv[2];
    if (argument == NULL)
    {
        printf("Watching for changes...\n");
        char *basePath = getBasePath();
        executeDevWatch(basePath);
        free(basePath);
    }

    DevWatchArgs arg = getDevWatchArg(argument);
    switch (arg)
    {
    case DEV_WATCH_ARG_HELP:
        // Execute Command
        break;
    case DEV_WATCH_ARG_UNKNOWN:
        // Handle Unknown Command
        break;
    default:
        // Do something
        break;
    }
}

int shouldIgnore(const char *path)
{
    for (int i = 0; i < IGNORE_PATTERNS_COUNT; i++)
    {
        if (strstr(path, ignorePatterns[i]) != NULL)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

char *getBasePath()
{
    char *cwd = malloc(MAX_PATH_LEN * sizeof(char));
    if (cwd == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    if (getcwd(cwd, MAX_PATH_LEN) != NULL)
    {
        printf("Current working dir: %s\n", cwd);
        return cwd;
    }
    else
    {
        perror("getcwd");
        free(cwd);
        exit(EXIT_FAILURE);
    }
}

void checkDirectory(const char *basePath, FileInfo **files, int *count, int *capacity)
{
    DIR *dir;
    struct dirent *ent;
    struct stat st;
    char path[MAX_PATH_LEN];

    printf("Checking directory: %s\n", basePath);

    if ((dir = opendir(basePath)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            snprintf(path, sizeof(path), "%s/%s", basePath, ent->d_name);

            if (shouldIgnore(path))
            {
                printf("\nIgnoring: %s\n\n", path);
                continue;
            }

            if (stat(path, &st) == 0)
            {
                if (S_ISDIR(st.st_mode))
                {
                    checkDirectory(path, files, count, capacity);
                }
                else
                {
                    if (*count >= *capacity)
                    {
                        *capacity *= 2;
                        *files = realloc(*files, *capacity * sizeof(FileInfo));
                        if (*files == NULL)
                        {
                            perror("realloc");
                            exit(EXIT_FAILURE);
                        }
                    }
                    strncpy((*files)[*count].path, path, sizeof((*files)[*count].path));
                    (*files)[*count].mtime = st.st_mtime;
                    (*count)++;
                }
            }
            else
            {
                perror("stat");
            }
        }
        closedir(dir);
    }
    else
    {
        perror("Could not open directory");
        exit(EXIT_FAILURE);
    }
}

void clearLine()
{
    printf("\33[2K\r"); // Clear the entire line and reset cursor
}

void *commandListener(void *arg)
{
    char command[256];
    while (1)
    {
        printf("Cryo$: ");
        fflush(stdout); // Flush immediately after printing the prompt
        if (fgets(command, sizeof(command), stdin) != NULL)
        {
            clearLine();
            command[strcspn(command, "\n")] = '\0'; // Remove newline character
            if (strcmp(command, "exit") == 0)
            {
                printf("Exiting...\n");
                exit(EXIT_SUCCESS);
            }
            else
            {
                clearLine();
                executeSysCommand(command);
                // printf("\nEnter command: ");
                fflush(stdout); // Flush immediately after printing the prompt
            }
        }
    }
    return NULL;
}

void executeDevWatch(const char *basePath)
{
    int capacity = MAX_CAPACITY;
    int count = 0;
    FileInfo *files = malloc(capacity * sizeof(FileInfo));
    if (files == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    checkDirectory(basePath, &files, &count, &capacity);

    pthread_t commandThread;
    if (pthread_create(&commandThread, NULL, commandListener, NULL) != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        for (int i = 0; i < count; i++)
        {
            struct stat st;
            if (stat(files[i].path, &st) == 0)
            {
                if (files[i].mtime != st.st_mtime)
                {
                    files[i].mtime = st.st_mtime;
                    clearLine();
                    printf("File has changed: %s\n", files[i].path);
                    fflush(stdout); // Flush immediately after printing the change notice
                    // Rebuild the project here
                    dirChangeEvent(files[i].path);
                    printf("Cryo$: ");
                    fflush(stdout); // Re-print command prompt after event
                }
            }
            else
            {
                perror("stat");
                exit(EXIT_FAILURE);
            }
        }
        sleep(SLEEP_DURATION);
    }

    free(files);
}

void dirChangeEvent(const char *basePath)
{
    char *fileName = strrchr(basePath, '/');
    if (!fileName)
    {
        fileName = strrchr(basePath, '\\');
    }

    if (fileName)
    {
        fileName++;
    }
    else
    {
        fileName = (char *)basePath;
    }

    printf("File Changed: %s\n", fileName);

    char *dot = strrchr(fileName, '.');
    if (dot && (strcmp(dot, ".c") == 0 || strcmp(dot, ".h") == 0 || strcmp(dot, ".cpp") == 0))
    {
        printf("Rebuilding project...\n");
        rebuildProject();
    }
    else
    {
        printf("Ignoring non-source file change.\n");
    }

    // Ensure prompt is reprinted after handling the file change
    // printf("Enter command: ");
    fflush(stdout);
}

bool findObjectFile(char *fileName)
{
    char *objectPath = malloc(MAX_PATH_LEN * sizeof(char));
    if (objectPath == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    char *dot = strrchr(fileName, '.');
    if (dot && strcmp(dot, ".c") == 0)
    {
        *dot = '\0';
    }

    char *cryoSourceDir = getenv("CRYO_PATH");
    if (cryoSourceDir == NULL)
    {
        fprintf(stderr, "CRYO_PATH not set.\n");
        free(objectPath);
        return false;
    }

    printf("CRYO_PATH: %s\n", cryoSourceDir);
    printf("Filename after removing extension: %s\n", fileName);

    snprintf(objectPath, MAX_PATH_LEN, "%s/src/bin/.o/%s.o", cryoSourceDir, fileName);

    printf("Object Path: %s\n", objectPath);

    if (access(objectPath, F_OK) != -1)
    {
        printf("<!> Object file found.\n");
        free(objectPath);
        return true;
    }
    else
    {
        printf("<!> Object file not found.\n");
        free(objectPath);
        return false;
    }
}

void rebuildProject()
{
    char originalCwd[MAX_PATH_LEN];
    char *cryoSourceDir = getenv("CRYO_PATH");
    if (cryoSourceDir == NULL)
    {
        fprintf(stderr, "CRYO_PATH not set.\n");
        return;
    }

    if (getcwd(originalCwd, sizeof(originalCwd)) == NULL)
    {
        perror("getcwd");
        return;
    }

    if (chdir(cryoSourceDir) != 0)
    {
        perror("chdir");
        return;
    }
    printf("<!> Rebuilding project...\n");
    // Execute the makefile in a new process and capture output
    executeSysCommand("make all");
    // Change back to the original working directory
    if (chdir(originalCwd) != 0)
    {
        perror("chdir");
    }

    printf("Project rebuilt.\n");

    // Execute the new executable
    char rebuildCommand[MAX_PATH_LEN];
#ifdef _WIN32
    snprintf(rebuildCommand, sizeof(rebuildCommand), "%s/src/bin/cryo wdev.exe", cryoSourceDir);
#else
    snprintf(rebuildCommand, sizeof(rebuildCommand), "%s/src/bin/cryo wdev", cryoSourceDir);
#endif
    executeSysCommand(rebuildCommand);

    // Ensure prompt is reprinted after rebuilding the project
    // printf("Enter command: ");
    fflush(stdout);
}

void renameExecutable(const char *oldPath, const char *newPath)
{
    if (rename(oldPath, newPath) != 0)
    {
        perror("rename");
        exit(EXIT_FAILURE);
    }
    printf("Renamed %s to %s\n", oldPath, newPath);
}

void replaceExecutable(const char *oldPath, const char *newPath)
{
    if (remove(newPath) != 0)
    {
        perror("remove");
        exit(EXIT_FAILURE);
    }
    if (rename(oldPath, newPath) != 0)
    {
        perror("rename");
        exit(EXIT_FAILURE);
    }
    printf("Replaced %s with %s\n", newPath, oldPath);
}

void executeSysCommand(const char *command)
{
    clearLine();
    printf("Executing command: %s\n", command);
    fflush(stdout); // Flush immediately after printing the command

    FILE *pipe = popen(command, "r");
    if (!pipe)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    size_t bufferSize = 4096;
    size_t bytesRead = 0;
    size_t totalBytesRead = 0;
    char *buffer = malloc(bufferSize);
    if (!buffer)
    {
        perror("malloc");
        pclose(pipe);
        exit(EXIT_FAILURE);
    }

    while ((bytesRead = fread(buffer + totalBytesRead, 1, bufferSize - totalBytesRead, pipe)) > 0)
    {
        totalBytesRead += bytesRead;
        if (totalBytesRead == bufferSize)
        {
            bufferSize *= 2;
            char *newBuffer = realloc(buffer, bufferSize);
            if (!newBuffer)
            {
                perror("realloc");
                free(buffer);
                pclose(pipe);
                exit(EXIT_FAILURE);
            }
            buffer = newBuffer;
        }
    }

    if (totalBytesRead > 0)
    {
        buffer[totalBytesRead] = '\0';
        printf("%s", buffer);
        fflush(stdout); // Flush after printing the buffer content
    }

    int status = pclose(pipe);
    if (status == -1)
    {
        perror("pclose");
    }
    else
    {
        printf("Command exited with status: %d\n", WEXITSTATUS(status));
    }

    printf("Finished executing command.\n");
    fflush(stdout); // Ensure the final print is flushed
    free(buffer);
}
