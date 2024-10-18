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
#include "jstd.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void test(void)
    {
        jStd jstd;
        jstd.readFile = jstd_readFile;
    }

    char *jstd_readFile(const char *path, FSPathOptions options)
    {
        printf("[JSTD : FS] Reading file: %s\n", path);
        const char *filePath = (const char *)malloc(strlen(path) + 1);
        if (filePath == NULL)
        {
            perror("[JSTD : FS] Not enough memory to read file");
            return NULL;
        }

        switch (options)
        {
        case PATH_DEFAULT:
            strcpy(filePath, path);
            break;
        case PATH_REL_PATH:
            strcpy(filePath, "./");
            strcat(filePath, path);
            break;
        case PATH_ABS_PATH:
        {
            char *cwd = jstd_getCWD();
            strcpy(filePath, cwd);
            strcat(filePath, "/");
            strcat(filePath, path);
            free(cwd);
            break;
        }
        default:
            perror("[JSTD : FS] Invalid path option");
            return NULL;
        }

        FILE *file = fopen(path, "rb"); // Open the file in binary mode to avoid transformations
        if (file == NULL)
        {
            perror("[JSTD : FS] Could not open file");
            return NULL;
        }

        fseek(file, 0, SEEK_END);
        size_t length = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (length == 0)
        {
            perror("[JSTD : FS] File is empty");
            fclose(file);
            return NULL;
        }

        char *buffer = (char *)malloc(length + 1);
        if (buffer == NULL)
        {
            perror("[JSTD : FS] Not enough memory to read file");
            fclose(file);
            return NULL;
        }

        size_t bytesRead = fread(buffer, 1, length, file);
        if (bytesRead < length)
        {
            perror("[JSTD : FS] Failed to read the full file");
            free(buffer);
            fclose(file);
            return NULL;
        }

        buffer[length] = '\0'; // Null-terminate the buffer
        return buffer;
    }

    char *jstd_getCWD(void)
    {
        char *cwd = getcwd(NULL, 0);
        if (cwd == NULL)
        {
            perror("[JSTD : FS] Could not get current working directory");
            return NULL;
        }
        return cwd;
    }

    char *jstd_getFileExtension(const char *path)
    {
        const char *extension = strrchr(path, '.');
        if (extension == NULL)
        {
            perror("[JSTD : FS] File has no extension");
            return NULL;
        }
        return extension;
    }

#ifdef __cplusplus
}
#endif
