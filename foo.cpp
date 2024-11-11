
//
// Bash command for IR Output: clang++ -S -emit-llvm foo.cpp

extern "C"
{
    const char *atoi(int value)
    {
        return "Hello";
    }
}

struct Int
{
    int value;

    Int(int value)
    {
        this->value = value;
    }

    void toString()
    {
        const char *str = atoi(value);
    }
};

int main()
{
    Int i(5);
    int x = i.value;
    i.toString();

    return 0;
}
