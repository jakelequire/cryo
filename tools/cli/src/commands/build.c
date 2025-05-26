/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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

BuildOptions *parse_build_options(int argc, char *argv[], int start_index)
{
    BuildOptions *options = (BuildOptions *)malloc(sizeof(BuildOptions));
    if (!options)
        return NULL;

    // Initialize defaults
    options->single_file = false;
    options->is_project = false;
    options->input_file = NULL;
    options->output_file = NULL;
    options->has_output = false;
    options->is_dev = false;
    options->use_gdb = false;
    options->project_dir = NULL;
    options->auto_run = false;

    // Parse build command options
    for (int i = start_index; i < argc; i++)
    {
        // Check for single file build
        if (stringCompare(argv[i], "-f"))
        {
            options->single_file = true;
            if (i + 1 < argc)
            {
                options->input_file = argv[++i];
            }
            else
            {
                free(options);
                return NULL; // Missing input file
            }
        }
        // Check for output file
        else if (stringCompare(argv[i], "-o"))
        {
            if (i + 1 < argc)
            {
                options->output_file = argv[++i];
                options->has_output = true;
            }
            else
            {
                free(options);
                return NULL; // Missing output file
            }
        }
        // Check for gdb mode
        else if (stringCompare(argv[i], "-g") || stringCompare(argv[i], "--gdb"))
        {
            options->use_gdb = true;
        }
        // Check for dev mode
        else if (stringCompare(argv[i], "-d") || stringCompare(argv[i], "--dev"))
        {
            options->is_dev = true;
        }
        // Check for auto run
        else if (stringCompare(argv[i], "-r") || stringCompare(argv[i], "--run"))
        {
            options->auto_run = true;
        }
    }

    // If `-f` was not specified, we will treat this as a project build.
    // Other arguments such as `--dev` and `-g` will still be respected.
    if (!options->single_file)
    {
        options->single_file = false;
        options->is_project = true;
        options->project_dir = getcwd(NULL, 0);
        printf("Project directory: %s\n", strdup(options->project_dir));
    }
    // Validate options for single file build
    else if (options->single_file && !options->input_file)
    {
        free(options);
        return NULL;
    }

    return options;
}

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
    bool logs_enabled = options->is_dev;
    if (!logs_enabled)
    {
        appendCompilerFlag("--disable-logs");
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
        return;
    }

    bool auto_run_enabled = options->auto_run;
    if (!auto_run_enabled)
    {
        return;
    }

    // Run the binary
    printf("Running main binary...\n");
    const char *buildDir = concatStrings((char *)project_dir, "/build/");
    printf("------------------------------------------------\n");
    system("clear");
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
