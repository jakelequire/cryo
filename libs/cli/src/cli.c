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
#include "cli.h"

DirectoryInfo *get_directory_info(char *argv[])
{
    DirectoryInfo *info = (DirectoryInfo *)malloc(sizeof(DirectoryInfo));
    char *absolute = getAbsolutePath(argv[0]);
    char *cryoRoot = trimLastDir(trimLastDir(absolute));
    // Bin Dir = cryoRoot + /bin
    char *binDir = concatStrings(cryoRoot, "/bin");

    info->cryo_abs_dir = strdup(cryoRoot);
    info->bin_dir = strdup(binDir);

    return info;
}

bool doesCompilerExeExist(DirectoryInfo *info)
{
    char *compiler = concatStrings((char *)info->bin_dir, "/compiler");

    FILE *file = fopen(compiler, "r");
    if (file)
    {
        fclose(file);
        return true;
    }

    return false;
}

void printDirectoryInfo(DirectoryInfo *info)
{
    printf("Cryo Directory: %s\n", info->cryo_abs_dir);
    printf("Bin Directory: %s\n", info->bin_dir);
}

int main(int argc, char *argv[])
{
    DirectoryInfo *info = get_directory_info(argv);

    // Check for no arguments
    if (argc == 1)
    {
        exe_CLI_help();
        return 0;
    }

    if (doesCompilerExeExist(info))
    {
        printf("Compiler exists!\n");
    }
    else
    {
        printf("Compiler does not exist!\n");
    }

    return 0;
}
