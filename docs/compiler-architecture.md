# The Cryo Compiler

This document describes the architecture of the Cryo compiler. The Cryo compiler is designed to be modular and extensible, allowing for easy addition of new features and improvements.

This is a passion project and is only maintained and developed by [me, Jake LeQuire](https://github.com/jakelequire). I am a self-taught developer and have treated this project as a learning experience and a way to improve my skills over the last year+. I have learned a lot about compiler design and implementation, and I hope to continue to improve the compiler over time.

I have no intentions of making this a production-ready compiler / language, but I do hope to make it a fun and interesting project to work on.

See the [release notes](./releaseNotes.md) for a list of features and improvements in each version of the compiler.

## Compiler Architecture
The Cryo compiler is designed to be modular and extensible. The compiler is divided into several components, each responsible for a specific part of the compilation process. The components are as follows:

- **lexer.c** - The lexer is responsible for tokenizing the input source code. The lexer reads the source code and produces a stream of tokens that are passed to the parser. The lexer is implemented in C and is designed to be extensible, allowing for easy addition of new tokens and token types.
- **parser.c** - The parser is responsible for parsing the stream of tokens produced by the lexer. The parser reads the tokens and produces an abstract syntax tree (AST) that represents the structure of the source code. The parser is designed to be extensible, allowing for easy addition of new grammar rules and parsing techniques.