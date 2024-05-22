# Compiler and Flags
CC = clang
CFLAGS = -I"C:/Program Files/LLVM/include" -I./src/include -Wall -g
LDFLAGS = -L"C:/Program Files/LLVM/lib"

# Define paths
BIN_DIR = ./src/bin/
SRC_DIR = ./src/
COMPILER_DIR = $(SRC_DIR)compiler/
UTILS_DIR = $(SRC_DIR)utils/
CLI_DIR = ./cli/
CLI_BIN_DIR = $(CLI_DIR)bin/

# Include directories
INCLUDES = -I./src/ -I./src/include/ 
CLI_INCLUDES = -I./cli/ 

# Source files
LEXER_SRC = $(COMPILER_DIR)lexer.c
PARSER_SRC = $(COMPILER_DIR)parser.c
CLI_SRC = $(CLI_DIR)cli.c
UTILS_SRC = $(UTILS_DIR)printAST.c
AST_SRC = $(COMPILER_DIR)ast.c
CODEGEN_SRC = $(COMPILER_DIR)codegen.c
IR_SRC = $(COMPILER_DIR)ir.c
MAIN_SRC = $(SRC_DIR)main.c
SEMANTIC_SRC = $(COMPILER_DIR)semanticS.c
COMMANDS_SRC = $(CLI_DIR)commands.c

# Object files
LEXER_OBJ = $(LEXER_SRC:.c=.o)
PARSER_OBJ = $(PARSER_SRC:.c=.o)
CLI_OBJ = $(CLI_SRC:.c=.o)
UTILS_OBJ = $(UTILS_SRC:.c=.o)
AST_OBJ = $(AST_SRC:.c=.o)
CODEGEN_OBJ = $(CODEGEN_SRC:.c=.o)
IR_OBJ = $(IR_SRC:.c=.o)
MAIN_OBJ = $(MAIN_SRC:.c=.o)
SEMANTIC_OBJ = $(SEMANTIC_SRC:.c=.o)
COMMANDS_OBJ = $(COMMANDS_SRC:.c=.o)

# Binaries
LEXER_BIN = $(BIN_DIR)lexer.exe
PARSER_BIN = $(BIN_DIR)parser.exe
MAIN_BIN = $(BIN_DIR)main.exe
CLI_BIN_EXE = $(CLI_BIN_DIR)cryo.exe

# LLVM libraries (available libraries in your LLVM installation)
LLVM_LIBS = -lLLVM-C -llibclang -lliblldb -llibomp -lLTO -lRemarks

# Default target
all: $(CLI_BIN_EXE) $(MAIN_BIN)

# Individual components
$(LEXER_OBJ): $(LEXER_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(PARSER_OBJ): $(PARSER_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(CLI_OBJ): $(CLI_SRC)
	$(CC) $(CFLAGS) $(CLI_INCLUDES) -c $< -o $@

$(UTILS_OBJ): $(UTILS_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(AST_OBJ): $(AST_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(CODEGEN_OBJ): $(CODEGEN_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(IR_OBJ): $(IR_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(MAIN_OBJ): $(MAIN_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SEMANTIC_OBJ): $(SEMANTIC_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(COMMANDS_OBJ): $(COMMANDS_SRC)
	$(CC) $(CFLAGS) $(CLI_INCLUDES) -c $< -o $@

# Linking binaries
$(LEXER_BIN): $(LEXER_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LLVM_LIBS)

$(PARSER_BIN): $(PARSER_OBJ) $(LEXER_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LLVM_LIBS)

$(MAIN_BIN): $(MAIN_OBJ) $(LEXER_OBJ) $(PARSER_OBJ) $(AST_OBJ) $(CODEGEN_OBJ) $(IR_OBJ) $(SEMANTIC_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LLVM_LIBS)

$(CLI_BIN_EXE): $(CLI_OBJ) $(COMMANDS_OBJ)
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

# Clean up
clean:
	rm -f $(BIN_DIR)*.exe $(CLI_BIN_DIR)*.exe $(SRC_DIR)compiler/*.o $(SRC_DIR)utils/*.o $(CLI_DIR)*.o

.PHONY: all clean runlexer runparser runmain runcli
