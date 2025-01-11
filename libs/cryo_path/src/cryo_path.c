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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

char *appendDir(const char *dir, const char *addedPath, bool needsSlash)
{
    // Append the new directory to the path
    char *newDir = (char *)malloc(1024);
    if (newDir)
    {
        strcpy(newDir, dir);
        if (needsSlash)
        {
            strcat(newDir, "/");
        }
        strcat(newDir, addedPath);
        return newDir;
    }

    return NULL;
}

char *getCurrentDir(void)
{
    char *buffer = (char *)malloc(1024);
    if (buffer)
    {
        return getcwd(buffer, 1024);
    }
    return NULL;
}

int main()
{
    const char *dir = getCurrentDir();
    const char *bin_dir = appendDir(dir, "bin/", true);
    fprintf(stdout, "%s\n", bin_dir);

    (void)free((void *)dir);
    (void)free((void *)bin_dir);
    return 0;
}
