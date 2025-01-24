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
    {
        fprintf(stderr, "Error: Failed to allocate memory for project settings\n");
        return NULL;
    }

    printf("Parsing cryoconfig file...\n");

    ConfigLexer *lexer = initConfigLexer(configString);
    if (!lexer)
    {
        fprintf(stderr, "Error: Failed to initialize lexer\n");
        return NULL;
    }

    printf("Config Parser initialized\n");

    ConfigTok *token = getNextConfigToken(lexer);
    while (token)
    {
        switch (token->token)
        {
        case CONFIG_TOK_PROJECT:
            printf("Found project section\n");
            break;
        case CONFIG_TOK_COMPILER:
            printf("Found compiler section\n");
            break;
        case CONFIG_TOK_DEPENDENCIES:
            printf("Found dependencies section\n");
            break;
        case CONFIG_TOK_BUILD:
            printf("Found build section\n");
            break;
        case CONFIG_TOK_RUN:
            printf("Found run section\n");
            break;
        default:
            printf("Unknown token: %s\n", token->value);
            break;
        }

        token = getNextConfigToken(lexer);
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
        printf("End of config file\n");
        return NULL;
    }

    // Skip whitespace
    while (lexer->configString[lexer->configPos] == ' ' ||
           lexer->configString[lexer->configPos] == '\n' ||
           lexer->configString[lexer->configPos] == '\t')
    {
        printf("Skipping whitespace\n");
        lexer->configPos++;
    }

    // Skip comments
    if (lexer->configString[lexer->configPos] == '#')
    {
        printf("Skipping comment\n");
        while (lexer->configString[lexer->configPos] != '\n')
        {
            lexer->configPos++;
        }
    }

    // Check for end of string
    if (lexer->configPos >= lexer->configLength)
    {
        printf("End of config file\n");
        return NULL;
    }

    // Check for section headers
    if (lexer->configString[lexer->configPos] == '[')
    {
        printf("Found section header\n");
        lexer->configPos++;
        size_t start = lexer->configPos;
        while (lexer->configString[lexer->configPos] != ']')
        {
            lexer->configPos++;
        }
        size_t end = lexer->configPos;
        lexer->configPos++;

        char *sectionName = (char *)malloc(end - start + 1);
        if (!sectionName)
        {
            fprintf(stderr, "Error: Failed to allocate memory for section name\n");
            return NULL;
        }

        strncpy(sectionName, lexer->configString + start, end - start);
        sectionName[end - start] = '\0';

        return newConfigTok(CONFIG_TOK_UNKNOWN, sectionName);
    }

    // Check for identifiers
    if (lexer->configString[lexer->configPos] == '"')
    {
        printf("Found string\n");
        lexer->configPos++;
        size_t start = lexer->configPos;
        while (lexer->configString[lexer->configPos] != '"')
        {
            lexer->configPos++;
        }
        size_t end = lexer->configPos;
        lexer->configPos++;

        char *value = (char *)malloc(end - start + 1);
        if (!value)
        {
            fprintf(stderr, "Error: Failed to allocate memory for value\n");
            return NULL;
        }

        strncpy(value, lexer->configString + start, end - start);
        value[end - start] = '\0';

        return newConfigTok(CONFIG_TOK_STRING, value);
    }

    return newConfigTok(CONFIG_TOK_UNKNOWN, NULL);
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
