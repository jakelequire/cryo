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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#if defined(__linux__)
#include <linux/limits.h>
#endif

char *getExecutablePath(void)
{
    char *buffer = (char *)malloc(PATH_MAX);
    if (!buffer)
        return NULL;

    ssize_t len;

#if defined(__linux__)
    len = readlink("/proc/self/exe", buffer, PATH_MAX - 1);
#elif defined(__APPLE__)
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(buffer, &size) != 0)
    {
        free(buffer);
        return NULL;
    }
    len = strlen(buffer);
#else
#error "Unsupported platform"
#endif

    if (len == -1)
    {
        free(buffer);
        return NULL;
    }

    buffer[len] = '\0';

    // Remove the executable name to get the directory
    char *last_slash = strrchr(buffer, '/');
    if (last_slash != NULL)
    {
        *(last_slash + 1) = '\0';
    }

    return buffer;
}

int main()
{
    char *bin_dir = getExecutablePath();
    if (bin_dir)
    {
        fprintf(stdout, "%s\n", bin_dir);
        free(bin_dir);
        return 0;
    }
    return 1;
}
