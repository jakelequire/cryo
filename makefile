CC = gcc
CFLAGS = -Wall -g

BIN_PATH = ./src/bin/
LEXER_PATH = ./src/
PARSER_PATH = ./src/
COMPILER_PATH = ./src/
MAIN_PATH = ./src/

TEST_DATA_PATH = ./src/tests/data/
TEST_RESULT_PATH = ./src/tests/result/
TEST_SCRIPT = ./src/tests/

RUN_TEST = $(BIN_PATH)main $(TEST_DATA_PATH)$(1) $(TEST_RESULT_PATH)$(1)

BUILD_LEXER = $(CC) $(CFLAGS) -o $(BIN_PATH)lexer $(LEXER_PATH)lexer.c
BUILD_COMPILER = $(CC) $(CFLAGS) -o $(BIN_PATH)compiler $(COMPILER_PATH)compiler.c
BUILD_PARSER = $(CC) $(CFLAGS) -o $(BIN_PATH)parser $(PARSER_PATH)parser.c $(LEXER_PATH)lexer.c
BUILD_MAIN = $(CC) $(CFLAGS) -o $(BIN_PATH)main $(MAIN_PATH)main.c $(LEXER_PATH)lexer.c $(PARSER_PATH)parser.c

all: lexer compiler parser

main:
	$(BUILD_MAIN)

lexer:
	$(BUILD_LEXER)

runlexer:
	$(BIN_PATH)lexer.exe

compiler:
	$(BUILD_COMPILER)

parser:
	$(BUILD_PARSER)

test:
	$(TEST_SCRIPT)run_tests.bat

clean:
	del /F $(BIN_PATH)lexer.exe $(BIN_PATH)compiler.exe $(BIN_PATH)parser.exe

.PHONY: all lexer compiler parser clean runlexer
