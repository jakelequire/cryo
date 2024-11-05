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

///
/// @brief          Replaces the printf function with a custom logMessage function that logs the message to the console.
/// @param type     The type of message to log (ERROR, WARNING, INFO, DEBUG)
/// @param line     The line number where the message was logged (use `__LINE__`)
/// @param module   The module where the message was logged
/// @param message  The message to log
/// @param          ... The arguments to log
///
void logMessage(const char *type, int line, const char *module, const char *message, ...);

const char *typeBufferFormatter(const char *type);

#endif // C_LOGGER_H