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
#ifndef LOGGER_CONFIG_H
#define LOGGER_CONFIG_H

#include <stdbool.h>
#include "settings/compilerSettings.h"

typedef struct CompilerSettings CompilerSettings;

typedef struct EnabledLogs
{
    bool lexer;
    bool parser;
    bool ast;
    bool symbolTable;
    bool types;
    bool linker;
    bool codegen;
    bool settings;
    bool arena;
    bool bootstrap;
    bool state;
    bool all;
    bool disabled;
} EnabledLogs;

// Declare the global variable as extern
extern EnabledLogs *g_enabledLogs;

// Function declarations
EnabledLogs *createDefaultEnabledLogs(bool setting);
void updateEnabledLogs(EnabledLogs *logs, bool setting);
void setLogStatus(EnabledLogs *logs, const char *logType, bool setting);
void setLogSettings(EnabledLogs *logs, CompilerSettings *settings);

// Macros
#define ARE_LOGS_ENABLED true

#define INIT_LOGS_WITH_SETTINGS(compiler_settings)        \
    do                                                    \
    {                                                     \
        setLogSettings(g_enabledLogs, compiler_settings); \
    } while (0)

#define DEBUG_PRINT_ENABLED() (g_enabledLogs && g_enabledLogs->all && !g_enabledLogs->disabled)

#define DEBUG_PRINT_FILTER(code)   \
    do                             \
    {                              \
        if (DEBUG_PRINT_ENABLED()) \
        {                          \
            code;                  \
        }                          \
    } while (0)

#define INIT_LOGS()                                                   \
    do                                                                \
    {                                                                 \
        g_enabledLogs = createDefaultEnabledLogs(ARE_LOGS_ENABLED);   \
        if (!g_enabledLogs)                                           \
        {                                                             \
            fprintf(stderr, "Failed to initialize logging system\n"); \
            exit(1);                                                  \
        }                                                             \
    } while (0)

#define IS_LOG_ENABLED(type) (                                                            \
    g_enabledLogs && ((strcmp(type, "Lexer") == 0 && g_enabledLogs->lexer) ||             \
                      (strcmp(type, "Parser") == 0 && g_enabledLogs->parser) ||           \
                      (strcmp(type, "AST") == 0 && g_enabledLogs->ast) ||                 \
                      (strcmp(type, "SymbolTable") == 0 && g_enabledLogs->symbolTable) || \
                      (strcmp(type, "Types") == 0 && g_enabledLogs->types) ||             \
                      (strcmp(type, "Linker") == 0 && g_enabledLogs->linker) ||           \
                      (strcmp(type, "Codegen") == 0 && g_enabledLogs->codegen) ||         \
                      (strcmp(type, "Settings") == 0 && g_enabledLogs->settings) ||       \
                      (strcmp(type, "Arena") == 0 && g_enabledLogs->arena) ||             \
                      (strcmp(type, "Bootstrap") == 0 && g_enabledLogs->bootstrap) ||     \
                      (strcmp(type, "CompilerState") == 0 && g_enabledLogs->state))) ||     \
                      (strcmp(type, "All") == 0 && g_enabledLogs->all) ||                 \
                        (strcmp(type, "Disabled") == 0 && g_enabledLogs->disabled))

#define CLEANUP_LOGS()        \
    do                        \
    {                         \
        free(g_enabledLogs);  \
        g_enabledLogs = NULL; \
    } while (0)

#define UPDATE_LOGS(setting)                       \
    do                                             \
    {                                              \
        updateEnabledLogs(g_enabledLogs, setting); \
    } while (0)

#define ENABLE_LOGS() UPDATE_LOGS(true)
#define DISABLE_LOGS() UPDATE_LOGS(false)

#endif // LOGGER_CONFIG_H
