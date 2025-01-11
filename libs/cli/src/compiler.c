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
#include "include/compiler.h"

int cryo_compile(char *args[], int argCount)
{
    printf("\n");
    printf("Starting Cryo Compiler...\n");
    const char *compiler_path = getCompilerExePath();
    if (!compiler_path)
    {
        printf("Error: Unable to locate the compiler binary\n");
        return 1;
    }
    printf("Compiler path: %s\n", compiler_path);
    printf("Arguments: \n");
    for (int i = 0; i < argCount; i++)
    {
        printf("%s\n", args[i]);
    }

    // Prepare the command
    char *command = concatStrings((char *)compiler_path, " ");
    for (int i = 0; i < argCount; i++)
    {
        command = concatStrings(command, args[i]);
        command = concatStrings(command, " ");
    }

    printf("\nCommand: %s\n", command);

    // Execute the command
    if (runSystemCommand(command))
    {
        printf("Compilation successful\n");
    }
    else
    {
        printf("Compilation failed\n");
    }

    return 0;
}
