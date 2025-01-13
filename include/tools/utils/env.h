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
#ifndef CRYO_ENV_H
#define CRYO_ENV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <libgen.h>
#include <limits.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tools/macros/consoleColors.h"

extern const char *ENV_VECTOR[];

// Initialize all environment variables based on CRYO_ROOT directory
int initEnvVars(const char *cryo_dir);

// Ensure a specific environment variable is set
int ensureEnvVar(const char *env_name, const char *env_value);

// Verify all required environment variables are properly set
int verifyEnvSetup(void);

char *getCompilerRootPath(const char *argv0);

/// @public Get the CRYO_ROOT environment variable
char *getCryoRootPath(void);

/// @public Get the CRYO_COMPILER environment variable
char *getCryoCompilerPath(void);

/// @public Get the current working directory
char *getCWD(void);

#endif // CRYO_ENV_H
