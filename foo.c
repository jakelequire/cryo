
// clang -S -emit-llvm foo.c

char *test(char *string)
{
    return string;
}

char *foo(char *string)
{
    string = "Reassigned!";
    return string;
}

int main()
{
    char *string = "Hello, World!";
    char *result = test(string);
    return 0;
}
