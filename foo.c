#include <stdio.h>
// clang -S -emit-llvm foo.c

void fib2(int n)
{
    int a = 0;
    int b = 1;
    int c;

    while (n > 0)
    {
        c = a + b;
        a = b;
        b = c;
        printf("%d\n", a);
        n = n - 1;
    }

    return;
}

int main()
{
    fib2(10);
    return 0;
}
