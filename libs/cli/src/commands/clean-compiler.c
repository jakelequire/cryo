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

#define MAX_CMD_LEN 1024

void exe_CLI_clean_compiler(CleanCompilerOptions *options)
{
    // Get the compiler directory
    char *compiler_dir = getCryoRootDir();

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
        snprintf(clean_command, MAX_CMD_LEN, "python3 %s/scripts/clean.py", compiler_dir);
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
    }
    else
    {
        printf("Error: No clean options specified\n");
        return;
    }
}
