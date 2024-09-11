

## 9/10/24
Additions:
    - Boolean values reading correctly now.
    - Better function handling
        * Correct return type output
    - Literal variables working (ints / strings / booleans)

Issues:
    - String arrays not correctly reflecting type. Currently displaying
    the largest strings length as the type:
    ["str", "strTwo", "strThree"] = [3 x [i8 x 9]] (+1 for null terminator)

    - Function calls not reflecting correct type in arguments.
    