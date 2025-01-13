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
#include "tools/utils/cryoProject.h"

#define MAX_CONFIG_SIZE 1024 * 10 // 10KB

void checkForCryoProject(const char *path, CompilerSettings *settings)
{
    if (!doesConfigExist(path))
    {
        fprintf(stderr, "Error: No Cryo project found in directory\n");
        return;
    }

    char *config = getConfigString(path);
    if (!config)
    {
        fprintf(stderr, "Error: Failed to read cryoconfig file\n");
        return;
    }

    // Parse the config string
    printf("Config:\n%s\n", config);
    char *cwd = getcwd(NULL, 0);
    printf("CWD: %s\n", cwd);

    settings->projectDir = cwd;
    settings->isProject = true;
}

bool doesConfigExist(const char *path)
{
    // Check for the existence of the `cryoconfig` file
    char *configPath = concatStrings(path, "/cryoconfig");
    if (!configPath)
    {
        fprintf(stderr, "Error: Failed to allocate memory for config path\n");
        return false;
    }

    FILE *configFile = fopen(configPath, "r");
    if (!configFile)
    {
        fprintf(stderr, "Error: Failed to open cryoconfig file\n");
        free(configPath);
        return false;
    }

    // Cleanup
    fclose(configFile);
    free(configPath);

    return true;
}

// Read the contents of the `cryoconfig` file and save it to a string
char *getConfigString(const char *path)
{
    // Check for the existence of the `cryoconfig` file
    char *configPath = concatStrings(path, "/cryoconfig");
    if (!configPath)
    {
        fprintf(stderr, "Error: Failed to allocate memory for config path\n");
        return NULL;
    }

    FILE *configFile = fopen(configPath, "r");
    if (!configFile)
    {
        fprintf(stderr, "Error: Failed to open cryoconfig file\n");
        free(configPath);
        return NULL;
    }

    char *config = (char *)malloc(MAX_CONFIG_SIZE);
    if (!config)
    {
        fprintf(stderr, "Error: Failed to allocate memory for config string\n");
        fclose(configFile);
        free(configPath);
        return NULL;
    }

    size_t bytesRead = fread(config, 1, MAX_CONFIG_SIZE, configFile);
    if (bytesRead < MAX_CONFIG_SIZE)
    {
        config[bytesRead] = '\0'; // Null-terminate the string
    }
    else
    {
        fprintf(stderr, "Error: Config file is too large\n");
        free(config);
        config = NULL;
    }

    // Cleanup
    fclose(configFile);
    free(configPath);

    return config;
}
