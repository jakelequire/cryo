
public fn (a: str, b: str) -> Result<str, Error> {
    const examplevar = "example";
    mut examplevar = 2 + 10;

    for(let i = 0; i < 10; i++) {
        examplevar += i;
    }

    switch(examplevar) {
        case 1:
            return Ok("1");
        case 2:
            return Ok("2");
        default:
            return Err("error");
    }

    return Ok(a + b);
}