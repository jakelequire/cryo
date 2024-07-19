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
#ifndef INIT_H
#define INIT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(name, mode) _mkdir(name)
#else
#include <unistd.h>
#endif


typedef enum {
    INIT_ARG_DIR,
    INIT_ARG_UNKNOWN
} InitArgs;



InitArgs getInitArgs        (char* arg);
void executeInitCmd         (char* argv[]);

void executeInit            (void);
void create_cryo_config     (void);
void create_cryo_main       (void);
void create_cryo_init       (void);
void create_directory       (const char* name);
void open_root_dir          (void);
void create_file            (char* filename, char* data);



#endif // INIT_H