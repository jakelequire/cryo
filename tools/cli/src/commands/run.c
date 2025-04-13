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

RunOptions *parse_run_options(int argc, char *argv[], int start_index)
{
    RunOptions *options = (RunOptions *)malloc(sizeof(RunOptions));
    if (!options)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Initialize defaults
    options->cwd = NULL;
    options->use_gdb = false;
    options->auto_run = false;

    for (int i = start_index; i < argc; i++)
    {
        if (strcmp(argv[i], "--gdb") == 0 || strcmp(argv[i], "-g") == 0)
        {
            options->use_gdb = true;
        }
        else if (strcmp(argv[i], "--auto-run") == 0 || strcmp(argv[i], "-r") == 0)
        {
            options->auto_run = true;
        }
        else if (strcmp(argv[i], "--cwd") == 0 || strcmp(argv[i], "-c") == 0)
        {
            if (i + 1 < argc)
            {
                options->cwd = argv[++i];
            }
            else
            {
                fprintf(stderr, "Error: Missing directory path for --cwd option\n");
                free(options);
                return NULL;
            }
        }
        // If it's not a recognized option, assume it's a directory path
        else if (i == start_index)
        {
            options->cwd = argv[i];
        }
    }

    return options;
}

void exe_CLI_run(RunOptions *options)
{
    if (!options)
    {
        fprintf(stderr, "Error: Invalid run command options\n");
        exe_CLI_help();
        return;
    }

    // Determine the project directory
    char *project_dir = NULL;
    if (options->cwd)
    {
        project_dir = (char *)options->cwd;
    }
    else
    {
        // Use current directory if none specified
        project_dir = getcwd(NULL, 0);
        if (!project_dir)
        {
            fprintf(stderr, "Error: Unable to get current working directory\n");
            return;
        }
    }

    printf("Project directory: %s\n", project_dir);

    // Check if cryoconfig exists to verify it's a valid project
    char *config_path = concatStrings(project_dir, "/cryoconfig");
    FILE *config_file = fopen(config_path, "r");
    if (!config_file)
    {
        fprintf(stderr, "Error: Not a valid cryo project (missing cryoconfig file)\n");
        if (!options->cwd)
            free(project_dir); // Only free if we allocated it
        free(config_path);
        return;
    }
    fclose(config_file);
    free(config_path);

    // Construct path to the main binary
    char *build_dir = concatStrings(project_dir, "/build");
    char *binary_path = concatStrings(build_dir, "/main");

    // Check if binary exists
    if (access(binary_path, X_OK) != 0)
    {
        fprintf(stderr, "Error: Binary not found or not executable at %s\n", binary_path);
        fprintf(stderr, "Did you run 'cryo build' first?\n");
        if (!options->cwd)
            free(project_dir);
        free(build_dir);
        free(binary_path);
        return;
    }

    printf("------------------------------------------------\n");
    if (options->use_gdb)
    {
        printf("Running binary with GDB: %s\n", binary_path);

        // Construct the GDB command
        char *gdb_command = (char *)malloc(strlen("gdb --args ") + strlen(binary_path) + 1);
        if (!gdb_command)
        {
            fprintf(stderr, "Error: Memory allocation failed\n");
            if (!options->cwd)
                free(project_dir);
            free(build_dir);
            free(binary_path);
            return;
        }

        sprintf(gdb_command, "gdb --args %s", binary_path);

        // Clear the screen for better visibility
        system("clear");

        // Execute the GDB command
        system(gdb_command);

        free(gdb_command);
    }
    else
    {
        printf("Running binary: %s\n", binary_path);

        // Clear the screen for better visibility
        system("clear");

        // Execute the binary directly
        runMainBinary(build_dir, "main");
    }

    printf("------------------------------------------------\n");

    // Cleanup
    if (!options->cwd)
        free(project_dir);
    free(build_dir);
    free(binary_path);
}
