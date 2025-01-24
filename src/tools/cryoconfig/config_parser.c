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
#include "tools/cryoconfig/cryoProject.h"

/*
Syntax for the cryoconfig file:

[project]                       <- CONFIG_TOK_PROJECT
project_name: "My Project"      <- CONFIG_TOK_IDENTIFIER : CONFIG_TOK_STRING
project_dir: "/path/to/project" <- CONFIG_TOK_IDENTIFIER : CONFIG_TOK_STRING
project_version: "1.0.0"        <- CONFIG_TOK_IDENTIFIER : CONFIG_TOK_STRING
*/

#define MAX_CONFIG_SIZE 1024 * 10 // 10KB

ProjectSettings *parseCryoConfig(const char *configString)
{
    ProjectSettings *settings = createEmptyProjectSettings();
    if (!settings)
        return NULL;

    ConfigLexer *lexer = initConfigLexer(configString);
    if (!lexer)
    {
        free(settings);
        return NULL;
    }

    ConfigTok *token;
    ConfigTok *value;

    while ((token = getNextConfigToken(lexer)))
    {
        switch (token->token)
        {
        case CONFIG_TOK_PROJECT:
            while ((token = getNextConfigToken(lexer)) && token->token == CONFIG_TOK_IDENTIFIER)
            {
                const char *id = token->value;
                if (!consumeConfigTok(lexer, CONFIG_TOK_STRING, &value))
                    continue;

                if (strcmp(id, "project_name") == 0)
                    settings->project_name = strdup(value->value);
                else if (strcmp(id, "project_dir") == 0)
                    settings->project_dir = strdup(value->value);
                else if (strcmp(id, "project_version") == 0)
                    settings->project_version = strdup(value->value);
                else if (strcmp(id, "project_author") == 0)
                    settings->project_author = strdup(value->value);
                else if (strcmp(id, "project_description") == 0)
                    settings->project_description = strdup(value->value);
                else if (strcmp(id, "project_license") == 0)
                    settings->project_license = strdup(value->value);
                else if (strcmp(id, "project_URL") == 0)
                    settings->project_URL = strdup(value->value);
            }
            break;

        case CONFIG_TOK_COMPILER:
            while ((token = getNextConfigToken(lexer)) && token->token == CONFIG_TOK_IDENTIFIER)
            {
                const char *id = token->value;
                if (!consumeConfigTok(lexer, CONFIG_TOK_STRING, &value))
                    continue;

                if (strcmp(id, "project_configPath") == 0)
                    settings->project_configPath = strdup(value->value);
                else if (strcmp(id, "project_buildPath") == 0)
                    settings->project_buildPath = strdup(value->value);
            }
            break;

        case CONFIG_TOK_DEPENDENCIES:
            settings->project_dependencyCount = 0;
            while ((token = getNextConfigToken(lexer)) && token->token == CONFIG_TOK_STRING)
            {
                if (settings->project_dependencyCount < MAX_PROJECT_DEPENDENCIES)
                {
                    settings->project_dependencies[settings->project_dependencyCount++] = strdup(token->value);
                }
            }
            break;

        case CONFIG_TOK_BUILD:
            while ((token = getNextConfigToken(lexer)) && token->token == CONFIG_TOK_IDENTIFIER)
            {
                const char *id = token->value;
                if (!consumeConfigTok(lexer, CONFIG_TOK_STRING, &value))
                    continue;

                if (strcmp(id, "project_buildType") == 0)
                    settings->project_buildType = strdup(value->value);
                else if (strcmp(id, "project_buildFlags") == 0)
                    settings->project_buildFlags = strdup(value->value);
                else if (strcmp(id, "project_buildOptions") == 0)
                    settings->project_buildOptions = strdup(value->value);
                else if (strcmp(id, "project_buildOutput") == 0)
                    settings->project_buildOutput = strdup(value->value);
            }
            break;

        case CONFIG_TOK_RUN:
            while ((token = getNextConfigToken(lexer)) && token->token == CONFIG_TOK_IDENTIFIER)
            {
                const char *id = token->value;
                if (!consumeConfigTok(lexer, CONFIG_TOK_STRING, &value))
                    continue;

                if (strcmp(id, "project_runCommand") == 0)
                    settings->project_runCommand = strdup(value->value);
                else if (strcmp(id, "project_runArgs") == 0)
                    settings->project_runArgs = strdup(value->value);
                else if (strcmp(id, "project_runOptions") == 0)
                    settings->project_runOptions = strdup(value->value);
                else if (strcmp(id, "project_runOutput") == 0)
                    settings->project_runOutput = strdup(value->value);
            }
            break;
        }
    }

    return settings;
}

ConfigLexer *initConfigLexer(const char *configString)
{
    ConfigLexer *lexer = (ConfigLexer *)malloc(sizeof(ConfigLexer));
    if (!lexer)
    {
        fprintf(stderr, "Error: Failed to allocate memory for lexer\n");
        return NULL;
    }

    lexer->configString = configString;
    lexer->configLength = strlen(configString);
    lexer->configPos = 0;

    return lexer;
}

bool consumeConfigTok(ConfigLexer *lexer, ConfigToken expected, ConfigTok **token)
{
    *token = getNextConfigToken(lexer);
    if (!*token || (*token)->token != expected)
    {
        return false;
    }
    return true;
}

ConfigTok *newConfigTok(ConfigToken token, const char *value)
{
    ConfigTok *tok = (ConfigTok *)malloc(sizeof(ConfigTok));
    if (!tok)
    {
        fprintf(stderr, "Error: Failed to allocate memory for token\n");
        return NULL;
    }

    tok->token = token;
    tok->value = value;

    return tok;
}

ConfigTok *getNextConfigToken(ConfigLexer *lexer)
{
    if (lexer->configPos >= lexer->configLength)
    {
        return NULL;
    }

    // Skip whitespace and newlines
    while (lexer->configPos < lexer->configLength &&
           (lexer->configString[lexer->configPos] == ' ' ||
            lexer->configString[lexer->configPos] == '\n' ||
            lexer->configString[lexer->configPos] == '\t'))
    {
        lexer->configPos++;
    }

    // Skip comments
    if (lexer->configPos < lexer->configLength &&
        lexer->configString[lexer->configPos] == '#')
    {
        while (lexer->configPos < lexer->configLength &&
               lexer->configString[lexer->configPos] != '\n')
        {
            lexer->configPos++;
        }
        return getNextConfigToken(lexer); // Recursively get next non-comment token
    }

    if (lexer->configPos >= lexer->configLength)
    {
        return NULL;
    }

    // Handle section headers
    if (lexer->configString[lexer->configPos] == '[')
    {
        lexer->configPos++;
        size_t start = lexer->configPos;

        while (lexer->configPos < lexer->configLength &&
               lexer->configString[lexer->configPos] != ']')
        {
            lexer->configPos++;
        }

        size_t end = lexer->configPos;
        lexer->configPos++;

        char *sectionName = (char *)malloc(end - start + 3); // +2 for [] and +1 for null
        if (!sectionName)
        {
            return NULL;
        }

        sectionName[0] = '[';
        strncpy(sectionName + 1, lexer->configString + start, end - start);
        sectionName[end - start + 1] = ']';
        sectionName[end - start + 2] = '\0';

        ConfigToken type = getTokenType(sectionName);
        free(sectionName);

        return newConfigTok(type, NULL);
    }

    // Handle identifiers
    if (isalpha(lexer->configString[lexer->configPos]))
    {
        size_t start = lexer->configPos;
        while (lexer->configPos < lexer->configLength &&
               (isalnum(lexer->configString[lexer->configPos]) ||
                lexer->configString[lexer->configPos] == '_'))
        {
            lexer->configPos++;
        }

        size_t end = lexer->configPos;

        // Skip colon and whitespace
        while (lexer->configPos < lexer->configLength &&
               (lexer->configString[lexer->configPos] == ' ' ||
                lexer->configString[lexer->configPos] == ':'))
        {
            lexer->configPos++;
        }

        char *identifier = (char *)malloc(end - start + 1);
        if (!identifier)
        {
            return NULL;
        }

        strncpy(identifier, lexer->configString + start, end - start);
        identifier[end - start] = '\0';

        return newConfigTok(CONFIG_TOK_IDENTIFIER, identifier);
    }

    // Handle strings
    if (lexer->configString[lexer->configPos] == '"')
    {
        lexer->configPos++;
        size_t start = lexer->configPos;

        while (lexer->configPos < lexer->configLength &&
               lexer->configString[lexer->configPos] != '"')
        {
            lexer->configPos++;
        }

        size_t end = lexer->configPos;
        lexer->configPos++;

        char *value = (char *)malloc(end - start + 1);
        if (!value)
        {
            return NULL;
        }

        strncpy(value, lexer->configString + start, end - start);
        value[end - start] = '\0';

        return newConfigTok(CONFIG_TOK_STRING, value);
    }

    // Skip unknown characters
    lexer->configPos++;
    return getNextConfigToken(lexer);
}

ConfigToken getTokenType(const char *token)
{
    if (strcmp(token, "[project]") == 0)
    {
        return CONFIG_TOK_PROJECT;
    }
    else if (strcmp(token, "[compiler]") == 0)
    {
        return CONFIG_TOK_COMPILER;
    }
    else if (strcmp(token, "[dependencies]") == 0)
    {
        return CONFIG_TOK_DEPENDENCIES;
    }
    else if (strcmp(token, "[build]") == 0)
    {
        return CONFIG_TOK_BUILD;
    }
    else if (strcmp(token, "[run]") == 0)
    {
        return CONFIG_TOK_RUN;
    }

    return CONFIG_TOK_UNKNOWN;
}

const char *getTokenValue(ConfigTok *token)
{
    return token->value;
}
