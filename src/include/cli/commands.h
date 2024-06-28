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
#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

// Command Types
typedef enum {
    CMD_HELP,       // help     (subcommands)
    CMD_BUILD,      // build    (subcommands)
    CMD_INIT,       // init     (subcommands)
    CMD_VERSION,    // version  (no subcommands)
    CMD_RUN,        // run      (subcommands)
    CMD_ENV,        // env      (no subcommands)
    CMD_UNKNOWN     // unknown  (no subcommands)
} CommandType;

// Help Args
typedef enum {
    HELP_HELP,      // help
    HELP_VERSION,   // version
    HELP_BUILD,     // build
    HELP_INIT,      // init
    HELP_RUN,       // run
    HELP_UNKNOWN    // unknown
} HelpArgs;

// Build Args
typedef enum BuildArgs {
    BUILD_SINGLE,   // -s
    BUILD_DIR       // -d
} BuildArgs;

// Env Args
typedef enum EnvArgs {
    VERBOSE_DEBUGGING,   // -V
} EnvArgs;



// Function prototypes
CommandType get_command_type(const char* command);

void execute_command(int argc, char* argv[]);
void build_program_(int argc, char* argv[]);
void help_command(void);
void version_command(void);
void help_with_command(char* help_args);


#endif // COMMANDS_H