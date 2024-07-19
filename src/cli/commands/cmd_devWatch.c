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

const char* ignorePatterns[] = {
    ".git",
    // "/bin",
    "/tests"
};


// <getHelpArg>
DevWatchArgs getDevWatchArg(char* arg) {
    if (strcmp(arg, "-help") == 0)      return DEV_WATCH_ARG_HELP;
    if (strcmp(arg, "version") == 0)    return DEV_WATCH_ARG_START;
    
    return DEV_WATCH_ARG_UNKNOWN;
}
// </getHelpArg>


// <executeDevWatchCmd>
void executeDevWatchCmd(char* argv[]) {
    char* argument = argv[2];
    if(argument == NULL) {
        printf("Watching for changes...\n");
        char* basePath = getBasePath();
        executeDevWatch(basePath);
        free(basePath);
    }

    DevWatchArgs arg = getDevWatchArg(argument);
    switch (arg) {
        case DEV_WATCH_ARG_HELP:
            // Execute Command
            break;
            
        // case DEV_WATCH_ARG_START:
        //     // Execute Command
        //     break;

        case DEV_WATCH_ARG_UNKNOWN:
            // Handle Unknown Command
            break;

        default:
            // Do something
    }
}
// <executeDevWatchCmd>


// <shouldIgnore>
int shouldIgnore(const char* path) {
    for (int i = 0; i < sizeof(ignorePatterns) / sizeof(ignorePatterns[0]); i++) {
        if (strstr(path, ignorePatterns[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}
// </shouldIgnore>


// <getBasePath>
char* getBasePath() {
    char* cwd = malloc(MAX_PATH_LEN * sizeof(char));
    if (cwd == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    if (getcwd(cwd, MAX_PATH_LEN) != NULL) {
        printf("Current working dir: %s\n", cwd);
        return cwd;
    } else {
        perror("getcwd");
        free(cwd);
        exit(EXIT_FAILURE);
    }
}
// </getBasePath>


// Function to check directory recursively
// <checkDirectory>
void checkDirectory(const char* basePath, FileInfo** files, int* count, int* capacity) {
    DIR* dir;
    struct dirent* ent;
    struct stat st;
    char path[MAX_PATH_LEN];

    printf("Checking directory: %s\n", basePath);

    if ((dir = opendir(basePath)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            sleep(1);
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            snprintf(path, sizeof(path), "%s/%s", basePath, ent->d_name);

            if (shouldIgnore(path)) {
                printf("\nIgnoring: %s\n\n", path);
                continue;
            }

            if (stat(path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    checkDirectory(path, files, count, capacity);
                } else {
                    if (*count >= *capacity) {
                        *capacity *= 2;
                        *files = realloc(*files, *capacity * sizeof(FileInfo));
                        if (*files == NULL) {
                            perror("realloc");
                            exit(EXIT_FAILURE);
                        }
                    }
                    strncpy((*files)[*count].path, path, sizeof((*files)[*count].path));
                    (*files)[*count].mtime = st.st_mtime;
                    (*count)++;
                }
            } else {
                perror("stat");
            }
        }
        closedir(dir);
    } else {
        perror("Could not open directory");
        exit(EXIT_FAILURE);
    }
}
// </checkDirectory>


// <executeDevWatch>
void executeDevWatch(const char* basePath) {
    int capacity = 10;
    int count = 0;
    FileInfo* files = malloc(capacity * sizeof(FileInfo));
    if (files == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    checkDirectory(basePath, &files, &count, &capacity);

    while (1) {
        for (int i = 0; i < count; i++) {
            struct stat st;
            if (stat(files[i].path, &st) == 0) {
                if (files[i].mtime != st.st_mtime) {
                    files[i].mtime = st.st_mtime;
                    printf("File has changed: %s\n", files[i].path);
                    // Rebuild the project here
                    dirChangeEvent(files[i].path);
                }
            } else {
                perror("stat");
                exit(EXIT_FAILURE);
            }
        }
    }

    free(files);
}
// </executeDevWatch>


// <dirChangeEvent>
void dirChangeEvent(const char* basePath) {
    char* fileName = strrchr(basePath, '/');
    if (!fileName) {
        fileName = strrchr(basePath, '\\');
    }

    if (fileName) {
        fileName++;
    } else {
        fileName = (char*)basePath;
    }

    printf("File Changed: %s\n", fileName);

    // Check if the file changed is a .c file
    char* dot = strrchr(fileName, '.');
    if (dot && strcmp(dot, ".c") == 0) {
        // Rebuild the project
        printf("Rebuilding project...\n");
        rebuildProject();
    } else {
        printf("Ignoring non-source file change.\n");
    }
}
// </dirChangeEvent>


// <findObjectFile>
bool findObjectFile(char* fileName) {
    char* objectPath = malloc(MAX_PATH_LEN * sizeof(char));
    if (objectPath == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Find the last occurrence of '.'
    char* dot = strrchr(fileName, '.');
    if (dot && strcmp(dot, ".c") == 0) {
        *dot = '\0';  // Remove the extension
    }

    char* cryoSourceDir = getenv("CRYO_PATH");
    if (cryoSourceDir == NULL) {
        fprintf(stderr, "CRYO_PATH not set.\n");
        free(objectPath);
        return false;
    }

    printf("CRYO_PATH: %s\n", cryoSourceDir);
    printf("Filename after removing extension: %s\n", fileName);

    strcpy(objectPath, cryoSourceDir);
    strcat(objectPath, "/src/bin/.o/");
    strcat(objectPath, fileName);
    strcat(objectPath, ".o");

    printf("Object Path: %s\n", objectPath);

    if (access(objectPath, F_OK) != -1) {
        printf("<!> Object file found.\n");
        // free(objectPath);
        // removeObjectFile(objectPath);
        return true;
    } else {
        printf("<!> Object file not found.\n");
        free(objectPath);
        return false;
    }
}
// </findObjectFile>





// <rebuildProject>
void rebuildProject() {
    char originalCwd[MAX_PATH_LEN];
    char* cryoSourceDir = getenv("CRYO_PATH");
    if (cryoSourceDir == NULL) {
        fprintf(stderr, "CRYO_PATH not set.\n");
        return;
    }

    // Get current working directory
    if (getcwd(originalCwd, sizeof(originalCwd)) == NULL) {
        perror("getcwd");
        return;
    }

    // Change to the directory where the Makefile is located
    if (chdir(cryoSourceDir) != 0) {
        perror("chdir");
        return;
    }

    // Rename the current executable
    char oldExecutablePath[MAX_PATH_LEN];
    char backupExecutablePath[MAX_PATH_LEN];
    snprintf(oldExecutablePath, sizeof(oldExecutablePath), "%s/src/bin/cryo.exe", cryoSourceDir);
    snprintf(backupExecutablePath, sizeof(backupExecutablePath), "%s/src/bin/cryo_backup.exe", cryoSourceDir);
    renameExecutable(oldExecutablePath, backupExecutablePath);

    // Execute the makefile in a new process and capture output
    executeSysCommand("mingw32-make all");

    // Replace the old executable with the new one
    replaceExecutable(backupExecutablePath, oldExecutablePath);

    // Change back to the original working directory
    if (chdir(originalCwd) != 0) {
        perror("chdir");
    }
    
    printf("Project rebuilt.\n");

    // Execute the new executable
    char rebuildCommand[MAX_PATH_LEN];
    strcpy(rebuildCommand, oldExecutablePath);
    strcat(rebuildCommand, " wdev");

    executeSysCommand(rebuildCommand);
}
// </rebuildProject>

// <renameExecutable>
void renameExecutable(const char* oldPath, const char* newPath) {
    if (rename(oldPath, newPath) != 0) {
        perror("rename");
        exit(EXIT_FAILURE);
    }
    printf("Renamed %s to %s\n", oldPath, newPath);
}
// </renameExecutable>

// <replaceExecutable>
void replaceExecutable(const char* oldPath, const char* newPath) {
    if (remove(newPath) != 0) {
        perror("remove");
        exit(EXIT_FAILURE);
    }
    if (rename(oldPath, newPath) != 0) {
        perror("rename");
        exit(EXIT_FAILURE);
    }
    printf("Replaced %s with %s\n", newPath, oldPath);
}
// </replaceExecutable>


// <executeCommand>
void executeSysCommand(const char* command) {
    printf("Executing command: %s\n", command);
    char buffer[128];
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
    }
    int status = pclose(pipe);
    if (status == -1) {
        perror("pclose");
    } else {
        printf("Command exited with status: %d\n", status);
    }
}
// </executeCommand>


