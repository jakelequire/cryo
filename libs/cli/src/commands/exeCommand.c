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
#include "commands.h"

int handleCommandArguments(int argc, char *argv[])
{
    if (argc == 1)
    {
        exe_CLI_help();
        return 0;
    }

    if (strcmp(argv[1], "help") == 0)
    {
        exe_CLI_help();
        return 0;
    }

    if (strcmp(argv[1], "env") == 0)
    {
        exe_env_cmd();
        return 0;
    }

    return 0;
}
