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

#define MAX_CMD_LEN 1024

CleanCompilerOptions *parse_clean_compiler_options(int argc, char *argv[], int start_index)
{
    CleanCompilerOptions *options = (CleanCompilerOptions *)malloc(sizeof(CleanCompilerOptions));
    if (!options)
        return NULL;

    // Initialize defaults
    options->clean_all = false;
    options->clean_custom = false;

    // Parse clean-compiler command options
    for (int i = start_index; i < argc; i++)
    {
        // Check for clean all
        if (stringCompare(argv[i], "--all"))
        {
            options->clean_all = true;
        }
        // Check for clean custom
        else if (stringCompare(argv[i], "--custom"))
        {
            options->clean_custom = true;
            if (i + 1 < argc)
            {
                options->custom_name = argv[++i];
            }
            else
            {
                free(options);
                return NULL; // Missing custom name
            }
        }
    }

    return options;
}

void exe_CLI_clean_compiler(CleanCompilerOptions *options)
{
    // Get the compiler directory
    char *compiler_dir = getCryoRootDir();

    char *cwd = getcwd(NULL, 0);

    // Now that we have the root dir, we need to access the clean script
    // This depends on the arguments of `CleanCompilerOptions`.
    char clean_command[MAX_CMD_LEN]; // Changed from char* array to char array

    // The directory we are interested in is under {root}/scripts/
    // In this directory there is `clean.py` and `custom-clean.py`
    // which custom-clean takes in a name argument to clean a specific
    // directory.
    if (options->clean_all)
    {
        printf("Cleaning all compiler directories...\n\n");
        char *clean_cmd = (char *)malloc(sizeof(char *) * MAX_CMD_LEN);
        strcpy(clean_cmd, "python3 ");
        strcat(clean_cmd, compiler_dir);
        strcat(clean_cmd, "/scripts/clean.py");

        printf("Running command: %s\n", clean_cmd);

        // Run the clean command in a new process
        if (!runSystemCommand(clean_cmd))
        {
            printf("Error: Failed to clean compiler directories\n");
        }

        printf("Successfully cleaned compiler directories\n");
        exit(EXIT_SUCCESS);
    }
    else if (options->clean_custom)
    {
        // Check and make sure a custom name was provided
        if (options->custom_name == NULL)
        {
            printf("Error: No custom name provided for clean\n");
            return;
        }
        printf("Cleaning custom directory: %s\n\n", options->custom_name);
        snprintf(clean_command, MAX_CMD_LEN, "python3 %s/scripts/custom-clean.py %s",
                 compiler_dir, options->custom_name);

        printf("Running command: %s\n", clean_command);

        // Run the clean command in a new process
        if (!runSystemCommand(clean_command))
        {
            printf("Error: Failed to clean custom directory\n");
        }

        printf("Successfully cleaned custom directory\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Error: No clean options specified\n");
        return;
    }
}
