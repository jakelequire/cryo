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
#include "cli/compiler.h"

//   snprintf(command, sizeof(command), "C:/Programming/apps/cryo/src/bin/main.exe %s", input_file);
// Function to get absolute path
char* get_abs_path(const char* local_path) {
    char temp_path[256];
    #ifdef _WIN32
    if (!_getcwd(temp_path, sizeof(temp_path))) {
    #else
    if (!getcwd(temp_path, sizeof(temp_path))) {
    #endif
        fprintf(stderr, "Error: Failed to get current directory.\n");
        return NULL;
    }

    char* abs_path = (char*)malloc(256);
    if (abs_path == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory.\n");
        return NULL;
    }

    snprintf(abs_path, 256, "%s/%s", temp_path, local_path);

    // Normalize path separators on Windows
    #ifdef _WIN32
    for (char *p = abs_path; *p; ++p) {
        if (*p == '/') *p = '\\';
    }
    #endif

    printf("[debug] <get_abs_path> Absolute path: %s\n", abs_path);
    return abs_path;
}

// Function to run the compiler
void run_compiler(const char *input_file) {
    char* input_path = get_abs_path(input_file);
    if (input_path == NULL) {
        fprintf(stderr, "Failed to get absolute path for input file.\n");
        exit(EXIT_FAILURE);
    }

    char command[256];
    snprintf(command, sizeof(command), "C:/Programming/apps/cryo/src/bin/main.exe %s", input_path);

    FILE *fp;
    char path[1035];

    // Open the command for reading.
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run compiler command\n");
        free(input_path);
        exit(EXIT_FAILURE);
    }

    // Read the output a line at a time - output it.
    while (fgets(path, sizeof(path), fp) != NULL) {
        printf("%s", path);
    }

    // Close the process.
    int status = pclose(fp);
    if (status == -1) {
        fprintf(stderr, "Error: Failed to close command stream.\n");
        free(input_path);
        exit(EXIT_FAILURE);
    } else if (status != 0) {
        fprintf(stderr, "Error: Compiler returned a non-zero exit code: %d\n", status);
        free(input_path);
        exit(EXIT_FAILURE);
    }

    free(input_path);
}








