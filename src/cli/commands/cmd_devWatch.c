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
    "./src/bin"
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
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            snprintf(path, sizeof(path), "%s/%s", basePath, ent->d_name);

            if (shouldIgnore(path)) {
                printf("Ignoring: %s\n", path);
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
        sleep(1);
        for (int i = 0; i < count; i++) {
            struct stat st;
            if (stat(files[i].path, &st) == 0) {
                if (files[i].mtime != st.st_mtime) {
                    files[i].mtime = st.st_mtime;
                    printf("File has changed: %s\n", files[i].path);
                    // Rebuild the project here
                }
            } else {
                perror("stat");
            }
        }
    }

    free(files);
}
// </executeDevWatch>