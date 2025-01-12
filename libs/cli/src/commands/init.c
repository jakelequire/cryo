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

void exe_CLI_init(InitOptions *options)
{
    const char *cwd = options->cwd;

    printf("Initializing Cryo Project in directory: %s\n", cwd);

    // Initialize the project directory structure
    int dirResult = initDirStructure(cwd);
    if (dirResult != 0)
    {
        fprintf(stderr, "Error: Failed to initialize project directory structure\n");
        return;
    }

    // Initialize the project settings
    int settingsResult = initProjectSettings(cwd);
    if (settingsResult != 0)
    {
        fprintf(stderr, "Error: Failed to initialize project settings\n");
        return;
    }

    // Initialize the main file
    int mainResult = initMainFile(cwd);
    if (mainResult != 0)
    {
        fprintf(stderr, "Error: Failed to initialize main file\n");
        return;
    }

    printf("Project initialized successfully\n");
}

int initDirStructure(const char *cwd)
{
    // Create the project directory structure
    char *src_dir = concatStrings((char *)cwd, "/src");

    // Create the directories
    mkdir(src_dir, 0777);

    // Cleanup
    free(src_dir);

    return 0;
}

int initMainFile(const char *cwd)
{
    // Create the main file
    char *main_file = concatStrings((char *)cwd, "/src/main.cryo");

    // Open the file
    FILE *file = fopen(main_file, "w");
    if (!file)
    {
        fprintf(stderr, "Error: Failed to create main file\n");
        return 1;
    }

    // Write the main file buffer to the file
    fwrite(MAIN_FILE_BUFFER, 1, strlen(MAIN_FILE_BUFFER), file);

    // Close the file
    fclose(file);

    // Cleanup
    free(main_file);

    return 0;
}

int initProjectSettings(const char *cwd)
{
    // Create the project settings file
    char *settings_file = concatStrings((char *)cwd, "/cryoconfig");

    // Open the file
    FILE *file = fopen(settings_file, "w");
    if (!file)
    {
        fprintf(stderr, "Error: Failed to create project settings file\n");
        return 1;
    }

    // Write the project settings buffer to the file
    fwrite(PROJECT_SETTINGS_BUFFER, 1, strlen(PROJECT_SETTINGS_BUFFER), file);

    // Close the file
    fclose(file);

    // Cleanup
    free(settings_file);

    return 0;
}
