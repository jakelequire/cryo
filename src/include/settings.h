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
#ifndef SETTINGS_H
#define SETTINGS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

int DEBUG_LEVEL = 0;
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL
#define SET_DEBUG_LEVEL(level) DEBUG_LEVEL = level

typedef struct CompilerSettings
{
    const char *cwd;
    const char *customOutputPath;
    bool activeBuild;
    int debugLevel;
} CompilerSettings;

typedef enum DebugLevel
{
    DEBUG_NONE = 0,
    DEBUG_LOW = 1,
    DEBUG_MEDIUM = 2,
    DEBUG_HIGH = 3
} DebugLevel;

#endif // SETTINGS_H
