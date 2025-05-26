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
#ifndef COMPILE_TIMER_H
#define COMPILE_TIMER_H
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "tools/macros/consoleColors.h"

#define STOP_COMPILER_TIMER                   \
    double elapsed = stopTimer(compileTimer); \
    getSystemInfo(buildStats);                \
    addElapsedTime(buildStats, elapsed);      \
    printBuildStats(buildStats);

#define START_COMPILER_TIMER                           \
    CompileTimer *compileTimer = createCompileTimer(); \
    startTimer(compileTimer);

typedef struct CompileTimer
{
    clock_t start;
    clock_t end;
    double elapsed;
} CompileTimer;

CompileTimer *createCompileTimer();
void startTimer(CompileTimer *timer);
double stopTimer(CompileTimer *timer);

#endif // COMPILE_TIMER_H