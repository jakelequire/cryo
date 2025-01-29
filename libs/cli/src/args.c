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

enum CLI_ARGS get_CLI_arg(char *arg)
{
    // help
    if (stringCompare(arg, "help") || stringCompare(arg, "-h"))
    {
        return CLI_HELP;
    }
    // --version
    else if (stringCompare(arg, "--version") || stringCompare(arg, "-v"))
    {
        return CLI_VERSION;
    }
    // build
    else if (stringCompare(arg, "build"))
    {
        return CLI_BUILD;
    }
    // build-compiler
    else if (stringCompare(arg, "bc"))
    {
        return CLI_BUILD_COMPILER;
    }
    // clean-compiler
    else if (stringCompare(arg, "cc"))
    {
        return CLI_CLEAN_COMPILER;
    }
    // init
    else if (stringCompare(arg, "init"))
    {
        return CLI_INIT;
    }
    // devserver
    else if (stringCompare(arg, "devserver"))
    {
        return CLI_DEVSERVER;
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
        return;
    }

    const char *cwd = getcwd(NULL, 0);

    // Check first if it's a help command
    if (stringCompare(argv[1], "help") || stringCompare(argv[1], "-h"))
    {
        // If there's an additional argument, it's a command-specific help
        if (argc > 2)
        {
            HelpOptions *options = parse_help_options(argc, argv, 2);
            exe_CLI_help_options(options);
            free(options);
        }
        else
        {
            // General help if no specific command provided
            exe_CLI_help();
        }
        return; // Important: return after handling help
    }

    for (int i = 1; i < argc; i++)
    {
        enum CLI_ARGS arg = get_CLI_arg(argv[i]);
        switch (arg)
        {
        case CLI_VERSION:
            exe_CLI_version();
            break;
        case CLI_BUILD:
            exe_CLI_build(parse_build_options(argc, argv, i + 1));
            return;
        case CLI_BUILD_COMPILER:
            exe_CLI_build_compiler();
            break;
        case CLI_CLEAN_COMPILER:
            exe_CLI_clean_compiler(parse_clean_compiler_options(argc, argv, i + 1));
            break;
        case CLI_INIT:
            exe_CLI_init(parse_init_options(argc, argv, i + 1, cwd));
            break;
        case CLI_DEVSERVER:
            exe_CLI_devserver();
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
