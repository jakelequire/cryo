# Compiler and Flags
CC = clang -g -D_CRT_SECURE_NO_WARNINGS
CFLAGS = -I"C:/Program Files/LLVM/include" -I./src/include -Wall -g
LDFLAGS = -L"C:/Program Files/LLVM/lib"

# Define paths
BIN_DIR = ./src/bin/
OBJ_DIR = $(BIN_DIR).o/
DEBUG_BIN_DIR = $(BIN_DIR)debug/
SRC_DIR = ./src/
COMPILER_DIR = $(SRC_DIR)compiler/
UTILS_DIR = $(SRC_DIR)utils/
CLI_DIR = $(SRC_DIR)cli/
CLI_COMMANDS_DIR = $(CLI_DIR)commands/
CLI_BIN_DIR = $(BIN_DIR)
TESTS_DIR = $(SRC_DIR)tests/

# Source files
COMPILER_SRC = $(COMPILER_DIR)ast.c $(COMPILER_DIR)codegen.c $(COMPILER_DIR)ir.c $(COMPILER_DIR)semantics.c $(COMPILER_DIR)lexer.c $(COMPILER_DIR)parser.c
UTILS_SRC = $(UTILS_DIR)logger.c
CLI_SRC = $(CLI_DIR)cli.c $(CLI_DIR)commands.c $(CLI_DIR)compiler.c $(CLI_COMMANDS_DIR)build.c $(CLI_COMMANDS_DIR)init.c
MAIN_SRC = $(SRC_DIR)main.c
LEXER_TEST_SRC = $(TESTS_DIR)lexer.test.c
PARSER_TEST_SRC = $(TESTS_DIR)parser.test.c

# Object files
COMPILER_OBJ = $(patsubst %.c, $(OBJ_DIR)%.o, $(notdir $(COMPILER_SRC)))
UTILS_OBJ = $(patsubst %.c, $(OBJ_DIR)%.o, $(notdir $(UTILS_SRC)))
CLI_OBJ = $(patsubst %.c, $(OBJ_DIR)%.o, $(notdir $(CLI_SRC)) $(notdir $(wildcard $(CLI_COMMANDS_DIR)/*.c)))
MAIN_OBJ = $(patsubst %.c, $(OBJ_DIR)%.o, $(notdir $(MAIN_SRC)))
LEXER_TEST_OBJ = $(patsubst %.c, $(OBJ_DIR)%.o, $(notdir $(LEXER_TEST_SRC)))
PARSER_TEST_OBJ = $(patsubst %.c, $(OBJ_DIR)%.o, $(notdir $(PARSER_TEST_SRC)))

# Binaries
LEXER_BIN = $(DEBUG_BIN_DIR)lexer.exe
PARSER_BIN = $(DEBUG_BIN_DIR)parser.exe
MAIN_BIN = $(BIN_DIR)main.exe
CLI_BIN_EXE = $(CLI_BIN_DIR)cryo.exe

# LLVM libraries (available libraries in your LLVM installation)
LLVM_LIBS = -lLLVM-C

# Default target
all: > $ clean_previous $(MAIN_BIN) $(CLI_BIN_EXE) $(LEXER_BIN) $(PARSER_BIN)

clean_previous:
	@if exist $(BIN_DIR)cleaned.txt $echo Hello

# Pattern rules for object files
$(OBJ_DIR)%.o: $(COMPILER_DIR)%.c
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)%.o: $(UTILS_DIR)%.c
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)%.o: $(CLI_DIR)%.c
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)%.o: $(CLI_COMMANDS_DIR)%.c
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)%.o: $(TESTS_DIR)%.c
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Linking binaries
$(MAIN_BIN): $(MAIN_OBJ) $(COMPILER_OBJ) $(UTILS_OBJ)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS) $(LLVM_LIBS)

$(CLI_BIN_EXE): $(CLI_OBJ)
	@if not exist $(CLI_BIN_DIR) mkdir $(CLI_BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS) $(LLVM_LIBS)

$(LEXER_BIN): $(LEXER_TEST_OBJ) $(OBJ_DIR)lexer.o $(OBJ_DIR)ast.o $(OBJ_DIR)codegen.o $(OBJ_DIR)ir.o $(OBJ_DIR)semantics.o
	@if not exist $(DEBUG_BIN_DIR) mkdir $(DEBUG_BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS) $(LLVM_LIBS)

$(PARSER_BIN): $(PARSER_TEST_OBJ) $(OBJ_DIR)parser.o $(OBJ_DIR)lexer.o $(OBJ_DIR)ast.o $(OBJ_DIR)codegen.o $(OBJ_DIR)ir.o $(OBJ_DIR)semantics.o $(OBJ_DIR)logger.o
	@if not exist $(DEBUG_BIN_DIR) mkdir $(DEBUG_BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS) $(LLVM_LIBS)

# Running executables
runlexer: $(LEXER_BIN)
	$(LEXER_BIN)

runparser: $(PARSER_BIN)
	$(PARSER_BIN)

runmain: $(MAIN_BIN)
	$(MAIN_BIN) ./src/tests/data/test1.cy

runcli: $(CLI_BIN_EXE)
	$(CLI_BIN_EXE)

# Clean up - remove object files and executables
clean:
	python ./scripts/clean.py

.PHONY: all clean runlexer runparser runmain runcli
