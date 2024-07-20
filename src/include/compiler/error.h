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
#ifndef ERROR_H
#define ERROR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "ast.h"

#define MAX_STACK_SIZE 10


typedef struct {
    char* functionName;
    int lineNumber;
} CryoStackFrame;

typedef struct {
    CryoStackFrame* stack;
    int size;
    int capacity;
} CryoCallStack;

typedef struct {
    CryoCallStack callStack;
} CryoInterpreter;

extern CryoCallStack callStack;


/* @Call_Stack */
void initCallStack              (CryoCallStack* callStack, int initialCapacity);
void freeCallStack              (CryoCallStack* callStack);
void resizeCallStack            (CryoCallStack *callStack);
void pushCallStack              (CryoCallStack* callStack, const char* functionName, int lineNumber);
void popCallStack               (CryoCallStack* callStack);
void printStackTrace            (CryoCallStack* callStack);



#endif // ERROR_H
