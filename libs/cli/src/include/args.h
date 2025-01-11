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

typedef struct BuildOptions
{
    bool single_file;  // -s flag
    char *input_file;  // input file path
    char *output_file; // -o flag output path
    bool has_output;   // whether -o was specified
} BuildOptions;

enum CLI_ARGS
{
    CLI_HELP,
    CLI_VERSION,
    CLI_BUILD,
    CLI_INIT,
    CLI_UNKNOWN
};

void handleArgs(int argc, char *argv[]);

enum CLI_ARGS get_CLI_arg(char *arg);

BuildOptions *parse_build_options(int argc, char *argv[], int start_index);

#endif // CLI_ARGS_H
