/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use self file except in compliance with the License.          *
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
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

#define MAX_CONFIG_SIZE 1024 * 10 // 10KB

void checkForCryoProject(CompilerSettings *settings)
{
    __STACK_FRAME__
    if (!settings->isSingleFile && settings->isProject)
    {
        logMessage(LMI, "INFO", "CryoConfig", "Initializing as project...");

        const char *projectDir = settings->projectDir;
        if (!projectDir)
        {
            fprintf(stderr, "Error: Invalid project directory\n");
            exit(EXIT_FAILURE);
        }
        logMessage(LMI, "INFO", "CryoConfig", "Project Directory: %s", projectDir);
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
        logMessage(LMI, "INFO", "CryoConfig", "CryoConfig Path: %s", cryoConfigPath);

        char *configString = getConfigString(cryoConfigPath);
        if (!configString)
        {
            fprintf(stderr, "Error: Failed to read config string\n");
            exit(EXIT_FAILURE);
        }

        DEBUG_PRINT_FILTER({
            printf("Config String: \n");
            printf("============================================\n\n");
            printf("%s\n", configString);
            printf("============================================\n");
        });

        ProjectSettings *projectSettings = parseCryoConfig(configString);
        if (!projectSettings)
        {
            fprintf(stderr, "Error: Failed to parse cryoconfig file\n");
            exit(EXIT_FAILURE);
        }

        logProjectSettings(projectSettings);

        String *projectName = _Str(projectSettings->project_name);
        if (projectName)
        {
            logMessage(LMI, "INFO", "CryoConfig", "Project Name: %s", projectName->c_str(projectName));
        }

        // Apply the active settings
        applyActiveSettings(projectSettings);

        projectSettings->logActiveSettings(projectSettings);

        return;
    }
    else
    {
        logMessage(LMI, "INFO", "CryoConfig", "Initializing as single file...");
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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

    settings->compiler_flagCount = 0;
    // Initialize compiler flags array
    for (int i = 0; i < MAX_COMPILER_FLAGS; i++)
    {
        settings->compiler_flags[i] = NULL;
    }

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

    // Initialize the internals
    settings->activeSettings = *createEmptyActiveSettings();
    // Initialize methods
    settings->doesSettingExist = doesProjectSettingExist;
    settings->logSettings = logProjectSettings;
    settings->logActiveSettings = logActiveSettings;

    return settings;
}

ActiveSettings *createEmptyActiveSettings(void)
{
    __STACK_FRAME__
    ActiveSettings *settings = (ActiveSettings *)malloc(sizeof(ActiveSettings));
    if (!settings)
    {
        fprintf(stderr, "Error: Failed to allocate memory for active settings\n");
        return NULL;
    }

    settings->project_name = false;
    settings->project_dir = false;
    settings->project_version = false;
    settings->project_author = false;
    settings->project_description = false;
    settings->project_license = false;
    settings->project_URL = false;

    settings->project_configPath = false;
    settings->project_buildPath = false;

    settings->project_buildType = false;
    settings->project_buildFlags = false;
    settings->project_buildOptions = false;
    settings->project_buildOutput = false;

    settings->project_runCommand = false;
    settings->project_runArgs = false;
    settings->project_runOptions = false;
    settings->project_runOutput = false;

    settings->project_dependencies = false;

    return settings;
}

bool doesProjectSettingExist(ProjectSettings *self, const char *setting)
{
    __STACK_FRAME__
    if (cStringCompare(setting, "project_name"))
    {
        return self->activeSettings.project_name;
    }
    else if (cStringCompare(setting, "project_dir"))
    {
        return self->activeSettings.project_dir;
    }
    else if (cStringCompare(setting, "project_version"))
    {
        return self->activeSettings.project_version;
    }
    else if (cStringCompare(setting, "project_author"))
    {
        return self->activeSettings.project_author;
    }
    else if (cStringCompare(setting, "project_description"))
    {
        return self->activeSettings.project_description;
    }
    else if (cStringCompare(setting, "project_license"))
    {
        return self->activeSettings.project_license;
    }
    else if (cStringCompare(setting, "project_URL"))
    {
        return self->activeSettings.project_URL;
    }
    else if (cStringCompare(setting, "project_configPath"))
    {
        return self->activeSettings.project_configPath;
    }
    else if (cStringCompare(setting, "project_buildPath"))
    {
        return self->activeSettings.project_buildPath;
    }
    else if (cStringCompare(setting, "project_buildType"))
    {
        return self->activeSettings.project_buildType;
    }
    else if (cStringCompare(setting, "project_buildFlags"))
    {
        return self->activeSettings.project_buildFlags;
    }
    else if (cStringCompare(setting, "project_buildOptions"))
    {
        return self->activeSettings.project_buildOptions;
    }
    else if (cStringCompare(setting, "project_buildOutput"))
    {
        return self->activeSettings.project_buildOutput;
    }
    else if (cStringCompare(setting, "project_runCommand"))
    {
        return self->activeSettings.project_runCommand;
    }
    else if (cStringCompare(setting, "project_runArgs"))
    {
        return self->activeSettings.project_runArgs;
    }
    else if (cStringCompare(setting, "project_runOptions"))
    {
        return self->activeSettings.project_runOptions;
    }
    else if (cStringCompare(setting, "project_runOutput"))
    {
        return self->activeSettings.project_runOutput;
    }
    else if (cStringCompare(setting, "project_dependencies"))
    {
        return self->activeSettings.project_dependencies;
    }
    return false;
}

void applyActiveSettings(ProjectSettings *settings)
{
    __STACK_FRAME__
    ActiveSettings *activeSettings = &settings->activeSettings;
    if (settings->project_name && settings->project_name != NULL)
    {
        activeSettings->project_name = true;
    }
    if (settings->project_dir && settings->project_dir != NULL)
    {
        activeSettings->project_dir = true;
    }
    if (settings->project_version && settings->project_version != NULL)
    {
        activeSettings->project_version = true;
    }
    if (settings->project_author && settings->project_author != NULL)
    {
        activeSettings->project_author = true;
    }
    if (settings->project_description && settings->project_description != NULL)
    {
        activeSettings->project_description = true;
    }
    if (settings->project_license && settings->project_license != NULL)
    {
        activeSettings->project_license = true;
    }
    if (settings->project_URL && settings->project_URL != NULL)
    {
        activeSettings->project_URL = true;
    }
    if (settings->project_configPath && settings->project_configPath != NULL)
    {
        activeSettings->project_configPath = true;
    }
    if (settings->project_buildPath && settings->project_buildPath != NULL)
    {
        activeSettings->project_buildPath = true;
    }
    if (settings->project_buildType && settings->project_buildType != NULL)
    {
        activeSettings->project_buildType = true;
    }
    if (settings->project_buildFlags && settings->project_buildFlags != NULL)
    {
        activeSettings->project_buildFlags = true;
    }
    if (settings->project_buildOptions && settings->project_buildOptions != NULL)
    {
        activeSettings->project_buildOptions = true;
    }
    if (settings->project_buildOutput && settings->project_buildOutput != NULL)
    {
        activeSettings->project_buildOutput = true;
    }
    if (settings->project_runCommand && settings->project_runCommand != NULL)
    {
        activeSettings->project_runCommand = true;
    }
    if (settings->project_runArgs && settings->project_runArgs != NULL)
    {
        activeSettings->project_runArgs = true;
    }
    if (settings->project_runOptions && settings->project_runOptions != NULL)
    {
        activeSettings->project_runOptions = true;
    }
    if (settings->project_runOutput && settings->project_runOutput != NULL)
    {
        activeSettings->project_runOutput = true;
    }
    if (settings->project_dependencyCount > 0)
    {
        activeSettings->project_dependencies = true;
    }
}
