const bar = Global.this(() => a + b + 5);

public fn (a: str, b: str) -> Result<str, Error> {
    const foo = bar;
    bar?.ok_then(|x| printf("{}", x)).ok_else(|e| printf("{}", e));
}
