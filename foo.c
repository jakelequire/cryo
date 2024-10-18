#include <stdio.h>
// clang -S -emit-llvm foo.c

typedef struct Int
{
    int value;
    int length;
} Int;

typedef struct String
{
    char *value;
    int length;
} String;

int main()
{
    Int testInt;
    String testString;

    return 0;
}
