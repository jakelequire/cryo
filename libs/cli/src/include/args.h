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
#ifndef CLI_ARGS_H
#define CLI_ARGS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "commands.h"
#include "utils.h"
#include "projectDir.h"

#define __CLI_VERSION__ "0.0.1"

enum CLI_ARGS
{
    CLI_HELP,           // `--help | -h`
    CLI_VERSION,        // `--version | -v`
    CLI_BUILD,          // `build`
    CLI_BUILD_COMPILER, // `build-compiler`
    CLI_CLEAN_COMPILER, // `clean-compiler`
    CLI_INIT,           // `init`
    CLI_UNKNOWN
};

typedef struct HelpOptions
{
    enum CLI_ARGS command;
} HelpOptions;

typedef struct BuildOptions
{
    bool single_file;  // -s flag
    bool is_project;   // -p flag
    char *input_file;  // input file path
    char *output_file; // -o flag output path
    bool has_output;   // whether -o was specified
    bool is_dev;       // -d flag or --dev
    bool use_gdb;      // -g flag
    char *project_dir; // -p flag
    bool auto_run;     // -r flag
} BuildOptions;

typedef struct InitOptions
{
    const char *cwd;
} InitOptions;

typedef struct CleanCompilerOptions
{
    bool clean_all;
    bool clean_custom;
    const char *custom_name;
} CleanCompilerOptions;

void handleArgs(int argc, char *argv[]);

enum CLI_ARGS get_CLI_arg(char *arg);

BuildOptions *parse_build_options(int argc, char *argv[], int start_index);
InitOptions *parse_init_options(int argc, char *argv[], int start_index, const char *argv0);
HelpOptions *parse_help_options(int argc, char *argv[], int start_index);
CleanCompilerOptions *parse_clean_compiler_options(int argc, char *argv[], int start_index);

#endif // CLI_ARGS_H
