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
#include "utils/fs.h"

// <readFile> *might rename to fsreadFile*
char *readFile(const char *path)
{
    printf("[FS] Reading file: %s\n", path);
    FILE *file = fopen(path, "rb"); // Open the file in binary mode to avoid transformations
    if (file == NULL)
    {
        perror("{FS} Could not open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    // handle fseek error
    fseek(file, 0, SEEK_SET);

    if (length == 0)
    {
        perror("{FS} File is empty");
        fclose(file);
        return NULL;
    }

    char *buffer = (char *)malloc(length + 1);
    if (buffer == NULL)
    {
        perror("{FS} Not enough memory to read file");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, length, file);
    if (bytesRead < length)
    {
        perror("{FS} Failed to read the full file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = '\0'; // Null-terminate the buffer
    fclose(file);
    return buffer;
}
// </readFile>
