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
/*------ <includes> ------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#include "common/common.h"
/*---------<end>---------*/
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
const char *getSTDFilePath(const char *subModule);
const char *trimFilePath(const char *filePath);

#define CUR_ROOT_DIR_ABS
#define CUR_ROOT_DIR_REL
#endif