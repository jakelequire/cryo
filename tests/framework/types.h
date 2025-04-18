#ifndef TYPES_H
#define TYPES_H

typedef enum TestType
{
    TEST_TYPE_AST,
    TEST_TYPE_IR,
    TEST_TYPE_STDOUT,
    TEST_TYPE_ALL
} TestType;

typedef enum TestResult
{
    TEST_RESULT_PASS,
    TEST_RESULT_FAIL,
    TEST_RESULT_SKIP
} TestResult;

typedef struct Test_t
{
    const char *name;
    const char *source_file;
    const char *answer_key_file;
    TestType test_type;
    TestResult (*test_function)(const char *source, const char *answer_key);
} Test;

#endif // TYPES_H