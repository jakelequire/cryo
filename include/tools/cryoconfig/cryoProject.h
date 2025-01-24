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
#ifndef CRYO_PROJECT_H
#define CRYO_PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#include "settings/compilerSettings.h"
#include "tools/cryoconfig/configTokens.h"

#define MAX_PROJECT_DEPENDENCIES 256

typedef struct ProjectSettings
{
    // [project] section
    const char *project_name;
    const char *project_dir;
    const char *project_version;
    const char *project_author;
    const char *project_description;
    const char *project_license;
    const char *project_URL;

    // [compiler] section
    const char *project_configPath;
    const char *project_buildPath;

    // [dependencies] section
    const char *project_dependencies[MAX_PROJECT_DEPENDENCIES];
    int project_dependencyCount;

    // [build] section
    const char *project_buildType;
    const char *project_buildFlags;
    const char *project_buildOptions;
    const char *project_buildOutput;

    // [run] section
    const char *project_runCommand;
    const char *project_runArgs;
    const char *project_runOptions;
    const char *project_runOutput;
} ProjectSettings;

#define INIT_PROJECT_CONFIG(settings) \
    checkForCryoProject(settings);

// =============================================================================
// Cryo Project Functions (cryoProject.c)

void checkForCryoProject(CompilerSettings *settings);
bool doesConfigExist(const char *path);
char *getConfigString(const char *path);
ProjectSettings *createEmptyProjectSettings(void);
void logProjectSettings(ProjectSettings *settings);

// =============================================================================
// Cryo Config Parser Functions (config_parser.c)

typedef struct ConfigLexer
{
    const char *configString;
    size_t configLength;
    size_t configPos;
} ConfigLexer;

typedef struct ConfigTok
{
    ConfigToken token;
    const char *value;
} ConfigTok;

ProjectSettings *parseCryoConfig(const char *configString);

ConfigLexer *initConfigLexer(const char *configString);
ConfigTok *newConfigTok(ConfigToken token, const char *value);

bool consumeConfigTok(ConfigLexer *lexer, ConfigToken expected, ConfigTok **token);
ConfigTok *getNextConfigToken(ConfigLexer *lexer);
ConfigToken getTokenType(const char *token);
const char *getTokenValue(ConfigTok *token);

#endif // CRYO_PROJECT_H
