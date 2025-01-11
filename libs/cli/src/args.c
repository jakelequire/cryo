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
#include "include/args.h"

bool stringCompare(char *str1, char *str2)
{
    return strcmp(str1, str2) == 0;
}

enum CLI_ARGS get_CLI_arg(char *arg)
{
    if (stringCompare(arg, "--help") || stringCompare(arg, "-h"))
    {
        return CLI_HELP;
    }
    else if (stringCompare(arg, "--version") || stringCompare(arg, "-v"))
    {
        return CLI_VERSION;
    }
    else if (stringCompare(arg, "build"))
    {
        return CLI_BUILD;
    }
    else if (stringCompare(arg, "init"))
    {
        return CLI_INIT;
    }
    else
    {
        return CLI_HELP;
    }

    return CLI_UNKNOWN;
}

void handleArgs(int argc, char *argv[])
{
    if (argc == 1)
    {
        exe_CLI_help();
    }

    for (int i = 1; i < argc; i++)
    {
        enum CLI_ARGS arg = get_CLI_arg(argv[i]);
        switch (arg)
        {
        case CLI_HELP:
            exe_CLI_help();
            break;
        case CLI_VERSION:
            exe_CLI_version();
            break;
        // The build command.
        // TODO: Need a way to have sub-commands for the build command.
        // - cryo build                             // Build from a project directory
        // - cryo build -s <file>                   // Build a single file
        // - cryo build -s <file> -o <output>       // Build a single file and output to a specific location
        case CLI_BUILD:
            exe_CLI_build();
            break;
        case CLI_INIT:
            exe_CLI_init();
            break;
        case CLI_UNKNOWN:
            exe_CLI_help();
            break;
        default:
            exe_CLI_help();
            break;
        }
    }
}
