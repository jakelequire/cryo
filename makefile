# Compiler and Flags
CC = clang
CFLAGS = -I"C:/Program Files/LLVM/include" -I./src/include -I./cli/ -Wall -g
LDFLAGS = -L"C:/Program Files/LLVM/lib"

# Define paths
BIN_DIR = ./src/bin/
SRC_DIR = ./src/
COMPILER_DIR = $(SRC_DIR)compiler/
UTILS_DIR = $(SRC_DIR)utils/
CLI_DIR = ./cli/
CLI_BIN_DIR = $(CLI_DIR)bin/
OBJS = $(SRCS:.c=.o)
TARGET = src/bin/main.exe

# Include directories
INCLUDES = -I./src/ -I./src/include/
CLI_INCLUDES = -I./cli/

# Source files
COMPILER_SRC =  $(COMPILER_DIR)lexer.c \
                $(COMPILER_DIR)parser.c \
                $(COMPILER_DIR)ast.c \
                $(COMPILER_DIR)codegen.c \
                $(COMPILER_DIR)ir.c \
                $(COMPILER_DIR)semantics.c

UTILS_SRC = $(UTILS_DIR)logger.c

CLI_SRC = $(CLI_DIR)cli.c \
          $(CLI_DIR)commands.c \
		  $(CLI_DIR)\compiler.c \
		  $(CLI_DIR)commands\build.c \
		  $(CLI_DIR)\commands\init.c \

MAIN_SRC = $(SRC_DIR)main.c

# Object files
COMPILER_OBJ = $(COMPILER_SRC:.c=.o)
UTILS_OBJ = $(UTILS_SRC:.c=.o)
CLI_OBJ = $(CLI_SRC:.c=.o)
MAIN_OBJ = $(MAIN_SRC:.c=.o)

# Binaries
LEXER_BIN = $(BIN_DIR)lexer.exe
PARSER_BIN = $(BIN_DIR)parser.exe
MAIN_BIN = $(BIN_DIR)main.exe
CLI_BIN_EXE = $(CLI_BIN_DIR)cryo.exe

# LLVM libraries (available libraries in your LLVM installation)
LLVM_LIBS = -lLLVM-C

# Default target
all: $(MAIN_BIN) $(CLI_BIN_EXE)

cli/cli.o: cli/cli.c cli/commands.h
	clang $(CFLAGS) -c cli/cli.c -o cli/cli.o

# Pattern rules for object files
$(COMPILER_DIR)%.o: $(COMPILER_DIR)%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(UTILS_DIR)%.o: $(UTILS_DIR)%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(CLI_DIR)%.o: $(CLI_DIR)%.c
	$(CC) $(CFLAGS) $(CLI_INCLUDES) -c $< -o $@

$(SRC_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Linking binaries
$(MAIN_BIN): $(MAIN_OBJ) $(COMPILER_OBJ) $(UTILS_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LLVM_LIBS)

$(CLI_BIN_EXE): $(CLI_OBJ)
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

# Clean up (Windows only) - remove object files and executables
clean:
	python ./scripts/clean.py



.PHONY: all clean runlexer runparser runmain runcli
