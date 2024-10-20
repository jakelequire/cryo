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
#ifndef HELP_H
#define HELP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli/cli.h"

/* =========================================================== */
// Help Args

typedef enum
{
    HELP_ARG_HELP,    // help
    HELP_ARG_VERSION, // version
    HELP_ARG_BUILD,   // build
    HELP_ARG_INIT,    // init
    HELP_ARG_UNKNOWN  // unknown
} HelpArgs;

/* =========================================================== */
// @Prototypes

HelpArgs getHelpArg(char *arg);
void executeHelpCmd(char *argv[]);

void executeHelpMenuCmd(void);

#endif // HELP_H