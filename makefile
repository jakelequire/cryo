#*******************************************************************************
#  Copyright 2024 Jacob LeQuire                                                *
#  SPDX-License-Identifier: Apache-2.0                                         *
#    Licensed under the Apache License, Version 2.0 (the "License");           *
#    you may not use this file except in compliance with the License.          *
#    You may obtain a copy of the License at                                   *
#                                                                              *
#    http://www.apache.org/licenses/LICENSE-2.0                                *
#                                                                              *
#    Unless required by applicable law or agreed to in writing, software       *
#    distributed under the License is distributed on an "AS IS" BASIS,         *
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
#    See the License for the specific language governing permissions and       *
#    limitations under the License.                                            *
#                                                                              *
#*******************************************************************************/

# --------------------------------------------- #
# `-O0`    - No optimization					#
# `-O1`    - Basic optimization					#
# `-O2`    - Further optimization				#
# `-O3`    - Maximum optimization				#
# `-Og`    - Optimize debugging experience		#
# `-Os`    - Optimize for size					#
# `-Ofast` - Optimize for speed					#
# `-Oz`    - Optimize for size					#
# --------------------------------------------- #
OPTIMIZATION = -O1
DEBUG_FLAGS =  -v -D_CRT_SECURE_NO_WARNINGS 
# C23
C_STANDARD = -std=c23
CXX_STANDARD = -std=c++23

# OS-specific settings for compilers
ifeq ($(OS), Windows_NT)
# Windows settings
	C_COMPILER = C:/msys64/mingw64/bin/gcc
	CXX_COMPILER = C:/msys64/mingw64/bin/g++
else
# Linux settings
	C_COMPILER = clang
	CXX_COMPILER = clang++
endif

# OS-specific settings
ifeq ($(OS), Windows_NT)
    # Windows settings
    CC = $(C_COMPILER) $(C_STANDARD) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CXX = $(CXX_COMPILER) $(CXX_STANDARD) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CFLAGS = -I"C:/msys64/mingw64/include" -I./src/include -I./src/include/runtime -I./src/include/cli \
			-I./src/include/compiler -I./src/include/utils -I./src/include/tests
    CXXFLAGS = -I"C:/msys64/mingw64/include" -I./src/include -I./src/include/runtime -I./src/include/cli \
			-I./src/include/compiler -I./src/include/utils -I./src/include/tests
    LDFLAGS = -L"C:/msys64/mingw64/lib" $(LLVM_LIBS) $(STDLIBS) -v
    LLVM_LIBS := -lLLVM 
    STDLIBS := -lmingw32 -lmingwex -lmsvcrt -lucrt -lpthread -lws2_32 -ladvapi32 -lshell32 -luser32 -lkernel32 -Wl,-subsystem,console
    MKDIR = if not exist
    RMDIR = rmdir /S /Q
    DEL = del /Q
    BIN_SUFFIX = .exe
else
    # Linux settings
    CC = $(C_COMPILER) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CXX = $(CXX_COMPILER) $(DEBUG_FLAGS) $(OPTIMIZATION)
    CFLAGS = -I./src/include -I./src/include/runtime -I./src/include/cli -I./src/include/compiler \
			-I./src/include/utils -I./src/include/tests
    CXXFLAGS = -I./src/include -I./src/include/runtime -I./src/include/cli -I./src/include/compiler \
			-I./src/include/utils -I./src/include/tests $(LLVM_CXXFLAGS) -fexceptions
    LLVM_CONFIG = llvm-config
    LLVM_CFLAGS = $(shell $(LLVM_CONFIG) --cflags)
	LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
    LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags) $(shell $(LLVM_CONFIG) --libs) $(shell $(LLVM_CONFIG) --system-libs)
    LDFLAGS = $(LLVM_LDFLAGS) -lpthread -v
	STD_LIBS = -lstdc++ -lm -lc -lgcc -lgcc_eh -lstdc++fs
    MKDIR = mkdir -p
    RMDIR = rm -rf
    DEL = rm -f
    BIN_SUFFIX =
endif

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
CRYO_DIR = $(SRC_DIR)cryo/
CPP_SEMANTICS_DIR = $(SRC_DIR)cpp/semantics/
CPP_UTILS_DIR = $(SRC_DIR)cpp/utils/
COMMON_DIR = $(SRC_DIR)common/

# --------------------------------------------------------------------------------- #

# CPP files
CPP_DIR = $(SRC_DIR)cpp/

# Source files
COMPILER_SRC = 	$(COMPILER_DIR)containers.c $(COMPILER_DIR)ast.c $(COMPILER_DIR)semantics.c $(COMPILER_DIR)lexer.c  \
				$(COMPILER_DIR)parser.c $(COMPILER_DIR)token.c $(COMPILER_DIR)symtable.c \
				$(COMPILER_DIR)error.c $(COMPILER_DIR)typedefs.c

# CLI files
CLI_SRC = $(CLI_DIR)cli.c $(CLI_COMMANDS_DIR)cmd_build.c $(CLI_COMMANDS_DIR)cmd_init.c \
			$(CLI_COMMANDS_DIR)cmd_devWatch.c $(CLI_COMMANDS_DIR)cmd_help.c $(CLI_COMMANDS_DIR)cmd_version.c 

# Utils files
UTILS_SRC = $(UTILS_DIR)fs.c $(UTILS_DIR)supportlibs.c $(UTILS_DIR)arena.c $(UTILS_DIR)utility.c

# CPP Files
CPPSRC = $(CPP_DIR)cppmain.cpp $(CPP_UTILS_DIR)backend_symtable.cpp $(CPP_DIR)codegen.cpp \
		$(CPP_DIR)generator.cpp $(CPP_DIR)types.cpp $(CPP_UTILS_DIR)debugger.cpp \
		$(CPP_SEMANTICS_DIR)variables.cpp $(CPP_SEMANTICS_DIR)functions.cpp \
		$(CPP_SEMANTICS_DIR)arrays.cpp $(CPP_DIR)declarations.cpp $(CPP_DIR)ifstatement.cpp \
		$(CPP_SEMANTICS_DIR)forloop.cpp $(CPP_SEMANTICS_DIR)binaryExpression.cpp $(CPP_SEMANTICS_DIR)structs.cpp \
		$(CPP_SEMANTICS_DIR)imports.cpp $(CPP_DIR)compilation.cpp $(CPP_SEMANTICS_DIR)whileStatement.cpp

# Common Files
COMMON_SRC = $(COMMON_DIR)common.c

# Main files
MAIN_SRC = $(SRC_DIR)main.c $(SRC_DIR)build.c $(SRC_DIR)compiler.c $(SRC_DIR)settings.c
RUNTIME_SRC = $(RUNTIME_DIR)runtime.c

# --------------------------------------------------------------------------------- #

# Cryo Lib Files
CRYO_SRC = $(CRYO_DIR)cryolib.c

# Object files
COMPILER_OBJ =  $(OBJ_DIR)global_compiler.o $(OBJ_DIR)settings.o $(OBJ_DIR)containers.o $(OBJ_DIR)ast.o $(OBJ_DIR)semantics.o \
				$(OBJ_DIR)lexer.o $(OBJ_DIR)parser.o $(OBJ_DIR)token.o $(OBJ_DIR)symtable.o $(OBJ_DIR)error.o $(OBJ_DIR)typedefs.o \

# CPP Object files
CPPOBJ =$(OBJ_DIR)global_compiler.o $(OBJ_DIR)debugger.o $(OBJ_DIR)backend_symtable.o $(OBJ_DIR)codegen.o \
		$(OBJ_DIR)cppmain.o $(OBJ_DIR)variables.o $(OBJ_DIR)functions.o $(OBJ_DIR)arrays.o \
		$(OBJ_DIR)generator.o $(OBJ_DIR)types.o $(OBJ_DIR)declarations.o $(OBJ_DIR)ifstatement.o \
		$(OBJ_DIR)forloop.o $(OBJ_DIR)binaryExpression.o $(OBJ_DIR)structs.o $(OBJ_DIR)imports.o \
		$(OBJ_DIR)compilation.o $(OBJ_DIR)whileStatement.o

# Utils Object files
UTILS_OBJ = $(OBJ_DIR)fs.o $(OBJ_DIR)supportlibs.o $(OBJ_DIR)arena.o $(OBJ_DIR)utility.o

# Common Object files
COMMON_OBJ = $(OBJ_DIR)common.o

# CLI Object files
CLI_OBJ = $(OBJ_DIR)cli.o $(OBJ_DIR)cmd_build.o $(OBJ_DIR)cmd_init.o \
			$(OBJ_DIR)cmd_devWatch.o $(OBJ_DIR)cmd_help.o $(OBJ_DIR)cmd_version.o 

# Main Object files
MAIN_OBJ = $(OBJ_DIR)main.o $(OBJ_DIR)build.o $(OBJ_DIR)global_compiler.o  $(OBJ_DIR)settings.o
RUNTIME_OBJ = $(OBJ_DIR)runtime.o
TEST_OBJ = $(OBJ_DIR)test.o

# Cryo Lib Object files
CRYO_OBJ = $(OBJ_DIR)cryolib.o

# --------------------------------------------------------------------------------- #

# Define the target binaries
MAIN_BIN = $(BIN_DIR)main$(BIN_SUFFIX)
CLI_BIN_EXE = $(BIN_DIR)cryo$(BIN_SUFFIX)
VISUAL_DEBUG_BIN = $(BIN_DIR)visualDebug$(BIN_SUFFIX)

# Default target
all: $(MAIN_BIN) $(CLI_BIN_EXE)

# Ensure the object directory exists
$(OBJ_DIR):
	$(MKDIR) $(OBJ_DIR)

# Ensure the bin directory exists
$(BIN_DIR):
	$(MKDIR) $(BIN_DIR)


# --------------------------------------------- #
#             Object file rules 				#
# --------------------------------------------- #

# ---------------------------------------------
# Utils Compilation rules
$(OBJ_DIR)fs.o: $(UTILS_DIR)fs.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)arena.o: $(UTILS_DIR)arena.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)supportlibs.o: $(UTILS_DIR)supportlibs.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)utility.o: $(UTILS_DIR)utility.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------------------
# Compilation rules
$(OBJ_DIR)%.o: $(COMPILER_DIR)%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)main.o: $(SRC_DIR)main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)build.o: $(SRC_DIR)build.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)runtime.o: $(RUNTIME_DIR)runtime.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)token.o : $(COMPILER_DIR)token.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)symtable.o : $(COMPILER_DIR)symtable.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)error.o : $(COMPILER_DIR)error.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)typedefs.o : $(COMPILER_DIR)typedefs.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)global_compiler.o: $(SRC_DIR)compiler.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)settings.o: $(SRC_DIR)settings.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	
# ---------------------------------------------
# CLI Compilation rules
$(OBJ_DIR)cli.o: $(CLI_DIR)cli.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)cmd_init.o: $(CLI_COMMANDS_DIR)cmd_init.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)cmd_build.o: $(CLI_COMMANDS_DIR)cmd_build.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)cmd_devWatch.o : $(CLI_COMMANDS_DIR)cmd_devWatch.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)cmd_help.o : $(CLI_COMMANDS_DIR)cmd_help.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)cmd_version.o : $(CLI_COMMANDS_DIR)cmd_version.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------------------
# CPP Compilation rules
$(OBJ_DIR)debugger.o: $(CPP_UTILS_DIR)debugger.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)backend_symtable.o: $(CPP_UTILS_DIR)backend_symtable.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)cppmain.o: $(CPP_DIR)cppmain.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)codegen.o: $(CPP_DIR)codegen.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)generator.o: $(CPP_DIR)generator.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)types.o: $(CPP_DIR)types.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)declarations.o: $(CPP_DIR)declarations.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)ifstatement.o: $(CPP_SEMANTICS_DIR)ifstatement.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)variables.o: $(CPP_SEMANTICS_DIR)variables.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)functions.o: $(CPP_SEMANTICS_DIR)functions.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)arrays.o: $(CPP_SEMANTICS_DIR)arrays.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)forloop.o: $(CPP_SEMANTICS_DIR)forloop.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)binaryExpression.o: $(CPP_SEMANTICS_DIR)binaryExpression.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)structs.o: $(CPP_SEMANTICS_DIR)structs.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)imports.o: $(CPP_SEMANTICS_DIR)imports.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)compilation.o: $(CPP_DIR)compilation.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)whileStatement.o: $(CPP_SEMANTICS_DIR)whileStatement.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
# ---------------------------------------------
# Common Compilation rules
$(OBJ_DIR)common.o: $(COMMON_DIR)common.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------------------
# Cryo Lib Compilation rules
$(OBJ_DIR)cryolib.o: $(CRYO_DIR)cryolib.c | $(OBJ_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@ 

# ---------------------------------------------
# Linking binaries
$(MAIN_BIN): $(MAIN_OBJ) $(COMPILER_OBJ) $(UTILS_OBJ) $(CPPOBJ) $(RUNTIME_OBJ) $(CRYO_OBJ) $(COMMON_OBJ) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(CLI_BIN_EXE): $(CLI_OBJ) $(COMPILER_OBJ) $(UTILS_OBJ) $(CPPOBJ) $(CRYO_OBJ) $(COMMON_OBJ) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(TEST_BIN): $(TEST_OBJ) $(RUNTIME_OBJ) $(COMPILER_OBJ) $(UTILS_OBJ) | $(DEBUG_BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

# ---------------------------------------------
# Clean up - remove object files and executables
clean:
	python3 ./scripts/clean.py

.PHONY: all clean runlexer runparser runmain runcli runtest
