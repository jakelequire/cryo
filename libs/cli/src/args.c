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
        return;
    }

    const char *cwd = getcwd(NULL, 0);

    // Check first if it's a help command
    if (stringCompare(argv[1], "--help") || stringCompare(argv[1], "-h"))
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
        case CLI_INIT:
            exe_CLI_init(parse_init_options(argc, argv, i + 1, cwd));
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
    options->is_dev = false;

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
        // Check for dev mode
        else if (stringCompare(argv[i], "-d") || stringCompare(argv[i], "--dev"))
        {
            options->is_dev = true;
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

// Parse init command options.
// The `cwd` option is where the command was executed from.
// For now, there are no additional options for the init command.
// e.g `cryo init` will initialize a project in the current directory.
InitOptions *parse_init_options(int argc, char *argv[], int start_index, const char *argv0)
{
    InitOptions *options = (InitOptions *)malloc(sizeof(InitOptions));
    if (!options)
        return NULL;

    // Initialize defaults
    options->cwd = argv0;

    // Parse init command options
    for (int i = start_index; i < argc; i++)
    {
        // No additional options for the init command at this time
    }

    return options;
}

HelpOptions *parse_help_options(int argc, char *argv[], int start_index)
{
    HelpOptions *options = (HelpOptions *)malloc(sizeof(HelpOptions));
    if (!options)
        return NULL;

    // Default to general help
    options->command = CLI_HELP;

    // If we have a command argument
    if (start_index < argc)
    {
        // Convert the command argument to the corresponding CLI_ARGS
        if (stringCompare(argv[start_index], "build"))
        {
            options->command = CLI_BUILD;
        }
        else if (stringCompare(argv[start_index], "init"))
        {
            options->command = CLI_INIT;
        }
        else if (stringCompare(argv[start_index], "version"))
        {
            options->command = CLI_VERSION;
        }
        // Add other commands as needed
    }

    return options;
}
