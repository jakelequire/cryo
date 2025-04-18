// ast_verifier.h
#ifndef AST_VERIFIER_H
#define AST_VERIFIER_H
#include "types.h"
#include <stdbool.h>

// Function to verify AST output against expected result
enum TestResult verify_ast(const char *actual, const char *expected);

// Register AST verification tests
void register_ast_tests(void);

#endif // AST_VERIFIER_H
