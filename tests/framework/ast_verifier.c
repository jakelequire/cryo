#include "ast_verifier.h"
#include "test_runner.h"
#include <stdio.h>
#include <string.h>

TestResult verify_ast(const char *ast_output, const char *expected_output)
{
    // For now, just do a simple string comparison
    // In a more advanced implementation, you might want to parse both ASTs
    // and compare them structurally
    if (strcmp(ast_output, expected_output) == 0)
    {
        return TEST_RESULT_PASS;
    }
    else
    {
        return TEST_RESULT_FAIL;
    }
}

void register_ast_tests(void)
{
    // Register all AST tests
    // This would be populated with your actual test cases

    // Example test case
    // Example test case
    Test test_case = {
        .name = "Variable Expression Test",
        .source_file = "tests/cryo_tests/0002.var_expressions.cryo",
        .answer_key_file = "tests/compiler_tests/answer_key/0002.var_expressions.ast",
        .test_type = TEST_TYPE_AST,
        .test_function = verify_ast};

    test_runner_add_test(test_case);

    // Add more test cases here
}
