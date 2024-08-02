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
#include "cli/build.h"


// <getBuildArgs>
BuildArgs getBuildArgs(char* arg) {
    if (strcmp(arg, "-s")  == 0)          return BUILD_ARG;
    if (strcmp(arg, "-dir") == 0)         return BUILD_ARG_DIR;

    return BUILD_ARG_UNKNOWN;
}
// </getBuildArgs>


// <executeBuildCmd>
void executeBuildCmd(char* argv[]) {
    printf("Building Cryo project...\n");

    // Arg 0 is the file path
    char* argument = argv[0];

    BuildArgs arg = getBuildArgs(argument);
    switch (arg) {
        case BUILD_ARG:
            // Execute Command
            break;
        case BUILD_ARG_DIR:
            // Execute Command
            break;
        
        case BUILD_ARG_UNKNOWN:
            // Execute Command
            printf("Unknown Build Args, Building Single File...\n");
            executeSingleFileBuild(argument);
            break;

        default:
            // Do something
    }
    exit(EXIT_SUCCESS);
}
// </executeBuildCmd>


/* =========================================================== */

// <executeSingleFileBuild>
void executeSingleFileBuild(char* filePath) {
    char* source = readSourceFile(filePath);
    compileSource(source);
}

// <readSourceFile>
char* readSourceFile(const char* filePath) {
    char* fullPath = getFullPath(filePath);

    char* source = readFile(fullPath);
    if (source == NULL) {
        fprintf(stderr, "Failed to read source file.\n");
        return NULL;
    }

    // printf("[DEBUG] Source: %s\n", source);
    return source;
}
// </readSourceFile>


// <getFullPath>
const char* getFullPath(const char* filePath) {
    char* fullPath = malloc(sizeof(char) * MAX_PATH_LENGTH);
    if (fullPath == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Get the current working directory
    char cwd[MAX_PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    // trim the first two characters in the `filePath` because it starts with `./`
    filePath += 2;

    // Concatenate the current working directory with the file path
    snprintf(fullPath, MAX_PATH_LENGTH, "%s/%s", cwd, filePath);
    // printf("[DEBUG] Full Path: %s\n", fullPath);
    return fullPath;
}
// </getFullPath>

// <compileSource>
void compileSource(const char* source) {
    int compile = cryoCompiler(source);
    if (compile == 1) {
        fprintf(stderr, "Error in compilation");
    }
}
// </compileSource>
