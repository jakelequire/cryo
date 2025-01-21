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
#ifndef MESSAGES_H
#define MESSAGES_H

#include "consoleColors.h"

// ====================================================================================================
// init.c Messages
// ====================================================================================================

#define MAIN_FILE_BUFFER "namespace Main;\n"                                   \
                         "\n"                                                  \
                         "function main(argc: int, argv: string[]) -> int {\n" \
                         "    printStr(\"Hello, World!\");\n"                  \
                         "    return 0;\n"                                     \
                         "}\n"

#define PROJECT_SETTINGS_BUFFER "# Cryo Project Settings\n"      \
                                "[project]\n"                    \
                                "project_name: \"MyProject\"\n"  \
                                "project_version: \"0.1.0\"\n"   \
                                "project_author: \"John Doe\"\n" \
                                "project_description: \"A Cryo project\"\n"

#define BUILDING_WITH_GDB "\n\n" BOLD CYAN                                                         \
                          "+--------------------------------------------------------+\n"           \
                          "|                Building with GDB enabled                |\n"          \
                          "+--------------------------------------------------------+" COLOR_RESET \
                          "\n\n"

#endif // MESSAGES_H
