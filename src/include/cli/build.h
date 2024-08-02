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
#ifndef BUILD_H
#define BUILD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli/cli.h"
#include "cli/compiler.h"

#define MAX_PATH_LENGTH 100

// extern int cryoCompiler(const char* source);
int cryoCompiler(const char* source);

/* =========================================================== */
// Build Args

typedef enum {
    BUILD_ARG,          // Single File Build
    BUILD_ARG_DIR,      // Builds full Cryo Directory
    BUILD_ARG_UNKNOWN,
} BuildArgs;


/* =========================================================== */
// @Prototypes

BuildArgs getBuildArgs          (char* arg);
void executeBuildCmd            (char* argv[]);

void executeSingleFileBuild     (char* filePath);
char* readSourceFile            (const char* filepath);
void compileSource              (const char* source);
const char* getFullPath         (const char* filePath);

#endif // BUILD_H
