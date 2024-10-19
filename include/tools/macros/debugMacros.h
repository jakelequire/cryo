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
#ifndef DEBUG_MACROS_H
#define DEBUG_MACROS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

/// @brief A macro to break the program at a specific point for debugging.
#define DEBUG_BREAKPOINT                                                                        \
    printf("\n#========================================================================#\n");   \
    printf("\n<!> Debug Breakpoint! Exiting...");                                               \
    printf("\n<!> Line: %i, Function: %s", __LINE__, __FUNCTION__);                             \
    printf("\n<!> File: %s\n", __FILE__);                                                       \
    printf("\n#========================================================================#\n\n"); \
    exit(0)

#define CONDITION_FAILED                                                                      \
    printf("\n#========================================================================#\n"); \
    printf("\n<!> Condition Failed! Exiting...");                                             \
    printf("\n<!> Line: %i, Function: %s", __LINE__, __FUNCTION__);                           \
    printf("\n<!> File: %s\n", __FILE__);                                                     \
    printf("\n#========================================================================#\n"); \
    exit(1)

#endif // DEBUG_MACROS_H
