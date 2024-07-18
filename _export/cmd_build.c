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
#include "cli/build.h"


BuildArgs getBuildArgs(char* arg) {
    if strcmp(arg, "-s") == 0           return BUILD_ARG;
    if strcmp(arg, "-dir") == 0         return BUILD_ARG_DIR;

    return BUILD_ARG_UNKNOWN;
}


void executeBuildCmd(char* argv[]) {
    char* argument = argv[0];
    if(argument == NULL) {
        // Error handling, idk yet
    }

    BuildArgs getBuildArgs(argument);
    switch (arg) {
        case BUILD_ARG:
            // Execute Command
            break;
        case BUILD_ARG_DIR:
            // Execute Command
            break;
        
        case BUILD_ARG_UNKNOWN:
            // Execute Command
            break;

        default:
            // Do something
    }
}
