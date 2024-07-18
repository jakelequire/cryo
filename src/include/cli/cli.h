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
#ifndef CLI_H
#define CLI_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli/help.h"
#include "cli/init.h"
#include "cli/build.h"
#include "cli/devWatch.h"
#include "cli/version.h"

extern char* cryo_logo;


/* =========================================================== */
// @Primary_Command

typedef enum {
    CMD_HELP,
    CMD_VERSION,
    CMD_BUILD,
    CMD_INIT,
    CMD_DEV_WATCH,
    CMD_UNKNOWN
} CommandType;



/* =========================================================== */
// @Prototypes

CommandType getCommandType          (const char* command);
void executeCommand                 (CommandType command, char* argv);


#endif
