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
#include "compiler/error.h"


CryoCallStack callStack;


/* ====================================================================== */
/* @Call_Stack */

// <initCallStack>
void initCallStack(CryoCallStack *callStack, int initialCapacity) {
    callStack->size = 0;
    callStack->capacity = initialCapacity;
    callStack->stack = (CryoStackFrame *)malloc(initialCapacity * sizeof(CryoStackFrame));
    if (callStack->stack == NULL) {
        fprintf(stderr, "Failed to allocate memory for call stack\n");
        exit(1);
    }
}
// </initCallStack>


// <freeCallStack>
void freeCallStack(CryoCallStack *callStack) {
    for (int i = 0; i < callStack->size; ++i) {
        free(callStack->stack[i].functionName);
    }
    free(callStack->stack);
}
// </freeCallStack>


// <resizeCallStack>
void resizeCallStack(CryoCallStack *callStack) {
    callStack->capacity *= 4;
    callStack->stack = (CryoStackFrame *)realloc(callStack->stack, callStack->capacity * sizeof(CryoStackFrame));
    if (callStack->stack == NULL) {
        fprintf(stderr, "Failed to reallocate memory for call stack\n");
        exit(1);
    }
}
// </resizeCallStack>


// <pushCallStack>
void pushCallStack(CryoCallStack *callStack, const char *functionName, int lineNumber) {
    if (callStack->size >= callStack->capacity) {
        resizeCallStack(callStack);
    }
    callStack->stack[callStack->size].functionName = strdup(functionName);
    if (callStack->stack[callStack->size].functionName == NULL) {
        fprintf(stderr, "Failed to duplicate function name\n");
        exit(1);
    }
    callStack->stack[callStack->size].lineNumber = lineNumber;
    callStack->size++;
}
// </pushCallStack>


// <popCallStack>
void popCallStack(CryoCallStack *callStack) {
    if (callStack->size > 0) {
        free(callStack->stack[callStack->size - 1].functionName);
        callStack->size--;
    } else {
        printf("Call stack underflow!\n");
    }
}
// </popCallStack>


// <printStackTrace>
void printStackTrace(CryoCallStack *callStack) {
    printf("Stack Trace:\n");
    for (int i = 0; i < callStack->size; i++) {
        printf("%d \tat %s (line %d)\n", i, callStack->stack[i].functionName, callStack->stack[i].lineNumber);
    }
} 
// </printStackTrace>




