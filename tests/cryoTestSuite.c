// cryo_test_suite.c
#include "./framework/test_runner.h"
#include "./framework/ast_verifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    test_runner_init();

    // Register all tests
    register_ast_tests();
    // register_ir_tests(); // You would implement these similarly to AST tests
    // register_stdout_tests();

    int result = 0;

    if (argc < 2)
    {
        // Run all tests by default
        result = test_runner_run_all();
    }
    else if (strcmp(argv[1], "ast") == 0)
    {
        result = test_runner_run_type(TEST_TYPE_AST);
    }
    else if (strcmp(argv[1], "ir") == 0)
    {
        result = test_runner_run_type(TEST_TYPE_IR);
    }
    else if (strcmp(argv[1], "stdout") == 0)
    {
        result = test_runner_run_type(TEST_TYPE_STDOUT);
    }
    else
    {
        printf("Unknown test type: %s\n", argv[1]);
        printf("Usage: %s [ast|ir|stdout]\n", argv[0]);
        return EXIT_FAILURE;
    }

    test_runner_cleanup();

    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
