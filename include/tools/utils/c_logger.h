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
#ifndef C_LOGGER_H
#define C_LOGGER_H
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "tools/macros/consoleColors.h"

/// @brief LMI - Logger Meta Info
/// @details This macro is used to get the line, file, and function name of the
///          caller of the logMessage function.
#define LMI \
    __LINE__, __FILE__, __func__

//
void logMessage(
    int line,
    const char *file,
    const char *func,
    const char *type,
    const char *module,
    const char *message, ...);

const char *getParentDirOfFile(const char *file);
char *stringShortener(const char *string, int length, int addDots);
char *getFileName(const char *file);
const char *typeBufferFormatter(const char *type);

#endif // C_LOGGER_H
