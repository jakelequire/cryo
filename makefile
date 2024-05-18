CC = gcc
CFLAGS = -Wall -g -Wextra
LDFLAGS = -lm

# Define paths
BIN_DIR = ./src/bin/
SRC_DIR = ./src/
COMPILER_DIR = $(SRC_DIR)compiler/
UTILS_DIR = $(SRC_DIR)utils/

LEXER_DIR = $(SRC_DIR)compiler/
PARSER_DIR = $(SRC_DIR)compiler/
TOKENIZER_DIR = $(SRC_DIR)compiler/
AST_DIR = $(SRC_DIR)compiler/

CLI_DIR = cli/
CLI_BIN = cli/bin/

# Include directories
INCLUDES = -I./src/ -I./src/include/
CLI_INCLUDES = -I./cli/

# Source files
LEXER_SRC = $(LEXER_DIR)lexer.c
PARSER_SRC = $(PARSER_DIR)parser.c
CLI_SRC = $(CLI_DIR)cli.c
UTILS_SRC = $(UTILS_DIR)printAST.c
AST_SRC = $(AST_DIR)ast.c
CODEGEN_SRC = $(COMPILER_DIR)codegen.c
IR_SRC = $(COMPILER_DIR)ir.c
MAIN_SRC = $(SRC_DIR)main.c
SEMANTIC_SRC = $(COMPILER_DIR)semanticS.c
COMMANDS_SRC = $(CLI_DIR)commands.c


# Binaries
LEXER_BIN = $(BIN_DIR)lexer
PARSER_BIN = $(BIN_DIR)parser
MAIN_BIN = $(BIN_DIR)main

# Default buildd
all: lexer parser cli main

# Individual components
lexer:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(LEXER_BIN) $(LEXER_SRC)

parser:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(PARSER_BIN) $(PARSER_SRC) $(LEXER_SRC)

main:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN_BIN) $(MAIN_SRC) $(LEXER_SRC) $(PARSER_SRC) $(AST_SRC) $(CODEGEN_SRC) $(IR_SRC) $(SEMANTIC_SRC) 

# ---------------------------------------------------------------
# CLI
cli:
	$(CC) $(CFLAGS) $(CLI_INCLUDES) -o $(CLI_BIN)cryo.exe $(CLI_SRC) $(COMMANDS_SRC)

runcli:
	$(CLI_BIN)
# ---------------------------------------------------------------

# Running executables
runlexer:
	$(LEXER_BIN)

runparser:
	$(PARSER_BIN)


runmain:
	$(MAIN_BIN) ./src/tests/data/test1.cy

# Clean up
clean:
	del /F $(BIN_DIR)*.exe

.PHONY: all lexer parser cli main clean runlexer runparser runcli runmain
