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
#ifndef PRINT_MACROS_H
#define PRINT_MACROS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include "tools/utils/fs.h"

#define START_COMPILATION_MESSAGE                                                              \
    printf("\n\n\n");                                                                          \
    printf("<!> ### ============================================================= ### <!>\n"); \
    printf("<!> ### - - - - - - - - - - - Start of Compilation - - - - - - - - -  ### <!>\n"); \
    printf("<!> ### ============================================================= ### <!>\n"); \
    printf("\n\n\n");

#define END_COMPILATION_MESSAGE                                                                \
    printf("\n\n\n");                                                                          \
    printf("<!> ### ============================================================= ### <!>\n"); \
    printf("<!> ### - - - - - - - - - - - End of Compilation - - - - - - - - - -  ### <!>\n"); \
    printf("<!> ### ============================================================= ### <!>\n"); \
    printf("\n\n\n");

#define PRINT_AST_START                                                                \
    printf("\n\n\n");                                                                  \
    printf("<> ~~~ ======================================================= ~~~ <>\n"); \
    printf("<> ~~~                     AST Tree Output                     ~~~ <>\n"); \
    printf("<> ~~~ ======================================================= ~~~ <>\n"); \
    printf("\n");

#define PRINT_AST_END                                                                  \
    printf("\n");                                                                      \
    printf("<> ~~~ ======================================================= ~~~ <>\n"); \
    printf("<> ~~~                 End of AST Tree Output                  ~~~ <>\n"); \
    printf("<> ~~~ ======================================================= ~~~ <>\n"); \
    printf("\n");

#endif // PRINT_MACROS_H
