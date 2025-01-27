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

// - cryo build                             // Build from a project directory
// - cryo build -f <file>                   // Build a single file
// - cryo build -f <file> -o <output>       // Build a single file and output to a specific location
// - cryo build -f <file> --dev|-d          // Build a single file in development mode

#define MAX_COMPILER_FLAGS 32

// Modify the global variables to include the array size
int compilerFlagsCount = 0;
char *compilerFlags[MAX_COMPILER_FLAGS] = {NULL};

// Entry point to the build command
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

    // Handle flags first
    handleBuildFlags(options);
    bool useGDB = options->use_gdb;

    if (options->single_file)
    {
        handle_single_file_build(options->input_file,
                                 options->has_output ? options->output_file : NULL,
                                 useGDB);
    }
    else
    {
        handle_project_build(options);
    }

    // Cleanup
    free(options);
}

// Modify handleBuildFlags to safely add flags
static bool appendCompilerFlag(const char *flag)
{
    if (compilerFlagsCount >= MAX_COMPILER_FLAGS)
    {
        printf("Error: Maximum number of compiler flags exceeded\n");
        return false;
    }

    compilerFlags[compilerFlagsCount++] = (char *)flag;
    return true;
}

static void handleBuildFlags(BuildOptions *options)
{
    if (options->is_dev)
    {
        printf("Building in development mode...\n");
        if (!appendCompilerFlag("--enable-logs"))
        {
            // Handle error if needed
            return;
        }
    }

    // Add more flag handling as needed
    if (options->has_output)
    {
        if (!appendCompilerFlag("-o") ||
            !appendCompilerFlag(options->output_file))
        {
            return;
        }
    }
}

static void handle_project_build(BuildOptions *options)
{
    printf("Building project from current directory...\n");

    if (!options->is_project)
    {
        fprintf(stderr, "Error: Project directory not specified\n");
        return;
    }

    // For a project build, the compiler expects `-p <project_dir>` flag
    const char *project_dir = options->project_dir;
    if (!project_dir)
    {
        printf("Error: Project directory not specified\n");
        return;
    }

    bool isUsingGDB = options->use_gdb;

    // Calculate total arguments needed
    const int base_args = 2; // For "-p" and project_dir
    const int total_args = base_args + compilerFlagsCount;

    // Allocate array on stack with computed size
    char *args_with_flags[total_args];

    // Add base arguments
    args_with_flags[0] = "-p";
    args_with_flags[1] = (char *)project_dir;

    // Add compiler flags
    for (int i = 0; i < compilerFlagsCount; i++)
    {
        args_with_flags[base_args + i] = compilerFlags[i];
    }

    // Debug print the full command
    printf("Full command: ");
    for (int i = 0; i < total_args; i++)
    {
        printf("%s ", args_with_flags[i]);
    }
    printf("\n");

    // Call compiler with complete argument list
    int compilerResult = cryo_compile(args_with_flags, total_args, isUsingGDB);
    if (compilerResult != 0)
    {
        printf("Error: Compilation failed\n");
    }

    // Run the binary
    printf("Running main binary...\n");
    const char *buildDir = concatStrings((char *)project_dir, "/build/");
    printf("------------------------------------------------\n");
    printf("\n");
    runMainBinary(buildDir, "main");

    return;
}

static void handle_single_file_build(const char *input_file, const char *output_file, bool useGDB)
{
    printf("Building single file: %s\n", input_file);
    if (output_file)
    {
        printf("Output will be written to: %s\n", output_file);
    }

    // Calculate total arguments needed
    const int base_args = 2; // For "-f" and input_file
    const int total_args = base_args + compilerFlagsCount;

    // Allocate array on stack with computed size
    char *args_with_flags[total_args];

    // Add base arguments
    args_with_flags[0] = "-f";
    args_with_flags[1] = (char *)input_file;

    // Add compiler flags
    for (int i = 0; i < compilerFlagsCount; i++)
    {
        args_with_flags[base_args + i] = compilerFlags[i];
    }

    // Call compiler with complete argument list
    cryo_compile(args_with_flags, total_args, useGDB);
}
