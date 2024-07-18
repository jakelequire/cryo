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
#include "cli/help.h"



// <getHelpArg>
HelpArgs getHelpArg(char* arg) {
    if strcmp(arg, "help") == 0         return HELP_ARG_HELP;
    if strcmp(arg, "version") == 0      return HELP_ARG_VERSION;
    if strcmp(arg, "build") == 0        return HELP_ARG_BUILD;
    if strcmp(arg, "init") == 0         return HELP_ARG_INIT;
    
    return HELP_ARG_UNKNOWN;
}
// </getHelpArg>



// <executeHelpCmd>
void executeHelpCmd(char* argv[]) {
    char* argument = argv[0];
    if(argument == NULL) {
        // Error handling, idk yet
    }

    HelpArgs getHelpArg(argument);
    switch (arg) {
        case HELP_ARG_HELP:
            // Execute Command
            break;
            
        case HELP_ARG_VERSION:
            // Execute Command
            break;

        case HELP_ARG_BUILD:
            // Execute Command
            break;

        case HELP_ARG_INIT:
            // Execute Command
            break;
        
        case HELP_ARG_UNKNOWN:
            // Handle Unknown Command
            break;

        default:
            // idk
            break;
    }
}
// </executeHelpCmd>




