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
#ifndef CRYO_TEST_SUITE_H
#define CRYO_TEST_SUITE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Structure to represent an individual test
typedef struct CryoTest
{
    const char *name;
    bool (*testFunc)(void);
    void (*setupFunc)(void);
    void (*teardownFunc)(void);
} CryoTest;

// Structure to represent the test suite
typedef struct CryoTestSuite
{
    const char *name;
    CryoTest *tests;
    int testCount;
    int capacity;
} CryoTestSuite;

// Function prototypes
CryoTestSuite *createTestSuite(const char *name, int capacity);
void addTest(CryoTestSuite *suite, const char *name, bool (*testFunc)(void), void (*setupFunc)(void), void (*teardownFunc)(void));
void runTests(const CryoTestSuite *suite);

#endif // CRYO_TEST_SUITE_H
