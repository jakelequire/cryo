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
#include "tools/logger/logger_config.h"
#include <stdlib.h>

// Define the global variable
EnabledLogs *g_enabledLogs = NULL;

EnabledLogs *createDefaultEnabledLogs(bool setting)
{
    EnabledLogs *enabledLogs = (EnabledLogs *)malloc(sizeof(EnabledLogs));
    if (!enabledLogs)
    {
        return NULL;
    }

    enabledLogs->lexer = setting;
    enabledLogs->parser = setting;
    enabledLogs->ast = setting;
    enabledLogs->symbolTable = setting;
    enabledLogs->types = setting;
    enabledLogs->linker = setting;
    enabledLogs->codegen = setting;
    enabledLogs->settings = setting;
    enabledLogs->arena = setting;
    enabledLogs->bootstrap = setting;
    enabledLogs->state = setting;
    enabledLogs->all = setting;

    return enabledLogs;
}

void updateEnabledLogs(EnabledLogs *logs, bool setting)
{
    logs->lexer = setting;
    logs->parser = setting;
    logs->ast = setting;
    logs->symbolTable = setting;
    logs->types = setting;
    logs->linker = setting;
    logs->codegen = setting;
    logs->settings = setting;
    logs->arena = setting;
    logs->bootstrap = setting;
    logs->state = setting;
    logs->all = setting;
}
