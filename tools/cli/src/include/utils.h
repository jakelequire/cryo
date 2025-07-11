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
#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include <stdbool.h>
#include <unistd.h>

char *trimLastDir(char *path);
char *getAbsolutePath(char *path);
char *concatStrings(char *str1, char *str2);
bool stringCompare(char *str1, char *str2);
bool runSystemCommand(const char *command);

char *getCompilerExePath(void);
char *getCompilerBinPath(void);
char *getCryoRootDir(void);

void runMainBinary(const char *buildDir, const char *binName);

#endif // UTILS_H