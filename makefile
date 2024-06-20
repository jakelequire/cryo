# Compiler and Flags
CC = clang -g -v -D_CRT_SECURE_NO_WARNINGS
CFLAGS = -I"C:/Program Files (x86)/LLVM/include" -I./src/include -I./src/include/runtime -I./src/include/cli -I./src/include/compiler -I./src/include/utils -I./src/include/tests
LDFLAGS = -L"C:/Program Files (x86)/LLVM/lib" -lLLVM-C
LLVM_LIBS = 


# Define paths
BIN_DIR = ./src/bin/
OBJ_DIR = $(BIN_DIR).o/
DEBUG_BIN_DIR = $(BIN_DIR)debug/
SRC_DIR = ./src/
COMPILER_DIR = $(SRC_DIR)compiler/
UTILS_DIR = $(SRC_DIR)utils/
CLI_DIR = $(SRC_DIR)cli/
CLI_COMMANDS_DIR = $(CLI_DIR)commands/
RUNTIME_DIR = $(SRC_DIR)runtime/
TESTS_DIR = $(SRC_DIR)tests/


# Source files
COMPILER_SRC = $(COMPILER_DIR)ast.c $(COMPILER_DIR)semantics.c $(COMPILER_DIR)lexer.c $(COMPILER_DIR)parser.c $(COMPILER_DIR)codegen.c
UTILS_SRC = $(UTILS_DIR)logger.c $(UTILS_SRC)fs.c
CLI_SRC = $(CLI_DIR)cli.c $(CLI_DIR)commands.c $(CLI_DIR)compiler.c $(CLI_COMMANDS_DIR)build.c $(CLI_COMMANDS_DIR)init.c $(CLI_COMMANDS_DIR)runtime_cmd.c
MAIN_SRC = $(SRC_DIR)main.c
RUNTIME_SRC = $(RUNTIME_DIR)runtime.c


# Object files
COMPILER_OBJ = $(OBJ_DIR)ast.o $(OBJ_DIR)semantics.o $(OBJ_DIR)lexer.o $(OBJ_DIR)parser.o $(OBJ_DIR)codegen.o 
UTILS_OBJ = $(OBJ_DIR)logger.o $(OBJ_DIR)fs.o
CLI_OBJ = $(OBJ_DIR)cli.o $(OBJ_DIR)commands.o $(OBJ_DIR)compiler.o $(OBJ_DIR)build.o $(OBJ_DIR)init.o $(OBJ_DIR)runtime_cmd.o
MAIN_OBJ = $(OBJ_DIR)main.o
CODEGEN_TEST_OBJ = $(OBJ_DIR)codegen.test.o
RUNTIME_OBJ = $(OBJ_DIR)runtime.o
TEST_OBJ = $(OBJ_DIR)test.o


# Binaries
MAIN_BIN = $(BIN_DIR)main.exe
CLI_BIN_EXE = $(BIN_DIR)cryo.exe
RUNTIME_BIN = $(BIN_DIR)runtime.exe
CODEGEN_BIN = $(DEBUG_BIN_DIR)codegen.exe

# Default target
all: $(MAIN_BIN) $(CLI_BIN_EXE) $(RUNTIME_BIN) $(CODEGEN_BIN)

# Ensure the object directory exists
$(OBJ_DIR):
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

# Compilation rules
$(OBJ_DIR)%.o: $(COMPILER_DIR)%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)logger.o: $(UTILS_DIR)logger.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)fs.o: $(UTILS_DIR)fs.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)cli.o: $(CLI_DIR)cli.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)commands.o: $(CLI_DIR)commands.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)runtime_cmd.o: $(CLI_COMMANDS_DIR)runtime_cmd.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)compiler.o: $(CLI_DIR)compiler.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)build.o: $(CLI_COMMANDS_DIR)build.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)init.o: $(CLI_COMMANDS_DIR)init.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)main.o: $(SRC_DIR)main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)codegen.test.o: $(TESTS_DIR)codegen.test.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)runtime.o: $(RUNTIME_DIR)runtime.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)codegen.o: $(COMPILER_DIR)codegen.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	
# Linking binaries
$(MAIN_BIN): $(MAIN_OBJ) $(COMPILER_OBJ) $(UTILS_OBJ)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS) $(LLVM_LIBS)

$(CLI_BIN_EXE): $(CLI_OBJ) $(UTILS_OBJ)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(RUNTIME_BIN): $(RUNTIME_OBJ) $(UTILS_OBJ) $(COMPILER_OBJ)
	@if not exist $(DEBUG_BIN_DIR) mkdir $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	
$(CODEGEN_BIN): $(CODEGEN_TEST_OBJ) $(UTILS_OBJ) $(OBJ_DIR)ast.o $(OBJ_DIR)parser.o $(OBJ_DIR)lexer.o $(OBJ_DIR)codegen.o
	@if not exist $(DEBUG_BIN_DIR) mkdir $(DEBUG_BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_BIN): $(TEST_OBJ) $(RUNTIME_OBJ) $(COMPILER_OBJ) $(UTILS_OBJ) 
	@if not exist $(DEBUG_BIN_DIR) mkdir $(DEBUG_BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

# Running executables
runmain: $(MAIN_BIN)
	$(MAIN_BIN) ./src/tests/data/test1.cy

runcli: $(CLI_BIN_EXE)
	$(CLI_BIN_EXE)

runtest: $(TEST_BIN)
	$(TEST_BIN)

# Clean up - remove object files and executables
clean:
	python ./scripts/clean.py

.PHONY: all clean runlexer runparser runmain runcli runtest