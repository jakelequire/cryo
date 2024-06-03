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
#include "cli/runtime_cmd.h"


// <run_runtime>
void run_runtime(void) {
    char command[256];
    snprintf(command, sizeof(command), "C:/Programming/apps/cryo/src/bin/runtime.exe %s");

    FILE *fp;
    char path[1035];

    // Open the command for reading.
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run compiler command\n");
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
        exit(EXIT_FAILURE);
    } else if (status != 0) {
        fprintf(stderr, "Error: Compiler returned a non-zero exit code: %d\n", status);
        exit(EXIT_FAILURE);
    }
}
// </run_runtime>
