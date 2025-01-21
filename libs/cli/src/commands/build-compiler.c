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
#include "../include/commands.h"

void exe_CLI_build_compiler(void)
{
    char *compiler_path = getCompilerExePath();
    if (!compiler_path)
    {
        printf("Error: Unable to locate the compiler binary\n");
        exit(0);
    }

    printf("Compiler path: %s\n", compiler_path);

    // Trim the last directory from the path
    char *compiler_dir = trimLastDir(trimLastDir(compiler_path));

    printf("Compiler directory: %s\n", compiler_dir);

    // Now that we have the compiler dir, we need to run the make file
    char *_make_command = concatStrings("make -C ", compiler_dir);
    char *make_command = concatStrings(_make_command, " all");

    printf("Make command: %s\n", make_command);

    // Execute the make command
    if (runSystemCommand(make_command))
    {
        printf("Compiler built successfully\n");
    }
    else
    {
        printf("Error: Failed to build the compiler\n");
    }

    free(compiler_path);
    exit(0);
}
