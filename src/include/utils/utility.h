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
#ifndef UTILITY_H
#define UTILITY_H
#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#else // UNIX
#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

// >==------ Prototypes ------==< //

///
/// @brief          Replaces the printf function with a custom logMessage function that logs the message to the console.
/// @param type     The type of message to log (ERROR, WARNING, INFO, DEBUG)
/// @param line     The line number where the message was logged (use `__LINE__`)
/// @param module   The module where the message was logged
/// @param message  The message to log
/// @param          ... The arguments to log
///
void logMessage(const char *type, int line, const char *module, const char *message, ...);

///
/// @brief          Converts an integer to a constant character pointer.
/// @param num      The integer to convert
/// @return         The constant character pointer
///
const char *intToConstChar(int num);

///
/// @brief          Converts an integer to a character pointer.
/// @param num      The integer to convert
/// @return         The character pointer
///
char *intToChar(int num);

///
/// @brief          Converts a character pointer to an integer.
/// @param str      The character pointer to convert
/// @return         The integer
///
int charToInt(char *str);

///
/// @brief          Concatenates two strings together.
/// @param str1     The first string
/// @param str2     The second string
/// @return         The concatenated string
///
char *concatStrings(const char *str1, const char *str2);

#define START_STDOUT_REDIRECT redirectStdout();
#define END_STDOUT_REDIRECT restoreStdout();

void redirectStdout();
void restoreStdout();

#endif // UTILITY_H