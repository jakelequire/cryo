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
#ifndef FS_H
#define FS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "tools/macros/debugMacros.h"
#include "tools/utils/c_logger.h"

typedef struct CryoSrcLocations
{
    const char *rootDir;
} CryoSrcLocations;

#ifdef __cplusplus
extern "C"
{
#endif

    char *readFile(const char *path);
    bool fileExists(const char *path);

#ifdef __cplusplus
}
#endif

char *readFile(const char *path);
bool fileExists(const char *path);
int dirExists(const char *path);
int createDir(const char *path);
void removeFile(const char *filePath);
const char *getSTDFilePath(const char *subModule);
const char *trimFilePath(const char *filePath);
char *getFileNameFromPathNoExt(const char *filePath);
char *getFileNameFromPath(const char *filePath);

const char *getCurRootDir(void);
const char *getCryoSrcLocation(void);
char *getCRuntimePath(void);

const char *appendStrings(const char *str1, const char *str2);
const char *appendExtensionToFileName(const char *fileName, const char *extension);
const char *appendPathToFileName(const char *path, const char *fileName, bool endingSlash);
const char *removeFileFromPath(const char *path);
const char *changeFileExtension(const char *fileName, const char *newExtension);

char *getPathFromCryoPath(void);
char *getPathFromEnvVar(void);
char *getCompilerBinPath(void);
char *getCompilerRootPath(void);

#define CUR_ROOT_DIR_ABS
#define CUR_ROOT_DIR_REL
#endif
