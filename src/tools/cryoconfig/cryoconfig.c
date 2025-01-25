/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use _this file except in compliance with the License.          *
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
#include "tools/cryoconfig/cryoconfig.h"

#define MAX_CONFIG_SIZE 1024 * 10 // 10KB

void checkForCryoProject(CompilerSettings *settings)
{
    if (!settings->isSingleFile && settings->isProject)
    {
        printf("Initializing as project...\n");

        const char *projectDir = settings->projectDir;
        if (!projectDir)
        {
            fprintf(stderr, "Error: Invalid project directory\n");
            exit(EXIT_FAILURE);
        }
        printf("Project Directory: %s\n", projectDir);
        const char *cryoConfigPath = concatStrings(projectDir, "/cryoconfig");
        if (!cryoConfigPath)
        {
            fprintf(stderr, "Error: Failed to allocate memory for cryoconfig path\n");
            exit(EXIT_FAILURE);
        }

        if (!doesConfigExist(cryoConfigPath))
        {
            fprintf(stderr, "Error: Project directory is not a valid Cryo project\n");
            exit(EXIT_FAILURE);
        }
        printf("Found cryoconfig file\n");

        char *configString = getConfigString(cryoConfigPath);
        if (!configString)
        {
            fprintf(stderr, "Error: Failed to read config string\n");
            exit(EXIT_FAILURE);
        }

        printf("Config String: \n");
        printf("============================================\n\n");
        printf("%s\n", configString);
        printf("============================================\n");

        ProjectSettings *projectSettings = parseCryoConfig(configString);
        if (!projectSettings)
        {
            fprintf(stderr, "Error: Failed to parse cryoconfig file\n");
            exit(EXIT_FAILURE);
        }

        logProjectSettings(projectSettings);

        String *projectName = Str(projectSettings->project_name);
        if (projectName)
        {
            printf("Project Name: %s\n", projectSettings->project_name);
        }

        DEBUG_BREAKPOINT;

        return;
    }
    else
    {
        printf("Single file mode\n");
        settings->isProject = false;
        settings->projectDir = NULL;

        return;
    }
}

// Checks for the existence of the `cryoconfig` file in the project directory
// The `path` passed in is the full path + the file name.
// `path/to/project/cryoconfig`
bool doesConfigExist(const char *path)
{
    FILE *configFile = fopen(path, "r");
    if (!configFile)
    {
        fprintf(stderr, "Error: Failed to open cryoconfig file\n");
        fprintf(stderr, "Path: %s\n", path);
        return false;
    }

    // Cleanup
    fclose(configFile);

    return true;
}

// Read the contents of the `cryoconfig` file and save it to a string
char *getConfigString(const char *path)
{
    FILE *configFile = fopen(path, "r");
    if (!configFile)
    {
        fprintf(stderr, "Error: Failed to open cryoconfig file\n");
        return NULL;
    }

    char *config = (char *)malloc(MAX_CONFIG_SIZE);
    if (!config)
    {
        fprintf(stderr, "Error: Failed to allocate memory for config string\n");
        fclose(configFile);
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

    return config;
}

ProjectSettings *createEmptyProjectSettings(void)
{
    ProjectSettings *settings = (ProjectSettings *)malloc(sizeof(ProjectSettings));
    if (!settings)
    {
        fprintf(stderr, "Error: Failed to allocate memory for project settings\n");
        return NULL;
    }

    settings->project_name = NULL;
    settings->project_dir = NULL;
    settings->project_version = NULL;
    settings->project_author = NULL;
    settings->project_description = NULL;
    settings->project_license = NULL;
    settings->project_URL = NULL;

    settings->project_configPath = NULL;
    settings->project_buildPath = NULL;

    settings->project_dependencyCount = 0;
    for (int i = 0; i < MAX_PROJECT_DEPENDENCIES; i++)
    {
        settings->project_dependencies[i] = NULL;
    }

    settings->project_buildType = NULL;
    settings->project_buildFlags = NULL;
    settings->project_buildOptions = NULL;
    settings->project_buildOutput = NULL;

    settings->project_runCommand = NULL;
    settings->project_runArgs = NULL;
    settings->project_runOptions = NULL;
    settings->project_runOutput = NULL;

    return settings;
}

bool doesProjectSettingExist(ProjectSettings *_this, const char *setting)
{
    if (cStringCompare(setting, "project_name"))
    {
        return _this->activeSettings.project_name;
    }
    else if (cStringCompare(setting, "project_dir"))
    {
        return _this->activeSettings.project_dir;
    }
    else if (cStringCompare(setting, "project_version"))
    {
        return _this->activeSettings.project_version;
    }
    else if (cStringCompare(setting, "project_author"))
    {
        return _this->activeSettings.project_author;
    }
    else if (cStringCompare(setting, "project_description"))
    {
        return _this->activeSettings.project_description;
    }
    else if (cStringCompare(setting, "project_license"))
    {
        return _this->activeSettings.project_license;
    }
    else if (cStringCompare(setting, "project_URL"))
    {
        return _this->activeSettings.project_URL;
    }
    else if (cStringCompare(setting, "project_configPath"))
    {
        return _this->activeSettings.project_configPath;
    }
    else if (cStringCompare(setting, "project_buildPath"))
    {
        return _this->activeSettings.project_buildPath;
    }
    else if (cStringCompare(setting, "project_buildType"))
    {
        return _this->activeSettings.project_buildType;
    }
    else if (cStringCompare(setting, "project_buildFlags"))
    {
        return _this->activeSettings.project_buildFlags;
    }
    else if (cStringCompare(setting, "project_buildOptions"))
    {
        return _this->activeSettings.project_buildOptions;
    }
    else if (cStringCompare(setting, "project_buildOutput"))
    {
        return _this->activeSettings.project_buildOutput;
    }
    else if (cStringCompare(setting, "project_runCommand"))
    {
        return _this->activeSettings.project_runCommand;
    }
    else if (cStringCompare(setting, "project_runArgs"))
    {
        return _this->activeSettings.project_runArgs;
    }
    else if (cStringCompare(setting, "project_runOptions"))
    {
        return _this->activeSettings.project_runOptions;
    }
    else if (cStringCompare(setting, "project_runOutput"))
    {
        return _this->activeSettings.project_runOutput;
    }
    else if (cStringCompare(setting, "project_dependencies"))
    {
        return _this->activeSettings.project_dependencies;
    }
    return false;
}

void logProjectSettings(ProjectSettings *settings)
{
    printf("\n---------------------[DEBUG PROJECT SETTINGS]---------------------\n");
    printf("Project Settings:\n");
    printf("\n> [project]\n");
    printf("Project Name: %s\n", settings->project_name);
    printf("Project Directory: %s\n", settings->project_dir);
    printf("Project Version: %s\n", settings->project_version);
    printf("Project Author: %s\n", settings->project_author);
    printf("Project Description: %s\n", settings->project_description);
    printf("Project License: %s\n", settings->project_license);
    printf("Project URL: %s\n", settings->project_URL);

    printf("\n> [compiler]\n");
    printf("Project Config Path: %s\n", settings->project_configPath);
    printf("Project Build Path: %s\n", settings->project_buildPath);

    printf("\n> [build]\n");
    printf("Project Build Type: %s\n", settings->project_buildType);
    printf("Project Build Flags: %s\n", settings->project_buildFlags);
    printf("Project Build Options: %s\n", settings->project_buildOptions);
    printf("Project Build Output: %s\n", settings->project_buildOutput);

    printf("\n> [run]\n");
    printf("Project Run Command: %s\n", settings->project_runCommand);
    printf("Project Run Args: %s\n", settings->project_runArgs);
    printf("Project Run Options: %s\n", settings->project_runOptions);
    printf("Project Run Output: %s\n", settings->project_runOutput);

    printf("\n> [dependencies]\n");
    printf("Project Dependencies: ");
    for (int i = 0; i < settings->project_dependencyCount; i++)
    {
        printf("%s ", settings->project_dependencies[i]);
    }
    printf("\n");
    printf("------------------------------------------------------------------\n\n");
}
