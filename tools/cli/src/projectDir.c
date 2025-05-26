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
#include "include/projectDir.h"

bool isProjectDir(void)
{
    const char *cwd = getcwd(NULL, 0);
    if (!cwd)
    {
        printf("Error getting current working directory\n");
        return false;
    }

    // Check for the `cryoconfig` file (no extension)
    char *configFileName = "cryoconfig";
    char *configFilePath = (char *)malloc(strlen(cwd) + strlen(configFileName) + 2);
    if (!configFilePath)
    {
        printf("Error allocating memory for config file path\n");
        return false;
    }

    strcpy(configFilePath, cwd);
    strcat(configFilePath, "/");
    strcat(configFilePath, configFileName);

    FILE *configFile = fopen(configFilePath, "r");
    if (!configFile)
    {
        printf("Error opening config file\n");
        free(configFilePath);
        return false;
    }

    printf("Found config file: %s\n", configFilePath);
    free(configFilePath);
    fclose(configFile);
    return true;
}
