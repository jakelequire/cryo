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
#include "tests/cryoTestSuite.h"

// Create a new test suite with the given capacity
CryoTestSuite *createTestSuite(const char *name, int capacity)
{
    CryoTestSuite *suite = (CryoTestSuite *)malloc(sizeof(CryoTestSuite));
    suite->name = name;
    suite->tests = (CryoTest *)malloc(sizeof(CryoTest) * capacity);
    suite->testCount = 0;
    suite->capacity = capacity;
    return suite;
}

// Add a test to the test suite
void addTest(CryoTestSuite *suite, const char *name, bool (*testFunc)(void), void (*setupFunc)(void), void (*teardownFunc)(void))
{
    if (suite->testCount >= suite->capacity)
    {
        fprintf(stderr, "Error: Test suite capacity exceeded\n");
        return;
    }
    suite->tests[suite->testCount].name = name;
    suite->tests[suite->testCount].testFunc = testFunc;
    suite->tests[suite->testCount].setupFunc = setupFunc;
    suite->tests[suite->testCount].teardownFunc = teardownFunc;
    suite->testCount++;
}

// Run all tests in the test suite and report results
void runTests(const CryoTestSuite *suite)
{
    int passed = 0;
    printf("Running test suite: %s\n", suite->name);
    for (int i = 0; i < suite->testCount; i++)
    {
        CryoTest *test = &suite->tests[i];
        printf("Running test: %s... ", test->name);
        if (test->setupFunc)
            test->setupFunc();
        bool result = test->testFunc();
        if (test->teardownFunc)
            test->teardownFunc();
        if (result)
        {
            printf("PASSED\n");
            passed++;
        }
        else
        {
            printf("FAILED\n");
        }
    }
    printf("Passed %d/%d tests in suite: %s\n", passed, suite->testCount, suite->name);
    free(suite->tests);
    free(suite);
}
