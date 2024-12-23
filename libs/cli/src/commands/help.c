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
#include "commands.h"

void exe_CLI_help(void)
{
    printf("\n" BOLD "Cryo CLI Help" COLOR_RESET "\n");
    printf(BOLD "Usage:" COLOR_RESET " cryo [command] [options]\n\n");
    printf(BOLD "Commands:" COLOR_RESET "\n");
    printf("  " BOLD "help" COLOR_RESET "       - Display this help message\n");
    printf("  " BOLD "build" COLOR_RESET "      - Build the project\n");
    printf("  " BOLD "version" COLOR_RESET "    - Display the version of the CLI\n");
    printf("  " BOLD "env" COLOR_RESET "        - Enter the Cryo environment\n");
    printf("\n" BOLD "Options:" COLOR_RESET "\n");

    printf("  " BOLD "build" COLOR_RESET "\n");
    printf("    --release    Build in release mode\n");
    printf("    --debug      Build in debug mode\n");

    printf("\n");
}
