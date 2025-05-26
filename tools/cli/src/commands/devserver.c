/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "../include/commands.h"

void exe_CLI_devserver(void)
{
    printf("Starting dev server...\n");
    const char *cryoRootDir = getCryoRootDir();
    const char *bin_file = "/bin/dev-server";
    printf("Cryo Root Dir: %s\n", cryoRootDir);
    char *bin_path = (char *)malloc(strlen(cryoRootDir) + strlen(bin_file) + 1);
    strcpy(bin_path, cryoRootDir);
    strcat(bin_path, bin_file);
    printf("Bin Path: %s\n", bin_path);

    // Spawn a new process for the dev server
    char *const args[2] = {bin_path, (char *const)cryoRootDir};
    int child_proc = execv(bin_path, args);
    if (child_proc == -1)
    {
        fprintf(stderr, "Failed to start dev server\n");
        exit(1);
    }
    free(bin_path);
}
