
struct IntStructTesting
{
    int value;
    IntStructTesting(int v) : value(v) {};
};

void fooTest(void) { IntStructTesting i = 34; }

int main()
{
    fooTest();
    return 0;
}