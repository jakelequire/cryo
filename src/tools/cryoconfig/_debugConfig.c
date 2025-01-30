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

void logProjectSettings(ProjectSettings *settings)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    DEBUG_PRINT_FILTER({
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
    });
}

void logActiveSettings(ProjectSettings *settings)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    ActiveSettings *activeSettings = &settings->activeSettings;
    if (!activeSettings)
    {
        fprintf(stderr, "Error: Active settings are not initialized\n");
        return;
    }
    DEBUG_PRINT_FILTER({
        printf("\n---------------------[DEBUG ACTIVE SETTINGS]---------------------\n");
        printf("Active Settings:\n");
        printf("\n> [project]\n");
        printf("Project Name: %s\n", activeSettings->project_name ? "true" : "false");
        printf("Project Directory: %s\n", activeSettings->project_dir ? "true" : "false");
        printf("Project Version: %s\n", activeSettings->project_version ? "true" : "false");
        printf("Project Author: %s\n", activeSettings->project_author ? "true" : "false");
        printf("Project Description: %s\n", activeSettings->project_description ? "true" : "false");
        printf("Project License: %s\n", activeSettings->project_license ? "true" : "false");
        printf("Project URL: %s\n", activeSettings->project_URL ? "true" : "false");

        printf("\n> [compiler]\n");
        printf("Project Config Path: %s\n", activeSettings->project_configPath ? "true" : "false");
        printf("Project Build Path: %s\n", activeSettings->project_buildPath ? "true" : "false");

        printf("\n> [build]\n");
        printf("Project Build Type: %s\n", activeSettings->project_buildType ? "true" : "false");
        printf("Project Build Flags: %s\n", activeSettings->project_buildFlags ? "true" : "false");
        printf("Project Build Options: %s\n", activeSettings->project_buildOptions ? "true" : "false");
        printf("Project Build Output: %s\n", activeSettings->project_buildOutput ? "true" : "false");

        printf("\n> [run]\n");
        printf("Project Run Command: %s\n", activeSettings->project_runCommand ? "true" : "false");
        printf("Project Run Args: %s\n", activeSettings->project_runArgs ? "true" : "false");
        printf("Project Run Options: %s\n", activeSettings->project_runOptions ? "true" : "false");
        printf("Project Run Output: %s\n", activeSettings->project_runOutput ? "true" : "false");

        printf("\n> [dependencies]\n");
        printf("Project Dependencies: %s\n", activeSettings->project_dependencies ? "true" : "false");
        printf("------------------------------------------------------------------\n\n");
    });
}
