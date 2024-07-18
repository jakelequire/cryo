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
#include "cli/cli.h"


// <getCommandType>
CommandType getCommandType(const char* command) {
    printf("Command: %s\n", command);
    if (strcmp(command, "help") == 0)     return CMD_HELP;
    if (strcmp(command, "version") == 0)  return CMD_VERSION;
    if (strcmp(command, "-v") == 0)       return CMD_VERSION;
    if (strcmp(command, "build") == 0)    return CMD_BUILD;
    if (strcmp(command, "init") == 0)     return CMD_INIT;
    if (strcmp(command, "wdev") == 0)     return CMD_DEV_WATCH;

    return CMD_UNKNOWN;
}
// </getCommandType>


// <executeCommand>
void executeCommand(CommandType command, char* argv) {
    switch(command) {
        case CMD_HELP:          executeHelpCmd(argv);
        case CMD_VERSION:       executeVersionCmd(argv);
        case CMD_BUILD:         executeBuildCmd(argv);
        case CMD_INIT:          executeInitCmd(argv);
        case CMD_DEV_WATCH:     executeDevWatchCmd(argv);

        default:                // executeUnknownCmd();
    }
}
// </executeCommand>



/* =========================================================== */
// <main>
int main(int argc, char** argv) {
    if (argc < 2) {
        executeHelpCmd(argv);
        return 1;
    }

    CommandType command = getCommandType(argv[1]);
    executeCommand(command, argv);

    return 0;
}
// </main>
