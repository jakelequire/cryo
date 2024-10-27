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
#include "tools/utils/compileTimer.h"

CompileTimer *createCompileTimer()
{
    CompileTimer *timer = (CompileTimer *)malloc(sizeof(CompileTimer));
    if (!timer)
    {
        fprintf(stderr, "Error: Failed to allocate memory for CompileTimer\n");
        return NULL;
    }
    return timer;
}

void startTimer(CompileTimer *timer)
{
    timer->start = clock();
}

double stopTimer(CompileTimer *timer)
{
    timer->end = clock();
    timer->elapsed = ((double)(timer->end - timer->start)) / CLOCKS_PER_SEC;
    return timer->elapsed;
}
