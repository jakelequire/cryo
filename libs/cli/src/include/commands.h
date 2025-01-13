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
#include <signal.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <errno.h>

#include "utils.h"
#include "consoleColors.h"
#include "args.h"
#include "compiler.h"
#include "messages.h"

typedef struct BuildOptions BuildOptions;
typedef struct InitOptions InitOptions;
typedef struct HelpOptions HelpOptions;

// =============================================================================
// Help Command (./commands/help.c)

void exe_CLI_help(void);
void exe_CLI_help_options(HelpOptions *options);

void helpcmd_build(void);

// =============================================================================
// Env Command (./commands/env.c)

typedef struct CryoEnvironment
{
    char *codeBuffer;
    size_t bufferSize;
    size_t bufferPos;
    char *completedBuffer;
    int running;       // New flag to control the main loop
    int lastWasEscape; // New field to track escape sequences
} CryoEnvironment;

int exe_env_cmd(void);
static CryoEnvironment *init_environment(void);
static int handle_escape_sequence(CryoEnvironment *env);
static int ensure_buffer_capacity(CryoEnvironment *env, size_t needed_size);
static void disable_raw_mode(void);
static int enable_raw_mode(void);
static int setup_signals(void);
static int check_quit_command(const char *buffer);

// =============================================================================
// Build Command (./commands/build.c)

void exe_CLI_build(BuildOptions *options);

// =============================================================================
// Init Command (./commands/init.c)

void exe_CLI_init(InitOptions *options);
int initDirStructure(const char *cwd);
int initMainFile(const char *cwd);
int initProjectSettings(const char *cwd);

// =============================================================================
// Version Command (./commands/version.c)

void exe_CLI_version(void);

#endif // COMMANDS_H
