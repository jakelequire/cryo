CC = gcc
CFLAGS = -Wall -g -Wextra
LDFLAGS = -lm

# Define paths
BIN_DIR = ./src/bin/
SRC_DIR = ./src/
CLI_DIR = $(SRC_DIR)cli/
LEXER_DIR = $(SRC_DIR)compiler/lexer/
PARSER_DIR = $(SRC_DIR)compiler/parser/
TOKENIZER_DIR = $(SRC_DIR)compiler/tokenizer/
UTILS_DIR = $(SRC_DIR)utils/

# Include directories
INCLUDES = -I./src/ -I./src/include/

# Source files
LEXER_SRC = $(LEXER_DIR)lexer.c
PARSER_SRC = $(PARSER_DIR)parser.c
CLI_SRC = $(CLI_DIR)cli.c
UTILS_SRC = $(UTILS_DIR)printAST.c
MAIN_SRC = $(SRC_DIR)main.c
AST_SRC = $(SRC_DIR)utils/ast.c

# Binaries
LEXER_BIN = $(BIN_DIR)lexer
PARSER_BIN = $(BIN_DIR)parser
CLI_BIN = $(BIN_DIR)cli
MAIN_BIN = $(BIN_DIR)main

# Default build
all: lexer parser cli main

# Individual components
lexer:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(LEXER_BIN) $(LEXER_SRC)

parser:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(PARSER_BIN) $(PARSER_SRC) $(LEXER_SRC)

cli:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(CLI_BIN) $(CLI_SRC) $(UTILS_SRC)

main:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN_BIN) $(MAIN_SRC) $(LEXER_SRC) $(PARSER_SRC) $(AST_SRC)

# Running executables
runlexer:
	$(LEXER_BIN)

runparser:
	$(PARSER_BIN)

runcli:
	$(CLI_BIN)

runmain:
	$(MAIN_BIN) ./src/tests/data/test1.cy

# Clean up
clean:
	del /F $(BIN_DIR)*.exe

.PHONY: all lexer parser cli main clean runlexer runparser runcli runmain
