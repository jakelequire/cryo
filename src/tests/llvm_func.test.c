#include <stdio.h>

extern int simple_func();

int main() {
    int result = simple_func();
    printf("Result: %d\n", result);
    return 0;
}
