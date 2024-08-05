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
#ifndef DEV_WATCH_H
#define DEV_WATCH_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define sleep(x) Sleep(x)
#define mkdir(name, mode) _mkdir(name)
#else
#include <unistd.h>
#include <sys/inotify.h>
#endif

#define MAX_PATH_LEN 1024

typedef enum
{
    DEV_WATCH_ARG_HELP,
    DEV_WATCH_ARG_START,

    DEV_WATCH_ARG_UNKNOWN
} DevWatchArgs;

typedef struct FileInfo
{
    char path[MAX_PATH_LEN];
    time_t mtime;
} FileInfo;

DevWatchArgs getDevWatchArg(char *arg);
void executeDevWatchCmd(char *argv[]);
int shouldIgnore(const char *path);
char *getBasePath();
void checkDirectory(const char *basePath, FileInfo **files, int *count, int *capacity);
void clearLine();
void *commandListener(void *arg);
void executeDevWatch(const char *basePath);
void dirChangeEvent(const char *basePath);
bool findObjectFile(char *fileName);
void rebuildProject();
void renameExecutable(const char *oldPath, const char *newPath);
void replaceExecutable(const char *oldPath, const char *newPath);
void executeSysCommand(const char *command);
#endif // DEV_WATCH_H
