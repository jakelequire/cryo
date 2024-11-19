
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

class FooClass
{
public:
    FooClass()
    {
        int a = 5;
        int b = 10;
        int c = a + b;
    }
};

int main()
{
    Int i(5);
    int x = i.value;
    i.toString();

    FooClass foo;

    return 0;
}
