// test_runner.h
#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "types.h"
#include <stdbool.h>

// Initialize the test runner
void test_runner_init(void);

// Add a test to be executed
void test_runner_add_test(Test test);

// Run all registered tests
int test_runner_run_all(void);

// Run tests of a specific type
int test_runner_run_type(TestType type);

// Clean up resources
void test_runner_cleanup(void);

// Utility functions
char *read_file_contents(const char *filename);

#endif // TEST_RUNNER_H
