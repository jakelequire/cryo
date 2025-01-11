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
        case CLI_BUILD:
            exe_CLI_build(parse_build_options(argc, argv, i + 1));
            return;
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

BuildOptions *parse_build_options(int argc, char *argv[], int start_index)
{
    BuildOptions *options = (BuildOptions *)malloc(sizeof(BuildOptions));
    if (!options)
        return NULL;

    // Initialize defaults
    options->single_file = false;
    options->input_file = NULL;
    options->output_file = NULL;
    options->has_output = false;

    // Parse build command options
    for (int i = start_index; i < argc; i++)
    {
        // Check for single file build
        if (stringCompare(argv[i], "-f"))
        {
            options->single_file = true;
            if (i + 1 < argc)
            {
                options->input_file = argv[++i];
            }
            else
            {
                free(options);
                return NULL; // Missing input file
            }
        }
        // Check for output file
        else if (stringCompare(argv[i], "-o"))
        {
            if (i + 1 < argc)
            {
                options->output_file = argv[++i];
                options->has_output = true;
            }
            else
            {
                free(options);
                return NULL; // Missing output file
            }
        }
    }

    // Validate options
    if (options->single_file && !options->input_file)
    {
        free(options);
        return NULL;
    }

    return options;
}
