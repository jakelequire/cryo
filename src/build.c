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
#include "build.h"

void moveBuildFile(char *fileName, char *cwd)
{
    printf("\nMoving build file: %s\n", fileName);
    printf("Current working directory: %s\n", cwd);
    char *command = (char *)malloc(strlen(fileName) + 50);

    // Validate the current working directory
    if (cwd == NULL)
    {
        fprintf(stderr, "Error: Current working directory is NULL\n");
        return;
    }

    // Check if /build/out/imports exists, if not, create it
    sprintf(command, "mkdir -p %s/build/out/imports/", cwd);
    system(command);

    // Move the file to the build directory (cwd)/build/out/
    printf("%s %s/build/out/", fileName, cwd);
    sprintf(command, "mv %s %s/build/out/imports/", fileName, cwd);
    system(command);

    // Free the command string
    free(command);
}

void compileBuildFile(char *fileName)
{
    printf("\nCompiling build file: %s\n", fileName);
    char *command = (char *)malloc(strlen(fileName) + 50);

    // Compile the file
    sprintf(command, "clang -o %s %s", fileName, fileName);
    system(command);

    // Free the command string
    free(command);
}
