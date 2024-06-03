PS C:\Programming\apps\cryo> .\src\bin\main.exe .\main.cryo
VERBOSE [[Main]]: Starting Task...
 | Token: TASK_START
{lexer} -------------- <Input Source Code> --------------


{lexer} Lexer initialized.
Start: 000002761E098B30
Current: 000002761E098B30

Source:
-------

public fn main() -> int {
    return 1;
}



{lexer} -------------------- <END> ----------------------

[DEBUG] Lexer initialized
{parser} [DEBUG] Program node allocated
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: public fn main() -> int {
    return 1;
}

{lexer} [DEBUG] Created token: public (Type: 55, Line: 2, Column: 2)
### [VERBOSE] [[Lexer]] Cryo Token: 55
{parser} [DEBUG] Token: public, Type: 55, Line: 2, Column: 2
{parser} [DEBUG] Parsing statement...
{parser} [DEBUG] Parsing function declaration...
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: fn main() -> int {
    return 1;
}

{lexer} [DEBUG] Created token: fn (Type: 52, Line: 2, Column: 4)
### [VERBOSE] [[Lexer]] Cryo Token: 52
{parser} [DEBUG] Token: fn, Type: 52, Line: 2, Column: 4
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: main() -> int {
    return 1;
}

{lexer} [DEBUG] Created token: m (Type: 1, Line: 2, Column: 6)
### [VERBOSE] [[Lexer]] Cryo Token: 1
{lexer} [DEBUG] Created token: main (Type: 1, Line: 2, Column: 9)
### [VERBOSE] [[Lexer]] Cryo Token: 1
{parser} [DEBUG] Token: main, Type: 1, Line: 2, Column: 9
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: () -> int {
    return 1;
}

{lexer} [DEBUG] Created token: ( (Type: 131, Line: 2, Column: 10)
### [VERBOSE] [[Lexer]] Cryo Token: 131
{parser} [DEBUG] Token: (, Type: 131, Line: 2, Column: 10
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: ) -> int {
    return 1;
}

{lexer} [DEBUG] Created token: ) (Type: 132, Line: 2, Column: 11)
### [VERBOSE] [[Lexer]] Cryo Token: 132
{parser} [DEBUG] Token: ), Type: 132, Line: 2, Column: 11
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: -> int {
    return 1;
}

{lexer} [DEBUG] Created token: -> (Type: 17, Line: 2, Column: 14)
### [VERBOSE] [[Lexer]] Cryo Token: 17
{parser} [DEBUG] Token: ->, Type: 17, Line: 2, Column: 14
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: int {
    return 1;
}

{lexer} [DEBUG] Created token: i (Type: 1, Line: 2, Column: 16)
### [VERBOSE] [[Lexer]] Cryo Token: 1
{lexer} [DEBUG] Created token: int (Type: 1, Line: 2, Column: 18)
### [VERBOSE] [[Lexer]] Cryo Token: 1
{parser} [DEBUG] Token: int, Type: 1, Line: 2, Column: 18
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: {
    return 1;
}

{lexer} [DEBUG] Created token: { (Type: 135, Line: 2, Column: 20)
### [VERBOSE] [[Lexer]] Cryo Token: 135
{parser} [DEBUG] Token: {, Type: 135, Line: 2, Column: 20
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: return 1;
}

{lexer} [DEBUG] Created token: r (Type: 1, Line: 3, Column: 6)
### [VERBOSE] [[Lexer]] Cryo Token: 1
{lexer} [DEBUG] Created token: return (Type: 1, Line: 3, Column: 11)
### [VERBOSE] [[Lexer]] Cryo Token: 1
{parser} [DEBUG] Token: return, Type: 1, Line: 3, Column: 11
{parser} [DEBUG] Parsing statement...
{parser} [DEBUG] Parsing expression statement...
{parser} [DEBUG] Parsing expression statement...
{parser} [DEBUG] Parsing expression...
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: 1;
}

{lexer} [DEBUG] Created token: 1 (Type: 18, Line: 3, Column: 13)
{lexer} [DEBUG] Number token: 1
### [VERBOSE] [[Lexer]] Cryo Token: 18
{parser} [DEBUG] Token: 1, Type: 18, Line: 3, Column: 13
{parser} [DEBUG] Getting next token...
VERBOSE [[Lexer]]: Processing char | Token: ;
}

{lexer} [DEBUG] Created token: ; (Type: 126, Line: 3, Column: 14)
### [VERBOSE] [[Lexer]] Cryo Token: 126
{parser} [DEBUG] Token: ;, Type: 126, Line: 3, Column: 14
{parser} Error: Expected an expression at line 3, column 14
PS C:\Programming\apps\cryo>