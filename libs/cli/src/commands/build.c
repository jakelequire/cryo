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

// The build command.
// TODO: Need a way to have sub-commands for the build command.
// - cryo build                             // Build from a project directory
// - cryo build -f <file>                   // Build a single file
// - cryo build -f <file> -o <output>       // Build a single file and output to a specific location

static void handle_project_build(void)
{
    printf("Building project from current directory...\n");
    // TODO: Implement project directory build logic
}

static void handle_single_file_build(const char *input_file, const char *output_file)
{
    printf("Building single file: %s\n", input_file);
    if (output_file)
    {
        printf("Output will be written to: %s\n", output_file);
    }
    // TODO: Implement single file build logic
}

void exe_CLI_build(BuildOptions *options)
{
    if (!options)
    {
        printf("Error: Invalid build command options\n");
        exe_CLI_help();
        return;
    }

    const char *compiler_path = getCompilerBinPath();
    if (!compiler_path)
    {
        printf("Error: Unable to locate the compiler binary\n");
        return;
    }
    printf("Compiler path: %s\n", compiler_path);

    if (options->single_file)
    {
        handle_single_file_build(options->input_file,
                                 options->has_output ? options->output_file : NULL);
    }
    else
    {
        handle_project_build();
    }

    // Cleanup
    free(options);
}
