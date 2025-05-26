// test_runner.c
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TESTS 1000

static Test tests[MAX_TESTS];
static int test_count = 0;

void test_runner_init(void)
{
    test_count = 0;
}

void test_runner_add_test(Test test)
{
    if (test_count < MAX_TESTS)
    {
        tests[test_count++] = test;
    }
    else
    {
        fprintf(stderr, "Error: Maximum number of tests reached\n");
        exit(EXIT_FAILURE);
    }
}

int test_runner_run_all(void)
{
    int passed = 0;
    int failed = 0;
    clock_t start_time = clock();

    printf("\n===== Running All Tests =====\n");

    for (int i = 0; i < test_count; i++)
    {
        printf("Test %d/%d: %s... ", i + 1, test_count, tests[i].name);

        char *source = read_file_contents(tests[i].source_file);
        char *answer_key = read_file_contents(tests[i].answer_key_file);

        if (!source || !answer_key)
        {
            printf("FAILED (could not read files)\n");
            failed++;
            free(source);
            free(answer_key);
            continue;
        }

        bool result = tests[i].test_function(source, answer_key);

        if (result)
        {
            printf("PASSED\n");
            passed++;
        }
        else
        {
            printf("FAILED\n");
            failed++;
        }

        free(source);
        free(answer_key);
    }

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("\n===== Test Results =====\n");
    printf("Total: %d\n", test_count);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Time: %.2f seconds\n", time_taken);

    return failed;
}

int test_runner_run_type(TestType type)
{
    int passed = 0;
    int failed = 0;
    int total = 0;
    clock_t start_time = clock();

    printf("\n===== Running %s Tests =====\n",
           type == TEST_TYPE_AST ? "AST" : type == TEST_TYPE_IR   ? "IR"
                                       : type == TEST_TYPE_STDOUT ? "STDOUT"
                                                                  : "Unknown");

    for (int i = 0; i < test_count; i++)
    {
        if (tests[i].test_type == type || tests[i].test_type == TEST_TYPE_ALL)
        {
            total++;
            printf("Test %d: %s... ", total, tests[i].name);

            char *source = read_file_contents(tests[i].source_file);
            char *answer_key = read_file_contents(tests[i].answer_key_file);

            if (!source || !answer_key)
            {
                printf("FAILED (could not read files)\n");
                failed++;
                free(source);
                free(answer_key);
                continue;
            }

            bool result = tests[i].test_function(source, answer_key);

            if (result)
            {
                printf("PASSED\n");
                passed++;
            }
            else
            {
                printf("FAILED\n");
                failed++;
            }

            free(source);
            free(answer_key);
        }
    }

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("\n===== Test Results =====\n");
    printf("Total: %d\n", total);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Time: %.2f seconds\n", time_taken);

    return failed;
}

void test_runner_cleanup(void)
{
    // Cleanup any resources if needed
}

char *read_file_contents(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, file_size, file);
    buffer[read_size] = '\0';

    fclose(file);
    return buffer;
}
